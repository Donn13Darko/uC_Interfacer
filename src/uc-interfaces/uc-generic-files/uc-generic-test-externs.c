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

/*
 * Implements all externs for compile tests of uc-generic-fsm
 *
*/

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "uc-generic-def.h"

#include "uc-generic-fsm.h"
#include "uc-generic-io.h"
#include "uc-generic-data-transmit.h"
#include "uc-generic-programmer.h"

/*** main function for testing setup ***/
int main(int argc, char const *argv[])
{
    // setup fsm
    fsm_setup(32);

    // Start fsm in poll mode (will loop until error)
    fsm_poll();

    // Exit (never reached)
    return 0;
}

/*** uc-generic-fsm extern functions ***/

void uc_reset() { /* Do Nothing*/ }
void uc_reset_buffers() { /* Do Nothing*/ }
uint8_t uc_getch() { return 0; }
void uc_delay_us(uint32_t us) { /* Do Nothing*/ }
void uc_delay_ms(uint32_t ms) { /* Do Nothing*/ }
uint32_t uc_bytes_available() { return 0xFFFFFFFF; }
uint8_t uc_send(uint8_t* data, uint32_t data_len) { return data_len; }

/* 
 * Expects the uc-generic file to be included if defined.
 * Will define all externs required for each uc-generic file to function.
*/
#ifdef UC_IO
uint16_t uc_dio_read(uint8_t pin_num) { return pin_num; }
uint16_t* uc_dio_read_all() { return 0; }
void uc_aio_set(uint8_t pin_num, uint8_t setting) { /* Do Nothing*/ }
void uc_aio_write(uint8_t pin_num, uint16_t value) { /* Do Nothing*/ }
uint16_t uc_aio_read(uint8_t pin_num) { return pin_num; }
uint16_t* uc_aio_read_all() { return 0; }
void uc_remote_conn() { /* Do Nothing*/ }
const uint8_t uc_dio_num_pins = 0;
const uint8_t uc_aio_num_pins = 0;
#endif

#if defined(UC_IO) || defined(UC_PROGRAMMER)
void uc_dio_set(uint8_t pin_num, uint8_t setting) { /* Do Nothing*/ }
void uc_dio_write(uint8_t pin_num, uint16_t value) { /* Do Nothing*/ }
#endif

#ifdef UC_DATA_TRANSMIT
void uc_data_handle(const uint8_t* buffer, uint8_t buffer_len)  { /* Do Nothing*/ }
#endif

#ifdef UC_PROGRAMMER
bool uc_programmer_setup(uint8_t prog_burn_method)  { return true; }
bool uc_programmer_write(uint8_t prog_file_format, uint8_t prog_burn_method,
                          const uint8_t* data, uint32_t data_len) { return true; }
void spi_exchange_bytes(const uint8_t* write_data, const uint8_t* read_data, uint32_t data_len) { /* Do Nothing*/ }
const uint8_t UC_DIO_SET_INPUT = 0;
const uint8_t UC_DIO_SET_OUTPUT = 1;
const uint8_t UC_PROGRAMMER_RESET_PIN = 0;
#endif

#ifdef UC_CUSTOM_CMD
void uc_custom_cmd(uint8_t major_key, uint8_t minor_key, const uint8_t* buffer, uint32_t buffer_len) { /* Do Nothing*/ }
#endif

#ifdef __cplusplus
}
#endif
