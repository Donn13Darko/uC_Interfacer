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

#ifndef GUI_PROGRAMMER_MINOR_KEYS_H
#define GUI_PROGRAMMER_MINOR_KEYS_H

#ifdef __cplusplus
extern "C"
{
#endif

// Programmer Minor Keys enum
typedef enum {
    // Error and reset
    MINOR_KEY_PROGRAMMER_ERROR = 0,

    // Set trasnmission size
    MINOR_KEY_PROGRAMMER_SET_TRANS_SIZE,

    // Programer Set
    MINOR_KEY_PROGRAMMER_SET_ADDR,

    // Programmer Send
    MINOR_KEY_PROGRAMMER_DATA,

    // Programmer Read
    MINOR_KEY_PROGRAMMER_READ_ALL,
    MINOR_KEY_PROGRAMMER_READ_ADDR
} MINOR_KEYS_PROGRAMMER;

#ifdef __cplusplus
}
#endif

#endif // GUI_PROGRAMMER_MINOR_KEYS_H
