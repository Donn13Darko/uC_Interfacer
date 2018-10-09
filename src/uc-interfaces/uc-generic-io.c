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

#include "uc-generic-io.h"

/* Packet #2 (p2) io key positions enum */
typedef enum {
    p2_sub_key_loc = 0,
    p2_pin_num_loc,
    p2_combo_loc,
    p2_value_high_loc,
    p2_value_low_loc
} P2_IO_Settings;

void uc_io(const uint8_t* buffer, uint8_t num_bytes)
{
    // Make sure we have at least one byte for sub_key
    if (num_bytes == 0) return;
    uint8_t sub_key = buffer[p2_sub_key_loc];

    // Verify & parse bytes
    uint16_t value = 0;
    if (num_bytes != (p2_value_low_loc+1))
    {
        // If not enough bytes for command return
        if ((sub_key == SUB_KEY_IO_DIO)
                || (sub_key == SUB_KEY_IO_AIO))
        {
            return;
        }
    } else
    {
        value = ((((uint16_t) buffer[p2_value_high_loc]) << 8)
                          | ((uint16_t) buffer[p2_value_low_loc]));
    }

    // Parse and act on sub key
    switch (sub_key)
    {
        case SUB_KEY_IO_DIO:
            uc_dio(buffer[p2_pin_num_loc], buffer[p2_combo_loc], value);
            break;
        case SUB_KEY_IO_AIO:
            uc_aio(buffer[p2_pin_num_loc], buffer[p2_combo_loc], value);
            break;
        case SUB_KEY_IO_DIO_READ:
            uc_dio_read();
            break;
        case SUB_KEY_IO_AIO_READ:
            uc_aio_read();
            break;
        case SUB_KEY_IO_REMOTE_CONN:
            uc_remote_conn();
            break;
        default:
            return;

    }
}
