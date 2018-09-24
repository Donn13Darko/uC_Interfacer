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

#ifndef GUI_PROGRAMMER_H
#define GUI_PROGRAMMER_H

#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include "GUI_BASE.h"

namespace Ui {
class GUI_PROGRAMMER;
}

class GUI_PROGRAMMER : public GUI_BASE
{
    Q_OBJECT

public:
    explicit GUI_PROGRAMMER(QString deviceType, size_t chunk, QWidget *parent = 0);
    ~GUI_PROGRAMMER();

    void reset_gui();

private slots:
    void on_BrowseHexFile_Button_clicked();
    void on_RefreshPreview_Button_clicked();
    void on_BurnData_Button_clicked();
    void on_HexFormat_Combo_currentIndexChanged(int);
    void on_BurnMethod_Combo_currentIndexChanged(int);

    void on_readSelect_buttonClicked(int);

private:
    Ui::GUI_PROGRAMMER *ui;
    QMap<QString, QString> deviceInstructions;

    size_t chunkSize;
    QByteArray loadedHex;

    static QMap<QString, QRegularExpression> hexFormats;
    static QMap<QString, QStringList> burnMethods;
    static QMap<QString, QMap<QString, QString>> instructionTexts;

    QString format_hex(QByteArray rawHex);
};

#endif // GUI_PROGRAMMER_H
