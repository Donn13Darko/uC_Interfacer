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

#include "gui-comm-bridge.hpp"

#include <QFile>

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

GUI_COMM_BRIDGE::GUI_COMM_BRIDGE(uint8_t num_guis, QObject *parent) :
    QObject(parent)
{
    // Setup base flags
    bridge_flags = 0x00;

    // Setup Ack variables
    ack_status = false;
    ack_key = MAJOR_KEY_ERROR;

    // Set generic defaults
    chunk_size = GUI_COMM_BRIDGE::default_chunk_size;

    // Init storage lists
    for (uint8_t i = 0; i < num_guis; i++)
    {
        // Set default checksum
        tab_checksums.append(DEFAULT_CHECKSUM_STRUCT);
    }

    // Connect dev loop signals and slots
    // All internal object connections so direct is okay
    connect(this, SIGNAL(devReady()),
            &devReadyLoop, SLOT(quit()),
            Qt::DirectConnection);
    connect(this, SIGNAL(reset()),
            &devReadyLoop, SLOT(quit()),
            Qt::DirectConnection);

    // Connect ack loop signals and slots
    // All internal object connections so direct is okay
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

    // Connect re-emit signals to slots
    // Use queued connections to return to main event loop first
    // (exits existing function stack to prevent overflow)
    connect(this, SIGNAL(transmit_file(quint8, quint8, QString, quint8, QString, GUI_BASE*)),
            this, SLOT(send_file(quint8, quint8, QString, quint8, QString, GUI_BASE*)),
            Qt::QueuedConnection);
    connect(this, SIGNAL(transmit_file_pack(quint8, quint8, QString, quint8, QString, GUI_BASE*)),
            this, SLOT(send_file_pack(quint8, quint8, QString, quint8, QString, GUI_BASE*)),
            Qt::QueuedConnection);
    connect(this, SIGNAL(transmit_chunk(quint8, quint8, QByteArray, quint8, QString, GUI_BASE*)),
            this, SLOT(send_chunk(quint8, quint8, QByteArray, quint8, QString, GUI_BASE*)),
            Qt::QueuedConnection);
    connect(this, SIGNAL(transmit_chunk_pack(quint8, quint8, QByteArray, quint8, QString, GUI_BASE*)),
            this, SLOT(send_chunk_pack(quint8, quint8, QByteArray, quint8, QString, GUI_BASE*)),
            Qt::QueuedConnection);
}

GUI_COMM_BRIDGE::~GUI_COMM_BRIDGE()
{
    // Delete each GUIs checksum info
    foreach (checksum_struct check, tab_checksums)
    {
        delete_checksum_info(&check);
    }
}

uint32_t GUI_COMM_BRIDGE::get_chunk_size()
{
    return chunk_size;
}

void GUI_COMM_BRIDGE::set_chunk_size(uint32_t chunk)
{
    chunk_size = chunk;
}

QStringList GUI_COMM_BRIDGE::get_supported_checksums()
{
    return supportedChecksums.keys();
}

void GUI_COMM_BRIDGE::set_tab_checksum(uint8_t gui_key, QStringList new_tab_checksum)
{
    // Get current checksum
    if (!gui_key || (tab_checksums.length() < gui_key)) return;
    checksum_struct current_check = tab_checksums.at(gui_key-1);

    // Get default struct
    checksum_struct default_check = supportedChecksums.value(
                new_tab_checksum.at(checksum_name_pos),
                DEFAULT_CHECKSUM_STRUCT
                );

    // Copy default information
    copy_checksum_info(&current_check, &default_check);

    // Copy executable path if new checksum is exe & exe provided
    set_checksum_exe(&current_check,
                     new_tab_checksum.at(checksum_exe_pos));

    // If checksum_start provided, overwrite default
    set_checksum_start(&current_check,
                       new_tab_checksum.at(checksum_start_pos),
                       new_tab_checksum.at(checksum_start_base_pos).toInt());

    // Save replaced checksum
    tab_checksums.replace(gui_key-1, current_check);
}

