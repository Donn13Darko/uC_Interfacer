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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "user-interfaces/gui-8aio-16dio-comm.h"
#include "user-interfaces/gui-data-transmit.h"
#include "user-interfaces/gui-programmer.h"
#include "user-interfaces/gui-custom-cmd.h"
#include "gui-helper.h"

#include <QMessageBox>
#include <QFileDialog>

// Setup supported GUIs list
QStringList
MainWindow::supportedGUIsList({
                                  "GENERAL SETTINGS",
                                  "Welcome",
                                  "IO",
                                  "Data Transmit",
                                  "Programmer",
                                  "Custom CMD"
                             });

// Setup supported GUIs map
QMap<QString, uint8_t>
MainWindow::supportedGUIsMap({
                                 {"GENERAL SETTINGS", GUI_TYPE_GENERAL_SETTINGS},
                                 {"Welcome", GUI_TYPE_WELCOME},
                                 {"IO", GUI_TYPE_IO},
                                 {"Data Transmit", GUI_TYPE_DATA_TRANSMIT},
                                 {"Programmer", GUI_TYPE_PROGRAMMER},
                                 {"Custom CMD", GUI_TYPE_CUSTOM_CMD}
                             });

// Setup suported checksums map
QMap<QString, checksum_struct>
MainWindow::supportedChecksums({
                                   {"CRC_8_LUT", {get_crc_8_LUT_size, get_crc_8_LUT, check_crc_8_LUT}},
                                   {"CRC_8_POLY", {get_crc_8_POLY_size, get_crc_8_POLY, check_crc_8_POLY}},
                                   {"CRC_16_LUT", {get_crc_16_LUT_size, get_crc_16_LUT, check_crc_16_LUT}},
                                   {"CRC_16_POLY", {get_crc_16_POLY_size, get_crc_16_POLY, check_crc_16_POLY}},
                                   {"CRC_32_LUT", {get_crc_32_LUT_size, get_crc_32_LUT, check_crc_32_LUT}},
                                   {"CRC_32_POLY", {get_crc_32_POLY_size, get_crc_32_POLY, check_crc_32_POLY}},
                                   {"CHECKSUM_EXE", {get_checksum_exe_size, get_checksum_exe, check_checksum_exe}}
                               });

// Setup static supported devices list
QStringList
MainWindow::supportedDevicesList({
                                     "Arduino Uno",
                                     "PC",
                                     "Local Programmer",
                                     "Other"
                                 });

// Setup static supported protocols list
QStringList
MainWindow::supportedProtocolsList({
                                       "RS-232",
                                       "TCP Client",
                                       "TCP Server",
                                       "UDP Socket"
                                   });

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Setup ui
    ui->setupUi(this);
    setWindowFlags(windowFlags()
                   | Qt::MSWindowsFixedSizeDialogHint);

    // Setup Welcome widget
    welcome_tab = new GUI_WELCOME(this);
    welcome_tab_text = "Welcome";

    // Setup More Options Dialog
    more_options_settings.reset_on_tab_switch = true;
    more_options_settings.send_little_endian = false;
    more_options_settings.chunk_size = GUI_BASE::default_chunk_size;
    more_options = new GUI_MORE_OPTIONS(&more_options_settings, supportedGUIsList, supportedChecksums.keys());

    // Set base parameters
    prev_tab = -1;
    device = nullptr;
    updateConnInfo = new QTimer();

    // Add specified values to combos
    ui->Device_Combo->clear();
    ui->Device_Combo->addItems(MainWindow::supportedDevicesList);
    ui->ConnType_Combo->clear();
    ui->ConnType_Combo->addItems(MainWindow::supportedProtocolsList);

    // Set Initial values
    setConnected(false);
    on_Device_Combo_activated(ui->Device_Combo->currentIndex());
    on_ConnType_Combo_currentIndexChanged(0);
    ui->ucOptions->addTab(welcome_tab, welcome_tab_text);

    // Add connections
    connect(updateConnInfo, SIGNAL(timeout()), this, SLOT(updateConnInfoCombo()));
}

