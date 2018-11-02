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

#include "software-spi-master.h"

/* Reverse bytes (used for MSB and LSB send/recv) */
uint8_t reverse_byte(uint8_t byte);

/* Setup SPI interface for communication */
void software_spi_master_setup(SPI_MASTER_INFO *spi_info)
{
    // Set SCLK to start polarity
    uc_dio_set(spi_info->SCLK_PIN, uc_dio_output, ((spi_info->bitFlags & SPI_MASTER_CLK_POL) >> SPI_MASTER_CLK_POL_POS));

    // Set MOSI to output
    uc_dio_set(spi_info->MOSI_PIN, uc_dio_output, 0);

    // Set MISO to input
    uc_dio_set(spi_info->MISO_PIN, uc_dio_input, 0);

    // Set setup flag
    spi_info->bitFlags |= SPI_MASTER_SETUP;
}

/* Exit SPI interface */
void software_spi_master_exit(SPI_MASTER_INFO *spi_info)
{
    // Set all pins to inputs
    uc_dio_set(spi_info->SCLK_PIN, uc_dio_input, 0);
    uc_dio_set(spi_info->MOSI_PIN, uc_dio_input, 0);
    uc_dio_set(spi_info->MISO_PIN, uc_dio_input, 0);

    // Clear setup flag
    spi_info->bitFlags &= ~SPI_MASTER_SETUP;
}

void software_spi_master_begin_transaction(SPI_IO_INFO *spi_info, uint8_t slave_select,
                                            uint32_t setup_delay_us)
{
    // Pull slave select low and wait for slave ready (delay)
    if (slave_select != 0xFF)
    {
        uc_dio_set(slave_select, uc_dio_output, 0);
        uc_delay_us(setup_delay_us);
    }

    // Set started bit in spi_info
    spi_info->bitFlags |= SPI_MASTER_TRANS_STARTED;
}

void software_spi_master_end_transaction(SPI_IO_INFO *spi_info, uint8_t slave_select)
{
    // Pull slave select high to end transaction
    if (slave_select != 0xFF)
    {
        uc_dio_set(slave_select, uc_dio_output, 1);
    }

    // Clear started bit in spi_info
    spi_info->bitFlags &= ~SPI_MASTER_TRANS_STARTED;
}

/* Perform a SPI read & write transaction 
 * Expects that software_spi_master_setup & software_spi_master_begin_transaction
 * have already been called
 */
uint8_t software_spi_master_byte_transaction(uint8_t send_byte, SPI_MASTER_INFO *spi_info)
{
    uint8_t read_byte = 0, i;
    uint8_t sclk_i = ((spi_info->bitFlags & SPI_MASTER_CLK_POL) >> SPI_MASTER_CLK_POL_POS);
    uint8_t sclk_d = !sclk_i;

    // Reverse byte if sending MSB first
    if (send_byte && !(spi_info->bitFlags & SPI_MASTER_MSB_LSB_TOGGLE))
    {
        send_byte = reverse_byte(send_byte);
    }

    if (spi_info->bitFlags & SPI_MASTER_CLK_PHA)
    {
        // CPHA = 1, set data on leading edge and collect after leading edge
        for (i = 0; i < 8; i++)
        {
            // Set data
            uc_dio_set(spi_info->MOSI, uc_dio_output, ((send_byte >> i) & 0x01));

            // Change edge
            uc_dio_set(spi_info->SCLK, uc_dio_output, sclk_d);

            // Read data
            read_byte = (read_byte << 1) | uc_dio_read(spi_info->MISO);

            // Wait timeout
            uc_delay_us(spi_info->SCLK_DELAY_US);

            // Change edge
            uc_dio_set(spi_info->SCLK, uc_dio_output, sclk_i);

            // Wait timeout
            uc_delay_us(spi_info->SCLK_DELAY_US);
        }
    } else
    {
        // CPHA = 0, set data after leading edge and collect after trailing eadge
        for (i = 0; i < 8; i++)
        {
            // Change edge
            uc_dio_set(spi_info->SCLK, uc_dio_output, sclk_d);

            // Set data
            uc_dio_set(spi_info->MOSI, uc_dio_output, ((send_byte >> i) & 0x01));

            // Wait timeout
            uc_delay_us(spi_info->SCLK_DELAY_US);

            // Change edge
            uc_dio_set(spi_info->SCLK, uc_dio_output, sclk_i);

            // Read data
            read_byte = (read_byte << 1) | uc_dio_read(spi_info->MISO);

            // Wait timeout
            uc_delay_us(spi_info->SCLK_DELAY_US);
        }
    }

    // Reverse byte if receiving LSB first
    if (read_byte && (spi_info->bitFlags & SPI_MASTER_MSB_LSB_TOGGLE))
    {
        read_byte = reverse_byte(read_byte);
    }

    // Return the read byte
    return read_byte;
}

/* Performs a SPI transaction
 * Will call setup, transaction begin, and transaction end if not already called
 * 
 * Args:
 * send_data & send_data_len: send data across MOSI
 * read_data & read_data_len: read & store data from MISO
 * spi_info: struct that describes the SPI connection
 * slave_select: Which, if any, slave to use
 * setup_delay_us: Microseconds to wait after setting up
 * transaction_delay_us: Microseconds to wait between each byte transaction
 * read_write_delay_us: Microseconds to wait between reading or writing if not doing at the same time
 */
