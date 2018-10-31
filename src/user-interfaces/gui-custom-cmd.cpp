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
    gui_key = MAJOR_KEY_CUSTOM_CMD;

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

void GUI_CUSTOM_CMD::reset_gui()
{
    // Clear received data
    on_FeedbackClear_Button_clicked();

    // Set default entered values
    ui->CustomCMDMajorKey_LineEdit->setText(QString::number(gui_key, 16));
    ui->CustomCMDMinorKey_LineEdit->setText("2");
    ui->CustomCMDKeyBase_LineEdit->setText("16");
    ui->CustomCMDBase_LineEdit->setText("16");
    ui->CustomCMD_PlainText->clear();
    ui->CustomCMDFilePath_LineEdit->clear();

    // Reset radio selections
    ui->CustomCMDFile_Radio->setChecked(true);
    ui->CustomCMDKeysInInput_CheckBox->setChecked(false);
    on_CustomCMD_RadioGroup_buttonClicked(0);

    // Set clear on set
    ui->FeedbackClearOnSet_CheckBox->setChecked(true);

    // Reset base (resets progress bar)
    GUI_BASE::reset_gui();
}

void GUI_CUSTOM_CMD::parseConfigMap(QMap<QString, QVariant>* configMap)
{
    // Pass to parent for additional parsing
    GUI_BASE::parseConfigMap(configMap);
}

void GUI_CUSTOM_CMD::receive_gui(QByteArray recvData)
{
    // See if this GUI sent CMD
    if (recvData.at(s1_major_key_loc) == (char) gui_key)
    {
        // See if known CMD
        switch (recvData.at(s1_minor_key_loc))
        {
            case MINOR_KEY_CUSTOM_CMD_SET_TRANS_SIZE:
                // Clear recv if clear on set checked
                if (ui->FeedbackClearOnSet_CheckBox->isChecked())
                    on_FeedbackClear_Button_clicked();

                // Set expected length
                set_expected_recv_length(recvData.mid(s1_end_loc));
                return;
            case MINOR_KEY_CUSTOM_CMD_CMD:
                // Update current recv length with each packet
                update_current_recv_length(recvData.mid(s1_end_loc));

                // Check if end or start cmd
                if (recvData.length() == 2)
                {
                    // If end of CMD, append a newline
                    if (start_data) rcvd_formatted.append('\n');

                    // Insert \n at end of plaintext
                    QTextCursor prev_cursor = ui->Feedback_PlainText->textCursor();
                    ui->Feedback_PlainText->moveCursor(QTextCursor::End);
                    ui->Feedback_PlainText->insertPlainText(QString('\n'));
                    ui->Feedback_PlainText->setTextCursor(prev_cursor);

                    // Exit (this is transparently added by send_chunk
                    // and shouldn't be used in normal operation)
                    return;
                }
                break;
        }
    }

    // Highlight keypair
    QString recvPlain = GUI_HELPER::byteArray_to_string(recvData);

    // Insert into class array (for saving in sent format)
    rcvd_formatted.append(recvPlain.toUtf8());

    // Insert at end of plaintext
    QTextCursor prev_cursor = ui->Feedback_PlainText->textCursor();
    ui->Feedback_PlainText->moveCursor(QTextCursor::End);
    ui->Feedback_PlainText->insertPlainText(recvPlain);
    ui->Feedback_PlainText->setTextCursor(prev_cursor);
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
    // Get bases for conversion
    uint8_t keyBase = ui->CustomCMDKeyBase_LineEdit->text().toUInt(nullptr, 10);
    uint8_t customCMDBase = ui->CustomCMDBase_LineEdit->text().toUInt(nullptr, 10);
    if (((keyBase < 2) && (ui->CustomCMDKeyBase_LineEdit->text() != "0"))
            || ((customCMDBase < 2) && (ui->CustomCMDBase_LineEdit->text() != "0")))
    {
        return;
    }

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
        // Cleanup the command line
        customCMD_line = customCMD_line.simplified();

        // If keys inside cmd line, split and parse the command line
        if (parseKeys)
        {
            // Split the command line
            QStringList customCMD = customCMD_line.split(' ');

            // Ignore malformed commands
            if (customCMD.length() < 3) continue;

            // Grab keys from files
            major_key_str = customCMD.takeFirst();
            minor_key_str = customCMD.takeFirst();

            // Reassemble cmd without keys
            customCMD_line = customCMD.join(' ');
        }

        // Major key, Minor key, data bytes
        send_custom_cmd(major_key_str,
                        minor_key_str,
                        keyBase,
                        customCMD_line,
                        customCMDBase
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

void GUI_CUSTOM_CMD::send_custom_cmd(QString majorKey_char, QString minorKey_char, uint8_t key_base, QString customCMD_bytes, uint8_t customCMD_base)
{
    // Build custom command keys
    uint8_t major_key, minor_key;

    // Read, parse, and add Major/Minor keys
    if (key_base < 2)
    {
        // Add directly
        major_key = majorKey_char.at(0).row();
        minor_key = minorKey_char.at(0).row();
    } else
    {
        // Convert keys to correct base
        major_key = majorKey_char.toInt(nullptr, key_base);
        minor_key = minorKey_char.toInt(nullptr, key_base);
    }

    // Read, parse, and add custom CMD
    if ((customCMD_base < 2) || (customCMD_bytes.length() == 0))
    {
        send_chunk(major_key, minor_key, customCMD_bytes.toUtf8(), true);
    } else
    {
        send_chunk(major_key,
                   minor_key,
                   GUI_HELPER::string_to_byteArray(customCMD_bytes, ' ', customCMD_base),
                   true);
    }
}
