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

#include <cstdint>

/*
 * Create a class that inherits this class and redefine
 * the protected functions defined here with comment
 * { Default does nothing }.
 *
 * Also to the top of the inheriting class .h definition
 * (before any includes), #define what type of crc to use
 * as well as if it should use a lookup table or compute
 * on the fly.
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

class UC_CONTROL_FSM
{
public:
    explicit UC_CONTROL_FSM(uint32_t buffer_len = 32);
    ~UC_CONTROL_FSM();

    void start_fsm();

protected:
    /* General GUI functions */
    /* Removes & returns one byte from received */
    uint8_t getch() {return 0;}
    /* Waits for timeout milliseconds (args: timeout) */
    void delay(uint32_t) {/*Default do nothing*/}
    /* Returns number of bytes available */
    uint32_t bytes_available() {return 0;}

    /* Pin I/O GUI */
    /* Set & read the DIO value (args: pin_num, setting, value) */
    void write_dio(uint8_t, uint8_t, uint32_t) {/*Default do nothing*/}
    /* (args: pin_num) */
    uint32_t read_dio(uint8_t) {return 0;}
    /* Set & read the AIO value */
    void write_aio(uint8_t, uint8_t, uint32_t) {/*Default do nothing*/}
    /* (args: pin_num) (args: pin_num, setting, value) */
    uint32_t read_aio(uint8_t) {return 0;}

private:
    bool read_bytes(uint32_t num_bytes, uint32_t timeout);
    uint32_t read_next(uint8_t* data_array, uint32_t num_bytes, uint32_t timeout);

    uint32_t buffer_len;
    uint8_t* buffer;
};

#endif // UC_CONTROL_FSM_H
