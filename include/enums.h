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

#ifndef ZK_ENUMS_H
#define ZK_ENUMS_H

typedef enum {
    ZK_SOCKS_ATYP_IP_V4 = 0x01,
    ZK_SOCKS_ATYP_DOMAIN_NAME = 0x03,
    ZK_SOCKS_ATYP_IP_V6 = 0x04
} zk_socks_atyp_e;

typedef enum {
    ZK_SOCKS_REP_SUCCEEDED = 0x00,
    ZK_SOCKS_REP_SERVER_FAILURE = 0x01,
    ZK_SOCKS_REP_NOT_ALLOWED_BY_RULESET = 0x02,
    ZK_SOCKS_REP_NETWORK_UNREACHABLE = 0x03,
    ZK_SOCKS_REP_HOST_UNREACHABLE = 0x04,
    ZK_SOCKS_REP_CONNECTION_REFUSED = 0x05,
    ZK_SOCKS_REP_TTL_EXPIRED = 0x06,
    ZK_SOCKS_REP_COMMAND_NOT_SUPPORTED = 0x07,
    ZK_SOCKS_REP_ADDRESS_TYPE_NOT_SUPPORTED = 0x08
} zk_socks_rep_e;

#endif /* ZK_ENUMS_H */
