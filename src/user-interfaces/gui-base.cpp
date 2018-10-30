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

// Setup send helpers
QMutex GUI_BASE::sendLock;
QList<QByteArray> GUI_BASE::msgList({});

// Setup recv helpers
QMutex GUI_BASE::rcvLock;
QByteArray GUI_BASE::rcvd_raw({});

// Setup Ack variables
bool GUI_BASE::ack_status = false;
uint8_t GUI_BASE::ack_key = MAJOR_KEY_ERROR;

// Setup Data variables
bool GUI_BASE::data_status = false;
uint8_t GUI_BASE::data_key = MAJOR_KEY_ERROR;

uint32_t GUI_BASE::chunk_size = GUI_BASE::default_chunk_size;
checksum_struct GUI_BASE::generic_checksum = DEFAULT_CHECKSUM_STRUCT;

// Setup suported checksums map
QMap<QString, checksum_struct>
GUI_BASE::supportedChecksums({
                                 {"CRC_8_LUT", {get_crc_8_LUT_size, get_crc_8_LUT, check_crc_8_LUT, 0, 0, 0}},
                                 {"CRC_8_POLY", {get_crc_8_POLY_size, get_crc_8_POLY, check_crc_8_POLY, 0, 0, 0}},
                                 {"CRC_16_LUT", {get_crc_16_LUT_size, get_crc_16_LUT, check_crc_16_LUT, 0, 0, 0}},
                                 {"CRC_16_POLY", {get_crc_16_POLY_size, get_crc_16_POLY, check_crc_16_POLY, 0, 0, 0}},
                                 {"CRC_32_LUT", {get_crc_32_LUT_size, get_crc_32_LUT, check_crc_32_LUT, 0, 0, 0}},
                                 {"CRC_32_POLY", {get_crc_32_POLY_size, get_crc_32_POLY, check_crc_32_POLY, 0, 0, 0}},
                                 {"CHECKSUM_EXE", {get_checksum_exe_size, get_checksum_exe, check_checksum_exe, 0, 0, 1}}
                             });

uint32_t num_s2_bytes;

GUI_BASE::GUI_BASE(QWidget *parent) :
    QWidget(parent)
{
    // Setup base flags
    base_flags = 0x00;

    // Init progress bars
    set_expected_recv_length(QByteArray());
    update_current_recv_length(QByteArray());

    // Init gui checksum variables
    gui_checksum = DEFAULT_CHECKSUM_STRUCT;

    // Connect read signals and slots
    connect(this, SIGNAL(readyRead(QByteArray)),
            this, SLOT(receive_gui(QByteArray)));

    // Connect data loop signals and slots
    connect(this, SIGNAL(readyRead(QByteArray)),
            &dataLoop, SLOT(quit()));
    connect(&dataTimer, SIGNAL(timeout()),
            &dataLoop, SLOT(quit()));
    connect(this, SIGNAL(resetting()),
            &dataLoop, SLOT(quit()));

    // Connect ack loop signals and slots
    connect(this, SIGNAL(ackReceived(QByteArray)),
            this, SLOT(checkAck(QByteArray)));
    connect(this, SIGNAL(ackChecked(bool)),
            &ackLoop, SLOT(quit()));
    connect(&ackTimer, SIGNAL(timeout()),
            &ackLoop, SLOT(quit()));
    connect(this, SIGNAL(resetting()),
            &ackLoop, SLOT(quit()));

    // Connect progress signals and slots
    // Wait till return to main event loop to process slots
    connect(this, SIGNAL(progress_update_recv(int, QString)),
            this, SLOT(set_progress_update_recv(int,QString)),
            Qt::QueuedConnection);
    connect(this, SIGNAL(progress_update_send(int, QString)),
            this, SLOT(set_progress_update_send(int,QString)),
            Qt::QueuedConnection);
}

GUI_BASE::~GUI_BASE()
{
    delete_checksum_info(&gui_checksum);
    delete_checksum_info(&generic_checksum);
}

void GUI_BASE::reset_remote()
{
    // Enter reset (set active and send_chunk flags, clear sent flag)
    base_flags |= (base_reset_flag | base_send_chunk_flag);

    // Clear any pending messages
    msgList.clear();

    // Emit resetting to free timers
    emit resetting();

    // Reset the gui (must not call reset remote)
    reset_gui();

    // Load reset CMD into msgList
    send({MAJOR_KEY_RESET, 0});

    // Exiting here returngs control to the main event loop
    // Required in order for the next packet to be sent
    // if currently in waitForAck or waitForData
}

void GUI_BASE::closing()
{
    // Set exit to true
    base_flags |= base_exit_flag;

    // Reset send
    reset_remote();

    // See if ready to exit
    close_base();
}

