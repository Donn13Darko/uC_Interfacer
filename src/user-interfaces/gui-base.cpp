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
checksum_struct GUI_BASE::generic_checksum = DEFAULT_CHECKSUM_STRUCT;
QByteArray GUI_BASE::generic_checksum_start = QByteArray();

// Setup suported checksums map
QMap<QString, checksum_struct>
GUI_BASE::supportedChecksums({
                                 {"CRC_8_LUT", {get_crc_8_LUT_size, get_crc_8_LUT, check_crc_8_LUT, nullptr, nullptr}},
                                 {"CRC_8_POLY", {get_crc_8_POLY_size, get_crc_8_POLY, check_crc_8_POLY, nullptr, nullptr}},
                                 {"CRC_16_LUT", {get_crc_16_LUT_size, get_crc_16_LUT, check_crc_16_LUT, nullptr, nullptr}},
                                 {"CRC_16_POLY", {get_crc_16_POLY_size, get_crc_16_POLY, check_crc_16_POLY, nullptr, nullptr}},
                                 {"CRC_32_LUT", {get_crc_32_LUT_size, get_crc_32_LUT, check_crc_32_LUT, nullptr, nullptr}},
                                 {"CRC_32_POLY", {get_crc_32_POLY_size, get_crc_32_POLY, check_crc_32_POLY, nullptr, nullptr}},
                                 {"CHECKSUM_EXE", {get_checksum_exe_size, get_checksum_exe, check_checksum_exe, nullptr, nullptr}}
                             });

uint8_t num_s2_bytes;

GUI_BASE::GUI_BASE(QWidget *parent) :
    QWidget(parent)
{
    // Init general variables
    reset_dev = false;
    exit_dev = false;
    send_closed = false;
    recv_closed = false;

    // Init Ack variables
    ack_status = false;
    ack_key = MAJOR_KEY_ERROR;
    ackTimer.setSingleShot(true);

    // Init Data variables
    data_status = false;
    data_key = MAJOR_KEY_ERROR;
    dataTimer.setSingleShot(true);

    // Init gui checksum variables
    gui_checksum_is_exe = false;
    gui_checksum = DEFAULT_CHECKSUM_STRUCT;
    gui_checksum_exe_path = "";
    gui_checksum_start = {};

    // Connect read signals and slots
    connect(this, SIGNAL(readyRead(QByteArray)),
            this, SLOT(receive_gui(QByteArray)),
            Qt::QueuedConnection);

    // Connect data loop signals and slots
    connect(this, SIGNAL(readyRead(QByteArray)),
            &dataLoop, SLOT(quit()),
            Qt::QueuedConnection);
    connect(this, SIGNAL(resetting()),
            &dataLoop, SLOT(quit()),
            Qt::QueuedConnection);

    // Connect ack loop signals and slots
    connect(this, SIGNAL(ackReceived(QByteArray)),
            this, SLOT(checkAck(QByteArray)),
            Qt::QueuedConnection);
    connect(this, SIGNAL(ackChecked(bool)),
            &ackLoop, SLOT(quit()),
            Qt::QueuedConnection);
    connect(&ackTimer, SIGNAL(timeout()),
            &ackLoop, SLOT(quit()),
            Qt::QueuedConnection);
    connect(this, SIGNAL(resetting()),
            &ackLoop, SLOT(quit()),
            Qt::QueuedConnection);
}

GUI_BASE::~GUI_BASE()
{
    exit_dev = true;
}

void GUI_BASE::closing()
{
    // Set exit to true
    exit_dev = true;

    // Reset send
    reset_remote();

    // See if ready to exit
    close_base();
}

void GUI_BASE::reset_remote()
{
    // Enter reset
    reset_dev = true;

    // Clear any pending messages
    msgList.clear();

    // Emit resetting to free timers
    emit resetting();

    // Load reset CMD into msgList
    send({MAJOR_KEY_RESET, 0, 0});

    // Exiting here returngs control to the main event loop
    // Required in order for the next packet to be sent
    // if currently in waitForAck or waitForData
}

