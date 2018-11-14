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
#include <QTemporaryFile>

#include "gui-base-major-keys.h"
#include "../gui-helpers/gui-generic-helper.h"
#include <QDebug>

class GUI_BASE : public QWidget
{
    Q_OBJECT

public:
    GUI_BASE(QWidget *parent = 0);
    ~GUI_BASE();

    virtual bool isClosable();
    virtual void setClosable(bool new_close);

    virtual uint8_t get_GUI_key();

    virtual QString get_GUI_tab_name();
    virtual void set_GUI_tab_name(QString new_name);

    virtual QString get_GUI_config();

    virtual bool acceptAllCMDs();

    virtual void parseConfigMap(QMap<QString, QVariant> *configMap);

    virtual bool waitForDevice(uint8_t minorKey);

signals:
    // Read updates
    void readyRead(QByteArray data);

    // File transmit
    void transmit_file(quint8 major_key, quint8 minor_key,
                       QString filePath, quint8 base = 0,
                       QString encoding = "^(.*)");
    void transmit_file_pack(quint8 major_key, quint8 minor_key,
                            QString filePath, quint8 base = 0,
                            QString encoding = "^(.*)");

    // Chunk transmit
    void transmit_chunk(quint8 major_key, quint8 minor_key,
                        QByteArray chunk = QByteArray(), quint8 base = 0,
                        QString encoding = "^(.*)");
    void transmit_chunk_pack(quint8 major_key, quint8 minor_key,
                             QByteArray chunk = QByteArray(), quint8 base = 0,
                             QString encoding = "^(.*)");

    // Send progress bar updates
    void progress_update_recv(int progress, QString label);
    void progress_update_send(int progress, QString label);

public slots:
    // Resets the GUI (must not call reset_remote())
    virtual void reset_gui();

protected slots:
    // Receive and parse data
    virtual void receive_gui(QByteArray recvData);

    // Reset the gui and connected device
    virtual void on_ResetGUI_Button_clicked();

    // Set progress bar updates
    virtual void set_progress_update_recv(int progress, QString label);
    virtual void set_progress_update_send(int progress, QString label);

    // Chunk sending from list
    void send_chunk(uint8_t major_key, uint8_t minor_key, std::initializer_list<uint8_t> chunk);

protected:
    // Local variables
    uint8_t gui_key;
    QString gui_name;
    QString gui_tab_name;
    bool closable;
    QString gui_config;

    // Receive arrays & variables
    QTemporaryFile rcvd_formatted;
    uint32_t current_recv_length;
    uint32_t expected_recv_length;
    QString expected_recv_length_str;

    // Save to file
    void save_rcvd_formatted();

    // Other functions
    void set_expected_recv_length(uint32_t expected_length);
    void update_current_recv_length(uint32_t recv_len);
};

#endif // GUI_BASE_H
