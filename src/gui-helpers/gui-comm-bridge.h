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
#include <QRegularExpression>
#include <QRegularExpressionMatch>

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
    uint32_t get_chunk_size();
    void set_chunk_size(uint32_t chunk);

    // Supported checksums
    static QStringList get_supported_checksums();

    // Checksum setters
    void set_tab_checksum(uint8_t gui_key, QStringList new_tab_checksum);

    // Parse input array
    void parseGenericConfigMap(QMap<QString, QVariant>* configMap);

    // Add/remove knowns guis
    void add_gui(GUI_BASE *new_gui);
    void remove_gui(GUI_BASE *old_gui);

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

    // File transmits
    void transmit_file(quint8 major_key, quint8 minor_key,
                       QString filePath, quint8 base,
                       QString encoding, GUI_BASE *sender);
    void transmit_file_pack(quint8 major_key, quint8 minor_key,
                            QString filePath, quint8 base,
                            QString encoding, GUI_BASE *sender);

    // Chunk transmits
    void transmit_chunk(quint8 major_key, quint8 minor_key,
                        QByteArray chunk, quint8 base,
                        QString encoding, GUI_BASE *sender);
    void transmit_chunk_pack(quint8 major_key, quint8 minor_key,
                             QByteArray chunk, quint8 base,
                             QString encoding, GUI_BASE *sender);

public slots:
    // Reset remtoe
    void reset_remote();

    // Receive data
    void receive(QByteArray recvData);

    // File sending
    void send_file(quint8 major_key, quint8 minor_key,
                   QString filePath, quint8 base = 0,
                   QString encoding = "^(.*)",
                   GUI_BASE *sender = nullptr);
    void send_file_pack(quint8 major_key, quint8 minor_key,
                        QString filePath, quint8 base = 0,
                        QString encoding = "^(.*)",
                        GUI_BASE *sender = nullptr);

    // Data chunk sending
    void send_chunk(quint8 major_key, quint8 minor_key,
                    QByteArray chunk = QByteArray(), quint8 base = 0,
                    QString encoding = "^(.*)", GUI_BASE *sender = nullptr);
    void send_chunk_pack(quint8 major_key, quint8 minor_key,
                         QByteArray chunk, quint8 base = 0,
                         QString encoding = "^(.*)",
                         GUI_BASE *sender = nullptr);

    // Signal bridge to open, close, or exit
    bool open_bridge();
    bool close_bridge();
    void destroy_bridge();

private slots:
    // Ack
    void send_ack(uint8_t majorKey);
    void waitForAck(int msecs = 5000);
    void checkAck(QByteArray ack);

    // Wait for data
    void waitForData(int msecs = 5000);

private:
    /* Bridge flag. Bits as follows:
     *  1) Exit Bridge
     *  2) Close Bridge
     *  3) Close Send Done
     *  4) Close Recv Done
     *  5) Reset remote active
     *  6) Reset send_chunk active
     *  7) Nothing
     *  8) Nothing
     */
    uint8_t bridge_flags;
    typedef enum {
        bridge_exit_flag = 0x01,
        bridge_close_flag = 0x02,
        bridge_close_send_flag = 0x04,
        bridge_close_recv_flag = 0x08,
        bridge_reset_flag = 0x10,
        bridge_reset_send_chunk_flag = 0x20
    } bridge_flags_enum;

    // Holds all info for waiting
    typedef struct send_struct {
        GUI_BASE* sender;
        uint8_t target;
        uint8_t major_key;
        uint8_t minor_key;
        uint8_t base;
        QVariant data;
        QString encoding;
    } send_struct;

    typedef enum {
        SEND_STRUCT_SEND_FILE = 0,
        SEND_STRUCT_SEND_FILE_PACK,
        SEND_STRUCT_SEND_CHUNK,
        SEND_STRUCT_SEND_CHUNK_PACK
    } SEND_STRUCT_TARGETS_ENUM;

    // Send helper variables
    QMutex sendLock;
    QList<send_struct> transmitList;

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
    QList<GUI_BASE*> known_guis;

    // Checksum info
    QList<checksum_struct> tab_checksums;
    static QMap<QString, checksum_struct> supportedChecksums;

    // Checksum main methods
    void getChecksum(const uint8_t* data, uint32_t data_len, uint8_t checksum_key,
                     uint8_t** checksum_array, uint32_t* checksum_size);
    bool check_checksum(const uint8_t* data, uint32_t data_len, const checksum_struct *check);

    // Checksum static helpers
    static void copy_checksum_info(checksum_struct *cpy_to, checksum_struct *cpy_from);
    static void delete_checksum_info(checksum_struct *check);
    static void set_checksum_exe(checksum_struct *check, QString checksum_exe);
    static void set_checksum_start(checksum_struct *check, QString checksum_start, uint8_t checksum_start_base);

    // Checks if packet requires special action
    void check_packet(uint8_t major_key);

    // Try to acquire sendLock
    bool get_send_lock(uint8_t major_key, uint8_t minor_key,
                       QVariant data, GUI_BASE *sending_gui,
                       uint8_t target, uint8_t base = 0,
                       QString regex = "^(.*)");

    // Handle send list
    void handle_next_send();

    // Transmit helpers
    void parse_file(quint8 major_key, quint8 minor_key,
                    QString filePath, quint8 base = 0,
                    QString encoding = "^(.*)",
                    GUI_BASE *sender = nullptr);
    QByteArray parse_data(quint8 major_key, quint8 minor_key, QByteArray data = QByteArray(),
                          quint8 base = 0,
                          QRegularExpression regex = QRegularExpression("^(.*)"),
                          bool send_updates = false, GUI_BASE *sender = nullptr,
                          quint32 c_pos = 0, quint32 t_pos = 0);
    QByteArray prepare_data(quint8 major_key, quint8 minor_key, QByteArray chunk = QByteArray());
    void transmit_data(QByteArray data);
};

#endif // GUI_COMM_BRIDGE_H
