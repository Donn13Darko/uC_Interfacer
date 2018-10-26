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
#include "gui-helpers/gui-helper.h"

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
    welcome_tab_label = "Welcome";

    // Setup More Options Dialog
    main_options_settings.reset_on_tab_switch = true;
    main_options_settings.send_little_endian = false;
    main_options_settings.chunk_size = GUI_BASE::default_chunk_size;
    more_options = new GUI_MORE_OPTIONS(&main_options_settings, &local_options_settings,
                                        supportedGUIsList, GUI_BASE::get_supported_checksums());

    // Set base parameters
    prev_tab = -1;
    device = nullptr;
    configMap = nullptr;
    speed = "";
    updateConnInfo = new QTimer();

    // Add values to Device combo
    ui->Device_Combo->blockSignals(true);
    ui->Device_Combo->clear();
    ui->Device_Combo->addItems(MainWindow::supportedDevicesList);
    ui->Device_Combo->blockSignals(false);

    // Add values to conn type combo
    ui->ConnType_Combo->blockSignals(true);
    ui->ConnType_Combo->clear();
    ui->ConnType_Combo->addItems(MainWindow::supportedProtocolsList);
    ui->ConnType_Combo->blockSignals(false);

    // Set Initial values
    setConnected(false);
    on_Device_Combo_activated(ui->Device_Combo->currentIndex());
    on_ConnType_Combo_currentIndexChanged(0);
    ui->ucOptions->addTab(welcome_tab, welcome_tab_label);

    // Add connections
    connect(updateConnInfo, SIGNAL(timeout()),
            this, SLOT(updateConnInfoCombo()));
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
    deviceConnected();
    // If connected, disconnect
    if (deviceConnected())
    {
        on_DeviceDisconnect_Button_clicked();
    }
    updateConnInfo->stop();

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
            deviceINI = ":/uc-interfaces/arduino-uno-uart-example.ini";
            break;
        case DEV_TYPE_PC:
            deviceINI = ":/uc-interfaces/pc-example.ini";
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

void MainWindow::on_Speed_Combo_activated(int)
{
    switch (getConnType())
    {
        case CONN_TYPE_RS_232:
        {
            if (ui->Speed_Combo->currentText() == "Other")
                GUI_HELPER::getUserString(&speed, "Custom Baudrate", "Baudrate");
            else
                speed = ui->Speed_Combo->currentText();
            break;
        }
        default:
            break;
    }
}

