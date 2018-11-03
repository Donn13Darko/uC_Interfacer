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

#include "gui-comm-bridge.h"

// Setup suported checksums map
QMap<QString, checksum_struct>
GUI_COMM_BRIDGE::supportedChecksums({
            {"CRC_8_LUT", {get_crc_8_LUT_size, get_crc_8_LUT, check_crc_8_LUT, 0, 0, 0}},
            {"CRC_8_POLY", {get_crc_8_POLY_size, get_crc_8_POLY, check_crc_8_POLY, 0, 0, 0}},
            {"CRC_16_LUT", {get_crc_16_LUT_size, get_crc_16_LUT, check_crc_16_LUT, 0, 0, 0}},
            {"CRC_16_POLY", {get_crc_16_POLY_size, get_crc_16_POLY, check_crc_16_POLY, 0, 0, 0}},
            {"CRC_32_LUT", {get_crc_32_LUT_size, get_crc_32_LUT, check_crc_32_LUT, 0, 0, 0}},
            {"CRC_32_POLY", {get_crc_32_POLY_size, get_crc_32_POLY, check_crc_32_POLY, 0, 0, 0}},
            {"CHECKSUM_EXE", {get_checksum_exe_size, get_checksum_exe, check_checksum_exe, 0, 0, 1}}
        });

uint32_t num_s2_bytes;

GUI_COMM_BRIDGE::GUI_COMM_BRIDGE(uint8_t num_guis, QObject *parent) :
    QObject(parent)
{
    // Setup base flags
    base_flags = 0x00;

    // Setup Ack variables
    ack_status = false;
    ack_key = MAJOR_KEY_ERROR;

    // Setup Data variables
    data_status = false;
    data_key = MAJOR_KEY_ERROR;

    // Set generic defaults
    chunk_size = GUI_COMM_BRIDGE::default_chunk_size;

    // Init storage lists
    for (uint8_t i = 0; i < num_guis; i++)
    {
        // Set default checksum
        gui_checksums.append(DEFAULT_CHECKSUM_STRUCT);

        // Add known guis
        known_guis.append(QList<GUI_BASE*>());
    }

    // Connect data loop signals and slots
    connect(this, SIGNAL(readyRead(QByteArray)),
            &dataLoop, SLOT(quit()),
            Qt::DirectConnection);
    connect(&dataTimer, SIGNAL(timeout()),
            &dataLoop, SLOT(quit()),
            Qt::DirectConnection);
    connect(this, SIGNAL(reset()),
            &dataLoop, SLOT(quit()),
            Qt::DirectConnection);

    // Connect ack loop signals and slots
    connect(this, SIGNAL(ackReceived(QByteArray)),
            this, SLOT(checkAck(QByteArray)),
            Qt::DirectConnection);
    connect(this, SIGNAL(ackChecked(bool)),
            &ackLoop, SLOT(quit()),
            Qt::DirectConnection);
    connect(&ackTimer, SIGNAL(timeout()),
            &ackLoop, SLOT(quit()),
            Qt::DirectConnection);
    connect(this, SIGNAL(reset()),
            &ackLoop, SLOT(quit()),
            Qt::DirectConnection);
}

GUI_COMM_BRIDGE::~GUI_COMM_BRIDGE()
{
    // Delete each GUIs checksum info
    foreach (checksum_struct gui_checksum, gui_checksums)
        delete_checksum_info(&gui_checksum);
}

size_t GUI_COMM_BRIDGE::get_chunk_size()
{
    return chunk_size;
}

void GUI_COMM_BRIDGE::set_chunk_size(size_t chunk)
{
    chunk_size = chunk;
}

QStringList GUI_COMM_BRIDGE::get_supported_checksums()
{
    return supportedChecksums.keys();
}

void GUI_COMM_BRIDGE::set_gui_checksum(uint8_t gui_key, QStringList new_gui_checksum)
{
    // Get default struct
    checksum_struct default_check = supportedChecksums.value(
                new_gui_checksum.at(checksum_name_pos),
                DEFAULT_CHECKSUM_STRUCT
                );

    // Copy default information
    copy_checksum_info(&gui_checksum, &default_check);

    // Copy executable path if new checksum is exe & exe provided
    set_checksum_exe(&gui_checksum,
                     new_gui_checksum.at(checksum_exe_pos));

    // If checksum_start provided, overwrite default
    set_checksum_start(&gui_checksum,
                       new_gui_checksum.at(checksum_start_pos),
                       new_gui_checksum.at(checksum_start_base_pos).toInt());
}