void GUI_BASE::set_gui_checksum(QStringList new_gui_checksum)
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
                       new_gui_checksum.at(checksum_start_pos).split(','),
                       new_gui_checksum.at(checksum_start_base_pos).toInt());
}

QStringList GUI_BASE::get_supported_checksums()
{
    return supportedChecksums.keys();
}

void GUI_BASE::set_generic_checksum(QStringList new_generic_checksum)
{
    // Get default struct
    checksum_struct default_check = supportedChecksums.value(
                new_generic_checksum.at(checksum_name_pos),
                DEFAULT_CHECKSUM_STRUCT
                );

    // Copy default information
    copy_checksum_info(&generic_checksum, &default_check);

    // Copy executable path if new checksum is exe & exe provided
    set_checksum_exe(&generic_checksum,
                     new_generic_checksum.at(checksum_exe_pos));

    // If checksum_start provided, overwrite default
    set_checksum_start(&generic_checksum,
                       new_generic_checksum.at(checksum_start_pos).split(','),
                       new_generic_checksum.at(checksum_start_base_pos).toInt());
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
        set_generic_checksum(setting.toStringList());
    }

    // Set chunk size
    setting = configMap->value("chunk_size");
    if (!setting.isNull())
    {
        set_chunk_size(setting.toUInt());
    }
}

size_t GUI_BASE::get_chunk_size()
{
    return chunk_size;
}

void GUI_BASE::set_chunk_size(size_t chunk)
{
    chunk_size = chunk;
}

