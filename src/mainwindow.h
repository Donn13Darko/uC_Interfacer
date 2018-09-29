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
#include "user-interfaces/gui-welcome.h"

// Order not important (supportedGUIsMap handles)
typedef enum {
    GUI_TYPE_ERROR = 0,
    GUI_TYPE_WELCOME,
    GUI_TYPE_IO,
    GUI_TYPE_DATA_TRANSMIT,
    GUI_TYPE_PROGRAMMER
} GUI_TYPE;

// Must be in same order as supportedDevicesList
typedef enum {
    DEV_TYPE_ERROR = 0,
    DEV_TYPE_ARDUINO_UNO,
    DEV_TYPE_PC,
    DEV_TYPE_OTHER
} DEV_TYPE;

// Must be in same order as supportedProtocolsList
typedef enum {
    CONN_TYPE_ERROR = 0,
    CONN_TYPE_RS_232,
    CONN_TYPE_TCP_CLIENT,
    CONN_TYPE_TCP_SERVER,
    CONN_TYPE_UDP
} CONN_TYPE;

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
    void on_DeviceCombo_activated(int);
    void on_ConnTypeCombo_currentIndexChanged(int);

    void on_DeviceConnect_Button_clicked();
    void on_DeviceDisconnect_Button_clicked();
    void on_MoreOptions_Button_clicked();

    void on_ucOptions_currentChanged(int index);

    void updateConnInfoCombo();

    void receive(QByteArray) {/*Default do nothing*/}

private:
    Ui::MainWindow *ui;
    GUI_WELCOME* welcome_tab;
    QString welcome_tab_text;
    int prev_tab;

    uint8_t deviceType;
    QString deviceINI;

    static QStringList supportedDevicesList;
    static QStringList supportedProtocolsList;
    static QMap<QString, uint8_t> supportedGUIsMap;

    QTimer updateConnInfo;
    Serial_RS232 *serial_rs232;

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