MainWindow::~MainWindow()
{
    // If connected, disconnect
    if (deviceConnected())
    {
        on_DeviceDisconnect_Button_clicked();
    }

    updateConnInfo->stop();
    delete updateConnInfo;
    delete welcome_tab;
    delete more_options;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent* e)
{
    // If connected, disconnect
    if (deviceConnected())
    {
        on_DeviceDisconnect_Button_clicked();
    }

    e->accept();
}

void MainWindow::connect_signals(bool conn)
{
    // Get curr widget
    QObject* curr_widget = (QObject*) ui->ucOptions->currentWidget();

    // Exit if no widget found
    if (!curr_widget) return;

    // Connect signals to slots
    connect2sender(curr_widget, conn);
}

void MainWindow::on_Device_Combo_activated(int)
{
    // Save previous value
    QString prev_deviceINI = deviceINI;

    // Get new INI file
    switch (getDevType())
    {
        case DEV_TYPE_ARDUINO_UNO:
            deviceINI = ":/uc-interfaces/gui-arduino-uno.ini";
            break;
        case DEV_TYPE_PC:
            deviceINI = ":/uc-interfaces/gui-pc.ini";
            break;
        case DEV_TYPE_OTHER:
            deviceINI = QFileDialog::getOpenFileName(this, tr("Open"), "",
                                                     tr("INI (*.ini);;All Files (*.*)"));;
            break;
        default:
            deviceINI = "";
            break;
    }

    // Failed config load
    if (deviceINI.isEmpty())
    {
        // Update welcome tab msg with failure
        welcome_tab->setMsg("Failed to locate config for "
                            + ui->Device_Combo->currentText()
                            + "!\nReverted to previous config: "
                            + prev_deviceINI);

        // Revert to previous dev type
        ui->Device_Combo->setCurrentIndex(deviceType-1);
        deviceINI = prev_deviceINI;
        return;
    } else
    {
        // Get new device and set welcome tab string
        deviceType = getDevType();
        welcome_tab->setMsg("Current Config: " + deviceINI);
    }
}

void MainWindow::on_ConnType_Combo_currentIndexChanged(int)
{
    // Process changes to the speed & info combos
    updateSpeedCombo();
    updateConnInfoCombo();
}

void MainWindow::on_DeviceConnect_Button_clicked()
{
    // Disable input switching
    setConnected(true);

    // Update selected info
    QString connInfo = ui->ConnInfo_Combo->currentText();
    QString speed = ui->Speed_Combo->currentText();

    // Try to connect to the device
    switch (getConnType())
    {
        case CONN_TYPE_RS_232:
        {
            // Create new object
            device = new Serial_RS232(connInfo, speed);
            break;
        }
        case CONN_TYPE_TCP_CLIENT:
        {
            // Parse input
            QStringList conn = connInfo.split(':');
            if (conn.length() != 2) break;

            // Create new object
            device = new TCP_CLIENT(conn[0], conn[1].toInt());
            break;
        }
        case CONN_TYPE_TCP_SERVER:
        {
            // Create new object
            device = new TCP_SERVER(connInfo.toInt());
            break;
        }
        case CONN_TYPE_UDP_SOCKET:
        {
            // Parse input
            QStringList conn = connInfo.split(':');
            if (conn.length() != 3) break;

            // Create new object
            device = new UDP_SOCKET(conn[0], conn[1].toInt(), conn[2].toInt());
            break;
        }
        default:
        {
            return;
        }
    }

    if (!device)
    {
        setConnected(false);
        return;
    }

    // Connect signals and slots
    connect(device, SIGNAL(deviceConnected()),
               this, SLOT(on_DeviceConnected()));
    connect(device, SIGNAL(deviceDisconnected()),
               this, SLOT(on_DeviceDisconnected()));

    // Try to connect
    device->open();
}

