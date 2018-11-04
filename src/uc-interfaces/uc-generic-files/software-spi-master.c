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
void software_spi_master_setup(SPI_MASTER_INFO *spi_master)
{
    // Check if already setup
    if (spi_master->SPI_FLAGS & SPI_MASTER_SETUP) return;

    // Setup SCLK to output
    uc_dio_set(spi_master->SCLK_PIN, uc_dio_output);

    // Set MOSI to output
    uc_dio_set(spi_master->MOSI_PIN, uc_dio_output);

    // Set MISO to input
    uc_dio_set(spi_master->MISO_PIN, uc_dio_input);

    // Set master setup flag
    spi_master->SPI_FLAGS |= SPI_MASTER_SETUP;
}

/* Setup slave pin for communication */
void software_spi_master_setup_slave(SPI_SLAVE_INFO *spi_slave)
{
    // Check if already setup
    if (spi_slave->SLAVE_FLAGS & SPI_SLAVE_SETUP) return;

    // Setup slave as unselected
    if (spi_slave->SLAVE_PIN_ADDR != 0xFF)
    {
        uc_dio_set(spi_slave->SLAVE_PIN_ADDR, uc_dio_output);
        uc_dio_write(spi_slave->SLAVE_PIN_ADDR, !(spi_slave->SLAVE_FLAGS & SPI_SLAVE_SELECTED_POL));
    }

    // Set slave setup flag
    spi_slave->SLAVE_FLAGS |= SPI_SLAVE_SETUP;
}

/* Exit SPI interface */
void software_spi_master_exit(SPI_MASTER_INFO *spi_master)
{
    // Set all pins to inputs
    uc_dio_set(spi_master->SCLK_PIN, uc_dio_input);
    uc_dio_set(spi_master->MOSI_PIN, uc_dio_input);
    uc_dio_set(spi_master->MISO_PIN, uc_dio_input);

    // Clear master setup flag
    spi_master->SPI_FLAGS &= ~SPI_MASTER_SETUP;
}

/* Exit slave interface */
void software_spi_master_exit_slave(SPI_SLAVE_INFO *spi_slave)
{
    // Setup slave as unselected
    if (spi_slave->SLAVE_PIN_ADDR != 0xFF)
    {
        uc_dio_set(spi_slave->SLAVE_PIN_ADDR, uc_dio_input);
    }

    // Clear slave setup flag
    spi_slave->SLAVE_FLAGS &= ~SPI_SLAVE_SETUP;
}

void software_spi_master_begin_transaction(SPI_MASTER_INFO *spi_master, SPI_SLAVE_INFO *spi_slave)
{
    // Set SCLK to slave start polarity
    // Negate value once to set as 0 or 1, negate again to set to idle state value
    uc_dio_write(spi_master->SCLK_PIN, !!(spi_slave->SLAVE_FLAGS & SPI_SLAVE_CLK_POL));

    // Set slave selected and wait for slave ready (setup_delay)
    if (spi_slave->SLAVE_PIN_ADDR != 0xFF)
    {
        uc_dio_write(spi_slave->SLAVE_PIN_ADDR, !!(spi_slave->SLAVE_FLAGS & SPI_SLAVE_SELECTED_POL));
        uc_delay_us(spi_slave->SETUP_DELAY_US);
    }

    // Set started bit in spi_master
    spi_master->SPI_FLAGS |= SPI_MASTER_TRANS_STARTED;
}

void software_spi_master_end_transaction(SPI_MASTER_INFO *spi_master, SPI_SLAVE_INFO *spi_slave)
{
    // Set slave unselected
    if (spi_slave->SLAVE_PIN_ADDR != 0xFF)
    {
        uc_dio_write(spi_slave->SLAVE_PIN_ADDR, !(spi_slave->SLAVE_FLAGS & SPI_SLAVE_SELECTED_POL));
    }

    // Can leave SCLK at slaves IDLE (gets overwritten during begin transaction)
    // Clear started bit in spi_master
    spi_master->SPI_FLAGS &= ~SPI_MASTER_TRANS_STARTED;
}

/* Perform a SPI read & write transaction 
 * Expects that setup and begin transaction have already been called
 */
