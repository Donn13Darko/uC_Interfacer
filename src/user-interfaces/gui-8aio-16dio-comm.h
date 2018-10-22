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

#ifndef GUI_8AIO_16DIO_COMM_H
#define GUI_8AIO_16DIO_COMM_H

#include "gui-pin-io-base.h"

#include <QObject>
#include <QTimer>
#include <QFile>
#include <QSlider>
#include <QTextStream>

namespace Ui {
class GUI_8AIO_16DIO_COMM;
}

class GUI_8AIO_16DIO_COMM : public GUI_PIN_BASE
{
    Q_OBJECT

public:
    explicit GUI_8AIO_16DIO_COMM(QWidget *parent = 0);
    ~GUI_8AIO_16DIO_COMM();

    void reset_gui();
    void parseConfigMap(QMap<QString, QVariant>* configMap);

private slots:
    void DIO_ComboChanged();
    void DIO_SliderValueChanged();
    void DIO_LineEditValueChanged();

    void AIO_ComboChanged();
    void AIO_SliderValueChanged();
    void AIO_LineEditValueChanged();

    void updateValues();
    void recordLogData();

    void on_StartUpdater_Button_clicked();
    void on_StopUpdater_Button_clicked();

    void on_LogSaveLocSelect_Button_clicked();
    void on_StartLog_Button_clicked();
    void on_StopLog_Button_clicked();

    void on_ConnConnect_Button_clicked();
    void on_ConnSend_Button_clicked();
    void on_ConnClearRecv_Button_clicked();
    void on_ConnSaveRecv_Button_clicked();

    void on_ConnType_Combo_currentIndexChanged(int);

protected:
    Ui::GUI_8AIO_16DIO_COMM *ui;

    void setConTypes(QStringList connTypes, QList<char> mapValues);

    virtual bool isDataRequest(uint8_t minorKey);
    virtual void setValues(uint8_t pinType, QByteArray values);
    virtual bool getPinTypeInfo(uint8_t pinType, PinTypeInfo *infoPtr);

private:
    void initialize();
    void setupUpdaters();

    void connectUniversalSlots();
};

#endif // GUI_8AIO_16DIO_COMM_H