void MainWindow::on_DeviceConnected() {
    // Disconnect connection signals from device
    if (device)
    {
        disconnect(device, SIGNAL(deviceConnected()),
                   this, SLOT(on_DeviceConnected()));
    }

    // If connected, add relevant tabs
    // Else, show failed to connect message
    if (deviceConnected())
    {
        // Remove all existing tabs
        ucOptionsClear();

        // Reset & load the GUI settings file
        // Read config settings
        QMap<QString, QMap<QString, QVariant>*>* configMap = \
                GUI_HELPER::readConfigINI(deviceINI);

        // Block signals from tab group
        ui->ucOptions->blockSignals(true);

        // Setup tabs
        bool contains_general_settings = false;
        uint8_t gui_type;
        QMap<QString, QVariant>* groupMap;
        QWidget* tab_holder;
        QString checksum_type;
        foreach (QString childGroup, configMap->keys())
        {
            // Verify that its a known GUI
            gui_type = getGUIType(childGroup.split('_').last());
            if (gui_type == GUI_TYPE_ERROR) continue;

            // Get group QMap
            groupMap = configMap->value(childGroup);

            // Instantiate and add GUI
            switch (gui_type)
            {
                case GUI_TYPE_GENERAL_SETTINGS:
                {
                    // Set flag to handle after reading in all other tabs
                    contains_general_settings = true;
                    continue;
                }
                case GUI_TYPE_WELCOME:
                {
                    tab_holder = new GUI_WELCOME(this);
                    GUI_WELCOME* welcome_holder = \
                            (GUI_WELCOME*) tab_holder;

                    welcome_holder->setHeader(groupMap->value("header", welcome_tab_text).toString());
                    welcome_holder->setMsg(groupMap->value("msg").toString());
                    break;
                }
                case GUI_TYPE_IO:
                {
                    tab_holder = new GUI_8AIO_16DIO_COMM(this);
                    GUI_8AIO_16DIO_COMM* io_holder = \
                            (GUI_8AIO_16DIO_COMM*) tab_holder;

                    // Parse the config map
                    io_holder->parseConfigMap(groupMap);

                    // Exit switch
                    break;
                }
                case GUI_TYPE_DATA_TRANSMIT:
                {
                    tab_holder = new GUI_DATA_TRANSMIT(this);
                    break;
                }
                case GUI_TYPE_PROGRAMMER:
                {
                    tab_holder = new GUI_PROGRAMMER(this);
                    GUI_PROGRAMMER* programmer_holder = \
                            (GUI_PROGRAMMER*) tab_holder;

                    programmer_holder->addHexFormats(groupMap->value("hex_formats").toStringList());
                    programmer_holder->removeHexFormats(groupMap->value("hex_formats_rm").toStringList());
                    programmer_holder->addBurnMethods(groupMap->value("burn_methods").toStringList());
                    break;
                }
                case GUI_TYPE_CUSTOM_CMD:
                {
                    tab_holder = new GUI_CUSTOM_CMD(this);
                    break;
                }
                default:
                {
                    continue;
                }
            }

            // Get gui type checksum encoding
            checksum_type = groupMap->value("checksum_type").toString();

            // Set checksum type struct (default CRC_8_LUT)
            ((GUI_BASE*) tab_holder)->set_gui_checksum(
                        supportedChecksums.value(
                            checksum_type,
                            supportedChecksums.value("CRC_8_LUT")));

            // If checksum_type == "CHECKSUM_EXE", set executable path
            if (checksum_type == "CHECKSUM_EXE")
            {
                ((GUI_BASE*) tab_holder)->set_gui_checksum(
                            groupMap->value("checksum_exe").toString());
            }

            // Add new GUI to tabs
            ui->ucOptions->addTab(tab_holder, groupMap->value("tab_name", childGroup).toString());
        }

        // Handle any changes in more options
        update_more_options();

        // Handle generic settings now that all tabs are loaded
        if (contains_general_settings)
        {
            // Get settings group
            QString gui_name = getGUIName(GUI_TYPE_GENERAL_SETTINGS);
            groupMap = configMap->value(gui_name);

            // Holder for each setting
            QString setting;

            // Check general checksum type setting
            // (only sets if not set at runtime in more options)
            setting = groupMap->value("checksum_type").toString();
            if (!setting.isEmpty()
                    && !more_options_settings.checksum_map.contains(gui_name))
            {
                // Only need to set one instance (static)
                // Set checksum type struct
                checksum_struct gen_check = supportedChecksums.value(
                            groupMap->value("generic_checksum_type").toString(),
                            supportedChecksums.value("CRC_8_LUT"));
                GUI_BASE::set_generic_checksum(gen_check);

                // If checksum_type == "CHECKSUM_EXE", set executable path
                if (setting == "CHECKSUM_EXE")
                {
                    QString checksum_exe_path = groupMap->value("checksum_exe").toString();
                    GUI_BASE::set_generic_checksum(checksum_exe_path);
                }
            }

            // Check reset tab setting (forces false from INI)
            if (!groupMap->value("reset_tabs_on_switch", "true").toBool())
            {
                more_options_settings.reset_on_tab_switch = false;
            }

            // Check little endian setting (forces a true from INI)
            if (groupMap->value("send_little_endian", "false").toBool())
            {
                more_options_settings.send_little_endian = true;
            }

            // Update chunk only if not changed from the default
            if (more_options_settings.chunk_size == GUI_BASE::default_chunk_size)
            {
                // Set base chunk size to config value or default value if non-existant
                GUI_BASE::set_chunk_size(groupMap->value("chunk_size", QString::number(GUI_BASE::default_chunk_size)).toInt());
            }
        }

        // Enable signals for tab group
        ui->ucOptions->blockSignals(false);

        // Delete map after use
        GUI_HELPER::deleteConfigMap(configMap);

        // Freshen tabs for first use
        on_ucOptions_currentChanged(ui->ucOptions->currentIndex());

        // Call reset remote twice
        // Reseting generally fails the first time after new connection
        reset_remote();
        reset_remote();
    } else
    {
        GUI_HELPER::showMessage("Error: Unable to connect to target!");
    }

    setConnected(deviceConnected());
}

