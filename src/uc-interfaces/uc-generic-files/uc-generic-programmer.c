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

#include "uc-generic-programmer.h"

// Variable definitions
static uint32_t programmer_expected_trans_size = 0;
static uint32_t programmer_curr_trans_size = 0;
static uint8_t programmer_status = 0;
static uint8_t prog_file_format = 0;
static uint8_t prog_burn_method = 0;

// Record file format definitions
typedef enum {
    INTEL_HEX_DATA = 0,
    INTEL_HEX_END_OF_FILE,
    INTEL_HEX_EXTENDED_SEGMENT_ADDR,
    INTEL_HEX_START_SEGMENT_ADDR,
    INTEL_HEX_EXTENDED_LINEAR_ADDR,
    INTEL_HEX_START_LINEAR_ADDR
} INTEL_HEX_RECORD_TYPES;

typedef enum {
    SREC_HEADER = 0,
    SREC_DATA_16BAddr,
    SREC_DATA_24BAddr,
    SREC_DATA_32BAddr,
    SREC_RESERVED,
    SREC_COUNT_16B,
    SREC_COUNT_24B,
    SREC_START_ADDR_32B,
    SREC_START_ADDR_24B,
    SREC_START_ADDR_16B
} SREC_RECORD_TYPES;

void uc_programmer(uint8_t major_key, uint8_t minor_key, const uint8_t* buffer, uint32_t buffer_len)
{
    // Parse and act on minor key
    switch (minor_key)
    {
        case MINOR_KEY_PROGRAMMER_SET_INFO:
        {
            // Check packet length
            if (buffer_len != s2_programmer_settings_info_end) break;

            // Clear all flags
            programmer_status = 0x00;

            // Set local variables
            prog_file_format = buffer[s2_programmer_format_loc];
            prog_burn_method = buffer[s2_programmer_burn_method_loc];

            // Setup burn method
            if (!uc_programmer_setup(prog_burn_method))
            {
                // Set setup flag
                programmer_status |= programmer_setup_flag;
            }

            // Break out
            break;
        }
        case MINOR_KEY_PROGRAMMER_SET_TRANS_SIZE:
        {
            // Check packet length
            if (buffer_len != s2_programmer_settings_trans_end) break;

            // Set expected size and reset current size
            programmer_expected_trans_size = *((uint32_t*) buffer);
            programmer_curr_trans_size = 0;
            break;
        }
        case MINOR_KEY_PROGRAMMER_SET_ADDR:
        {
            // Set the current read/write address

            // Break out
            break;
        }
        case MINOR_KEY_PROGRAMMER_DATA:
        {
            // Verify no error and setup
            if (programmer_status & programmer_error_flag) break;
            else if (!(programmer_status & programmer_setup_flag)) break;

            // Check if start or end command
            if (buffer_len == 0)
            {
                // Toggle the start flag
                programmer_status ^= programmer_start_flag;
                break;
            }

            // Verify that start transaction has been set
            if (!(programmer_status & programmer_start_flag)) break;

            // Send transmission (if received complete line)
            if (uc_programmer_write(prog_file_format, prog_burn_method, buffer, buffer_len))
            {
                // Update current received size
                programmer_curr_trans_size += buffer_len;
            } else
            {
                // If failed, set error flag
                programmer_status |= programmer_error_flag;
            }

            // Break out
            break;
        }
        case MINOR_KEY_PROGRAMMER_READ:
        {
            // Read at the current address with the preset method

            // Break out
            break;
        }
    }

    // Check if need to send a ready signal
    switch (minor_key)
    {
        case MINOR_KEY_PROGRAMMER_DATA:
        {
            // Only send if not a start or end cmd
            if (buffer_len == 0) break;

            // Else, fall through to send ready
        }
        case MINOR_KEY_PROGRAMMER_SET_INFO:
        case MINOR_KEY_PROGRAMMER_SET_ADDR:
        case MINOR_KEY_PROGRAMMER_READ:
        {
            // Send device ready
            fsm_send_ready();
            break;
        }
    }
}

