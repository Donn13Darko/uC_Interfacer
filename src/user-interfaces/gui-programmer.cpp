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

#include "gui-programmer.hpp"
#include "ui_gui-programmer.h"

GUI_PROGRAMMER::GUI_PROGRAMMER(QWidget *parent) :
    GUI_BASE(parent),
    ui(new Ui::GUI_PROGRAMMER)
{
    // Setup UI
    ui->setupUi(this);

    // Set GUI Type & Default Name
    set_gui_key(MAJOR_KEY_PROGRAMMER);
    set_gui_name("Programmer");

    // Setup progress bars
    ui->Programmer_ProgressBar->setMinimum(0);
    ui->Programmer_ProgressBar->setMaximum(100);

    // Reset GUI
    reset_gui();
}

GUI_PROGRAMMER::~GUI_PROGRAMMER()
{
    delete ui;
}

void GUI_PROGRAMMER::parseConfigMap(QMap<QString, QVariant> *configMap)
{
    // Pass to parent for additional parsing
    GUI_BASE::parseConfigMap(configMap);

    // Parse individual values
    addFileFormats(configMap->value("file_formats").toStringList());
    addBurnMethods(configMap->value("burn_methods").toStringList());
}

void GUI_PROGRAMMER::addFileFormats(QStringList fileFormatsMap)
{
    uint8_t fileBase;
    QStringList fileFormat, header_split;
    foreach (QString fileFormatString, fileFormatsMap)
    {
        // Parse input string
        fileFormat = fileFormatString.split('=');
        if (fileFormat.length() < 2
                || fileFormat.at(0).isEmpty()) continue;

        // See if base encoded
        header_split = fileFormat.takeFirst().split(',');
        if (header_split.length() < 2) fileBase = 0;
        else fileBase = header_split.at(1).toInt(nullptr, 10);

        // Add file format to combo and map
        ui->FileFormat_Combo->addItem(header_split.at(0));
        fileFormats.insert(header_split.at(0),
                           QPair<uint8_t, QString>(fileBase, fileFormat.join('=')));
    }
}

void GUI_PROGRAMMER::addBurnMethods(QStringList burnMethodsMap)
{
    QStringList burnMethod;
    foreach (QString burnMethodString, burnMethodsMap)
    {
        // Parse input string
        burnMethod = burnMethodString.split('=');
        if (burnMethod.length() != 2) continue;

        // Add burn method to combo and map
        ui->BurnMethod_Combo->addItem(burnMethod[0]);
        burnMethods.insert(burnMethod[0], burnMethod[1]);
    }

    // Update instructions text
    on_BurnMethod_Combo_currentIndexChanged(0);
}

bool GUI_PROGRAMMER::waitForDevice(uint8_t minorKey)
{
    switch (minorKey)
    {
        case MINOR_KEY_PROGRAMMER_SET_INFO:
        case MINOR_KEY_PROGRAMMER_SET_ADDR:
        case MINOR_KEY_PROGRAMMER_DATA:
        case MINOR_KEY_PROGRAMMER_READ: // Stall while device is written to/read from (slow)
            return true;
        default:
            return GUI_BASE::waitForDevice(minorKey);
    }
}

void GUI_PROGRAMMER::reset_gui()
{
    // Reset base (resets send progress bar)
    GUI_BASE::reset_gui();

    // Clear received data
    on_ReadDataClear_Button_clicked();

    // Clear entered data
    ui->File_LineEdit->clear();
    ui->FilePreview_PlainText->clear();

    // Reset radio selection
    ui->ReadAll_Radio->setChecked(true);
    on_ReadData_RadioGroup_buttonClicked(0);

    // Reset address range reading
    ui->ReadAddrLower_LineEdit->setText("0");
    ui->ReadAddrUpper_LineEdit->setText("0");
    ui->ReadAddrBase_LineEdit->setText("16");

    // Reset file format selection
    ui->FileFormat_Combo->setCurrentIndex(0);
    curr_fileFormat = ui->FileFormat_Combo->currentText();
    on_FileFormat_Combo_activated(0);

    // Set checkboxes
    ui->FilePreview_CheckBox->setChecked(true);
    ui->ReadDataClearOnSet_CheckBox->setChecked(true);
    progress_divisor = 1;
    progress_adjuster = 0;
}

