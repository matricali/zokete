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

#include "enums.h"
#include "logger.h"
#include "server.h"

#define BUFSIZE 8096

void zk_socks_write_reply(zk_server_connection_t conn, zk_socks_rep_e rep, zk_socks_atyp_e atyp)
{
    char reply[10] = { 0x05, rep, 0x00, atyp, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0 };
    zk_server_write(conn, reply, sizeof(reply));
}

void zk_server_process_request(zk_server_connection_t cli_conn)
{
    long ret;

    static char buffer[BUFSIZE + 1];


    ret = zk_server_read(cli_conn, buffer, BUFSIZE);
    if (ret == 0 || ret == -1) {
        (void)close(cli_conn.sockfd);
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
        zk_logger(ZK_LOG_INFO, "Unsuported protocol version (%02x)\n", buffer[i]);
        goto close_routine;
    }

    // authentication method selection
    int nmethods = (int)buffer[++i];

    // looking for implemented authentication method
    bool valid_method = false;
    while (--nmethods >= 0) {
        if (buffer[++i] == ZK_SOCKS_METHOD_NO_AUTH) {
            valid_method = true;
        }
    }

    if (!valid_method) {
        zk_logger(ZK_LOG_ERROR, "No valid authentication method.\n");
        char msg[2] = { 0x05, ZK_SOCKS_METHOD_NO_ACCEPTABLE_METHODS };
        zk_server_write(cli_conn, msg, sizeof(msg));
        goto close_routine;
    }

    // METHOD selection message
    char msg[2] = { 0x05, ZK_SOCKS_METHOD_NO_AUTH };
    zk_server_write(cli_conn, msg, sizeof(msg));

    // 4. Request
    ret = zk_server_read(cli_conn, buffer, BUFSIZE);
    i = 0;

    uint8_t protocol = buffer[0];
    uint8_t command = buffer[1];
    uint8_t rsv = buffer[2];
    uint8_t atyp = buffer[3];

    if (protocol != 0x05) {
        zk_logger(ZK_LOG_INFO, "Unsuported protocol version (%02x)\n", protocol);
        goto close_routine;
    }

    if (rsv != 0x00) {
        zk_logger(ZK_LOG_ERROR, "RSV must be 0x00. (%02x)\n", rsv);
        goto close_routine;
    }

    if (command != ZK_SOCKS_CMD_CONNECT) {
        zk_logger(ZK_LOG_ERROR, "Command not supported. (%02x)\n", command);
        zk_socks_write_reply(cli_conn, ZK_SOCKS_REP_COMMAND_NOT_SUPPORTED, atyp);
        goto close_routine;
    }

    if (atyp != ZK_SOCKS_ATYP_IP_V4) {
        zk_logger(ZK_LOG_ERROR, "Address type not supported. (%02x)\n", atyp);
        zk_socks_write_reply(cli_conn, ZK_SOCKS_REP_ADDRESS_TYPE_NOT_SUPPORTED, atyp);
        goto close_routine;
    }

    uint32_t dst_addr_ipv4 = 0;
    memcpy(&dst_addr_ipv4, ((char*)buffer) + 4, 4);
    dst_addr_ipv4 = ntohl(dst_addr_ipv4);

    uint16_t dst_port = 0;
    memcpy(&dst_port, ((char*)buffer) + (i + 8), 2);
    dst_port = ntohs(dst_port);

    struct in_addr dst_addr;
    dst_addr.s_addr = htonl(dst_addr_ipv4);

    zk_logger(ZK_LOG_INFO, "CONNECT %s:%d\n", inet_ntoa(dst_addr), dst_port);

    // Connect to target host
    zk_server_connection_t target_conn = {};

    target_conn.sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (target_conn.sockfd == -1) {
        zk_logger(ZK_LOG_FATAL, "Socket creation failed...\n");
        zk_socks_write_reply(cli_conn, ZK_SOCKS_REP_SERVER_FAILURE, atyp);
        goto close_routine;
    }

    target_conn.servaddr.sin_family = AF_INET;
    target_conn.servaddr.sin_addr = dst_addr;
    target_conn.servaddr.sin_port = htons(dst_port);

    if (connect(target_conn.sockfd, (struct sockaddr*)&target_conn.servaddr, sizeof(target_conn.servaddr)) != 0) {
        zk_logger(ZK_LOG_FATAL, "Connection with the server failed....\n");
        zk_socks_write_reply(cli_conn, ZK_SOCKS_REP_CONNECTION_REFUSED, atyp);
        goto close_routine;
    }

    // Connection OK
    zk_logger(ZK_LOG_INFO, "%s:%d Connection succesfull!\n", inet_ntoa(dst_addr), dst_port);
    zk_socks_write_reply(cli_conn, ZK_SOCKS_REP_SUCCEEDED, atyp);
    zk_server_socket_pipe(cli_conn, target_conn);

    // Close connection to target
    close(target_conn.sockfd);

close_routine:
    sleep(1);
    close(cli_conn.sockfd);
    exit(EXIT_SUCCESS);
}

