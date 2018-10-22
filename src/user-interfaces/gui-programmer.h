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

#include "gui-base.h"
#include "gui-programmer-minor-keys.h"

namespace Ui {
class GUI_PROGRAMMER;
}

class GUI_PROGRAMMER : public GUI_BASE
{
    Q_OBJECT

public:
    explicit GUI_PROGRAMMER(QWidget *parent = 0);
    ~GUI_PROGRAMMER();

    virtual void reset_gui();
    void addHexFormats(QStringList hexFormatsMap);
    void removeHexFormats(QStringList hexFormatsList);
    void addBurnMethods(QStringList burnMethodsMap);

protected slots:
    virtual void receive_gui(QByteArray recvData);

protected:
    virtual bool isDataRequest(uint8_t minorKey);

private slots:
    void on_BrowseHexFile_Button_clicked();
    void on_RefreshPreview_Button_clicked();
    void on_BurnData_Button_clicked();
    void on_HexFormat_Combo_activated(int);
    void on_BurnMethod_Combo_currentIndexChanged(int);

    void on_readSelect_buttonClicked(int);
    void on_ClearReadData_Button_clicked();
    void on_ReadData_Button_clicked();
    void on_SaveReadData_Button_clicked();

private:
    Ui::GUI_PROGRAMMER *ui;

    QString curr_hexFormat;
    QMap<QString, QRegularExpression> hexFormats;
    QMap<QString, QString> burnMethods;

    QByteArray loadedHex;
    QString format_hex(QByteArray rawHex);
};

#endif // GUI_PROGRAMMER_H
