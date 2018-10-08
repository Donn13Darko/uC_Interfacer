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

#ifndef UC_CONTROL_FSM_H
#define UC_CONTROL_FSM_H

// Set checksum to CRC8 w/ lookup table
#define __crc_8
#define __crc_LUT

/*
 * Create a file that inplements main and defines the externs
 *
 * Also to the top of the inheriting .h (before any includes),
 * #define what type of crc to use as well as whether it should
 * use a lookup table or compute on the fly.
 *
 * CRC type defines: __crc_8, __crc_16, __crc_32, or __crc_cust
 * CRC Lookup table: #define __crc_LUT
 * If __crc_cust defined, must do the following
 * (see crc-calcs.cpp/.h for example):
 *  1) typdef _____ crc_t to whatever the type
 *  2) Provide one of the following:
 *    a) LUT if __crc_LUT: static const _____ crc_table[256] = ;
 *    b) Otherwise reverse poly: static const _____ crc_poly = ;
 *
*/

typedef enum {
    GUI_TYPE_ERROR = 0,
    GUI_TYPE_WELCOME,
    GUI_TYPE_IO,
    GUI_TYPE_DATA_TRANSMIT,
    GUI_TYPE_PROGRAMMER
} GUI_TYPE;

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

void fsm_start(uint32_t buffer_len);
bool fsm_read_bytes(uint32_t num_bytes, uint32_t timeout);
uint32_t fsm_read_next(uint8_t* data_array, uint32_t num_bytes, uint32_t timeout);

/* General GUI functions */
/* Removes & returns one byte from received */
extern uint8_t fsm_getch();
/* Waits for timeout milliseconds (args: timeout) */
extern void fsm_delay(uint32_t timeout);
/* Returns number of bytes available */
extern uint32_t fsm_bytes_available();

/* Pin I/O GUI */
/* Set & read the DIO value */
extern void fsm_write_dio(uint8_t pin_num, uint8_t setting, uint32_t value);
extern uint32_t fsm_read_dio(uint8_t pin_num);
/* Set & read the AIO value */
extern void fsm_write_aio(uint8_t pin_num, uint8_t setting, uint32_t value);
extern uint32_t fsm_read_aio(uint8_t pin_num);

#ifdef __cplusplus
}
#endif

#endif // UC_CONTROL_FSM_H
