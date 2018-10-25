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

void uc_io(uint8_t minor_key, const uint8_t* buffer, uint8_t buffer_len)
{
    // Verify & parse bytes
    uint16_t value = 0;
    if (buffer_len != s2_io_end_loc)
    {
        // If not enough bytes for command return
        if ((minor_key == MINOR_KEY_IO_DIO_SET)
                || (minor_key == MINOR_KEY_IO_AIO_SET))
        {
            return;
        }
    } else
    {
        value = (buffer[s2_io_value_high_loc] << 8)
                          | buffer[s2_io_value_low_loc];
    }

    // Parse and act on minor key
    switch (minor_key)
    {
        case MINOR_KEY_IO_DIO_SET:
            uc_dio(buffer[s2_io_pin_num_loc], buffer[s2_io_combo_loc], value);
            break;
        case MINOR_KEY_IO_AIO_SET:
            uc_aio(buffer[s2_io_pin_num_loc], buffer[s2_io_combo_loc], value);
            break;
        case MINOR_KEY_IO_DIO_READ:
            uc_dio_read();
            break;
        case MINOR_KEY_IO_AIO_READ:
            uc_aio_read();
            break;
        case MINOR_KEY_IO_REMOTE_CONN:
            uc_remote_conn();
            break;
        default:
            return;

    }
}