bool FILE_FORMAT_INTEL_HEX_DECODE(const uint8_t* data, uint32_t data_len,
                                  uint32_t *address,
                                  uint8_t **data_line, uint8_t *data_line_len)
{
    // Verify minimum data_len
    if (data_len < 5) return false;

    // Choose action based on record type (4th byte)
    switch (data[3])
    {
        case INTEL_HEX_DATA:
        {
            // Set address info
            // Address array start at 2nd byte
            // Address is always 2 bytes
            *address = *((uint16_t*) (data + 1));

            // Set data info
            // Data starts after record type (5th byte)
            // Data length is first byte
            *data_line = (uint8_t*) data + 4;
            *data_line_len = data[0];

            // Done setting info
            return true;
        }
        case INTEL_HEX_EXTENDED_SEGMENT_ADDR:
        {
            // Multiply by 16 and add to each subsequent data record address
            programmer_segment_addr = ((uint32_t) *((uint16_t*) data + 4)) << 4;
        }
        case INTEL_HEX_EXTENDED_LINEAR_ADDR:
        {
            // Set upper 16 bits of extended linear address
            programmer_extended_linear_addr = ((uint32_t) *((uint16_t*) data + 4)) << 16;
        }
        case INTEL_HEX_END_OF_FILE:         // Nothing to do so ignore
        case INTEL_HEX_START_SEGMENT_ADDR:  // No influence on execution start so ignore
        case INTEL_HEX_START_LINEAR_ADDR:   // No influence on execution start so ignore
        {
            // Set len fields to 0 (nothing to do)
            *data_line_len = 0;
            return true;
        }
        default:
        {
            return false;
        }
    }
}

/*** AVR ICSP Functions & Helpers ***/

bool BURN_METHOD_AVR_ICSP_SETUP()
{
    // Set reset pin to output and set it high
    uc_dio_set(UC_PROGRAMMER_RESET_PIN, UC_DIO_SET_OUTPUT);
    uc_dio_write(UC_PROGRAMMER_RESET_PIN, 1);

    // Setup byte holder
    uint8_t spi_setup_bytes[4];
    spi_setup_bytes[0] = 0xAC;  // CMD Start Byte
    spi_setup_bytes[2] = 0;     // Device echo back
    spi_setup_bytes[3] = 0;     // Device echo back
    
    // Enable memory access (set second byte to 0x53)
    // Must be first CMD after reset pulled high (everything else will be ignored)
    spi_setup_bytes[1] = 0x53;
    spi_exchange_bytes(spi_setup_bytes, 0, 4);

    // Erase the chip (set second byte to 0x80)
    spi_setup_bytes[1] = 0x80;
    spi_exchange_bytes(spi_setup_bytes, 0, 4);

    return true;
}

/* Page vs byte programming mode. */
bool BURN_METHOD_AVR_ICSP_WRITE(uint32_t address, const uint8_t* data, uint32_t data_len)
{
    // Build write page address cmd
    uint8_t cmd_packet[4];
    cmd_packet[0] = 0x4C;
    cmd_packet[1] = (address >> 8) & 0xFF;
    cmd_packet[2] = address & 0xFF;
    cmd_packet[3] = 0;

    // Send page write cmd
    spi_exchange_bytes(cmd_packet, 0, 4);

    // Send page data
    spi_exchange_bytes(data, 0, data_len);

    // Read back each byte of the written line
    uint8_t read_data[data_len];
    for (uint32_t i = 0; i < data_len; i++)
    {
        spi_exchange_bytes(0, read_data, data_len);
        spi_exchange_bytes(0, read_data, data_len);
    }

    // Check each byte of the line
    for (uint32_t i = 0; i < data_len; i++)
    {
        if (data[i] != read_data[i])
        {
            return false;
        }
    }

    return true;
}