void GUI_COMM_BRIDGE::parseGenericConfigMap(QMap<QString, QVariant>* configMap)
{
    // Holder for each setting
    QVariant setting;

    // Check general checksum type setting
    // (only sets if not set at runtime in more options)
    setting = configMap->value("checksum_set");
    if (!setting.isNull())
    {
        // Set generic checksum type
        set_gui_checksum(MAJOR_KEY_GENERAL_SETTINGS, setting.toStringList());
    }

    // Set chunk size
    setting = configMap->value("chunk_size");
    if (!setting.isNull())
    {
        set_chunk_size(setting.toUInt());
    }
}

void GUI_COMM_BRIDGE::add_gui(GUI_BASE *new_gui)
{
    uint8_t gui_list_pos = (new_gui->get_GUI_key() >> s1_major_key_bit_shift)-1;
    if (known_guis.length() < gui_list_pos) return;

    known_guis.at(gui_list_pos).append(new_gui);
}

void GUI_COMM_BRIDGE::receive(QByteArray recvData)
{
    // Check if recieving empty data array or exiting
    if (recvData.isEmpty() || (base_flags & base_exit_flag)) return;

    // Add data to recv
    rcvd_raw.append(recvData);

    // Lock recv to prevent spamming/blocking
    if (!rcvLock.tryLock()) return;

    // Setup loop variables
    bool exit_recv = false;
    uint8_t major_key, minor_key, bits;
    uint32_t expected_len, checksum_size;
    uint32_t rcvd_len = rcvd_raw.length();
    QByteArray tmp;
    uint8_t gui_list_pos;
    checksum_struct gui_checksum;

    // Recv Loop
    while ((0 < rcvd_len) && !exit_recv && !(base_flags & base_exit_flag))
    {
        // Check to see if keys in rcvd
        expected_len = num_s1_bytes;
        if (rcvd_len < expected_len) break; // Break out of Recv Loop

        // Parse keys
        major_key = (uchar) rcvd_raw.at(s1_major_key_loc);
        minor_key = (uchar) rcvd_raw.at(s1_minor_key_loc);

        // Decode keys
        bits = major_key & s1_num_s2_bytes_bit_mask;
        major_key &= s1_major_key_bit_mask;

        // Adjust byte length of 3 (want uint32_t not uint24_t)
        if (bits == 0x03) bits += 1;

        // Check if byte length in received
        expected_len += bits;
        if (rcvd_len < expected_len) break; // Break out of Recv Loop

        // Get gui & verify gui key
        gui_list_pos = (major_key >> s1_major_key_bit_shift)-1;
        if (!(known_guis.length() < gui_list_pos))
        {
            // Clear rcvd if key error
            rcvd_raw.clear();

            // Ack error
            send_ack(MAJOR_KEY_ERROR);

            // Break out of Recv Loop
            break;
        }

        // Select checksum
        gui_checksum = gui_checksums.at(gui_list_pos);

        // Key Switch
        switch (major_key)
        {
            // Check if ack or reset
            case MAJOR_KEY_ACK:
            case MAJOR_KEY_RESET:
            {
                // Set generic checksum executable if using
                if (gui_checksum.checksum_is_exe)
                    set_executable_checksum_exe(gui_checksum.checksum_exe);

                // Verify enough bytes
                checksum_size = gui_checksum.get_checksum_size();
                exit_recv = (rcvd_len < (expected_len+checksum_size));
                if (exit_recv) break;  // Break out of Key Switch

                // Check Checksum (checksum failure handled in Act Switch)
                exit_recv = !check_checksum((const uint8_t*) rcvd_raw.constData(),
                                            expected_len, &gui_checksum);

                // Act Switch
                switch (major_key)
                {
                    case MAJOR_KEY_ACK:
                    {
                        // If not error, emit ackReceived
                        if (!exit_recv)
                        {
                            // Build temporary array to send off data
                            tmp.clear();
                            tmp.append((char) major_key);
                            tmp.append((char) minor_key);

                            // Emit ack received
                            emit ackReceived(tmp);
                        }

                        // Break out of Act Switch
                        break;
                    }
                    case MAJOR_KEY_RESET:
                    {
                        // Check if error
                        if (exit_recv)
                        {
                            // Ack error if checksum error
                            send_ack(MAJOR_KEY_ERROR);
                        } else
                        {
                            // Ack success
                            send_ack(major_key);

                            // Set reset flags to breakout of send_chunk
                            base_flags |= base_send_chunk_flag;

                            // Emit reset to everything
                            emit reset();
                        }

                        // Break out of Act Switch
                        break;
                    }
                }

                // Clear array if error, else remove packet from rcvd
                if (exit_recv) rcvd_raw.clear();
                else rcvd_raw.remove(0, expected_len+checksum_size);

                // Break out of Key Switch
                break;
            }
            // Check if part of GUI Keys
            case MAJOR_KEY_IO:
            case MAJOR_KEY_WELCOME:
            case MAJOR_KEY_PROGRAMMER:
            case MAJOR_KEY_CUSTOM_CMD:
            case MAJOR_KEY_DATA_TRANSMIT:
            {
                // Parse num_s2_bytes
                num_s2_bytes = GUI_HELPER::byteArray_to_uint32(
                            rcvd_raw.mid(s1_num_s2_bytes_loc,bits));

                // Check if second stage in rcvd
                expected_len += num_s2_bytes;
                exit_recv = rcvd_len < (expected_len);
                if (exit_recv) break; // Break out of Key Switch

                // Set gui checksum executable if using
                if (gui_checksum.checksum_is_exe)
                    set_executable_checksum_exe(gui_checksum.checksum_exe);
                checksum_size = gui_checksum.get_checksum_size();

                // Check if checksum in rcvd
                exit_recv = (rcvd_len < (expected_len+checksum_size));
                if (exit_recv) break; // Break out of Key Switch

                // Check Checksum
                exit_recv = !check_checksum((const uint8_t*) rcvd_raw.constData(),
                                            expected_len, &gui_checksum);
                if (exit_recv)
                {
                    // Clear rcvd if checksum error
                    rcvd_raw.clear();

                    // Ack error
                    send_ack(MAJOR_KEY_ERROR);

                    // Break out of Key Switch
                    break;
                }

                // Build temporary array to send off data and ack
                tmp.clear();
                tmp.append((char) major_key);
                tmp.append((char) minor_key);
                tmp.append(rcvd_raw.mid(s1_num_s2_bytes_loc+bits, num_s2_bytes));

                // Ack success & set data status
                send_ack(major_key);
                data_status = true;

                // Emit readyRead - Send to all registered base guis
                foreach (GUI_BASE* gui, known_guis.at(gui_list_pos))
                    emit gui->readyRead(tmp);

                // Remove data from rcvd_raw
                rcvd_raw.remove(0, expected_len+checksum_size);

                // Break out of Key Switch
                break;
            }
            // If unknown
            default:
            {
                // Clear buffers and set exit
                rcvd_raw.clear();
                exit_recv = true;

                // Ack error
                send_ack(MAJOR_KEY_ERROR);

                // Break out of Key Switch
                break;
            }
        }

        // Update rcvd_len
        rcvd_len = rcvd_raw.length();
    }

    // Unlock recv lock
    rcvLock.unlock();

    // If exiting, check if ready
    if (base_flags & base_exit_flag) close_bridge();
}

