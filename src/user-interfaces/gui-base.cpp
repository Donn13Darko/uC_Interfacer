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
#include <QFileDialog>
#include <QTextStream>
#include <QSettings>
#include <QProcess>

uint8_t GUI_BASE::chunk_size = GUI_BASE::default_chunk_size;
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
    ackTimer.setSingleShot(true);

    // Init Data variables
    data_status = false;
    data_key = MAJOR_KEY_ERROR;
    dataTimer.setSingleShot(true);

    // Connect signals
    connect(this, SIGNAL(readyRead(QByteArray)),
            this, SLOT(receive_gui(QByteArray)));
    connect(this, SIGNAL(readyRead(QByteArray)),
            &dataLoop, SLOT(quit()));
    connect(&dataTimer, SIGNAL(timeout()),
            &dataLoop, SLOT(quit()));

    connect(this, SIGNAL(ackReceived(QByteArray)),
            this, SLOT(checkAck(QByteArray)));
    connect(this, SIGNAL(ackChecked(bool)),
            &ackLoop, SLOT(quit()));
    connect(&ackTimer, SIGNAL(timeout()),
            &ackLoop, SLOT(quit()));
}

GUI_BASE::~GUI_BASE()
{
}

void GUI_BASE::reset_remote()
{
    // Clear msg list
    msgList.clear();

    // Send reset command
    send({MAJOR_KEY_RESET, 0, 0});

    // Clear buffers (prevents key errors after reset)
    rcvd_raw.clear();
    rcvd_formatted.clear();
}

