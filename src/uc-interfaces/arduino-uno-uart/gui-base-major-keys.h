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

#ifndef GUI_BASE_MAJOR_KEYS_H
#define GUI_BASE_MAJOR_KEYS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

// Major Keys enum
typedef enum {
    // Error and reset
    MAJOR_KEY_ERROR = 0,
    MAJOR_KEY_RESET,

    // GUI Types (Major Keys)
    GUI_TYPE_ERROR,
    GUI_TYPE_GENERAL_SETTINGS,
    GUI_TYPE_WELCOME,
    GUI_TYPE_IO,
    GUI_TYPE_DATA_TRANSMIT,
    GUI_TYPE_PROGRAMMER,
    GUI_TYPE_CUSTOM_CMD,

    // Responses
    MAJOR_KEY_READ_RESPONSE,

    // Action confirmations
    MAJOR_KEY_ACK
} GUI_BASE_MAJOR_KEYS;

/* First stage (s1) generic key positions */
typedef enum {
    s1_major_key_loc = 0,
    s1_minor_key_loc,
    s1_num_s2_bytes_loc,
    s1_end_loc
} S1_Major_Settings;

// Variables
static const uint8_t packet_retries = 2;
static const uint32_t packet_timeout = 500; // ms
static const uint8_t num_s1_bytes = s1_end_loc;
extern uint8_t num_s2_bytes;

/*
 * Struct for settings the checksum functions
 * Function signatures must match the others (only name differs)
*/

typedef struct checksum_struct {
    uint32_t (*get_checksum_size) ();
    void (*get_checksum) (const uint8_t*, uint32_t, uint8_t*, uint8_t*);
    bool (*check_checksum) (const uint8_t*, const uint8_t*);
} checksum_struct;

#ifdef __cplusplus
}
#endif

#endif // GUI_BASE_MAJOR_KEYS_H
