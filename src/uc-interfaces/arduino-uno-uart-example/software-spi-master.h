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

#ifndef SOFTWaRE_SPI_MASTER_H
#define SOFTWaRE_SPI_MASTER_H

/*
 * Create a file that inplements main() to start the program
 * and defines the externs matching the descriptions below.
 *
*/

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/* SPI Master Structure */
typedef struct SPI_MASTER_INFO {
    uint8_t MOSI_PIN;
    uint8_t MISO_PIN;
    uint8_t SCLK_PIN;
    uint32_t SCLK_DELAY_US;
    uint8_t SPI_FLAGS; // See SPI_MASTER_FLAGS_ENUM
} SPI_MASTER_INFO;
#define SPI_MASTER_DEFAULT {0, 0, 0, 0, 0}

typedef enum {
    // Set after setup called
    SPI_MASTER_SETUP = 0x01,
    // Set after begin transaction called
    SPI_MASTER_TRANS_STARTED = 0x02,
    // If set, transmits LSB first
    SPI_MASTER_MSB_LSB_TOGGLE = 0x04,
    // Select if reading or writing first
    // If neither or both set, performs at same time
    SPI_MASTER_WRITE_FIRST = 0x08,
    SPI_MASTER_READ_FIRST = 0x10,
    // Use if send_len and read_len not same length
    // And reading at the same time
    SPI_MASTER_ALIGN_RIGHT = 0x20,
    // Defines SCLK polarity
    // setting 0 is pulse high/idle low, 1 is pulse low/idle high
    SPI_MASTER_CLK_POL = 0x40,
    // Defines SCLK data bit timing
    SPI_MASTER_CLK_PHA = 0x80
} SPI_MASTER_FLAGS_ENUM;


/* SPI MASTER Functions */

/* Setup SPI interface for communication */
void software_spi_master_setup(SPI_MASTER_INFO *spi_info);

/* Exit SPI interface */
void software_spi_master_exit(SPI_MASTER_INFO *spi_info);

/* Start SPI transaction */
void software_spi_master_begin_transaction(SPI_MASTER_INFO *spi_info, uint8_t slave_select,
                                            uint32_t setup_delay_us);

/* End SPI transaction */
void software_spi_master_end_transaction(SPI_MASTER_INFO *spi_info, uint8_t slave_select);

/* Perform a SPI read & write transaction */
uint8_t software_spi_master_byte_transaction(uint8_t write_byte, SPI_MASTER_INFO *spi_info);

/* Performs a SPI transaction */
void software_spi_master_perform_transaction(uint8_t *write_data, uint32_t write_data_len,
                                              uint8_t *read_data, uint32_t read_data_len,
                                              SPI_MASTER_INFO *spi_info, uint8_t slave_select,
                                              uint32_t setup_delay_us, uint32_t transaction_delay_us,
                                              uint32_t read_write_delay_us);

/* Sends write_data_len bytes from write_data array. */
void software_spi_master_write_bytes(uint8_t *write_data, uint32_t write_data_len,
                                      SPI_MASTER_INFO *spi_info, uint8_t slave_select,
                                      uint32_t setup_delay_us, uint32_t transaction_delay_us);

/* Reads read_data_len bytes into read_data array */
void software_spi_master_read_bytes(uint8_t *read_data, uint32_t read_data_len,
                                      SPI_MASTER_INFO *spi_info, uint8_t slave_select,
                                      uint32_t setup_delay_us, uint32_t transaction_delay_us);

/* Sends a single byte acorss the SPI connection */
void software_spi_master_write_byte(uint8_t write_byte,
                                    SPI_MASTER_INFO *spi_info, uint8_t slave_select,
                                    uint32_t setup_delay_us);

/* Reads and returns a single byte */
uint8_t software_spi_master_read_byte(SPI_MASTER_INFO *spi_info, uint8_t slave_select,
                                        uint32_t setup_delay_us);


/*** Following extern functions must be defined on a per uC basis ***/

/* Set or read the DIO value */
extern void uc_dio_set(uint8_t pin_num, uint8_t setting, uint16_t value);
extern uint16_t uc_dio_read(uint8_t pin_num);

/* Waits for timeout microseconds (args: timeout) */
extern void uc_delay_us(uint32_t us);

/* Setting key (second arg) for uc_dio_set that sets pin to digital input */
extern const uint8_t uc_dio_input;  
/* Setting key (second arg) for uc_dio_set that sets pin to digital output */
extern const uint8_t uc_dio_output;

#ifdef __cplusplus
}
#endif

#endif // SOFTWaRE_SPI_MASTER_H
