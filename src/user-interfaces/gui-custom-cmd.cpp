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

    // Set GUI Type & Default Name
    gui_key = MAJOR_KEY_CUSTOM_CMD;
    gui_name = "GUI Custom CMD";

    // Setup Progress bars
    ui->CustomCMD_ProgressBar->setMinimum(0);
    ui->CustomCMD_ProgressBar->setMaximum(100);
    ui->Feedback_ProgressBar->setMinimum(0);
    ui->Feedback_ProgressBar->setMaximum(100);

    // Reset GUI
    reset_gui();
}

GUI_CUSTOM_CMD::~GUI_CUSTOM_CMD()
{
    delete ui;
}

bool GUI_CUSTOM_CMD::acceptAllCMDs()
{
    return ui->FeedbackLogAllCMDs_CheckBox->isChecked();
}

void GUI_CUSTOM_CMD::parseConfigMap(QMap<QString, QVariant> *configMap)
{
    // Pass to parent for additional parsing
    GUI_BASE::parseConfigMap(configMap);
}

void GUI_CUSTOM_CMD::reset_gui()
{
    // Reset base (resets progress bar)
    GUI_BASE::reset_gui();

    // Clear received data
    on_FeedbackClear_Button_clicked();

    // Reset recv bases
    recv_key_base = 16;
    recv_cmd_base = 16;

    // Reset send bases
    send_key_base = 16;
    send_cmd_base = 16;

    // Set default entered values
    ui->CustomCMDMajorKey_LineEdit->setText(QString::number(gui_key, send_key_base));
    ui->CustomCMDMinorKey_LineEdit->setText(QString::number(MINOR_KEY_CUSTOM_CMD_CMD, send_key_base));
    ui->CustomCMDBase_LineEdit->setText(QString::number(send_cmd_base, 10));
    ui->CustomCMDKeyBase_LineEdit->setText(QString::number(send_key_base, 10));
    ui->CustomCMD_PlainText->clear();
    ui->CustomCMDFilePath_LineEdit->clear();

    // Reset radio selections
    ui->CustomCMDFile_Radio->setChecked(true);
    ui->CustomCMDKeysInInput_CheckBox->setChecked(false);
    on_CustomCMD_RadioGroup_buttonClicked(0);

    // Set Feedback checkboxes
    ui->FeedbackLogAllCMDs_CheckBox->setChecked(false);
    ui->FeedbackClearOnSet_CheckBox->setChecked(true);
    ui->FeedbackAppendNewline_CheckBox->setChecked(true);
}

void GUI_CUSTOM_CMD::receive_gui(QByteArray recvData)
{
    // See if this GUI sent CMD
    bool updateCMD_base = false;
    if (recvData.at(s1_major_key_loc) == (char) gui_key)
    {
        // See if known CMD
        switch (recvData.at(s1_minor_key_loc))
        {
            case MINOR_KEY_CUSTOM_CMD_SET_TRANS_SIZE:
            {
                // Clear recv if clear on set checked
                if (ui->FeedbackClearOnSet_CheckBox->isChecked())
                    on_FeedbackClear_Button_clicked();

                // Set expected length
                set_expected_recv_length(GUI_HELPER::byteArray_to_uint32(recvData.mid(s1_end_loc)));
                return;
            }
            case MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE:
            {
                updateCMD_base = true;
            }
            case MINOR_KEY_CUSTOM_CMD_CMD:
            {
                // Update current recv length with each packet
                update_current_recv_length(recvData.mid(s1_end_loc).length());

                if (recvData.length() == 2)
                {
                    // If end of CMD is newline exit
                    if (rcvd_formatted.endsWith('\n') || rcvd_formatted.isEmpty()) return;

                    // Insert newline into class array
                    rcvd_formatted.append('\n');

                    // Insert newline at end of plaintext
                    QTextCursor prev_cursor = ui->Feedback_PlainText->textCursor();
                    ui->Feedback_PlainText->moveCursor(QTextCursor::End);
                    ui->Feedback_PlainText->insertPlainText(QString('\n'));
                    ui->Feedback_PlainText->setTextCursor(prev_cursor);
                    return;
                }
                break;
            }
        }
    } else
    {
        // If not log all CMDs set, return (will only log GUI_CUSTOM_CMD cmds)
        if (!ui->FeedbackLogAllCMDs_CheckBox->isChecked()) return;
    }

    // Parse key pair and data based on bases
    QString recvPlain = GUI_HELPER::encode_byteArray(recvData.left(s1_end_loc), recv_key_base, ' ');
    recvPlain += GUI_HELPER::encode_byteArray(recvData.mid(s1_end_loc), recv_cmd_base, ' ');

    // Append newline if required
    if (ui->FeedbackAppendNewline_CheckBox->isChecked() && !recvPlain.endsWith('\n'))
        recvPlain.append('\n');

    // Insert into class array (for saving in sent format)
    rcvd_formatted.append(recvPlain.toLatin1());

    // Insert at end of plaintext
    QTextCursor prev_cursor = ui->Feedback_PlainText->textCursor();
    ui->Feedback_PlainText->moveCursor(QTextCursor::End);
    ui->Feedback_PlainText->insertPlainText(recvPlain);
    ui->Feedback_PlainText->setTextCursor(prev_cursor);

    // Update cmd base if value set
    if (updateCMD_base)
    {
        recv_key_base = recvData.at(s1_end_loc);
        recv_cmd_base = recvData.at(s1_end_loc+1);
    }
}

