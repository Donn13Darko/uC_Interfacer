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

#include "serial-com-port.hpp"

QStringList
SERIAL_COM_PORT::Baudrate_Defaults({
                                    "1200", "2400", "4800",
                                    "9600", "19200", "39400",
                                    "57600", "115200", "230400",
                                    "460800", "921600", "Other"
                                });

SERIAL_COM_PORT::SERIAL_COM_PORT(Serial_COM_Port_Settings *serial_settings, QObject *parent) :
    COMMS_BASE(parent)
{
    // Create new serial port
    serial_com_port = new QSerialPort(this);
    initSuccess = (initSuccess && serial_com_port);
    if (!initSuccess) return;

    // Call parser for settings
    parseSettings(serial_settings);

    connect(serial_com_port, SIGNAL(readyRead()),
            this, SLOT(read()),
            Qt::DirectConnection);
}

SERIAL_COM_PORT::~SERIAL_COM_PORT()
{
    if (isConnected()) close();

    delete serial_com_port;
}

void SERIAL_COM_PORT::open()
{
    if (!initSuccess)
    {
        emit deviceDisconnected();
        return;
    }

    connected = serial_com_port->open(QIODevice::ReadWrite);
    if (isConnected())
    {
        connect(serial_com_port, SIGNAL(errorOccurred(QSerialPort::SerialPortError)),
                this, SLOT(checkError(QSerialPort::SerialPortError)),
                Qt::DirectConnection);
        emit deviceConnected();
    } else
    {
        emit deviceDisconnected();
    }
}

bool SERIAL_COM_PORT::isConnected()
{
    return connected;
}

QStringList *SERIAL_COM_PORT::getDevices()
{
    QStringList *portNames = new QStringList();
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    foreach (QSerialPortInfo i, ports)
    {
        portNames->append(i.portName());
    }

    return portNames;
}

void SERIAL_COM_PORT::close()
{
    serial_com_port->close();
    connected = false;
}

void SERIAL_COM_PORT::write(QByteArray writeData)
{
    // Acquire Lock
    writeLock->lock();

    // Write data (try to force start)
    serial_com_port->write((const QByteArray) writeData);
    serial_com_port->flush();

    // Unlock lock
    writeLock->unlock();
}

void SERIAL_COM_PORT::read()
{
    // Acquire Lock
    readLock->lock();

    // Read data
    QByteArray recvData = serial_com_port->readAll();

    // Emit signal
    emit readyRead(recvData);

    // Unlock lock
    readLock->unlock();
}

void SERIAL_COM_PORT::checkError(QSerialPort::SerialPortError)
{
    connected = false;
    emit deviceDisconnected();
}

void SERIAL_COM_PORT::parseSettings(Serial_COM_Port_Settings *serial_settings)
{
    // Set port name
    if (serial_settings->port.isEmpty())
    {
        initSuccess = false;
        return;
    } else
    {
        serial_com_port->setPortName(serial_settings->port);
    }

    // Try setting baudrate (sets direction to All, baudrate can't be 0)
    initSuccess = (initSuccess
                   && serial_settings->baudrate
                   && serial_com_port->setBaudRate(serial_settings->baudrate));
    if (!initSuccess) return;

    // Try setting dataBits, (must be between 5 and 8)
    initSuccess = (initSuccess
                   && (5 <= serial_settings->dataBits)
                   && (serial_settings->dataBits <= 8)
                   && serial_com_port->setDataBits((QSerialPort::DataBits) serial_settings->dataBits));
    if (!initSuccess) return;

    // Try setting flow control (must be less than 2, can be 0)
    initSuccess = (initSuccess
                   && (serial_settings->flowControl <= 2)
                   && serial_com_port->setFlowControl((QSerialPort::FlowControl) serial_settings->flowControl));
    if (!initSuccess) return;

    // Try setting parity (must be less than 5 and not 1, can be 0)
    initSuccess = (initSuccess
                   && (serial_settings->parity != 1)
                   && (serial_settings->parity <= 5)
                   && serial_com_port->setParity((QSerialPort::Parity) serial_settings->parity));
    if (!initSuccess) return;

    // Try setting stop bits (must be between 1 and 3)
    initSuccess = (initSuccess
                   && (1 <= serial_settings->stopBits)
                   && (serial_settings->stopBits <= 3)
                   && serial_com_port->setStopBits((QSerialPort::StopBits) serial_settings->stopBits));
    if (!initSuccess) return;
}
