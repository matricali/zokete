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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "logger.h"

FILE* logfile = NULL;

void zk_logger(enum zk_log_level level, const char* format, ...)
{
    if (level == ZK_LOG_NEVER || level == ZK_LOG_NONE) {
        return;
    }

    va_list arg;
    char message[1024];
    char datetime[26];

    time_t timer;
    struct tm* tm_info;

    if (logfile == NULL) {
        logfile = fopen("zoketed.log", "a");
        if (logfile < 0) {
            zk_logger(ZK_LOG_ERROR, "Error opening log file.\n");
        }
    }

    time(&timer);
    tm_info = localtime(&timer);
    strftime(datetime, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    memset(message, 0, 1024);

    va_start(arg, format);
    vsprintf(message, format, arg);
    va_end(arg);

    if (logfile > 0) {
        fprintf(logfile, "[%s][%d] %s", datetime, getpid(), message);
    }

    if (level < ZK_LOG_WARNING) {
        fprintf(stderr, "[%s][%d] %s", datetime, getpid(), message);
    } else if (level <= ZK_LOG_DEBUG) {
        printf("[%s][%d] %s", datetime, getpid(), message);
    }
}
