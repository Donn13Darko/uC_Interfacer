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
#include <QSettings>

GUI_BASE::GUI_BASE(QWidget *parent) :
    QWidget(parent)
{
}

GUI_BASE::~GUI_BASE()
{
}

void GUI_BASE::set_chunkSize(size_t chunk)
{
    chunkSize = chunk;
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
    // Append crc before sending
    data.append((char) get_crc((const uint8_t*) data.data(),
                               (uint8_t) data.length(), (uint8_t) 0));

    // Emit write command for connected class
    emit write_data(data);
}

void GUI_BASE::send(std::initializer_list<uint8_t> data)
{
    QByteArray dataArray;
    foreach (char i, data)
    {
        dataArray.append(i);
    }

    send(dataArray);
}

void GUI_BASE::sendFile(QString filePath)
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
                 GUI_TYPE_DATA_TRANSMIT,
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
             GUI_TYPE_DATA_TRANSMIT,
             0,
         });
}

void GUI_BASE::reset_remote()
{
    send({MAJOR_KEY_RESET, 0});
    send({MAJOR_KEY_RESET, 0});
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
    if ((ack.length() != 2) || (((uint8_t) ack[0]) != MAJOR_KEY_ACK)
            || (((uint8_t) ack[1]) != MAJOR_KEY_SUCCESS))
    {
        GUI_HELPER::showMessage("Error: Incorrect ACK, operation failed!");
        return false;
    } else
    {
        return true;
    }
}