void MainWindow::on_DeviceDisconnected()
{
    // Remove widgets from GUI
    on_DeviceDisconnect_Button_clicked();

    // Notify user of connection loss
    GUI_HELPER::showMessage("Error: Connection to target lost!");
}

void MainWindow::on_DeviceDisconnect_Button_clicked()
{
    // Reset the remote if connected
    if (deviceConnected()) reset_remote();

    // Disconnect any connected slots
    if (device)
    {
        disconnect(device, SIGNAL(deviceConnected()),
                   this, SLOT(on_DeviceConnected()));
        disconnect(device, SIGNAL(deviceDisconnected()),
                   this, SLOT(on_DeviceDisconnected()));
    }

    // Disconnect connections
    connect2sender(ui->ucOptions->currentWidget(), false);

    if (device)
    {
        device->close();
        device->deleteLater();
        device = nullptr;
    }

    // Remove widgets
    ucOptionsClear();

    // Add welcome widget
    ui->ucOptions->addTab(welcome_tab, welcome_tab_text);

    // Set to disconnected mode
    setConnected(false);

    // Refresh device & conn type combos
    on_ConnType_Combo_currentIndexChanged(0);
}

void MainWindow::on_MoreOptions_Button_clicked()
{
    // Reset More Options dialog
    more_options->reset_gui();

    // Run dialog and set values if connected
    if (more_options->exec() && deviceConnected())
    {
        update_more_options();
    }
}