void GUI_PROGRAMMER::receive_gui(QByteArray recvData)
{
    // Verify length
    if (recvData.length() < s1_end_loc) return;

    // Get gui key
    uint8_t local_gui_key = get_gui_key();

    // Get data without keys
    QByteArray data = recvData.mid(s1_end_loc);

    // See if this GUI sent CMD
    if (recvData.at(s1_major_key_loc) == (char) local_gui_key)
    {
        // See if known CMD
        switch (recvData.at(s1_minor_key_loc))
        {
            case MINOR_KEY_PROGRAMMER_SET_TRANS_SIZE:
            {
                // Clear recv if clear on set checked
                if (ui->ReadDataClearOnSet_CheckBox->isChecked())
                    on_ReadDataClear_Button_clicked();

                // Set expected length
                set_expected_recv_length(GUI_GENERIC_HELPER::byteArray_to_uint32(data));
                return;
            }
            case MINOR_KEY_PROGRAMMER_DATA:
            {
                // Update current recv length with each packet
                update_current_recv_length(data.length());

                // Send data ready if not a signal packet
                if (!data.isEmpty())
                {
                    emit transmit_chunk(MAJOR_KEY_DEV_READY, 0);
                }
                break;
            }
            case MINOR_KEY_PROGRAMMER_SET_INFO:
            case MINOR_KEY_PROGRAMMER_SET_ADDR:
            case MINOR_KEY_PROGRAMMER_READ:
            {
                // Emit device ready
                emit transmit_chunk(MAJOR_KEY_DEV_READY, 0);

                // Exit
                return;
            }
        }
    } else
    {
        // Ignore any CMDs not meant for this GUI
        return;
    }

    // Insert into global array (for saving in original format)
    rcvd_formatted_append(data);

    // Insert at end of plaintext
    QTextCursor prev_cursor = ui->ReadData_PlainText->textCursor();
    ui->ReadData_PlainText->moveCursor(QTextCursor::End);
    ui->ReadData_PlainText->insertPlainText(QString(data));
    ui->ReadData_PlainText->setTextCursor(prev_cursor);
}

void GUI_PROGRAMMER::set_progress_update_recv(int progress, QString label)
{
    if (progress_divisor != 0)
        ui->Programmer_ProgressBar->setValue((progress_adjuster + progress) / progress_divisor);
    ui->ProgrammerRead_ProgressBar->setValue(progress);

    if (progress_divisor == 1)
        ui->ProgrammerProgress_Label->setText(label);
    ui->ProgrammerRead_Label->setText(label);
}

void GUI_PROGRAMMER::set_progress_update_send(int progress, QString label)
{
    if (progress_divisor != 0)
        ui->Programmer_ProgressBar->setValue((progress_adjuster + progress) / progress_divisor);

    if (progress_divisor == 1)
        ui->ProgrammerProgress_Label->setText(label);
}

Ui::GUI_PROGRAMMER *GUI_PROGRAMMER::get_ui()
{
    return ui;
}

void GUI_PROGRAMMER::on_BrowseFile_Button_clicked()
{
    // Select programmer file
    QString file;
    if (GUI_GENERIC_HELPER::getOpenFilePath(&file,
                                    tr("HEX (*.hex);;"
                                       "BIN (*.bin);;"
                                       "SREC (*.s* *.mot *.mxt);;"
                                       "All Files (*)")))
    {
        // Set file text
        ui->File_LineEdit->setText(file);

        // Load in file preview
        on_RefreshPreview_Button_clicked();
    }
}

void GUI_PROGRAMMER::on_RefreshPreview_Button_clicked()
{
    // Reset preview
    refresh_file();
}

void GUI_PROGRAMMER::on_BurnData_Button_clicked()
{
    // Get format information pair
    QPair<uint8_t, QString> fileFormat = fileFormats.value(ui->FileFormat_Combo->currentText());

    // Get filePath
    QString filePath = ui->File_LineEdit->text();
    if (filePath.isEmpty())
    {
        // Shoe error if file path empty
        emit progress_update_send(0, "Error: No file provided!");
        return;
    }

    // Create programming info bytearray
    QByteArray info;
    info.append((uint8_t) ui->FileFormat_Combo->currentIndex());
    info.append((uint8_t) ui->BurnMethod_Combo->currentIndex());

    // Get gui key
    uint8_t local_gui_key = get_gui_key();

    // Set programming attributes
    emit transmit_chunk(local_gui_key, MINOR_KEY_PROGRAMMER_SET_INFO, info);

    // Get file size
    qint64 fileSize = GUI_GENERIC_HELPER::getFileSize(filePath);
    if (fileSize < 0) return;

    // Set file size
    emit transmit_chunk(local_gui_key, MINOR_KEY_PROGRAMMER_SET_TRANS_SIZE,
                        GUI_GENERIC_HELPER::uint32_to_byteArray((uint32_t) fileSize));

    // Send file
    emit transmit_file_pack(local_gui_key, MINOR_KEY_PROGRAMMER_DATA,
                            filePath, fileFormat.first,
                            fileFormat.second);
}

