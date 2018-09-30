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

#ifndef SERIAL_RS232_H
#define SERIAL_RS232_H

#include <QObject>
#include <QMutex>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>

class Serial_RS232 : public QObject
{
    Q_OBJECT

public:
    Serial_RS232(QString port, QString baudrate = "9600", QObject *parent = NULL);
    ~Serial_RS232();

    void open();
    void close();
    bool isConnected();

    static QStringList* getDevices();
    static QStringList Baudrate_Defaults;

signals:
    void deviceConnected();
    void deviceDisconnected();
    void readyRead(QByteArray readData);

public slots:
    void write(QByteArray writeData);
    void write(std::initializer_list<uint8_t> writeData);
    void checkError(QSerialPort::SerialPortError);

private slots:
    void read();

private:
    QSerialPort *rs232;
    QMutex *readLock;
    QMutex *writeLock;

    bool connected;
};

#endif // SERIAL_RS232_H
