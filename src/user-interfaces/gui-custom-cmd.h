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
#include "gui-custom-cmd-minor-keys.h"

namespace Ui {
class GUI_CUSTOM_CMD;
}

class GUI_CUSTOM_CMD : public GUI_BASE
{
    Q_OBJECT

public:
    explicit GUI_CUSTOM_CMD(QWidget *parent = 0);
    ~GUI_CUSTOM_CMD();

    virtual void reset_gui();
    virtual void parseConfigMap(QMap<QString, QVariant>* configMap);

protected slots:
    virtual void receive_gui(QByteArray recvData);

    virtual void set_progress_update_recv(int progress, QString label);
    virtual void set_progress_update_send(int progress, QString label);

private slots:
    void on_FeedbackSave_Button_clicked();
    void on_FeedbackClear_Button_clicked();

    void on_CustomCMDKeysInInput_CheckBox_stateChanged(int);
    void on_CustomCMDBrowseFile_Button_clicked();
    void on_CustomCMDSend_Button_clicked();

    void on_CustomCMD_RadioGroup_buttonClicked(int);

private:
    Ui::GUI_CUSTOM_CMD *ui;

    // Send key base storage
    uint8_t send_key_base;
    uint8_t send_cmd_base;

    // Rcv key base storage
    uint8_t recv_key_base;
    uint8_t recv_cmd_base;

    void input_select(bool fileIN, bool manualIN);
    void send_custom_cmd(QString majorKey_char, QString minorKey_char, QString customCMD_bytes);
};

#endif // GUI_CUSTOM_CMD_H
