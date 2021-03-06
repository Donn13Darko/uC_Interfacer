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

#include "tcp-client.hpp"

TCP_CLIENT::TCP_CLIENT(QString ip, int port, QObject *parent) :
    COMMS_BASE(parent)
{
    // Create new client
    client = new QTcpSocket(this);
    initSuccess = (initSuccess && client);
    if (!initSuccess) return;

    // Set variables
    server_ip = ip;
    server_port = port;

    connect(client, SIGNAL(readyRead()),
            this, SLOT(read()),
            Qt::DirectConnection);
    connect(client, SIGNAL(disconnected()),
            this, SLOT(disconnectClient()),
            Qt::DirectConnection);
}

TCP_CLIENT::~TCP_CLIENT()
{
    if (isConnected()) close();

    delete client;
}

void TCP_CLIENT::open()
{
    // Connect signals and slots
    connect(client, SIGNAL(connected()),
            this, SLOT(connectClient()),
            Qt::DirectConnection);

    // Attempt to connect
    client->connectToHost(server_ip, server_port, QIODevice::ReadWrite);
}

bool TCP_CLIENT::isConnected()
{
    return (client && (client->state() == QTcpSocket::ConnectedState));
}

void TCP_CLIENT::close()
{
    // Remove close slot to prevent infinite loop
    disconnect(client, SIGNAL(disconnected()),
               this, SLOT(disconnectClient()));

    // Disconnect
    client->disconnectFromHost();
}

void TCP_CLIENT::connectClient()
{
    disconnect(client, SIGNAL(connected()),
               this, SLOT(connectClient()));
    emit deviceConnected();
}

void TCP_CLIENT::disconnectClient()
{
    emit deviceDisconnected();
}

void TCP_CLIENT::write(QByteArray writeData)
{
    // Acquire Lock
    writeLock->lock();

    // Write data (try to force start)
    client->write((const QByteArray) writeData);
    client->flush();

    // Unlock lock
    writeLock->unlock();
}

void TCP_CLIENT::read()
{
    // Acquire Lock
    readLock->lock();

    // Read data
    QByteArray recvData = client->readAll();

    // Emit signal
    emit readyRead(recvData);

    // Unlock lock
    readLock->unlock();
}
