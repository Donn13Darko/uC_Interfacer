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

#ifndef GUI_DATA_TRANSMIT_H
#define GUI_DATA_TRANSMIT_H

#include "gui-base.h"

namespace Ui {
class GUI_DATA_TRANSMIT;
}

class GUI_DATA_TRANSMIT : public GUI_BASE
{
    Q_OBJECT

public:
    explicit GUI_DATA_TRANSMIT(QWidget *parent = 0);
    ~GUI_DATA_TRANSMIT();

    void reset_gui();

private slots:
    void on_MSG_Sel_buttonClicked(int);
    void on_SendMSG_Button_clicked();

    void on_BrowseFile_Button_clicked();
    void on_SaveAs_Button_clicked();

    void on_ClearReceived_Button_clicked();

    void receive_data_transmit();

private:
    Ui::GUI_DATA_TRANSMIT *ui;

    void input_select(bool fileIN, bool plainIN);
};

#endif // GUI_DATA_TRANSMIT_H