void GUI_CUSTOM_CMD::set_progress_update_recv(int progress, QString label)
{
    ui->Feedback_ProgressBar->setValue(progress);
    ui->FeedbackProgress_Label->setText(label);
}

void GUI_CUSTOM_CMD::set_progress_update_send(int progress, QString label)
{
    ui->CustomCMD_ProgressBar->setValue(progress);
    ui->CustomCMDProgress_Label->setText(label);
}

void GUI_CUSTOM_CMD::on_FeedbackSave_Button_clicked()
{
    save_rcvd_formatted();
}

void GUI_CUSTOM_CMD::on_FeedbackClear_Button_clicked()
{
    ui->Feedback_PlainText->clear();
    rcvd_formatted.clear();
    set_expected_recv_length(0);
}

void GUI_CUSTOM_CMD::on_CustomCMDKeysInInput_CheckBox_stateChanged(int)
{
    if (ui->CustomCMDKeysInInput_CheckBox->isChecked()
            || ui->CustomCMDFile_Radio->isChecked())
    {
        ui->CustomCMDMajorKey_LineEdit->setEnabled(false);
        ui->CustomCMDMinorKey_LineEdit->setEnabled(false);
    } else
    {
        ui->CustomCMDMajorKey_LineEdit->setEnabled(true);
        ui->CustomCMDMinorKey_LineEdit->setEnabled(true);
    }
}

void GUI_CUSTOM_CMD::on_CustomCMDBrowseFile_Button_clicked()
{
    // Select file to send
    QString file;
    if (GUI_HELPER::getOpenFilePath(&file))
        ui->CustomCMDFilePath_LineEdit->setText(file);
}

