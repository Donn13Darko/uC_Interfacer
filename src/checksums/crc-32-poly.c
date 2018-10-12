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

#include "crc-32-poly.h"

// Reverse of poly 0x1EDC6F41
static const uint32_t __crc_32_POLY = 0x105EC76F;
static const uint8_t __crc_32_POLY_LEN = sizeof(uint32_t);

void get_crc_32_POLY(const uint8_t* data_array, uint32_t data_len, uint8_t* crc_start, uint8_t* data_crc)
{
    // Make a copy of the pointer
    const uint8_t *data_p = data_array;
    
    // Create the start crc number
    uint8_t i;
    uint32_t crc = 0;
    for (i = 0; i < __crc_32_POLY_LEN; i++)
    {
        crc = ((crc << 8) | crc_start[i]);
    }

    // Compute crc
    while (data_len--)
    {
        crc ^= *data_p++;
        for (i = 0; i < 8; i++)
        {
            if (crc & 0x1)
                crc ^= __crc_32_POLY;
            crc >>= 1;
        }
    }

    // Load crc into data array
    i = __crc_32_POLY_LEN - 1;
    do
    {
        data_crc[i] = (uint8_t) (crc & 0xFF);
        crc = crc >> 8;
    } while (0 < i--);
}

bool check_crc_32_POLY(const uint8_t* data_crc, const uint8_t* cmp_crc)
{
    // Check each byte of the crc array
    for (uint8_t i = 0; i < __crc_32_POLY_LEN; i++)
    {
        if (data_crc[i] != cmp_crc[i])
        {
            return false;
        }
    }
    return true;
}

uint32_t get_crc_32_POLY_size()
{
    return __crc_32_POLY_LEN;
}