void GUI_COMM_BRIDGE::send_file(uint8_t major_key, uint8_t minor_key, QString filePath)
{
    send_chunk(major_key, minor_key, GUI_HELPER::loadFile(filePath), true);
}

void GUI_COMM_BRIDGE::send_file_chunked(uint8_t major_key, uint8_t minor_key, QString filePath, char sep)
{
    foreach (QByteArray chunk, GUI_HELPER::loadFile(filePath).split(sep))
    {
        send_chunk(major_key, minor_key, chunk);
    }
}

void GUI_COMM_BRIDGE::send_chunk(uint8_t major_key, uint8_t minor_key, QByteArray chunk, bool force_envelope)
{
    // Verify this will terminate & not exiting
    if ((chunk_size == 0) || (base_flags & base_exit_flag)) return;

    // Check if reset & packet not the resetting packet
    // Assumes reset occurs before any other signals and that
    // any packets calling this will be after a reset
    if ((base_flags & base_reset_flag) && (major_key != MAJOR_KEY_RESET))
    {
        return;  // Still sending data return
    } else if ((base_flags & base_send_chunk_flag) && (major_key != MAJOR_KEY_RESET))
    {
        base_flags &= ~base_send_chunk_flag; // Clear base_send_chunk bit
    } else if (major_key == MAJOR_KEY_RESET)
    {
        // Enter reset (set active and send_chunk flags, clear sent flag)
        base_flags |= (base_reset_flag | base_send_chunk_flag);

        // Clear any pending messages
        msgList.clear();

        // Force end of timers
        emit reset();
    }

    // Setup base variables
    QByteArray data, curr;
    uint32_t pos = 0;
    uint32_t end_pos = chunk.length();
    bool emit_progress = (major_key == gui_key);

    // Get sender info for emiting signals
    GUI_BASE* sending_gui = sender();

    // Reset progress (if sending from gui)
    if (emit_progress) emit sending_gui->progress_update_send(0, "");

    // Send start of data chunk
    force_envelope |= (chunk_size < end_pos);
    if (force_envelope)
    {
        // Make start packet and send
        data.clear();
        data.append((char) major_key);
        data.append((char) minor_key);
        transmit(data);

        // Check if reset set during transmission eventloop
        if (base_flags) return;
    }

    // Send data chunk
    uint32_t data_len, bits;
    do
    {
        // Clear data
        data.clear();

        // Get next data chunk and add info
        curr = chunk.mid(pos, chunk_size);

        // Compute size of data chunk
        data_len = curr.length();
        if (data_len == 0) bits = 0x00;
        else if (data_len <= 0xFF) bits = 0x01;
        else if (data_len <= 0xFFFF) bits = 0x02;
        else bits = 0x03;

        // Load into data array
        data.append((char) (major_key | bits));
        data.append((char) minor_key);

        // Adjust byte length of 3 (want uint32_t not uint24_t)
        if (bits == 0x03) bits += 1;

        // Load data_len into data
        data.append(GUI_HELPER::uint32_to_byteArray(data_len).right(bits));

        // Append data
        data.append(curr);

        // Transmit data to device
        transmit(data);

        // Check if reset set during transmission eventloop
        if (base_flags) return;

        // Increment position counter
        // Do not use chunk_size to enable dyanmic setting
        pos += curr.length();

        // Update progress
        if (emit_progress)
            emit sending_gui->progress_update_send(qRound(((float) pos / end_pos) * 100.0f), "");
    } while (pos < end_pos);

    // Send end of data chunk
    if (force_envelope)
    {
        data.clear();
        data.append((char) major_key);
        data.append((char) minor_key);
        transmit(data);

        // Check if reset set during transmission eventloop
        if (base_flags) return;
    }

    // Signal done to user if from gui
    if (emit_progress) emit sending_gui->progress_update_send(100, "Done!");
}

