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

#include <QMessageBox>
#include <QFileDialog>

// Setup supported GUIs map
QMap<QString, uint8_t>
MainWindow::supportedGUIsMap({
                                 {"Welcome", GUI_TYPE_WELCOME},
                                 {"IO", GUI_TYPE_IO},
                                 {"Data Transmit", GUI_TYPE_DATA_TRANSMIT},
                                 {"Programmer", GUI_TYPE_PROGRAMMER},
                             });

// Setup static supported devices list
QStringList
MainWindow::supportedDevicesList({
                                     "Arduino Uno",
                                     "PC",
                                     "Other"
                                 });

// Setup static supported protocols list
QStringList
MainWindow::supportedProtocolsList({
                                       "RS-232",
                                       "TCP Client",
                                       "TCP Server",
                                       "UDP"
                                   });

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Setup base GUI items
    ui->setupUi(this);
    welcome_tab = new GUI_WELCOME(this);
    welcome_tab_text = "Welcome";

    // Set base parameters
    serial_rs232 = nullptr;
    prev_tab = -1;

    // Add specified values to combos
    ui->DeviceCombo->clear();
    ui->DeviceCombo->addItems(MainWindow::supportedDevicesList);
    ui->ConnTypeCombo->clear();
    ui->ConnTypeCombo->addItems(MainWindow::supportedProtocolsList);

    // Set Initial values
    setConnected(false);
    on_DeviceCombo_activated(ui->DeviceCombo->currentIndex());
    on_ConnTypeCombo_currentIndexChanged(ui->ConnTypeCombo->currentIndex());

    // Add connections
    connect(&updateConnInfo, SIGNAL(timeout()), this, SLOT(updateConnInfoCombo()));
}

MainWindow::~MainWindow()
{
    // If connected, disconnect
    if (ui->DeviceDisconnect_Button->isEnabled())
        on_DeviceDisconnect_Button_clicked();

    delete welcome_tab;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent* e)
{
    // If connected, disconnect
    if (ui->DeviceDisconnect_Button->isEnabled())
        on_DeviceDisconnect_Button_clicked();

    e->accept();
}

// Show message box
bool MainWindow::showMessage(QString msg)
{
    QMessageBox n;
    n.setText(msg);
    return n.exec();
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

void MainWindow::on_DeviceCombo_activated(int)
{
    // Save previous value
    uint8_t prev_deviceType = deviceType;
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
        welcome_tab->setMsg("Failed to load config for "
                            + ui->DeviceCombo->currentText()
                            + "!\nReverted to Previous Config: "
                            + prev_deviceINI);

        // Revert to previous dev type
        ui->DeviceCombo->setCurrentIndex(prev_deviceType-1);
        deviceINI = prev_deviceINI;
        return;
    }

    deviceType = getDevType();
    welcome_tab->setMsg("Current Config: " + deviceINI);
}

void MainWindow::on_ConnTypeCombo_currentIndexChanged(int)
{
    // Process changes to the speed & info combos
    updateSpeedCombo();
    updateConnInfoCombo();
}

