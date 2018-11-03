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

#ifndef GUI_COMM_BRIDGE_H
#define GUI_COMM_BRIDGE_H

// Base object include
#include <QObject>

// Required object includes
#include <QMap>
#include <QMutex>
#include <QTimer>
#include <QVariant>
#include <QEventLoop>

// Local object includes
#include "../user-interfaces/gui-base-major-keys.h"
#include "../user-interfaces/gui-base.h"
#include "../checksums/checksums.h"
#include "gui-helper.h"

class GUI_COMM_BRIDGE : public QObject
{
    Q_OBJECT

public:
    GUI_COMM_BRIDGE(uint8_t num_guis, QObject *parent = 0);
    ~GUI_COMM_BRIDGE();

    // Chunk getter & setters
    size_t get_chunk_size();
    void set_chunk_size(size_t chunk);

    // Supported checksums
    static QStringList get_supported_checksums();

    // Checksum setters
    void set_gui_checksum(uint8_t gui_key, QStringList new_gui_checksum);

    // Parse input array
    void parseGenericConfigMap(QMap<QString, QVariant>* configMap);

    // Add gui tab to known list
    void add_gui(GUI_BASE *new_gui);

    // Default chunk size
    static const uint32_t default_chunk_size = 32;

signals:
    // Write data
    void write_data(QByteArray data);

    // Ack info
    void ackReceived(QByteArray ack);
    void ackChecked(bool ackStatus);

    // Reset connected GUIs
    void reset();

public slots:
    // Receive data
    void receive(QByteArray recvData);

    // File sending
    void send_file(uint8_t major_key, uint8_t minor_key, QString filePath);
    void send_file_chunked(uint8_t major_key, uint8_t minor_key, QString filePath, char sep);

    // Chunk sending
    void send_chunk(uint8_t major_key, uint8_t minor_key, QByteArray chunk = QByteArray(), bool force_envelope = false);

    // Ack
    void send_ack(uint8_t majorKey);
    void waitForAck(int msecs = 5000);
    void checkAck(QByteArray ack);
    bool check_checksum(const uint8_t* data, uint32_t data_len, checksum_struct* check);

    // Wait for data
    void waitForData(int msecs = 5000);

    // Signal ready to close
    void closing();

private:
    // Base flag. Bits as follows:
    //  1) Exit General
    //  2) Exit Send Done
    //  3) Exit Recv Done
    //  4) Reset active
    //  5) Reset send_chunk
    uint8_t base_flags;
    typedef enum {
        base_exit_flag = 0x01,
        base_exit_send_flag = 0x02,
        base_exit_recv_flag = 0x04,
        base_reset_flag = 0x08,
        base_send_chunk_flag = 0x10
    } base_flags_enum;

    // Send helper variables
    QMutex sendLock;
    QList<QByteArray> msgList;

    // Rcv helper variables
    QMutex rcvLock;
    QByteArray rcvd_raw;

    // Ack helper variables
    bool ack_status;
    uint8_t ack_key;
    QTimer ackTimer;
    QEventLoop ackLoop;

    // Data helper variables
    bool data_status;
    uint8_t data_key;
    QTimer dataTimer;
    QEventLoop dataLoop;

    // Chunk variables
    uint32_t chunk_size;

    // GUI List - Position == key
    QList<QList<GUI_BASE*>> known_guis;

    // Checksum info
    QList<checksum_struct> gui_checksums;
    static QMap<QString, checksum_struct> supportedChecksums;

    // Checksum helpers
    void getChecksum(const uint8_t* data, uint32_t data_len, uint8_t checksum_key,
                            uint8_t** checksum_array, uint32_t* checksum_size);
    static void copy_checksum_info(checksum_struct *cpy_to, checksum_struct *cpy_from);
    static void delete_checksum_info(checksum_struct *check);
    static void set_checksum_exe(checksum_struct *check, QString checksum_exe);
    static void set_checksum_start(checksum_struct *check, QString checksum_start, uint8_t checksum_start_base);

    // Transmit across connection
    void transmit(QByteArray data);

    // Checks if ready to exit
    void close_bridge();
};

#endif // GUI_COMM_BRIDGE_H