void GUI_COMM_BRIDGE::send_ack(uint8_t majorKey)
{
    // Build ack packet
    QByteArray ack_packet;
    ack_packet.append((char) MAJOR_KEY_ACK);
    ack_packet.append((char) majorKey);

    // Get checksum
    uint8_t* checksum_array;
    uint32_t checksum_size = 0;
    getChecksum((const uint8_t*) ack_packet.constData(), ack_packet.length(),
                MAJOR_KEY_ACK, &checksum_array, &checksum_size);

    // Append checksum
    ack_packet.append((const char*) checksum_array, checksum_size);

    // Delete checksum array (done using)
    delete[] checksum_array;

    // Send ack immediately
    // Not expecting ack back (makes this possible)
    emit write_data(ack_packet);
}

void GUI_COMM_BRIDGE::waitForAck(int msecs)
{
    // Wait for achChecked or timeout
    ackTimer.start(msecs);
    ackLoop.exec();

    // Stop timer
    ackTimer.stop();
}

void GUI_COMM_BRIDGE::checkAck(QByteArray ack)
{
    // Check ack against inputs
    ack_status = ((ack.at(s1_major_key_loc) == (char) MAJOR_KEY_ACK)
            && (ack.at(s1_minor_key_loc) == (char) ack_key));

    // Emit checked signal
    emit ackChecked(ack_status);
}