void GUI_CUSTOM_CMD::on_CustomCMDSend_Button_clicked()
{
    // Get default key start base for conversion
    uint8_t ui_setBase_num;
    QString ui_setBase = ui->CustomCMDKeyBase_LineEdit->text();
    if (!ui_setBase.isEmpty())
    {
        ui_setBase_num = ui_setBase.toInt(nullptr, 10);
        if ((1 < ui_setBase_num) || (ui_setBase == "0"))
        {
            send_key_base = ui_setBase_num;
        }
    }

    // Get default cmd start base for conversion
    ui_setBase = ui->CustomCMDBase_LineEdit->text();
    if (!ui_setBase.isEmpty())
    {
        ui_setBase_num = ui_setBase.toInt(nullptr, 10);
        if ((1 < ui_setBase_num) || (ui_setBase == "0"))
        {
            send_cmd_base = ui_setBase_num;
        }
    }

    // Transmit cmd base at start
    // (Have other guis ignore CMDs that don't have the relevant gui_key as major_key)
    send_chunk(gui_key, MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE,
                {send_key_base, send_cmd_base});

    // Setup variables
    bool parseKeys = false;
    QString customCMD_bytes;
    QString major_key_str, minor_key_str;

    // Select cmd based on radio
    if (ui->CustomCMDFile_Radio->isChecked())
    {
        // Read entire file
        customCMD_bytes = QString(GUI_HELPER::loadFile(ui->CustomCMDFilePath_LineEdit->text()));

        // See if any data
        if (customCMD_bytes.length() == 0) return;

        // Set parse keys from data
        parseKeys = true;
    } else if (ui->CustomCMDManual_Radio->isChecked())
    {
        // Get custom cmd bytes
        customCMD_bytes = ui->CustomCMD_PlainText->toPlainText();

        // Set parse keys variable
        parseKeys = ui->CustomCMDKeysInInput_CheckBox->isChecked();

        // Get keys if not parsing from CMD
        if (!parseKeys)
        {
            major_key_str = ui->CustomCMDMajorKey_LineEdit->text();
            minor_key_str = ui->CustomCMDMinorKey_LineEdit->text();
        }
    }

    // Parse custom file line by line
    foreach (QString customCMD_line, customCMD_bytes.split('\n'))
    {
        // If keys inside cmd line, split and parse the command line
        if (parseKeys)
        {
            // Split the command line
            QStringList customCMD = customCMD_line.split(' ', QString::KeepEmptyParts);

            // Ignore malformed commands
            if (customCMD.length() < 2) continue;

            // Grab keys from files
            major_key_str = customCMD.takeFirst();
            minor_key_str = customCMD.takeFirst();

            // Reassemble cmd without keys
            customCMD_line = customCMD.join(' ');
        }

        // Major key, Minor key, data bytes
        send_custom_cmd(major_key_str,
                        minor_key_str,
                        customCMD_line
                        );
    }
}

void GUI_CUSTOM_CMD::on_CustomCMD_RadioGroup_buttonClicked(int)
{
    if (ui->CustomCMDFile_Radio->isChecked())
        input_select(true, false);
    else if (ui->CustomCMDManual_Radio->isChecked())
        input_select(false, true);

    on_CustomCMDKeysInInput_CheckBox_stateChanged(0);
}

void GUI_CUSTOM_CMD::input_select(bool fileIN, bool manualIN)
{
    ui->CustomCMDFilePath_LineEdit->setEnabled(fileIN);
    ui->CustomCMDBrowseFile_Button->setEnabled(fileIN);

    ui->CustomCMD_PlainText->setEnabled(manualIN);
    ui->CustomCMDKeysInInput_CheckBox->setEnabled(manualIN);
}

void GUI_CUSTOM_CMD::send_custom_cmd(QString majorKey_char, QString minorKey_char, QString customCMD_bytes)
{
    // Build custom command keys
    uint8_t major_key, minor_key;

    // Read, parse, and add Major/Minor keys
    if (send_key_base < 2)
    {
        // Add directly
        major_key = majorKey_char.at(0).cell();
        minor_key = minorKey_char.at(0).cell();
    } else
    {
        // Convert keys to correct base
        major_key = majorKey_char.toInt(nullptr, send_key_base);
        minor_key = minorKey_char.toInt(nullptr, send_key_base);
    }

    // Check if line is key settings & update locals if so
    uint8_t send_cmd_base_old = send_cmd_base;
    switch (major_key)
    {
        case MAJOR_KEY_CUSTOM_CMD:
        {
            switch (minor_key)
            {
                case MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE:
                {
                    // Split bases and verify length
                    QStringList bases = customCMD_bytes.split(' ');
                    if (2 < bases.length()) return;

                    // Parse new bases (uses old cmd_base)
                    uint8_t send_key_base_new = bases.at(0).toInt(nullptr, send_cmd_base);
                    uint8_t send_cmd_base_new = bases.at(1).toInt(nullptr, send_cmd_base);

                    // Only set and send if new values (removes duplicate sends)
                    if ((send_key_base_new == send_key_base)
                            && (send_cmd_base_new == send_cmd_base))
                    {
                        return;
                    } else
                    {
                        send_key_base = send_key_base_new;
                        send_cmd_base = send_cmd_base_new;
                    }
                    break;
                }
                break;
            }
        }
    }

    // Send CMD
    emit transmit_chunk(major_key, minor_key,
                        customCMD_bytes.toLatin1(), send_cmd_base_old,
                        "^(.*?) (.*?)[ (.*?)]*\\w");
}
