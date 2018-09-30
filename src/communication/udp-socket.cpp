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

#include "udp-socket.h"
#include <QDebug>
#include <QNetworkDatagram>

UDP_SOCKET::UDP_SOCKET(QString client_ip, int client_port, int server_port, QObject *parent) :
    QObject(parent)
{
    client = new QUdpSocket(this);
    server = new QUdpSocket(this);
    udp_client_ip = QHostAddress(client_ip);
    udp_client_port = client_port;
    udp_server_port = server_port;

    readLock = new QMutex(QMutex::Recursive);
    writeLock = new QMutex(QMutex::Recursive);

    connect(server, SIGNAL(readyRead()),
            this, SLOT(read()));
    connect(server, SIGNAL(disconnected()),
            this, SLOT(disconnectClient()));
}

UDP_SOCKET::~UDP_SOCKET()
{
    if (isConnected()) close();

    delete client;
    delete server;
    delete readLock;
    delete writeLock;
}

void UDP_SOCKET::open()
{
    // Attempt to bind port
    bool connected = server->bind(udp_server_port);
    if (!connected && !server->waitForConnected(1000))
        emit deviceDisconnected();
    else
        emit deviceConnected();
}

void UDP_SOCKET::close()
{
    // Remove close slot to prevent infinite loop
    disconnect(server, SIGNAL(disconnected()),
            this, SLOT(disconnectClient()));

    // Disconnect
    server->disconnectFromHost();
}

bool UDP_SOCKET::isConnected()
{
    return (server && (server->state() == QUdpSocket::BoundState));
}

void UDP_SOCKET::disconnectClient()
{
    emit deviceDisconnected();
}

void UDP_SOCKET::write(QByteArray writeData)
{
    writeLock->lock();

    qDebug() << "S: " << writeData;
    client->writeDatagram((const QByteArray) writeData,
                          udp_client_ip, udp_client_port);
    client->waitForBytesWritten();

    writeLock->unlock();
}

void UDP_SOCKET::write(std::initializer_list<uint8_t> writeData)
{
    QByteArray data;
    foreach (char i, writeData)
    {
        data.append(i);
    }

    write(data);
}

void UDP_SOCKET::read()
{
    readLock->lock();

    QByteArray recvData;
    while (server->hasPendingDatagrams())
    {
        recvData += server->receiveDatagram().data();
    }
    emit readyRead(recvData);
    qDebug() << "R: " << recvData;

    readLock->unlock();
}
