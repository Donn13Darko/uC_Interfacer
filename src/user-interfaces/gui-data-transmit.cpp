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
    guiType = GUI_TYPE_DATA_TRANSMIT;

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
    // Create Key array (Major & Minor)
    QByteArray keys;
    keys.append((char) guiType);
    keys.append((char) MINOR_KEY_DATA_TRANSMIT_DATA);

    // Find which radio button selected
    if (ui->File_Radio->isChecked())
    {
        send_file(keys, ui->FilePath_LineEdit->text());
    } else if (ui->Input_Radio->isChecked())
    {
        send_chunk(keys, ui->msg_PlainText->toPlainText().toUtf8());
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
    ui->recv_PlainText->clear();
    rcvd_formatted.clear();
}

void GUI_DATA_TRANSMIT::receive_gui(QByteArray recvData)
{
    // Remove Major key, minor key, and byte length
    recvData.remove(0, s1_end_loc);

    // Insert into global array (for saving in original format)
    rcvd_formatted.append(recvData);

    // Insert plaintext at end
    QTextCursor prev_cursor = ui->recv_PlainText->textCursor();
    ui->recv_PlainText->moveCursor(QTextCursor::End);
    ui->recv_PlainText->insertPlainText(QString(recvData));
    ui->recv_PlainText->setTextCursor(prev_cursor);
}

void GUI_DATA_TRANSMIT::input_select(bool fileIN, bool plainIN)
{
    ui->FilePath_LineEdit->setEnabled(fileIN);
    ui->BrowseFile_Button->setEnabled(fileIN);
    ui->msg_PlainText->setEnabled(plainIN);
}
