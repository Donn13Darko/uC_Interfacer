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

#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include "comms-base.h"
#include <QTcpSocket>

class TCP_CLIENT : public COMMS_BASE
{
    Q_OBJECT

public:
    TCP_CLIENT(QString ip, int port, QObject *parent = NULL);
    ~TCP_CLIENT();

    virtual void open();
    virtual bool isConnected();

public slots:
    virtual void close();
    void connectClient();
    void disconnectClient();
    virtual void write(QByteArray writeData);

private slots:
    virtual void read();

private:
    QTcpSocket *client;

    QString server_ip;
    int server_port;
};

#endif // TCP_CLIENT_H
