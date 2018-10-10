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

#ifndef UC_GENERIC_FSM_H
#define UC_GENERIC_FSM_H

/*
 * Create a file that inplements main() to start the program
 * and defines the externs matching descriptions below.
 *
 * To the top of the defining .cpp/.h (before any includes),
 * #define what type of crc to use as well as whether it should
 * use a lookup table or compute on the fly. See below for details.
 *
 * CRC type defines: __crc_8, __crc_16, __crc_32, or __crc_cust
 * For CRC Lookup table add "#define __crc_LUT"
 * If __crc_cust defined, must do the following
 * (see crc-calcs.cpp/.h for example definitions):
 *   1) typdef _____ crc_t to whatever the variable type is
 *   2) Provide one of the following:
 *     a) LUT & max address if using __crc_LUT:
 *       // Used to & the generated LUT address
 *       // Prevents out of bounds errors
 *       _____ __crc_LUT_MAX = 0x...;
 *       -and-
 *       // LUT with size defined by __crc_LUT_MAX+1
 *       static const _____ crc_table[...] = {....};
 *     b) Otherwise provide poly (Reverse=True):
 *       static const _____ crc_poly = 0x....;
 *
*/

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "../communication/general-comms.h"

/* FSM Functions */
void fsm_setup(uint32_t buffer_len);
void fsm_destroy();
void fsm_poll();
void fsm_isr();
void fsm_run();
void fsm_ack(uint8_t ack_key);
bool fsm_read_bytes(uint32_t num_bytes, uint32_t timeout);
uint32_t fsm_read_next(uint8_t* data_array, uint32_t num_bytes, uint32_t timeout);

/*** Following extern functions must be defined on a per uC basis ***/

/* Resets all the data and pins on the uc */
extern void uc_reset();
/* Resets receive buffer on the uc */
extern void uc_reset_buffers();
/* Removes & returns one byte from received */
extern uint8_t uc_getch();
/* Waits for timeout milliseconds (args: timeout) */
extern void uc_delay(uint32_t ms);
/* Returns number of bytes available */
extern uint8_t uc_bytes_available();
/* Sends data across the connection, returns number of bytes sent */
extern uint8_t uc_send(uint8_t* data, uint8_t data_len);

/*
 * Recomend including the predefined FSM for each of the below
 * and writing a main() plus externs for each extern defined in the
 * FSM as opposed to reimplimenting the key parsing included (unless
 * space efficiency, speed, or not using (stub it))
*/
/* Parses IO subkeys and acts */
extern void uc_io(const uint8_t* buffer, uint8_t num_bytes);
/* Parses Data Transmit subkeys and acts */
extern void uc_data_transmit(const uint8_t* buffer, uint8_t num_bytes);
/* Parses Programmer subkeys and acts */
extern void uc_programmer(const uint8_t* buffer, uint8_t num_bytes);

#ifdef __cplusplus
}
#endif

#endif // UC_GENERIC_FSM_H
