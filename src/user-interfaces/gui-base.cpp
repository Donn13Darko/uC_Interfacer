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

#include "gui-base.h"

#include <QMessageBox>
#include <QFile>
#include <QTimer>
#include <QFileDialog>
#include <QTextStream>
#include <QEventLoop>

float GUI_BASE::S2MS = 1000.0;

GUI_BASE::GUI_BASE(QWidget *parent) :
    QWidget(parent)
{
}

GUI_BASE::~GUI_BASE()
{
}

bool GUI_BASE::showMessage(QString msg)
{
    QMessageBox n;
    n.setText(msg);
    return n.exec();
}

void GUI_BASE::receive(QByteArray recvData)
{
    rcvd += recvData;
    emit readyRead();
}

void GUI_BASE::send(QString data)
{
    send(data.toUtf8());
}

void GUI_BASE::send(QByteArray data)
{
    emit write_data(data);
}

void GUI_BASE::send(std::initializer_list<uint8_t> data)
{
    emit write_data(data);
}

void GUI_BASE::sendFile(QString filePath, uint8_t chunkSize)
{
    uint32_t enumFlags = QIODevice::ReadOnly;
    QFile sFile(filePath);
    if (!sFile.open((QIODevice::OpenModeFlag) enumFlags)) return;

    int sizeRead;
    char chunkRead[chunkSize];
    while (!sFile.atEnd())
    {
        // Zero array then read next chunk
        memset(chunkRead, 0, chunkSize);
        sizeRead = sFile.readLine(chunkRead, chunkSize);

        // Ensure no error and read is greater than zero
        if (0 < sizeRead) continue;
        rcvd.clear();

        // Send read chunk size
        send({
                 JSON_FILE,
                 (uint8_t) sizeRead,
             });

        // Send next chunk
        send(QString(chunkRead));

        // Wait for ack back
        waitForResponse(2, 1000);

        // Check ack
        if (!checkAck(rcvd)) break;
    }
    sFile.close();

    // Send file done (0 will never be sent above)
    send({
             JSON_FILE,
             0,
         });
}

bool GUI_BASE::getOpenFilePath(QString *filePath, QString fileTypes)
{
    *filePath = QFileDialog::getOpenFileName(this, tr("Open"),
                                             "", fileTypes);

    return !filePath->isEmpty();
}

bool GUI_BASE::getSaveFilePath(QString *filePath, QString fileTypes)
{
    *filePath = QFileDialog::getSaveFileName(this, tr("Save Location"),
                                             "", fileTypes);

    return !filePath->isEmpty();
}

bool GUI_BASE::saveFile(QString filePath, QByteArray data)
{
    uint32_t enumFlags = QIODevice::WriteOnly;
    QFile sFile(filePath);
    if (!sFile.open((QIODevice::OpenModeFlag) enumFlags))
        return false;

    qint64 res = sFile.write(data);
    sFile.close();

    if (res < 0)
        return false;
    return true;
}

QByteArray GUI_BASE::loadFile(QString filePath)
{
    uint32_t enumFlags = QIODevice::ReadOnly;
    QFile sFile(filePath);
    if (!sFile.open((QIODevice::OpenModeFlag) enumFlags)) return QByteArray();

    QByteArray data = sFile.readAll();
    sFile.close();

    return data;
}

void GUI_BASE::reset_remote()
{
    send({JSON_RESET, JSON_RESET});
    send({JSON_RESET, JSON_RESET});
}

void GUI_BASE::waitForResponse(int len, int msecs)
{
    // Setup time keeping
    int updateRate = 10;
    QEventLoop loop;
    QTimer timer;
    connect(this,  SIGNAL(readyRead()), &loop, SLOT(quit()) );
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));

    // Wait for time
    while ((rcvd.length() < len) && (0 < msecs))
    {
        msecs -= updateRate;
        timer.start(updateRate);
        loop.exec();
    }
}

bool GUI_BASE::checkAck(QByteArray ack)
{
    // Check ack
    if ((ack.length() != 2) || (((uint8_t) ack[0]) != JSON_COPY)
            || (((uint8_t) ack[1]) != JSON_SUCCESS))
    {
        showMessage("Error: Incorrect ACK, operation failed!");
        return false;
    } else
    {
        return true;
    }
}
