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

/* Reverse bits (used for MSB and LSB send/recv) */
uint32_t get_next_bits(void *data, uint32_t pos, uint8_t bit_width);
void set_next_bits(uint32_t bits, void *data, uint32_t pos, uint8_t bit_width);

/* Reverse bits (used for MSB and LSB send/recv) */
uint32_t reverse_bits(uint32_t data, uint8_t bits);

/* Setup SPI interface for communication */
void software_spi_master_setup(SPI_MASTER_INFO *spi_info)
{
    // Set SCLK to start polarity
    // Negate value once to set as 0 or 1, negate again to set to idle state value
    uc_dio_set(spi_info->SCLK_PIN, uc_dio_output);
    uc_dio_write(spi_info->SCLK_PIN, !!(spi_info->SPI_FLAGS & SPI_MASTER_CLK_POL));

    // Set MOSI to output
    uc_dio_set(spi_info->MOSI_PIN, uc_dio_output);
    uc_dio_write(spi_info->MOSI_PIN, 0);

    // Set MISO to input
    uc_dio_set(spi_info->MISO_PIN, uc_dio_input);

    // Set setup flag
    spi_info->SPI_FLAGS |= SPI_MASTER_SETUP;
}

/* Setup slave pin for communication */
void software_spi_master_setup_slave(uint8_t slave_select)
{
    // Set slave for use with bus
    uc_dio_set(slave_select, uc_dio_output);
    uc_dio_write(slave_select, 1);
}

/* Exit SPI interface */
void software_spi_master_exit(SPI_MASTER_INFO *spi_info)
{
    // Set all pins to inputs
    uc_dio_set(spi_info->SCLK_PIN, uc_dio_input);
    uc_dio_set(spi_info->MOSI_PIN, uc_dio_input);
    uc_dio_set(spi_info->MISO_PIN, uc_dio_input);

    // Clear setup flag
    spi_info->SPI_FLAGS &= ~SPI_MASTER_SETUP;
}

void software_spi_master_begin_transaction(SPI_MASTER_INFO *spi_info, uint8_t slave_select,
                                            uint32_t setup_delay_us)
{
    // Pull slave select low and wait for slave ready (delay)
    if (slave_select != 0xFF)
    {
        uc_dio_set(slave_select, uc_dio_output);
        uc_dio_write(slave_select, 0);
        uc_delay_us(setup_delay_us);
    }

    // Set started bit in spi_info
    spi_info->SPI_FLAGS |= SPI_MASTER_TRANS_STARTED;
}

void software_spi_master_end_transaction(SPI_MASTER_INFO *spi_info, uint8_t slave_select)
{
    // Pull slave select high to end transaction
    if (slave_select != 0xFF)
    {
        uc_dio_write(slave_select, 1);
    }

    // Clear started bit in spi_info
    spi_info->SPI_FLAGS &= ~SPI_MASTER_TRANS_STARTED;
}

/* Perform a SPI read & write transaction 
 * Expects that software_spi_master_setup & software_spi_master_begin_transaction
 * have already been called
 */
