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

#include <stdio.h>
#include <stdlib.h>

#include "logger.h"
#include "server.h"

void zk_print_banner()
{
    printf("zoketed v0.1 - Simple SOCKS5 Serverf (https://github.com/matricali/zokete)\n");
}

void zk_usage(const char* p)
{
    printf("usage: %s \n", p);
}

int main(int argc, char** argv)
{
    zk_logger(ZK_LOG_INFO, "Starting coso\n");
    zk_server_start(1080);
    exit(EXIT_SUCCESS);
}