void GUI_BASE::set_gui_checksum(QStringList new_gui_checksum)
{
    // Set new checksum information
    gui_checksum_exe_path = new_gui_checksum.at(checksum_exe_pos) + '\0';
    gui_checksum = supportedChecksums.value(
                new_gui_checksum.at(checksum_name_pos),
                DEFAULT_CHECKSUM_STRUCT
                );
    gui_checksum_is_exe = (gui_checksum.get_checksum == get_checksum_exe);

    // Load in new checksum start
    gui_checksum_start.clear();
    uint8_t base = new_gui_checksum.at(checksum_start_base_pos).toInt();
    QStringList new_gui_checksum_start = new_gui_checksum.at(checksum_start_pos).split(',');
    for (auto byte = new_gui_checksum_start.cbegin(); byte != new_gui_checksum_start.cend(); byte++)
    {
        gui_checksum_start.append((char) byte->toInt(nullptr, base));
    }

    // Pad checksum start if not long enough
    if (gui_checksum_is_exe)
    {
        gui_checksum.checksum_exe = gui_checksum_exe_path.toUtf8().constData();
        set_executable_checksum_exe(gui_checksum.checksum_exe);
    }
    uint32_t checksum_size = gui_checksum.get_checksum_size();
    for (uint32_t i = gui_checksum_start.length(); i < checksum_size; i++)
    {
        gui_checksum_start.prepend((char) 0);
    }
    gui_checksum.checksum_start = (const uint8_t*) gui_checksum_start.constData();
}

QStringList GUI_BASE::get_supported_checksums()
{
    return supportedChecksums.keys();
}

void GUI_BASE::set_generic_checksum(QStringList new_generic_checksum)
{
    // Set new checksum information
    GUI_BASE::generic_checksum_exe_path = new_generic_checksum.at(checksum_exe_pos) + '\0';
    GUI_BASE::generic_checksum = supportedChecksums.value(
                new_generic_checksum.at(checksum_name_pos),
                DEFAULT_CHECKSUM_STRUCT
                );
    GUI_BASE::generic_checksum_is_exe = (GUI_BASE::generic_checksum.get_checksum == get_checksum_exe);

    // Load in new checksum start
    GUI_BASE::generic_checksum_start.clear();
    uint8_t base = new_generic_checksum.at(checksum_start_base_pos).toInt();
    QStringList new_generic_checksum_start = new_generic_checksum.at(checksum_start_pos).split(',');
    for (auto byte = new_generic_checksum_start.cbegin(); byte != new_generic_checksum_start.cend(); byte++)
    {
        GUI_BASE::generic_checksum_start.append((char) byte->toInt(nullptr, base));
    }

    // Pad checksum if not long enough
    if (GUI_BASE::generic_checksum_is_exe)
    {
        GUI_BASE::generic_checksum.checksum_exe = GUI_BASE::generic_checksum_exe_path.toUtf8().constData();
        set_executable_checksum_exe(GUI_BASE::generic_checksum.checksum_exe);
    }
    uint32_t checksum_size = GUI_BASE::generic_checksum.get_checksum_size();
    for (uint32_t i = GUI_BASE::generic_checksum_start.length(); i < checksum_size; i++)
    {
        GUI_BASE::generic_checksum_start.prepend((char) 0);
    }
    GUI_BASE::generic_checksum.checksum_start = (const uint8_t*) GUI_BASE::generic_checksum_start.constData();
}

void GUI_BASE::parseGenericConfigMap(QMap<QString, QVariant>* configMap)
{
    // Holder for each setting
    QVariant setting;

    // Check general checksum type setting
    // (only sets if not set at runtime in more options)
    setting = configMap->value("checksum_set");
    if (!setting.isNull())
    {
        // Set generic checksum type
        GUI_BASE::set_generic_checksum(setting.toStringList());
    }

    // Set chunk size
    setting = configMap->value("chunk_size");
    if (!setting.isNull())
    {
        GUI_BASE::set_chunk_size(setting.toUInt());
    }
}

size_t GUI_BASE::get_chunk_size()
{
    return GUI_BASE::chunk_size;
}

void GUI_BASE::set_chunk_size(size_t chunk)
{
    GUI_BASE::chunk_size = chunk;
}

void GUI_BASE::reset_gui()
{
    // Default do nothing
}

uint8_t GUI_BASE::get_GUI_key()
{
    return gui_key;
}

void GUI_BASE::parseConfigMap(QMap<QString, QVariant>* configMap)
{
    // Default variable for holding each setting
    QVariant setting;

    // Get gui type checksum
    setting = configMap->value("checksum_set");
    if (!setting.isNull())
    {
        // Set checksum type struct
        set_gui_checksum(setting.toStringList());
    }
}

