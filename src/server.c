/*
 * This file is part of the zokete distribution
 * (https://github.com/matricali/zokete).
 *
 * Copyright (c) 2019 Jorge Matricali.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <arpa/inet.h>
#include <limits.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "logger.h"
#include "server.h"

#define BUFSIZE 8096

void zk_server_process_request(int socket_fd)
{
    long ret;

    static unsigned char buffer[BUFSIZE + 1];

    ret = zk_server_read(socket_fd, buffer, BUFSIZE);

    if (ret == 0 || ret == -1) {
        (void)close(socket_fd);
        zk_logger(ZK_LOG_ERROR, "Failed to read from client.\n");
        goto close_routine;
    }

    if (ret > 0 && ret < BUFSIZE) {
        /* Cerrar buffer */
        buffer[ret] = 0;
    } else {
        buffer[0] = 0;
    }

    // cursor position
    int i = 0;

    // version identifier
    if (buffer[i] != 0x05) {
        zk_logger(ZK_LOG_INFO, "Unsuported version (%02x)\n", buffer[i]);
        goto close_routine;
    }

    // authentication method selection
    int nmethods = (int)buffer[++i];

    // looking for implemented authentication method
    bool valid_method = false;
    while (--nmethods > 0) {
        if (buffer[++i] == 0x00) {
            valid_method = true;
        }
    }

    if (!valid_method) {
        zk_logger(ZK_LOG_ERROR, "No valid authentication method.\n");
        unsigned char msg[2] = { 0x05, 0xFF };
        zk_server_write(socket_fd, msg, sizeof(msg));
        goto close_routine;
    }

    // METHOD selection message
    unsigned char msg[2] = { 0x05, 0x00 };
    zk_server_write(socket_fd, msg, sizeof(msg));

    // 4. Request
    ret = zk_server_read(socket_fd, buffer, BUFSIZE);
    i = 0;

    char protocol = buffer[i++];
    char command = buffer[i++];
    char rsv = buffer[i++];
    char atyp = buffer[i++];

    printf("Protocolo version: %c\n", protocol);
    printf("Comando: ");
    switch (command) {
    case 0x01:
        printf("CONNECT");
        break;
    case 0x02:
        printf("BIND");
        break;
    case 0x03:
        printf("UDP_ASSOCIATE");
        break;
    }
    printf("\nFamilia: ");
    switch (atyp) {
    case 0x01:
        printf("IP_V4");
        break;
    case 0x03:
        printf("DOMAIN_NAME");
        break;
    case 0x04:
        printf("IP_V6");
        break;
    }
    printf("\n");

    if (command != 0x01) { // CONNECT
        zk_logger(ZK_LOG_ERROR, "Command not supported. (%02x)\n", command);
        unsigned char msg[10] = { 0x05, 0x07, 0x00, atyp, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00 };
        zk_server_write(socket_fd, msg, sizeof(msg));
        goto close_routine;
    }

    if (atyp != 0x01) { // IP_V4
        zk_logger(ZK_LOG_ERROR, "Address type not supported. (%02x)\n", atyp);
        unsigned char msg[10] = { 0x05, 0x08, 0x00, atyp, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0 };
        zk_server_write(socket_fd, msg, sizeof(msg));
        goto close_routine;
    }

    // struct  sockaddr_in dst_addr;
    // memset(&dst_addr, 0, sizeof(dst_addr));
    // dst_addr.sin_family = AF_INET;
    // dst_addr.sin_port = htons(port);
    // // dst_addr.sin_addr =

    // unsigned long dst_addr_l = 0;
    // memcpy(dst_addr_l, (unsigned long*)&buffer[i], 4);
    i += 4;
    // for (int c = 0; c < 4; ++c) {
    //     i++;
    //     dst_addr_l |= ((unsigned long)buffer[i] << (4 * c));
    // }
    // unsigned int dst_port = (unsigned char)buffer[i++] << CHAR_BIT | (unsigned char)buffer[i++];
    unsigned int dst_port = 0;
    memcpy(dst_port, (unsigned int*)&buffer[i], 2);
    i += 2;

    // struct in_addr dst_addr;
    // dst_addr.s_addr = dst_addr_l;
    // printf("DST_ADDR: %s\n", inet_ntoa(dst_addr));
    printf("DST_PORT: %d\n", dst_port);

close_routine:
    sleep(1);
    close(socket_fd);
    exit(EXIT_SUCCESS);
}

int zk_server_read(int fd, unsigned char buf[], size_t nbyte)
{
    int ret = read(fd, buf, nbyte);
    printf("%d bytes recibidos: ", ret);

    for (int i = 0; i < ret; ++i) {
        printf("%02x ", buf[i]);
    }
    printf("\n");
    return ret;
}

int zk_server_write(int fd, unsigned char buf[], size_t nbyte)
{
    int ret = write(fd, buf, nbyte);
    printf("%d bytes enviados: ", ret);

    for (int i = 0; i < ret; ++i) {
        printf("%02x ", buf[i]);
    }
    printf("\n");
    return ret;
}

int zk_server_start(const unsigned int port)
{
    int listenfd = 0;
    int socketfd = 0;
    int iEnabled = 1;
    pid_t pid = 0;

    static struct sockaddr_in serv_addr;
    static struct sockaddr_in cli_addr;
    socklen_t len = 0;

    zk_logger(ZK_LOG_INFO, "Starting zoketed on port %d...\n", port);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        zk_logger(ZK_LOG_FATAL, "Error opening listen socket.\n");
        return EXIT_FAILURE;
    }

    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char*)&iEnabled,
        sizeof(iEnabled));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        zk_logger(ZK_LOG_FATAL, "Cannot bind port\n");
        return EXIT_FAILURE;
    }

    if (listen(listenfd, 64) < 0) {
        zk_logger(ZK_LOG_FATAL, "Cannot listen on port\n");
        return EXIT_FAILURE;
    }

    for (;;) {
        len = sizeof(cli_addr);

        socketfd = accept(listenfd, (struct sockaddr*)&cli_addr, &len);

        char client_address[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(cli_addr.sin_addr), client_address, INET_ADDRSTRLEN);

        if (socketfd < 0) {
            zk_logger(ZK_LOG_FATAL, "Cannot accept incoming connection from %s\n", client_address);
            (void)close(socketfd);
            return EXIT_FAILURE;
        }

        zk_logger(ZK_LOG_INFO, "Incoming connection from %s\n", client_address);

        pid = fork();
        if (pid < 0) {
            zk_logger(ZK_LOG_FATAL, "Cannot fork!\n");
            return EXIT_FAILURE;
        }

        if (pid == 0) {
            (void)close(listenfd);
            zk_server_process_request(socketfd);
        } else {
            (void)close(socketfd);
        }
    }
}
