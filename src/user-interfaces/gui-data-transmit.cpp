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

#include "gui-data-transmit.h"
#include "ui_gui-data-transmit.h"

#include <QFile>
#include <QFileDialog>

GUI_DATA_TRANSMIT::GUI_DATA_TRANSMIT(QWidget *parent) :
    GUI_BASE(parent),
    ui(new Ui::GUI_DATA_TRANSMIT)
{
    // Setup UI
    ui->setupUi(this);

    // Set GUI Type
    gui_key = MAJOR_KEY_DATA_TRANSMIT;

    // Setup progress bars
    ui->SendMsg_ProgressBar->setMinimum(0);
    ui->SendMsg_ProgressBar->setMaximum(100);
    ui->RecvMsg_ProgressBar->setMinimum(0);
    ui->RecvMsg_ProgressBar->setMaximum(100);

    // Reset GUI
    reset_gui();
}

GUI_DATA_TRANSMIT::~GUI_DATA_TRANSMIT()
{
    delete ui;
}

void GUI_DATA_TRANSMIT::reset_gui()
{
    // Clear received data
    on_ClearReceived_Button_clicked();

    // Clear entered data
    ui->FilePath_LineEdit->clear();

    // Reset radio selection
    ui->File_Radio->setChecked(true);
    on_MSG_Sel_buttonClicked(0);

    // Set clear on set
    ui->ClearOnSet_CheckBox->setChecked(true);

    // Reset base (resets progress bars)
    GUI_BASE::reset_gui();
}

void GUI_DATA_TRANSMIT::parseConfigMap(QMap<QString, QVariant>* configMap)
{
    // Pass to parent for additional parsing
    GUI_BASE::parseConfigMap(configMap);
}

void GUI_DATA_TRANSMIT::on_MSG_Sel_buttonClicked(int)
{
    if (ui->File_Radio->isChecked())
        input_select(true, false);
    else if (ui->Input_Radio->isChecked())
        input_select(false, true);
}

void GUI_DATA_TRANSMIT::on_SendMSG_Button_clicked()
{
    // Find which radio button is selected
    if (ui->File_Radio->isChecked())
    {
        // Get filePath
        QString filePath = ui->FilePath_LineEdit->text();

        // Send size
        send_chunk(gui_key, MINOR_KEY_DATA_TRANSMIT_SET_TRANS_SIZE,
                   GUI_HELPER::uint32_to_byteArray(GUI_HELPER::getFileSize(filePath)));

        // Send file
        send_file(gui_key, MINOR_KEY_DATA_TRANSMIT_DATA, filePath);
    } else if (ui->Input_Radio->isChecked())
    {
        // Get data
        QByteArray data = ui->MSG_PlainText->toPlainText().toUtf8();

        // Send size
        send_chunk(gui_key, MINOR_KEY_DATA_TRANSMIT_SET_TRANS_SIZE,
                   GUI_HELPER::uint32_to_byteArray(data.length()));

        // Send plaintext
        send_chunk(gui_key, MINOR_KEY_DATA_TRANSMIT_DATA, data);
    }
}

void GUI_DATA_TRANSMIT::on_BrowseFile_Button_clicked()
{
    // Select file to send
    QString file;
    if (GUI_HELPER::getOpenFilePath(&file))
        ui->FilePath_LineEdit->setText(file);
}

void GUI_DATA_TRANSMIT::on_SaveAs_Button_clicked()
{
    save_rcvd_formatted();
}

void GUI_DATA_TRANSMIT::on_ClearReceived_Button_clicked()
{
    ui->Recv_PlainText->clear();
    rcvd_formatted.clear();
    set_expected_recv_length(0);
}

void GUI_DATA_TRANSMIT::receive_gui(QByteArray recvData)
{
    // Get data without keys
    QByteArray data = recvData.mid(s1_end_loc);

    // See if this GUI sent CMD
    if (recvData.at(s1_major_key_loc) == (char) gui_key)
    {
        // See if known CMD
        switch (recvData.at(s1_minor_key_loc))
        {
            case MINOR_KEY_DATA_TRANSMIT_SET_TRANS_SIZE:
                // Clear recv if clear on set checked
                if (ui->ClearOnSet_CheckBox->isChecked())
                    on_ClearReceived_Button_clicked();

                // Set expected length
                set_expected_recv_length(data);
                return;
            case MINOR_KEY_DATA_TRANSMIT_DATA:
                // Update current recv length with each packet
                update_current_recv_length(data);
                break;
        }
    }

    // Check if any data to add
    if (data.isEmpty()) return;

    // Insert into global array (for saving in original format)
    rcvd_formatted.append(data);

    // Insert at end of plaintext
    QTextCursor prev_cursor = ui->Recv_PlainText->textCursor();
    ui->Recv_PlainText->moveCursor(QTextCursor::End);
    ui->Recv_PlainText->insertPlainText(QString(data));
    ui->Recv_PlainText->setTextCursor(prev_cursor);
}

void GUI_DATA_TRANSMIT::set_progress_update_recv(int progress, QString label)
{
    ui->RecvMsg_ProgressBar->setValue(progress);
    ui->RecvMsgProgress_Label->setText(label);
}

void GUI_DATA_TRANSMIT::set_progress_update_send(int progress, QString label)
{
    ui->SendMsg_ProgressBar->setValue(progress);
    ui->SendMsgProgress_Label->setText(label);
}

void GUI_DATA_TRANSMIT::input_select(bool fileIN, bool plainIN)
{
    ui->FilePath_LineEdit->setEnabled(fileIN);
    ui->BrowseFile_Button->setEnabled(fileIN);
    ui->MSG_PlainText->setEnabled(plainIN);
}
