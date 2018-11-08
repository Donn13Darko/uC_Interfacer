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

#include "gui-helpers/gui-create-new-tabs.h"
#include "gui-helpers/gui-more-options.h"
#include "gui-helpers/gui-comm-bridge.h"

#include "communication/serial-com-port.h"
#include "communication/tcp-client.h"
#include "communication/tcp-server.h"
#include "communication/udp-socket.h"

#include "user-interfaces/gui-base-major-keys.h"
#include "user-interfaces/gui-welcome.h"

// Must be in same order as supportedDevicesList
typedef enum {
    DEV_TYPE_ERROR = 0,
    DEV_TYPE_ARDUINO_UNO,
    DEV_TYPE_PC,
    DEV_TYPE_LOCAL_PROGRAMMER,
    DEV_TYPE_OTHER
} DEV_TYPE;

// Must be in same order as supportedProtocolsList
typedef enum {
    CONN_TYPE_ERROR = 0,
    CONN_TYPE_SERIAL_COM_PORT,
    CONN_TYPE_TCP_CLIENT,
    CONN_TYPE_TCP_SERVER,
    CONN_TYPE_UDP_SOCKET
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

    void closeEvent(QCloseEvent *e);

private slots:
    void on_Device_Combo_activated(int);
    void on_ConnType_Combo_currentIndexChanged(int);
    void on_Speed_Combo_activated(int);

    void on_DeviceConnect_Button_clicked();
    void on_DeviceDisconnect_Button_clicked();

    void on_MoreOptions_Button_clicked();
    void moreOptions_accepted();

    void createNewTabs_accepted();
    void on_removeTab();

    void on_DeviceConnected();
    void on_DeviceDisconnected();

    void on_ucOptions_currentChanged(int index);

    void updateConnInfoCombo();

private:
    // Main GUI
    Ui::MainWindow *ui;

    // Default Welcome tab
    GUI_WELCOME *welcome_tab;

    // '+' tab (blank base gui + create tab dialog)
    GUI_BASE *add_new_tab;
    GUI_CREATE_NEW_TABS *new_tab_gui;

    // More options dialog
    GUI_MORE_OPTIONS *more_options;
    MoreOptions_struct main_options_settings;
    MoreOptions_struct *local_options_settings;

    // Comm Bridge
    GUI_COMM_BRIDGE *comm_bridge;

    // Tab holder
    int prev_tab;

    // Device helpers
    uint8_t deviceType;
    QString deviceINI;

    static QStringList supportedGUIsList;
    static QStringList supportedDevicesList;
    static QStringList supportedProtocolsList;
    QMap<QString, QMap<QString, QVariant>*> *configMap;

    QTimer *updateConnInfo;
    COMMS_BASE *device;
    QString speed;

    void updateSpeedCombo();
    void setConnected(bool conn);
    void ucOptionsClear();
    bool deviceConnected();

    uint8_t getDevType();
    uint8_t getConnType();
    uint8_t getGUIType(QString type);
    QString getGUIName(uint8_t type);
    QStringList getConnSpeeds();

    // Create a new gui based on the configuration info
    GUI_BASE *create_new_tab(uint8_t gui_key, QMap<QString, QVariant> *guiConfigMap);

    // More options gui parser
    void update_options(MoreOptions_struct *options);

    // Connection option parsers
    void options_serial_com_port(MoreOptions_struct *options,
                                 QMap<QString, QVariant> *groupMap,
                                 Serial_COM_Port_Settings *settings);
};

#endif // MAINWINDOW_H