void MainWindow::on_ucOptions_currentChanged(int index)
{
    // No change
    if (prev_tab == index) return;

    // Disconnet old signals
    if (prev_tab != -1)
    {
        QObject* prev_widget = ui->ucOptions->widget(prev_tab);
        if (prev_widget)
        {
            // Disconnect slots/signals
            connect2sender(prev_widget, false);
            disconnect(prev_widget, SIGNAL(connect_signals(bool)),
                       this, SLOT(connect_signals(bool)));

            // Reset the previous GUI (uses virtual function)
            ((GUI_BASE*) prev_widget)->reset_gui();
        }
    }

    // Connect new signals
    QObject* curr_widget = (QObject*) ui->ucOptions->currentWidget();
    if (curr_widget)
    {
        // Enable device to manage signals
        connect(curr_widget, SIGNAL(connect_signals(bool)),
                this, SLOT(connect_signals(bool)));

        // Set signal acceptance to true
        connect_signals(true);
    }

    // Update previous tab index
    prev_tab = index;

    // Reset the Remote for the new tab (if connected)
    if (deviceConnected()
            && more_options_settings.reset_on_tab_switch)
    {
        reset_remote();
    }
}

void MainWindow::updateConnInfoCombo()
{
    // Stop timers and disable connect if device connected
    if (deviceConnected())
    {
        updateConnInfo->stop();
        ui->DeviceConnect_Button->setEnabled(false);
        return;
    }

    // Otherwise handle based on connection type
    switch (getConnType())
    {
        case CONN_TYPE_RS_232:
        {
            if (!updateConnInfo->isActive()) {
                updateConnInfo->start(1000);
                ui->ConnInfo_Combo->setEditable(false);
            }

            QStringList* avail = Serial_RS232::getDevices();
            if (avail->length() != 0)
            {
                QString curr = ui->ConnInfo_Combo->currentText();
                ui->ConnInfo_Combo->clear();
                ui->ConnInfo_Combo->addItems(*avail);
                ui->ConnInfo_Combo->setCurrentText(curr);
                ui->DeviceConnect_Button->setEnabled(true);
            } else
            {
                ui->ConnInfo_Combo->clear();
                ui->DeviceConnect_Button->setEnabled(false);
            }
            delete avail;
            break;
        }
        default:
        {
            // Stop timer and set connect enabled
            updateConnInfo->stop();
            ui->DeviceConnect_Button->setEnabled(true);

            // If changing from autofill, clear and make editable
            ui->ConnInfo_Combo->clear();
            ui->ConnInfo_Combo->setEditable(true);

            break;
        }
    }
}

void MainWindow::updateSpeedCombo()
{
    // Get speed list
    QStringList newItems = getConnSpeeds();

    // Load into combo
    if (newItems.length() == 0)
    {
        ui->Speed_Combo->clear();
        ui->Speed_Combo->setEnabled(false);
    } else
    {
        // Save previous speed
        QString prevSpeed = ui->Speed_Combo->currentText();

        // Clear and load new speed list in
        ui->Speed_Combo->clear();
        ui->Speed_Combo->setEnabled(true);
        ui->Speed_Combo->addItems(newItems);

        // Set to previous speed if in new items
        if (newItems.contains(prevSpeed))
        {
            ui->Speed_Combo->setCurrentText(prevSpeed);
        }
    }
}

void MainWindow::setConnected(bool conn)
{
    bool op_conn = !conn;

    // Set Combos Enabled
    ui->Device_Combo->setEnabled(op_conn);
    ui->ConnType_Combo->setEnabled(op_conn);
    ui->Speed_Combo->setEnabled(op_conn);
    ui->ConnInfo_Combo->setEnabled(op_conn);

    // Set Buttons Enabled
    ui->DeviceConnect_Button->setEnabled(op_conn);
    ui->DeviceDisconnect_Button->setEnabled(conn);
}

void MainWindow::reset_remote()
{
    GUI_BASE* curr = (GUI_BASE*) ui->ucOptions->currentWidget();
    if (curr) curr->reset_remote();
}

