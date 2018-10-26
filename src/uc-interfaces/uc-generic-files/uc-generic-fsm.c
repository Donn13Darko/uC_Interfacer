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

/*
 * Replace the following section with the desired checksum.
 * This will need to be input as part of the INI file under each
 * GUI section (gets broken apart based on the GUI tab type).
 * Multiple instances of the same tab type must have the same checksum.
 * Default checksum is CRC-8 POLY.
*/
#include "../checksums/crc-8-lut.h"

// GUI checksums
checksum_struct io_checksum = DEFAULT_CHECKSUM_STRUCT;
checksum_struct data_transfer_checksum = DEFAULT_CHECKSUM_STRUCT;
checksum_struct programmer_checksum = DEFAULT_CHECKSUM_STRUCT;
checksum_struct custom_cmd_checksum = DEFAULT_CHECKSUM_STRUCT;

// Default checksum (for acks, errors, and resets);
checksum_struct default_checksum = DEFAULT_CHECKSUM_STRUCT;

// Recv buffers
uint8_t *fsm_buffer;
uint8_t *fsm_checksum_buffer;
uint8_t *fsm_checksum_cmp_buffer;
uint8_t *fsm_ack_buffer;
uint8_t *fsm_send_buffer;

uint32_t fsm_buffer_len;
uint32_t fsm_send_buffer_len;

// Key & packet holders
uint32_t num_s2_bytes;
uint8_t major_key, minor_key, num_s2_bytes_len;
uint32_t curr_packet_stage, checksum_max_size;

// Buffer pointer holder
uint8_t* fsm_buffer_ptr;

// Function prototypes (local access only)
void fsm_ack(uint8_t ack_key);
bool fsm_read_next(uint8_t* data_array, uint32_t num_bytes, uint32_t timeout);
bool fsm_check_checksum(uint8_t* data, uint32_t data_len, uint8_t* checksum_cmp);
checksum_struct* fsm_get_checksum_struct(uint8_t gui_key);

void fsm_setup(uint32_t buffer_len)
{
    // Initialize variables
    major_key = MAJOR_KEY_ERROR; // All errors are 0
    minor_key = MAJOR_KEY_ERROR; // All errors are 0
    curr_packet_stage = 1;

    // Enfore a minimum length of 10 (min required for basic io control)
    if (buffer_len < 10) fsm_buffer_len = 10;
    else fsm_buffer_len = buffer_len;
    fsm_send_buffer_len = buffer_len;

    // Select largest checksum for buffer size
    checksum_max_size = default_checksum.get_checksum_size();
    if (checksum_max_size < io_checksum.get_checksum_size())
        checksum_max_size = io_checksum.get_checksum_size();
    if (checksum_max_size < data_transfer_checksum.get_checksum_size())
        checksum_max_size = data_transfer_checksum.get_checksum_size();
    if (checksum_max_size < programmer_checksum.get_checksum_size())
        checksum_max_size = programmer_checksum.get_checksum_size();

    // Malloc initial buffers
    fsm_buffer = malloc(fsm_buffer_len*sizeof(fsm_buffer));
    fsm_ack_buffer = malloc(num_s1_bytes*sizeof(fsm_ack_buffer));
    fsm_send_buffer = malloc(fsm_send_buffer_len*sizeof(fsm_ack_buffer));
    fsm_checksum_buffer = malloc(checksum_max_size*sizeof(fsm_checksum_buffer));
    fsm_checksum_cmp_buffer = malloc(checksum_max_size*sizeof(fsm_checksum_cmp_buffer));

    // Reset to start defaults
    uc_reset();
}

void fsm_destroy()
{
    // Free buffers
    free(fsm_buffer);
    free(fsm_ack_buffer);
    free(fsm_checksum_buffer);
    free(fsm_checksum_cmp_buffer);
}

