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

#include "uc-generic-programmer.h"

uint32_t expected_trans_size = 0;
uint32_t curr_trans_size = 0;

void uc_programmer(uint8_t major_key, uint8_t minor_key, const uint8_t* buffer, uint32_t buffer_len)
{
    // Verify bytes for command or return
    switch (minor_key)
    {
        case MINOR_KEY_PROGRAMMER_SET_TRANS_SIZE:
        case MINOR_KEY_PROGRAMMER_SET_ADDR:
        {
            if (buffer_len != s2_programmer_settings_trans_end) return;
            else break;
        }
        case MINOR_KEY_PROGRAMMER_SET_INFO:
        {
            if (buffer_len != s2_programmer_settings_info_end) return;
            else break;
        }
    }

    // Parse and act on minor key
    switch (minor_key)
    {
        case MINOR_KEY_PROGRAMMER_SET_INFO:
        {
            // Set transmision info
            break;
        }
        case MINOR_KEY_PROGRAMMER_SET_TRANS_SIZE:
        {
            // Set expected size and reset current size
            expected_trans_size = *((uint32_t*) buffer);
            curr_trans_size = 0;
            break;
        }
        case MINOR_KEY_PROGRAMMER_SET_ADDR:
        {
            // Set the current read/write address
            break;
        }
        case MINOR_KEY_PROGRAMMER_DATA:
        {
            // Update current received size
            curr_trans_size += buffer_len;
            break;
        }
        case MINOR_KEY_PROGRAMMER_READ:
        {
            // Read at the current address with the preset method
            break;
        }
    }
}