uint32_t software_spi_master_transaction(uint32_t write_data, SPI_MASTER_INFO *spi_master, SPI_SLAVE_INFO *spi_slave)
{
    // Setup variabless
    uint32_t read_data = 0;
    uint8_t i = spi_slave->SLAVE_DATA_BITS;

    // Set clk pulse and idle states
    // Negate to set as 0 or 1
    uint8_t sclk_pulse = !(spi_slave->SLAVE_FLAGS & SPI_SLAVE_CLK_POL);
    uint8_t sclk_idle = !sclk_pulse;

    // Reverse bits if sending LSB first
    if (write_data && (spi_slave->SLAVE_FLAGS & SPI_SLAVE_MSB_LSB_TOGGLE))
    {
        write_data = reverse_bits(write_data, spi_slave->SLAVE_DATA_BITS);
    }

    // Select which clock phase we are using
    if (spi_slave->SLAVE_FLAGS & SPI_SLAVE_CLK_PHA)
    {
        // CPHA = 1
        // "out" changes data on/after leading edge
        // "in" captures data on/after trailing edge
        while (i)
        {
            // Decrement i
            i -= 1;

            // Change clk edge (to pulse state) - leading edge
            uc_dio_write(spi_master->SCLK_PIN, sclk_pulse);

            // Set data (on/after leading edge)
            uc_dio_write(spi_master->MOSI_PIN, ((write_data >> i) & 0x01));

            // Hold SCLK for timeout
            uc_delay_us(spi_slave->SCLK_ACTIVE_PULSE_US);

            // Change clk edge (back to idle state) - trailing edge
            uc_dio_write(spi_master->SCLK_PIN, sclk_idle);

            // Read data (on/after trailing edge)
            read_data |= uc_dio_read(spi_master->MISO_PIN) << i;

            // Hold SCLK for timeout
            uc_delay_us(spi_slave->SCLK_IDLE_PULSE_US);
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
            uc_dio_write(spi_master->MOSI_PIN, ((write_data >> i) & 0x01));

            // Hold SCLK for timeout
            uc_delay_us(spi_slave->SCLK_IDLE_PULSE_US);
            
            // Change clk edge (to pulse state) - leading edge
            uc_dio_write(spi_master->SCLK_PIN, sclk_pulse);

            // Read data (right on/after leading edge)
            read_data |= uc_dio_read(spi_master->MISO_PIN) << i;

            // Hold SCLK for timeout
            uc_delay_us(spi_slave->SCLK_ACTIVE_PULSE_US);

            // Change clk edge (back to idle state) - trailing edge
            uc_dio_write(spi_master->SCLK_PIN, sclk_idle);
        }
    }

    // Reverse bits if receiving LSB first
    if (read_data && (spi_slave->SLAVE_FLAGS & SPI_SLAVE_MSB_LSB_TOGGLE))
    {
        read_data = reverse_bits(read_data, spi_slave->SLAVE_DATA_BITS);
    }

    // Return the read info
    return read_data;
}

/* Performs multiple SPI transactions
 * Assumes setup already called. Will call transaction begin and end
 * 
 * Args:
 * write_data & write_data_len: send data across MOSI
 * read_data & read_data_len: read & store data from MISO
 * spi_master: struct that describes the SPI connection
 * spi_slave: Which, if any, slave to use
 */
