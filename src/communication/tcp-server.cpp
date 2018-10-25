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

TCP_SERVER::TCP_SERVER(int port, QObject *parent) :
    COMMS_BASE(parent)
{
    // Create new server
    server = new QTcpServer(this);
    initSuccess = (initSuccess && server);
    if (!initSuccess) return;

    // Set new server values
    server->setMaxPendingConnections(1);
    server_client = nullptr;
    listen_port = port;

    // Create new message box
    connecting_msg = new QMessageBox();
    initSuccess = (initSuccess && connecting_msg);
    if (!initSuccess) return;

    // Set message box values
    connecting_msg->setText("Waiting for connection...");
    connecting_msg->addButton(QMessageBox::Cancel);
    connecting_msg->setModal(true);
}

TCP_SERVER::~TCP_SERVER()
{
    if (isConnected()) close();

    if (server_client) delete server_client;
    delete server;
    delete connecting_msg;
}

void TCP_SERVER::open()
{
    // Connect signals before use
    connect(server, SIGNAL(newConnection()),
            this, SLOT(connectClient()));
    connect(connecting_msg, SIGNAL(finished(int)),
            this, SLOT(connecting_finished(int)));

    // Begin listening
    server->listen(QHostAddress::Any, listen_port);
    connecting_msg->show();
}

bool TCP_SERVER::isConnected()
{
    return (server_client && (server_client->state() == QTcpSocket::ConnectedState));
}

void TCP_SERVER::close()
{
    // Disconnect server_client
    if (server_client)
    {
        disconnect(server_client, SIGNAL(disconnected()),
                   this, SLOT(disconnectClient()));
        server_client->disconnectFromHost();
    }

    // Disconnect server
    server->close();
}

void TCP_SERVER::write(QByteArray writeData)
{
    writeLock->lock();

    qDebug() << "S: " << writeData;
    server_client->write((const QByteArray) writeData);
    server_client->waitForBytesWritten();

    writeLock->unlock();
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
        // Get next connection from server
        server_client = server->nextPendingConnection();
        server->close();
        connecting_msg->hide();

        // Connect client signals and slots
        connect(server_client, SIGNAL(readyRead()), this, SLOT(read()));
        connect(server_client, SIGNAL(disconnected()), this, SLOT(disconnectClient()));

        // Remove uneeded connections
        disconnect(server, SIGNAL(newConnection()), this, SLOT(connectClient()));
        disconnect(connecting_msg, SIGNAL(finished(int)),
                this, SLOT(connecting_finished(int)));

        // Notify host to conitnue
        emit deviceConnected();
    }
}

void TCP_SERVER::disconnectClient()
{
    emit deviceDisconnected();
}

void TCP_SERVER::connecting_finished(int res)
{
    if ((res == QMessageBox::Cancel) || (res == QMessageBox::Close))
    {
        close();
        emit deviceDisconnected();
    }
}
