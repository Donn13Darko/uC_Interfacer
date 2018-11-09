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
                                       "COM Port",
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
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);

    // Set base parameters
    prev_tab = -1;
    device = nullptr;
    configMap = nullptr;
    speed = "";
    updateConnInfo = new QTimer();

    // Setup Welcome widget
    welcome_tab = new GUI_WELCOME(this);
    welcome_tab->set_GUI_tab_name("Welcome");
    welcome_tab->setButtonsEnabled(false);
    welcome_tab->setClosable(false);
    welcome_tab->hide();

    // Setup add new tab (blank GUI_BASE)
    add_new_tab = new GUI_BASE(this);
    add_new_tab->set_GUI_tab_name("+");
    add_new_tab->setClosable(false);
    add_new_tab->hide();

    // Setup add new tab GUI dialog
    new_tab_gui = new GUI_CREATE_NEW_TABS(&configMap, this);
    new_tab_gui->setModal(true);
    new_tab_gui->hide();
    connect(new_tab_gui, SIGNAL(accepted()),
            this, SLOT(createNewTabs_accepted()),
            Qt::DirectConnection);

    // Setup More Options Dialog
    main_options_settings.reset_on_tab_switch = false;
    main_options_settings.send_little_endian = false;
    main_options_settings.chunk_size = GUI_COMM_BRIDGE::default_chunk_size;
    more_options = new GUI_MORE_OPTIONS(&main_options_settings, &local_options_settings,
                                        supportedGUIsList, GUI_COMM_BRIDGE::get_supported_checksums(),
                                        this);
    more_options->setModal(true);
    more_options->hide();
    connect(more_options, SIGNAL(accepted()),
            this, SLOT(moreOptions_accepted()),
            Qt::DirectConnection);

    // Setup Comm Bridge
    comm_bridge = new GUI_COMM_BRIDGE(supportedGUIsList.length(), this);

    // Add values to Device combo
    bool prev_block_status;
    prev_block_status = ui->Device_Combo->blockSignals(true);
    ui->Device_Combo->clear();
    ui->Device_Combo->addItems(MainWindow::supportedDevicesList);
    ui->Device_Combo->blockSignals(prev_block_status);

    // Add values to conn type combo
    prev_block_status = ui->ConnType_Combo->blockSignals(true);
    ui->ConnType_Combo->clear();
    ui->ConnType_Combo->addItems(MainWindow::supportedProtocolsList);
    ui->ConnType_Combo->blockSignals(prev_block_status);

    // Set Initial values
    setConnected(false);
    on_Device_Combo_activated(ui->Device_Combo->currentIndex());
    on_ConnType_Combo_currentIndexChanged(0);

    // Add welcome GUI for setup and to make the next steps possible
    ui->ucOptions->addTab(welcome_tab, welcome_tab->get_GUI_tab_name());

    // 1) Set closable to true to generate private button instance
    // 2) Grab genereated button and remove from widget (set to nullptr)
    // 3) Disconnect button from all slots and set closable to false
    // 4) Connect button instance to new slots
    ui->ucOptions->setTabsClosable(true);
    QTabBar *tab_bar = ui->ucOptions->tabBar();
    tab_closeButton = tab_bar->tabButton(0, QTabBar::RightSide);
    tab_bar->setTabButton(0, QTabBar::RightSide, nullptr);
    disconnect(tab_closeButton, 0, 0, 0);
    ui->ucOptions->setTabsClosable(false);
    connect(tab_closeButton, SIGNAL(clicked()),
            this, SLOT(on_tabCloseRequested()),
            Qt::DirectConnection);

    // Add update selections connections
    connect(updateConnInfo, SIGNAL(timeout()),
            this, SLOT(updateConnInfoCombo()),
            Qt::DirectConnection);
}

