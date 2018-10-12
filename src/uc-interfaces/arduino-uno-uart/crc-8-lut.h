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

#ifndef CRC_8_LUT_H
#define CRC_8_LUT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

// Computes CRC 8 for data_array with start value
void get_crc_8_LUT(const uint8_t* data_array, uint32_t data_len, uint8_t* crc_start, uint8_t* data_crc);

// Checks CRC 8
bool check_crc_8_LUT(const uint8_t* data_crc, const uint8_t* cmp_crc);

// Gets byte length of CRC 8 checksum
uint32_t get_crc_8_LUT_size();

#ifdef __cplusplus
}
#endif

#endif // CRC_8_LUT_H
