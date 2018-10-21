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

uint8_t GUI_BASE::chunkSize = 32;
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

    // Connect signals
    connect(this, SIGNAL(readyRead()),
            this, SLOT(receive_gui()));
}

GUI_BASE::~GUI_BASE()
{
}

void GUI_BASE::reset_remote()
{
    send({MAJOR_KEY_RESET, 0, 0});

    // Clear buffers (prevents key errors after reset)
    rcvd_raw.clear();
    rcvd_formatted.clear();
}

void GUI_BASE::set_chunkSize(size_t chunk)
{
    GUI_BASE::chunkSize = chunk;
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

void GUI_BASE::reset_gui()
{
    // Default do nothing
}

void GUI_BASE::receive(QByteArray recvData)
{
    // Add data to recv
    rcvd_raw.append(recvData);
    uint8_t rcvd_len = rcvd_raw.length();
    uint8_t expected_len = num_s1_bytes;

    // Check to see if first stage in rcvd
    if (rcvd_len < expected_len) return;

    // Check to see if it's an Ack packet (no second stage)
    uint32_t checksum_size;
    if (rcvd_raw.at(s1_major_key_loc) == (char) MAJOR_KEY_ACK)
    {
        // Set executable if using
        if (generic_checksum_is_exe) set_executable_checksum_other(generic_checksum_exe_path.toUtf8().constData());

        // Verify enough bytes
        checksum_size = GUI_BASE::generic_checksum.get_checksum_size();
        if (rcvd_len < (expected_len+checksum_size)) return;

        // Check Checksum
        if (!check_checksum((const uint8_t*) rcvd_raw.data(), expected_len, &GUI_BASE::generic_checksum))
        {
            rcvd_raw.clear();
            return;
        }

        // Check & remove ack
        ack_status = checkAck();
        rcvd_raw.remove(0, num_s1_bytes+checksum_size);

        // Emit ack received
        emit ackReceived();
        return;
    }
    // Set executable if using
    if (gui_checksum_is_exe) set_executable_checksum_other(gui_checksum_exe_path.toUtf8().constData());
    checksum_size = gui_checksum.get_checksum_size();

    // Check if second stage & checksum in rcvd
    expected_len += rcvd_raw.at(s1_num_s2_bytes_loc);
    if (rcvd_len < (expected_len+checksum_size)) return;

    // Check Checksum & remove if valid
    if (!check_checksum((const uint8_t*) rcvd_raw.data(), expected_len, &gui_checksum))
    {
        rcvd_raw.clear();
        return;
    } else
    {
        rcvd_raw.remove(expected_len, checksum_size);
    }

    // Ack back
    send({
             MAJOR_KEY_ACK,
             (uint8_t) rcvd_raw.at(0),
             0
         });

    // Emit readyRead
    emit readyRead();
}

void GUI_BASE::receive_gui()
{
    // Default just clear rcvd_raw
    rcvd_raw.clear();
}

void GUI_BASE::send(QString data)
{
    send(data.toUtf8());
}

void GUI_BASE::send(QByteArray data)
{
    transmit(data);
}

void GUI_BASE::send(std::initializer_list<uint8_t> data)
{
    send(GUI_HELPER::initList2ByteArray(data));
}

void GUI_BASE::send_file(QByteArray start, QString filePath)
{
    send_chunk(start, GUI_HELPER::loadFile(filePath));
}

void GUI_BASE::send_file_chunked(QByteArray start, QString filePath, char sep)
{
    foreach (QByteArray chunk, GUI_HELPER::loadFile(filePath).split(sep))
    {
        send_chunk(start, chunk);
    }
}

void GUI_BASE::send_chunk(QByteArray start, QByteArray chunk)
{
    QByteArray data, curr;
    uint32_t pos = 0;
    uint32_t end_pos = chunk.length();
    while (pos < end_pos)
    {
        // Clear data and add start array
        data.clear();
        data.append(start);

        // Get next data chunk and add info
        curr = chunk.mid(pos, chunkSize);
        data.append((char) curr.length());
        data.append(curr);

        // Transmit data to device
        transmit(data);

        // Increment position counter
        pos += chunkSize;
    }
}

void GUI_BASE::send_chunk(std::initializer_list<uint8_t> start, QByteArray chunk)
{
    send_chunk(
                GUI_HELPER::initList2ByteArray(start),
                chunk
                );
}

void GUI_BASE::send_chunk(QByteArray start, std::initializer_list<uint8_t> chunk)
{
    send_chunk(
                start,
                GUI_HELPER::initList2ByteArray(chunk)
                );
}

void GUI_BASE::send_chunk(std::initializer_list<uint8_t> start, std::initializer_list<uint8_t> chunk)
{
    send_chunk(
                GUI_HELPER::initList2ByteArray(start),
                GUI_HELPER::initList2ByteArray(chunk)
                );
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
    if ((rcvd_raw.at(s1_major_key_loc) == (char) MAJOR_KEY_ACK)
            && (rcvd_raw.at(s1_minor_key_loc) == (char) ack_key))
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

void GUI_BASE::save_rcvd_formatted()
{
    // Select file save location
    QString fileName;
    if (!GUI_HELPER::getSaveFilePath(&fileName))
        return;

    // Save file
    if (!GUI_HELPER::saveFile(fileName, rcvd_formatted))
        GUI_HELPER::showMessage("ERROR: Failed to save file!");
}

void GUI_BASE::transmit(QByteArray data)
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

        // If sending an ack, just send and continue
        if (currData.at(0) == MAJOR_KEY_ACK)
        {
            emit write_data(currData);
            continue;
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
