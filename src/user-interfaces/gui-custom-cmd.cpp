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
    ui->MajorKey_LineEdit->setText(QString::number(guiType));
    ui->KeyBase_LineEdit->setText("16");
    ui->customCMDBase_LineEdit->setText("16");

    // Connect signals
    connect(this, SIGNAL(readyRead()),
            this, SLOT(receive_custom_cmd()));
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
    uint8_t keyBase = ui->KeyBase_LineEdit->text().toUInt(nullptr, 10);
    uint8_t customCMDBase = ui->customCMDBase_LineEdit->text().toUInt(nullptr, 10);
    if (((keyBase < 2) && (ui->KeyBase_LineEdit->text() != "0"))
            || ((customCMDBase < 2) && (ui->customCMDBase_LineEdit->text() != "0")))
        return;

    // Select cmd based on radio
    if (ui->File_Radio->isChecked())
    {
        // Read entire file
        QByteArray customCMD_bytes = GUI_HELPER::loadFile(ui->FilePath_LineEdit->text());

        // Parse custom file line by line
        foreach (QByteArray customCMD_line, customCMD_bytes.split('\n'))
        {
            QList<QByteArray> customCMD = customCMD_line.split(' ');
            // Major key, Minor key, data bytes
            send_custom_cmd(
                            customCMD.takeFirst(),
                            customCMD.takeFirst(),
                            keyBase,
                            customCMD.join(' '),
                            customCMDBase
                        );
        }
    } else if (ui->Manual_Radio->isChecked())
    {
        send_custom_cmd(
                    ui->MajorKey_LineEdit->text(),
                    ui->MinorKey_LineEdit->text(),
                    keyBase,
                    ui->customCMDBase_LineEdit->text().toUtf8(),
                    customCMDBase
                );
    }
}

void GUI_CUSTOM_CMD::on_cmdSelect_buttonClicked(int)
{
    if (ui->File_Radio->isChecked())
        input_select(true, false);
    else if (ui->Manual_Radio->isChecked())
        input_select(false, true);
}

void GUI_CUSTOM_CMD::receive_custom_cmd()
{
    // Remove Major key, minor key, and byte length
    rcvd.remove(0, s1_end_loc);

    // Insert plaintext at end
    QTextCursor prev_cursor = ui->Feedback_PlainText->textCursor();
    ui->Feedback_PlainText->moveCursor(QTextCursor::End);
    ui->Feedback_PlainText->insertPlainText(QString(rcvd));
    ui->Feedback_PlainText->setTextCursor(prev_cursor);

    // Clear byte array
    rcvd.clear();
}

void GUI_CUSTOM_CMD::input_select(bool fileIN, bool manualIN)
{
    ui->FilePath_LineEdit->setEnabled(fileIN);
    ui->BrowseFile_Button->setEnabled(fileIN);
    ui->customCMD_PlainText->setEnabled(manualIN);
    ui->MinorKey_LineEdit->setEnabled(manualIN);
}

void GUI_CUSTOM_CMD::send_custom_cmd(QString majorKey_char, QString minorKey_char, uint8_t key_base, QByteArray customCMD_bytes, uint8_t customCMD_base)
{
    // Build custom command keys
    QByteArray keys;

    // Read, parse, and add Major/Minor keys
    if (key_base < 2)
    {
        // Add directly
        keys.append(majorKey_char.at(0).row());
        keys.append(minorKey_char.at(0).row());
    } else
    {
        // Convert keys to correct base
        uint8_t majorKey = majorKey_char.toInt(nullptr, key_base);
        uint8_t minorKey = minorKey_char.toInt(nullptr, key_base);

        // Add converted value
        keys.append((char) majorKey);
        keys.append((char) minorKey);
    }

    // Read, parse, and add custom CMD
    if (customCMD_base < 2)
    {
        send_chunk(keys, customCMD_bytes);
    } else
    {
        // Modify cmd bytes into correct format
        QByteArray byte_num, data;
        foreach (QByteArray cmd_byte, customCMD_bytes.split(' '))
        {
            byte_num.setNum(QString(cmd_byte).toInt(nullptr, customCMD_base));
            data.append(byte_num);
        }
        send_chunk(keys, data);
    }
}
