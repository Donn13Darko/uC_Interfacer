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

#ifndef GUI_BASE_TEST_CLASS_H
#define GUI_BASE_TEST_CLASS_H

// Testing class
#include "../../src/user-interfaces/gui-base.hpp"

// Class includes
#include <QByteArray>

class GUI_BASE_TEST_CLASS : public GUI_BASE
{
    Q_OBJECT

public:
    GUI_BASE_TEST_CLASS(QWidget *parent = 0);
    ~GUI_BASE_TEST_CLASS();

    /*** Defines any needed setters & accessors for testing ***/
    void set_gui_key_test(uint8_t new_key);
    void set_gui_name_test(QString new_name);

    void rcvd_formatted_append_test(QByteArray data);
    QByteArray rcvd_formatted_readAll_test();
    qint64 rcvd_formatted_size_test();
    void rcvd_formatted_clear_test();
    void rcvd_formatted_save_test(QString fileName);

    void set_expected_recv_length_test(uint32_t expected_length);
    void update_current_recv_length_test(uint32_t recv_len);

    void on_ResetGUI_Button_clicked_test();
    void send_chunk_test(uint8_t major_key, uint8_t minor_key, QList<uint8_t> chunk);

    QVariant get_gui_map_value_test(QString key);
    void set_gui_map_value_test(QString key, QVariant value);
};

#endif // GUI_BASE_TEST_CLASS_H
