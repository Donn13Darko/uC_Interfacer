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

    virtual void parseConfigMap(QMap<QString, QVariant> *configMap);

    void addFileFormats(QStringList fileFormatsMap);
    void removeFileFormats(QStringList fileFormatsList);
    void addBurnMethods(QStringList burnMethodsMap);

    virtual bool isDataRequest(uint8_t minorKey);

public slots:
    virtual void reset_gui();

protected slots:
    virtual void receive_gui(QByteArray recvData);

    virtual void set_progress_update_recv(int progress, QString label);
    virtual void set_progress_update_send(int progress, QString label);

private slots:
    void on_BrowseFile_Button_clicked();
    void on_RefreshPreview_Button_clicked();
    void on_BurnData_Button_clicked();
    void on_FileFormat_Combo_activated(int);
    void on_BurnMethod_Combo_currentIndexChanged(int);
    void on_FilePreview_CheckBox_stateChanged(int);

    void on_ReadData_RadioGroup_buttonClicked(int);
    void on_ReadDataClear_Button_clicked();
    void on_ReadData_Button_clicked();
    void on_ReadDataSave_Button_clicked();

private:
    Ui::GUI_PROGRAMMER *ui;

    QString curr_fileFormat;
    QMap<QString, QPair<uint8_t, QString>> fileFormats;
    QMap<QString, QString> burnMethods;

    QByteArray format_file(QByteArray rawFile);
    void refresh_file();

    uint8_t progress_divisor;
    uint8_t progress_adjuster;
};

#endif // GUI_PROGRAMMER_H
