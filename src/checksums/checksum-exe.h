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

#ifndef CHECKSUM_EXE_H
#define CHECKSUM_EXE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * Instructions: Uses command line arguments to
 * retrieve the checksum.
 *
 * 1) ./checksum.exe -get_checksum data data_len checksum_start
 *  The exe must print the checksum to stdout
 * 2) ./checksum.exe -check_checksum data_checksum cmp_checksum
 *  The exe must print 0 for false or 1 for true to stdout
 * 3) ./checksum.exe -get_checksum_size
 *  The exe must print the size as 4 bytes to stdout
 *
*/

// Sets executable path for next calls
void set_executable_checksum_exe(const char* new_exe_path);

// Computes checksum for data_array with start value
void get_checksum_exe(const uint8_t* data_array, uint32_t data_len, const uint8_t* checksum_start, uint8_t* data_checksum);

// Checks checksum
bool check_checksum_exe(const uint8_t* data_checksum, const uint8_t* cmp_checksum);

// Gets byte length of checksum
uint32_t get_checksum_exe_size();

#ifdef __cplusplus
}
#endif

#endif // CHECKSUM_EXE_H
