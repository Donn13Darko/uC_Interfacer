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

#ifndef GUI_IO_CONTROL_TEST_CLASS_H
#define GUI_IO_CONTROL_TEST_CLASS_H

// Testing class
#include "../../src/user-interfaces/gui-io-control.hpp"

class GUI_IO_CONTROL_TEST_CLASS : public GUI_IO_CONTROL
{
    Q_OBJECT

public:
    GUI_IO_CONTROL_TEST_CLASS(QWidget *parent = 0);
    ~GUI_IO_CONTROL_TEST_CLASS();

    /*** Defines any needed setters & accessors for testing ***/
    QByteArray rcvd_formatted_readAll_test();
    qint64 rcvd_formatted_size_test();
    void rcvd_formatted_clear_test();

    void set_expected_recv_length_test(uint32_t expected_length);
    uint32_t get_expected_recv_length_test();

    void update_current_recv_length_test(uint32_t recv_len);
    uint32_t get_current_recv_length_test();

    void on_ResetGUI_Button_clicked_test();
};

#endif // GUI_IO_CONTROL_TEST_CLASS_H
