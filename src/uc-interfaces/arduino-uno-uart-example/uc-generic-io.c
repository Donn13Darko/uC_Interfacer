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

void uc_io(uint8_t major_key, uint8_t minor_key, const uint8_t* buffer, uint8_t buffer_len)
{
    // If not enough bytes for command return
    if (((minor_key == MINOR_KEY_IO_DIO_SET) || (minor_key == MINOR_KEY_IO_AIO_SET))
            && (buffer_len != s2_io_end_loc))
        return;
    else if (((minor_key == MINOR_KEY_IO_DIO_READ_PIN) || (minor_key == MINOR_KEY_IO_AIO_READ_PIN))
            && (buffer_len != 1))
        return;

    // Parse and act on minor key
    switch (minor_key)
    {
        case MINOR_KEY_IO_DIO_SET:
        {
            uint16_t value = ((uint16_t) buffer[s2_io_value_high_loc] << 8) | buffer[s2_io_value_low_loc];
            uc_dio_set(buffer[s2_io_pin_num_loc], buffer[s2_io_combo_loc], value);
            break;
        }
        case MINOR_KEY_IO_AIO_SET:
        {
            uint16_t value = ((uint16_t) buffer[s2_io_value_high_loc] << 8) | buffer[s2_io_value_low_loc];
            uc_aio_set(buffer[s2_io_pin_num_loc], buffer[s2_io_combo_loc], value);
            break;
        }
        case MINOR_KEY_IO_DIO_READ_PIN:
        {
            uint16_t read_data = uc_dio_read(buffer[s2_io_pin_num_loc]);
            fsm_send(major_key, minor_key, (const uint8_t*) &read_data, 2);
            break;
        }
        case MINOR_KEY_IO_AIO_READ_PIN:
        {
            uint16_t read_data = uc_aio_read(buffer[s2_io_pin_num_loc]);
            fsm_send(major_key, minor_key, (const uint8_t*) &read_data, 2);
            break;
        }
        case MINOR_KEY_IO_DIO_READ_ALL:
        {
            uint16_t* read_data = uc_dio_read_all();
            fsm_send(major_key, minor_key, (const uint8_t*) read_data, uc_dio_num_pins << 1);
            break;
        }
        case MINOR_KEY_IO_AIO_READ_ALL:
        {
            uint16_t* read_data = uc_aio_read_all();
            fsm_send(major_key, minor_key, (const uint8_t*) read_data, uc_aio_num_pins << 1);
            break;
        }
        case MINOR_KEY_IO_REMOTE_CONN:
        {
            uc_remote_conn();
            break;
        }
        default:
        {
            break;
        }

    }
}
