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

#ifndef ZK_SERVER_H
#define ZK_SERVER_H

void zk_server_process_request(int socket_fd);
int zk_server_read(int fd, unsigned char buf[], size_t nbyte);
int zk_server_write(int fd, unsigned char buf[], size_t nbyte);
int zk_server_start(const unsigned int port);

#endif /* ZK_SERVER_H */
