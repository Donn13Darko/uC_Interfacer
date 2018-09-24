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

#include "../communication/json_info.h"
#include <QDebug>

class GUI_BASE : public QWidget
{
    Q_OBJECT

public:
    GUI_BASE(QWidget *parent = 0);
    ~GUI_BASE();

    static bool showMessage(QString msg);
    void reset_gui() {/*Default do nothing*/}

signals:
    void write_data(QByteArray data);
    void write_data(std::initializer_list<uint8_t> data);
    void connect_signals(bool connect);
    void readyRead();

private slots:
    void receive(QByteArray recvData);

protected:
    static float S2MS;
    QByteArray rcvd;

    void send(QString data);
    void send(QByteArray data);
    void send(std::initializer_list<uint8_t> data);
    void sendFile(QString filePath, uint8_t chunkSize);

    bool getOpenFilePath(QString *filePath, QString fileTypes = tr("All Files (*.*)"));
    bool getSaveFilePath(QString *filePath, QString fileTypes = tr("All Files (*.*)"));
    bool saveFile(QString filePath, QByteArray data);
    QByteArray loadFile(QString filePath);

    void reset_remote();
    void waitForResponse(int len, int msecs = 5000);
    bool checkAck(QByteArray ack);
};

#endif // GUI_BASE_H
