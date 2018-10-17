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

#include "checksum-other.h"

// Executable path
const char* exe_path = "";
uint32_t exe_len = 0;
FILE* fp = NULL;

void set_executable(const char* new_exe_path)
{
    exe_path = new_exe_path;
    exe_len = strlen(exe_path);
}

void run_cmd(const char* cmd)
{
    if (fp != NULL) pclose(fp);
    fp = popen(cmd, "r");
}

void get_checksum_OTHER(const uint8_t* data_array, uint32_t data_len, uint8_t* checksum_start, uint8_t* data_checksum)
{
    // Get size of checksum
    uint32_t checksum_size = get_checksum_OTHER_size();

    // Build command line argument
    uint32_t cmd_len = exe_len+data_len+sizeof(data_len)+checksum_size+5;
    char cmd[cmd_len];
    snprintf(cmd, cmd_len, "%s %s %s %s", exe_path, data_array, data_len, checksum_start);

    // Compute checksum
    run_cmd(cmd);
    if ((fp == NULL)
            || fgets((char*) data_checksum, checksum_size, fp) == NULL)
    {
        return;
    }
}

bool check_checksum_OTHER(const uint8_t* data_checksum, const uint8_t* cmp_checksum)
{
    // Get size of checksum
    uint32_t checksum_size = get_checksum_OTHER_size();

    // Check each byte of the checksum
    for (uint32_t i = 0; i < checksum_size; i++)
    {
        if (data_checksum[i] != cmp_checksum[i])
        {
            return false;
        }
    }
    return true;
}

uint32_t get_checksum_OTHER_size()
{
    // Build command line argument
    uint32_t cmd_len = exe_len+7;
    char cmd[cmd_len];
    snprintf(cmd, cmd_len, "%s -size", exe_path);
    char checksum_size_str[4];

    // Compute size
    run_cmd(cmd);
    if ((fp == NULL)
            || fgets(checksum_size_str, sizeof(checksum_size_str), fp) == NULL)
    {
        return 0;
    }

    // Convert size into uint32_t
    uint32_t checksum_size = 0;
    uint8_t i = 3;
    do
    {
        checksum_size = ((checksum_size << 8) | ((uint8_t) checksum_size_str[i]));
    } while (0 < i--);

    return checksum_size;
}
