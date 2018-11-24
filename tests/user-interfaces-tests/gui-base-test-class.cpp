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

void GUI_BASE_TEST_CLASS::rcvd_formatted_append_test(QByteArray data)
{
    rcvd_formatted_append(data);
}

QByteArray GUI_BASE_TEST_CLASS::rcvd_formatted_readAll_test()
{
    return rcvd_formatted_readAll();
}

qint64 GUI_BASE_TEST_CLASS::rcvd_formatted_size_test()
{
    return rcvd_formatted_size();
}

void GUI_BASE_TEST_CLASS::rcvd_formatted_clear_test()
{
    rcvd_formatted_clear();
}

void GUI_BASE_TEST_CLASS::rcvd_formatted_save_test(QString fileName)
{
    rcvd_formatted_save(fileName);
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

void GUI_BASE_TEST_CLASS::update_current_recv_length_test(uint32_t recv_len)
{
    update_current_recv_length(recv_len);
}

uint32_t GUI_BASE_TEST_CLASS::get_current_recv_length_test()
{
    return current_recv_length;
}

CONFIG_MAP *GUI_BASE_TEST_CLASS::get_gui_config_test()
{
    return gui_config;
}

QMap<QString, QVariant> *GUI_BASE_TEST_CLASS::get_gui_map_test()
{
    return gui_map;
}

void GUI_BASE_TEST_CLASS::on_ResetGUI_Button_clicked_test()
{
    on_ResetGUI_Button_clicked();
}

void GUI_BASE_TEST_CLASS::send_chunk_test(uint8_t major_key, uint8_t minor_key, QList<uint8_t> chunk)
{
    send_chunk(major_key, minor_key, chunk);
}
