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
    uint8_t MOSI_PIN;       // SPI MOSI Pin
    uint8_t MISO_PIN;       // SPI MISO Pin
    uint8_t SCLK_PIN;       // SPI SCLK Pin
    uint8_t SPI_FLAGS;      // See SPI_MASTER_FLAGS_ENUM
} SPI_MASTER_INFO;
#define SPI_MASTER_DEFAULT {0, 0, 0, 0}

typedef enum {
    // Set after setup called
    SPI_MASTER_SETUP = 0x01,
    // Set after begin transaction called
    SPI_MASTER_TRANS_STARTED = 0x02
} SPI_MASTER_FLAGS_ENUM;


/* SPI Slave Struct */
typedef struct SPI_SLAVE_INFO {
    uint8_t SLAVE_PIN_ADDR;         // Pin for slave select (if 0xFF, ignored)
    uint32_t SETUP_DELAY_US;        // Time to wait after setting slave select
    uint32_t SCLK_ACTIVE_PULSE_US;  // Clock tick pulse time (leading edge time)
    uint32_t SCLK_IDLE_PULSE_US;    // Same as IDLE state (tailing edge time)
    uint32_t TRANSACTION_DELAY_US;  // Wait between transmissions
    uint32_t READ_WRITE_DELAY_US;   // Wait between reading and writing data
    uint8_t SLAVE_DATA_BITS;        // Number of bits per transaction (Min: 0, Max: 32, Default: 8)
    uint8_t SLAVE_FLAGS;            // See SPI_SLAVE_FLAGS_ENUM
} SPI_SLAVE_INFO;
#define SPI_SLAVE_DEFAULT {0xFF, 0, 0, 0, 0, 0, 8, 0}

typedef enum {
    // Set after setup called
    SPI_SLAVE_SETUP = 0x01,
    // Inverts the polarity of slave select
    // If set, slave selected = HIGH and not selected = LOW
    SPI_SLAVE_SELECTED_POL = 0x02,
    // If set, transmits LSB first
    SPI_SLAVE_MSB_LSB_TOGGLE = 0x04,
    // Select if reading or writing first
    // If neither or both set, performs at same time
    SPI_SLAVE_WRITE_FIRST = 0x08,
    SPI_SLAVE_READ_FIRST = 0x10,
    // Use if send_len and read_len not same length
    // And reading at the same time
    SPI_SLAVE_ALIGN_RIGHT = 0x20,
    // Defines SCLK polarity
    // setting 0 is pulse high/idle low, 1 is pulse low/idle high
    SPI_SLAVE_CLK_POL = 0x40,
    // Defines read/write phase and timing
    SPI_SLAVE_CLK_PHA = 0x80
} SPI_SLAVE_FLAGS_ENUM;


/* SPI MASTER Functions */

/* Setup SPI interface for communication */
void software_spi_master_setup(SPI_MASTER_INFO *spi_master);

/* Setup slave pin for communication */
void software_spi_master_setup_slave(SPI_SLAVE_INFO *spi_slave);

/* Exit SPI interface */
void software_spi_master_exit(SPI_MASTER_INFO *spi_master);

/* End slave pin for communication */
void software_spi_master_exit_slave(SPI_SLAVE_INFO *spi_slave);

/* Start SPI transaction */
void software_spi_master_begin_transaction(SPI_MASTER_INFO *spi_master, SPI_SLAVE_INFO *spi_slave);

/* End SPI transaction */
void software_spi_master_end_transaction(SPI_MASTER_INFO *spi_master, SPI_SLAVE_INFO *spi_slave);

/* Perform a SPI read & write transaction */
uint32_t software_spi_master_transaction(uint32_t write_data, SPI_MASTER_INFO *spi_master, SPI_SLAVE_INFO *spi_slave);

/* Performs multiple SPI transactions */
void software_spi_master_perform_transaction(void *write_data, uint32_t write_data_len,
                                              void *read_data, uint32_t read_data_len,
                                              SPI_MASTER_INFO *spi_master, SPI_SLAVE_INFO *spi_slave);


/*** Following extern functions must be defined on a per uC basis ***/

/* Set or read the DIO value */
extern void uc_dio_set(uint8_t pin_num, uint8_t setting);
extern void uc_dio_write(uint8_t pin_num, uint16_t value);
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
