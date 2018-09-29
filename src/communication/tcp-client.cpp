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

#include "tcp-client.h"
#include <QDebug>

TCP_CLIENT::TCP_CLIENT(QString ip, int port, QObject *parent) :
    QObject(parent)
{
    client = new QTcpSocket(this);
    server_ip = ip;
    server_port = port;

    readLock = new QMutex(QMutex::Recursive);
    writeLock = new QMutex(QMutex::Recursive);

    connect(client, SIGNAL(readyRead()), this, SLOT(read()));
}

TCP_CLIENT::~TCP_CLIENT()
{
    if (isConnected()) close();
    delete client;
}

void TCP_CLIENT::open()
{
    // Attempt to connect
    client->connectToHost(server_ip, server_port, QIODevice::ReadWrite);

    // Wait some time for connection
    if (client->state() != QTcpSocket::ConnectedState)
        client->waitForConnected(5000);
}

void TCP_CLIENT::close()
{
    client->close();
}

bool TCP_CLIENT::isConnected()
{
    return (client->state() == QTcpSocket::ConnectedState);
}

void TCP_CLIENT::write(QByteArray writeData)
{
    writeLock->lock();

    qDebug() << "S: " << writeData;
    client->write((const QByteArray) writeData);
    client->waitForBytesWritten();

    writeLock->unlock();
}

void TCP_CLIENT::write(std::initializer_list<uint8_t> writeData)
{
    QByteArray data;
    foreach (char i, writeData)
    {
        data.append(i);
    }

    write(data);
}

void TCP_CLIENT::read()
{
    readLock->lock();

    QByteArray recvData = client->readAll();
    emit readyRead(recvData);
    qDebug() << "R: " << recvData;

    readLock->unlock();
}
