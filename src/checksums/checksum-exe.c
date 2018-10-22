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

#include "checksum-exe.h"

// Executable path
const char* exe_path = "";
uint32_t exe_len = 0;

void set_executable_checksum_exe(const char* new_exe_path)
{
    exe_path = new_exe_path;
    exe_len = strlen(exe_path);
}

void get_checksum_exe(const uint8_t* data_array, uint32_t data_len, uint8_t* checksum_start, uint8_t* data_checksum)
{
    // Get size of checksum
    uint32_t checksum_size = get_checksum_exe_size();

    // Build command line argument
    uint32_t cmd_len = exe_len+data_len+sizeof(data_len)+checksum_size+20;
    char cmd[cmd_len];
    snprintf(cmd, cmd_len, "%s -get_checksum %s %s %s", exe_path, data_array, data_len, checksum_start);

    // Compute checksum
    FILE* fp = popen(cmd, "r");
    if ((fp == NULL)
            || fgets((char*) data_checksum, checksum_size, fp) == NULL)
    {
        pclose(fp);
        return;
    }
    pclose(fp);
}

bool check_checksum_exe(const uint8_t* data_checksum, const uint8_t* cmp_checksum)
{
    // Get size of checksum
    uint32_t checksum_size = get_checksum_exe_size();

    // Build command line argument
    uint32_t cmd_len = exe_len+2*checksum_size*sizeof(data_checksum)+20;
    char cmd[cmd_len];
    snprintf(cmd, cmd_len, "%s -check_checksum %s %s", exe_path, data_checksum, cmp_checksum);

    // Compare checksums
    char return_val[5];
    FILE* fp = popen(cmd, "r");
    if ((fp == NULL)
            || fgets((char*) return_val, 2, fp) == NULL)
    {
        pclose(fp);
        return false;
    }
    pclose(fp);

    // Return value
    if (return_val[0] == '0') return false;
    else return true;
}

uint32_t get_checksum_exe_size()
{
    // Build command line argument
    uint32_t cmd_len = exe_len+20;
    char cmd[cmd_len];
    snprintf(cmd, cmd_len, "%s -get_checksum_size", exe_path);
    char checksum_size_str[5];

    // Compute size
    FILE* fp = popen(cmd, "r");
    if ((fp == NULL)
            || fgets(checksum_size_str, 4, fp) == NULL)
    {
        pclose(fp);
        return 0;
    }
    pclose(fp);

    // Convert size into uint32_t
    uint32_t checksum_size = 0;
    uint8_t i = 3;
    do
    {
        checksum_size = ((checksum_size << 8) | ((uint8_t) checksum_size_str[i]));
    } while (0 < i--);

    return checksum_size;
}
