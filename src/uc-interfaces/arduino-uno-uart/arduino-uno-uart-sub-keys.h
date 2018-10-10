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

#ifndef GUI_ARDUINO_UNO_UART_SUB_KEYS_H
#define GUI_ARDUINO_UNO_UART_SUB_KEYS_H

#ifdef __cplusplus
extern "C"
{
#endif

// Used in uC code
typedef enum {
    IO_INPUT = 0,
    IO_OUTPUT,
    IO_PWM,
    IO_SERVO_DEG,
    IO_SERVO_US
} IO_COMBOS;

typedef enum {
    IO_OFF = 0,
    IO_ON
} IO_PINS;

#ifdef __cplusplus
}
#endif

#endif // GUI_ARDUINO_UNO_UART_SUB_KEYS_H
