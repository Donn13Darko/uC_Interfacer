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

    // GUI Types (Major Keys)
    // Must be kept here and in same order as
    // supportedGUIsList in mainwindow.cpp
    MAJOR_KEY_GENERAL_SETTINGS,
    MAJOR_KEY_WELCOME,
    MAJOR_KEY_IO,
    MAJOR_KEY_DATA_TRANSMIT,
    MAJOR_KEY_PROGRAMMER,
    MAJOR_KEY_CUSTOM_CMD,

    // Reset cmds
    MAJOR_KEY_RESET,

    // Action confirmations
    MAJOR_KEY_ACK
} GUI_BASE_MAJOR_KEYS;

// Num ;en bytes enum
typedef enum {
    num_s2_bits_0 = 0x00,
    num_s2_bits_1 = 0x01,
    num_s2_bits_2 = 0x02,
    num_s2_bits_3 = 0x03,
    num_s2_bits_4 = 0x04,
} num_s2_len_bytes;

/* First stage (s1) generic key positions */
typedef enum {
    s1_major_key_loc = 0,
    s1_minor_key_loc,
    s1_end_loc
} S1_Major_Settings;

// Variables
static const uint32_t packet_timeout = 500; // ms
static const uint8_t num_s1_bytes = s1_end_loc;
static const uint8_t s1_major_key_byte_mask = 0x3F;
static const uint8_t s1_num_s2_bits_byte_mask = 0x03;
static const uint8_t s1_num_s2_bits_byte_shift = 6;
extern uint32_t num_s2_bytes;

/*
 * Struct for settings the checksum functions
 * Function signatures must match the others (only name differs)
*/

typedef struct checksum_struct {
    uint32_t (*get_checksum_size) ();
    void (*get_checksum) (const uint8_t*, uint32_t, const uint8_t*, uint8_t*);
    bool (*check_checksum) (const uint8_t*, const uint8_t*);
    const uint8_t *checksum_start;
    const char *checksum_exe;
    uint8_t checksum_is_exe;
} checksum_struct;
#define DEFAULT_CHECKSUM_STRUCT {get_crc_8_LUT_size, get_crc_8_LUT, check_crc_8_LUT, 0, 0, 0}

#ifdef __cplusplus
}
#endif

#endif // GUI_BASE_MAJOR_KEYS_H
