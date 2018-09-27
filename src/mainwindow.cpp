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

#include "uc-interfaces/gui-arduino-uno-io.h"
#include "user-interfaces/gui-data-transmit.h"
#include "user-interfaces/gui-programmer.h"

#include <QMessageBox>
#include <QFileDialog>

// Setup static supported devices list
QStringList
MainWindow::supportedDevicesList({
                                     "Arduino Uno",
                                     "PC",
                                     "Other"
                                 });

// Setup static supported devices map
QMap<QString, uint8_t>
MainWindow::supportedDevicesMap({
                                 {"Arduino Uno", DEV_TYPE_ARDUINO_UNO},
                                 {"PC", DEV_TYPE_PC},
                                 {"Other", DEV_TYPE_OTHER}
                             });

// Setup static supported protocols list
QStringList
MainWindow::supportedProtocolsList({
                                       "RS-232",
                                       "TCP",
                                       "UDP"
                                   });

// Setup static supported protocols map
QMap<QString, uint8_t>
MainWindow::supportedProtocolsMap({
                                   {"RS-232", CONN_TYPE_RS_232},
                                   {"TCP", CONN_TYPE_TCP},
                                   {"UDP", CONN_TYPE_UDP}
                               });

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Setup base GUI items
    ui->setupUi(this);

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
    on_DeviceCombo_currentIndexChanged(ui->DeviceCombo->currentIndex());
    on_ConnTypeCombo_currentIndexChanged(ui->ConnTypeCombo->currentIndex());
    on_SpeedCombo_currentIndexChanged(ui->SpeedCombo->currentIndex());

    // Add connections
    connect(&updateConnInfo, SIGNAL(timeout()), this, SLOT(updateConnInfoCombo()));
}

MainWindow::~MainWindow()
{
    // If connected, disconnect
    if (ui->DeviceDisconnect_Button->isEnabled())
        on_DeviceDisconnect_Button_clicked();

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

void MainWindow::on_DeviceCombo_currentIndexChanged(int)
{
    // Save previous value
    QString prev_deviceType = deviceType;

    // Update Changed Data
    deviceType = ui->DeviceCombo->currentText();

    // Get new INI file
    deviceINI = ":/uc-interfaces/";
    switch (getDevType())
    {
        case DEV_TYPE_ARDUINO_UNO:
            deviceINI += "gui-arduino-uno.ini";
            break;
        case DEV_TYPE_OTHER:
            deviceINI = QFileDialog::getOpenFileName(this, tr("Open"), "",
                                                     tr("INI (*.ini);;All Files (*.*)"));;
            break;
    }

    if (deviceINI.isEmpty())
    {
        ui->DeviceCombo->setCurrentText(prev_deviceType);
    }
}

void MainWindow::on_ConnTypeCombo_currentIndexChanged(int)
{
    // Update Changed Data
    connType = ui->ConnTypeCombo->currentText();

    // Process changes to the speed & info combos
    updateSpeedCombo();
    updateConnInfoCombo();
}

void MainWindow::on_SpeedCombo_currentIndexChanged(int)
{
    // Update Changed Data
    speed = ui->SpeedCombo->currentText();
}

void MainWindow::on_DeviceConnect_Button_clicked()
{
    // Get connection info
    connInfo = ui->ConnInfoCombo->currentText();

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

    // Error out of can't connect to hardware
    if (connected)
    {
        // Reset & load the GUI settings file
        QSettings gui_settings(deviceINI, QSettings::IniFormat);
        QStringList key_groups = gui_settings.childGroups();
//        qDebug() << deviceINI;
//        qDebug() << gui_settings.fileName();
//        qDebug() << gui_settings.childGroups();
//        qDebug() << gui_settings.value("Programmer/instructions");

        QMap<QString, QVariant> temp;
        foreach (QString i, key_groups)
        {
            gui_settings.beginGroup(i);
        }

        // Setup tabs
        ui->ucOptions->blockSignals(true);
        QWidget* tab_holder;
        switch (getDevType())
        {
            case DEV_TYPE_ARDUINO_UNO:
            {
                tab_holder = new ArduinoUno_IO(this);
                ui->ucOptions->addTab(tab_holder, "I/O");
                tab_holder = new GUI_DATA_TRANSMIT(arduino_chunk_size, this);
                ui->ucOptions->addTab(tab_holder, "Data Transfer");
                tab_holder = new GUI_PROGRAMMER(deviceType, arduino_chunk_size, this);
                ui->ucOptions->addTab(tab_holder, "Programmer");
                break;
            }
            default:
                break;
        }
        ui->ucOptions->blockSignals(false);

        // Freshen tabs for first use
        on_ucOptions_currentChanged(ui->ucOptions->currentIndex());

        // Set to connected mode
        connect2sender(this, true);

        // Reset the remote
        // (this window must be in connected mode prior to call)
        reset_remote();
    } else
    {
        showMessage("Error: Unable to connect to target!");
        on_DeviceDisconnect_Button_clicked();
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
        if (qobject_cast<ArduinoUno_IO*>(prev_widget)) ((ArduinoUno_IO*) prev_widget)->reset_gui();
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

void MainWindow::updateSpeedCombo()
{
    ui->SpeedCombo->clear();
    ui->SpeedCombo->setEnabled(true);

    QStringList newItems = getConnSpeeds();
    if (newItems.length() == 0) ui->SpeedCombo->setEnabled(false);
    else ui->SpeedCombo->addItems(newItems);

    on_SpeedCombo_currentIndexChanged(ui->SpeedCombo->currentIndex());
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
    return supportedProtocolsMap.value(connType, CONN_TYPE_ERROR);
}

uint8_t MainWindow::getDevType()
{
    return supportedDevicesMap.value(deviceType, DEV_TYPE_ERROR);
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
