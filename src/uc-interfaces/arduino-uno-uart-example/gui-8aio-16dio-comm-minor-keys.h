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

#ifndef GUI_8AIO_16DIO_COMM_MINOR_KEYS_H
#define GUI_8AIO_16DIO_COMM_MINOR_KEYS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "gui-pin-io-base-minor-keys.h"

// 8AIO 16DIO COMM Minor Keys enum
typedef enum {
    // Error and reset
    MINOR_KEY_8AIO_16DIO_COMM_ERROR = 0,

    // Remote Communications
    MINOR_KEY_IO_REMOTE_CONN = MINOR_KEY_IO_MAIN_END + 1,
    MINOR_KEY_IO_REMOTE_CONN_SET,
    MINOR_KEY_IO_REMOTE_CONN_READ,
    MINOR_KEY_IO_REMOTE_CONN_SEND

} MINOR_KEYS_8AIO_16DIO_COMM;

#ifdef __cplusplus
}
#endif

#endif // GUI_8AIO_16DIO_COMM_MINOR_KEYS_H
