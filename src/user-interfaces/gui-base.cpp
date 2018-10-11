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

uint8_t num_s1_bytes = s1_crc_loc;
uint8_t num_s2_bytes;

GUI_BASE::GUI_BASE(QWidget *parent) :
    QWidget(parent)
{
    // Init Ack variables
    ack_status = false;
    ack_key = MAJOR_KEY_ERROR;

    // Init CRC variables
    crc_cmp = 0;
    crc_start = 0;
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
    uint8_t expected_len = num_s1_bytes;

    // Check to see if first stage in rcvd
    if (rcvd_len < expected_len) return;

    // Check to see if it's an Ack packet (no second stage)
    if (rcvd.at(s1_major_key_loc) == (char) MAJOR_KEY_ACK)
    {
        // Verify enough bytes
        if (rcvd_len < (expected_len+crc_size)) return;

        // Check crc
        crc_cmp = build_crc((const uint8_t*) rcvd.mid(expected_len, crc_size).data());
        if (!check_crc((const uint8_t*) rcvd.data(), expected_len, crc_cmp, crc_start))
        {
            rcvd.clear();
            return;
        }

        // Check ack
        ack_status = checkAck();

        // Emit ack received
        emit ackReceived();
        return;
    }

    // Check if second stage & crc in rcvd
    expected_len += rcvd.at(s1_num_s2_bytes_loc);
    if (rcvd_len < (expected_len+crc_size)) return;

    // Check crc in packet
    crc_cmp = build_crc((const uint8_t*) rcvd.mid(expected_len, crc_size).data());
    if (!check_crc((const uint8_t*) rcvd.data(), expected_len, crc_cmp, crc_start))
    {
        rcvd.clear();
        return;
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
    crc_t msg_crc;
    QByteArray currData;
    uint8_t crcArray[crc_size];
    while (!msgList.isEmpty())
    {
        // Get next data to send
        currData = msgList.takeFirst();
        ack_key = currData.at(s1_major_key_loc);
        ack_status = false;

        // Append crc before sending
        msg_crc = get_crc((const uint8_t*) currData.data(),
                          currData.length(), 0);
        build_byte_array(msg_crc, (uint8_t*) crcArray);
        currData.append((const char*) crcArray, crc_size);

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
    connect(this, SIGNAL(ackReceived()), &loop, SLOT(quit()));
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));

    // Wait for ackReceived or timeout
    timer.start(msecs);
    loop.exec();
}

bool GUI_BASE::checkAck()
{
    // Check ack against inputs
    bool status = false;
    if ((rcvd.at(s1_major_key_loc) == (char) MAJOR_KEY_ACK)
            && (rcvd.at(s1_num_s2_bytes_loc) == (char) ack_key))
    {
        status = true;
    }

    rcvd.remove(0, num_s1_bytes+crc_size);
    return status;
}
