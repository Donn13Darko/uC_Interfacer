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

#ifndef COMMS_BASE_H
#define COMMS_BASE_H

#include <QObject>
#include <QMutex>

class COMMS_BASE : public QObject
{
    Q_OBJECT

public:
    COMMS_BASE(QObject *parent = NULL);
    ~COMMS_BASE();

    virtual void open();
    virtual bool isConnected();
    virtual bool initSuccessful();

signals:
    void deviceConnected();
    void deviceDisconnected();
    void readyRead(QByteArray readData);

public slots:
    virtual void close();
    virtual void write(QByteArray writeData);

protected slots:
    virtual void read();

protected:
    QMutex *readLock;
    QMutex *writeLock;

    bool connected;
    bool initSuccess;
};

#endif // COMMS_BASE_H