void software_spi_master_perform_transaction(void *write_data, uint32_t write_data_len,
                                              void *read_data, uint32_t read_data_len,
                                              SPI_MASTER_INFO *spi_master, SPI_SLAVE_INFO *spi_slave)
{
    // Start transaction
    software_spi_master_begin_transaction(spi_master, spi_slave);

    // Select transaction method
    uint32_t i, j, curr_data;
    switch ((spi_slave->SLAVE_FLAGS & SPI_SLAVE_WRITE_FIRST) | (spi_slave->SLAVE_FLAGS & SPI_SLAVE_READ_FIRST))
    {
        case SPI_SLAVE_WRITE_FIRST:
        {
            // Write data
            for (i = 0; i < write_data_len; i++)
            {
                curr_data = get_next_bits(write_data, i, spi_slave->SLAVE_DATA_BITS);
                software_spi_master_transaction(curr_data, spi_master, spi_slave);
                uc_delay_us(spi_slave->TRANSACTION_DELAY_US);
            }

            // Delay between read and writes
            if (write_data_len && read_data_len)
              uc_delay_us(spi_slave->TRANSACTION_DELAY_US);

            // Read data
            for (i = 0; i < read_data_len; i++)
            {
                curr_data = software_spi_master_transaction(0, spi_master, spi_slave);
                set_next_bits(curr_data, read_data, i, spi_slave->SLAVE_DATA_BITS);
                uc_delay_us(spi_slave->TRANSACTION_DELAY_US);
            }

            break;
        }
        case SPI_SLAVE_READ_FIRST:
        {
            // Read data
            for (i = 0; i < read_data_len; i++)
            {
                curr_data = software_spi_master_transaction(0, spi_master, spi_slave);
                set_next_bits(curr_data, read_data, i, spi_slave->SLAVE_DATA_BITS);
                uc_delay_us(spi_slave->TRANSACTION_DELAY_US);
            }

            // Delay between read and writes
            if (write_data_len && read_data_len)
              uc_delay_us(spi_slave->READ_WRITE_DELAY_US);

            // Write data
            for (i = 0; i < write_data_len; i++)
            {
                curr_data = get_next_bits(write_data, i, spi_slave->SLAVE_DATA_BITS);
                software_spi_master_transaction(curr_data, spi_master, spi_slave);
                uc_delay_us(spi_slave->TRANSACTION_DELAY_US);
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
                    curr_data = get_next_bits(write_data, i, spi_slave->SLAVE_DATA_BITS);
                    curr_data = software_spi_master_transaction(curr_data, spi_master, spi_slave);
                    set_next_bits(curr_data, read_data, i, spi_slave->SLAVE_DATA_BITS);
                    uc_delay_us(spi_slave->TRANSACTION_DELAY_US);
                }
            } else if (read_data_len < write_data_len)
            {
                // Align right means fix smaller to tail end
                if (spi_slave->SLAVE_FLAGS & SPI_SLAVE_ALIGN_RIGHT)
                {
                    // Begin by writing only
                    for (i = 0; i < (write_data_len - read_data_len); i++)
                    {
                        curr_data = get_next_bits(write_data, i, spi_slave->SLAVE_DATA_BITS);
                        software_spi_master_transaction(curr_data, spi_master, spi_slave);
                        uc_delay_us(spi_slave->TRANSACTION_DELAY_US);
                    }

                    // Delay between read and writes
                    if (write_data_len && read_data_len)
                      uc_delay_us(spi_slave->READ_WRITE_DELAY_US);

                    // Finish by writing and reading data packets
                    j = 0;
                    for (i = (write_data_len - read_data_len); i < write_data_len; i++)
                    {

                        curr_data = get_next_bits(write_data, i, spi_slave->SLAVE_DATA_BITS);
                        curr_data = software_spi_master_transaction(curr_data, spi_master, spi_slave);
                        set_next_bits(curr_data, read_data, j++, spi_slave->SLAVE_DATA_BITS);
                        uc_delay_us(spi_slave->TRANSACTION_DELAY_US);
                    }
                } else
                {
                    // Begin by writing & reading
                    for (i = 0; i < read_data_len; i++)
                    {
                        curr_data = get_next_bits(write_data, i, spi_slave->SLAVE_DATA_BITS);
                        curr_data = software_spi_master_transaction(curr_data, spi_master, spi_slave);
                        set_next_bits(curr_data, read_data, i, spi_slave->SLAVE_DATA_BITS);
                        uc_delay_us(spi_slave->TRANSACTION_DELAY_US);
                    }

                    // Delay between read and writes
                    if (write_data_len && read_data_len)
                      uc_delay_us(spi_slave->READ_WRITE_DELAY_US);

                    // Finish by writing remaining data packets
                    for (i = read_data_len; i < write_data_len; i++)
                    {
                        curr_data = get_next_bits(write_data, i, spi_slave->SLAVE_DATA_BITS);
                        software_spi_master_transaction(curr_data, spi_master, spi_slave);
                        uc_delay_us(spi_slave->TRANSACTION_DELAY_US);
                    }
                }
            } else if (write_data_len < read_data_len)
            {
                // Align right means fix smaller to tail end
                if (spi_slave->SLAVE_FLAGS & SPI_SLAVE_ALIGN_RIGHT)
                {
                    // Begin by reading only
                    for (i = 0; i < (read_data_len - write_data_len); i++)
                    {
                        curr_data = software_spi_master_transaction(0, spi_master, spi_slave);
                        set_next_bits(curr_data, read_data, i, spi_slave->SLAVE_DATA_BITS);
                        uc_delay_us(spi_slave->TRANSACTION_DELAY_US);
                    }

                    // Delay between read and writes
                    if (write_data_len && read_data_len)
                      uc_delay_us(spi_slave->READ_WRITE_DELAY_US);

                    // Finish by writing and reading data packets
                    j = 0;
                    for (i = (read_data_len - write_data_len); i < read_data_len; i++)
                    {
                        curr_data = get_next_bits(write_data, j++, spi_slave->SLAVE_DATA_BITS);
                        curr_data = software_spi_master_transaction(curr_data, spi_master, spi_slave);
                        set_next_bits(curr_data, read_data, i, spi_slave->SLAVE_DATA_BITS);
                        uc_delay_us(spi_slave->TRANSACTION_DELAY_US);
                    }
                } else
                {
                    // Begin by writing & reading
                    for (i = 0; i < write_data_len; i++)
                    {
                        curr_data = get_next_bits(write_data, i, spi_slave->SLAVE_DATA_BITS);
                        curr_data = software_spi_master_transaction(curr_data, spi_master, spi_slave);
                        set_next_bits(curr_data, read_data, i, spi_slave->SLAVE_DATA_BITS);
                        uc_delay_us(spi_slave->TRANSACTION_DELAY_US);
                    }

                    // Delay between read and writes
                    if (write_data_len && read_data_len)
                      uc_delay_us(spi_slave->READ_WRITE_DELAY_US);

                    // Finish by reading remaining data bytes
                    for (i = write_data_len; i < read_data_len; i++)
                    {
                        curr_data = software_spi_master_transaction(0, spi_master, spi_slave);
                        set_next_bits(curr_data, read_data, i, spi_slave->SLAVE_DATA_BITS);
                        uc_delay_us(spi_slave->TRANSACTION_DELAY_US);
                    }
                }
            }
            break;
        }
    }

    // End transaction
    software_spi_master_end_transaction(spi_master, spi_slave);
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