void GUI_COMM_BRIDGE::parseGenericConfigMap(QMap<QString, QVariant> *configMap)
{
    // Holder for each setting
    QVariant setting;

    // Check general checksum type setting
    // (only sets if not set at runtime in more options)
    setting = configMap->value("checksum_set");
    if (!setting.isNull())
    {
        // Set generic checksum type
        set_tab_checksum(MAJOR_KEY_GENERAL_SETTINGS, setting.toStringList());
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
    // Get value list to insert in
    if (!known_guis.contains(new_gui))
        known_guis.append(new_gui);
}

void GUI_COMM_BRIDGE::remove_gui(GUI_BASE *old_gui)
{
    // Remove all instances of gui from list
    known_guis.removeAll(old_gui);
}

void GUI_COMM_BRIDGE::reset_remote()
{
    // Post the reset key, will emit reset()
    send_chunk(MAJOR_KEY_RESET, 0);
}

void GUI_COMM_BRIDGE::receive(QByteArray recvData)
{
    // Check if recieving empty data array or exiting
    if (recvData.isEmpty() || (bridge_flags & bridge_close_flag)) return;

    // Add data to recv
    rcvd_raw.append(recvData);

    // Lock recv to prevent spamming/blocking
    if (!rcvLock.tryLock()) return;

    // Setup loop variables
    bool exit_recv = false;
    uint8_t major_key, minor_key, num_s2_bits;
    uint32_t expected_len, checksum_size;
    uint32_t rcvd_len = rcvd_raw.length();
    QByteArray tmp;
    const checksum_struct *check;

    // Recv Loop
    while ((0 < rcvd_len) && !exit_recv && !(bridge_flags & bridge_close_flag))
    {
        // Check to see if keys in rcvd
        expected_len = num_s1_bytes;
        if (rcvd_len < expected_len) break; // Break out of Recv Loop

        // Parse keys
        major_key = (uchar) rcvd_raw.at(s1_major_key_loc);
        minor_key = (uchar) rcvd_raw.at(s1_minor_key_loc);

        // Decode keys
        num_s2_bits = (major_key >> s1_num_s2_bits_byte_shift) & s1_num_s2_bits_byte_mask;
        major_key &= s1_major_key_byte_mask;

        // Adjust byte length of 3 (want uint32_t not uint24_t)
        if (num_s2_bits == num_s2_bits_3) num_s2_bits = num_s2_bits_4;

        // Check if byte length in received
        expected_len += num_s2_bits;
        if (rcvd_len < expected_len) break; // Break out of Recv Loop

        // Select checksum (default to 0)
        if (!major_key || (tab_checksums.length() < major_key)) check = &tab_checksums.at(MAJOR_KEY_GENERAL_SETTINGS-1);
        else check = &tab_checksums.at(major_key-1);

        // Key Switch
        switch (major_key)
        {
            // Check if ack or reset
            case MAJOR_KEY_ACK:
            case MAJOR_KEY_RESET:
            case MAJOR_KEY_DEV_READY:
            {
                // Set generic checksum executable if using
                if (check->checksum_is_exe)
                    set_executable_checksum_exe(check->checksum_exe);

                // Verify enough bytes
                checksum_size = check->get_checksum_size();
                exit_recv = (rcvd_len < (expected_len+checksum_size));
                if (exit_recv) break;  // Break out of Key Switch

                // Check Checksum (checksum failure handled in Act Switch)
                exit_recv = !check_checksum((const uint8_t*) rcvd_raw.constData(),
                                            expected_len, check);

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
                            bridge_flags |= bridge_reset_send_chunk_flag;

                            // Emit reset to everything
                            emit reset();
                        }

                        // Break out of Act Switch
                        break;
                    }
                    case MAJOR_KEY_DEV_READY:
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

                            // Emit device ready for more
                            emit devReady();
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
                            rcvd_raw.mid(s1_end_loc, num_s2_bits));

                // Check if second stage in rcvd
                expected_len += num_s2_bytes;
                exit_recv = rcvd_len < (expected_len);
                if (exit_recv) break; // Break out of Key Switch

                // Set gui checksum executable if using
                if (check->checksum_is_exe)
                    set_executable_checksum_exe(check->checksum_exe);
                checksum_size = check->get_checksum_size();

                // Check if checksum in rcvd
                exit_recv = (rcvd_len < (expected_len+checksum_size));
                if (exit_recv) break; // Break out of Key Switch

                // Check Checksum
                exit_recv = !check_checksum((const uint8_t*) rcvd_raw.constData(),
                                            expected_len, check);
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
                tmp.append(rcvd_raw.mid(s1_end_loc+num_s2_bits, num_s2_bytes));

                // Ack success
                send_ack(major_key);

                // Emit readyRead - Send to all registered base guis
                // (Indexing without check encforced by switch statement case)
                foreach (GUI_BASE *gui, known_guis)
                {
                    if ((gui->get_GUI_key() == major_key)
                            || gui->acceptAllCMDs())
                    {
                        emit gui->readyRead(tmp);
                    }
                }

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
    if (bridge_flags & bridge_close_flag) close_bridge();
}

void GUI_COMM_BRIDGE::send_file(quint8 major_key, quint8 minor_key,
                                QString filePath, quint8 base,
                                QString encoding, GUI_BASE *sending_gui)
{
    // Try to acquire sendLock (or add to waiting list)
    if (!get_send_lock(major_key, minor_key,
                       QVariant(filePath), sending_gui,
                       SEND_STRUCT_SEND_FILE, base, encoding))
    {
        return;
    }

    // Get sending GUI if not present
    if (!sending_gui) sending_gui = (GUI_BASE*) sender();

    // Parse the file with the given information
    parse_file(major_key, minor_key,
               filePath, base,
               encoding, sending_gui);

    // Release lock
    sendLock.unlock();

    // Check if other packets waiting
    handle_next_send();
}

void GUI_COMM_BRIDGE::send_file_pack(quint8 major_key, quint8 minor_key,
                                     QString filePath, quint8 base,
                                     QString encoding, GUI_BASE *sending_gui)
{
    // Try to acquire sendLock (or add to waiting list)
    if (!get_send_lock(major_key, minor_key,
                       QVariant(filePath), sending_gui,
                       SEND_STRUCT_SEND_FILE_PACK, base, encoding))
    {
        return;
    }

    // Get sending GUI if not present
    if (!sending_gui) sending_gui = (GUI_BASE*) sender();

    // Send start of file
    parse_data(major_key, minor_key);

    // If no flags, parse the file with the given information
    if (!bridge_flags)
    {
        parse_file(major_key, minor_key,
                   filePath, base,
                   encoding, sending_gui);
    }


    // If no flags, send end of file
    if (!bridge_flags) parse_data(major_key, minor_key);

    // Release lock
    sendLock.unlock();

    // Check if other packets waiting
    handle_next_send();
}

void GUI_COMM_BRIDGE::send_chunk(quint8 major_key, quint8 minor_key,
                                 QByteArray chunk, quint8 base,
                                 QString encoding, GUI_BASE *sending_gui)
{
    // Try to acquire sendLock (or add to waiting list)
    if (!get_send_lock(major_key, minor_key,
                       QVariant(chunk), sending_gui,
                       SEND_STRUCT_SEND_CHUNK, base, encoding))
    {
        return;
    }

    // Get sending GUI if not present
    if (!sending_gui) sending_gui = (GUI_BASE*) sender();

    // Send chunk across
    parse_data(major_key, minor_key, chunk,
               base, QRegularExpression(encoding),
               sending_gui);

    // Release lock
    sendLock.unlock();

    // Check if other signals waiting
    handle_next_send();
}

void GUI_COMM_BRIDGE::send_chunk_pack(quint8 major_key, quint8 minor_key,
                                      QByteArray chunk, quint8 base,
                                      QString encoding, GUI_BASE *sending_gui)
{
    // Try to acquire sendLock (or add to waiting list)
    if (!get_send_lock(major_key, minor_key,
                       QVariant(chunk), sending_gui,
                       SEND_STRUCT_SEND_CHUNK_PACK, base, encoding))
    {
        return;
    }

    // Get sending GUI if not present
    if (!sending_gui) sending_gui = (GUI_BASE*) sender();

    // Send start of chunk
    parse_data(major_key, minor_key);

    // Reset progress
    emit sending_gui->progress_update_send(0, "");

    // Send chunk across
    parse_data(major_key, minor_key, chunk,
               base, QRegularExpression(encoding),
               sending_gui, true, 0, chunk.length());

    // If no flags, send end of file and progress updates
    if (!bridge_flags)
    {
        // Send end of chunk
        parse_data(major_key, minor_key);

        // Signal done to user
        emit sending_gui->progress_update_send(100, "Done!");
    }

    // Release lock
    sendLock.unlock();

    // Check if other signals waiting
    handle_next_send();
}

bool GUI_COMM_BRIDGE::open_bridge()
{
    // Clear everything but the exit flag
    bridge_flags &= bridge_exit_flag;

    // Clear any accidental sends/recvs
    transmitList.clear();
    rcvd_raw.clear();

    // True if all flags cleared, false if bridge_exit_flag set
    return !bridge_flags;
}

bool GUI_COMM_BRIDGE::close_bridge()
{
    // Set close bridge (if not already set)
    bridge_flags |= bridge_close_flag;

    // Clear send list
    transmitList.clear();

    // Emit reset to try and free timers
    emit reset();

    // Try acquire sendLock if not already done
    if (!(bridge_flags & bridge_close_send_flag) && sendLock.tryLock())
    {
        bridge_flags |= bridge_close_send_flag;
        sendLock.unlock();
    }

    // Try acquire rcvLock if not already done
    if (!(bridge_flags & bridge_close_recv_flag) && rcvLock.tryLock())
    {
        bridge_flags |= bridge_close_recv_flag;
        rcvLock.unlock();
    }

    // Check close successful
    if (!(bridge_flags & (bridge_close_send_flag | bridge_close_recv_flag))) return false;

    // If exit flag set, tell object to destroy itself
    if (bridge_flags & bridge_exit_flag) destroy_bridge();

    // Return object closed
    return true;
}

void GUI_COMM_BRIDGE::destroy_bridge()
{
    // Set exit flag (if not already set)
    bridge_flags |= bridge_exit_flag;

    // If not closed, close
    // Else, schedule object for deletion
    if (!(bridge_flags & (bridge_close_send_flag | bridge_close_recv_flag))) close_bridge();
    else this->deleteLater();
}

void GUI_COMM_BRIDGE::send_ack(uint8_t majorKey)
{
    // Build ack packet
    QByteArray ack_packet;
    ack_packet.append((char) MAJOR_KEY_ACK);
    ack_packet.append((char) majorKey);

    // Get checksum
    uint8_t *checksum_array;
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

void GUI_COMM_BRIDGE::getChecksum(const uint8_t *data, uint32_t data_len, uint8_t checksum_key,
                                  uint8_t **checksum_array, uint32_t *checksum_size)
{
    // Get checksum (if key is 0 or not recognized, use default key)
    const checksum_struct *check;
    if (!checksum_key || (tab_checksums.length() < checksum_key)) check = &tab_checksums.at(MAJOR_KEY_GENERAL_SETTINGS-1);
    else check = &tab_checksums.at(checksum_key-1);

    // Set executable if using
    if (check->checksum_is_exe)
        set_executable_checksum_exe(check->checksum_exe);

    // Get size
    *checksum_size = check->get_checksum_size();

    // Malloc checksum array
    *checksum_array = (uint8_t*) malloc(sizeof(uint8_t) * (*checksum_size));
    memset(*checksum_array, 0, *checksum_size);

    check->get_checksum(data, data_len, check->checksum_start, *checksum_array);
}

bool GUI_COMM_BRIDGE::check_checksum(const uint8_t *data, uint32_t data_len, const checksum_struct *check)
{
    // Create checksum variables
    uint32_t checksum_size = check->get_checksum_size();
    uint8_t fsm_checksum_cmp_buffer[checksum_size];

    // Compute checksum on data
    check->get_checksum(data, data_len, check->checksum_start, fsm_checksum_cmp_buffer);

    // Compare generated to received checksum
    return check->check_checksum(data+data_len, fsm_checksum_cmp_buffer);
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
        char *new_check_exe = (char*) malloc(sizeof(char) * strlen(cpy_from->checksum_exe));
        strcpy(new_check_exe, cpy_from->checksum_exe);
        set_executable_checksum_exe(new_check_exe);
        cpy_to->checksum_exe = new_check_exe;
    }

    // Copy checksum start array
    if (cpy_from->checksum_start != nullptr)
    {
        uint32_t checksum_size = cpy_from->get_checksum_size();
        uint8_t *new_check_start = (uint8_t*) malloc(sizeof(uint8_t) * checksum_size);
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
    char *new_exe_path = (char*) malloc(sizeof(char) * checksum_exe.length());
    strcpy((char*) new_exe_path, checksum_exe.toLatin1().constData());

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
    uint8_t *new_check_start = (uint8_t*) malloc(sizeof(uint8_t) * checksum_size);
    memset(new_check_start, 0, checksum_size);

    // Build base start from supplied checksum start
    QByteArray start_convert = GUI_HELPER::decode_byteArray(checksum_start.toLatin1(),
                                                            checksum_start_base, ' ');

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

void GUI_COMM_BRIDGE::check_packet(uint8_t major_key)
{
    // Check if packet is reset and not currently reseting
    if ((major_key == MAJOR_KEY_RESET)
            && (!(bridge_flags & bridge_reset_flag)))
    {
        // Enter reset (set active and send_chunk flags, clear sent flag)
        bridge_flags |= (bridge_reset_flag | bridge_reset_send_chunk_flag);

        // Clear any pending messages
        transmitList.clear();

        // Force end of timers
        emit reset();
    }
}

bool GUI_COMM_BRIDGE::get_send_lock(uint8_t major_key, uint8_t minor_key,
                                    QVariant data, GUI_BASE *sending_gui,
                                    uint8_t target, uint8_t base,
                                    QString encoding)
{
    // Verify not exiting or closed
    if (bridge_flags & (bridge_exit_flag | bridge_close_flag))
        return false;

    // Check packet
    check_packet(major_key);

    // Check if any packets waiting
    bool status = transmitList.isEmpty();

    // If packets waiting, check to see if matching
    if (!status)
    {
        // Get next packet in line
        send_struct curr = transmitList.first();

        // Check if this call is equal to next packet
        status = ((curr.sender == sending_gui)
                  && (curr.target == target)
                  && (curr.major_key == major_key)
                  && (curr.minor_key == minor_key)
                  && (curr.base == base)
                  && (curr.data == data)
                  && (curr.encoding == encoding));

        // If equal, try to aquire lock and begin sending
        if (status)
        {
            // Try to acquire lock, exit if fails
            // No need to add if fails, it's already the first packet
            if (!sendLock.tryLock()) return false;

            // Otherwise, lock acquired so remove packet
            // from waiting list it is now in sending
            transmitList.takeFirst();

            // Acquired lock and removed self from list
            return true;
        } else if (sending_gui)
        {
            // Handles case that this packet has already been added
            // sendList but is not the first element
            return false;
        }

        // If not equal, add to back of transmitList
        // Status set to False
    }

    // Handles:
    // Empty transmitList (!status = false, call tryLock to try sending)
    // Failed comparison (!status = true, no call to tryLock)
    if (!status || !sendLock.tryLock())
    {
        // Build new struct
        send_struct curr;
        curr.target = target;
        curr.major_key = major_key;
        curr.minor_key = minor_key;
        curr.base = base;
        curr.data = data;
        curr.encoding = encoding;

        // If sender null, get sender
        if (!sending_gui) curr.sender = (GUI_BASE*) sender();
        else curr.sender = sending_gui;

        // Add struct to transmitList
        transmitList.append(curr);

        // Exit to wait for calling
        return false;
    }

    // Success! Acquired lock
    return true;
}

void GUI_COMM_BRIDGE::handle_next_send()
{
    // Process list until we get a valid hit or empty
    send_struct next;
    while (!transmitList.isEmpty())
    {
        // Retrieve first one
        next = transmitList.first();

        // Have object re-emit slot for us
        switch (next.target)
        {
            case SEND_STRUCT_SEND_FILE:
                emit transmit_file(next.major_key, next.minor_key,
                                   next.data.toString(), next.base,
                                   next.encoding, next.sender);
                return;
            case SEND_STRUCT_SEND_FILE_PACK:
                emit transmit_file_pack(next.major_key, next.minor_key,
                                        next.data.toString(), next.base,
                                        next.encoding, next.sender);
                return;
            case SEND_STRUCT_SEND_CHUNK:
                emit transmit_chunk(next.major_key, next.minor_key,
                                    next.data.toByteArray(), next.base,
                                    next.encoding, next.sender);
                return;
            case SEND_STRUCT_SEND_CHUNK_PACK:
                emit transmit_chunk_pack(next.major_key, next.minor_key,
                                         next.data.toByteArray(), next.base,
                                         next.encoding, next.sender);
                return;
        }
    }
}

void GUI_COMM_BRIDGE::parse_file(quint8 major_key, quint8 minor_key,
                                 QString filePath, quint8 base,
                                 QString encoding, GUI_BASE *sender)
{
    // Verify encoding not expecting end of data
    // (May load in parts of file at a time)
    if (encoding.endsWith('$') && !encoding.endsWith("\\$"))
    {
        encoding.chop(1);
    }

    // Reset progress
    emit sender->progress_update_send(0, "");

    // Open file for reading
    QFile data_file(filePath);
    if (data_file.open(QIODevice::ReadOnly))
    {
        // Setup tracking variables
        uint32_t file_pos = 0;
        uint32_t file_chunk_size = 1048576; // 1MB Default
        uint32_t file_size = data_file.size();

        // Read in file in chunk size data bits
        QByteArray data_chunk;
        while (!data_file.atEnd() && !bridge_flags)
        {
            // See if chunk size updated: max(1MB, chunk_size)
            if (file_chunk_size < chunk_size) file_chunk_size = chunk_size;

            // Read in next large file chunk
            data_chunk.append((QByteArray) data_file.read(file_chunk_size));
            if (data_chunk.isEmpty()) break;

            // Send file chunks w/ updates
            data_chunk = parse_data(major_key, minor_key, data_chunk,
                                    base, QRegularExpression(encoding),
                                    sender, true, file_pos, file_size);

            // Update pos after send
            file_pos += file_chunk_size - data_chunk.length();
        }

        // Close file
        data_file.close();

        // If no flags, send progress update
        // (add error signals here too?)
        if (!bridge_flags) emit sender->progress_update_send(100, "Done!");
    } else
    {
        // Set progress error
        emit sender->progress_update_send(0, "Error: Unable to open file!");
    }
}

QByteArray GUI_COMM_BRIDGE::parse_data(quint8 major_key, quint8 minor_key, QByteArray data,
                                       quint8 base, QRegularExpression regex, GUI_BASE *sender,
                                       bool send_updates, quint32 c_pos, quint32 t_pos)
{
    // Verify this will terminate
    if (chunk_size == 0) return QByteArray();

    // Check if reset & packet not the resetting packet
    // Assumes reset occurs before any other signals and that
    // any packets calling this will be after a reset
    if (((bridge_flags & bridge_reset_flag) && (major_key != MAJOR_KEY_RESET))
            || (bridge_flags & (bridge_close_flag | bridge_exit_flag)))
    {
        // Still waiting for reset send so return
        return QByteArray();
    } else if ((bridge_flags & bridge_reset_send_chunk_flag) && (major_key != MAJOR_KEY_RESET))
    {
        // Reset has been sent so clear base_send_chunk bit
        bridge_flags &= ~bridge_reset_send_chunk_flag;
    }

    // Setup progress updates
    QString end_pos_str;
    if (send_updates && t_pos)
    {
        end_pos_str = "/" + QString::number(t_pos / 1000.0f) + "KB";
    }

    // Ensure that regex always begins at start of data
    if (!regex.pattern().startsWith('^'))
        regex.setPattern(regex.pattern().prepend('^'));

    // Set dot matches everything
    regex.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);

    // Setup loop variables
    QString data_str = QString::fromLatin1((const char*) data.data(), data.length());
    QStringList curr_parse_match;
    QByteArray curr_parse, curr_chunk;
    uint32_t curr_pos, end_pos, parse_len;

    // Try and read entire chunk
    // Will return what can't/wasn't sent
    do
    {
        // Try parsing next data chunk
        curr_parse_match = regex.match(data_str).capturedTexts();

        // Return remaining data if parsing failed
        if (curr_parse_match.isEmpty()) return data_str.toLatin1();

        // Take first element (first is entire captured string), length
        // will be used to remove characters at the end.
        parse_len = curr_parse_match.takeFirst().length();

        // Decode pieces using provided base
        curr_parse.clear();
        foreach (QString parse_str, curr_parse_match)
        {
            curr_parse += GUI_HELPER::decode_byteArray(parse_str.toLatin1(), base);
        }

        // Reset position variables
        curr_pos = 0;
        end_pos = curr_parse.length();

        // Send all bytes in current parse chunk
        do
        {
            // Get next data chunk and add info
            curr_chunk = curr_parse.mid(curr_pos, chunk_size);

            // Transmit data to device
            transmit_data(prepare_data(major_key, minor_key, curr_chunk));

            // Check if reset set during transmission eventloop
            if (bridge_flags) return data_str.toLatin1();

            // Increment position counter
            // Do not use chunk_size to enable dyanmic setting
            curr_pos += curr_chunk.length();


            // If sender available, see if other actions necessary
            if (sender)
            {
                // Emit an update if enabled and t_pos != 0
                if (send_updates && t_pos)
                {
                    emit sender->progress_update_send(qRound(((float) (c_pos + curr_pos) / t_pos) * 100.0f),
                                                      QString::number((float) (c_pos + curr_pos) / 1000.0f) + end_pos_str);
                }

                // Wait for devReady if command requires it
                if (sender->waitForDevice(minor_key))
                {
                    // Enter loop to return control to the
                    // main application event loop for cmd waiting
                    devReadyLoop.exec();

                    // Check if reset set during devReadyLoop
                    if (bridge_flags) return data_str.toLatin1();
                }
            }
        } while ((curr_pos < end_pos) && chunk_size);

        // Increment c_pos with new position
        c_pos += curr_pos;

        // Remove parsed data (including non sent characters)
        data_str.remove(0, parse_len);
    } while (!data_str.isEmpty() && !bridge_flags);

    return data_str.toLatin1();
}

/* Prepares a data packet for sending.
 * Encodes num_s2_bits, num_s2_bytes, and attaches checksum
 * Returned bytearray is ready for sending without modifications.
 */
QByteArray GUI_COMM_BRIDGE::prepare_data(quint8 major_key, quint8 minor_key, QByteArray data)
{
    // Setup variables
    QByteArray ret_data;
    uint8_t *checksum_array;
    uint32_t data_len, num_s2_bits, checksum_size;

    // Compute size of data chunk
    data_len = data.length();
    if (data_len == 0) num_s2_bits = num_s2_bits_0;
    else if (data_len <= 0xFF) num_s2_bits = num_s2_bits_1;
    else if (data_len <= 0xFFFF) num_s2_bits = num_s2_bits_2;
    else num_s2_bits = num_s2_bits_3;

    // Load into data array
    ret_data.append((char) (major_key | (num_s2_bits << s1_num_s2_bits_byte_shift)));
    ret_data.append((char) minor_key);

    // Adjust byte length of 3 (want uint32_t not uint24_t)
    if (num_s2_bits == num_s2_bits_3) num_s2_bits = num_s2_bits_4;

    // Load data_len into data
    ret_data.append(GUI_HELPER::uint32_to_byteArray(data_len).right(num_s2_bits));

    // Append data
    ret_data.append(data);

    // Get checksum
    getChecksum((const uint8_t*) ret_data.constData(), ret_data.length(),
                ack_key, &checksum_array, &checksum_size);

    // Append checksum
    ret_data.append((const char*) checksum_array, checksum_size);

    // Delete checksum array (done using)
    delete[] checksum_array;

    // Return packet ready for transmit
    return ret_data;
}

void GUI_COMM_BRIDGE::transmit_data(QByteArray data)
{
    // Check if trying to send empty data array or exiting
    if (data.isEmpty() || (bridge_flags & bridge_close_flag)) return;

    // Get next data to send
    ack_status = false;
    ack_key = ((char) data.at(s1_major_key_loc) & s1_major_key_byte_mask);
    bool isReset = (ack_key == MAJOR_KEY_RESET);

    // Send data and verify ack
    do
    {
        // Emit write command to connected device
        emit write_data(data);

        // Wait for CMD ack back
        waitForAck(packet_timeout);
    } while (!ack_status
             && (!(bridge_flags & bridge_reset_flag) || isReset)
             && !(bridge_flags & bridge_close_flag));

    // Check if reseting and if CMD was reset
    if ((bridge_flags & bridge_reset_flag) && isReset)
    {
        // Clear buffers (prevents key errors after reset)
        rcvd_raw.clear();
        transmitList.clear();

        // Clear reset active flag
        bridge_flags &= ~bridge_reset_flag;
    }

    // If exiting, check if ready
    if (bridge_flags & bridge_close_flag) close_bridge();
}