MainWindow::~MainWindow()
{
    // If connected, disconnect
    if (deviceConnected())
    {
        on_DeviceDisconnect_Button_clicked();
    }

    // Delete config map
    if (configMap) GUI_HELPER::deleteConfigMap(&configMap);

    // Stop connection timers
    updateConnInfo->stop();

    // Tell bridge to exit (once locks freed)
    comm_bridge->destroy_bridge();

    // Delete objects
    delete updateConnInfo;
    delete welcome_tab;
    delete add_new_tab;
    delete new_tab_gui;
    delete more_options;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    // If connected, disconnect
    if (deviceConnected())
    {
        on_DeviceDisconnect_Button_clicked();
    }
    updateConnInfo->stop();

    e->accept();
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
        case CONN_TYPE_SERIAL_COM_PORT:
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

    // Delete the config settings if not already done
    if (configMap) GUI_HELPER::deleteConfigMap(&configMap);

    // Read config settings
    configMap = GUI_HELPER::readConfigINI(deviceINI);
    if (!configMap) return;

    // Update selected info
    QString connInfo = ui->ConnInfo_Combo->currentText();

    // Try to connect to the device
    switch (getConnType())
    {
        case CONN_TYPE_SERIAL_COM_PORT:
        {
            // Create a new settings struct
            Serial_COM_Port_Settings dev_settings = Serial_COM_Port_Settings_DEFAULT;

            // Call parse function
            QMap<QString, QVariant> tmpMap;
            options_serial_com_port(&main_options_settings,
                                    configMap->value(ui->ConnType_Combo->currentText(),
                                                     &tmpMap),
                                    &dev_settings);

            // Create new object
            device = new SERIAL_COM_PORT(&dev_settings, this);
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
            // Create defaults
            QHostAddress addr = QHostAddress::Any;
            int port = 0;

            // Parse input
            QStringList conn = connInfo.split(':');
            uint8_t conn_len = conn.length();
            if (conn_len == 1)
            {
                port = connInfo.toInt();
            } else if (conn_len == 2)
            {
                addr = QHostAddress(conn.at(0));
                port = conn.at(1).toInt();
            }

            // Create new object
            device = new TCP_SERVER(addr, port, this);
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
    // Use queued connection for thread expansion
    connect(device, SIGNAL(deviceConnected()),
            this, SLOT(on_DeviceConnected()),
            Qt::QueuedConnection);
    connect(device, SIGNAL(deviceDisconnected()),
            this, SLOT(on_DeviceDisconnected()),
            Qt::QueuedConnection);

    // Try to connect
    device->open();
}

void MainWindow::on_DeviceConnected() {
    // Disconnect connection signals from device
    if (device)
    {
        // Remove device connected connections
        // Prevents accidental loops if emitted more than once
        disconnect(device, SIGNAL(deviceConnected()),
                   this, SLOT(on_DeviceConnected()));
    }

    // If connected, add relevant tabs
    // Else, show failed to connect message
    if (deviceConnected())
    {
        // Remove all existing tabs
        ucOptionsClear();

        // Try to open the bridge
        if (!comm_bridge->open_bridge())
        {
            on_DeviceDisconnect_Button_clicked();
            return;
        }

        // Connect bridge to device connections if bridge opened
        // Use queued connection for thread expansion
        connect(device, SIGNAL(readyRead(QByteArray)),
                comm_bridge, SLOT(receive(QByteArray)),
                Qt::QueuedConnection);
        connect(comm_bridge, SIGNAL(write_data(QByteArray)),
                device, SLOT(write(QByteArray)),
                Qt::QueuedConnection);

        // Block signals from tab group
        bool prev_block_status = ui->ucOptions->blockSignals(true);

        // Setup tabs
        uint8_t gui_key;
        GUI_BASE *tab_holder;
        QMap<QString, QVariant> *groupMap;
        foreach (QString childGroup, configMap->keys())
        {
            // Verify that its a known GUI
            gui_key = getGUIType(childGroup.split('_').last());
            if (gui_key == MAJOR_KEY_ERROR) continue;

            // Create new tab
            groupMap = configMap->value(childGroup);
            tab_holder = create_new_tab(gui_key, groupMap);

            // If tab creation failed, continue
            if (!tab_holder) continue;

            // Add new GUI to tabs
            ui->ucOptions->addTab(tab_holder, tab_holder->get_GUI_tab_name());
        }

        // Add dynamic addition tab group ('+' tab)
        ui->ucOptions->addTab(add_new_tab, add_new_tab->get_GUI_tab_name());

        // Force any changes in more options
        update_options(&main_options_settings);

        // Enable signals for tab group
        ui->ucOptions->blockSignals(prev_block_status);

        // Delete the config settings after use
        if (configMap) GUI_HELPER::deleteConfigMap(&configMap);

        // Freshen tabs for first use
        on_ucOptions_currentChanged(ui->ucOptions->currentIndex());

        // Commands generally fail the first time after new connection
        // Manually call reset remote (if shut off for tab switches)
        if (!main_options_settings.reset_on_tab_switch && deviceConnected())
            comm_bridge->reset_remote();
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
    // Reset the remote
    if (deviceConnected()) comm_bridge->reset_remote();

    // Disconnect any connected slots
    if (device)
    {
        // Remove device connected/disconnected connections
        disconnect(device, SIGNAL(deviceConnected()),
                   this, SLOT(on_DeviceConnected()));
        disconnect(device, SIGNAL(deviceDisconnected()),
                   this, SLOT(on_DeviceDisconnected()));

        // Remove device to bridge connections
        disconnect(device, SIGNAL(readyRead(QByteArray)),
                   comm_bridge, SLOT(receive(QByteArray)));
        disconnect(comm_bridge, SIGNAL(write_data(QByteArray)),
                   device, SLOT(write(QByteArray)));
    }

    // Remove device
    if (device)
    {
        device->close();
        device->deleteLater();
        device = nullptr;
    }

    // Remove widgets
    ucOptionsClear();

    // Close bridge
    comm_bridge->close_bridge();

    // Add welcome widget
    ui->ucOptions->addTab(welcome_tab, welcome_tab->get_GUI_tab_name());

    // Set to disconnected mode
    setConnected(false);

    // Refresh device & conn type combos
    on_ConnType_Combo_currentIndexChanged(0);
}

void MainWindow::on_MoreOptions_Button_clicked()
{
    // Reset More Options dialog
    more_options->reset_gui();

    // Show dialog (accept is linked to update function)
    // Don't want to block main application from running
    more_options->show();
}

void MainWindow::moreOptions_accepted()
{
    // Verify device is connected before trying
    // to set devices
    if (deviceConnected())
    {
        // Local options contains only changes
        update_options(local_options_settings);
    }
}

void MainWindow::createNewTabs_accepted()
{
    // Verify settings saved
    if (!configMap) return;

    // Block signals from tab group
    bool prev_block_status = ui->ucOptions->blockSignals(true);

    // Get insert positions
    int index;
    int tab_pos = new_tab_gui->get_tab_index();

    // Setup hold variables
    GUI_BASE *tab_holder;

    if (tab_pos == -1)
    {
        // Set tab_pos to index to add before '+' GUI
        index = ui->ucOptions->count() - 1;
        tab_pos = index;

        // Parse config maps and insert before '+'
        uint8_t gui_key;
        foreach (QString childGroup, configMap->keys())
        {
            // Verify that its a known GUI
            gui_key = getGUIType(childGroup.split('_').last());
            if (gui_key == MAJOR_KEY_ERROR) continue;

            // Create new tab
            tab_holder = create_new_tab(gui_key, configMap->value(childGroup));

            // If tab creation failed, continue
            if (!tab_holder) continue;

            // Add new GUI to tabs
            ui->ucOptions->insertTab(tab_pos, tab_holder, tab_holder->get_GUI_tab_name());
            tab_pos += 1;
        }

        // If index is 0, only add_tab was present at start so set
        // prev_tab to -1 to force a refresh with current index.
        // Other cases will force a refresh since pre
        if (index == 0) prev_tab = -1;
    } else
    {
        // Set index to tab_pos since we are updating an existing GUI
        index = tab_pos;

        // Get calling tab
        tab_holder = (GUI_BASE*) ui->ucOptions->widget(tab_pos);
        QTabBar *tab_bar = ui->ucOptions->tabBar();

        // Verify acquired and configMap settings
        if (tab_holder && tab_bar)
        {
            if (configMap->keys().length() == 1)
            {
                // Get gui group
                QString childGroup = configMap->firstKey();

                // Verify that its a known GUI & the GUI of interest
                uint8_t gui_key = getGUIType(childGroup.split('_').last());
                if (gui_key == tab_holder->get_GUI_key())
                {
                    // Reparse the config map (will overwrite existing one)
                    tab_holder->parseConfigMap(configMap->value(childGroup));

                    // Update the tab text (if reset)
                    ui->ucOptions->setTabText(index, tab_holder->get_GUI_tab_name());

                    // Update close button changes (start by removing then adding if needed)
                    tab_bar->setTabButton(prev_tab, QTabBar::RightSide, nullptr);
                    if (tab_holder->isClosable())
                    {
                        tab_bar->setTabButton(prev_tab, QTabBar::RightSide, tab_closeButton);
                    }
                } else
                {
                    GUI_HELPER::showMessage("Error: Incorrect GUI key detected!");
                }
            } else
            {
                GUI_HELPER::showMessage("Error: Multiple or no configurations detected!");
            }
        }
    }

    // Delete the config settings after use
    if (configMap) GUI_HELPER::deleteConfigMap(&configMap);

    // Force any changes in more options
    update_options(&main_options_settings);

    // Enable signals for tab group
    ui->ucOptions->blockSignals(prev_block_status);

    // Set tab to first new tab added
    // Calls currentChanged(int)
    ui->ucOptions->setCurrentIndex(index);
}

void MainWindow::on_ucOptions_currentChanged(int index)
{
    // Allocate holder
    GUI_BASE *tab_holder = (GUI_BASE*) ui->ucOptions->widget(index);

    // Check if '+' tab
    if (tab_holder && (tab_holder == add_new_tab))
    {
        // Block signals during work
        bool prev_block_status = ui->ucOptions->blockSignals(true);

        // Show previous tab while adding new tab
        // If no previous tab, set active widget as previous tab
        if (prev_tab != -1)
        {
            ui->ucOptions->setCurrentIndex(prev_tab);
        } else
        {
            // This will set tab 0 as prev_tab
            // Should only be called if '+' tab is last tab
            // after tabCloseRequested is called
            prev_tab = 0;
            ui->ucOptions->setCurrentIndex(prev_tab);
        }

        // Show new tab creation popup/wizard
        // Will emit accepted() or rejected() when done
        new_tab_gui->reset_gui();
        new_tab_gui->set_title("Create New Tab(s)");
        new_tab_gui->show();

        // Enable signals
        ui->ucOptions->blockSignals(prev_block_status);

        // Exit function
        return;
    }

    // No change (check here to allow multiple clicks on add_new_tab)
    if (prev_tab == index) return;

    // Get tab bar
    QTabBar *tab_bar = ui->ucOptions->tabBar();

    // Disconnet old signals
    if (prev_tab != -1)
    {
        // Remove tab exit button
        tab_bar->setTabButton(prev_tab, QTabBar::RightSide, nullptr);

        // Get old widget
        tab_holder = (GUI_BASE*) ui->ucOptions->widget(prev_tab);

        // Reset the previous GUI
        if (tab_holder && main_options_settings.reset_on_tab_switch)
            tab_holder->reset_gui();
    }

    // Update previous tab to index
    prev_tab = index;

    // Add close button if not welcome tab
    tab_holder = (GUI_BASE*) ui->ucOptions->widget(prev_tab);
    if (tab_holder && tab_holder->isClosable())
        tab_bar->setTabButton(prev_tab, QTabBar::RightSide, tab_closeButton);

    // Reset the Remote for the new tab (if connected & enabled on tab switch)
    if (main_options_settings.reset_on_tab_switch && deviceConnected())
        comm_bridge->reset_remote();
}

void MainWindow::on_ucOptions_tabBarClicked(int index)
{
    // Only allow multiple clicks for add_new_tab
    // Prevents add_tab widget being the last widget and
    // unable to add more (currentChanged() signal not emitted)
    if ((index == prev_tab)
            && (ui->ucOptions->currentWidget() == add_new_tab))
    {
        on_ucOptions_currentChanged(index);
    }
}

void MainWindow::on_ucOptions_tabBarDoubleClicked(int index)
{
    // Only allow double clicks on current tab
    if (prev_tab != index) return;

    // Get & verify tab of interest
    GUI_BASE *tab_holder = (GUI_BASE*) ui->ucOptions->widget(index);
    if (!tab_holder) return;

    // Check that its not add tab or welcome
    if ((tab_holder == welcome_tab)
            || (tab_holder == add_new_tab))
    {
        return;
    }

    // Load info into new_tab
    new_tab_gui->reset_gui();
    new_tab_gui->set_title("Update Tab Settings");
    new_tab_gui->set_config_tab(index, tab_holder->get_GUI_config());
    new_tab_gui->show();
}

void MainWindow::on_tabCloseRequested()
{
    // Get & check the tab holder
    // (close button only located on current index)
    int index = ui->ucOptions->currentIndex();
    GUI_BASE *tab_holder = (GUI_BASE*) ui->ucOptions->widget(index);
    if (!tab_holder) return;

    // Ignore if exit called on welcome tab or new tab
    if ((tab_holder == welcome_tab)
            && (tab_holder == add_new_tab))
    {
        return;
    }

    // Block signals for sending tab
    tab_holder->blockSignals(true);

    // Block signals for tab group
    bool prev_block_status = ui->ucOptions->blockSignals(true);

    // Remove tab close button
    ui->ucOptions->tabBar()->setTabButton(index, QTabBar::RightSide, nullptr);

    // If possible, set current tab to the tab to the left
    // of the tab getting removed
    if (0 < index) ui->ucOptions->setCurrentIndex(index - 1);

    // Remove from tabs
    if (index != -1) ui->ucOptions->removeTab(index);

    // Remove from comm_bridge
    comm_bridge->remove_gui(tab_holder);

    // Enable signals for tab group
    ui->ucOptions->blockSignals(prev_block_status);

    // Delete (will not be called on default welcome tab to blank + tab)
    delete tab_holder;

    // If closing current tab, set prev_tab to -1
    if (prev_tab == index) prev_tab = -1;

    // Update uc_options
    on_ucOptions_currentChanged(ui->ucOptions->currentIndex());
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
        case CONN_TYPE_SERIAL_COM_PORT:
        {
            if (!updateConnInfo->isActive()) {
                updateConnInfo->start(1000);
                ui->ConnInfo_Combo->setEditable(false);
            }

            QStringList *avail = SERIAL_COM_PORT::getDevices();
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

void MainWindow::ucOptionsClear()
{
    bool prev_block_status = ui->ucOptions->blockSignals(true);
    GUI_BASE *tab_holder;
    QTabBar *tab_bar = ui->ucOptions->tabBar();
    for (int i = (ui->ucOptions->count() - 1); 0 <= i; i--)
    {
        // Get tab at position
        tab_holder = (GUI_BASE*) ui->ucOptions->widget(i);
        if (!tab_holder) continue;

        // Disable signals to prevent sending close
        tab_holder->blockSignals(true);

        // Remove tab close button
        tab_bar->setTabButton(i, QTabBar::RightSide, nullptr);

        // Remove from gui & bridge
        ui->ucOptions->removeTab(i);
        comm_bridge->remove_gui(tab_holder);

        // If not default welcome tab to blank + tab, delete
        if ((tab_holder != welcome_tab)
                && (tab_holder != add_new_tab))
        {
            delete tab_holder;
        }
    }
    ui->ucOptions->blockSignals(prev_block_status);

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
    return (uint8_t) (supportedGUIsList.indexOf(type)+1);
}

QString MainWindow::getGUIName(uint8_t type)
{
    // Check that type is valid
    if (!type || (supportedGUIsList.length() < type)) return "GUI TYPE ERROR";

    // Return the string
    return supportedGUIsList.at(type-1);
}

QStringList MainWindow::getConnSpeeds()
{
    // Returns the available speeds for the device type
    switch (getConnType())
    {
        case CONN_TYPE_SERIAL_COM_PORT: return SERIAL_COM_PORT::Baudrate_Defaults;
        default: return {};
    }
}

GUI_BASE *MainWindow::create_new_tab(uint8_t gui_key, QMap<QString, QVariant> *guiConfigMap)
{
    // Verify gui config
    if (!guiConfigMap) return nullptr;

    // Instantiate and add GUI
    GUI_BASE *tab_holder = nullptr;
    switch (gui_key)
    {
        case MAJOR_KEY_GENERAL_SETTINGS:
        {
            comm_bridge->parseGenericConfigMap(guiConfigMap);

            // Check reset tab setting (forces true from INI)
            if (guiConfigMap->value("reset_tabs_on_switch", "false").toBool())
            {
                main_options_settings.reset_on_tab_switch = true;
            }

            // Check little endian setting (forces a true from INI)
            if (guiConfigMap->value("send_little_endian", "false").toBool())
            {
                main_options_settings.send_little_endian = true;
            }

            // Check chunk size setting (overrides if options setting is default)
            if ((main_options_settings.chunk_size == GUI_COMM_BRIDGE::default_chunk_size)
                    && guiConfigMap->contains("chunk_size"))
            {
                main_options_settings.chunk_size = guiConfigMap->value("chunk_size").toInt();
            }

            // If general settings, move to next (no GUI)
            return nullptr;
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
            break;
        }
    }
    if (!tab_holder) return nullptr;

    // Call config map parser
    tab_holder->parseConfigMap(guiConfigMap);

    // Add new GUI to comm bridge
    comm_bridge->add_gui(tab_holder);

    // Connect tab signals to bridge slots
    // Use queued connection for thread expansion
    connect(tab_holder, SIGNAL(transmit_file(quint8, quint8, QString, quint8, QString)),
            comm_bridge, SLOT(send_file(quint8, quint8, QString, quint8, QString)),
            Qt::QueuedConnection);
    connect(tab_holder, SIGNAL(transmit_file_pack(quint8, quint8, QString, quint8, QString)),
            comm_bridge, SLOT(send_file_pack(quint8, quint8, QString, quint8, QString)),
            Qt::QueuedConnection);
    connect(tab_holder, SIGNAL(transmit_chunk(quint8, quint8, QByteArray, quint8, QString)),
            comm_bridge, SLOT(send_chunk(quint8, quint8, QByteArray, quint8, QString)),
            Qt::QueuedConnection);
    connect(tab_holder, SIGNAL(transmit_chunk_pack(quint8, quint8, QByteArray, quint8, QString)),
            comm_bridge, SLOT(send_chunk_pack(quint8, quint8, QByteArray, quint8, QString)),
            Qt::QueuedConnection);

    // Connect bridge signals to tab slots
    // Use queued connection for thread expansion
    connect(comm_bridge, SIGNAL(reset()),
            tab_holder, SLOT(reset_gui()),
            Qt::QueuedConnection);

    // Return the new GUI
    return tab_holder;
}

void MainWindow::update_options(MoreOptions_struct *options)
{
    // Set chunk size
    comm_bridge->set_chunk_size(options->chunk_size);

    // Set checksums
    QStringList checksum_info;
    foreach (QString gui_name, options->checksum_map.keys())
    {
        // Get checksum setting
        checksum_info = options->checksum_map.value(gui_name);

        // Set the new checksum
        comm_bridge->set_tab_checksum(getGUIType(gui_name), checksum_info);
    }
}

void MainWindow::options_serial_com_port(MoreOptions_struct *options,
                                         QMap<QString, QVariant> *groupMap,
                                         Serial_COM_Port_Settings *settings)
{
    // Add basic info
    settings->port = ui->ConnInfo_Combo->currentText();
    settings->baudrate = speed.toInt();

    // Parse custom list & config map
    if (!groupMap->isEmpty() || !options->custom.isEmpty())
    {
        // Create holding list
        QString setting;
        QStringList filteredSettings;

        // Check if data bits setting
        setting = "dataBits";
        filteredSettings = options->custom.filter(setting);
        if (!filteredSettings.isEmpty())
            settings->dataBits = filteredSettings.at(0).split(':').at(1).toInt();
        else if (groupMap->contains(setting))
            settings->dataBits = groupMap->value(setting).toInt();

        // Check if flowControl setting
        setting = "flowControl";
        filteredSettings = options->custom.filter(setting);
        if (!filteredSettings.isEmpty())
            settings->flowControl = filteredSettings.at(0).split(':').at(1).toInt();
        else if (groupMap->contains(setting))
            settings->flowControl = groupMap->value(setting).toInt();

        // Check if parity setting
        setting = "parity";
        filteredSettings = options->custom.filter(setting);
        if (!filteredSettings.isEmpty())
            settings->parity = filteredSettings.at(0).split(':').at(1).toInt();
        else if (groupMap->contains(setting))
            settings->parity = groupMap->value(setting).toInt();

        // Check if stopBits setting
        setting = "stopBits";
        filteredSettings = options->custom.filter(setting);
        if (!filteredSettings.isEmpty())
            settings->stopBits = filteredSettings.at(0).split(':').at(1).toInt();
        else if (groupMap->contains(setting))
            settings->stopBits = groupMap->value(setting).toInt();
    }
}
