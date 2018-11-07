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
#include "gui-data-transmit-minor-keys.h"

namespace Ui {
class GUI_DATA_TRANSMIT;
}

class GUI_DATA_TRANSMIT : public GUI_BASE
{
    Q_OBJECT

public:
    explicit GUI_DATA_TRANSMIT(QWidget *parent = 0);
    ~GUI_DATA_TRANSMIT();

    virtual void parseConfigMap(QMap<QString, QVariant> *configMap);

public slots:
    virtual void reset_gui();

protected slots:
    virtual void receive_gui(QByteArray recvData);

    virtual void set_progress_update_recv(int progress, QString label);
    virtual void set_progress_update_send(int progress, QString label);

private slots:
    void on_Send_RadioGroup_buttonClicked(int);
    void on_Send_Button_clicked();

    void on_SendBrowseFile_Button_clicked();
    void on_RecvSave_Button_clicked();

    void on_RecvClear_Button_clicked();

private:
    Ui::GUI_DATA_TRANSMIT *ui;

    void input_select(bool fileIN, bool plainIN);
};

#endif // GUI_DATA_TRANSMIT_H
