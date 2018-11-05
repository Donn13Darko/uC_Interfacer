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

#include "gui-programmer.h"
#include "ui_gui-programmer.h"

GUI_PROGRAMMER::GUI_PROGRAMMER(QWidget *parent) :
    GUI_BASE(parent),
    ui(new Ui::GUI_PROGRAMMER)
{
    // Setup UI
    ui->setupUi(this);

    // Set GUI Type
    gui_key = MAJOR_KEY_PROGRAMMER;

    // Read config settings
    QMap<QString, QMap<QString, QVariant>*>* configMap = \
            GUI_HELPER::readConfigINI(":/user-interfaces/gui-programmer.ini");

    // Add configs to local maps
    QMap<QString, QVariant>* groupMap;
    if (configMap->contains("settings"))
    {
        groupMap = configMap->value("settings");
        addHexFormats(groupMap->value("hex_formats").toStringList());
        addBurnMethods(groupMap->value("burn_methods").toStringList());
    }

    // Delete map after use
    GUI_HELPER::deleteConfigMap(configMap);

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

void GUI_PROGRAMMER::parseConfigMap(QMap<QString, QVariant>* configMap)
{
    // Parse individual values
    addHexFormats(configMap->value("hex_formats").toStringList());
    removeHexFormats(configMap->value("hex_formats_rm").toStringList());
    addBurnMethods(configMap->value("burn_methods").toStringList());

    // Pass to parent for additional parsing
    GUI_BASE::parseConfigMap(configMap);
}

void GUI_PROGRAMMER::addHexFormats(QStringList hexFormatsMap)
{
    QStringList hexFormat;
    foreach (QString hexFormatString, hexFormatsMap)
    {
        // Parse input string
        hexFormat = hexFormatString.split('=');
        if (hexFormat.length() != 2
                || hexFormat[0].isEmpty()) continue;

        // Add hex format to combo and map
        ui->HexFormat_Combo->addItem(hexFormat[0]);
        hexFormats.insert(hexFormat[0], QRegularExpression(hexFormat[1]));
    }
}

void GUI_PROGRAMMER::removeHexFormats(QStringList hexFormatsList)
{
    int pos;
    foreach (QString hexFormat, hexFormatsList)
    {
        // Get hex format in combo
        pos = ui->HexFormat_Combo->findText(hexFormat);
        if (pos == -1) continue;

        // Remove hex format from combo and map
        ui->HexFormat_Combo->removeItem(pos);
        hexFormats.remove(hexFormat);
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

void GUI_PROGRAMMER::reset_gui()
{
    // Clear received data
    on_ReadDataClear_Button_clicked();

    // Clear entered data
    ui->HexFile_LineEdit->clear();
    ui->HexPreview_PlainText->clear();
    loadedHex.clear();

    // Reset radio selection
    ui->ReadAll_Radio->setChecked(true);
    on_ReadData_RadioGroup_buttonClicked(0);

    // Reset address range reading
    ui->ReadAddrLower_LineEdit->setText("0");
    ui->ReadAddrUpper_LineEdit->setText("0");
    ui->ReadAddrBase_LineEdit->setText("16");

    // Reset hex format selection
    ui->HexFormat_Combo->setCurrentIndex(0);
    curr_hexFormat = ui->HexFormat_Combo->currentText();
    on_HexFormat_Combo_activated(0);

    // Set checkboxes
    ui->HexPreview_CheckBox->setChecked(true);
    ui->ReadDataClearOnSet_CheckBox->setChecked(true);
    progress_divisor = 1;
    progress_adjuster = 0;

    // Reset base (resets send progress bar)
    GUI_BASE::reset_gui();
}

void GUI_PROGRAMMER::receive_gui(QByteArray recvData)
{
    // Get data without keys
    QByteArray data = recvData.mid(s1_end_loc);

    // See if this GUI sent CMD
    if (recvData.at(s1_major_key_loc) == (char) gui_key)
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
                set_expected_recv_length(GUI_HELPER::byteArray_to_uint32(data));
                return;
            }
            case MINOR_KEY_PROGRAMMER_DATA:
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

    // Insert into global array (for saving in original format)
    rcvd_formatted.append(data);

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

    if (progress_divisor == 1)
        ui->ProgrammerProgress_Label->setText(label);
}

void GUI_PROGRAMMER::set_progress_update_send(int progress, QString label)
{
    if (progress_divisor != 0)
        ui->Programmer_ProgressBar->setValue((progress_adjuster + progress) / progress_divisor);

    if (progress_divisor == 1)
        ui->ProgrammerProgress_Label->setText(label);
}

bool GUI_PROGRAMMER::isDataRequest(uint8_t minorKey)
{
    switch (minorKey)
    {
        case MINOR_KEY_PROGRAMMER_SET_ADDR:
        case MINOR_KEY_PROGRAMMER_DATA:
            return true;
        default:
            return GUI_BASE::isDataRequest(minorKey);
    }
}

void GUI_PROGRAMMER::on_BrowseHexFile_Button_clicked()
{
    // Select programmer file
    QString file;
    if (GUI_HELPER::getOpenFilePath(&file, tr("HEX (*.hex);;All Files (*.*)")))
    {
        // Set file text
        ui->HexFile_LineEdit->setText(file);

        // Load in file preview
        on_RefreshPreview_Button_clicked();
    }
}

void GUI_PROGRAMMER::on_RefreshPreview_Button_clicked()
{
    // Clear existing data
    loadedHex.clear();

    // Get file path
    QString filePath = ui->HexFile_LineEdit->text();
    if (filePath.isEmpty()) return;

    // Reload file if it exists
    loadedHex = GUI_HELPER::loadFile(filePath);

    // Reset preview
    refresh_hex();
}

void GUI_PROGRAMMER::on_BurnData_Button_clicked()
{
    // Verify that a file has been loaded
    if (loadedHex.isEmpty()) return;

    foreach (QString data_line, loadedHex.split('\n'))
    {
        // Clear all whitespace other than spaces
        data_line = data_line.simplified();

        // Remove all those spaces
        data_line.replace(" ", "");

        // Convert to bytes and send across (assumes data in hex)
        emit transmit_chunk(gui_key, MINOR_KEY_PROGRAMMER_DATA,
                            QByteArray::fromHex(data_line.toUtf8()));
    }
}

void GUI_PROGRAMMER::on_HexFormat_Combo_activated(int)
{
    // Check if selection is other and prompt for user input
    if (ui->HexFormat_Combo->currentText() == "Other")
    {
        QString hexRegexStr;
        QRegularExpression otherRegex = hexFormats["Other"];

        // Set to current other text
        hexRegexStr = otherRegex.pattern();

        // Get or update the text
        if (GUI_HELPER::getUserString(&hexRegexStr, "Other Hex Format", "Enter hex format regex:"))
        {
            // Only update regex if it changed
            if (otherRegex.pattern() != hexRegexStr)
            {
                otherRegex.setPattern(hexRegexStr);
                hexFormats.insert("Other", otherRegex);
            }
        } else
        {
            // Return to previous entry if change canceled
            ui->HexFormat_Combo->setCurrentText(curr_hexFormat);
            return;
        }
    }
    curr_hexFormat = ui->HexFormat_Combo->currentText();

    // Update regex string
    ui->HexRegex_PlainText->setPlainText(hexFormats.value(curr_hexFormat).pattern());
    ui->HexRegex_PlainText->moveCursor(QTextCursor::Start);

    // Refresh the preview
    refresh_hex();
}

void GUI_PROGRAMMER::on_BurnMethod_Combo_currentIndexChanged(int)
{
    ui->Instructions_PlainText->clear();
    ui->Instructions_PlainText->appendPlainText(burnMethods.value(ui->BurnMethod_Combo->currentText()));
    ui->Instructions_PlainText->moveCursor(QTextCursor::Start);
}

void GUI_PROGRAMMER::on_HexPreview_CheckBox_stateChanged(int)
{
    // If state changes, refresh the hex preview
    refresh_hex();
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
    rcvd_formatted.clear();
    set_expected_recv_length(0);
}

void GUI_PROGRAMMER::on_ReadData_Button_clicked()
{
    // FixMe - Request read operation here
}

void GUI_PROGRAMMER::on_ReadDataSave_Button_clicked()
{
    save_rcvd_formatted();
}

QString GUI_PROGRAMMER::format_hex(QByteArray rawHex)
{
    // Prepare loaded file for parsing
    QStringList hexList = QString(rawHex).split('\n');

    // Get regex matcher
    QRegularExpression hexReg = hexFormats.value(ui->HexFormat_Combo->currentText());

    QString final;
    QStringList captureList;
    foreach (QString i, hexList)
    {
        if (i.isEmpty()) continue;

        // Attempt to match with regex
        // Should be in format [nnaaaatt_dd_cc] ([NN Addr TT Data CC])
        // Transfer match to capture groups
        captureList = hexReg.match(i.trimmed()).capturedTexts();
        if (!captureList.isEmpty())
        {
            captureList.removeFirst();
            final += captureList.join(" ");
        }
        final += "\n";
    }

    return final;
}

void GUI_PROGRAMMER::refresh_hex()
{
    // Clear current data entry
    ui->HexPreview_PlainText->clear();

    // Only update format if file loaded
    if (loadedHex.isEmpty() || !ui->HexPreview_CheckBox->isChecked()) return;

    // Update Hex
    ui->HexPreview_PlainText->appendPlainText(format_hex(loadedHex));

    // Set cursor to top
    ui->HexPreview_PlainText->moveCursor(QTextCursor::Start);
}

