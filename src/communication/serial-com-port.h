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

#ifndef SERIAL_COM_PORT_H
#define SERIAL_COM_PORT_H

#include "comms-base.h"
#include <QSerialPort>
#include <QSerialPortInfo>

typedef struct {
    QString port;
    int32_t baudrate;
    uint8_t dataBits;
    uint8_t flowControl;
    uint8_t parity;
    uint8_t stopBits;
} Serial_COM_Port_Settings;
#define Serial_COM_Port_Settings_DEFAULT Serial_COM_Port_Settings{\
    .port="", .baudrate=9600, .dataBits=8,\
    .flowControl=0, .parity=0, .stopBits=1}

class SERIAL_COM_PORT : public COMMS_BASE
{
    Q_OBJECT

public:
    SERIAL_COM_PORT(Serial_COM_Port_Settings *serial_settings, QObject *parent = NULL);
    ~SERIAL_COM_PORT();

    virtual void open();
    virtual void close();
    virtual bool isConnected();

    static QStringList *getDevices();
    static QStringList Baudrate_Defaults;

public slots:
    virtual void write(QByteArray writeData);

private slots:
    virtual void read();
    void checkError(QSerialPort::SerialPortError);

private:
    QSerialPort *serial_com_port;

    void parseSettings(Serial_COM_Port_Settings *serial_settings);
};

#endif // SERIAL_COM_PORT_H
