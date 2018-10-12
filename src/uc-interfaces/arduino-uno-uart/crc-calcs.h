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

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

/*
 * CRC type defines: __crc_8, __crc_16, __crc_32, or __crc_cust
 * For CRC Lookup table add "#define __crc_LUT"
 * If __crc_cust defined, must do the following
 * (see crc-calcs.cpp/.h for example definitions):
 *   1) typdef _____ crc_t to whatever the variable type is
 *   2) Provide one of the following:
 *     a) LUT & max address if using __crc_LUT:
 *       // Used to & the generated LUT address
 *       // Prevents out of bounds errors
 *       _____ __crc_LUT_MAX = 0x...;
 *       -and-
 *       // LUT with size defined by __crc_LUT_MAX+1
 *       static const _____ crc_table[...] = {....};
 *     b) Otherwise provide poly (Reverse=True):
 *       static const _____ crc_poly = 0x....;
 *
*/
#define __crc_32
#define __crc_LUT

#if defined(__crc_8)
typedef uint8_t crc_t;

#elif defined(__crc_16)
typedef uint16_t crc_t;

#elif defined(__crc_32)
typedef uint32_t crc_t;

#elif defined(__crc_cust)
extern uint8_t crc_table[];
// Uncomment & add correct typedef here
//typedef _____ crc_t;

#else
// Default to CRC8 with CPU solution
#define __crc_8
typedef uint8_t crc_t;

#endif

#if !defined __crc_cust
// Used to & the generated LUT address
// Prevents out of bounds errors
// Using a 256 value table (uint8_t max)
static const uint8_t __crc_LUT_MAX = 0xFF;
#endif

// Gets CRC for data_array with start value crc
crc_t get_crc(const uint8_t* data_array, uint32_t data_len, crc_t crc_start);

// Size constant
static const uint8_t crc_size = sizeof(crc_t);

// CRC helpers
void build_byte_array(crc_t crc, uint8_t* data_array);
crc_t build_crc(const uint8_t* data_array);

// CRC check alg
bool check_crc(const uint8_t* data_array, uint32_t data_len, crc_t crc_cmp, crc_t crc_start);

#ifdef __cplusplus
}
#endif

#endif // CRC_CALCS_H