void GUI_BASE::reset_gui()
{
    // Reset progress bars info
    start_data = true;
    current_recv_length = 0;
    expected_recv_length = 1;

    emit progress_update_recv(0, "");
    emit progress_update_send(0, "");
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

        // Key Switch
        switch (major_key)
        {
            case MAJOR_KEY_ACK:
            case MAJOR_KEY_RESET:
            {
                // Set generic checksum executable if using
                if (generic_checksum.checksum_is_exe)
                    set_executable_checksum_exe(generic_checksum.checksum_exe);

                // Verify enough bytes
                checksum_size = generic_checksum.get_checksum_size();
                exit_recv = (rcvd_len < (expected_len+checksum_size));
                if (exit_recv) break;  // Break out of Key Switch

                // Check Checksum (checksum failure handled in Act Switch)
                exit_recv = !check_checksum((const uint8_t*) rcvd_raw.constData(), expected_len, &generic_checksum);

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

                            // Emit reseting
                            emit resetting();

                            // Reset the gui (must not call reset remote)
                            reset_gui();
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
            default:
            {
                // Parse num_s2_bytes
                num_s2_bytes = GUI_HELPER::byteArray_to_uint32(rcvd_raw.mid(s1_num_s2_bytes_loc, bits));

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
                exit_recv = !check_checksum((const uint8_t*) rcvd_raw.constData(), expected_len, &gui_checksum);
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

                // Emit readyRead
                emit readyRead(tmp);

                // Remove data from rcvd_raw
                rcvd_raw.remove(0, expected_len+checksum_size);

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
    if (base_flags & base_exit_flag) close_base();
}

void GUI_BASE::receive_gui(QByteArray)
{
    // Default do nothing
}

void GUI_BASE::on_ResetGUI_Button_clicked()
{
    // Reset the Remote
    // (reset_remote() enforces a GUI reset)
    reset_remote();
}

void GUI_BASE::set_progress_update_recv(int, QString)
{
    // Default do nothing
}

void GUI_BASE::set_progress_update_send(int, QString)
{
    // Default do nothing
}

void GUI_BASE::send(QString data)
{
    send(data.toUtf8());
}

void GUI_BASE::send(QByteArray data)
{
    send_chunk(data.at(s1_major_key_loc), data.at(s1_minor_key_loc), data.mid(s1_end_loc));
}

void GUI_BASE::send(std::initializer_list<uint8_t> data)
{
    send(GUI_HELPER::initList_to_byteArray(data));
}

void GUI_BASE::send_file(uint8_t major_key, uint8_t minor_key, QString filePath)
{
    send_chunk(major_key, minor_key, GUI_HELPER::loadFile(filePath), true);
}

void GUI_BASE::send_file_chunked(uint8_t major_key, uint8_t minor_key, QString filePath, char sep)
{
    foreach (QByteArray chunk, GUI_HELPER::loadFile(filePath).split(sep))
    {
        send_chunk(major_key, minor_key, chunk);
    }
}

void GUI_BASE::send_chunk(uint8_t major_key, uint8_t minor_key, QByteArray chunk, bool force_envelope)
{
    // Verify this will terminate & not exiting
    if ((chunk_size == 0) || (base_flags & base_exit_flag)) return;

    // Check if reset & packet not the resetting packet
    // Assumes reset occurs before any other signals and that
    // any packets calling this will be after a reset
    if ((base_flags & base_reset_flag) && (major_key != MAJOR_KEY_RESET))
        return;  // Still sending data return
    else if ((base_flags & base_send_chunk_flag) && (major_key != MAJOR_KEY_RESET))
        base_flags &= ~base_send_chunk_flag; // Clear base_send_chunk bit

    // Setup base variables
    QByteArray data, curr;
    uint32_t pos = 0;
    uint32_t end_pos = chunk.length();
    bool emit_progress = (major_key == gui_key);

    // Reset progress (if sending from gui)
    if (emit_progress) emit progress_update_send(0, "");

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
            emit progress_update_send(qRound(((float) pos / end_pos) * 100.0f), "");
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
    if (emit_progress) emit progress_update_send(100, "Done!");
}

void GUI_BASE::send_chunk(uint8_t major_key, uint8_t minor_key, std::initializer_list<uint8_t> chunk, bool force_envelope)
{
    send_chunk(
                major_key,
                minor_key,
                GUI_HELPER::initList_to_byteArray(chunk),
                force_envelope
                );
}

void GUI_BASE::send_ack(uint8_t majorKey)
{
    // Build ack packet
    QByteArray ack_packet;
    ack_packet.append((char) MAJOR_KEY_ACK);
    ack_packet.append((char) majorKey);

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
    // Not expecting ack back (makes this possible)
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
    uint8_t fsm_checksum_cmp_buffer[checksum_size];

    // Compute checksum on data
    check->get_checksum(data, data_len, check->checksum_start, fsm_checksum_cmp_buffer);

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

void GUI_BASE::set_expected_recv_length(QByteArray recv_length)
{
    // Convert and set qbytearray
    expected_recv_length = GUI_HELPER::byteArray_to_uint32(recv_length);

    // Reset progress bar
    emit progress_update_recv(0, "");
}

void GUI_BASE::update_current_recv_length(QByteArray recvData)
{
    // Find length of data
    int data_len = recvData.length();

    // Start and stop sent by
    if (data_len == 0)
    {
        // See if starting new file
        if (start_data)
        {
            current_recv_length = 0;
            emit progress_update_recv(0, "");
        } else
        {
            expected_recv_length = 0;
            emit progress_update_recv(100, "Done!");
        }

        // Toggle start_data
        start_data = !start_data;

        // Exit after setting
        return;
    }

    // Update received length
    current_recv_length += data_len;

    // Update progress bar if total recv length known
    if (expected_recv_length != 0)
        emit progress_update_recv(qRound(((float) current_recv_length/expected_recv_length) * 100.0f), "");
}

void GUI_BASE::transmit(QByteArray data)
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

        // Else, send data and verify ack
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
    if (base_flags & base_exit_flag) close_base();
}

void GUI_BASE::getChecksum(const uint8_t* data, uint32_t data_len, uint8_t checksum_key,
                           uint8_t** checksum_array, uint32_t* checksum_size)
{
    // Get checksum
    checksum_struct* check;
    if (checksum_key == (char) gui_key)
    {
        // Set checksum info
        check = &gui_checksum;
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

void GUI_BASE::copy_checksum_info(checksum_struct *cpy_to, checksum_struct *cpy_from)
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

void GUI_BASE::delete_checksum_info(checksum_struct *check)
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

void GUI_BASE::set_checksum_exe(checksum_struct *check, QString checksum_exe)
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

void GUI_BASE::set_checksum_start(checksum_struct *check, QStringList checksum_start, uint8_t checksum_start_base)
{
    // Verify that data present
    if (checksum_start.isEmpty()) return;

    // Create new array
    if (check->checksum_is_exe) set_executable_checksum_exe(check->checksum_exe);
    uint32_t checksum_size = check->get_checksum_size();
    uint8_t* new_check_start = (uint8_t*) malloc(checksum_size*sizeof(uint8_t));
    memset(new_check_start, 0, checksum_size);

    // Build from supplied checksum start
    uint32_t j = checksum_size - 1;
    for (uint32_t i = 0; i < checksum_size; i++)
    {
        new_check_start[i] = (char) checksum_start.at(j).toInt(nullptr, checksum_start_base);
        j -= 1;
    }

    // Delete and replace default
    if (check->checksum_start != nullptr) delete[] check->checksum_start;
    check->checksum_start = new_check_start;
}

void GUI_BASE::close_base()
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
