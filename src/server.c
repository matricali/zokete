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
#include <errno.h>
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
#include "socks/replies.h"

#define BUFSIZE 8096

void zk_server_process_request(int socket_fd)
{
    long ret;

    static uint8_t buffer[BUFSIZE + 1];

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
    while (--nmethods >= 0) {
        if (buffer[++i] == 0x00) {
            valid_method = true;
        }
    }

    if (!valid_method) {
        zk_logger(ZK_LOG_ERROR, "No valid authentication method.\n");
        uint8_t msg[2] = { 0x05, 0xFF };
        zk_server_write(socket_fd, msg, sizeof(msg));
        goto close_routine;
    }

    // METHOD selection message
    uint8_t msg[2] = { 0x05, 0x00 };
    zk_server_write(socket_fd, msg, sizeof(msg));

    // 4. Request
    ret = zk_server_read(socket_fd, buffer, BUFSIZE);
    i = 0;

    uint8_t protocol = buffer[i++];
    uint8_t command = buffer[i++];
    uint8_t rsv = buffer[i++];
    uint8_t atyp = buffer[i++];

    if (command != 0x01) { // CONNECT
        zk_logger(ZK_LOG_ERROR, "Command not supported. (%02x)\n", command);
        uint8_t msg[10] = { 0x05, ZK_SOCKS_REP_COMMAND_NOT_SUPPORTED,
            0x00, atyp, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00 };
        zk_server_write(socket_fd, msg, sizeof(msg));
        goto close_routine;
    }

    if (atyp != 0x01) { // IP_V4
        zk_logger(ZK_LOG_ERROR, "Address type not supported. (%02x)\n", atyp);
        uint8_t msg[10] = { 0x05, ZK_SOCKS_REP_ADDRESS_TYPE_NOT_SUPPORTED,
            0x00, atyp, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0 };
        zk_server_write(socket_fd, msg, sizeof(msg));
        goto close_routine;
    }

    uint32_t dst_addr_ipv4 = 0;

    for (int x = 0; x < 4; x++) {
        dst_addr_ipv4 = (dst_addr_ipv4 << CHAR_BIT) | buffer[i++];
    }

    uint16_t dst_port = (buffer[i++] << CHAR_BIT) | buffer[i++];
    struct in_addr dst_addr;
    dst_addr.s_addr = htonl(dst_addr_ipv4);

    zk_logger(ZK_LOG_INFO, "CONNECT %s:%d\n", inet_ntoa(dst_addr), dst_port);

    // Connect to target host
    int target_sockfd;
    struct sockaddr_in target_servaddr;

    target_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (target_sockfd == -1) {
        zk_logger(ZK_LOG_FATAL, "Socket creation failed...\n");
        uint8_t reply[10] = { 0x05, ZK_SOCKS_REP_SERVER_FAILURE, 0x00, atyp, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0 };
        zk_server_write(socket_fd, reply, sizeof(reply));
        goto close_routine;
    }

    bzero(&target_servaddr, sizeof(target_servaddr));
    target_servaddr.sin_family = AF_INET;
    target_servaddr.sin_addr = dst_addr;
    target_servaddr.sin_port = htons(dst_port);

    if (connect(target_sockfd, (struct sockaddr*)&target_servaddr, sizeof(target_servaddr)) != 0) {
        zk_logger(ZK_LOG_FATAL, "Connection with the server failed....\n");
        uint8_t reply[10] = { 0x05, ZK_SOCKS_REP_CONNECTION_REFUSED, 0x00, atyp, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0 };
        zk_server_write(socket_fd, reply, sizeof(reply));
        goto close_routine;
    }

    // Connection OK
    zk_logger(ZK_LOG_INFO, "%s:%d Connection succesfull!\n", inet_ntoa(dst_addr), dst_port);
    uint8_t reply[10] = { 0x05, ZK_SOCKS_REP_SUCCEEDED, 0x00, atyp, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0 };
    zk_server_write(socket_fd, reply, sizeof(reply));

    zk_server_socket_pipe(socket_fd, target_sockfd);

close_routine:
    sleep(1);
    close(socket_fd);
    exit(EXIT_SUCCESS);
}

int zk_server_read(int fd, uint8_t buf[], size_t nbyte)
{
    int ret = read(fd, buf, nbyte);
    printf("%d bytes recibidos: ", ret);

    for (int i = 0; i < ret; ++i) {
        printf("%02x ", buf[i]);
    }
    printf("\n");
    return ret;
}

int zk_server_write(int fd, uint8_t buf[], size_t nbyte)
{
    int ret = write(fd, buf, nbyte);
    printf("%d bytes enviados: ", ret);

    for (int i = 0; i < ret; ++i) {
        printf("%02x ", buf[i]);
    }
    printf("\n");
    return ret;
}

void zk_server_socket_pipe(int fd0, int fd1)
{
    int maxfd, ret;
    fd_set rd_set;
    size_t nread;
    char buffer_r[BUFSIZE];

    zk_logger(ZK_LOG_INFO, "Connecting two sockets\n");

    maxfd = (fd0 > fd1) ? fd0 : fd1;
    for (;;) {
        FD_ZERO(&rd_set);
        FD_SET(fd0, &rd_set);
        FD_SET(fd1, &rd_set);
        ret = select(maxfd + 1, &rd_set, NULL, NULL, NULL);

        if (ret < 0 && errno == EINTR) {
            continue;
        }

        if (FD_ISSET(fd0, &rd_set)) {
            nread = zk_server_read(fd0, buffer_r, BUFSIZE);
            if (nread <= 0) {
                break;
            }
            zk_server_write(fd1, buffer_r, nread);
        }

        if (FD_ISSET(fd1, &rd_set)) {
            nread = zk_server_read(fd1, buffer_r, BUFSIZE);
            if (nread <= 0) {
                break;
            }
            zk_server_write(fd0, buffer_r, nread);
        }
    }
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
