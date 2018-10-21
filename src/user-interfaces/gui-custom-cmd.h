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

#ifndef GUI_CUSTOM_CMD_H
#define GUI_CUSTOM_CMD_H

#include "gui-base.h"

namespace Ui {
class GUI_CUSTOM_CMD;
}

class GUI_CUSTOM_CMD : public GUI_BASE
{
    Q_OBJECT

public:
    explicit GUI_CUSTOM_CMD(QWidget *parent = 0);
    ~GUI_CUSTOM_CMD();

    void reset_gui();

protected slots:
    void receive_gui();

private slots:
    void on_SaveFeedback_Button_clicked();
    void on_BrowseFile_Button_clicked();
    void on_ClearFeedback_Button_clicked();

    void on_sendCustomCMD_Button_clicked();

    void on_cmdSelect_buttonClicked(int);

private:
    Ui::GUI_CUSTOM_CMD *ui;

    void input_select(bool fileIN, bool manualIN);
    void send_custom_cmd(QString majorKey_char, QString minorKey_char, uint8_t key_base, QByteArray customCMD_bytes, uint8_t customCMD_base);
};

#endif // GUI_CUSTOM_CMD_H
