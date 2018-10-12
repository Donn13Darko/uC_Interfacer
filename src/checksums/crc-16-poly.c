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

#include "crc-16-poly.h"

// Reverse of poly 0x1021
static const uint16_t __crc_16_POLY = 0x8408;
static const uint8_t __crc_16_POLY_LEN = sizeof(uint16_t);

void get_crc_16_POLY(const uint8_t* data_array, uint32_t data_len, uint8_t* crc_start, uint8_t* data_crc)
{
    // Make a copy of the pointer
    const uint8_t *data_p = data_array;
    uint16_t crc = ((((uint16_t) crc_start[0]) << 8) | crc_start[1]);

    // Compute crc
    uint8_t i;
    while (data_len--)
    {
        crc ^= *data_p++;
        for (i = 0; i < 8; i++)
        {
            if (crc & 0x1)
                crc ^= __crc_16_POLY;
            crc >>= 1;
        }
    }

    // Load crc into data array
    data_crc[1] = (uint8_t) (crc & 0xFF);
    data_crc[0] = (uint8_t) ((crc >> 8) & 0xFF);
}

bool check_crc_16_POLY(const uint8_t* data_crc, const uint8_t *cmp_crc)
{
    // Check each byte of the crc array
    return ((data_crc[0] == cmp_crc[0]) && (data_crc[1] == cmp_crc[1]));
}

uint32_t get_crc_16_POLY_size()
{
    return __crc_16_POLY_LEN;
}
