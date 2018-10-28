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

void GUI_PROGRAMMER::reset_gui()
{
    // Clear received data
    on_ClearReadData_Button_clicked();

    // Clear entered data
    ui->HexFile_LineEdit->clear();
    ui->HexPreview_PlainText->clear();
    loadedHex.clear();

    // Reset radio selection
    ui->ReadAll_Radio->setChecked(true);
    on_readSelect_buttonClicked(0);

    // Reset hex format selection
    ui->HexFormat_Combo->setCurrentIndex(0);
    curr_hexFormat = ui->HexFormat_Combo->currentText();
    on_HexFormat_Combo_activated(0);

    // Set clear on set
    ui->ClearOnSet_CheckBox->setChecked(true);
    progress_divisor = 1;
    progress_adjuster = 0;

    // Reset base (resets send progress bar)
    GUI_BASE::reset_gui();
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
                // Clear recv if clear on set checked
                if (ui->ClearOnSet_CheckBox->isChecked())
                    on_ClearReadData_Button_clicked();

                // Set expected length
                set_expected_recv_length(data);
                return;
            case MINOR_KEY_PROGRAMMER_DATA:
                // Update current recv length with each packet
                update_current_recv_length(data);
                break;
        }
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
        case MINOR_KEY_PROGRAMMER_READ_ALL:
        case MINOR_KEY_PROGRAMMER_READ_ADDR:
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
    // Get file path
    QString filePath = ui->HexFile_LineEdit->text();
    if (filePath.isEmpty()) return;

    // Reload file if it exists
    loadedHex = GUI_HELPER::loadFile(filePath);

    // Reset preview
    on_HexFormat_Combo_activated(0);
}

void GUI_PROGRAMMER::on_BurnData_Button_clicked()
{
    //  [NN Addr TT Data CC]
    QByteArray data;
    QStringList curr;
    uint8_t prog_line_length;
    QStringList formattedHexList = ui->HexPreview_PlainText->toPlainText().split('\n');
    foreach (QString i, formattedHexList)
    {
        curr = i.split(' ');
        if (curr.isEmpty() || (curr.length() < 5)) continue;

        // Send across address
        if (!curr[3].isEmpty() && (curr[3].length() == 4))
        {
            // Send Packet #1
            send({
                     gui_key,
                     2
                 });

            // Send Packet #2
            data.clear();
            data.append((char) MINOR_KEY_PROGRAMMER_SET_ADDR);
            data.append(QByteArray::fromHex(curr[3].toUtf8()));
            send(data);
        }

        // Send across data
        if (!curr[4].isEmpty() && (1 < curr[4].length()))
        {
            // Calculate command length
            prog_line_length = curr[4].length();
            prog_line_length = (prog_line_length / 2) + (prog_line_length % 2);

            // Send Package
            data.clear();
            data.append((char) gui_key);
            data.append((char) MINOR_KEY_PROGRAMMER_DATA);
            data.append((char) prog_line_length);
            data.append(QByteArray::fromHex(curr[4].toUtf8()));
            send(data);
        }
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
            if (otherRegex.pattern() == hexRegexStr) return;
            otherRegex.setPattern(hexRegexStr);
            hexFormats.insert("Other", otherRegex);
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

    // Only update format if file loaded
    if (loadedHex.isEmpty()) return;

    // Update Hex
    ui->HexPreview_PlainText->clear();
    ui->HexPreview_PlainText->appendPlainText(format_hex(loadedHex));

    // Set cursor to top
    ui->HexPreview_PlainText->moveCursor(QTextCursor::Start);
}

void GUI_PROGRAMMER::on_BurnMethod_Combo_currentIndexChanged(int)
{
    ui->Instructions_PlainText->clear();
    ui->Instructions_PlainText->appendPlainText(burnMethods.value(ui->BurnMethod_Combo->currentText()));
    ui->Instructions_PlainText->moveCursor(QTextCursor::Start);
}

void GUI_PROGRAMMER::on_readSelect_buttonClicked(int)
{
    ui->ReadAddr_Edit->setEnabled(ui->ReadAddr_Radio->isChecked());
}

void GUI_PROGRAMMER::on_ClearReadData_Button_clicked()
{
    ui->ReadData_PlainText->clear();
    rcvd_formatted.clear();
    set_expected_recv_length(0);
}

void GUI_PROGRAMMER::on_ReadData_Button_clicked()
{
    // FixMe - Request read operation here
}

void GUI_PROGRAMMER::on_SaveReadData_Button_clicked()
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
        // Should be in format [nnaaaatt_dd_cc]
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

