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

#include "gui-custom-cmd.h"
#include "ui_gui-custom-cmd.h"

#include <QFile>
#include <QTextStream>

GUI_CUSTOM_CMD::GUI_CUSTOM_CMD(QWidget *parent) :
    GUI_BASE(parent),
    ui(new Ui::GUI_CUSTOM_CMD)
{
    ui->setupUi(this);
    guiType = GUI_TYPE_CUSTOM_CMD;

    // Set radio values
    ui->File_Radio->setChecked(true);
    on_cmdSelect_buttonClicked(0);

    // Set default base
    ui->MinorKeyBase_LineEdit->setText("16");
    ui->customCMDBase_LineEdit->setText("16");
}

GUI_CUSTOM_CMD::~GUI_CUSTOM_CMD()
{
    delete ui;
}

void GUI_CUSTOM_CMD::reset_gui()
{
    ui->Feedback_PlainText->clear();

    // Set radio values
    ui->File_Radio->setChecked(true);
    on_cmdSelect_buttonClicked(0);
}

void GUI_CUSTOM_CMD::on_SaveFeedback_Button_clicked()
{
    // Select file save location
    QString fileName;
    if (!GUI_HELPER::getSaveFilePath(&fileName))
        return;

    // Save file
    if (!GUI_HELPER::saveFile(fileName, ui->Feedback_PlainText->toPlainText().toUtf8()))
        GUI_HELPER::showMessage("ERROR: Failed to save file!");
}

void GUI_CUSTOM_CMD::on_BrowseFile_Button_clicked()
{
    // Select file to send
    QString file;
    if (GUI_HELPER::getOpenFilePath(&file))
        ui->FilePath_LineEdit->setText(file);
}

void GUI_CUSTOM_CMD::on_ClearFeedback_Button_clicked()
{
    ui->Feedback_PlainText->clear();
}

void GUI_CUSTOM_CMD::on_sendCustomCMD_Button_clicked()
{
    // Get bases for conversion
    uint8_t minorKeyBase = ui->MinorKeyBase_LineEdit->text().toUInt(nullptr, 10);
    uint8_t customCMDBase = ui->customCMDBase_LineEdit->text().toUInt(nullptr, 10);
    if (((minorKeyBase < 2) && (ui->MinorKeyBase_LineEdit->text() != "0"))
            || ((customCMDBase < 2) && (ui->customCMDBase_LineEdit->text() != "0")))
        return;

    // Select cmd based on radio
    if (ui->File_Radio->isChecked())
    {
        // Open file
        QFile customCMD_file(ui->FilePath_LineEdit->text());
        if (!customCMD_file.open(QIODevice::ReadOnly)) return;

        // Parse custom file line by line
        QStringList customCMD_line;
        QTextStream customCMD_stream(&customCMD_file);
        while (!customCMD_stream.atEnd())
        {
            // Read next line
            customCMD_line = customCMD_stream.readLine().split(" ");
            if (customCMD_line.length() == 0) continue;

            // First char is minor key
            send_custom_cmd(customCMD_line.takeFirst(), minorKeyBase,
                            customCMD_line, customCMDBase);
        }

        // Close file
        customCMD_file.close();
    } else if (ui->Manual_Radio->isChecked())
    {
        send_custom_cmd(ui->MinorKeyBase_LineEdit->text(), minorKeyBase, ui->customCMDBase_LineEdit->text().split(" "), customCMDBase);
    }
}

void GUI_CUSTOM_CMD::on_cmdSelect_buttonClicked(int)
{
    if (ui->File_Radio->isChecked())
        input_select(true, false);
    else if (ui->Manual_Radio->isChecked())
        input_select(false, true);
}

void GUI_CUSTOM_CMD::receive_data_transmit()
{
    ui->Feedback_PlainText->appendPlainText(QString(rcvd));
}

void GUI_CUSTOM_CMD::input_select(bool fileIN, bool manualIN)
{
    ui->FilePath_LineEdit->setEnabled(fileIN);
    ui->BrowseFile_Button->setEnabled(fileIN);
    ui->customCMD_PlainText->setEnabled(manualIN);
    ui->MinorKey_LineEdit->setEnabled(manualIN);
}

void GUI_CUSTOM_CMD::send_custom_cmd(QString minorKey_str, uint8_t minorKey_base, QStringList customCMD_lst, uint8_t customCMD_base)
{
    // Build custom command
    QByteArray data;
    data.append((char) guiType);

    // Read, parse, and add minor key
    if (minorKey_base == 0)
    {
        data.append((char) ui->MinorKey_LineEdit->text().toUtf8()[0]);
    } else
    {
        uint8_t minorKey = minorKey_str.toUInt(nullptr, minorKey_base);
        if ((minorKey == 0) && (minorKey_str != "0")) return;
        data.append((char) minorKey);
    }

    // Read, parse, and add custom CMD
    if (customCMD_base == 0)
    {
        QByteArray customCMD_bytes = customCMD_lst.join(" ").toUtf8();
        data.append((char) customCMD_bytes.length());
        data.append(customCMD_bytes);
    } else
    {
        data.append((char) customCMD_lst.length());
        QByteArray byte_num;
        foreach (QString data, customCMD_lst)
        {
            byte_num.setNum(data.toInt(nullptr, customCMD_base));
            data.append(byte_num);
        }
    }

    // Send command across
    send(data);
}
