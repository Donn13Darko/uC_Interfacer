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

#include "uc-generic-data-transmit.h"

uint32_t data_transmit_expected_trans_size = 0;
uint32_t data_transmit_curr_trans_size = 0;

void uc_data_transmit(uint8_t major_key, uint8_t minor_key, const uint8_t* buffer, uint32_t buffer_len)
{
    // Verify bytes for command or return
    switch (minor_key)
    {
        case MINOR_KEY_DATA_TRANSMIT_SET_TRANS_SIZE:
        {
            if (buffer_len != s2_data_transmit_settings_trans_end) return;
            else break;
        }
    }

    // Parse and act on minor key
    switch (minor_key)
    {
        case MINOR_KEY_DATA_TRANSMIT_SET_TRANS_SIZE:
        {
            // Set expected size and reset current size
            data_transmit_expected_trans_size = *((uint32_t*) buffer);
            data_transmit_curr_trans_size = 0;
            break;
        }
        case MINOR_KEY_DATA_TRANSMIT_DATA:
        {
            // Update current received size
            data_transmit_curr_trans_size += buffer_len;

            // Send data to be handled
            uc_data_handle(buffer, buffer_len);
            break;
        }
    }
}
