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
 * Replace the following section with the desired checksums.
 * This will need to be input as part of the INI file under each
 * GUI section (gets broken apart based on the GUI tab type).
 * Multiple instances of the same tab type must have the same checksum.
 * DEFAULT_CHECKSUM_STRUCT is CRC-8 LUT (#defined in gui-base-major-keys.h).
*/
#include "../../checksums/crc-8-lut.h"

// GUI checksums
#ifdef UC_IO
checksum_struct io_checksum = DEFAULT_CHECKSUM_STRUCT;
#endif
#ifdef UC_DATA_TRANSMIT
checksum_struct data_transfer_checksum = DEFAULT_CHECKSUM_STRUCT;
#endif
#ifdef UC_PROGRAMMER
checksum_struct programmer_checksum = DEFAULT_CHECKSUM_STRUCT;
#endif
#ifdef UC_CUSTOM_CMD
checksum_struct custom_cmd_checksum = DEFAULT_CHECKSUM_STRUCT;
#endif

// Default checksum (for acks, errors, and resets);
checksum_struct default_checksum = DEFAULT_CHECKSUM_STRUCT;

// Dynamic buffer
uint8_t *fsm_buffer;
uint8_t *fsm_buffer_ptr;
uint32_t fsm_buffer_len;

// Static buffers
uint8_t *fsm_ready_buffer;
uint8_t *fsm_ack_buffer;
uint8_t *fsm_checksum_buffer;

// Key & packet holders
uint32_t num_s2_bytes, checksum_max_size, num_default_packet_bytes;
uint8_t major_key, minor_key, num_s2_bits;
uint8_t curr_packet_stage;

typedef enum {
    packet_stage_error = 0,
    packet_stage_read_keys,
    packet_stage_read_num_bytes,
    packet_stage_read_data,
    packet_stage_read_checksum
} packet_stages;

// Function prototypes (local access only)
void fsm_ack(uint8_t ack_key);
bool fsm_read_next(uint8_t* data_array, uint32_t num_bytes, uint32_t timeout);
bool fsm_check_checksum(const uint8_t* data, uint32_t data_len, const uint8_t* checksum_cmp);
checksum_struct* fsm_get_checksum_struct(uint8_t gui_key);

void fsm_setup(uint32_t buffer_len)
{
    // Initialize variables
    fsm_error = 0;                // Set to no error
    major_key = MAJOR_KEY_ERROR;  // All errors are 0
    minor_key = MAJOR_KEY_ERROR;  // All errors are 0
    curr_packet_stage = 1;        // Set to stage 1

    // Select largest checksum size for buffer
    uint32_t checksum_size_cmp;
    checksum_max_size = default_checksum.get_checksum_size();

    // Malloc static buffers (will always have default size checksum)
    num_default_packet_bytes = num_s1_bytes+checksum_max_size;
    fsm_ready_buffer = (uint8_t*) malloc(sizeof(uint8_t) * num_default_packet_bytes);
    fsm_ack_buffer = (uint8_t*) malloc(sizeof(uint8_t) * num_default_packet_bytes);

    // Find max checksum size (for defined interfaces)
#ifdef UC_IO
    checksum_size_cmp = io_checksum.get_checksum_size();
    if (checksum_max_size < checksum_size_cmp) checksum_max_size = checksum_size_cmp;
#endif

#ifdef UC_DATA_TRANSMIT
    checksum_size_cmp = data_transfer_checksum.get_checksum_size();
    if (checksum_max_size < checksum_size_cmp) checksum_max_size = checksum_size_cmp;
#endif

#ifdef UC_PROGRAMMER
    checksum_size_cmp = programmer_checksum.get_checksum_size();
    if (checksum_max_size < checksum_size_cmp) checksum_max_size = checksum_size_cmp;
#endif

#ifdef UC_CUSTOM_CMD
    checksum_size_cmp = custom_cmd_checksum.get_checksum_size();
    if (checksum_max_size < checksum_size_cmp) checksum_max_size = checksum_size_cmp;
#endif

    // Require at least 2 bytes (for major and minor key)
    // Recommend a minimum length of 8 (allows for basic control without any reallocs, min 2+1+4)
    if (buffer_len < 2) buffer_len = 2;
    fsm_buffer_len = buffer_len + checksum_max_size;

    // Malloc checksum buffer (needs to be maximum size for no reallocs)
    fsm_checksum_buffer = (uint8_t*) malloc(sizeof(uint8_t) * checksum_max_size);

    // Malloc initial dynamic buffer (attempt to prevent fragmentation by mallocing last)
    fsm_buffer = (uint8_t*) malloc(sizeof(uint8_t) * fsm_buffer_len);

    // Reset to start defaults
    uc_reset();

    // Preload all info for fsm_ready_buffer
    // Will never change across execution of program
    if (fsm_ready_buffer)
    {
        // Set ready buffer values
        fsm_ready_buffer[s1_major_key_loc] = MAJOR_KEY_DEV_READY;
        fsm_ready_buffer[s1_minor_key_loc] = 0;

        // Computer default checksum on buffer (always default)
        default_checksum.get_checksum(fsm_ready_buffer, num_s1_bytes,
                                        default_checksum.checksum_start,
                                        fsm_ready_buffer+num_s1_bytes);
    }

    // Check mallocs and set error accordingly
    fsm_error |= !(fsm_buffer && fsm_ready_buffer && fsm_ack_buffer && fsm_checksum_buffer);
}

void fsm_destroy()
{
    // Free dynamic buffer
    free(fsm_buffer);

    // Free static buffers
    free(fsm_ready_buffer);
    free(fsm_ack_buffer);
    free(fsm_checksum_buffer);

    // Set fsm_error for malloc nullptr
    // Forces setup call again to use fsm
    fsm_error |= 0x01;
}

void fsm_poll()
{
    // Setup local variables
    uint32_t min_buffer_len;

    // Loop while no errors (error only set if malloc or realloc fails)
    while (!fsm_error)
    {
        // Reset buffer pointer
        fsm_buffer_ptr = fsm_buffer;

        // Read first stage or loop after timeout
        if (!fsm_read_next(fsm_buffer_ptr, num_s1_bytes, packet_timeout)) continue;
        fsm_buffer_ptr += num_s1_bytes;

        // Parse keys
        major_key = fsm_buffer[s1_major_key_loc];
        minor_key = fsm_buffer[s1_minor_key_loc];

        // Decode keys
        num_s2_bits = (major_key >> s1_num_s2_bits_byte_shift) & s1_num_s2_bits_byte_mask;
        major_key &= s1_major_key_byte_mask;

        // Adjust byte length of 3 (want uint32_t not uint24_t)
        if (num_s2_bits == num_s2_bits_3) num_s2_bits = num_s2_bits_4;

        // Verify buffer large enough for num_s2_bits
        min_buffer_len = num_s1_bytes + num_s2_bits;
        if (fsm_buffer_len < min_buffer_len)
        {
            // Make the buffer just large enough, prioritize space over speed
            fsm_buffer_len = min_buffer_len;
            fsm_buffer = (uint8_t*) realloc(fsm_buffer, sizeof(uint8_t) * fsm_buffer_len);

            // Verify realloc
            fsm_error |= !fsm_buffer;
            if (fsm_error) break;

            // Update pointer
            fsm_buffer_ptr = fsm_buffer + num_s1_bytes;
        }

        // Read number of bytes in second stage or ACK failed after packet_timeout ms
        // Reset uc buffers and return to first stage if failed
        if (!fsm_read_next(fsm_buffer_ptr, num_s2_bits, packet_timeout))
        {
            fsm_ack(MAJOR_KEY_ERROR);
            uc_reset_buffers();
            continue;
        }
        
        // Calculate num_s2_bytes
        switch (num_s2_bits)
        {
            case num_s2_bits_1:
                // 1 byte
                num_s2_bytes = (uint8_t) *fsm_buffer_ptr;
                break;
            case num_s2_bits_2:
                // 2 bytes
                num_s2_bytes = (uint16_t) *((uint16_t*) fsm_buffer_ptr);
                break;
            case num_s2_bits_4:
                // 4 bytes
                num_s2_bytes = *((uint32_t*) fsm_buffer_ptr);
                break;
            default:
                num_s2_bytes = 0;
                break;
        }
        fsm_buffer_ptr += num_s2_bits;

        // Get checksum_size
        uint32_t checksum_size = fsm_get_checksum_struct(major_key)->get_checksum_size();

        // Verify buffer large enough for bytes + checksum
        min_buffer_len += num_s2_bytes + checksum_size;
        if (fsm_buffer_len < min_buffer_len)
        {
            // Make the buffer just large enough, prioritize space over speed
            fsm_buffer_len = min_buffer_len;
            fsm_buffer = (uint8_t*) realloc(fsm_buffer, sizeof(uint8_t) * fsm_buffer_len);

            // Verify realloc
            fsm_error |= !fsm_buffer;
            if (fsm_error) break;

            // Update pointer
            fsm_buffer_ptr = fsm_buffer + num_s1_bytes + num_s2_bits;
        }

        // Read Second stage or ACK failed after packet_timeout ms
        // Reset uc buffers and return to first stage if failed
        if (!fsm_read_next(fsm_buffer_ptr, num_s2_bytes, packet_timeout))
        {
            fsm_ack(MAJOR_KEY_ERROR);
            uc_reset_buffers();
            continue;
        }
        fsm_buffer_ptr += num_s2_bytes;

        // Read Checksum
        if (!fsm_read_next(fsm_buffer_ptr, checksum_size, packet_timeout))
        {
            fsm_ack(MAJOR_KEY_ERROR);
            uc_reset_buffers();
            continue;
        }

        // Ignore ack and continue (should never get an ack here)
        if (major_key == MAJOR_KEY_ACK) continue;

        // Check Checksum
        if (!fsm_check_checksum(fsm_buffer, min_buffer_len-checksum_size, fsm_buffer_ptr))
        {
            fsm_ack(MAJOR_KEY_ERROR);
            uc_reset_buffers();
            continue;
        }
        fsm_buffer_ptr -= num_s2_bytes;

        // Send Packet Ack
        fsm_ack(major_key);

        // Run FSM
        fsm_run();
    }

    // Destroy fsm if error
    fsm_destroy();
}

// UPDATED BUT NO REALLOC & NEEDS TESTING
bool fsm_isr()
{
    // Read Keys
    if (curr_packet_stage == packet_stage_read_keys)
    {
        // Only read if enough values present to not block
        if (uc_bytes_available() < num_s1_bytes) return false;

        // Reset fsm_buffer_ptr for first read
        fsm_buffer_ptr = fsm_buffer;

        // Read with 0 timeout
        if (!fsm_read_next(fsm_buffer_ptr, num_s1_bytes, 0)) return false;

        // Parse info for first stage
        major_key = fsm_buffer[s1_major_key_loc];
        minor_key = fsm_buffer[s1_minor_key_loc];

        // Decode num_s2_bits and major key
        num_s2_bits = (major_key >> s1_num_s2_bits_byte_shift) & s1_num_s2_bits_byte_mask;
        major_key &= s1_major_key_byte_mask;

        // Adjust byte length of 3 (want uint32_t not uint24_t)
        if (num_s2_bits == num_s2_bits_3) num_s2_bits = num_s2_bits_4;

        // Move buffer pointer
        fsm_buffer_ptr += num_s1_bytes;

        // If no data go to checksum stage, otherwise read num_s2_bytes
        if (num_s2_bits == num_s2_bits_0) curr_packet_stage = packet_stage_read_checksum;
        else curr_packet_stage = packet_stage_read_num_bytes;
    }

    // Read num_s2_bytes
    if (curr_packet_stage == packet_stage_read_num_bytes)
    {
        // Only read if enough values present to not block
        if (uc_bytes_available() < num_s2_bits) return false;

        // Read second stage with 0 timeout
        if (!fsm_read_next(fsm_buffer_ptr, num_s2_bits, 0)) return false;

        // Calculate num_s2_bytes
        switch (num_s2_bits)
        {
            case num_s2_bits_1:
                // 1 byte
                num_s2_bytes = (uint8_t) *fsm_buffer_ptr;
                break;
            case num_s2_bits_2:
                // 2 bytes
                num_s2_bytes = (uint16_t) *((uint16_t*) fsm_buffer_ptr);
                break;
            case num_s2_bits_4:
                // 4 bytes
                num_s2_bytes = *((uint32_t*) fsm_buffer_ptr);
                break;
            default:
                num_s2_bytes = 0;
                break;
        }

        // Move buffer pointer
        fsm_buffer_ptr += num_s2_bits;

        // Move to read data stage
        curr_packet_stage = packet_stage_read_data;
    } 

    // Read data
    if (curr_packet_stage == packet_stage_read_data)
    {
        // Only read if enough values present to not block
        if (uc_bytes_available() < num_s2_bytes) return false;

        // Read second stage with 0 timeout
        if (!fsm_read_next(fsm_buffer_ptr, num_s2_bytes, 0)) return false;

        // Move buffer pointer
        fsm_buffer_ptr += num_s2_bytes;

        // Move to third stage
        curr_packet_stage = packet_stage_read_checksum;
    } 

    // Read checksum & check data
    if (curr_packet_stage == packet_stage_read_checksum)
    {
        // Only read if enough values present to not block
        uint32_t checksum_size = fsm_get_checksum_struct(major_key)->get_checksum_size();
        if (uc_bytes_available() < checksum_size) return false;

        // Read Checksum with 0 timeout
        if (!fsm_read_next(fsm_checksum_buffer, checksum_size, 0)) return false;

        // Check Checksum
        if (!fsm_check_checksum(fsm_buffer, num_s1_bytes+num_s2_bits+num_s2_bytes, fsm_buffer_ptr))
        {
            fsm_ack(MAJOR_KEY_ERROR);
            uc_reset_buffers();
            return false;
        }

        // Send Packet Ack
        fsm_ack(major_key);

        // Move buffer pointer
        fsm_buffer_ptr -= num_s2_bytes;

        // Return to first stage for next call
        curr_packet_stage = packet_stage_read_keys;

        // Ready for fsm call
        return true;
    }

    // Handle error conditions
    if (packet_stage_read_keys < curr_packet_stage)
    {
        // Reset the buffers
        uc_reset_buffers();

        // Reset packet stage and return false
        curr_packet_stage = packet_stage_read_keys;
        return false;
    }

    // Not ready for fsm call
    return false;
}

void fsm_run()
{
    // Parse and act on major key
    switch (major_key)
    {
#ifdef UC_IO
        case MAJOR_KEY_IO:
            uc_io(major_key, minor_key, fsm_buffer_ptr, num_s2_bytes);
            break;
#endif
#ifdef UC_DATA_TRANSMIT
        case MAJOR_KEY_DATA_TRANSMIT:
            uc_data_transmit(major_key, minor_key, fsm_buffer_ptr, num_s2_bytes);
            break;
#endif
#ifdef UC_PROGRAMMER
        case MAJOR_KEY_PROGRAMMER:
            uc_programmer(major_key, minor_key, fsm_buffer_ptr, num_s2_bytes);
            break;
#endif
#ifdef UC_CUSTOM_CMD
        case MAJOR_KEY_CUSTOM_CMD:
            uc_custom_cmd(major_key, minor_key, fsm_buffer_ptr, num_s2_bytes);
            break;
#endif
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
    // Set ack keys
    fsm_ack_buffer[s1_major_key_loc] = MAJOR_KEY_ACK;
    fsm_ack_buffer[s1_minor_key_loc] = ack_key;

    // Compute checksum with defualt (fsm_ack_buffer will alawys be large enough)
    default_checksum.get_checksum(fsm_ack_buffer, num_s1_bytes,
                                    default_checksum.checksum_start,
                                    fsm_ack_buffer+num_s1_bytes);

    // Send ack
    uc_send(fsm_ack_buffer, num_default_packet_bytes);
}

void fsm_send(uint8_t s_major_key, uint8_t s_minor_key, const uint8_t* data, uint32_t data_len)
{
    // Find data_len size
    if ((data_len == 0) || (data == 0)) num_s2_bits = num_s2_bits_0;
    else if (data_len < 0xFF) num_s2_bits = num_s2_bits_1;
    else if (data_len < 0xFFFF) num_s2_bits = num_s2_bits_2;
    else num_s2_bits = num_s2_bits_4;

    // Find checksum size
    checksum_struct* check = fsm_get_checksum_struct(s_major_key);
    uint32_t checksum_size = check->get_checksum_size();

    // Realloc if need bigger send buffer
    uint32_t min_buffer_len = num_s1_bytes+num_s2_bits+data_len+checksum_size;
    if (fsm_buffer_len < min_buffer_len)
    {
        // Make the buffer just large enough, prioritize space over speed
        fsm_buffer_len = min_buffer_len;
        fsm_buffer = (uint8_t*) realloc(fsm_buffer, sizeof(uint8_t) * fsm_buffer_len);

        // Verify realloc
        fsm_error |= !fsm_buffer;
        if (fsm_error) return;
    }
    fsm_buffer_ptr = fsm_buffer;

    // Fill in major and minor key (handle special case of 4 bytes)
    if (num_s2_bits == num_s2_bits_4) fsm_buffer[s1_major_key_loc] = s_major_key | (num_s2_bits_3 << s1_num_s2_bits_byte_shift);
    else fsm_buffer[s1_major_key_loc] = s_major_key | (num_s2_bits << s1_num_s2_bits_byte_shift);
    fsm_buffer[s1_minor_key_loc] = s_minor_key;
    fsm_buffer_ptr += num_s1_bytes;

    // Fill in data length bytes
    switch (num_s2_bits)
    {
        case num_s2_bits_1:
            // 1 byte
            *((uint8_t*) fsm_buffer_ptr) = (uint8_t) data_len;
            break;
        case num_s2_bits_2:
            // 2 bytes
            *((uint16_t*) fsm_buffer_ptr) = (uint16_t) data_len;
            break;
        case num_s2_bits_4:
            // 4 bytes
            *((uint32_t*) fsm_buffer_ptr) = (uint32_t) data_len;
            break;
    }
    fsm_buffer_ptr += num_s2_bits;

    // Copy data into fsm buffer & update data len
    memcpy(fsm_buffer_ptr, data, data_len);
    fsm_buffer_ptr += data_len;

    // Construct checksum for data
    check->get_checksum(fsm_buffer, min_buffer_len-checksum_size, check->checksum_start, fsm_buffer_ptr);

    // If sending ack, send and return (should never get something back)
    if (s_major_key == MAJOR_KEY_ACK)
    {
        uc_send(fsm_buffer, min_buffer_len);
        return;
    }

    // Else, send & verify data packet transmission
    do
    {
        // Send data followed by checksum
        // Checksum needs to be sent right after data
        uc_send(fsm_buffer, min_buffer_len);

        // Read ack (happens only if if not sending an ack)
        fsm_read_next(fsm_ack_buffer, num_s1_bytes+checksum_size, packet_timeout);
        if (fsm_check_checksum(fsm_ack_buffer, num_s1_bytes, fsm_ack_buffer+num_s1_bytes))
        {
            // Handle ack errors or resets
            switch (fsm_ack_buffer[s1_major_key_loc] & s1_major_key_byte_mask)
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

void fsm_send_ready()
{
    // Send ready
    uc_send(fsm_ready_buffer, num_default_packet_bytes);
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
        uc_delay_ms(check_delay);
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

bool fsm_check_checksum(const uint8_t* data, uint32_t data_len, const uint8_t* checksum_cmp)
{
    checksum_struct* check = fsm_get_checksum_struct(data[s1_major_key_loc] & s1_major_key_byte_mask);
    uint32_t checksum_size = check->get_checksum_size();
    check->get_checksum(data, data_len, check->checksum_start, fsm_checksum_buffer);
    return check->check_checksum(checksum_cmp, fsm_checksum_buffer);
}

checksum_struct* fsm_get_checksum_struct(uint8_t gui_key)
{
    switch (gui_key)
    {
#ifdef UC_IO
        case MAJOR_KEY_IO:
            return &io_checksum;
#endif
#ifdef UC_DATA_TRANSMIT
        case MAJOR_KEY_DATA_TRANSMIT:
            return &data_transfer_checksum;
#endif
#ifdef UC_PROGRAMMER
        case MAJOR_KEY_PROGRAMMER:
            return &programmer_checksum;
#endif
#ifdef UC_CUSTOM_CMD
        case MAJOR_KEY_CUSTOM_CMD:
            return &custom_cmd_checksum;
#endif
        default:
            return &default_checksum;
    }

}
