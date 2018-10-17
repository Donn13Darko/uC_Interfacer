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
#include "gui-programmer-minor-keys.h"

GUI_PROGRAMMER::GUI_PROGRAMMER(QWidget *parent) :
    GUI_BASE(parent),
    ui(new Ui::GUI_PROGRAMMER)
{
    // Init UI and variables
    ui->setupUi(this);
    loadedHex = QByteArray();
    curr_hexFormat = 0;
    guiType = GUI_TYPE_PROGRAMMER;

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

    // Setup UI for user
    ui->ReadAll_Radio->setChecked(true);
    on_readSelect_buttonClicked(0);
}

GUI_PROGRAMMER::~GUI_PROGRAMMER()
{
    delete ui;
}

void GUI_PROGRAMMER::reset_gui()
{
    ui->HexFile_LineEdit->setText("");
    ui->HexPreview_Edit->clear();
    ui->HexPreview_Edit->appendPlainText("");
    ui->ReadData_Edit->clear();
    ui->ReadData_Edit->appendPlainText("");
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
    QByteArray data;
    QStringList curr;
    uint8_t prog_line_length;
    QStringList formattedHexList = ui->HexPreview_Edit->toPlainText().split('\n');
    foreach (QString i, formattedHexList)
    {
        curr = i.split(' ');
        if (curr.isEmpty() || (curr.length() < 5)) continue;

        // Send across address
        if (!curr[3].isEmpty() && (curr[3].length() == 4))
        {
            // Send Packet #1
            send({
                     GUI_TYPE_PROGRAMMER,
                     2
                 });

            // Send Packet #2
            data.clear();
            data.append((char) MINOR_KEY_PROGRAMMER_ADDR);
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
            data.append((char) GUI_TYPE_PROGRAMMER);
            data.append((char) prog_line_length);
            data.append((char) MINOR_KEY_PROGRAMMER_DATA);
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
            ui->HexFormat_Combo->setCurrentIndex(curr_hexFormat);
            return;
        }
    }
    curr_hexFormat = ui->HexFormat_Combo->currentIndex();

    // Only update format if file loaded
    if (loadedHex.isEmpty()) return;

    // Update Hex
    ui->HexPreview_Edit->clear();
    ui->HexPreview_Edit->appendPlainText(format_hex(loadedHex));

    // Set cursor to top
    ui->HexPreview_Edit->moveCursor(QTextCursor::Start);
}

void GUI_PROGRAMMER::on_BurnMethod_Combo_currentIndexChanged(int)
{
    ui->Instructions_TextEdit->clear();
    ui->Instructions_TextEdit->appendPlainText(burnMethods.value(ui->BurnMethod_Combo->currentText()));
    ui->Instructions_TextEdit->moveCursor(QTextCursor::Start);
}

void GUI_PROGRAMMER::on_readSelect_buttonClicked(int)
{
    ui->ReadAddr_Edit->setEnabled(ui->ReadAddr_Radio->isChecked());
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