int zk_server_read(zk_server_connection_t conn, char buf[], size_t nbyte)
{
    int ret = read(conn.sockfd, buf, nbyte);
    return ret;
}

int zk_server_write(zk_server_connection_t conn, char buf[], size_t nbyte)
{
    int ret = write(conn.sockfd, buf, nbyte);
    return ret;
}

void zk_server_socket_pipe(zk_server_connection_t conn0, zk_server_connection_t conn1)
{
    int maxfd, ret;
    fd_set rd_set;
    size_t nread;
    char buffer_r[BUFSIZE];

    zk_logger(ZK_LOG_INFO, "Connecting two sockets\n");

    maxfd = (conn0.sockfd > conn1.sockfd) ? conn0.sockfd : conn1.sockfd;

    char addr0[INET_ADDRSTRLEN] = {};
    char addr1[INET_ADDRSTRLEN] = {};
    inet_ntop(AF_INET, &conn0.servaddr.sin_addr, addr0, sizeof(addr0));
    inet_ntop(AF_INET, &conn1.servaddr.sin_addr, addr1, sizeof(addr1));

    for (;;) {
        FD_ZERO(&rd_set);
        FD_SET(conn0.sockfd, &rd_set);
        FD_SET(conn1.sockfd, &rd_set);
        ret = select(maxfd + 1, &rd_set, NULL, NULL, NULL);

        if (ret < 0 && errno == EINTR) {
            continue;
        }

        if (FD_ISSET(conn0.sockfd, &rd_set)) {
            nread = zk_server_read(conn0, buffer_r, BUFSIZE);
            if (nread <= 0) {
                break;
            }
            zk_server_write(conn1, buffer_r, nread);
            zk_logger(ZK_LOG_INFO, "%s >> %s - %d bytes.\n", addr0, addr1, nread);
        }

        if (FD_ISSET(conn1.sockfd, &rd_set)) {
            nread = zk_server_read(conn1, buffer_r, BUFSIZE);
            if (nread <= 0) {
                break;
            }
            zk_server_write(conn0, buffer_r, nread);
            zk_logger(ZK_LOG_INFO, "%s << %s - %d bytes.\n", addr0, addr1, nread);
        }
    }
}

int zk_server_start(const unsigned int port)
{
    int listenfd = 0;
    int iEnabled = 1;
    pid_t pid = 0;

    static struct sockaddr_in serv_addr;

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
        zk_server_connection_t cli_conn = {};
        socklen_t len = sizeof(cli_conn.servaddr);

        cli_conn.sockfd = accept(listenfd, (struct sockaddr*)&cli_conn.servaddr, &len);

        if (cli_conn.sockfd < 0) {
            zk_logger(ZK_LOG_FATAL, "Cannot accept incoming connection from %s.\n", inet_ntoa(cli_conn.servaddr.sin_addr));
            (void)close(cli_conn.sockfd);
            return EXIT_FAILURE;
        }

        zk_logger(ZK_LOG_INFO, "Incoming connection from %s.\n", inet_ntoa(cli_conn.servaddr.sin_addr));

        pid = fork();
        if (pid < 0) {
            zk_logger(ZK_LOG_FATAL, "Cannot fork!\n");
            return EXIT_FAILURE;
        }

        if (pid == 0) {
            (void)close(listenfd);
            zk_server_process_request(cli_conn);
        } else {
            (void)close(cli_conn.sockfd);
        }
    }
}
