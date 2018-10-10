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

#ifndef GUI_BASE_H
#define GUI_BASE_H

#include <QObject>
#include <QWidget>
#include <QMap>
#include <QVariant>
#include <QMutex>

#include "../communication/crc-calcs.h"
#include "../communication/general-comms.h"
#include "../gui-helper.h"
#include <QDebug>

class GUI_BASE : public QWidget
{
    Q_OBJECT

public:
    GUI_BASE(QWidget *parent = 0);
    ~GUI_BASE();

    void reset_gui() {/*Default do nothing*/}
    void reset_remote();
    void set_chunkSize(size_t chunk);

signals:
    void write_data(QByteArray data);
    void connect_signals(bool connect);
    void ackReady();
    void readyRead();

protected slots:
    void receive(QByteArray recvData);

protected:
    const float S2MS = 1000.0f;
    QByteArray rcvd;
    uint8_t chunkSize;

    void send(QString data);
    void send(QByteArray data);
    void send(std::initializer_list<uint8_t> data);
    void sendFile(QString filePath);

    void waitForAck(int msecs = 5000);
    bool checkAck();

private:
    bool ack_status;
    uint8_t ack_key;
    QMutex sendLock;
    QList<QByteArray> msgList;
};

#endif // GUI_BASE_H