void GUI_PROGRAMMER::on_FileFormat_Combo_activated(int)
{
    // Check if selection is other and prompt for user input
    if (ui->FileFormat_Combo->currentText() == "Other")
    {
        // Get current information
        QPair<uint8_t, QString> otherPair = fileFormats["Other"];
        QString fileRegexStr = otherPair.second;

        // Get or update the text
        if (GUI_GENERIC_HELPER::getUserString(&fileRegexStr, "Other Format", "Enter format as 'base,regex':"))
        {
            // Parse input string
            QStringList input_str = fileRegexStr.split(',');

            // Get base
            uint8_t base = 0;
            if (1 < input_str.length())
                base = input_str.takeFirst().toInt(nullptr, 10);
            fileRegexStr = input_str.join(',');

            // Only update regex if it changed
            if ((otherPair.first != base)
                    && (otherPair.second != fileRegexStr))
            {
                fileFormats.insert("Other", QPair<uint8_t, QString>(base, fileRegexStr));
            }
        } else
        {
            // Return to previous entry if change canceled
            ui->FileFormat_Combo->setCurrentText(curr_fileFormat);
            return;
        }
    }
    curr_fileFormat = ui->FileFormat_Combo->currentText();

    // Update regex string
    ui->FileRegex_PlainText->setPlainText(fileFormats.value(curr_fileFormat).second);
    ui->FileRegex_PlainText->moveCursor(QTextCursor::Start);

    // Refresh the preview
    refresh_file();
}

void GUI_PROGRAMMER::on_BurnMethod_Combo_currentIndexChanged(int)
{
    ui->Instructions_PlainText->clear();
    ui->Instructions_PlainText->appendPlainText(burnMethods.value(ui->BurnMethod_Combo->currentText()));
    ui->Instructions_PlainText->moveCursor(QTextCursor::Start);
}

void GUI_PROGRAMMER::on_FilePreview_CheckBox_stateChanged(int)
{
    // If state changes, refresh the file preview
    refresh_file();
}

void GUI_PROGRAMMER::on_ReadData_RadioGroup_buttonClicked(int)
{
    bool enable_addr = ui->ReadAddr_Radio->isChecked();
    ui->ReadAddrLower_LineEdit->setEnabled(enable_addr);
    ui->ReadAddrUpper_LineEdit->setEnabled(enable_addr);
    ui->ReadAddrBase_LineEdit->setEnabled(enable_addr);
}

void GUI_PROGRAMMER::on_ReadDataClear_Button_clicked()
{
    ui->ReadData_PlainText->clear();
    rcvd_formatted_clear();
    set_expected_recv_length(0);
}

void GUI_PROGRAMMER::on_ReadData_Button_clicked()
{
    // Setup read info
    QByteArray info;

    // Check if reading address range
    bool ok;
    int base, addr;
    if (ui->ReadAddr_Radio->isChecked())
    {
        // Get base value
        base = ui->ReadAddrBase_LineEdit->text().toInt(&ok, 10);
        if (!ok) return;

        // Get lower address and add to info
        addr = ui->ReadAddrLower_LineEdit->text().toInt(&ok, base);
        if (!ok) return;
        if (addr < 0) addr = 0;
        info.append(GUI_GENERIC_HELPER::uint32_to_byteArray((uint32_t) addr));

        // Get upper address and add to info
        addr = ui->ReadAddrUpper_LineEdit->text().toInt(&ok, base);
        if (!ok) return;
        if (0 <= addr)
        {
            info.append(GUI_GENERIC_HELPER::uint32_to_byteArray((uint32_t) addr));
        }
    }

    // Send read with info
    emit transmit_chunk(get_gui_key(), MINOR_KEY_PROGRAMMER_READ, info);
}

void GUI_PROGRAMMER::on_ReadDataSave_Button_clicked()
{
    rcvd_formatted_save();
}

QByteArray GUI_PROGRAMMER::format_file(QByteArray rawFile)
{
    // Get format information pair
    QPair<uint8_t, QString> fileFormat = fileFormats.value(ui->FileFormat_Combo->currentText());

    // If base is zero return rawFile bytes
    if (fileFormat.first == 0) return rawFile;

    // Prepare loaded file for parsing
    QList<QByteArray> fileList = rawFile.split('\n');

    // Setup variables for loop
    QByteArray final;
    QStringList captureList;
    QRegularExpression fileRegex = QRegularExpression(fileFormat.second,
                                                      QRegularExpression::DotMatchesEverythingOption);

    // Loop through each line looking for a match
    foreach (QByteArray i, fileList)
    {
        // Skip blank lines
        if (i.isEmpty()) continue;

        // Attempt to match with regex group
        captureList = fileRegex.match(i.trimmed()).capturedTexts();
        if (!captureList.isEmpty())
        {
            // First group is entire line
            captureList.removeFirst();
            final += captureList.join(" ") + '\n';
        }
    }

    return final;
}

void GUI_PROGRAMMER::refresh_file()
{
    // Clear current data entry
    ui->FilePreview_PlainText->clear();
    QString filePath = ui->File_LineEdit->text();

    // Only update format if selected && file < 10MB (10485760 Bytes)
    if (!ui->FilePreview_CheckBox->isChecked()
            || (10485760 < GUI_GENERIC_HELPER::getFileSize(filePath)))
    {
        return;
    }

    // Update File
    ui->FilePreview_PlainText->appendPlainText(format_file(GUI_GENERIC_HELPER::loadFile(filePath)));

    // Set cursor to top
    ui->FilePreview_PlainText->moveCursor(QTextCursor::Start);
}

