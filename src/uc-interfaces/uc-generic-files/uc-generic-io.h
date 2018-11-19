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

#include "uc-generic-def.h"
#include "../../user-interfaces/gui-io-control-minor-keys.h"

/* IO Functions */
/* Parses IO minor key and calls uc specific code */
void uc_io(uint8_t major_key, uint8_t minor_key, const uint8_t* buffer, uint32_t buffer_len);

/*** Following externs are defined in uc-generic-fsm (or need to be defiend elsewhere if not using) ***/
extern void fsm_send(uint8_t s_major_key, uint8_t s_minor_key, const uint8_t* data, uint32_t data_len);
extern void fsm_send_ready();

/*** Following extern functions must be defined on a per uC basis ***/

/* Set or read DIO value(s) */
extern void uc_dio_set(uint8_t pin_num, uint8_t setting);
extern void uc_dio_write(uint8_t pin_num, uint16_t value);
extern uint16_t uc_dio_read(uint8_t pin_num);
extern uint16_t* uc_dio_read_all();

/* Set or read AIO value(s) */
extern void uc_aio_set(uint8_t pin_num, uint8_t setting);
extern void uc_aio_write(uint8_t pin_num, uint16_t value);
extern uint16_t uc_aio_read(uint8_t pin_num);
extern uint16_t* uc_aio_read_all();

/* Set Remote Conn info */
extern void uc_remote_conn();

/* Helper variables */
extern const uint8_t uc_dio_num_pins;
extern const uint8_t uc_aio_num_pins;

#ifdef __cplusplus
}
#endif

#endif // UC_GENERIC_IO_H
