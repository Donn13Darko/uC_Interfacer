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
    void on_RESET_BUTTON_clicked();

    void DIO_ComboChanged();
    void DIO_SliderValueChanged();
    void DIO_TextValueChanged();

    void AIO_ComboChanged();
    void AIO_SliderValueChanged();
    void AIO_TextValueChanged();

    void updateValues();
    void io_receive(QByteArray recvData);
    void recordLogData();

    void on_updateStarter_clicked();
    void on_updateStopper_clicked();

    void on_selectSaveLocation_clicked();
    void on_startLog_clicked();
    void on_stopLog_clicked();

    void on_ConnectButton_clicked();
    void on_SendButton_clicked();
    void on_ClearRecvButton_clicked();

    void on_ConnTypeCombo_currentIndexChanged(int);

protected:
    Ui::GUI_8AIO_16DIO_COMM *ui;

    void setNumPins(uint8_t pinType, uint8_t num_dev_pins, uint8_t start_num);
    void setCombos(uint8_t pinTypes, QList<QString> combos);

    void setConTypes(QStringList connTypes, QList<char> mapValues);

private:
    void initialize();
    void setupUpdaters();

    void connectUniversalSlots();

    void setValues(uint8_t pinType, QByteArray values);

    void recordPinValues(PinTypeInfo *pInfo);

    bool getPinTypeInfo(uint8_t pinType, PinTypeInfo *infoPtr);
};

#endif // GUI_8AIO_16DIO_COMM_H
