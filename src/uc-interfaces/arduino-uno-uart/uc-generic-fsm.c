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

// Recv buffer
uint8_t *fsm_buffer;
uint8_t *fsm_ack_buffer;

// Key & packet holders
uint8_t num_s1_bytes, num_s2_bytes;
uint8_t major_key, curr_packet_stage;
uint8_t num_s1_bytes_ack;
crc_t crc_cmp;

// Buffer pointer holder
uint8_t* fsm_buffer_ptr;

void fsm_setup(uint32_t buffer_len)
{
    // Initialize values
    major_key = MAJOR_KEY_ERROR;
    curr_packet_stage = 1;
    num_s1_bytes = s1_crc_loc;
    num_s1_bytes_ack = num_s1_bytes + crc_size;

    // Malloc buffers
    fsm_buffer = malloc(buffer_len*sizeof(fsm_buffer));
    fsm_ack_buffer = malloc(num_s1_bytes_ack*sizeof(fsm_ack_buffer));

    // Reset to start defaults
    uc_reset();
}

void fsm_destroy()
{
    // Free buffers
    free(fsm_buffer);
    free(fsm_ack_buffer);
}

void fsm_poll()
{
    // Loop forever
    while (true)
    {
        // Reset buffer pointer
        fsm_buffer_ptr = fsm_buffer;

        // Read first stage or loop after 1 second
        if (!fsm_read_next(fsm_buffer_ptr, num_s1_bytes, 1000)) continue;
        fsm_buffer_ptr += num_s1_bytes;

        // Store first stage info
        major_key = fsm_buffer[s1_major_key_loc];
        num_s2_bytes = fsm_buffer[s1_num_s2_bytes_loc];

        // Read Second stage or ACK failed after 1 second
        // Reset uc buffers and return to first stage if failed
        if (!fsm_read_next(fsm_buffer_ptr, num_s2_bytes, 1000))
        {
            fsm_ack(MAJOR_KEY_ERROR);
            uc_reset_buffers();
            continue;
        }
        fsm_buffer_ptr += num_s2_bytes;

        // Read CRC
        if (!fsm_read_next(fsm_buffer_ptr, crc_size, 1000))
        {
            fsm_ack(MAJOR_KEY_ERROR);
            uc_reset_buffers();
            continue;
        }

        // Check CRC
        crc_cmp = build_crc(fsm_buffer_ptr);
        if (!check_crc(fsm_buffer, num_s1_bytes+num_s2_bytes, crc_cmp, 0))
        {
            fsm_ack(MAJOR_KEY_ERROR);
            uc_reset_buffers();
            continue;
        }

        // Send Packet Ack
        fsm_ack(major_key);

        // Run FSM
        fsm_run();
    }
}

void fsm_isr()
{
    // Select which packet stage we are receiving
    if (curr_packet_stage == 1)
    {
        // Only read if enough values present to not block
        if (uc_bytes_available() == num_s1_bytes)
        {
            fsm_buffer_ptr = fsm_buffer;
            // Attempt to read with no timeout
            if (fsm_read_next(fsm_buffer_ptr, num_s1_bytes, 0))
            {
                // Parse info for first stage
                major_key = fsm_buffer[s1_major_key_loc];
                num_s2_bytes = fsm_buffer[s1_num_s2_bytes_loc];

                // If no second stage go to stage 4
                if (num_s2_bytes == 0)
                {
                    // Move to third stage
                    curr_packet_stage = 3;
                } else
                {
                    // Move to second stage
                    curr_packet_stage = 2;
                }
            }
        }
    } else if (curr_packet_stage == 2)
    {
        // Only read if enough values present to not block
        if (uc_bytes_available() == num_s2_bytes)
        {
            fsm_buffer_ptr += num_s1_bytes;
            if (fsm_read_next(fsm_buffer_ptr, num_s2_bytes, 0))
            {
                // Move to third stage
                curr_packet_stage = 3;
            }
        }
    } else if (curr_packet_stage == 3)
    {
        // Only read if enough values present to not block
        if (uc_bytes_available() == crc_size)
        {
            fsm_buffer_ptr += num_s2_bytes;
            if (fsm_read_next(fsm_buffer_ptr, crc_size, 1000))
            {
                // Check CRC
                crc_cmp = build_crc(fsm_buffer_ptr);
                if (!check_crc(fsm_buffer, num_s1_bytes+num_s2_bytes, crc_cmp, 0))
                {
                    fsm_ack(MAJOR_KEY_ERROR);
                    uc_reset_buffers();
                    return;
                }

                // Send Packet Ack
                fsm_ack(major_key);

                // Run FSM
                fsm_run();

                // Move to first stage
                curr_packet_stage = 1;
            }
        }
    } else
    {
        // Encountered error, reset everything
        curr_packet_stage = 1;
        uc_reset_buffers();
    }
}

void fsm_run()
{
    // Remove fsm major buffer info before passing on
    fsm_buffer_ptr = fsm_buffer+num_s1_bytes;

    // Parse and act on major key
    switch (major_key)
    {
        case MAJOR_KEY_RESET:
            uc_reset();
            break;
        case GUI_TYPE_IO:
            uc_io(fsm_buffer_ptr, num_s2_bytes);
            break;
        case GUI_TYPE_DATA_TRANSMIT:
            uc_data_transmit(fsm_buffer_ptr, num_s2_bytes);
            break;
        case GUI_TYPE_PROGRAMMER:
            uc_programmer(fsm_buffer_ptr, num_s2_bytes);
            break;
        default:
            return;
    }
}

void fsm_ack(uint8_t ack_key)
{
    // Fill buffer
    fsm_ack_buffer[s1_major_key_loc] = MAJOR_KEY_ACK;
    fsm_ack_buffer[s1_num_s2_bytes_loc] = ack_key;

    // Get and add crc to buffer
    crc_t crc = get_crc(fsm_ack_buffer, s1_crc_loc, 0);
    build_byte_array(crc, fsm_ack_buffer+s1_crc_loc);

    // Send buffer
    uc_send(fsm_ack_buffer, num_s1_bytes_ack);
}

bool fsm_read_next(uint8_t* data_array, uint32_t num_bytes, uint32_t timeout)
{
    // Set control variables
    uint32_t check_delay = 10; // ms
    uint32_t wait_time = 0;

    // Wait for num_bytes to be received
    while (uc_bytes_available() < num_bytes)
    {
        uc_delay(check_delay);
        wait_time += check_delay;
        if (timeout < wait_time) return false;
    }

    // Read bytes into array
    for (uint32_t i = 0; i < num_bytes; i++)
    {
        data_array[i] = uc_getch();
    }
    return true;
}
