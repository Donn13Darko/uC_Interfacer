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

#include "uc-generic-fsm.h"

UC_CONTROL_FSM::UC_CONTROL_FSM(uint32_t buffer_len)
{
    this->buffer_len = buffer_len;
    this->buffer = new uint8_t[buffer_len];
    return;
}

UC_CONTROL_FSM::~UC_CONTROL_FSM()
{
    delete[] this->buffer;
    return;
}

void UC_CONTROL_FSM::start_fsm()
{
    uint32_t bytes_read;
    while (true)
    {
        // Read next major key or break after 5 seconds
        if ((read_next(this->buffer, 3, 5000) != 3)
                || (check_crc(this->buffer, 2, this->buffer[2])))
        {
            continue;
        }

        // Parse the major key
        switch(key)
        {
            case GUI_TYPE_IO:
                break;
            case GUI_TYPE_DATA_TRANSMIT:
                break;
            case GUI_TYPE_PROGRAMMER:
                break;
            default:
                continue;

        }
    }
}

uint32_t UC_CONTROL_FSM::read_next(uint8_t* data_array, uint32_t num_bytes, uint32_t timeout)
{
    // Set control variables
    uint32_t check_delay = 10; // ms
    uint32_t wait_time = 0;

    // Wait for num_bytes to be received
    while (this->bytes_available() < num_bytes)
    {
        this->delay(this->check_delay);
        wait_time += check_delay;
        if (timeout < wait_time) return 0;
    }

    // Read bytes into array
    for (uint32_t i = 0; i < num_bytes; i++)
    {
        data_array[i] = this->getch();
    }
    return num_bytes;
}

bool UC_CONTROL_FSM::check_crc_8(uint8_t *data_array, uint8_t data_len, uint8_t crc)
{
    return false;
}
