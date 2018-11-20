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

#ifndef UC_GENERIC_PROGRAMMER_H
#define UC_GENERIC_PROGRAMMER_H

/*
 * Create a file that inplements main() to start the program
 * and defines the externs matching descriptions below and in
 * uc-generic-fsm. This file creates a base FSM for the io extern
 * in uc-generic-fsm.
 *
*/

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#include "uc-generic-def.h"
#include "../../user-interfaces/gui-programmer-minor-keys.h"

/* Programmer Functions */
/* Parses minor key and calls uc specific code */
void uc_programmer(uint8_t major_key, uint8_t minor_key, const uint8_t* buffer, uint32_t buffer_len);

/* Defines functions for AVR_ICSP setup and programming */
void BURN_METHOD_AVR_ICSP_SETUP();
void BURN_METHOD_AVR_ICSP_PROG(const uint8_t* prog_line, uint32_t line_len);

/* Defines functions for PIC18_ICSP setup and programming */
void BURN_METHOD_PIC18_ICSP_SETUP();
void BURN_METHOD_PIC18_ICSP_PROG(const uint8_t* prog_line, uint32_t line_len);

/* Defines functions for PIC32_ICSP setup and programming */
void BURN_METHOD_PIC32_ICSP_SETUP();
void BURN_METHOD_PIC32_ICSP_PROG(const uint8_t* prog_line, uint32_t line_len);

/* Defines functions for PIC32_ICSP_4WIRE setup and programming */
void BURN_METHOD_PIC32_ICSP_4WIRE_SETUP();
void BURN_METHOD_PIC32_ICSP_4WIRE_PROG(const uint8_t* prog_line, uint32_t line_len);

/*** Following externs are defined in uc-generic-fsm (or need to be defiend elsewhere if not using) ***/
extern void fsm_send(uint8_t s_major_key, uint8_t s_minor_key, const uint8_t* data, uint32_t data_len);
extern void fsm_send_ready();

/*** Following extern functions must be defined on a per uC basis ***/

/* Select and call the proper programming setup function (some defined here and can be used here) */
extern void uc_programmer_setup(uint8_t prog_file_format, uint8_t prog_burn_method);
/* Select and call the proper programming write function (some defined here and can be used here) */
extern void uc_programmer_write(const uint8_t* prog_line, uint32_t line_len);

/* Perform SPI transactions */
extern void spi_write_bytes(const uint8_t* write_data, uint32_t write_len);
extern void spi_read_bytes(const uint8_t* read_data, uint32_t read_len);

/* Set DIO value(s) */
extern void uc_dio_set(uint8_t pin_num, uint8_t setting);
extern void uc_dio_write(uint8_t pin_num, uint16_t value);

/* Setting key (second arg) for uc_dio_set that sets pin to digital input */
extern const uint8_t UC_DIO_SET_INPUT;  
/* Setting key (second arg) for uc_dio_set that sets pin to digital output */
extern const uint8_t UC_DIO_SET_OUTPUT;
/* Reset pin for setting ICSP */
extern const uint8_t UC_PROGRAMMER_RESET_PIN;

#ifdef __cplusplus
}
#endif

#endif // UC_GENERIC_PROGRAMMER_H
