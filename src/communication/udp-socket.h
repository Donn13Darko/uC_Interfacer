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

#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include <QObject>
#include <QMutex>
#include <QUdpSocket>
#include <QHostAddress>

class UDP_SOCKET : public QObject
{
    Q_OBJECT

public:
    UDP_SOCKET(QString client_ip, int client_port, int server_port, QObject *parent = NULL);
    ~UDP_SOCKET();

    void open();
    bool isConnected();

signals:
    void deviceConnected();
    void deviceDisconnected();
    void readyRead(QByteArray readData);

public slots:
    void close();
    void disconnectClient();
    void write(QByteArray writeData);
    void write(std::initializer_list<uint8_t> writeData);

private slots:
    void read();

private:
    QUdpSocket *client;
    QUdpSocket *server;

    QHostAddress udp_client_ip;
    int udp_client_port;
    int udp_server_port;

    QMutex *readLock;
    QMutex *writeLock;
};

#endif // UDP_SOCKET_H