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

#include "uc-generic-fsm.h"
#include "../communication/crc-calcs.h"

uint8_t *fsm_buffer;

void fsm_start(uint32_t buffer_len)
{
    fsm_buffer = malloc(buffer_len*sizeof(fsm_buffer));
    while (true)
    {
        // Read next major key or break after 5 seconds
        if (fsm_read_bytes(3, 5000)) continue;

        // Parse the major key
        switch(fsm_buffer[0])
        {
            case GUI_TYPE_IO:
                break;
            case GUI_TYPE_DATA_TRANSMIT:
                break;
            case GUI_TYPE_PROGRAMMER:
                break;
            default:
                continue;

        }
    }

    free(fsm_buffer);
}

bool fsm_read_bytes(uint32_t num_bytes, uint32_t timeout)
{
    return ((fsm_read_next(fsm_buffer, num_bytes, timeout) == num_bytes)
                    && (check_crc(fsm_buffer, num_bytes-1, fsm_buffer[num_bytes-1], 0)));
}

uint32_t fsm_read_next(uint8_t* data_array, uint32_t num_bytes, uint32_t timeout)
{
    // Set control variables
    uint32_t check_delay = 10; // ms
    uint32_t wait_time = 0;

    // Wait for num_bytes to be received
    while (fsm_bytes_available() < num_bytes)
    {
        fsm_delay(check_delay);
        wait_time += check_delay;
        if (timeout < wait_time) return 0;
    }

    // Read bytes into array
    for (uint32_t i = 0; i < num_bytes; i++)
    {
        data_array[i] = fsm_getch();
    }
    return num_bytes;
}
