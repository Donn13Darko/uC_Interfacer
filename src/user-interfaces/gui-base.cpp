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

uint8_t num_p2_bytes;

GUI_BASE::GUI_BASE(QWidget *parent) :
    QWidget(parent)
{
    ack_status = false;
    ack_key = MAJOR_KEY_ERROR;
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
    // Add data to recv
    rcvd.append(recvData);
    uint8_t rcvd_len = rcvd.length();

    // Check to see if Packet #1 in rcvd
    if (rcvd_len < num_p1_bytes) return;

    // Check Packet #1 and discard all data if crc_check fails
    if (!check_crc((uint8_t*) rcvd.data(), p1_crc_loc, rcvd.at(p1_crc_loc), 0))
    {
        rcvd.clear();
        return;
    }

    // Check if Ack packet (no Packet #2)
    if (rcvd.at(p1_major_key_loc) == (char) MAJOR_KEY_ACK)
    {
        ack_status = checkAck();
        emit ackReady();
        return;
    }

    // Check if expecting a Packet #2
    num_p2_bytes = rcvd.at(p1_num_p2_bytes_loc);
    if (num_p2_bytes != 0)
    {
        QByteArray p2 = rcvd;
        p2.remove(0, num_p1_bytes);

        // Check Packet #2 crc and discard all data if failed
        if (p2.length() < num_p2_bytes) return;

        // Check crc in Packet #2
        if (!check_crc((uint8_t*) p2.data(), num_p2_bytes-1, rcvd.at(rcvd_len), 0))
        {
            rcvd.clear();
            return;
        }
    }

    // Emit readyRead
    emit readyRead();
}

void GUI_BASE::send(QString data)
{
    send(data.toUtf8());
}

void GUI_BASE::send(QByteArray data)
{
    // Exit if empty data array sent
    if (data.isEmpty()) return;

    // Append to msgList
    msgList.append(data);

    // Lock send to prevent spamming/blocking
    if (!sendLock.tryLock()) return;

    // Send all messages in list
    uint8_t i, j;
    QByteArray currData;
    while (!msgList.isEmpty())
    {
        // Get next data to send
        currData = msgList.takeFirst();
        ack_key = currData.at(p1_major_key_loc);
        ack_status = false;

        // Append crc before sending
        currData.append((char) get_crc((const uint8_t*) currData.data(),
                                       currData.length(), 0));

        // Send data and verify ack
        // Retry packet_retries times
        i = 0;
        do
        {
            // Emit write command to connected device
            emit write_data(currData);

            // Wait to get ack back
            // Can timeout pack_retries times
            j = 0;
            do
            {
                // Wait for ack back
                waitForAck(500);
            } while (!ack_status && (++j < packet_retries));
        } while (!ack_status && (++i < packet_retries));
    }

    // Unlock send
    sendLock.unlock();
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

        // Send read chunk size
        send({
                 GUI_TYPE_DATA_TRANSMIT,
                 (uint8_t) sizeRead,
             });

        // Get read chunck ack

        // Send next chunk
        send(QString(chunkRead));
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

    // Clear buffers (prevents key errors after reset)
    rcvd.clear();
}

void GUI_BASE::waitForAck(int msecs)
{
    // Setup time keeping
    QEventLoop loop;
    QTimer timer;
    connect(this, SIGNAL(ackReady()), &loop, SLOT(quit()));
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));

    // Wait for ackReady or timeout
    timer.start(msecs);
    loop.exec();
}

bool GUI_BASE::checkAck()
{
    // Check ack against inputs
    bool status = false;
    if ((rcvd.at(p1_major_key_loc) == (char) MAJOR_KEY_ACK)
            && (rcvd.at(p1_num_p2_bytes_loc) == (char) ack_key))
    {
        status = true;
    }

    rcvd.remove(0, num_p1_bytes);
    return status;
}
