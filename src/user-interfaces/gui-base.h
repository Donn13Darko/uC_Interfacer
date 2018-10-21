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

#ifndef GUI_BASE_H
#define GUI_BASE_H

#include <QObject>
#include <QWidget>
#include <QMap>
#include <QVariant>
#include <QMutex>

#include "../communication/comms-general.h"
#include "../checksums/checksums.h"
#include "../gui-helper.h"
#include <QDebug>

class GUI_BASE : public QWidget
{
    Q_OBJECT

public:
    GUI_BASE(QWidget *parent = 0);
    ~GUI_BASE();

    void reset_remote();

    void set_gui_checksum(QString new_gui_checksum);
    void set_gui_checksum(checksum_struct new_gui_checksum);

    // Static functions
    static void set_chunkSize(size_t chunk);
    static void set_generic_checksum(QString new_generic_checksum);
    static void set_generic_checksum(checksum_struct new_generic_checksum);

    // Virtual functions
    virtual void reset_gui();

signals:
    void write_data(QByteArray data);
    void connect_signals(bool connect);
    void readyRead();
    void ackReceived();

protected slots:
    void receive(QByteArray recvData);

    // Virtual slots
    virtual void receive_gui();

protected:
    // Local variables
    const float S2MS = 1000.0f;
    uint8_t guiType;

    // Receive arrays
    QByteArray rcvd_raw;
    QByteArray rcvd_formatted;

    // Direct sends
    void send(QString data);
    void send(QByteArray data);
    void send(std::initializer_list<uint8_t> data);

    // File sending
    void send_file(QByteArray start, QString filePath);
    void send_file_chunked(QByteArray start, QString filePath, char sep);

    // Chunk sending
    void send_chunk(QByteArray start, QByteArray chunk);
    void send_chunk(std::initializer_list<uint8_t> start, QByteArray chunk);
    void send_chunk(QByteArray start, std::initializer_list<uint8_t> chunk);
    void send_chunk(std::initializer_list<uint8_t> start, std::initializer_list<uint8_t> chunk);

    // Ack
    void waitForAck(int msecs = 5000);
    bool checkAck();
    bool check_checksum(const uint8_t* data, uint32_t data_len, checksum_struct* check);

    // Save to file
    void save_rcvd_formatted();

private:
    bool ack_status;
    QMutex sendLock;
    uint8_t ack_key;
    QList<QByteArray> msgList;

    bool gui_checksum_is_exe = false;
    QString gui_checksum_exe_path = "";
    checksum_struct gui_checksum{get_crc_8_LUT_size, get_crc_8_LUT, check_crc_8_LUT};

    // Static class members
    static uint8_t chunkSize;
    static bool generic_checksum_is_exe;
    static QString generic_checksum_exe_path;
    static checksum_struct generic_checksum;

    // Send to device
    void transmit(QByteArray data);
};

#endif // GUI_BASE_H
