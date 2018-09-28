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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QMap>
#include <QTimer>
#include <QCloseEvent>
#include <QSettings>

#include "communication/json-info.h"
#include "communication/serial-rs232.h"
#include "user-interfaces/gui-base.h"

typedef enum {
    GUI_TYPE_ERROR = 0,
    GUI_TYPE_IO,
    GUI_TYPE_DATA_TRANSMIT,
    GUI_TYPE_PROGRAMMING
} GUI_TYPE;

typedef enum {
    CONN_TYPE_ERROR = 0,
    CONN_TYPE_RS_232,
    CONN_TYPE_TCP,
    CONN_TYPE_UDP
} CONN_TYPE;

typedef enum {
    DEV_TYPE_ERROR = 0,
    DEV_TYPE_ARDUINO_UNO,
    DEV_TYPE_PC,
    DEV_TYPE_OTHER
} DEV_TYPE;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void closeEvent(QCloseEvent* e);
    static bool showMessage(QString msg);

signals:
    void write_data(QByteArray data);
    void write_data(std::initializer_list<uint8_t> data);

public slots:
    void connect_signals(bool connect);

private slots:
    void on_DeviceCombo_currentIndexChanged(int);
    void on_ConnTypeCombo_currentIndexChanged(int);
    void on_SpeedCombo_currentIndexChanged(int);

    void on_DeviceConnect_Button_clicked();
    void on_DeviceDisconnect_Button_clicked();
    void on_MoreOptions_Button_clicked();

    void updateConnInfoCombo();

    void on_ucOptions_currentChanged(int index);

    void receive(QByteArray) {/*Default do nothing*/}

private:
    Ui::MainWindow *ui;
    int prev_tab;

    QString deviceType;
    QString deviceINI;
    QString connType;
    QString speed;
    QString connInfo;

    static QStringList supportedDevicesList;
    static QStringList supportedProtocolsList;

    static QMap<QString, uint8_t> supportedGUIsMap;
    static QMap<QString, uint8_t> supportedDevicesMap;
    static QMap<QString, uint8_t> supportedProtocolsMap;

    QTimer updateConnInfo;
    Serial_RS232 *serial_rs232;

//    QSettings gui_settings;

    uint8_t arduino_chunk_size = 32;

    void updateSpeedCombo();
    void setConnected(bool conn);
    void reset_remote();
    void connect2sender(QObject* obj, bool conn);

    uint8_t getDevType();
    uint8_t getConnType();
    uint8_t getGUIType(QString type);
    QStringList getConnSpeeds();
    QObject* getConnObject(int type = -1);
};

#endif // MAINWINDOW_H
