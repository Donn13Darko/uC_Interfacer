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

#ifndef UC_GENERIC_IO_H
#define UC_GENERIC_IO_H

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
#include <stdbool.h>
#include <stdlib.h>

/* IO Functions */
/* Parses IO subkeys and calls uc specific code */
void uc_io(const uint8_t* buffer, uint8_t num_bytes);

/*** Following extern functions must be defined on a per uC basis ***/

/* Set or read the DIO value */
extern void uc_dio(uint8_t pin_num, uint8_t setting, uint16_t value);
extern void uc_dio_read();
/* Set or read the AIO value */
extern void uc_aio(uint8_t pin_num, uint8_t setting, uint16_t value);
extern void uc_aio_read();
/* Set Remote Conn info */
extern void uc_remote_conn();


#ifdef __cplusplus
}
#endif

#endif // UC_GENERIC_IO_H