void fsm_poll()
{
    // Loop forever
    while (true)
    {
        // Reset buffer pointer
        fsm_buffer_ptr = fsm_buffer;

        // Read first stage or loop after 1 second
        if (!fsm_read_next(fsm_buffer_ptr, num_s1_bytes, packet_timeout)) continue;
        fsm_buffer_ptr += num_s1_bytes;

        // Store first stage info
        major_key = fsm_buffer[s1_major_key_loc];
        minor_key = fsm_buffer[s1_minor_key_loc];

        // Modify Major key for encoding
        num_s2_bytes_len = major_key & s1_num_s2_bytes_bit_mask;
        major_key &= s1_major_key_bit_mask;

        // Handle case of 3 (want uint32_t not uint24_t)
        if (num_s2_bytes_len == 0x03) num_s2_bytes_len += 1;

        // Read number of bytes in second stage or ACK failed after packet_timeout ms
        // Reset uc buffers and return to first stage if failed
        if (!fsm_read_next(fsm_buffer_ptr, num_s2_bytes_len, packet_timeout))
        {
            fsm_ack(MAJOR_KEY_ERROR);
            uc_reset_buffers();
            continue;
        }
        
        // Parse num_s2_bytes
        switch (num_s2_bytes_len)
        {
            case 0x01:
                // 1 byte
                num_s2_bytes = (uint8_t) *((uint8_t*) fsm_buffer_ptr);
                break;
            case 0x02:
                // 2 bytes
                num_s2_bytes = (uint16_t) *((uint16_t*) fsm_buffer_ptr);
                break;
            case 0x04:
                num_s2_bytes = *((uint32_t*) fsm_buffer_ptr);
                break;
            default:
                num_s2_bytes = 0;
                break;
        }
        fsm_buffer_ptr += num_s2_bytes_len;

        // Read Second stage or ACK failed after packet_timeout ms
        // Reset uc buffers and return to first stage if failed
        if (!fsm_read_next(fsm_buffer_ptr, num_s2_bytes, packet_timeout))
        {
            fsm_ack(MAJOR_KEY_ERROR);
            uc_reset_buffers();
            continue;
        }

        // Read Checksum
        uint32_t checksum_size = fsm_get_checksum_struct(major_key)->get_checksum_size();
        if (!fsm_read_next(fsm_checksum_buffer, checksum_size, packet_timeout))
        {
            fsm_ack(MAJOR_KEY_ERROR);
            uc_reset_buffers();
            continue;
        }

        // Ignore ack and continue (should never get an ack here)
        if (major_key == MAJOR_KEY_ACK) continue;

        // Check Checksum
        if (!fsm_check_checksum(fsm_buffer, num_s1_bytes+num_s2_bytes_len+num_s2_bytes, fsm_checksum_buffer))
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

bool fsm_isr()
{
    // Select which packet stage we are receiving
    if (curr_packet_stage == 1)
    {
        // Only read if enough values present to not block
        if (uc_bytes_available() == num_s1_bytes)
        {
            // Reset fsm_buffer_ptr for first read
            fsm_buffer_ptr = fsm_buffer;

            // Read with 0 timeout
            if (fsm_read_next(fsm_buffer_ptr, num_s1_bytes, 0))
            {
                // Increment buffer pointer
                fsm_buffer_ptr += num_s1_bytes;

                // Parse info for first stage
                major_key = fsm_buffer[s1_major_key_loc];
                minor_key = fsm_buffer[s1_minor_key_loc];
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
            // Read second stage with 0 timeout
            if (fsm_read_next(fsm_buffer_ptr, num_s2_bytes, 0))
            {
                // Move to third stage
                curr_packet_stage = 3;
            }
        }
    } else if (curr_packet_stage == 3)
    {
        // Only read if enough values present to not block
        uint32_t checksum_size = fsm_get_checksum_struct(major_key)->get_checksum_size();
        if (uc_bytes_available() == checksum_size)
        {
            // Read Checksum with 0 timeout
            if (fsm_read_next(fsm_checksum_buffer, checksum_size, 0))
            {
                // Check Checksum
                if (!fsm_check_checksum(fsm_buffer, num_s1_bytes+num_s2_bytes, fsm_checksum_buffer))
                {
                    fsm_ack(MAJOR_KEY_ERROR);
                    uc_reset_buffers();
                    return false;
                }

                // Send Packet Ack
                fsm_ack(major_key);

                // Return to first stage for next call
                curr_packet_stage = 1;

                // Ready for fsm call
                return true;
            }
        }
    } else
    {
        // Encountered error, reset everything
        curr_packet_stage = 1;
        uc_reset_buffers();
    }

    // Not ready for fsm call
    return false;
}

void fsm_run()
{
    // Parse and act on major key
    switch (major_key)
    {
        case MAJOR_KEY_IO:
            uc_io(minor_key, fsm_buffer_ptr, num_s2_bytes);
            break;
        case MAJOR_KEY_DATA_TRANSMIT:
            uc_data_transmit(minor_key, fsm_buffer_ptr, num_s2_bytes);
            break;
        case MAJOR_KEY_PROGRAMMER:
            uc_programmer(minor_key, fsm_buffer_ptr, num_s2_bytes);
            break;
        case MAJOR_KEY_CUSTOM_CMD:
            uc_custom_cmd(minor_key, fsm_buffer_ptr, num_s2_bytes);
            break;
        case MAJOR_KEY_RESET:
            uc_reset();
            break;
        default: // Will fall through for MAJOR_KEY_ERROR
            uc_reset_buffers();
            break;
    }
}

void fsm_ack(uint8_t ack_key)
{
    // Send buffer (fsm_send attaches checksum)
    fsm_send(MAJOR_KEY_ACK, ack_key, 0, 0);
}

void fsm_send(uint8_t s_major_key, uint8_t s_minor_key, uint8_t* data, uint32_t data_len)
{
    // Build send buffer
    uint8_t bits;
    if ((data_len == 0) || (data == 0)) bits = 0x00;
    else if (data_len < 0xFF) bits = 0x01;
    else if (data_len < 0xFFFF) bits = 0x02;
    else bits = 0x03;

    // Fill in fsm major and minor key
    fsm_send_buffer[s1_major_key_loc] = s_major_key | bits;
    fsm_send_buffer[s1_minor_key_loc] = s_minor_key;

    // Fill in data length bits
    switch (bits)
    {
        case 0x01:
            *((uint8_t*) fsm_send_buffer+s1_num_s2_bytes_loc) = (uint8_t) data_len;
            break;
        case 0x02:
            *((uint16_t*) fsm_send_buffer+s1_num_s2_bytes_loc) = (uint16_t) data_len;
            break;
        case 0x03:
            bits += 1;
            *((uint32_t*) fsm_send_buffer+s1_num_s2_bytes_loc) = (uint32_t) data_len;
            break;
    }

    // Copy data into fsm buffer & update data len
    if (bits != 0x00)
    {
        memcpy(fsm_send_buffer+num_s1_bytes+bits, data, data_len);
        data_len += num_s1_bytes+bits;
    } else
    {
        data_len = num_s1_bytes;
    }

    // Construct checksum for data
    checksum_struct* check = fsm_get_checksum_struct(s_major_key);
    uint32_t checksum_size = check->get_checksum_size();
    check->get_checksum(fsm_send_buffer, data_len, check->checksum_start, fsm_send_buffer+data_len);

    // If just acking, send and return
    if (s_major_key == MAJOR_KEY_ACK)
    {
        uc_send(fsm_send_buffer, data_len+checksum_size);
        return;
    }

    // Else, send & verify data packet transmission
    do
    {
        // Send data followed by checksum
        // Checksum needs to be sent right after data
        uc_send(fsm_send_buffer, data_len+checksum_size);

        // Read ack (happens only if if not sending an ack)
        fsm_read_next(fsm_ack_buffer, num_s1_bytes, packet_timeout);
        fsm_read_next(fsm_checksum_buffer, checksum_size, packet_timeout);
        if (fsm_check_checksum(fsm_ack_buffer, num_s1_bytes, fsm_checksum_buffer))
        {
            // Handle ack errors or resets
            switch (fsm_ack_buffer[s1_major_key_loc] & s1_major_key_bit_mask)
            {
                case MAJOR_KEY_ACK: // If keys match exit otherwise send again
                    // Ack stores the major key of the sent packet in the minor key location
                    if (fsm_ack_buffer[s1_minor_key_loc] == s_major_key)
                        return;
                    break;
                case MAJOR_KEY_RESET: // Reset and exit if reset received
                    uc_reset();
                    return;
                default: // Default reset buffers and send again
                    uc_reset_buffers();
                    break;
            }
        }
    } while (true);
}

bool fsm_read_next(uint8_t* data_array, uint32_t num_bytes, uint32_t timeout)
{
    // Return true of waiting for 0 bytes
    if (num_bytes == 0) return true;

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

bool fsm_check_checksum(uint8_t* data, uint32_t data_len, uint8_t* checksum_cmp)
{
    checksum_struct* check = fsm_get_checksum_struct(data[s1_major_key_loc] & s1_major_key_bit_mask);
    uint32_t checksum_size = check->get_checksum_size();
    check->get_checksum(data, data_len, check->checksum_start, fsm_checksum_cmp_buffer);
    return check->check_checksum(checksum_cmp, fsm_checksum_cmp_buffer);
}

checksum_struct* fsm_get_checksum_struct(uint8_t gui_key)
{
    switch (gui_key)
    {
        case MAJOR_KEY_IO:
            return &io_checksum;
        case MAJOR_KEY_DATA_TRANSMIT:
            return &data_transfer_checksum;
        case MAJOR_KEY_PROGRAMMER:
            return &programmer_checksum;
        case MAJOR_KEY_CUSTOM_CMD:
            return &custom_cmd_checksum;
        default:
            return &default_checksum;
    }

}
