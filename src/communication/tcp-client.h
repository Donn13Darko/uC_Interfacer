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

#include <QObject>
#include <QMutex>
#include <QTcpSocket>

class TCP_CLIENT : public QObject
{
    Q_OBJECT

public:
    TCP_CLIENT(QString ip, int port, QObject *parent = NULL);
    ~TCP_CLIENT();

    void open();
    bool isConnected();

signals:
    void deviceConnected();
    void deviceDisconnected();
    void readyRead(QByteArray readData);

public slots:
    void close();
    void connectClient();
    void disconnectClient();
    void write(QByteArray writeData);
    void write(std::initializer_list<uint8_t> writeData);

private slots:
    void read();

private:
    QTcpSocket *client;

    QString server_ip;
    int server_port;

    QMutex *readLock;
    QMutex *writeLock;
};

#endif // TCP_CLIENT_H
