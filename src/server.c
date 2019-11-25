/*
 * This file is part of the zokete distribution
 * (https://github.com/matricali/zokete). Copyright (c) 2019 Jorge Matricali.
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
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "logger.h"

#define BUFSIZE 8096

void zk_server_process_request(int socket_fd)
{
    long ret;

    static char buffer[BUFSIZE + 1];
    // struct http_request request = {};

    ret = read(socket_fd, buffer, BUFSIZE);

    printf("Recibiendo %d bytes...\n", ret);

    for (int i = 0; i < ret; ++i) {
        printf("[%d]=%02x\n", i, buffer[i]);
    }

    if (ret == 0 || ret == -1) {
        (void)close(socket_fd);
        zk_logger(ZK_LOG_ERROR, "Failed to read from client.\n");
    }

    if (ret > 0 && ret < BUFSIZE) {
        /* Cerrar buffer */
        buffer[ret] = 0;
    } else {
        buffer[0] = 0;
    }

    sprintf(buffer, "HTTP/1.1 200 OK\nServer: zokete\nContent-Length: 0\nConnection: close\n\n");
    (void)write(socket_fd, buffer, strlen(buffer));

    sleep(1);
    close(socket_fd);
    exit(1);
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
