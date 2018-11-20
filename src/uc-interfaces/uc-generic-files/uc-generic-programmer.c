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
uint32_t programmer_expected_trans_size = 0;
uint32_t programmer_curr_trans_size = 0;
uint8_t programmer_status = 0;

typedef enum {
    programmer_start_flag = 0x01,
    programmer_setup_flag = 0x02,
    programmer_error_flag = 0x04
} PROGRAMMER_STATUS_FLAGS;

void uc_programmer(uint8_t major_key, uint8_t minor_key, const uint8_t* buffer, uint32_t buffer_len)
{
    // Verify bytes for command or return
    switch (minor_key)
    {
        case MINOR_KEY_PROGRAMMER_SET_TRANS_SIZE:
        case MINOR_KEY_PROGRAMMER_SET_ADDR:
        {
            if (buffer_len != s2_programmer_settings_trans_end) return;
            else break;
        }
        case MINOR_KEY_PROGRAMMER_SET_INFO:
        {
            if (buffer_len != s2_programmer_settings_info_end) return;
            else break;
        }
    }

    // Parse and act on minor key
    switch (minor_key)
    {
        case MINOR_KEY_PROGRAMMER_SET_INFO:
        {
            // Setup transmission
            uc_programmer_setup(buffer[s2_programmer_format_loc], buffer[s2_programmer_burn_method_loc]);
            break;
        }
        case MINOR_KEY_PROGRAMMER_SET_TRANS_SIZE:
        {
            // Set expected size and reset current size
            programmer_expected_trans_size = *((uint32_t*) buffer);
            programmer_curr_trans_size = 0;
            break;
        }
        case MINOR_KEY_PROGRAMMER_SET_ADDR:
        {
            // Set the current read/write address
            break;
        }
        case MINOR_KEY_PROGRAMMER_DATA:
        {
            // Verify no error
            if (programmer_status & programmer_error_flag) return;

            // Check if start ot end command
            if (buffer_len == 0)
            {
                // If started, then clear the flag
                if (programmer_status & programmer_start_flag)
                {
                    programmer_status &= ~programmer_start_flag;
                } else
                {
                    programmer_status |= programmer_start_flag;
                }
                return;
            }

            // Verify that start transaction has been set
            if (!(programmer_status & programmer_start_flag)) return;

            // Update current received size
            programmer_curr_trans_size += buffer_len;

            // Send transmission (if received complete line)
            uc_programmer_write(buffer, buffer_len);
            break;
        }
        case MINOR_KEY_PROGRAMMER_READ:
        {
            // Read at the current address with the preset method
            break;
        }
    }
}

void BURN_METHOD_AVR_ICSP_SETUP()
{
    // Clear setup flags
    programmer_status = 0x00;

    // Set reset pin to output and set it high
    uc_dio_set(UC_PROGRAMMER_RESET_PIN, UC_DIO_SET_OUTPUT);
    uc_dio_write(UC_PROGRAMMER_RESET_PIN, 1);

    // Send byte holder
    uint8_t spi_setup_bytes[4];
    spi_setup_bytes[2] = 0;
    spi_setup_bytes[3] = 0;
    
    // Configure setup to enable programming
    // Assumes slave select is set in spi_send_byte (if needed)
    spi_setup_bytes[0] = 0xAC; // Binary: 10101100
    spi_setup_bytes[1] = 0x53; // Binray: 01010011
    spi_write_bytes(spi_setup_bytes, 4);

    // Configure setup to erase the chip
    spi_setup_bytes[0] = 0xAC; // Binary: 10101100
    spi_setup_bytes[1] = 0x80; // Binary: 10000000
    spi_write_bytes(spi_setup_bytes, 4);

    // Set setup flag
    programmer_status |= programmer_setup_flag;
}

void BURN_METHOD_AVR_ICSP_PROG(const uint8_t* prog_line, uint32_t line_len)
{
    // Check if error active
    if (programmer_status & programmer_error_flag) return;

    // Send the line start address

    // Send the line data
    spi_write_bytes(prog_line, line_len);

    // Send the line start address

    // Read back the written line
    uint8_t read_line[line_len];
    spi_read_bytes(read_line, line_len);

    // Check each byte of the line
    for (uint32_t i = 0; i < line_len; i++)
    {
        if (prog_line[i] != read_line[i])
        {
            programmer_status |= programmer_error_flag;
            break;
        }
    }
}
