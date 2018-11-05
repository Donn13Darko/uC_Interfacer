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
    ui->Send_ProgressBar->setMinimum(0);
    ui->Send_ProgressBar->setMaximum(100);
    ui->Recv_ProgressBar->setMinimum(0);
    ui->Recv_ProgressBar->setMaximum(100);

    // Reset GUI
    reset_gui();
}

GUI_DATA_TRANSMIT::~GUI_DATA_TRANSMIT()
{
    delete ui;
}

void GUI_DATA_TRANSMIT::parseConfigMap(QMap<QString, QVariant>* configMap)
{
    // Pass to parent for additional parsing
    GUI_BASE::parseConfigMap(configMap);
}

void GUI_DATA_TRANSMIT::reset_gui()
{
    // Clear received data
    on_RecvClear_Button_clicked();

    // Set default entered values
    ui->Send_PlainText->clear();
    ui->SendFilePath_LineEdit->clear();

    // Reset radio selection
    ui->SendFile_Radio->setChecked(true);
    on_Send_RadioGroup_buttonClicked(0);

    // Set clear on set
    ui->RecvClearOnSet_CheckBox->setChecked(true);

    // Reset base (resets progress bars)
    GUI_BASE::reset_gui();
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
            {
                // Clear recv if clear on set checked
                if (ui->RecvClearOnSet_CheckBox->isChecked())
                    on_RecvClear_Button_clicked();

                // Set expected length
                set_expected_recv_length(GUI_HELPER::byteArray_to_uint32(data));
                return;
            }
            case MINOR_KEY_DATA_TRANSMIT_DATA:
            {
                // Update current recv length with each packet
                update_current_recv_length(data.length());
                break;
            }
        }
    } else
    {
        // Ignore any CMDs not meant for this GUI
        return;
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
    ui->Recv_ProgressBar->setValue(progress);
    ui->RecvProgress_Label->setText(label);
}

void GUI_DATA_TRANSMIT::set_progress_update_send(int progress, QString label)
{
    ui->Send_ProgressBar->setValue(progress);
    ui->SendProgress_Label->setText(label);
}

void GUI_DATA_TRANSMIT::on_Send_RadioGroup_buttonClicked(int)
{
    if (ui->SendFile_Radio->isChecked())
        input_select(true, false);
    else if (ui->SendInput_Radio->isChecked())
        input_select(false, true);
}

void GUI_DATA_TRANSMIT::on_Send_Button_clicked()
{
    // Find which radio button is selected
    if (ui->SendFile_Radio->isChecked())
    {
        // Get filePath
        QString filePath = ui->SendFilePath_LineEdit->text();

        // Send size
        emit transmit_chunk(gui_key, MINOR_KEY_DATA_TRANSMIT_SET_TRANS_SIZE,
                            GUI_HELPER::uint32_to_byteArray(GUI_HELPER::getFileSize(filePath)));

        // Send file
        emit transmit_file(gui_key, MINOR_KEY_DATA_TRANSMIT_DATA, filePath);
    } else if (ui->SendInput_Radio->isChecked())
    {
        // Get data
        QByteArray data = ui->Send_PlainText->toPlainText().toUtf8();

        // Send size
        emit transmit_chunk(gui_key, MINOR_KEY_DATA_TRANSMIT_SET_TRANS_SIZE,
                            GUI_HELPER::uint32_to_byteArray(data.length()));

        // Send plaintext
        emit transmit_chunk(gui_key, MINOR_KEY_DATA_TRANSMIT_DATA, data, true);
    }
}

void GUI_DATA_TRANSMIT::on_SendBrowseFile_Button_clicked()
{
    // Select file to send
    QString file;
    if (GUI_HELPER::getOpenFilePath(&file))
        ui->SendFilePath_LineEdit->setText(file);
}

void GUI_DATA_TRANSMIT::on_RecvSave_Button_clicked()
{
    save_rcvd_formatted();
}

void GUI_DATA_TRANSMIT::on_RecvClear_Button_clicked()
{
    ui->Recv_PlainText->clear();
    rcvd_formatted.clear();
    set_expected_recv_length(0);
}

void GUI_DATA_TRANSMIT::input_select(bool fileIN, bool plainIN)
{
    ui->SendFilePath_LineEdit->setEnabled(fileIN);
    ui->SendBrowseFile_Button->setEnabled(fileIN);
    ui->Send_PlainText->setEnabled(plainIN);
}
