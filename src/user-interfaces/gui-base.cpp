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
#include <QProcess>

bool GUI_BASE::generic_checksum_is_exe = false;
QString GUI_BASE::generic_checksum_exe_path = "";
checksum_struct GUI_BASE::generic_checksum{get_crc_8_LUT_size, get_crc_8_LUT, check_crc_8_LUT};

uint8_t num_s2_bytes;

GUI_BASE::GUI_BASE(QWidget *parent) :
    QWidget(parent)
{
    // Init Ack variables
    ack_status = false;
    ack_key = MAJOR_KEY_ERROR;

    // Init checksum variables

}

GUI_BASE::~GUI_BASE()
{
}

void GUI_BASE::reset_remote()
{
    send({MAJOR_KEY_RESET, 0});

    // Clear buffers (prevents key errors after reset)
    rcvd.clear();
}

void GUI_BASE::set_chunkSize(size_t chunk)
{
    chunkSize = chunk;
}

void GUI_BASE::set_gui_checksum(QString new_gui_checksum)
{
    gui_checksum_is_exe = true;
    gui_checksum_exe_path = new_gui_checksum;
}

void GUI_BASE::set_gui_checksum(checksum_struct new_gui_checksum)
{
    gui_checksum = new_gui_checksum;
}

void GUI_BASE::set_generic_checksum(QString new_generic_checksum)
{
    GUI_BASE::generic_checksum_is_exe = true;
    GUI_BASE::generic_checksum_exe_path = new_generic_checksum;
}

void GUI_BASE::set_generic_checksum(checksum_struct new_generic_checksum)
{
    GUI_BASE::generic_checksum = new_generic_checksum;
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
    uint32_t checksum_size;
    if (rcvd.at(s1_major_key_loc) == (char) MAJOR_KEY_ACK)
    {
        // Set executable if using
        if (generic_checksum_is_exe) set_executable_checksum_other(generic_checksum_exe_path.toUtf8().constData());

        // Verify enough bytes
        checksum_size = GUI_BASE::generic_checksum.get_checksum_size();
        if (rcvd_len < (expected_len+checksum_size)) return;

        // Check Checksum
        if (!check_checksum((const uint8_t*) rcvd.data(), expected_len, &GUI_BASE::generic_checksum))
        {
            rcvd.clear();
            return;
        }

        // Check & remove ack
        ack_status = checkAck();
        rcvd.remove(0, num_s1_bytes+checksum_size);

        // Emit ack received
        emit ackReceived();
        return;
    }
    // Set executable if using
    if (gui_checksum_is_exe) set_executable_checksum_other(gui_checksum_exe_path.toUtf8().constData());
    checksum_size = gui_checksum.get_checksum_size();

    // Check if second stage & checksum in rcvd
    expected_len += rcvd.at(s1_num_s2_bytes_loc);
    if (rcvd_len < (expected_len+checksum_size)) return;

    // Check crc in packet
    // Check Checksum
    if (!check_checksum((const uint8_t*) rcvd.data(), expected_len, &gui_checksum))
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
    QByteArray currData;
    while (!msgList.isEmpty())
    {
        // Get next data to send
        currData = msgList.takeFirst();
        ack_key = currData.at(s1_major_key_loc);
        ack_status = false;

        // Append checksum before sending
        if (ack_key == (char) guiType)
        {
            // Set executable if using
            if (gui_checksum_is_exe) set_executable_checksum_other(gui_checksum_exe_path.toUtf8().constData());

            // Get checksum info
            uint32_t checksum_size = gui_checksum.get_checksum_size();
            uint8_t checksumArray[checksum_size];
            uint8_t checksum_start[checksum_size] = {0};
            gui_checksum.get_checksum((const uint8_t*) currData.data(), currData.length(),
                                             checksum_start, checksumArray);

            // Append checksum
            currData.append((const char*) checksumArray, checksum_size);
        } else
        {
            // Set executable if using
            if (generic_checksum_is_exe) set_executable_checksum_other(generic_checksum_exe_path.toUtf8().constData());

            // Get checksum info
            uint32_t checksum_size = GUI_BASE::generic_checksum.get_checksum_size();
            uint8_t checksumArray[checksum_size];
            uint8_t checksum_start[checksum_size] = {0};
            GUI_BASE::generic_checksum.get_checksum((const uint8_t*) currData.data(), currData.length(),
                                             checksum_start, checksumArray);

            // Append checksum
            currData.append((const char*) checksumArray, checksum_size);
        }

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
                waitForAck(packet_timeout);
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
            && (rcvd.at(s1_minor_key_loc) == (char) ack_key))
    {
        status = true;
    }

    return status;
}

bool GUI_BASE::check_checksum(const uint8_t* data, uint32_t data_len, checksum_struct* check)
{
    // Create checksum variables
    uint32_t checksum_size = check->get_checksum_size();
    uint8_t checksum_start[checksum_size] = {0};
    uint8_t fsm_checksum_cmp_buffer[checksum_size];

    // Compute checksum on data
    check->get_checksum(data, data_len, checksum_start, fsm_checksum_cmp_buffer);

    // Compare generated to received checksum
    return check->check_checksum(data+data_len, fsm_checksum_cmp_buffer);
}
