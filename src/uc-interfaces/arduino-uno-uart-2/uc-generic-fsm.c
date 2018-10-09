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
#include "crc-calcs.h"
#include "general-comms.h"

uint8_t *fsm_buffer;

/* Packet #1 (p1) generic key positions */
typedef enum {
    p1_major_key_loc = 0,
    p1_num_p2_bytes_loc,
    p1_crc_loc
} P1_Major_Settings;

void fsm_start(uint32_t buffer_len)
{
    // Setup key and byte holders
    uint8_t major_key, num_p2_bytes;

    // Malloc recv buffer
    fsm_buffer = malloc(buffer_len*sizeof(fsm_buffer));

    // Loop forever
    while (true)
    {
        // Read Packet #1 or break after 5 seconds
        if (!fsm_read_bytes(p1_crc_loc+1, 5000)) continue;

        // Send Packet #1 Ack (add)

        // Store Packet #1 info
        major_key = fsm_buffer[p1_major_key_loc];
        num_p2_bytes = fsm_buffer[p1_num_p2_bytes_loc];

        // Read Packet #2 only if being sending
        if (num_p2_bytes != 0)
        {
            // Read Packet #2 or break after 5 seconds
            if (!fsm_read_bytes(num_p2_bytes, 5000))
            {
                // Add ack for CMD fail

                // Continue on to next CMD
                continue;
            }
        }

        // Send Packet #2 Ack (add)

        // Ignore Packet #2 crc (already verified)
        num_p2_bytes -= 1;

        // Parse and act on major key
        switch (major_key)
        {
            case MAJOR_KEY_RESET:
                uc_reset();
                break;
            case GUI_TYPE_IO:
                uc_io(fsm_buffer, num_p2_bytes);
                break;
            case GUI_TYPE_DATA_TRANSMIT:
                uc_data_transmit(fsm_buffer, num_p2_bytes);
                break;
            case GUI_TYPE_PROGRAMMER:
                uc_programmer(fsm_buffer, num_p2_bytes);
                break;
            default:
                continue;

        }
    }

    free(fsm_buffer);
}

bool fsm_read_bytes(uint32_t num_bytes, uint32_t timeout)
{
    if (fsm_read_next(fsm_buffer, num_bytes, timeout) == num_bytes)
    {
        if (check_crc(fsm_buffer, num_bytes-1, fsm_buffer[num_bytes-1], 0))
        {
            return true;
        } else
        {
            // Add checksum fail ack

            // Reset uc to prevent buffer errors
            uc_reset();
        }
    }
    return false;
}

uint32_t fsm_read_next(uint8_t* data_array, uint32_t num_bytes, uint32_t timeout)
{
    // Set control variables
    uint32_t check_delay = 100; // ms
    uint32_t wait_time = 0;

    // Wait for num_bytes to be received
    while (uc_bytes_available() < num_bytes)
    {
        uc_delay(check_delay);
        wait_time += check_delay;
        if (timeout < wait_time) return 0;
    }

    // Read bytes into array
    for (uint32_t i = 0; i < num_bytes; i++)
    {
        data_array[i] = uc_getch();
    }
    return num_bytes;
}
