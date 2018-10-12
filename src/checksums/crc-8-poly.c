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

#include "crc-8-poly.h"

// Reverse of poly 0x07
static const uint8_t __crc_8_POLY = 0xE0;
static const uint8_t __crc_8_POLY_LEN = sizeof(uint8_t);

void get_crc_8_POLY(const uint8_t* data_array, uint32_t data_len, uint8_t* crc_start, uint8_t* data_crc)
{
    // Make a copy of the pointer
    const uint8_t *data_p = data_array;
    uint8_t crc = crc_start[0];

    // Compute crc
    uint8_t i;
    while (data_len--)
    {
        crc ^= *data_p++;
        for (i = 0; i < 8; i++)
        {
            if (crc & 0x1)
                crc ^= __crc_8_POLY;
            crc >>= 1;
        }
    }

    // Load crc into data array
    data_crc[0] = (uint8_t) (crc & 0xFF);
}

bool check_crc_8_POLY(const uint8_t* data_crc, const uint8_t *cmp_crc)
{
    // Check each byte of the crc array
    return (data_crc[0] == cmp_crc[0]);
}

uint32_t get_crc_8_POLY_size()
{
    return __crc_8_POLY_LEN;
}