void MainWindow::connect2sender(QObject* obj, bool conn)
{
    // Get connection type & verify connection
    if (!obj || !device) return;

    // Connect or disconnect signals
    if (conn) {
        connect(obj, SIGNAL(write_data(QByteArray)),
                device, SLOT(write(QByteArray)),
                Qt::QueuedConnection);

        connect(device, SIGNAL(readyRead(QByteArray)),
                obj, SLOT(receive(QByteArray)),
                Qt::QueuedConnection);
    } else {
        disconnect(obj, SIGNAL(write_data(QByteArray)),
                   device, SLOT(write(QByteArray)));

        disconnect(device, SIGNAL(readyRead(QByteArray)),
                   obj, SLOT(receive(QByteArray)));
    }
}

void MainWindow::ucOptionsClear()
{
    ui->ucOptions->blockSignals(true);
    QWidget* tab;
    for (int i = ui->ucOptions->count(); 0 < i; i--)
    {
        tab = ui->ucOptions->widget(0);
        ui->ucOptions->removeTab(0);
        if (tab != welcome_tab) delete tab;
    }
    ui->ucOptions->blockSignals(false);

    prev_tab = -1;
}

bool MainWindow::deviceConnected()
{
    // Verify connection object exists
    if (!device) return false;
    else return device->isConnected();
}

uint8_t MainWindow::getConnType()
{
    // currentIndex() returns -1 on error which maps to CONN_TYPE_ERROR
    return (uint8_t) (ui->ConnType_Combo->currentIndex()+1);
}

uint8_t MainWindow::getDevType()
{
    // currentIndex() returns -1 on error which maps to DEV_TYPE_ERROR
    return (uint8_t) (ui->Device_Combo->currentIndex()+1);
}

uint8_t MainWindow::getGUIType(QString type)
{
    return supportedGUIsMap.value(type, GUI_TYPE_ERROR);
}

QString MainWindow::getGUIName(uint8_t type)
{
    return supportedGUIsList.at(type-(GUI_TYPE_ERROR+1));
}

QStringList MainWindow::getConnSpeeds()
{
    switch (getConnType())
    {
        case CONN_TYPE_RS_232: return Serial_RS232::Baudrate_Defaults;
        default: return {};
    }
}

void MainWindow::update_more_options()
{
    // Set global runtime values
    // Set chunk size
    GUI_BASE::set_chunk_size(more_options_settings.chunk_size);

    // Set generic checksum
    QStringList checksum_info;
    QString gui_name = getGUIName(GUI_TYPE_GENERAL_SETTINGS);
    if (more_options_settings.checksum_map.contains(gui_name))
    {
        // Get checksum setting
        checksum_info = more_options_settings.checksum_map.value(gui_name);

        // Set the new generic checksum
        GUI_BASE::set_generic_checksum(checksum_info.at(1));
        GUI_BASE::set_generic_checksum(supportedChecksums.value(checksum_info.at(0)));
    }

    // Set class values
    QStringList checksum_changes = more_options_settings.checksum_map.keys();

    // Iterate over each tab and apply any changes
    GUI_BASE* tab_holder;
    uint8_t num_tabs = ui->ucOptions->count();
    for (uint8_t i = 0; i < num_tabs; i++)
    {
        // Get tab
        tab_holder = (GUI_BASE*) ui->ucOptions->widget(i);
        if (tab_holder == 0) continue;

        // Set gui checksum
        gui_name = getGUIName(tab_holder->get_GUI_type());
        if (checksum_changes.contains(gui_name))
        {
            // Get checksum setting
            checksum_info = more_options_settings.checksum_map.value(gui_name);

            // Set the new gui checksum
            tab_holder->set_gui_checksum(checksum_info.at(1));
            tab_holder->set_gui_checksum(supportedChecksums.value(checksum_info.at(0)));
        }
    }
}