void MainWindow::on_DeviceConnect_Button_clicked()
{
    // Update selected info
    QString connInfo = ui->ConnInfoCombo->currentText();
    QString speed = ui->SpeedCombo->currentText();

    // Try to connect to the device
    bool connected = false;
    switch (getConnType())
    {
        case CONN_TYPE_RS_232:
            serial_rs232 = new Serial_RS232(connInfo, speed);
            serial_rs232->open();
            connected = serial_rs232->isConnected();
            break;
        default:
            connected = false;
            break;
    }

    // If connected, add relevant tabs
    // Else, show failed to connect message
    if (connected)
    {
        // Reset & load the GUI settings file
        QSettings gui_settings(deviceINI, QSettings::IniFormat);

        // Setup tabs
        ui->ucOptions->blockSignals(true);
        uint8_t gui_type;
        QMap<QString, QVariant> configMap;
        QWidget* tab_holder;
        foreach (QString childGroup, gui_settings.childGroups())
        {
            // Verify that its a known GUI
            gui_type = getGUIType(childGroup.split('_').last());
            if (gui_type == GUI_TYPE_ERROR) continue;

            // Clear configMap
            configMap.clear();

            // Begin GUI group settings
            gui_settings.beginGroup(childGroup);
            foreach (QString childKey, gui_settings.childKeys())
            {
                configMap.insert(childKey, gui_settings.value(childKey));
            }

            // Exit GUI group settings
            gui_settings.endGroup();

            // Instantiate and add GUI
            switch (gui_type)
            {
                case GUI_TYPE_WELCOME:
                {
                    tab_holder = new GUI_WELCOME(this);
                    GUI_WELCOME* welcome_holder = \
                            (GUI_WELCOME*) tab_holder;

                    welcome_holder->setHeader(configMap.value("header", welcome_tab_text).toString());
                    welcome_holder->setMsg(configMap.value("msg").toString());
                    break;
                }
                case GUI_TYPE_IO:
                {
                    tab_holder = new GUI_8AIO_16DIO_COMM(this);
                    GUI_8AIO_16DIO_COMM* io_holder = \
                            (GUI_8AIO_16DIO_COMM*) tab_holder;

                    // Setup pintypes variable
                    uint8_t pinType;

                    // Add DIO controls
                    pinType = JSON_DIO;
                    io_holder->setNumPins(pinType, configMap.value("dio_num").toInt());
                    io_holder->setPinNumbers(pinType, configMap.value("dio_start_num").toInt());
                    io_holder->addNewPinSettings(pinType, configMap.value("dio_pin_settings").toStringList());
                    io_holder->setCombos(pinType, configMap.value("dio_combo_settings").toStringList());

                    // Add AIO controls
                    pinType = JSON_AIO;
                    io_holder->setNumPins(pinType, configMap.value("aio_num").toInt());
                    io_holder->setPinNumbers(pinType, configMap.value("aio_start_num").toInt());
                    io_holder->addNewPinSettings(pinType, configMap.value("aio_pin_settings").toStringList());
                    io_holder->setCombos(pinType, configMap.value("aio_combo_settings").toStringList());

                    // Add Transmit controls
                    pinType = REMOTE_CONN_REMOTE;
                    io_holder->addNewPinSettings(pinType, configMap.value("remote_pin_settings").toStringList());
                    io_holder->setCombos(pinType, configMap.value("remote_combo_settings").toStringList());

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

                    programmer_holder->addHexFormats(configMap.value("hex_formats").toStringList());
                    programmer_holder->removeHexFormats(configMap.value("hex_formats_rm").toStringList());
                    programmer_holder->addBurnMethods(configMap.value("burn_methods").toStringList());
                    break;
                }
                default:
                    continue;
            }

            // Set base chunk size to config value or 0 if non-existant
            ((GUI_BASE*) tab_holder)->set_chunkSize(configMap.value("chunk_size").toInt());

            // Add new GUI to tabs
            ui->ucOptions->addTab(tab_holder, configMap.value("tab_name", childGroup).toString());
        }
        ui->ucOptions->blockSignals(false);

        // Freshen tabs for first use
        on_ucOptions_currentChanged(ui->ucOptions->currentIndex());

        // Set to connected mode
        connect2sender(this, true);

        // Reset the remote
        // (the window must be in connected mode prior to call)
        reset_remote();
    } else
    {
        showMessage("Error: Unable to connect to target!");
    }

    setConnected(connected);
}

void MainWindow::on_DeviceDisconnect_Button_clicked()
{
    // Reset the remote
    reset_remote();

    // Disconnect connections
    connect2sender(ui->ucOptions->currentWidget(), false);
    connect2sender(this, false);

    // Disconnect from connection
    switch (getConnType())
    {
        case CONN_TYPE_RS_232:
            if (!serial_rs232) break;
            else if (serial_rs232->isConnected()) serial_rs232->close();
            delete serial_rs232;
            serial_rs232 = nullptr;
            break;
        default:
            break;
    }

    // Remove widgets
    QWidget* tmp;
    ui->ucOptions->blockSignals(true);
    for (int tabIndex = (ui->ucOptions->count() - 1); 0 <= tabIndex; tabIndex--)
    {
        tmp = ui->ucOptions->widget(tabIndex);
        ui->ucOptions->removeTab(tabIndex);
        delete tmp;
    }
    ui->ucOptions->blockSignals(false);
    prev_tab = -1;

    // Set to disconnected mode
    setConnected(false);
}

void MainWindow::on_MoreOptions_Button_clicked()
{
    QMessageBox moreOptions(this);

    moreOptions.exec();
    return;
}

void MainWindow::on_ucOptions_currentChanged(int index)
{
    // No change
    if (prev_tab == index) return;

    // Disconnet old signals
    if (prev_tab != -1)
    {
        QObject* prev_widget = (QObject*) ui->ucOptions->widget(prev_tab);

        // Disconnect slots/signals
        connect2sender(prev_widget, false);
        disconnect(prev_widget, SIGNAL(connect_signals(bool)),
                   this, SLOT(connect_signals(bool)));

        // Reset the previous GUI (qobject_cast returns null if cast not possible)
        if (qobject_cast<GUI_8AIO_16DIO_COMM*>(prev_widget)) ((GUI_8AIO_16DIO_COMM*) prev_widget)->reset_gui();
        else if (qobject_cast<GUI_DATA_TRANSMIT*>(prev_widget)) ((GUI_DATA_TRANSMIT*) prev_widget)->reset_gui();
        else if (qobject_cast<GUI_PROGRAMMER*>(prev_widget)) ((GUI_PROGRAMMER*) prev_widget)->reset_gui();
    }

    // Reset the remote
    reset_remote();

    // Connect new signals
    QObject* curr_widget = (QObject*) ui->ucOptions->currentWidget();
    connect(curr_widget, SIGNAL(connect_signals(bool)),
            this, SLOT(connect_signals(bool)));
    connect_signals(true);

    // Update previous tab index
    prev_tab = index;
}

