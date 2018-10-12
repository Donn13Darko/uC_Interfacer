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
    ui->setupUi(this);
    guiType = GUI_TYPE_DATA_TRANSMIT;

    // Set radio values
    ui->File_Radio->setChecked(true);
    on_MSG_Sel_buttonClicked(0);
}

GUI_DATA_TRANSMIT::~GUI_DATA_TRANSMIT()
{
    delete ui;
}

void GUI_DATA_TRANSMIT::reset_gui()
{
    // Set radio values
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
    // Find which radio button selected
    if (ui->Input_Radio->isChecked())
        send(ui->msg_PlainText->toPlainText());
    else if (ui->File_Radio->isChecked())
        sendFile(ui->FilePathEdit->text());
}

void GUI_DATA_TRANSMIT::on_BrowseFile_Button_clicked()
{
    // Select file to send
    QString file;
    if (GUI_HELPER::getOpenFilePath(&file))
        ui->FilePathEdit->setText(file);
}

void GUI_DATA_TRANSMIT::on_SaveAs_Button_clicked()
{
    // Select file save location
    QString fileName;
    if (!GUI_HELPER::getSaveFilePath(&fileName))
        return;

    // Save file
    if (!GUI_HELPER::saveFile(fileName, received))
        GUI_HELPER::showMessage("ERROR: Failed to save file!");
}

void GUI_DATA_TRANSMIT::on_ClearReceived_Button_clicked()
{
    received.clear();
    ui->recv_PlainText->clear();
}

void GUI_DATA_TRANSMIT::receive_data_transmit()
{
    received.append(rcvd);
    ui->recv_PlainText->appendPlainText(QString(rcvd));
}

void GUI_DATA_TRANSMIT::input_select(bool fileIN, bool plainIN)
{
    ui->FilePathEdit->setEnabled(fileIN);
    ui->BrowseFile_Button->setEnabled(fileIN);
    ui->msg_PlainText->setEnabled(plainIN);
}
