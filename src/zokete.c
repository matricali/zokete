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

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "logger.h"
#include "server.h"

static unsigned int port = 1080;

void zk_print_banner()
{
    puts("zoketed v0.1 - A simple SOCKS5 Server (https://github.com/matricali/zokete)");
}

void zk_usage(const char* p)
{
    printf("usage: %s [-hv] [--help] [--version] [-p port] [--port=port]\n", p);
}

int main(int argc, char** argv)
{
    int opt;
    int option_index = 0;

    static struct option long_options[] = {
        { "version", no_argument, 0, 'v' },
        { "help", no_argument, 0, 'h' },
        { "port", required_argument, 0, 'p' },
        { 0, 0, 0, 0 }
    };

    while ((opt = getopt_long(argc, argv, "vhp:", long_options, &option_index)) != -1) {
        switch (opt) {
        case 'v':
            zk_print_banner();
            exit(EXIT_SUCCESS);
            break;

        case 'h':
            zk_print_banner();
            puts("");
            zk_usage(argv[0]);
            exit(EXIT_SUCCESS);
            break;

        case 'p':
            port = atoi(optarg);
            break;

        case '?':
            /* getopt_long already printed an error message. */
            exit(EXIT_FAILURE);
            break;

        default:
            abort();
        }
    }

    zk_server_start(port);
    exit(EXIT_SUCCESS);
}