uint32_t software_spi_master_transaction(uint32_t write_data, SPI_MASTER_INFO *spi_info)
{
    // Setup variabless
    uint32_t read_data = 0;
    uint8_t i = spi_info->SPI_DATA_BITS;

    // Set clk pulse and idle states
    // Negate to set as 0 or 1
    uint8_t sclk_pulse = !(spi_info->SPI_FLAGS & SPI_MASTER_CLK_POL);
    uint8_t sclk_idle = !sclk_pulse;

    // Reverse bits if sending LSB first
    if (write_data && (spi_info->SPI_FLAGS & SPI_MASTER_MSB_LSB_TOGGLE))
    {
        write_data = reverse_bits(write_data, spi_info->SPI_DATA_BITS);
    }

    // Select which clock phase we are using
    if (spi_info->SPI_FLAGS & SPI_MASTER_CLK_PHA)
    {
        // CPHA = 1
        // "out" changes data on/after leading edge
        // "in" captures data on/after trailing edge
        while (i)
        {
            // Decrement i
            i -= 1;

            // Change clk edge (to pulse state) - leading edge
            uc_dio_write(spi_info->SCLK_PIN, sclk_pulse);

            // Set data (on/after leading edge)
            uc_dio_write(spi_info->MOSI_PIN, ((write_data >> i) & 0x01));

            // Hold SCLK for timeout
            uc_delay_us(spi_info->SCLK_PULSE_US);

            // Change clk edge (back to idle state) - trailing edge
            uc_dio_write(spi_info->SCLK_PIN, sclk_idle);

            // Read data (on/after trailing edge)
            read_data |= uc_dio_read(spi_info->MISO_PIN) << i;

            // Hold SCLK for timeout
            uc_delay_us(spi_info->SCLK_PULSE_US);
        }
    } else
    {
        // CPHA = 0
        // "out" changes data before leading edge (on trailing edge of previous clock)
        // "in" captures data on/after leading edge
        while (i)
        {
            // Decrement i
            i -= 1;

            // Set data (before leading edge)
            uc_dio_write(spi_info->MOSI_PIN, ((write_data >> i) & 0x01));

            // Hold SCLK for timeout
            uc_delay_us(spi_info->SCLK_PULSE_US);
            
            // Change clk edge (to pulse state) - leading edge
            uc_dio_write(spi_info->SCLK_PIN, sclk_pulse);

            // Read data (right on/after leading edge)
            read_data |= uc_dio_read(spi_info->MISO_PIN) << i;

            // Hold SCLK for timeout
            uc_delay_us(spi_info->SCLK_PULSE_US);

            // Change clk edge (back to idle state) - trailing edge
            uc_dio_write(spi_info->SCLK_PIN, sclk_idle);
        }
    }

    // Reverse bits if receiving LSB first
    if (read_data && (spi_info->SPI_FLAGS & SPI_MASTER_MSB_LSB_TOGGLE))
    {
        read_data = reverse_bits(read_data, spi_info->SPI_DATA_BITS);
    }

    // Return the read info
    return read_data;
}

/* Performs multiple SPI transactions
 * Will call setup, transaction begin, and transaction end if not already called
 * 
 * Args:
 * write_data & write_data_len: send data across MOSI
 * read_data & read_data_len: read & store data from MISO
 * spi_info: struct that describes the SPI connection
 * slave_select: Which, if any, slave to use
 * setup_delay_us: Microseconds to wait after setting up
 * transaction_delay_us: Microseconds to wait between each byte transaction
 * read_write_delay_us: Microseconds to wait between reading or writing if not doing at the same time
 */