void software_spi_master_perform_transaction(uint8_t *send_data, uint32_t send_data_len,
                                              uint8_t *read_data, uint32_t read_data_len,
                                              SPI_MASTER_INFO *spi_info, uint8_t slave_select,
                                              uint32_t setup_delay_us, uint32_t transaction_delay_us,
                                              uint32_t read_write_delay_us)
{
    // Setup SPI interface (if not already setup)
    if (!(spi_info->bitFlags & SPI_MASTER_SETUP))
        software_spi_master_setup(spi_info);

    // Start transaction
    software_spi_master_begin_transaction(spi_info, slave_select, setup_delay_us);

    // Select transaction method
    uint32_t i;
    switch ((spi_info->bitFlags & SPI_MASTER_WRITE_FIRST) | (spi_info->bitFlags & SPI_MASTER_READ_FIRST))
    {
        case SPI_MASTER_WRITE_FIRST:
        {
            // Write data
            for (i = 0; i < send_data_len; i++)
            {
                software_spi_master_perform_transaction(send_data[i], spi_info);
                uc_delay_us(transaction_delay_us);
            }

            // Delay between read and writes
            if (send_data_len && read_data_len)
              uc_delay_us(read_write_delay_us);

            // Read data
            for (i = 0; i < read_data_len; i++)
            {
                read_data[i] = software_spi_master_perform_transaction(0, spi_info);
                uc_delay_us(transaction_delay_us);
            }

            break;
        }
        case SPI_MASTER_READ_FIRST:
        {
            // Read data
            for (i = 0; i < read_data_len; i++)
            {
                read_data[i] = software_spi_master_perform_transaction(0, spi_info);
                uc_delay_us(transaction_delay_us);
            }

            // Delay between read and writes
            if (send_data_len && read_data_len)
              uc_delay_us(read_write_delay_us);

            // Write data
            for (i = 0; i < send_data_len; i++)
            {
                software_spi_master_perform_transaction(send_data[i], spi_info);
                uc_delay_us(transaction_delay_us);
            }

            break;
        }
        default:
        {
            // Read & write data at same time
            for (i = 0; i < read_data_len; i++)
            {
                read_data[i] = software_spi_master_perform_transaction(send_data[i], spi_info);
                uc_delay_us(transaction_delay_us);
            }

            break;
        }
    }

    // End transaction
    software_spi_master_end_transaction(spi_info, slave_select);
}

/* Sends send_data_len bytes from send_data array.
 * Makes a call to software_spi_master_perform_transaction with provided arguments.
 *
 * Args:
 * send_data & send_data_len: send data across MOSI
 * spi_info: struct that describes the SPI connection
 * slave_select: Which, if any, slave to use
 */
void software_spi_master_send_bytes(uint8_t *send_data, uint32_t send_data_len,
                                      SPI_MASTER_INFO *spi_info, uint8_t slave_select = 0xFF,
                                      uint32_t setup_delay_us, uint32_t transaction_delay_us)
{
  // Call perform transaction
  software_spi_master_perform_transaction(send_data, send_data_len, 0, 0,
                                          spi_info, slave_select,
                                          setup_delay_us, transaction_delay_us, 0);
}

/* Reads read_data_len bytes into read_data array
 * Makes a call to software_spi_master_perform_transaction with provided arguments.
 *
 * Args:
 * read_data & read_data_len: read & store data from MISO
 * spi_info: struct that describes the SPI connection
 * slave_select: Which, if any, slave to use
 */
void software_spi_master_read_bytes(uint8_t *read_data, uint32_t read_data_len,
                                    SPI_MASTER_INFO *spi_info, uint8_t slave_select = 0xFF,
                                    uint32_t setup_delay_us, uint32_t transaction_delay_us)
{
  // Call perform transaction
  software_spi_master_perform_transaction(0, 0, read_data, read_data_len,
                                          spi_info, slave_select,
                                          setup_delay_us, transaction_delay_us, 0);
}

/* Sends a single byte acorss the SPI connection. */
void software_spi_master_send_byte(uint8_t send_byte,
                                    SPI_MASTER_INFO *spi_info, uint8_t slave_select = 0xFF,
                                    uint32_t setup_delay_us)
{
    // Call perform transaction
    software_spi_master_perform_transaction(&send_byte, 1, 0, 0, spi_info, slave_select, setup_delay_us, 0, 0);
}

/* Reads and returns a single byte. */
uint8_t software_spi_master_read_byte(SPI_MASTER_INFO *spi_info, uint8_t slave_select,
                                        uint32_t setup_delay_us)
{
    // Setup variables
    uint8_t read_byte;

    // Call perform transaction
    software_spi_master_perform_transaction(0, 0, &read_byte, 1, spi_info, slave_select, setup_delay_us, 0, 0);

    // Return byte
    return read_byte;
}

/* Reverse bytes (used for MSB and LSB send/recv) */
uint8_t reverse_byte(uint8_t byte)
{
    byte = (byte & 0xF0) >> 4 | (byte & 0x0F) << 4;
    byte = (byte & 0xCC) >> 2 | (byte & 0x33) << 2;
    byte = (byte & 0xAA) >> 1 | (byte & 0x55) << 1;
    return byte;
}
