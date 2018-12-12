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

#ifndef GUI_PROGRAMMER_TEST_CLASS_H
#define GUI_PROGRAMMER_TEST_CLASS_H

// Testing class
#include "../../src/user-interfaces/gui-programmer.hpp"

#include <QCheckBox>

class GUI_PROGRAMMER_TEST_CLASS : public GUI_PROGRAMMER
{
    Q_OBJECT

public:
    GUI_PROGRAMMER_TEST_CLASS(QWidget *parent = 0);
    ~GUI_PROGRAMMER_TEST_CLASS();

    /*** Defines any needed setters & accessors for testing ***/
    QByteArray rcvd_formatted_readAll_test();
    qint64 rcvd_formatted_size_test();

    bool reset_clicked_test();

private:
    Ui::GUI_PROGRAMMER *ui_ptr;

    void set_checked_click_test(QCheckBox *check, bool b);
};

#endif // GUI_PROGRAMMER_TEST_CLASS_H
