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

#include "tcp-server.h"
#include <QDebug>

TCP_SERVER::TCP_SERVER(int port, QObject *parent) :
    QObject(parent)
{
    server = new QTcpServer(this);
    server->setMaxPendingConnections(1);
    server_client = new QTcpSocket(this);
    listen_port = port;

    connecting = new QMessageBox();
    connecting->setText("Waiting for connection...");
    connecting->addButton(QMessageBox::Cancel);

    readLock = new QMutex(QMutex::Recursive);
    writeLock = new QMutex(QMutex::Recursive);
}

TCP_SERVER::~TCP_SERVER()
{
    if (isConnected()) close();
    delete server;
    delete server_client;
    delete connecting;
}

void TCP_SERVER::open()
{
    // Begin listening
    server->listen(QHostAddress::Any, listen_port);
    connect(server, SIGNAL(newConnection()), this, SLOT(connectClient()));

    connecting->show();
    connect(connecting, SIGNAL(rejected()), this, SLOT(close()));
}

void TCP_SERVER::close()
{
    server->close();
    server_client->disconnectFromHost();
}

bool TCP_SERVER::isConnected()
{
    return (server_client->state() == QTcpSocket::ConnectedState);
}

void TCP_SERVER::write(QByteArray writeData)
{
    writeLock->lock();

    qDebug() << "S: " << writeData;
    server_client->write((const QByteArray) writeData);
    server_client->waitForBytesWritten();

    writeLock->unlock();
}

void TCP_SERVER::write(std::initializer_list<uint8_t> writeData)
{
    QByteArray data;
    foreach (char i, writeData)
    {
        data.append(i);
    }

    write(data);
}

void TCP_SERVER::read()
{
    readLock->lock();

    QByteArray recvData = server_client->readAll();
    emit readyRead(recvData);
    qDebug() << "R: " << recvData;

    readLock->unlock();
}

void TCP_SERVER::connectClient()
{
    // Open pending connections
    if (server->hasPendingConnections())
    {
        server_client = server->nextPendingConnection();
        server->close();
        connecting->hide();

        connect(server_client, SIGNAL(readyRead()), this, SLOT(read()));
        disconnect(server, SIGNAL(newConnection()), this, SLOT(connectClient()));
        disconnect(connecting, SIGNAL(rejected()), this, SLOT(close()));
    }
}
