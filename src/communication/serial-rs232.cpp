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

QStringList
Serial_RS232::Baudrate_Defaults({
                                    "1200", "2400", "4800",
                                    "9600", "19200", "39400",
                                    "57600", "115200", "230400",
                                    "460800", "921600", "Other"
                                });

Serial_RS232::Serial_RS232(Serial_RS232_Settings* serial_settings, QObject *parent) :
    COMMS_BASE(parent)
{
    // Create new serial port
    rs232 = new QSerialPort(this);
    initSuccess = (initSuccess && rs232);
    if (!initSuccess) return;

    // Call parser for settings
    parseSettings(serial_settings);

    connect(rs232, SIGNAL(readyRead()),
            this, SLOT(read()));
}

Serial_RS232::~Serial_RS232()
{
    if (isConnected()) close();

    delete rs232;
}

void Serial_RS232::open()
{
    if (!initSuccess)
    {
        emit deviceDisconnected();
        return;
    }

    connected = rs232->open(QIODevice::ReadWrite);
    if (isConnected())
    {
        connect(rs232, SIGNAL(errorOccurred(QSerialPort::SerialPortError)),
                this, SLOT(checkError(QSerialPort::SerialPortError)));
        emit deviceConnected();
    } else
    {
        emit deviceDisconnected();
    }
}

bool Serial_RS232::isConnected()
{
    return connected;
}

QStringList* Serial_RS232::getDevices()
{
    QStringList* portNames = new QStringList();
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    foreach (QSerialPortInfo i, ports)
    {
        portNames->append(i.portName());
    }

    return portNames;
}

void Serial_RS232::close()
{
    rs232->close();
    connected = false;
}

void Serial_RS232::write(QByteArray writeData)
{
    writeLock->lock();

    qDebug() << "S: " << writeData;
    rs232->write((const QByteArray) writeData);
    rs232->flush();

    writeLock->unlock();
}

void Serial_RS232::read()
{
    readLock->lock();
    QByteArray recvData = rs232->readAll();
    emit readyRead(recvData);
    qDebug() << "R: " << recvData;
    readLock->unlock();
}

void Serial_RS232::checkError(QSerialPort::SerialPortError)
{
    connected = false;
    emit deviceDisconnected();
}

void Serial_RS232::parseSettings(Serial_RS232_Settings* serial_settings)
{
    // Set port name
    if (serial_settings->port.isEmpty())
    {
        initSuccess = false;
        return;
    } else
    {
        rs232->setPortName(serial_settings->port);
    }

    // Try setting baudrate (sets direction to All, baudrate can't be 0)
    initSuccess = (initSuccess
                   && serial_settings->baudrate
                   && rs232->setBaudRate(serial_settings->baudrate));
    if (!initSuccess) return;

    // Try setting dataBits, (must be between 5 and 8)
    initSuccess = (initSuccess
                   && (5 <= serial_settings->dataBits)
                   && (serial_settings->dataBits <= 8)
                   && rs232->setDataBits((QSerialPort::DataBits) serial_settings->dataBits));
    if (!initSuccess) return;

    // Try setting flow control (must be less than 2, can be 0)
    initSuccess = (initSuccess
                   && (serial_settings->flowControl <= 2)
                   && rs232->setFlowControl((QSerialPort::FlowControl) serial_settings->flowControl));
    if (!initSuccess) return;

    // Try setting parity (must be less than 5 and not 1, can be 0)
    initSuccess = (initSuccess
                   && (serial_settings->parity != 1)
                   && (serial_settings->parity <= 5)
                   && rs232->setParity((QSerialPort::Parity) serial_settings->parity));
    if (!initSuccess) return;

    // Try setting stop bits (must be between 1 and 3)
    initSuccess = (initSuccess
                   && (1 <= serial_settings->stopBits)
                   && (serial_settings->stopBits <= 3)
                   && rs232->setStopBits((QSerialPort::StopBits) serial_settings->stopBits));
    if (!initSuccess) return;
}
