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

#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "comms-base.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QMessageBox>

class TCP_SERVER : public COMMS_BASE
{
    Q_OBJECT

public:
    TCP_SERVER(int port, QObject *parent = NULL);
    ~TCP_SERVER();

    virtual void open();
    virtual bool isConnected();

public slots:
    virtual void close();
    virtual void write(QByteArray writeData);

private slots:
    virtual void read();
    void connectClient();
    void disconnectClient();
    void connecting_finished(int res);

private:
    QTcpServer *server;
    QTcpSocket *server_client;
    QMessageBox *connecting_msg;

    int listen_port;
};

#endif // TCP_SERVER_H