void software_spi_master_perform_transaction(void *write_data, uint32_t write_data_len,
                                              void *read_data, uint32_t read_data_len,
                                              SPI_MASTER_INFO *spi_info, uint8_t slave_select,
                                              uint32_t setup_delay_us, uint32_t transaction_delay_us,
                                              uint32_t read_write_delay_us)
{
    // Setup SPI interface (if not already setup)
    if (!(spi_info->SPI_FLAGS & SPI_MASTER_SETUP))
        software_spi_master_setup(spi_info);

    // Start transaction
    software_spi_master_begin_transaction(spi_info, slave_select, setup_delay_us);

    // Select transaction method
    uint32_t i, j, curr_data;
    switch ((spi_info->SPI_FLAGS & SPI_MASTER_WRITE_FIRST) | (spi_info->SPI_FLAGS & SPI_MASTER_READ_FIRST))
    {
        case SPI_MASTER_WRITE_FIRST:
        {
            // Write data
            for (i = 0; i < write_data_len; i++)
            {
                curr_data = get_next_bits(write_data, i, spi_info->SPI_DATA_BITS);
                software_spi_master_transaction(curr_data, spi_info);
                uc_delay_us(transaction_delay_us);
            }

            // Delay between read and writes
            if (write_data_len && read_data_len)
              uc_delay_us(read_write_delay_us);

            // Read data
            for (i = 0; i < read_data_len; i++)
            {
                curr_data = software_spi_master_transaction(0, spi_info);
                set_next_bits(curr_data, read_data, i, spi_info->SPI_DATA_BITS);
                uc_delay_us(transaction_delay_us);
            }

            break;
        }
        case SPI_MASTER_READ_FIRST:
        {
            // Read data
            for (i = 0; i < read_data_len; i++)
            {
                curr_data = software_spi_master_transaction(0, spi_info);
                set_next_bits(curr_data, read_data, i, spi_info->SPI_DATA_BITS);
                uc_delay_us(transaction_delay_us);
            }

            // Delay between read and writes
            if (write_data_len && read_data_len)
              uc_delay_us(read_write_delay_us);

            // Write data
            for (i = 0; i < write_data_len; i++)
            {
                curr_data = get_next_bits(write_data, i, spi_info->SPI_DATA_BITS);
                software_spi_master_transaction(curr_data, spi_info);
                uc_delay_us(transaction_delay_us);
            }

            break;
        }
        default:
        {
            // FIXME - Needs to be space and time optimized
            // Read & write data at same time
            if (read_data_len == write_data_len)
            {
                for (i = 0; i < read_data_len; i++)
                {
                    curr_data = get_next_bits(write_data, i, spi_info->SPI_DATA_BITS);
                    curr_data = software_spi_master_transaction(curr_data, spi_info);
                    set_next_bits(curr_data, read_data, i, spi_info->SPI_DATA_BITS);
                    uc_delay_us(transaction_delay_us);
                }
            } else if (read_data_len < write_data_len)
            {
                // Align right means fix smaller to tail end
                if (spi_info->SPI_FLAGS & SPI_MASTER_ALIGN_RIGHT)
                {
                    // Begin by writing only
                    for (i = 0; i < (write_data_len - read_data_len); i++)
                    {
                        curr_data = get_next_bits(write_data, i, spi_info->SPI_DATA_BITS);
                        software_spi_master_transaction(curr_data, spi_info);
                        uc_delay_us(transaction_delay_us);
                    }

                    // Delay between read and writes
                    if (write_data_len && read_data_len)
                      uc_delay_us(read_write_delay_us);

                    // Finish by writing and reading data packets
                    j = 0;
                    for (i = (write_data_len - read_data_len); i < write_data_len; i++)
                    {

                        curr_data = get_next_bits(write_data, i, spi_info->SPI_DATA_BITS);
                        curr_data = software_spi_master_transaction(curr_data, spi_info);
                        set_next_bits(curr_data, read_data, j++, spi_info->SPI_DATA_BITS);
                        uc_delay_us(transaction_delay_us);
                    }
                } else
                {
                    // Begin by writing & reading
                    for (i = 0; i < read_data_len; i++)
                    {
                        curr_data = get_next_bits(write_data, i, spi_info->SPI_DATA_BITS);
                        curr_data = software_spi_master_transaction(curr_data, spi_info);
                        set_next_bits(curr_data, read_data, i, spi_info->SPI_DATA_BITS);
                        uc_delay_us(transaction_delay_us);
                    }

                    // Delay between read and writes
                    if (write_data_len && read_data_len)
                      uc_delay_us(read_write_delay_us);

                    // Finish by writing remaining data packets
                    for (i = read_data_len; i < write_data_len; i++)
                    {
                        curr_data = get_next_bits(write_data, i, spi_info->SPI_DATA_BITS);
                        software_spi_master_transaction(curr_data, spi_info);
                        uc_delay_us(transaction_delay_us);
                    }
                }
            } else if (write_data_len < read_data_len)
            {
                // Align right means fix smaller to tail end
                if (spi_info->SPI_FLAGS & SPI_MASTER_ALIGN_RIGHT)
                {
                    // Begin by reading only
                    for (i = 0; i < (read_data_len - write_data_len); i++)
                    {
                        curr_data = software_spi_master_transaction(0, spi_info);
                        set_next_bits(curr_data, read_data, i, spi_info->SPI_DATA_BITS);
                        uc_delay_us(transaction_delay_us);
                    }

                    // Delay between read and writes
                    if (write_data_len && read_data_len)
                      uc_delay_us(read_write_delay_us);

                    // Finish by writing and reading data packets
                    j = 0;
                    for (i = (read_data_len - write_data_len); i < read_data_len; i++)
                    {
                        curr_data = get_next_bits(write_data, j++, spi_info->SPI_DATA_BITS);
                        curr_data = software_spi_master_transaction(curr_data, spi_info);
                        set_next_bits(curr_data, read_data, i, spi_info->SPI_DATA_BITS);
                        uc_delay_us(transaction_delay_us);
                    }
                } else
                {
                    // Begin by writing & reading
                    for (i = 0; i < write_data_len; i++)
                    {
                        curr_data = get_next_bits(write_data, i, spi_info->SPI_DATA_BITS);
                        curr_data = software_spi_master_transaction(curr_data, spi_info);
                        set_next_bits(curr_data, read_data, i, spi_info->SPI_DATA_BITS);
                        uc_delay_us(transaction_delay_us);
                    }

                    // Delay between read and writes
                    if (write_data_len && read_data_len)
                      uc_delay_us(read_write_delay_us);

                    // Finish by reading remaining data bytes
                    for (i = write_data_len; i < read_data_len; i++)
                    {
                        curr_data = software_spi_master_transaction(0, spi_info);
                        set_next_bits(curr_data, read_data, i, spi_info->SPI_DATA_BITS);
                        uc_delay_us(transaction_delay_us);
                    }
                }
            }
            break;
        }
    }

    // End transaction
    software_spi_master_end_transaction(spi_info, slave_select);
}

