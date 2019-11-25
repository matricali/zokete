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

#ifndef ZK_LOGGER_H
#define ZK_LOGGER_H

enum zk_log_level {
    ZK_LOG_NONE,
    ZK_LOG_FATAL,
    ZK_LOG_ERROR,
    ZK_LOG_WARNING,
    ZK_LOG_NOTICE,
    ZK_LOG_INFO,
    ZK_LOG_DEBUG,
    ZK_LOG_NEVER
};

void zk_logger(enum zk_log_level level, const char* format, ...);

#endif /* ZK_LOGGER_H */
