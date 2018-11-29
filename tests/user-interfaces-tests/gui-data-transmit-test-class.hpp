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

#ifndef GUI_DATA_TRANSMIT_TEST_CLASS_H
#define GUI_DATA_TRANSMIT_TEST_CLASS_H

// Testing class
#include "../../src/user-interfaces/gui-data-transmit.hpp"

#include <QCheckBox>

class GUI_DATA_TRANSMIT_TEST_CLASS : public GUI_DATA_TRANSMIT
{
    Q_OBJECT

public:
    GUI_DATA_TRANSMIT_TEST_CLASS(QWidget *parent = 0);
    ~GUI_DATA_TRANSMIT_TEST_CLASS();

    /*** Defines any needed setters & accessors for testing ***/
    QByteArray rcvd_formatted_readAll_test();
    qint64 rcvd_formatted_size_test();

    void set_expected_recv_length_test(uint32_t expected_length);
    void update_current_recv_length_test(uint32_t recv_len);

    void receive_gui_test(QByteArray data);
    void reset_clicked_test();

private:
    Ui::GUI_DATA_TRANSMIT *ui_ptr;

    void set_checked_click_test(QCheckBox *check, bool b);
};

#endif // GUI_DATA_TRANSMIT_TEST_CLASS_H