/* Sends write_data_len transactions from write_data array.
 * Makes a call to software_spi_master_perform_transaction with provided arguments.
 *
 * Args:
 * write_data & write_data_len: send data across MOSI
 * spi_info: struct that describes the SPI connection
 * slave_select: Which, if any, slave to use
 */
void software_spi_master_write_data(void *write_data, uint32_t write_data_len,
                                      SPI_MASTER_INFO *spi_info, uint8_t slave_select,
                                      uint32_t setup_delay_us, uint32_t transaction_delay_us)
{
  // Call perform transaction
  software_spi_master_perform_transaction(write_data, write_data_len, 0, 0,
                                          spi_info, slave_select,
                                          setup_delay_us, transaction_delay_us, 0);
}

/* Reads read_data_len transactions into read_data array
 * Makes a call to software_spi_master_perform_transaction with provided arguments.
 *
 * Args:
 * read_data & read_data_len: read & store data from MISO
 * spi_info: struct that describes the SPI connection
 * slave_select: Which, if any, slave to use
 */
void software_spi_master_read_data(void *read_data, uint32_t read_data_len,
                                    SPI_MASTER_INFO *spi_info, uint8_t slave_select,
                                    uint32_t setup_delay_us, uint32_t transaction_delay_us)
{
  // Call perform transaction
  software_spi_master_perform_transaction(0, 0, read_data, read_data_len,
                                          spi_info, slave_select,
                                          setup_delay_us, transaction_delay_us, 0);
}

/* Sends a single transaction acorss the SPI connection. */
void software_spi_master_write_single(uint32_t write_data,
                                        SPI_MASTER_INFO *spi_info, uint8_t slave_select,
                                        uint32_t setup_delay_us)
{
    // Call perform transaction
    software_spi_master_perform_transaction(&write_data, 1, 0, 0, spi_info, slave_select, setup_delay_us, 0, 0);
}

/* Reads and returns a single transaction. */
uint32_t software_spi_master_read_single(SPI_MASTER_INFO *spi_info, uint8_t slave_select,
                                        uint32_t setup_delay_us)
{
    // Setup variables
    uint32_t read_data;

    // Call perform transaction
    software_spi_master_perform_transaction(0, 0, &read_data, 1, spi_info, slave_select, setup_delay_us, 0, 0);

    // Return data
    return read_data;
}

/* Sends a single transaction acorss the SPI connection. */
uint32_t software_spi_master_read_write_single(uint32_t write_data,
                                                SPI_MASTER_INFO *spi_info, uint8_t slave_select,
                                                uint32_t setup_delay_us)
{
    // Setup variables
    uint32_t read_data;

    // Call perform transaction
    software_spi_master_perform_transaction(&write_data, 1, &read_data, 1, spi_info, slave_select, setup_delay_us, 0, 0);

    // Return data
    return read_data;
}

/* Get the next bits of data */
uint32_t get_next_bits(void *data, uint32_t pos, uint8_t data_len)
{
    if (data_len <= 8)
        return ((uint8_t*) data)[pos];
    else if (data_len <= 16)
        return ((uint16_t*) data)[pos];
    else if (data_len <= 32)
        return ((uint32_t*) data)[pos];
    else
        return 0;
}

/* Set the next bits of data */
void set_next_bits(uint32_t bits, void *data, uint32_t pos, uint8_t data_len)
{
    if (data_len <= 8)
        ((uint8_t*) data)[pos] = (uint8_t) bits;
    else if (data_len <= 16)
        ((uint16_t*) data)[pos] = (uint16_t) bits;
    else if (data_len <= 32)
        ((uint32_t*) data)[pos] = bits;
}

/* Reverse bits (used for LSB send/recv) */
uint32_t reverse_bits(uint32_t data, uint8_t bits)
{
    // Copy data
    uint32_t r_data = 0;

    // Loop until zero
    while (data && bits)
    {
        r_data = (r_data << 1) | (data & 0x01);
        data >>= 1;
        bits -= 1;
    }
    r_data <<= bits;

    // Return the data reversed
    return r_data;
}