bool GUI_COMM_BRIDGE::check_checksum(const uint8_t* data, uint32_t data_len, checksum_struct* check)
{
    // Create checksum variables
    uint32_t checksum_size = check->get_checksum_size();
    uint8_t fsm_checksum_cmp_buffer[checksum_size];

    // Compute checksum on data
    check->get_checksum(data, data_len, check->checksum_start, fsm_checksum_cmp_buffer);

    // Compare generated to received checksum
    return check->check_checksum(data+data_len, fsm_checksum_cmp_buffer);
}

void GUI_COMM_BRIDGE::waitForData(int msecs)
{
    // Wait for dataReceived or timeout
    dataTimer.start(msecs);
    dataLoop.exec();

    // Stop timer
    dataTimer.stop();
}

void GUI_COMM_BRIDGE::closing()
{
    // Set exit to true
    base_flags |= base_exit_flag;

    // Clear any pending messages
    msgList.clear();

    // Emit reset to free timers
    emit reset();

    // See if ready to exit
    close_bridge();
}

void GUI_COMM_BRIDGE::getChecksum(const uint8_t* data, uint32_t data_len, uint8_t checksum_key,
                                  uint8_t** checksum_array, uint32_t* checksum_size)
{
    // Adjust type for correct bit shifts
    uint8_t gui_list_pos = (checksum_key >> s1_major_key_bit_shift)-1;

    // Get checksum
    checksum_struct* check;
    if (gui_list_pos < gui_checksums.length())
    {
        // Set checksum info
        check = &gui_checksums.at(gui_list_pos);
    } else
    {
        // Set checksum info
        check = &generic_checksum;
    }

    // Set executable if using
    if (check->checksum_is_exe)
        set_executable_checksum_exe(check->checksum_exe);

    // Get size
    *checksum_size = check->get_checksum_size();

    // Malloc checksum array
    *checksum_array = (uint8_t*) malloc((*checksum_size)*sizeof(**checksum_array));
    memset(*checksum_array, 0, *checksum_size);

    check->get_checksum(data, data_len, check->checksum_start, *checksum_array);
}

void GUI_COMM_BRIDGE::copy_checksum_info(checksum_struct *cpy_to, checksum_struct *cpy_from)
{
    // Delete info from cpy to
    // Sets checksum_start and checksum_exe to nullptr
    delete_checksum_info(cpy_to);

    // Copy function/generic information
    cpy_to->get_checksum_size = cpy_from->get_checksum_size;
    cpy_to->get_checksum = cpy_from->get_checksum;
    cpy_to->check_checksum = cpy_from->check_checksum;
    cpy_to->checksum_is_exe = cpy_from->checksum_is_exe;

    // Copy executable information
    if (cpy_from->checksum_is_exe)
    {
        // Error no null ptr
        if (cpy_from->checksum_exe == nullptr) return;

        //
        char* new_check_exe = (char*) malloc(strlen(cpy_from->checksum_exe)*sizeof(char));
        strcpy(new_check_exe, cpy_from->checksum_exe);
        set_executable_checksum_exe(new_check_exe);
        cpy_to->checksum_exe = new_check_exe;
    }

    // Copy checksum start array
    if (cpy_from->checksum_start != nullptr)
    {
        uint32_t checksum_size = cpy_from->get_checksum_size();
        uint8_t* new_check_start = (uint8_t*) malloc(checksum_size*sizeof(uint8_t));
        memcpy(new_check_start, cpy_from->checksum_start, checksum_size);
        cpy_to->checksum_start = new_check_start;
    }
}

void GUI_COMM_BRIDGE::delete_checksum_info(checksum_struct *check)
{
    // Delete start
    if (check->checksum_start != nullptr)
    {
        delete[] check->checksum_start;
        check->checksum_start = nullptr;
    }

    // Delete exe
    if (check->checksum_exe != nullptr)
    {
        delete[] check->checksum_exe;
        check->checksum_exe = nullptr;
    }
}

