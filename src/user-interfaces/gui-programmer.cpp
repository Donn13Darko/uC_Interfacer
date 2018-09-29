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
    // Create UI and arrays
    ui->setupUi(this);
    loadedHex = QByteArray();

    // Read config settings
    QMap<QString, QMap<QString, QVariant>*>* configMap = readConfigINI(":/user-interfaces/gui-programmer.ini");

    // Add configs to local maps
    QMap<QString, QVariant>* groupMap;
    if (configMap->contains("settings"))
    {
        groupMap = configMap->value("settings");
        addHexFormats(groupMap->value("hex_formats").toStringList());
        addBurnMethods(groupMap->value("burn_methods").toStringList());
    }

    // Delete map after use
    deleteConfigMap(configMap);

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
        if (hexFormat.length() != 2) continue;

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
    if (getOpenFilePath(&file, tr("HEX (*.hex);;All Files (*.*)")))
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
    loadedHex = loadFile(filePath);

    // Reset preview
    on_HexFormat_Combo_currentIndexChanged(0);
}

void GUI_PROGRAMMER::on_BurnData_Button_clicked()
{
    // Send start of programming
    send({
             JSON_PROGRAM,
             PROGRAMNING_INFO_START
         });

    QStringList formattedHexList = ui->HexPreview_Edit->toPlainText().split('\n');
    QStringList curr;
    foreach (QString i, formattedHexList)
    {
        curr = i.split(' ');
        if (curr.isEmpty() || (curr.length() < 5)) continue;

        // Send across address
        if (!curr[3].isEmpty())// && (curr[3].length() == 4))
        {
            // Clear buffers
            rcvd.clear();

            send({
                     JSON_PROGRAM,
                     PROGRAMNING_INFO_ADDRESS,
                     2
                 });
            send(QByteArray::fromHex(curr[3].toUtf8()));

            // Wait for ack back
            waitForResponse(2, 1000);

            // Check ack
            if (!checkAck(rcvd)) break;
        }

        // Send across data
        if (!curr[4].isEmpty())// && (1 < curr[4].length()))
        {
            // Clear buffers
            rcvd.clear();

            // Send next program line
            send({
                     JSON_PROGRAM,
                     PROGRAMNING_INFO_DATA,
                     (uint8_t) (curr[4].length() / 2)
                 });
            send(QByteArray::fromHex(curr[4].toUtf8()));

            // Wait for ack back
            waitForResponse(2, 1000);

            // Check ack
            if (!checkAck(rcvd)) break;
        }
    }

    // Send end of programming
    send({
             JSON_PROGRAM,
             PROGRAMNING_INFO_END
         });
}

void GUI_PROGRAMMER::on_HexFormat_Combo_currentIndexChanged(int)
{
    // Only update format if exists
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
    QStringList hexList = QString(rawHex).split('\n');
    QRegularExpression hexReg = hexFormats.value(ui->HexFormat_Combo->currentText());

    QString final;
    QRegularExpressionMatch hexRegMatch;
    foreach (QString i, hexList)
    {
        if (i.isEmpty()) continue;

        // Attempt to match with regex
        // Should be in format [nnaaaatt_dd_cc]
        hexRegMatch = hexReg.match(i.trimmed());
        final += hexRegMatch.captured(1) + " " + hexRegMatch.captured(3) + " " \
                + hexRegMatch.captured(5) + " " + hexRegMatch.captured(2) + " " \
                + hexRegMatch.captured(4) + "\n";
    }

    return final;
}
