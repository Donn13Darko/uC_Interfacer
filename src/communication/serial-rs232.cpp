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

#include "serial-rs232.h"
#include <QDebug>

QStringList
Serial_RS232::Baudrate_Defaults({
                                    "1200", "2400", "4800",
                                    "9600", "19200", "39400",
                                    "57600", "115200"
                                });

Serial_RS232::Serial_RS232(QString port, QString baudrate, QObject *parent) :
    QObject(parent)
{
    rs232 = new QSerialPort(this);
    rs232->setPortName(port);
    rs232->setBaudRate(baudrate.toInt());
    rs232->setDataBits(QSerialPort::Data8);

    maxSize = 1024;
    readLock = new QMutex(QMutex::Recursive);
    writeLock = new QMutex(QMutex::Recursive);

    connect(rs232, SIGNAL(readyRead()), this, SLOT(read()));
}

Serial_RS232::~Serial_RS232()
{
    delete rs232;
}

void Serial_RS232::open()
{
    connected = rs232->open(QIODevice::ReadWrite);
}

void Serial_RS232::close()
{
    rs232->close();
    connected = false;
}

bool Serial_RS232::isConnected()
{
    return connected;
}

QStringList Serial_RS232::getDevices()
{
    QStringList portNames;
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for (auto i = ports.begin(); i != ports.end(); i++)
        portNames.append((*i).portName());

    return portNames;
}

void Serial_RS232::write(QByteArray writeData)
{
    writeLock->lock();

    qDebug() << "S: " << writeData;
    rs232->write((const QByteArray) writeData);
    rs232->waitForBytesWritten();

    writeLock->unlock();
}

void Serial_RS232::write(std::initializer_list<uint8_t> writeData)
{
    QByteArray data;
    for (auto i = writeData.begin(); i != writeData.end(); i++)
    {
        data.append((char) (*i));
    }

    write(data);
}

void Serial_RS232::read()
{
    readLock->lock();
    QByteArray recvData = rs232->readAll();
    emit readyRead(recvData);
    qDebug() << "R: " << recvData;
    readLock->unlock();
}
