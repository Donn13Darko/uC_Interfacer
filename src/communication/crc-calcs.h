/*
 * uC Interface - A GUI for Programming & Interfacing with Microcontrollers
 * Copyright (C) 2018  Mitchell Oleson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CRC_CALCS_H
#define CRC_CALCS_H

#include <cstdint>

#ifdef __cplusplus
extern "C"
{
#endif

#if defined __crc_8
typedef uint8_t crc_t;

#elif defined __crc_16
typedef uint16_t crc_t;

#elif defined __crc_32
typedef uint32_t crc_t;

#elif !defined __crc_cust
// Default to CRC8 with CPU solution
#define __crc_8
typedef uint8_t crc_t;

#endif

static crc_t get_crc(const uint8_t* data_array, crc_t data_len, crc_t crc = 0);
static bool check_crc(const uint8_t* data_array, crc_t data_len, crc_t crc_cmp, crc_t crc = 0);

#ifdef __cplusplus
}
#endif

#endif // CRC_CALCS_H