void MainWindow::on_DeviceConnect_Button_clicked()
{
    // Disable input switching
    setConnected(true);

    // Reset the config settings file
    if (configMap)
    {
        GUI_HELPER::deleteConfigMap(configMap);
        configMap = nullptr;
    }

    // Read config settings
    configMap = GUI_HELPER::readConfigINI(deviceINI);

    // Update selected info
    QString connInfo = ui->ConnInfo_Combo->currentText();

    // Try to connect to the device
    switch (getConnType())
    {
        case CONN_TYPE_RS_232:
        {
            // Create and fill new settings struct
            Serial_RS232_Settings dev_settings = Serial_RS232_Settings_DEFAULT;
            dev_settings.port = connInfo;
            dev_settings.baudrate = speed.toInt();

            // Parse custom list & config map
            if (configMap->contains(ui->ConnType_Combo->currentText())
                                 || !main_options_settings.custom.isEmpty())
            {
                // Create holding list
                QString setting;
                QStringList filteredSettings;

                // Get group QMap
                QMap<QString, QVariant> tmpMap;
                QMap<QString, QVariant>* groupMap = \
                        configMap->value(ui->ConnType_Combo->currentText(), &tmpMap);

                // Check if data bits setting
                setting = "dataBits";
                filteredSettings = main_options_settings.custom.filter(setting);
                if (!filteredSettings.isEmpty())
                    dev_settings.dataBits = filteredSettings.at(0).split(':').at(1).toInt();
                else if (groupMap->contains(setting))
                    dev_settings.dataBits = groupMap->value(setting).toInt();
                // Check if direction setting
                setting = "direction";
                filteredSettings = main_options_settings.custom.filter(setting);
                if (!filteredSettings.isEmpty())
                    dev_settings.direction = filteredSettings.at(0).split(':').at(1);
                else if (groupMap->contains(setting))
                    dev_settings.direction = groupMap->value(setting).toString();

                // Check if flowControl setting
                setting = "flowControl";
                filteredSettings = main_options_settings.custom.filter(setting);
                if (!filteredSettings.isEmpty())
                    dev_settings.flowControl = filteredSettings.at(0).split(':').at(1);
                else if (groupMap->contains(setting))
                    dev_settings.flowControl = groupMap->value(setting).toString();

                // Check if parity setting
                setting = "parity";
                filteredSettings = main_options_settings.custom.filter(setting);
                if (!filteredSettings.isEmpty())
                    dev_settings.parity = filteredSettings.at(0).split(':').at(1);
                else if (groupMap->contains(setting))
                    dev_settings.parity = groupMap->value(setting).toString();

                // Check if stopBits setting
                setting = "stopBits";
                filteredSettings = main_options_settings.custom.filter(setting);
                if (!filteredSettings.isEmpty())
                    dev_settings.stopBits = filteredSettings.at(0).split(':').at(1).toFloat();
                else if (groupMap->contains(setting))
                    dev_settings.stopBits = groupMap->value(setting).toFloat();
            }

            // Create new object
            device = new Serial_RS232(&dev_settings, this);
            break;
        }
        case CONN_TYPE_TCP_CLIENT:
        {
            // Parse input
            QStringList conn = connInfo.split(':');
            if (conn.length() != 2) break;

            // Create new object
            device = new TCP_CLIENT(conn[0], conn[1].toInt(), this);
            break;
        }
        case CONN_TYPE_TCP_SERVER:
        {
            // Create new object
            device = new TCP_SERVER(connInfo.toInt(), this);
            break;
        }
        case CONN_TYPE_UDP_SOCKET:
        {
            // Parse input
            QStringList conn = connInfo.split(':');
            if (conn.length() != 3) break;

            // Create new object
            device = new UDP_SOCKET(conn[0], conn[1].toInt(), conn[2].toInt(), this);
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
    } else if (!device->initSuccessful())
    {
        GUI_HELPER::showMessage("Error: Initilization failed!");
        on_DeviceDisconnect_Button_clicked();
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

        // Block signals from tab group
        ui->ucOptions->blockSignals(true);

        // Setup tabs
        uint8_t gui_key;
        QMap<QString, QVariant>* groupMap;
        QWidget* tab_holder;
        foreach (QString childGroup, configMap->keys())
        {
            // Verify that its a known GUI
            gui_key = getGUIType(childGroup.split('_').last());
            if (gui_key == MAJOR_KEY_ERROR) continue;

            // Get group QMap
            groupMap = configMap->value(childGroup);

            // Instantiate and add GUI
            switch (gui_key)
            {
                case MAJOR_KEY_GENERAL_SETTINGS:
                {
                    GUI_BASE::parseGenericConfigMap(groupMap);

                    // Check reset tab setting (forces false from INI)
                    if (!groupMap->value("reset_tabs_on_switch", "true").toBool())
                    {
                        main_options_settings.reset_on_tab_switch = false;
                    }

                    // Check little endian setting (forces a true from INI)
                    if (groupMap->value("send_little_endian", "false").toBool())
                    {
                        main_options_settings.send_little_endian = true;
                    }

                    // Check chunk size setting (overrides if options setting is default)
                    if ((main_options_settings.chunk_size == GUI_BASE::default_chunk_size)
                            && groupMap->contains("chunk_size"))
                    {
                        main_options_settings.chunk_size = groupMap->value("chunk_size").toInt();
                    }
                    continue;
                }
                case MAJOR_KEY_WELCOME:
                {
                    tab_holder = new GUI_WELCOME(this);
                    break;
                }
                case MAJOR_KEY_IO:
                {
                    tab_holder = new GUI_8AIO_16DIO_COMM(this);
                    break;
                }
                case MAJOR_KEY_DATA_TRANSMIT:
                {
                    tab_holder = new GUI_DATA_TRANSMIT(this);
                    break;
                }
                case MAJOR_KEY_PROGRAMMER:
                {
                    tab_holder = new GUI_PROGRAMMER(this);
                    break;
                }
                case MAJOR_KEY_CUSTOM_CMD:
                {
                    tab_holder = new GUI_CUSTOM_CMD(this);
                    break;
                }
                default:
                {
                    continue;
                }
            }

            // Set delete on close attribute
            tab_holder->setAttribute(Qt::WA_DeleteOnClose);

            // Call config map parser
            ((GUI_BASE*) tab_holder)->parseConfigMap(groupMap);

            // Add new GUI to tabs
            ui->ucOptions->addTab(tab_holder, groupMap->value("tab_name", childGroup).toString());
        }

        // Force any changes in more options
        update_options(&main_options_settings);

        // Enable signals for tab group
        ui->ucOptions->blockSignals(false);

        // Delete map now that loading finished
        GUI_HELPER::deleteConfigMap(configMap);
        configMap = nullptr;

        // Freshen tabs for first use
        on_ucOptions_currentChanged(ui->ucOptions->currentIndex());

        // Commands generally fail the first time after new connection
        // Manually call reset remote (if shut off for tab switches)
        if (!main_options_settings.reset_on_tab_switch) reset_remote();
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

    // Remove device
    if (device)
    {
        device->close();
        device->deleteLater();
        device = nullptr;
    }

    // Remove widgets
    ucOptionsClear();

    // Add welcome widget
    ui->ucOptions->addTab(welcome_tab, welcome_tab_label);

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
        update_options(local_options_settings);
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

            // Reset the previous GUI
            if (main_options_settings.reset_on_tab_switch)
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
    if (main_options_settings.reset_on_tab_switch) reset_remote();
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

    on_Speed_Combo_activated(0);
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
    // Verifu device is connected
    if (!deviceConnected()) return;

    // Send reset command
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
                device, SLOT(write(QByteArray)));

        connect(device, SIGNAL(readyRead(QByteArray)),
                obj, SLOT(receive(QByteArray)));
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
    QWidget* tab_holder;
    for (int i = (ui->ucOptions->count() - 1); 0 <= i; i--)
    {
        tab_holder = ui->ucOptions->widget(i);
        tab_holder->blockSignals(true);
        ui->ucOptions->removeTab(i);
        if (tab_holder != welcome_tab) ((GUI_BASE*) tab_holder)->closing();
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
    // currentIndex() returns -1 on error which maps to CONN_TYPE_ERROR (0 == universal error)
    return (uint8_t) (ui->ConnType_Combo->currentIndex()+1);
}

uint8_t MainWindow::getDevType()
{
    // currentIndex() returns -1 on error which maps to DEV_TYPE_ERROR (0 == universal error)
    return (uint8_t) (ui->Device_Combo->currentIndex()+1);
}

uint8_t MainWindow::getGUIType(QString type)
{
    // indexOf() returns -1 on error which maps to MAJOR_KEY_ERROR (0 == universal error)
    return (uint8_t) ((supportedGUIsList.indexOf(type)+1) << s1_major_key_bit_shift);
}

QString MainWindow::getGUIName(uint8_t type)
{
    // Check that type is valid
    if (supportedGUIsList.length() < type) return "ERROR";

    // Return the string
    return supportedGUIsList.at((type >> s1_major_key_bit_shift)-1);
}

QStringList MainWindow::getConnSpeeds()
{
    // Returns the available speeds for the device type
    switch (getConnType())
    {
        case CONN_TYPE_RS_232: return Serial_RS232::Baudrate_Defaults;
        default: return {};
    }
}

void MainWindow::update_options(MoreOptions_struct* options)
{
    // Set chunk size
    GUI_BASE::set_chunk_size(options->chunk_size);

    // Set generic checksum
    QStringList checksum_info;
    QString gui_name = getGUIName(MAJOR_KEY_GENERAL_SETTINGS);
    if (options->checksum_map.contains(gui_name))
    {
        // Get checksum setting
        checksum_info = options->checksum_map.value(gui_name);

        // Set the new generic checksum
        GUI_BASE::set_generic_checksum(checksum_info);
    };

    // Set class values
    QStringList checksum_changes = options->checksum_map.keys();

    // Iterate over each tab and apply any changes
    GUI_BASE* tab_holder;
    uint8_t num_tabs = ui->ucOptions->count();
    for (uint8_t i = 0; i < num_tabs; i++)
    {
        // Get tab
        tab_holder = (GUI_BASE*) ui->ucOptions->widget(i);
        if (!tab_holder) continue;

        // Set gui checksum
        gui_name = getGUIName(tab_holder->get_GUI_key());
        if (checksum_changes.contains(gui_name))
        {
            // Get checksum setting
            checksum_info = options->checksum_map.value(gui_name);

            // Set the new gui checksum
            tab_holder->set_gui_checksum(checksum_info);
        }
    }
}
