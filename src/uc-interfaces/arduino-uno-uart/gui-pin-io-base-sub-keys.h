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

#ifndef GUI_PIN_BASE_SUB_KEYS_H
#define GUI_PIN_BASE_SUB_KEYS_H

// IO Sub Keys enum
typedef enum {
    // Error and reset
    SUB_KEY_IO_ERROR = 0,

    // Pin type set
    SUB_KEY_IO_DIO,
    SUB_KEY_IO_AIO,

    // Pin type read
    SUB_KEY_IO_DIO_READ,
    SUB_KEY_IO_AIO_READ,

    // Remote Communications
    SUB_KEY_IO_REMOTE_CONN
} SUB_KEY_IO;

/* Packet #2 (p2) io key positions enum */
typedef enum {
    p2_sub_key_loc = 0,
    p2_pin_num_loc,
    p2_combo_loc,
    p2_value_high_loc,
    p2_value_low_loc
} P2_IO_Settings;

#endif // GUI_PIN_BASE_SUB_KEYS_H
