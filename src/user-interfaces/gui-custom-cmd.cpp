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
    // Setup UI
    ui->setupUi(this);

    // Set GUI Type
    guiType = GUI_TYPE_CUSTOM_CMD;

    // Reset GUI
    reset_gui();
}

GUI_CUSTOM_CMD::~GUI_CUSTOM_CMD()
{
    delete ui;
}

void GUI_CUSTOM_CMD::reset_gui()
{
    // Clear received data
    on_ClearFeedback_Button_clicked();

    // Set default entered values
    ui->MajorKey_LineEdit->setText(QString::number(guiType));
    ui->MinorKey_LineEdit->setText("0");
    ui->KeyBase_LineEdit->setText("10");
    ui->CustomCMDBase_LineEdit->setText("0");

    // Reset radio selection
    ui->File_Radio->setChecked(true);
    on_cmdSelect_buttonClicked(0);
}

void GUI_CUSTOM_CMD::on_SaveFeedback_Button_clicked()
{
    save_rcvd_formatted();
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
    rcvd_formatted.clear();
}

void GUI_CUSTOM_CMD::on_SendCustomCMD_Button_clicked()
{
    // Get bases for conversion
    uint8_t keyBase = ui->KeyBase_LineEdit->text().toUInt(nullptr, 10);
    uint8_t customCMDBase = ui->CustomCMDBase_LineEdit->text().toUInt(nullptr, 10);
    if (((keyBase < 2) && (ui->KeyBase_LineEdit->text() != "0"))
            || ((customCMDBase < 2) && (ui->CustomCMDBase_LineEdit->text() != "0")))
        return;

    // Select cmd based on radio
    if (ui->File_Radio->isChecked())
    {
        // Read entire file
        QByteArray customCMD_bytes = GUI_HELPER::loadFile(ui->FilePath_LineEdit->text());
        if (customCMD_bytes.length() == 0) return;

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
                    ui->CustomCMD_PlainText->toPlainText().toUtf8(),
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

void GUI_CUSTOM_CMD::receive_gui(QByteArray recvData)
{
    // Remove Major key, minor key, and byte length
    recvData.remove(0, s1_end_loc);

    // Insert into global array (for saving in original format)
    rcvd_formatted.append(recvData);

    // Insert plaintext at end
    QTextCursor prev_cursor = ui->Feedback_PlainText->textCursor();
    ui->Feedback_PlainText->moveCursor(QTextCursor::End);
    ui->Feedback_PlainText->insertPlainText(QString(recvData));
    ui->Feedback_PlainText->setTextCursor(prev_cursor);
}

void GUI_CUSTOM_CMD::input_select(bool fileIN, bool manualIN)
{
    ui->FilePath_LineEdit->setEnabled(fileIN);
    ui->BrowseFile_Button->setEnabled(fileIN);
    ui->MajorKey_LineEdit->setEnabled(manualIN);
    ui->MinorKey_LineEdit->setEnabled(manualIN);
    ui->CustomCMD_PlainText->setEnabled(manualIN);
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
    if ((customCMD_base < 2) || (customCMD_bytes.length() == 0))
    {
        send_chunk(keys, customCMD_bytes);
    } else
    {
        // Modify cmd bytes into correct format
        QByteArray data;
        foreach (QByteArray cmd_byte, customCMD_bytes.split(' '))
        {
            data.append((char) QString(cmd_byte).toInt(nullptr, customCMD_base));
        }
        send_chunk(keys, data);
    }
}