void GUI_BASE::set_chunk_size(size_t chunk)
{
    GUI_BASE::chunk_size = chunk;
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

uint8_t GUI_BASE::get_GUI_type()
{
    return gui_type;
}

void GUI_BASE::receive(QByteArray recvData)
{
    // Add data to recv
    rcvd_raw.append(recvData);

    // Lock recv to prevent spamming/blocking
    if (!recvLock.tryLock()) return;

    // Loop until break or rcvd is empty
    uint8_t expected_len;
    uint8_t rcvd_len = rcvd_raw.length();
    while (0 < rcvd_len)
    {
        // Check to see if first stage in rcvd
        expected_len = num_s1_bytes;
        if (rcvd_len < expected_len) break;

        // Check to see if it's an Ack packet (no second stage)
        uint32_t checksum_size;
        if (rcvd_raw.at(s1_major_key_loc) == (char) MAJOR_KEY_ACK)
        {
            // Set generic checksum executable if using
            if (generic_checksum_is_exe)
                set_executable_checksum_exe(generic_checksum_exe_path.toUtf8().constData());

            // Verify enough bytes
            checksum_size = GUI_BASE::generic_checksum.get_checksum_size();
            if (rcvd_len < (expected_len+checksum_size)) break;

            // Check Checksum
            if (!check_checksum((const uint8_t*) rcvd_raw.data(), expected_len, &GUI_BASE::generic_checksum))
            {
                rcvd_raw.clear();
                break;
            }

            // Emit ack received
            emit ackReceived(rcvd_raw.mid(0, expected_len));

            // Remove ack
            rcvd_raw.remove(0, expected_len+checksum_size);
        } else
        {
            // Set gui checksum executable if using
            if (gui_checksum_is_exe) set_executable_checksum_exe(gui_checksum_exe_path.toUtf8().constData());
            checksum_size = gui_checksum.get_checksum_size();

            // Check if second stage & checksum in rcvd
            expected_len += rcvd_raw.at(s1_num_s2_bytes_loc);
            if (rcvd_len < (expected_len+checksum_size)) break;

            // Check Checksum
            if (!check_checksum((const uint8_t*) rcvd_raw.data(), expected_len, &gui_checksum))
            {
                // Clear rcvd if error
                rcvd_raw.clear();

                // Ack error
                send_ack(MAJOR_KEY_ERROR);
                break;
            }

            // Ack success & set data status
            send_ack((uint8_t) rcvd_raw.at(s1_major_key_loc));
            data_status = true;

            // Emit readyRead
            emit readyRead(rcvd_raw.left(expected_len));

            // Remove data from rcvd_raw
            rcvd_raw.remove(0, expected_len+checksum_size);
        }

        // Update rcvd_len
        rcvd_len = rcvd_raw.length();
    }

    // Unlock recv lock
    recvLock.unlock();
}

void GUI_BASE::receive_gui(QByteArray)
{
    // Default do nothing
}

void GUI_BASE::on_ResetGUI_Button_clicked()
{
    // Reset the GUI
    reset_gui();

    // Reset the Remote
    reset_remote();
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
    // Verify this will terminate
    if (chunk_size == 0) return;

    // Parse and send data
    QByteArray data, curr;
    uint32_t pos = 0;
    uint32_t end_pos = chunk.length();
    while (pos < end_pos)
    {
        // Clear data and add start array
        data.clear();
        data.append(start);

        // Get next data chunk and add info
        curr = chunk.mid(pos, chunk_size);
        data.append((char) curr.length());
        data.append(curr);

        // Transmit data to device
        transmit(data);

        // Increment position counter
        pos += chunk_size;
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

void GUI_BASE::send_ack(uint8_t majorKey)
{
    // Build ack packet
    QByteArray ack_packet;
    ack_packet.append((char) MAJOR_KEY_ACK);
    ack_packet.append((char) majorKey);
    ack_packet.append((char) 0);

    // Get checksum
    uint8_t* checksum_array;
    uint32_t checksum_size = 0;
    getChecksum((const uint8_t*) ack_packet.data(), ack_packet.length(),
                ack_key, nullptr, &checksum_array, &checksum_size);

    // Append checksum
    ack_packet.append((const char*) checksum_array, checksum_size);

    // Delete checksum array (done using)
    delete[] checksum_array;

    // Send ack immediatly
    emit write_data(ack_packet);
}

void GUI_BASE::waitForAck(int msecs)
{
    // Wait for achChecked or timeout
    ackTimer.start(msecs);
    ackLoop.exec();

    // Stop timer
    ackTimer.stop();
}

void GUI_BASE::checkAck(QByteArray ack)
{
    // Check ack against inputs
    ack_status = ((ack.at(s1_major_key_loc) == (char) MAJOR_KEY_ACK)
            && (ack.at(s1_minor_key_loc) == (char) ack_key));

    // Emit checked signal
    emit ackChecked(ack_status);
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

void GUI_BASE::waitForData(int msecs)
{
    // Wait for dataReceived or timeout
    dataTimer.start(msecs);
    dataLoop.exec();

    // Stop timer
    dataTimer.stop();
}

bool GUI_BASE::isDataRequest(uint8_t)
{
    return false;
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
    uint8_t i;
    QByteArray currData;
    uint8_t* checksum_array;
    uint32_t checksum_size = 0;
    while (!msgList.isEmpty())
    {
        // Get next data to send
        currData = msgList.takeFirst();
        ack_key = currData.at(s1_major_key_loc);
        ack_status = false;

        // Get checksum
        getChecksum((const uint8_t*) currData.data(), currData.length(),
                    ack_key, nullptr, &checksum_array, &checksum_size);

        // Append checksum
        currData.append((const char*) checksum_array, checksum_size);

        // Delete checksum array (done using)
        delete[] checksum_array;

        // Send data and verify ack
        // Retry packet_retries times
        i = 0;
        do
        {
            // Emit write command to connected device
            emit write_data(currData);

            // Wait for CMD ack back
            waitForAck(packet_timeout);
        } while (!ack_status && (++i < packet_retries));

        // Wait for data read if CMD success and was request
        if (ack_status && isDataRequest(currData.at(s1_minor_key_loc)))
        {
            i = 0;
            data_status = false;
            do
            {
                // Wait for data packet back
                waitForData(packet_timeout);
            } while (!data_status && (++i < packet_retries));
        }
    }

    // Unlock send lock
    sendLock.unlock();
}

void GUI_BASE::getChecksum(const uint8_t* data, uint8_t data_len,
                           uint8_t checksum_key, uint8_t* checksum_start,
                           uint8_t** checksum_array, uint32_t* checksum_size)
{
    // Get checksum
    checksum_struct check;
    if (checksum_key == (char) gui_type)
    {
        // Set executable if using
        if (gui_checksum_is_exe)
            set_executable_checksum_exe(gui_checksum_exe_path.toUtf8().constData());

        // Set checksum info
        check = gui_checksum;
    } else
    {
        // Set executable if using
        if (generic_checksum_is_exe)
            set_executable_checksum_exe(generic_checksum_exe_path.toUtf8().constData());

        // Set checksum info
        check = GUI_BASE::generic_checksum;
    }

    // Get size
    *checksum_size = check.get_checksum_size();

    // Malloc checksum array
    *checksum_array = (uint8_t*) malloc((*checksum_size)*sizeof(uint8_t));
    memset(*checksum_array, 0, *checksum_size);

    // Check if supplied a checksum_start
    if (checksum_start == nullptr) checksum_start = *checksum_array;

    check.get_checksum(data, data_len, checksum_start, *checksum_array);
}
