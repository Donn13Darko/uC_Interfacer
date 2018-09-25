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

#include "GUI_PROGRAMMER.h"
#include "ui_GUI_PROGRAMMER.h"

// Setup static hex format and regex parsing map
QMap<QString, QRegularExpression>
GUI_PROGRAMMER::hexFormats({
                               {"Microchip 8-bit",
                                QRegularExpression("^:(.{2})(.{4})(.{2})(.*)(.{2})$")},
                               {"Microchip 16-bit",
                                QRegularExpression("^:(.{2})(.{4})(.{2})(.*)(.{2})$")},
                               {"Microchip 32-bit",
                                QRegularExpression("^:(.{2})(.{4})(.{2})(.*)(.{2})$")}
                           });

// Setup static burn methods list
QMap<QString, QStringList>
GUI_PROGRAMMER::burnMethods({
                                {"Arduino Uno/Genuino",
                                 {"AVR ICSP",
                                  "PIC18 ICSP",
                                  "PIC32 ICSP 2-Wire",
                                  "PIC32 ICSP 4-Wire",
                                  "Arduino Bootloader"}
                                }
                            });

// Setup static instructions list
QMap<QString, QMap<QString, QString>>
GUI_PROGRAMMER::instructionTexts({
                                     {"Arduino Uno/Genuino",
                                      {
                                          {"AVR ICSP",
                                           QString("Arduino Pinout:\nPin 10: RESET\nPin 11: MOSI\nPin 12: MISO\nPin 13: SCK\n")
                                           + "\n"
                                           + "Target Pinout:\nConnect one-to-one.\ni.e. SCK->SCK, MISO->MISO, etc.\n"
                                           + "\n"
                                           + "6-Pin ICSP Pinout:\nPin 1: MISO\nPin 2: +VCC\nPin 3: SCK\n"
                                           + "Pin 4: MOSI\nPin 5: RESET\nPin 6: GROUND\n"
                                           + "\n"
                                           + "10-Pin ICSP Pinout:\nPin 1: MISO\nPin 2: +VCC\nPin 3: NC\n"
                                           + "Pin 4: GROUND\nPin 5: RESET\nPin 6: GROUND\nPin 7: SCK\n"
                                           + "Pin 8: GROUND\nPin 9: MISO\nPin 10: GROUND\n"
                                          },
                                          {"PIC18 ICSP",
                                           QString("Arduino Pinout:\nPin 10: MCLR\nPin 11: ICSPDAT\nPin 13: ICSPCLK\n")
                                           + "\n"
                                           + "Target Pinout:\nConnect one-to-one.\ni.e. ICSPCLK->ICSPCLK, ICSPDAT->ICSPDAT, etc.\n"
                                           + "\n"
                                          },
                                          {"PIC32 ICSP 2-Wire",
                                           QString("Arduino Pinout:\n")
                                           + "\n"
                                           + "Target Pinout:\n"
                                           + "\n"
                                          },
                                          {"PIC32 ICSP 4-Wire",
                                           QString("Arduino Pinout:\n")
                                           + "\n"
                                           + "Target Pinout:\n"
                                           + "\n"
                                          },
                                          {"Arduino Bootloader",
                                           QString("Arduino Pinout:\nPin 0: RX\nPin 1: TX\n")
                                           + "\n"
                                           + "Target Pinout:\nRX to Arduino TX\nTX to Arduino RX\n"
                                          }
                                      }
                                     }
                                 });

GUI_PROGRAMMER::GUI_PROGRAMMER(QString deviceType, size_t chunk, QWidget *parent) :
    GUI_BASE(parent),
    ui(new Ui::GUI_PROGRAMMER)
{
    ui->setupUi(this);
    chunkSize = chunk;
    loadedHex = QByteArray();
    deviceInstructions = instructionTexts.value(deviceType);

    ui->HexFormat_Combo->addItems(GUI_PROGRAMMER::hexFormats.keys());
    ui->BurnMethod_Combo->addItems(GUI_PROGRAMMER::burnMethods.value(deviceType));

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
    for (int i = 0; i < formattedHexList.length(); i++)
    {
        curr = formattedHexList[i].split(' ');
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
    ui->Instructions_TextEdit->appendPlainText(deviceInstructions.value(ui->BurnMethod_Combo->currentText()));
}

void GUI_PROGRAMMER::on_readSelect_buttonClicked(int)
{
    ui->ReadAddr_Edit->setEnabled(ui->ReadAddr_Radio->isChecked());
}

QString GUI_PROGRAMMER::format_hex(QByteArray rawHex)
{
    QStringList hexList = QString(rawHex).split('\n');
    QRegularExpression hexReg = hexFormats.value(ui->HexFormat_Combo->currentText());

    QString final, curr;
    QRegularExpressionMatch hexRegMatch;
    for (int i = 0; i < hexList.length(); i++)
    {
        // Grab next line
        curr = hexList[i];
        if (curr.isEmpty()) continue;

        // Attempt to match with regex
        // Should be in format [nnaaaatt_dd_cc]
        hexRegMatch = hexReg.match(curr.trimmed());
        final += hexRegMatch.captured(1) + " " + hexRegMatch.captured(3) + " " \
                + hexRegMatch.captured(5) + " " + hexRegMatch.captured(2) + " " \
                + hexRegMatch.captured(4) + "\n";
    }

    return final;
}

