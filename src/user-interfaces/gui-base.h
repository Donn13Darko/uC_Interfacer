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

#include <QWidget>
#include <QMap>
#include <QVariant>
#include <QMutex>
#include <QTimer>
#include <QEventLoop>

#include "gui-base-major-keys.h"
#include "../checksums/checksums.h"
#include "../gui-helpers/gui-helper.h"
#include <QDebug>

class GUI_BASE : public QWidget
{
    Q_OBJECT

public:
    GUI_BASE(QWidget *parent = 0);
    ~GUI_BASE();

    void reset_remote();
    void closing();

    // GUI Checksum functions
    void set_gui_checksum(QStringList new_gui_checksum);

    // Static checksum functions
    static QStringList get_supported_checksums();
    static void set_generic_checksum(QStringList new_generic_checksum);

    // Other static functions
    static size_t get_chunk_size();
    static void set_chunk_size(size_t chunk);
    static void parseGenericConfigMap(QMap<QString, QVariant>* configMap);

    // Virtual functions
    virtual void reset_gui();       // Must not call reset_remote()
    virtual uint8_t get_GUI_key();
    virtual void parseConfigMap(QMap<QString, QVariant>* configMap);

    // Static members
    static const uint32_t default_chunk_size = 32;

signals:
    void write_data(QByteArray data);
    void ackReceived(QByteArray ack);
    void readyRead(QByteArray data);
    void connect_signals(bool connect);
    void ackChecked(bool ackStatus);
    void resetting();

    void progress_update_recv(int progress, QString label);
    void progress_update_send(int progress, QString label);

protected slots:
    void receive(QByteArray recvData);
    void checkAck(QByteArray ack);

    // Virtual slots
    virtual void receive_gui(QByteArray recvData);
    virtual void on_ResetGUI_Button_clicked();
    virtual void set_progress_update_recv(int progress, QString label);
    virtual void set_progress_update_send(int progress, QString label);

protected:
    // Local variables
    uint8_t gui_key;

    // Receive arrays & variables
    QByteArray rcvd_formatted;
    uint32_t current_recv_length;
    uint32_t expected_recv_length;
    bool start_data;

    // Direct sends
    void send(QString data);
    void send(QByteArray data);
    void send(std::initializer_list<uint8_t> data);

    // File sending
    void send_file(uint8_t major_key, uint8_t minor_key, QString filePath);
    void send_file_chunked(uint8_t major_key, uint8_t minor_key, QString filePath, char sep);

    // Chunk sending
    void send_chunk(uint8_t major_key, uint8_t minor_key, QByteArray chunk, bool force_envelope = false);
    void send_chunk(uint8_t major_key, uint8_t minor_key, std::initializer_list<uint8_t> chunk, bool force_envelope = false);

    // Ack
    void send_ack(uint8_t majorKey);
    void waitForAck(int msecs = 5000);
    bool check_checksum(const uint8_t* data, uint32_t data_len, checksum_struct* check);

    // Wait for data
    void waitForData(int msecs = 5000);
    virtual bool isDataRequest(uint8_t minorKey);

    // Save to file
    void save_rcvd_formatted();

    // Other functions
    void set_expected_recv_length(QByteArray recv_length);
    void update_current_recv_length(QByteArray recvData);

private:
    // Private variables
    bool exit_dev;

    // Send helper variables
    QMutex sendLock;
    QList<QByteArray> msgList;
    bool reset_dev;

    // Recv helper variables
    QMutex recvLock;
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

    // GUI Checksum helpers
    checksum_struct gui_checksum;

    // Static class members
    static uint32_t chunk_size;
    static checksum_struct generic_checksum;
    static QMap<QString, checksum_struct> supportedChecksums;

    // Send to device
    void transmit(QByteArray data);
    void getChecksum(const uint8_t* data, uint32_t data_len, uint8_t checksum_key,
                     uint8_t** checksum_array, uint32_t* checksum_size);

    void close_base();
    bool send_closed;
    bool recv_closed;
};

#endif // GUI_BASE_H