void MainWindow::updateConnInfoCombo()
{
    switch (getConnType())
    {
        case CONN_TYPE_RS_232:
        {
            if (!updateConnInfo.isActive()) {
                updateConnInfo.start(1000);
            }
            ui->ConnInfoCombo->setEditable(false);

            QStringList avail = Serial_RS232::getDevices();
            QString curr = ui->ConnInfoCombo->currentText();
            ui->ConnInfoCombo->clear();
            ui->ConnInfoCombo->addItems(avail);
            ui->ConnInfoCombo->setCurrentText(curr);
            break;
        }
        default:
        {
            updateConnInfo.stop();
            ui->ConnInfoCombo->setEditable(true);
            break;
        }
    }
}

void MainWindow::updateSpeedCombo()
{
    ui->SpeedCombo->clear();
    ui->SpeedCombo->setEnabled(true);

    QStringList newItems = getConnSpeeds();
    if (newItems.length() == 0) ui->SpeedCombo->setEnabled(false);
    else ui->SpeedCombo->addItems(newItems);
}

void MainWindow::setConnected(bool conn)
{
    bool op_conn = !conn;

    // Set Combos Enabled
    ui->DeviceCombo->setEnabled(op_conn);
    ui->ConnTypeCombo->setEnabled(op_conn);
    ui->SpeedCombo->setEnabled(op_conn);
    ui->ConnInfoCombo->setEnabled(op_conn);

    // Set Buttons Enabled
    ui->DeviceConnect_Button->setEnabled(op_conn);
    ui->DeviceDisconnect_Button->setEnabled(conn);

    // Add or remove welcome tab
    ui->ucOptions->blockSignals(true);
    if (conn && (0 < ui->ucOptions->count()) &&
            (ui->ucOptions->tabText(0) == welcome_tab_text))
    {
        ui->ucOptions->removeTab(0);
    } else if (op_conn && (ui->ucOptions->count() == 0))
    {
        ui->ucOptions->addTab(welcome_tab, welcome_tab_text);
    }
    ui->ucOptions->blockSignals(false);
}

void MainWindow::reset_remote()
{
    emit write_data({JSON_RESET, JSON_RESET});
    emit write_data({JSON_RESET, JSON_RESET});
}

void MainWindow::connect2sender(QObject* obj, bool conn)
{
    // Get connection type & verify connection
    QObject* conn_obj = getConnObject();
    if (!obj || !conn_obj) return;

    // Connect or disconnect signals
    if (conn) {
        connect(obj, SIGNAL(write_data(QByteArray)),
                conn_obj, SLOT(write(QByteArray)));
        connect(obj, SIGNAL(write_data(std::initializer_list<uint8_t>)),
                conn_obj, SLOT(write(std::initializer_list<uint8_t>)));

        connect(conn_obj, SIGNAL(readyRead(QByteArray)),
                obj, SLOT(receive(QByteArray)));
    } else {
        disconnect(obj, SIGNAL(write_data(QByteArray)),
                   conn_obj, SLOT(write(QByteArray)));
        disconnect(obj, SIGNAL(write_data(std::initializer_list<uint8_t>)),
                   conn_obj, SLOT(write(std::initializer_list<uint8_t>)));

        disconnect(conn_obj, SIGNAL(readyRead(QByteArray)),
                   obj, SLOT(receive(QByteArray)));
    }
}

uint8_t MainWindow::getConnType()
{
    // currentIndex() returns -1 on error which maps to TYPE_ERROR
    return (uint8_t) (ui->ConnTypeCombo->currentIndex()+1);
}

uint8_t MainWindow::getDevType()
{
    // currentIndex() returns -1 on error which maps to TYPE_ERROR
    return (uint8_t) (ui->DeviceCombo->currentIndex()+1);
}

uint8_t MainWindow::getGUIType(QString type)
{
    return supportedGUIsMap.value(type, GUI_TYPE_ERROR);
}


QStringList MainWindow::getConnSpeeds()
{
    switch (getConnType())
    {
        case CONN_TYPE_RS_232: return Serial_RS232::Baudrate_Defaults;
        default: return {};
    }
}

QObject* MainWindow::getConnObject(int type)
{
    // Find type if not provided
    if (type == -1) type = getConnType();

    // Select connection object
    switch (type)
    {
        case CONN_TYPE_RS_232: return serial_rs232;
        default: return nullptr;
    }
}