void GUI_BASE::receive(QByteArray recvData)
{
    // Check if recieving empty data array or exiting
    if (recvData.isEmpty() || exit_dev) return;

    // Add data to recv
    rcvd_raw.append(recvData);

    // Lock recv to prevent spamming/blocking
    if (!recvLock.tryLock()) return;

    // Loop until break or rcvd is empty
    uint8_t expected_len;
    uint8_t rcvd_len = rcvd_raw.length();
    while ((0 < rcvd_len) && !exit_dev)
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
            if (!check_checksum((const uint8_t*) rcvd_raw.constData(), expected_len, &GUI_BASE::generic_checksum))
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
            if (!check_checksum((const uint8_t*) rcvd_raw.constData(), expected_len, &gui_checksum))
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

    // If exiting, check if ready
    if (exit_dev) close_base();
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

    // Setup base variables
    QByteArray data, curr;
    uint32_t pos = 0;
    uint32_t end_pos = chunk.length();

    // Send start of data chunk
    // (if multiple packets are required)
    if (chunk_size < end_pos)
    {
        data.append(start);
        data.append((char) 0);
        transmit(data);
    }

    // Send data chunk
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

    // Send end of data chunk
    // (if multiple packets were required)
    if (chunk_size < end_pos)
    {
        data.clear();
        data.append(start);
        data.append((char) 0);
        transmit(data);
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
    getChecksum((const uint8_t*) ack_packet.constData(), ack_packet.length(),
                ack_key, &checksum_array, &checksum_size);

    // Append checksum
    ack_packet.append((const char*) checksum_array, checksum_size);

    // Delete checksum array (done using)
    delete[] checksum_array;

    // Send ack immediately
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
    // Check if trying to send empty data array or exiting
    if (data.isEmpty() || exit_dev) return;

    // Don't load message if resetting and it isn't a reset
    bool isReset = (data.at(s1_major_key_loc) == (char) MAJOR_KEY_RESET);
    if (reset_dev && !isReset) return;

    // Append to msgList
    msgList.append(data);

    // Lock send to prevent spamming/blocking
    if (!sendLock.tryLock()) return;

    // Send all messages in list
    QByteArray currData;
    uint8_t* checksum_array;
    uint32_t checksum_size = 0;
    while (!msgList.isEmpty() && !exit_dev)
    {
        // Get next data to send
        currData = msgList.takeFirst();
        ack_key = currData.at(s1_major_key_loc);
        ack_status = false;
        data_status = false;
        isReset = (currData.at(s1_major_key_loc) == (char) MAJOR_KEY_RESET);

        // Get checksum
        getChecksum((const uint8_t*) currData.constData(), currData.length(),
                    ack_key, &checksum_array, &checksum_size);

        // Append checksum
        currData.append((const char*) checksum_array, checksum_size);

        // Delete checksum array (done using)
        delete[] checksum_array;

        // Send data and verify ack
        do
        {
            // Emit write command to connected device
            emit write_data(currData);

            // Wait for CMD ack back
            waitForAck(packet_timeout);
        } while (!ack_status && (!reset_dev || isReset) && !exit_dev);

        // Wait for data read if CMD success and was data request
        // Resets will never request data back (only an ack)
        if (ack_status
                && !reset_dev
                && !exit_dev
                && isDataRequest(currData.at(s1_minor_key_loc)))
        {
            do
            {
                // Wait for data packet back
                waitForData(packet_timeout);
            } while (!data_status && !reset_dev && !exit_dev);
        }

        // Check if reseting and was reset
        if (reset_dev && isReset)
        {
            // Clear buffers (prevents key errors after reset)
            rcvd_raw.clear();
            rcvd_formatted.clear();

            // Reset reset flag
            reset_dev = false;
        }
    }

    // Unlock send lock if not exiting
    sendLock.unlock();

    // If exiting, check if ready
    if (exit_dev) close_base();
}

void GUI_BASE::getChecksum(const uint8_t* data, uint8_t data_len, uint8_t checksum_key,
                           uint8_t** checksum_array, uint32_t* checksum_size)
{
    // Get checksum
    checksum_struct check;
    if (checksum_key == (char) gui_key)
    {
        // Set checksum info
        check = gui_checksum;
    } else
    {
        // Set checksum info
        check = GUI_BASE::generic_checksum;
    }
    // Set executable if using
    if (check.checksum_exe)
        set_executable_checksum_exe(check.checksum_exe);

    // Get size
    *checksum_size = check.get_checksum_size();

    // Malloc checksum array
    *checksum_array = (uint8_t*) malloc((*checksum_size)*sizeof(uint8_t));
    memset(*checksum_array, 0, *checksum_size);

    check.get_checksum(data, data_len, check.checksum_start, *checksum_array);
}

void GUI_BASE::close_base()
{
    // Ensure exit set
    if (!exit_dev) return;

    // Test send & recv locks (non-block)
    if (!send_closed) send_closed = sendLock.tryLock();
    if (!recv_closed) recv_closed = recvLock.tryLock();
    if (!(send_closed && recv_closed)) return;

    // Close widget if able to aquire both
    while (!close());

    // Release both locks
    sendLock.unlock();
    recvLock.unlock();
}
