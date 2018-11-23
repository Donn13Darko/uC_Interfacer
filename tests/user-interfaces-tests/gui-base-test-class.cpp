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

#include "gui-base-test-class.hpp"

// Object includes
#include <QString>

GUI_BASE_TEST_CLASS::GUI_BASE_TEST_CLASS(QWidget *parent) :
    GUI_BASE(parent)
{
    /* DO NOTHING */
}

GUI_BASE_TEST_CLASS::~GUI_BASE_TEST_CLASS()
{
    /* DO NOTHING */
}

void GUI_BASE_TEST_CLASS::set_gui_key_test(uint8_t new_key)
{
    // Set new gui key
    gui_key = new_key;
}

void GUI_BASE_TEST_CLASS::set_gui_name_test(QString new_name)
{
    // Update gui_config
    if (gui_config) gui_config->insert(new_name, gui_config->take(gui_name));

    // Set new gui name
    gui_name = new_name;
}

QByteArray GUI_BASE_TEST_CLASS::get_rcvd_formatted_data_test()
{
    // Seek to start
    rcvd_formatted.seek(0);

    // Read all
    QByteArray data = rcvd_formatted.readAll();

    // Seek to end
    rcvd_formatted.seek(rcvd_formatted.size());

    // Return read
    return data;
}

qint64 GUI_BASE_TEST_CLASS::get_rcvd_formatted_size_test()
{
    return rcvd_formatted.size();
}

void GUI_BASE_TEST_CLASS::update_current_recv_length_test(uint32_t recv_len)
{
    update_current_recv_length(recv_len);
}

uint32_t GUI_BASE_TEST_CLASS::get_current_recv_length_test()
{
    return current_recv_length;
}

void GUI_BASE_TEST_CLASS::set_expected_recv_length_test(uint32_t expected_length)
{
    set_expected_recv_length(expected_length);
}

uint32_t GUI_BASE_TEST_CLASS::get_expected_recv_length_test()
{
    return expected_recv_length;
}

QString GUI_BASE_TEST_CLASS::get_expected_recv_length_str_test()
{
    return expected_recv_length_str;
}