void GUI_COMM_BRIDGE::set_checksum_exe(checksum_struct *check, QString checksum_exe)
{
    // Return if not exe or is exe path empty
    if (!check->checksum_is_exe || checksum_exe.isEmpty()) return;

    // Ensure exe path null terminated
    checksum_exe += '\0';

    // Create and copy into new array
    char* new_exe_path = (char*) malloc(checksum_exe.length()*sizeof(char));
    strcpy((char*) new_exe_path, checksum_exe.toUtf8().constData());

    // Delete and replace current value
    if (check->checksum_exe != nullptr) delete[] check->checksum_exe;
    check->checksum_exe = new_exe_path;
}

void GUI_COMM_BRIDGE::set_checksum_start(checksum_struct *check, QString checksum_start, uint8_t checksum_start_base)
{
    // Verify that data present
    if (checksum_start.isEmpty()) return;

    // Create new array
    if (check->checksum_is_exe) set_executable_checksum_exe(check->checksum_exe);
    uint32_t checksum_size = check->get_checksum_size();
    uint8_t* new_check_start = (uint8_t*) malloc(checksum_size*sizeof(uint8_t));
    memset(new_check_start, 0, checksum_size);

    // Build base start from supplied checksum start
    QByteArray start_convert = GUI_HELPER::string_to_byteArray(checksum_start, checksum_start_base);

    // Pad start until its checksum_size
    uint32_t start_len = start_convert.length();
    while (start_len < checksum_size)
    {
        start_convert.prepend((char) 0);
        start_len += 1;
    }

    // Copy start into new_check_start
    memcpy(new_check_start, start_convert.constData(), checksum_size);

    // Delete and replace default
    if (check->checksum_start != nullptr) delete[] check->checksum_start;
    check->checksum_start = new_check_start;
}

void GUI_COMM_BRIDGE::transmit(QByteArray data)
{
    // Check if trying to send empty data array or exiting
    if (data.isEmpty() || (base_flags & base_exit_flag)) return;

    // Append to msgList
    msgList.append(data);

    // Lock send to prevent spamming/blocking
    if (!sendLock.tryLock()) return;

    // Loop variables
    bool isReset;
    QByteArray currData;
    uint8_t* checksum_array;
    uint32_t checksum_size = 0;

    // Send all messages in list
    while (!msgList.isEmpty() && !(base_flags & base_exit_flag))
    {
        // Get next data to send
        currData = msgList.takeFirst();
        ack_status = false;
        data_status = false;
        ack_key = ((char) currData.at(s1_major_key_loc) & s1_major_key_bit_mask);
        isReset = (ack_key == MAJOR_KEY_RESET);

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
        } while (!ack_status
                 && (!(base_flags & base_reset_flag) || isReset)
                 && !(base_flags & base_exit_flag));

        // Wait for data read if CMD success and was data request
        // Resets will never request data back (only an ack)
        if (ack_status
                && !base_flags
                && isDataRequest(currData.at(s1_minor_key_loc)))
        {
            do
            {
                // Wait for data packet back
                waitForData(packet_timeout);
            } while (!data_status
                     && !base_flags);
        }

        // Check if reseting and was reset
        if ((base_flags & base_reset_flag) && isReset)
        {
            // Clear buffers (prevents key errors after reset)
            rcvd_raw.clear();
            rcvd_formatted.clear();
            msgList.clear();

            // Clear reset active flag
            base_flags &= ~base_reset_flag;
        }
    }

    // Unlock send lock
    sendLock.unlock();

    // If exiting, check if ready
    if (base_flags & base_exit_flag) close_bridge();
}

void GUI_COMM_BRIDGE::close_bridge()
{
    // Ensure exit is set
    if (!(base_flags & base_exit_flag)) return;

    // Try acquire sendLock if not already done
    if (!(base_flags & base_exit_send_flag) && sendLock.tryLock())
    {
        base_flags |= base_exit_send_flag;
        sendLock.unlock();
    }


    // Try acquire rcvLock if not already done
    if (!(base_flags & base_exit_recv_flag) && rcvLock.tryLock())
    {
        base_flags |= base_exit_recv_flag;
        rcvLock.unlock();
    }

    // Check status
    if (!(base_flags & (base_exit_send_flag | base_exit_recv_flag))) return;

    // Schedule for deletion
    this->deleteLater();
}

