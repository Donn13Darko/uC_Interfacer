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

// Recv buffer
uint8_t *fsm_buffer;
uint8_t *fsm_ack_buffer;

// Key & packet holders
uint8_t major_key, curr_packet_num, num_p2_bytes;

void fsm_setup(uint32_t buffer_len)
{
    // Initialize values
    major_key = MAJOR_KEY_ERROR;
    curr_packet_num = 1;

    // Malloc buffers
    fsm_buffer = malloc(buffer_len*sizeof(fsm_buffer));
    fsm_ack_buffer = malloc(num_p1_bytes*sizeof(fsm_ack_buffer));

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
    // Useful variables
    bool good_read;
    uint16_t loop_count;

    // Loop forever
    while (true)
    {
        // Read Packet #1 or loop after 5 seconds
        if (!fsm_read_bytes(num_p1_bytes, 5000)) continue;

        // Store Packet #1 info
        major_key = fsm_buffer[p1_major_key_loc];
        num_p2_bytes = fsm_buffer[p1_num_p2_bytes_loc];

        // Send Packet #1 Ack
        fsm_ack(major_key);

        // Read Packet #2 only if its being sent
        if (num_p2_bytes != 0)
        {
            // Read Packet #2 or ACK failed after 1 second
            // Exit after retries attempts
            do
            {
                // Try reading
                good_read = fsm_read_bytes(num_p2_bytes, 1000);

                // Check read, increment loop count/check if time to break
                if (good_read || (++loop_count == packet_retries)) break;
            } while (!good_read);

            // Reset to Packet #1 if still failing
            if (!good_read) continue;

            // Send Packet #2 Ack (minor_key = 0)
            fsm_ack(fsm_buffer[0]);

            // Ignore Packet #2 crc (already verified)
            num_p2_bytes -= 1;
        }

        // Run FSM
        fsm_run();
    }
}

void fsm_isr()
{
    // Select which packet we are expecting
    if (curr_packet_num == 1)
    {
        // Only read if enough values present to not block
        if (uc_bytes_available() == num_p1_bytes)
        {
            // Attempt to read with no timeout
            if (fsm_read_bytes(num_p1_bytes, 0))
            {
                // Parse info for Packet #1
                major_key = fsm_buffer[p1_major_key_loc];
                num_p2_bytes = fsm_buffer[p1_num_p2_bytes_loc];

                // Send Packet #1 Ack
                fsm_ack(major_key);

                // Run FSM if no Packet #2
                if (num_p2_bytes == 0)
                {
                    // Run FSM
                    fsm_run();

                    // Reset to Packet #1
                    curr_packet_num = 1;
                } else
                {
                    // Move to Packet #2
                    curr_packet_num = 2;
                }
            }
        }
    } else if (curr_packet_num == 2)
    {
        // Only read if enough values present to not block
        if (uc_bytes_available() == num_p2_bytes)
        {
            if (fsm_read_bytes(num_p2_bytes, 0))
            {
                // Send Packet #2 Ack (minor_key = 0)
                fsm_ack(fsm_buffer[0]);

                // Ignore Packet #2 crc (already verified)
                num_p2_bytes -= 1;

                // Run FSM
                fsm_run();

                // Reset to Packet #1
                curr_packet_num = 1;
            }
        }
    } else
    {
        // Encountered error, reset everything
        curr_packet_num = 1;
        uc_reset();
    }
}

void fsm_run()
{
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
            return;
    }
}

void fsm_ack(uint8_t ack_key)
{
    // Fill buffer
    fsm_ack_buffer[p1_major_key_loc] = MAJOR_KEY_ACK;
    fsm_ack_buffer[p1_num_p2_bytes_loc] = ack_key;
    fsm_ack_buffer[p1_crc_loc] = get_crc(fsm_ack_buffer, p1_crc_loc, 0);

    // Send buffer
    uc_send(fsm_ack_buffer, num_p1_bytes);
}

bool fsm_read_bytes(uint32_t num_bytes, uint32_t timeout)
{
    uint32_t num_bytes_m1 = num_bytes - 1;
    if (fsm_read_next(fsm_buffer, num_bytes, timeout) == num_bytes)
    {
        if (check_crc(fsm_buffer, num_bytes_m1, fsm_buffer[num_bytes_m1], 0))
        {
            return true;
        } else
        {
            // Checksum Failed Ack
            fsm_ack(MAJOR_KEY_ERROR);

            // Reset send/receive buffers to prevent sync errors
            uc_reset_buffers();
        }
    }
    return false;
}

uint32_t fsm_read_next(uint8_t* data_array, uint32_t num_bytes, uint32_t timeout)
{
    // Set control variables
    uint32_t check_delay = 10; // ms
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
