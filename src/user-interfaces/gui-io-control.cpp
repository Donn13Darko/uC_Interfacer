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

#include "gui-io-control.hpp"
#include "ui_gui-io-control.h"

#include <QDateTime>

GUI_IO_CONTROL::GUI_IO_CONTROL(QWidget *parent) :
    GUI_BASE(parent),
    ui(new Ui::GUI_IO_CONTROL)
{
    // Setup ui
    ui->setupUi(this);

    // Set GUI Type & Default Name
    set_gui_key(MAJOR_KEY_IO);
    set_gui_name("IO");

    // Set buttons
    ui->StartUpdater_Button->setText("Start");
    ui->StartLog_Button->setText("Start Log");

    // Set class pin variables
    bytesPerPin = 2;

    // Setup AIO info
    AIO_Grid = new QGridLayout();
    ui->AIOVLayout->insertLayout(1, AIO_Grid);
    num_AIOcols = 1;

    // Setup DIO info
    DIO_Grid = new QGridLayout();
    ui->DIOVLayout->insertLayout(1, DIO_Grid);
    num_DIOcols = 2;

    // Set log file parameters
    logFile = NULL;
    logStream = NULL;
    logIsRecording = false;

    // Connect updaters
    // All internal object connections so direct is okay
    connect(&AIO_READ, SIGNAL(timeout()),
            this, SLOT(updateValues()),
            Qt::DirectConnection);
    connect(&DIO_READ, SIGNAL(timeout()),
            this, SLOT(updateValues()),
            Qt::DirectConnection);
    connect(&logTimer, SIGNAL(timeout()),
            this, SLOT(recordLogData()),
            Qt::DirectConnection);

    // Reset GUI
    reset_gui();
}

GUI_IO_CONTROL::~GUI_IO_CONTROL()
{
    // Stop loggers & updaters
    on_StopLog_Button_clicked();
    on_StopUpdater_Button_clicked();

    // Delete all the maps
    clear_all_maps();

    // Delete local variables
    delete AIO_Grid;
    delete DIO_Grid;
    delete ui;
}

void GUI_IO_CONTROL::parseConfigMap(QMap<QString, QVariant> *configMap)
{
    // Pass to parent for parsing
    GUI_BASE::parseConfigMap(configMap);

    // Setup pintypes variable
    PinTypeInfo pInfo;
    pinList.clear();

    // Add DIO controls
    if (!getPinTypeInfo(MINOR_KEY_IO_DIO, &pInfo)) return;
    addPinType(pInfo.pinType);
    addComboSettings(&pInfo, configMap->value("dio_combo_settings").toStringList());
    setPinCombos(&pInfo, configMap->value("dio_pin_settings").toStringList());
    destroy_unused_pins(&pInfo);
    update_pin_grid(&pInfo);

    // Add AIO controls
    if (!getPinTypeInfo(MINOR_KEY_IO_AIO, &pInfo)) return;
    addPinType(pInfo.pinType);
    addComboSettings(&pInfo, configMap->value("aio_combo_settings").toStringList());
    setPinCombos(&pInfo, configMap->value("aio_pin_settings").toStringList());
    destroy_unused_pins(&pInfo);
    update_pin_grid(&pInfo);

    // Add Remote controls
    if (!getPinTypeInfo(MINOR_KEY_IO_REMOTE_CONN, &pInfo)) return;
    addPinType(pInfo.pinType);
    addComboSettings(&pInfo, configMap->value("remote_combo_settings").toStringList());

    // Update remote comm combo
    bool prev_block_status = ui->ConnType_Combo->blockSignals(true);
    ui->ConnType_Combo->clear();
    ui->ConnType_Combo->addItems(controlMap.value(pInfo.pinType)->keys());
    ui->ConnType_Combo->blockSignals(prev_block_status);

    // Sort and emit pinList update
    pinList.sort();
    emit pin_update(pinList);

    // Reset after parse (forces updates)
    reset_gui();
}

bool GUI_IO_CONTROL::waitForDevice(uint8_t minorKey)
{
    switch (minorKey)
    {
        case MINOR_KEY_IO_REMOTE_CONN_SET:
        case MINOR_KEY_IO_REMOTE_CONN_READ:
        case MINOR_KEY_IO_REMOTE_CONN_SEND:
            return true;
        case MINOR_KEY_IO_AIO_READ:
            // Check if read pin
        case MINOR_KEY_IO_AIO_READ_ALL:
            return aio_read_requested;
        case MINOR_KEY_IO_DIO_READ:
            // Check if read pin
        case MINOR_KEY_IO_DIO_READ_ALL:
            return dio_read_requested;
        default:
            return GUI_BASE::waitForDevice(minorKey);
    }
}

void GUI_IO_CONTROL::reset_gui()
{
    // Reset base first
    GUI_BASE::reset_gui();

    // Reset request variables
    dio_read_requested = false;
    aio_read_requested = false;
    dio_read_requested_double = false;
    aio_read_requested_double = false;
    dio_read_pins.clear();
    aio_read_pins.clear();
    dio_read_pins_double.clear();
    aio_read_pins_double.clear();

    // Stop logging and updating if running
    on_StopLog_Button_clicked();
    on_StopUpdater_Button_clicked();

    // Set to disconnected state
    ui->ConnConnect_Button->setText("Connect");
    ui->ConnSend_Button->setEnabled(false);
    devConnected = false;

    // Setup loop variables
    QComboBox *comboBox;
    bool prev_block_status;

    // Reset pin settings
    foreach (uint8_t pinType, pinMap.keys())
    {
        // Set combo to start
        foreach (QHBoxLayout* pin, *pinMap.value(pinType))
        {
            // Make sure pin valid
            if (!pin) continue;

            // Get combo box to reset
            comboBox = (QComboBox*) pin->itemAt(io_combo_pos)->widget();

            // Block signals on combo box
            prev_block_status = comboBox->blockSignals(true);

            // Reset combo box
            comboBox->setCurrentIndex(0);
            inputsChanged(pinType, comboBox, io_combo_pos);

            // Reset signal status on combo box
            comboBox->blockSignals(prev_block_status);
        }
    }
}

void GUI_IO_CONTROL::chart_update_request(QList<QString> data_points, GUI_CHART_ELEMENT *target_element)
{
    // Update and emit back
    QList<QVariant> data;

    // Setup variables
    uint8_t pinType = 0, pinNum = 0;
    QStringList pinNum_split;
    QHBoxLayout *item;
    bool pin_error;
    QVariant val;

    // Get each element
    foreach (QString pin, data_points)
    {
        // Set to no pin error
        pin_error = false;

        // Get pin str
        pinNum_split = pin.split('_');
        if (pinNum_split.length() != 2) pin_error = true;

        // If no error, get pin info
        if (!pin_error)
        {
            // Get pin type
            if (pinNum_split.at(0) == "AIO") pinType = MINOR_KEY_IO_AIO;
            else if (pinNum_split.at(0) == "DIO") pinType = MINOR_KEY_IO_DIO;
            else pin_error = true;

            // Get pin number
            bool ok = false;
            pinNum = pinNum_split.at(1).toInt(&ok);
            if (!ok) pin_error = true;
        }

        // Get pin value
        if (!pin_error && get_pin_layout(pinType, pinNum, &item))
        {
            val = QVariant(((QLineEdit*) item->itemAt(io_line_edit_pos)->widget())->text().toDouble());
        } else
        {
            val = QVariant(-1.0);
        }

        // Add to data list
        data.append(val);
    }

    // Emit back to target graph
    emit target_element->update_receive(data);
}

void GUI_IO_CONTROL::recordPinValues(PinTypeInfo *pInfo)
{
    if (logStream == nullptr) return;

    *logStream << pInfo->pinType;

    QLineEdit *lineEditValue;
    foreach (QHBoxLayout* pin, *pinMap.value(pInfo->pinType))
    {
        // Make sure pin valid
        if (!pin) continue;

        // Add comma
        *logStream << ",";

        // Get lineEdit value
        lineEditValue = (QLineEdit*) pin->itemAt(io_line_edit_pos)->widget();

        // Append pin value
        *logStream << lineEditValue->text();
    }
    // Add new line
    *logStream << "\n";

    // Flush data to file
    logStream->flush();

    // Emit updated
    emit log_updated();
}

void GUI_IO_CONTROL::receive_gui(QByteArray recvData)
{
    // Get gui key
    uint8_t local_gui_key = get_gui_key();

    // Verify length & ignore commands not meant for this GUI
    if ((recvData.length() < s1_end_loc)
            || (recvData.at(s1_major_key_loc) != (char) local_gui_key))
    {
        return;
    }

    uint8_t minor_key = recvData.at(s1_minor_key_loc);
    switch (minor_key)
    {
        case MINOR_KEY_IO_AIO_READ:
        case MINOR_KEY_IO_DIO_READ:
        {
            // Get pin num
            uint8_t pinNum = recvData.at(s1_end_loc);

            // Check if request or respone
            if (recvData.length() == (num_s1_bytes+1))
            {
                // Packet is a request so get pin values
                uint32_t pinValue;
                QByteArray pinValues;
                QSlider *sliderValue;

                // Get pin info
                PinTypeInfo pInfo;
                getPinTypeInfo(minor_key, &pInfo);

                // Make sure pin valid
                QHBoxLayout *pin;
                if (!get_pin_layout(pInfo.pinType, pinNum, &pin))
                {
                    // Exit out of parse
                    break;
                }

                // Append pin num to return array
                pinValues.append(pinNum);

                // Get slider value
                sliderValue = (QSlider*) pin->itemAt(io_slider_pos)->widget();

                // Get value
                pinValue = sliderValue->value();

                // Parse value to uint16_t and append to pin values
                pinValues.append(GUI_GENERIC_HELPER::uint32_to_byteArray(pinValue).right(2));

                // Send values back
                emit transmit_chunk(local_gui_key, minor_key, pinValues);

                // Send device ready
                emit transmit_chunk(MAJOR_KEY_DEV_READY, 0);

                // Exit out of parse
                break;
            }

            // Otherwise packet is a response so...
            // Check if need to send another data request
            // Otherwise, clear read request
            if (minor_key == MINOR_KEY_IO_AIO_READ)
            {
                // Verify GUI requested
                if (!aio_read_pins.contains(pinNum)) break;

                // Check if double request
                if (aio_read_pins_double.contains(pinNum))
                {
                    // Clear double
                    aio_read_pins_double.removeAll(pinNum);

                    // Emit another request for data
                    emit transmit_chunk(local_gui_key, minor_key,
                                        GUI_GENERIC_HELPER::qList_to_byteArray({pinNum}));
                } else
                {
                    // Clear single request
                    aio_read_pins.removeAll(pinNum);
                }
            } else if (minor_key == MINOR_KEY_IO_DIO_READ)
            {
                // Verify GUI requested
                if (!dio_read_pins.contains(pinNum)) break;

                // Check if double request
                if (dio_read_pins_double.contains(pinNum))
                {
                    // Clear double
                    dio_read_pins_double.removeAll(pinNum);

                    // Emit another request for data
                    emit transmit_chunk(local_gui_key, minor_key,
                                        GUI_GENERIC_HELPER::qList_to_byteArray({pinNum}));
                } else
                {
                    // Clear single request
                    dio_read_pins.removeAll(pinNum);
                }
            }

            // Set values with minor key
            setValues(minor_key, recvData.mid(s1_end_loc));
            break;

            // Exit out of parse
            break;
        }
        case MINOR_KEY_IO_AIO_READ_ALL:
        case MINOR_KEY_IO_DIO_READ_ALL:
        {
            // Check if request or respone
            if (recvData.length() == num_s1_bytes)
            {
                // Packet is a request so get pin values
                uint32_t pinValue;
                QByteArray pinValues;
                QSlider *sliderValue;

                // Get pin info
                PinTypeInfo pInfo;
                getPinTypeInfo(minor_key, &pInfo);

                // Parse all pin values
                foreach (QHBoxLayout* pin, *pinMap.value(pInfo.pinType))
                {
                    // Make sure pin valid
                    if (!pin) continue;

                    // Get slider value
                    sliderValue = (QSlider*) pin->itemAt(io_slider_pos)->widget();

                    // Get value
                    pinValue = sliderValue->value();

                    // Parse value to uint16_t and append to pin values
                    pinValues.append(GUI_GENERIC_HELPER::uint32_to_byteArray(pinValue).right(2));
                }

                // Send values back
                emit transmit_chunk(local_gui_key, minor_key, pinValues);

                // Send device ready
                emit transmit_chunk(MAJOR_KEY_DEV_READY, 0);

                // Exit out of parse
                break;
            }

            // Otherwise packet is a response so...
            // Check if need to send another data request
            // Otherwise, clear read request
            if (minor_key == MINOR_KEY_IO_AIO_READ_ALL)
            {
                // Verify GUI requested
                if (!aio_read_requested) break;;

                // Check if double request
                if (aio_read_requested_double)
                {
                    // Clear double
                    aio_read_requested_double = false;

                    // Emit another request for data
                    emit transmit_chunk(local_gui_key, minor_key);
                } else
                {
                    // Clear single request
                    aio_read_requested = false;
                }
            } else if (minor_key == MINOR_KEY_IO_DIO_READ_ALL)
            {
                // Verify GUI requested
                if (!dio_read_requested) break;

                // Check if double request
                if (dio_read_requested_double)
                {
                    // Clear double
                    dio_read_requested_double = false;

                    // Emit another request for data
                    emit transmit_chunk(local_gui_key, minor_key);
                } else
                {
                    // Clear single request
                    dio_read_requested = false;
                }
            }

            // Set values with minor key
            setValues(minor_key, recvData.mid(s1_end_loc));
            break;
        }
        case MINOR_KEY_IO_AIO_SET:
        case MINOR_KEY_IO_DIO_SET:
        case MINOR_KEY_IO_AIO_WRITE:
        case MINOR_KEY_IO_DIO_WRITE:
        {
            // Set values with minor key
            setValues(minor_key, recvData.mid(s1_end_loc));
            break;
        }
        case MINOR_KEY_IO_REMOTE_CONN_CONNECTED:
        {
            // Check length
            if (recvData.length() != 3) break;

            // Check connected
            bool conn_status = recvData.at(2);

            // Parse what to do
            if (devConnected && !conn_status)
            {
                // Set connected bools
                devConnected = false;
                if (!devConnectRequested)
                {
                    GUI_GENERIC_HELPER::showMessage("Error: IO remote conn lost!");
                } else
                {
                    devConnectRequested = false;
                }

                // Set conneccted button
                ui->ConnConnect_Button->setText("Connect");
            } else if (devConnectRequested && !devConnected)
            {
                if (conn_status)
                {
                    // Set connected bools
                    devConnected = true;
                    devConnectRequested = false;

                    // Set connected button
                    ui->ConnConnect_Button->setText("Disconnect");
                } else
                {
                    GUI_GENERIC_HELPER::showMessage("Error: IO remote conn failed!");
                }
            }
            break;
        }
        case MINOR_KEY_IO_REMOTE_CONN_SET:
        {
            // Check length
            if (recvData.length() != 4) break;

            // Get combo values
            uint8_t conn_type = recvData.at(0);
            uint8_t conn_dev = recvData.at(1);
            uint8_t conn_speed = recvData.at(2);
            uint8_t conn_addr = recvData.at(3);

            // Check combo values
            if ((ui->ConnType_Combo->count() <= conn_type)
                    || (ui->ConnDeviceNum_Combo->count() <= conn_dev)
                    || (ui->ConnSpeed_Combo->count() <= conn_speed)
                    || (ui->ConnAddr_Combo->count() <= conn_addr))
            {
                break;
            }

            // Set combos
            ui->ConnType_Combo->setCurrentIndex(conn_type);
            ui->ConnDeviceNum_Combo->setCurrentIndex(conn_dev);
            ui->ConnSpeed_Combo->setCurrentIndex(conn_speed);
            ui->ConnAddr_Combo->setCurrentIndex(conn_addr);

            break;
        }
        case MINOR_KEY_IO_REMOTE_CONN_READ:
        case MINOR_KEY_IO_REMOTE_CONN_SEND:
        {
            ui->ConnRecv_PlainText->appendPlainText(recvData.mid(s1_end_loc));
            rcvd_formatted_append(recvData);
            break;
        }
    }
}

void GUI_IO_CONTROL::request_read_all(uint8_t pinType)
{
    // Get pin info
    PinTypeInfo pInfo;
    if (!getPinTypeInfo(pinType, &pInfo)) return;

    // Request read (or set double)
    uint8_t requestType;
    switch (pInfo.pinType)
    {
        case MINOR_KEY_IO_AIO:
        {
            // Check if already waiting for a response
            if (aio_read_requested)
            {
                aio_read_requested_double = true;
                return;
            }

            // Set requested and key
            aio_read_requested = true;
            requestType = MINOR_KEY_IO_AIO_READ_ALL;

            // Break out of selection
            break;
        }
        case MINOR_KEY_IO_DIO:
        {
            // Check if already waiting for a response
            if (dio_read_requested)
            {
                dio_read_requested_double = true;
                return;
            }

            // Set requested and key
            dio_read_requested = true;
            requestType = MINOR_KEY_IO_DIO_READ_ALL;

            // Break out of selection
            break;
        }
        default:
        {
            return;
        }
    }

    // Emit request for data
    emit transmit_chunk(get_gui_key(), requestType);
}

void GUI_IO_CONTROL::request_read_pin(uint8_t pinType, uint8_t pinNum)
{
    // Get pin info
    PinTypeInfo pInfo;
    if (!getPinTypeInfo(pinType, &pInfo)) return;

    // Request read (or set double)
    uint8_t requestType;
    switch (pInfo.pinType)
    {
        case MINOR_KEY_IO_AIO:
        {
            // Check if already waiting for a response
            if (aio_read_pins.contains(pinNum))
            {
                if (!aio_read_pins_double.contains(pinNum))
                {
                    aio_read_pins_double.append(pinNum);
                }
                return;
            }

            // Set requested and key
            aio_read_pins.append(pinNum);
            requestType = MINOR_KEY_IO_AIO_READ;

            // Break out of selection
            break;
        }
        case MINOR_KEY_IO_DIO:
        {
            // Check if already waiting for a response
            if (dio_read_pins.contains(pinNum))
            {
                if (!dio_read_pins_double.contains(pinNum))
                {
                    dio_read_pins_double.append(pinNum);
                }
                return;
            }

            // Set requested and key
            dio_read_pins.append(pinNum);
            requestType = MINOR_KEY_IO_DIO_READ;

            // Break out of selection
            break;
        }
        default:
        {
            return;
        }
    }

    // Emit request for data
    emit transmit_chunk(get_gui_key(), requestType,
                        GUI_GENERIC_HELPER::qList_to_byteArray({pinNum}));
}

QStringList GUI_IO_CONTROL::get_pin_list()
{
    return pinList;
}

bool GUI_IO_CONTROL::get_pin_layout(uint8_t pinType, uint8_t pin_num, QHBoxLayout **itemLayout)
{
    // Set itemLayout to nullptr
    *itemLayout = nullptr;

    // Get pins
    QList<QHBoxLayout*> *pins = pinMap.value(pinType);

    // Verify pins
    if (!pins) return false;

    // Search pins for item
    QHBoxLayout *pin;
    int curr_pin_num;
    int num_pins = pins->length();
    for (int i = 0; i < num_pins; i++)
    {
        pin = pins->at(i);
        curr_pin_num = ((QLabel*) pin->itemAt(io_label_pos)->widget())->text().toInt(nullptr, 10);
        if (curr_pin_num == pin_num)
        {
            *itemLayout = pin;
            return true;
        }
    }

    // If reached, couldn't find pin in pins
    return false;
}

bool GUI_IO_CONTROL::get_widget_layout(uint8_t pinType, QWidget *item, QHBoxLayout **itemLayout)
{
    // Set itemLayout to nullptr
    *itemLayout = nullptr;

    // Get pins
    QList<QHBoxLayout*> *pins = pinMap.value(pinType);

    // Verify item & pins
    if (!(item && pins)) return false;

    // Search pins for item
    QHBoxLayout *pin;
    int num_pins = pins->length();
    for (int i = 0; i < num_pins; i++)
    {
        pin = pins->at(i);
        if (pin->indexOf(item) != -1)
        {
            *itemLayout = pin;
            return true;
        }
    }

    // If reached, couldn't find pin in pins
    return false;
}

Ui::GUI_IO_CONTROL *GUI_IO_CONTROL::get_ui()
{
    return ui;
}

void GUI_IO_CONTROL::DIO_ComboValueChanged()
{
    // Propogate updates
    QByteArray data;
    inputsChanged(MINOR_KEY_IO_DIO, sender(), io_combo_pos, &data);

    // Verify data packet
    if (data.length() < 4) return;

    // Send update
    emit transmit_chunk(get_gui_key(), MINOR_KEY_IO_DIO_SET, data);
}

void GUI_IO_CONTROL::DIO_SliderValueChanged()
{
    // Set message for clicked button
    QByteArray data;
    inputsChanged(MINOR_KEY_IO_DIO, sender(), io_slider_pos, &data);

    // Verify data packet
    if (data.length() < 4) return;

    // Remove combo setting (unneeded)
    data.remove(s2_io_combo_loc, 1);

    // Send update
    emit transmit_chunk(get_gui_key(), MINOR_KEY_IO_DIO_WRITE, data);
}

void GUI_IO_CONTROL::DIO_LineEditValueChanged()
{
    // Set message for clicked button
    QByteArray data;
    inputsChanged(MINOR_KEY_IO_DIO, sender(), io_line_edit_pos, &data);

    // Verify data packet
    if (data.length() < 4) return;

    // Remove combo setting (unneeded)
    data.remove(s2_io_combo_loc, 1);

    // Send update
    emit transmit_chunk(get_gui_key(), MINOR_KEY_IO_DIO_WRITE, data);
}

void GUI_IO_CONTROL::AIO_ComboValueChanged()
{
    // Send message for edited button
    QByteArray data;
    inputsChanged(MINOR_KEY_IO_AIO, sender(), io_combo_pos, &data);

    // Verify data packet
    if (data.length() < 4) return;

    // Send update
    emit transmit_chunk(get_gui_key(), MINOR_KEY_IO_AIO_SET, data);
}

void GUI_IO_CONTROL::AIO_SliderValueChanged()
{
    // Send message for edited button
    QByteArray data;
    inputsChanged(MINOR_KEY_IO_AIO, sender(), io_slider_pos, &data);

    // Verify data packet
    if (data.length() < 4) return;

    // Remove combo setting (unneeded)
    data.remove(s2_io_combo_loc, 1);

    // Send update
    emit transmit_chunk(get_gui_key(), MINOR_KEY_IO_AIO_WRITE, data);
}

void GUI_IO_CONTROL::AIO_LineEditValueChanged()
{
    // Send message for edited button
    QByteArray data;
    inputsChanged(MINOR_KEY_IO_AIO, sender(), io_line_edit_pos, &data);

    // Verify data packet
    if (data.length() < 4) return;

    // Remove combo setting (unneeded)
    data.remove(s2_io_combo_loc, 1);

    // Send update
    emit transmit_chunk(get_gui_key(), MINOR_KEY_IO_AIO_WRITE, data);
}

void GUI_IO_CONTROL::updateValues()
{
    // Get caller to find request type
    QTimer *caller = (QTimer*) sender();

    // Call request_read_all with correct pin type
    if (caller == &AIO_READ) request_read_all(MINOR_KEY_IO_AIO);
    else if (caller == &DIO_READ) request_read_all(MINOR_KEY_IO_DIO);
}

void GUI_IO_CONTROL::recordLogData()
{
    if (!logIsRecording) return;

    PinTypeInfo pInfo;
    if (getPinTypeInfo(MINOR_KEY_IO_AIO_READ_ALL, &pInfo)) recordPinValues(&pInfo);
    if (getPinTypeInfo(MINOR_KEY_IO_DIO_READ_ALL, &pInfo)) recordPinValues(&pInfo);

}

void GUI_IO_CONTROL::on_StartUpdater_Button_clicked()
{
    ui->StartUpdater_Button->setText("Reset");

    DIO_READ.start((int) (GUI_GENERIC_HELPER::S2MS * ui->DIO_UR_LineEdit->text().toFloat()));
    AIO_READ.start((int) (GUI_GENERIC_HELPER::S2MS * ui->AIO_UR_LineEdit->text().toFloat()));
}

void GUI_IO_CONTROL::on_StopUpdater_Button_clicked()
{
    // Stop timers
    DIO_READ.stop();
    AIO_READ.stop();

    // Set button text
    ui->StartUpdater_Button->setText("Start");

    // Clear double requests
    dio_read_requested_double = false;
    aio_read_requested_double = false;
    dio_read_pins_double.clear();
    aio_read_pins_double.clear();
}

void GUI_IO_CONTROL::on_LogSaveLocSelect_Button_clicked()
{
    // Get file
    QString filePath;
    if (GUI_GENERIC_HELPER::getSaveFilePath(&filePath))
        ui->LogSaveLoc_LineEdit->setText(filePath);
}

void GUI_IO_CONTROL::on_StartLog_Button_clicked()
{
    bool error = false;
    if (logIsRecording)
        error = GUI_GENERIC_HELPER::showMessage("Error: Already recording!");
    else if (ui->LogSaveLoc_LineEdit->text().isEmpty())
        error = GUI_GENERIC_HELPER::showMessage("Error: Must provide log file!");
    if (error) return;

    uint32_t enumFlags = QIODevice::WriteOnly;
    if (ui->AppendLog_CheckBox->isChecked()) enumFlags |= QIODevice::Append;
    else enumFlags |= QIODevice::Truncate;

    if (logFile) delete logFile;
    logFile = new QFile(ui->LogSaveLoc_LineEdit->text());
    if (!logFile->open((QIODevice::OpenModeFlag) enumFlags))
        error = GUI_GENERIC_HELPER::showMessage("Error: Couldn't open log file!");
    if (error) return;

    if (logStream) delete logStream;
    logStream = new QTextStream(logFile);
    *logStream << "Started: " << QDateTime::currentDateTimeUtc().toString() << " ";
    *logStream << "with update rate " << ui->LOG_UR_LineEdit->text() << " seconds\n";
    logStream->flush();

    logTimer.start((int) (GUI_GENERIC_HELPER::S2MS * ui->LOG_UR_LineEdit->text().toFloat()));
    ui->StartLog_Button->setText("Running");
    ui->StartLog_Button->setEnabled(false);
    ui->AppendLog_CheckBox->setEnabled(false);
    ui->LOG_UR_LineEdit->setEnabled(false);
    logIsRecording = true;
}

void GUI_IO_CONTROL::on_StopLog_Button_clicked()
{
    if (!logIsRecording) return;
    logTimer.stop();

    logStream->flush();
    logFile->close();
    delete logStream;
    delete logFile;
    logStream = NULL;
    logFile = NULL;

    ui->StartLog_Button->setText("Start Log");
    ui->StartLog_Button->setEnabled(true);
    ui->AppendLog_CheckBox->setEnabled(true);
    ui->LOG_UR_LineEdit->setEnabled(true);
    logIsRecording = false;
}

void GUI_IO_CONTROL::on_ConnConnect_Button_clicked()
{
    // Holder for connection info (if connecting)
    QByteArray msg;
    devConnectRequested = true;

    // Set connection status
    if (devConnected)
    {
        // Add connection values
        msg.append((char) 0x00);

        // Transmit disconnect
        emit transmit_chunk(get_gui_key(), MINOR_KEY_IO_REMOTE_CONN_CONNECTED, msg);
    } else
    {
        // Add connection values
        msg.append((char) ui->ConnType_Combo->currentIndex());
        msg.append((char) ui->ConnDeviceNum_Combo->currentIndex());
        msg.append((char) ui->ConnSpeed_Combo->currentIndex());
        msg.append((char) ui->ConnAddr_Combo->currentIndex());

        // Transmit connection request
        emit transmit_chunk(get_gui_key(), MINOR_KEY_IO_REMOTE_CONN_SET, msg);
    }

    // Enable or disable send
    ui->ConnSend_Button->setEnabled(devConnected);
}

void GUI_IO_CONTROL::on_ConnSend_Button_clicked()
{
    // Check connected
    if (!devConnected) return;

    // Send input data
    emit transmit_chunk(get_gui_key(), MINOR_KEY_IO_REMOTE_CONN_SEND,
                        ui->ConnMsg_PlainText->toPlainText().toLatin1());
}

void GUI_IO_CONTROL::on_ConnClearRecv_Button_clicked()
{
    ui->ConnRecv_PlainText->clear();
    rcvd_formatted_clear();
}

void GUI_IO_CONTROL::on_ConnSaveRecv_Button_clicked()
{
    rcvd_formatted_save();
}

void GUI_IO_CONTROL::on_ConnType_Combo_currentIndexChanged(int)
{
    // Get current combo & info
    QString currVal = ui->ConnType_Combo->currentText();
    uint8_t type = controlMap.value(MINOR_KEY_IO_REMOTE_CONN)->value(currVal);

    // Enable/disable speed combo
    ui->ConnSpeed_Combo->setEnabled(
                !disabledValueSet.value(MINOR_KEY_IO_REMOTE_CONN)->contains(type));

    // Set connect devices values (or leave editable)
    QStringList deviceConns = devSettings.value(currVal);
    ui->ConnAddr_Combo->clear();
    ui->ConnAddr_Combo->addItems(deviceConns);
    ui->ConnAddr_Combo->setEditable(deviceConns.isEmpty());
}

void GUI_IO_CONTROL::on_CreatePlots_Button_clicked()
{
    // Setup graph
    GUI_CHART_VIEW *chart_view = new GUI_CHART_VIEW();
    chart_view->set_data_list(pinList);
    chart_view->setAttribute(Qt::WA_DeleteOnClose);
    chart_view->setModal(false);

    // Connect destroy signal to close signal
    connect(this, SIGNAL(destroyed()),
            chart_view, SLOT(close()),
            Qt::QueuedConnection);

    // Connect chart view update request
    connect(chart_view, SIGNAL(update_request(QList<QString>,GUI_CHART_ELEMENT*)),
            this, SLOT(chart_update_request(QList<QString>,GUI_CHART_ELEMENT*)),
            Qt::QueuedConnection);

    // Connect pin update signals to plot update
    connect(this, SIGNAL(pin_update(QStringList)),
            chart_view, SLOT(set_data_list(QStringList)),
            Qt::QueuedConnection);

    // Show the chart view
    chart_view->show();
}

void GUI_IO_CONTROL::inputsChanged(uint8_t pinType, QObject *caller, uint8_t io_pos, QByteArray *data)
{
    // Grab pin info
    PinTypeInfo pInfo;
    if (!getPinTypeInfo(pinType, &pInfo)) return;

    // Get mapping info
    QMap<QString, uint8_t> *pinControlMap = controlMap.value(pInfo.pinType);
    QMap<uint8_t, RangeList*> *pinRangeMap = rangeMap.value(pInfo.pinType);
    QList<uint8_t> *pinDisabledSet = disabledValueSet.value(pInfo.pinType);
    if (!(pinControlMap && pinRangeMap && pinDisabledSet)) return;

    // Get pin info of button clicked
    QHBoxLayout *pin;
    if (!get_widget_layout(pInfo.pinType, (QWidget*) caller, &pin)) return;

    // Get widgets
    QLabel *label = (QLabel*) pin->itemAt(io_label_pos)->widget();
    QComboBox *comboBox = (QComboBox*) pin->itemAt(io_combo_pos)->widget();
    QSlider *sliderValue = (QSlider*) pin->itemAt(io_slider_pos)->widget();
    QLineEdit *lineEditValue = (QLineEdit*) pin->itemAt(io_line_edit_pos)->widget();
    if (!(label && comboBox && sliderValue && lineEditValue)) return;

    // Set Pin Num
    QString pinNum = QString::number(label->text().toInt());

    // Clarify IO selection
    uint8_t io_combo = pinControlMap->value(comboBox->currentText());

    // Get range list for use in next sections
    RangeList *rList = pinRangeMap->value(io_combo);

    // Set IO if combo changed
    float newVAL;
    bool prev_block_status;
    switch (io_pos)
    {
        case io_combo_pos:
        {
            // Enable/Disable pins if selection changed
            bool disableClicks = pinDisabledSet->contains(io_combo);
            sliderValue->setAttribute(Qt::WA_TransparentForMouseEvents, disableClicks);
            lineEditValue->setAttribute(Qt::WA_TransparentForMouseEvents, disableClicks);

            // Update slider range (will reset slider value)
            updateSliderRange(sliderValue, rList);

            // Fall through to next case to update info
        }
        case io_slider_pos:
        {
            // Update values based on slider
            newVAL = ((float) sliderValue->value()) / rList->div;
            if (pInfo.pinType == MINOR_KEY_IO_DIO) newVAL = qRound(newVAL);

            // Update lineEdit value
            prev_block_status = lineEditValue->blockSignals(true);
            lineEditValue->setText(QString::number(newVAL));
            lineEditValue->blockSignals(prev_block_status);
            break;
        }
        case io_line_edit_pos:
        {
            // Update values if text box changed
            newVAL = rList->div * lineEditValue->text().toFloat();
            if (pInfo.pinType == MINOR_KEY_IO_DIO) newVAL = qRound(newVAL);

            // Update slider value
            prev_block_status = sliderValue->blockSignals(true);
            sliderValue->setValue(newVAL);
            sliderValue->blockSignals(prev_block_status);
            break;
        }
        default:
        {
            // No change or error
            return;
        }
    }

    if (data != nullptr)
    {
        // Scale & verify value
        if (io_pos != io_line_edit_pos) newVAL = (newVAL - (float) rList->min) * rList->div;
        else newVAL = newVAL - ((float) rList->min * rList->div);
        if (newVAL < 0)
        {
            GUI_GENERIC_HELPER::showMessage("Error: Invalid Pin Value:  " \
                                            + QString::number(newVAL));
            return;
        }
        uint16_t v = (uint16_t) (qRound(newVAL) & 0xFFFF);

        // Build pin data array
        data->append((char) pinNum.toInt());    // Pin Num
        data->append((char) ((v >> 8) & 0xFF)); // Value High
        data->append((char) (v & 0xFF));        // Value Low
        data->append((char) io_combo);          // Combo setting
    }
}

void GUI_IO_CONTROL::updateSliderRange(QSlider *slider, RangeList *rList)
{
    // Disable signals
    bool prev_block_status = slider->blockSignals(true);

    // Change settings
    slider->setMinimum(rList->min);
    slider->setMaximum(rList->max);
    slider->setTickInterval(rList->step);
    slider->setSingleStep(rList->step);
    slider->setPageStep(rList->step);

    // Set to 0, or min/max if 0 out of range
    if (0 < rList->min) slider->setValue(rList->min);
    else if (rList->max < 0) slider->setValue(rList->max);
    else slider->setValue(0);

    // Enable signals
    slider->blockSignals(prev_block_status);
}

void GUI_IO_CONTROL::setConTypes(QStringList connTypes, QList<char> mapValues)
{
    if (connTypes.length() != mapValues.length()) return;

    QMap<QString, uint8_t> *pinControlMap = controlMap.value(MINOR_KEY_IO_REMOTE_CONN);
    for (uint8_t i = 0; i < connTypes.length(); i++)
    {
        pinControlMap->insert(connTypes.at(i), mapValues.at(i));
    }

    ui->ConnType_Combo->clear();
    ui->ConnType_Combo->addItems(connTypes);

    on_ConnType_Combo_currentIndexChanged(ui->ConnType_Combo->currentIndex());
}

void GUI_IO_CONTROL::setPinCombos(PinTypeInfo *pInfo, QList<QString> combos)
{
    // Retrieve & verify pin type maps
    QList<QHBoxLayout*> *pins = pinMap.value(pInfo->pinType);
    QMap<QString, uint8_t> *pinControlMap = controlMap.value(pInfo->pinType);
    QMap<uint8_t, RangeList*> *pinRangeMap = rangeMap.value(pInfo->pinType);
    QList<uint8_t> *pinDisabledSet = disabledValueSet.value(pInfo->pinType);
    if (!(pins && pinControlMap && pinRangeMap && pinDisabledSet)) return;

    // Set pinType_str prepend
    QString pinType_str, pin_str;
    if (pInfo->pinType == MINOR_KEY_IO_AIO) pinType_str = "AIO_";
    else if (pInfo->pinType == MINOR_KEY_IO_DIO) pinType_str = "DIO_";
    else pinType_str = "_";

    // Setup arrays & constructs for use in the loop
    uint8_t IO;
    QList<uint8_t> pinNums;
    QList<QString> listValues;
    QStringList comboStr_split;
    bool prev_block_status, disableClicks;

    // Setup widget holders
    QLabel *itemLabel;
    QComboBox *itemCombo;
    QSlider *itemSlider;
    QLineEdit *itemLineEdit;
    QHBoxLayout *pin_layout;
    QHBoxLayout *new_pin;

    // Iterate though each combo setting
    foreach (QString comboStr, combos)
    {
        // Reset pinNums array
        pinNums.clear();

        // Parse input nums
        comboStr_split = comboStr.split('=');
        if (comboStr_split.length() != 2) continue;

        // Generate pin list to apply values to
        foreach (QString pinNum, comboStr_split.at(0).split(','))
        {
            // See if defines a range
            if (pinNum.contains(':'))
            {
                // Split range list
                QStringList numStr_split = pinNum.split(':');
                if (numStr_split.length() != 2) continue;

                // Get start and end of range
                uint8_t start_pin = numStr_split.at(0).toInt();
                uint8_t end_pin = numStr_split.at(1).toInt();

                // Add each element to the list
                for (uint8_t curr_pin = start_pin; curr_pin <= end_pin; curr_pin++)
                {
                    pinNums.append(curr_pin);
                }
            } else
            {
                // Convert value to int an verify
                uint8_t p = pinNum.toInt();
                if ((p == 0) && (pinNum.at(0) != '0')) continue;

                // Add the pin to the list
                pinNums.append(p);
            }
        }

        // Get combo values
        listValues = comboStr_split.at(1).split(',');

        // Set combo for each pin in the list
        foreach (uint8_t pin, pinNums)
        {
            // Find row & column of desired pin
            // If not found, create a new pin and insert it into
            // the map & list at the correct location (ordered by number)
            if (!get_pin_layout(pInfo->pinType, pin, &pin_layout))
            {
                // Create the new pin
                new_pin = create_pin();
                if (!new_pin) continue;

                // Set the pin label
                ((QLabel*) new_pin->itemAt(io_label_pos)->widget())->setText(QString("%1").arg(pin, 2, 10, QChar('0')));

                // Connect pin to slots
                connect_pin(pInfo->pinType, new_pin);

                // Add the pin to the list
                addPinLayout(new_pin, pins);

                // Set pin_layout to the new pin
                pin_layout = new_pin;
            }

            // Get all the pin items
            itemLabel = (QLabel*) pin_layout->itemAt(io_label_pos)->widget();
            itemCombo = (QComboBox*) pin_layout->itemAt(io_combo_pos)->widget();
            itemSlider = (QSlider*) pin_layout->itemAt(io_slider_pos)->widget();
            itemLineEdit = (QLineEdit*) pin_layout->itemAt(io_line_edit_pos)->widget();

            // Block signals to combo
            prev_block_status = itemCombo->blockSignals(true);

            // Update combo & get map information
            itemCombo->clear();
            itemCombo->addItems(listValues);
            IO = pinControlMap->value(itemCombo->currentText());

            // Enable or disable pin group
            disableClicks = pinDisabledSet->contains(IO);
            itemSlider->setAttribute(Qt::WA_TransparentForMouseEvents, disableClicks);
            itemLineEdit->setAttribute(Qt::WA_TransparentForMouseEvents, disableClicks);

            // Update slider range
            RangeList *rList = pinRangeMap->value(IO);
            updateSliderRange(itemSlider, rList);

            // Enble signals to combo
            itemCombo->blockSignals(prev_block_status);

            // Add pin to pinList
            pin_str = pinType_str + itemLabel->text();
            if (!pinList.contains(pin_str)) pinList.append(pin_str);
        }
    }
}

void GUI_IO_CONTROL::addPinType(uint8_t pinType)
{
    // Add new pinType to each map
    if (!pinMap.contains(pinType)) pinMap.insert(pinType, new QList<QHBoxLayout*>());
    if (!controlMap.contains(pinType)) controlMap.insert(pinType, new QMap<QString, uint8_t>());
    if (!disabledValueSet.contains(pinType)) disabledValueSet.insert(pinType, new QList<uint8_t>());
    if (!rangeMap.contains(pinType)) rangeMap.insert(pinType, new QMap<uint8_t, RangeList*>());
}

void GUI_IO_CONTROL::addPinLayout(QHBoxLayout *pin, QList<QHBoxLayout *> *pins)
{
    // Get insert pin number
    int insert_pin_num = ((QLabel*) pin->itemAt(io_label_pos)->widget())->text().toInt();

    // Find insertion position (assume pins inserted in order until now)
    int curr_pin_num;
    for (int i = (pins->length()-1); 0 <= i; i--)
    {
        // Get current pin number
        curr_pin_num = ((QLabel*) pins->at(i)->itemAt(io_label_pos)->widget())->text().toInt();

        // Check if less than inserted pin
        if (curr_pin_num < insert_pin_num)
        {
            // Insert at position after
            pins->insert(i+1, pin);
            return;
        }
    }

    // If reached here, pin goes at front of list
    pins->prepend(pin);
}

void GUI_IO_CONTROL::addComboSettings(PinTypeInfo *pInfo, QList<QString> newSettings)
{
    // Setup lists to hold setting
    QList<QString> settingValues;
    QList<QString> pinCombos, pinSetDisabled;
    QList<RangeList*> pinRanges;

    // Get each setting
    foreach (QString setting_str, newSettings)
    {
        // Split settings string into values [name,setEnabled,rangeList]
        settingValues = setting_str.split(',');

        // Add to combos
        pinCombos.append(settingValues.at(0));

        // Add to disabled set
        if (settingValues.at(1).toLower() == "true")
            pinSetDisabled.append(settingValues.at(0));

        // Add to ranges
        pinRanges.append(makeRangeList(settingValues.at(2)));
    }

    // Add range and pin controls
    uint8_t pinType = pInfo->pinType;
    addPinControls(pinType, pinCombos);
    addPinRangeMap(pinType, pinCombos, pinRanges);

    // Get maps
    QMap<QString, uint8_t> *pinControlMap = controlMap.value(pinType);
    QList<uint8_t> *pinDisabledSet = disabledValueSet.value(pinType);

    // Clear disabled set
    pinDisabledSet->clear();

    // Set pin disabled controls
    foreach (QString i, pinSetDisabled)
    {
        pinDisabledSet->append(pinControlMap->value(i));
    }
}

void GUI_IO_CONTROL::addPinControls(uint8_t pinType, QList<QString> keys)
{
    // Get map
    QMap<QString, uint8_t> *pinControlMap = controlMap.value(pinType);
    int key_num = pinControlMap->size();

    // Add each string to pinMap
    foreach (QString key, keys)
    {
        // Verify its a new value
        if (!pinControlMap->contains(key))
        {
            pinControlMap->insert(key, key_num);
            key_num += 1;
        }
    }
}

void GUI_IO_CONTROL::addPinRangeMap(uint8_t pinType, QList<QString> keys, QList<RangeList*> values)
{
    // Verify lengths
    if (keys.length() != values.length()) return;

    // Get maps
    QMap<QString, uint8_t> *pinControlMap = controlMap.value(pinType);
    QMap<uint8_t, RangeList*> *pinRangeMap = rangeMap.value(pinType);

    // Add to each string
    uint8_t key_num;
    for (uint8_t i = 0; i < keys.length(); i++)
    {
        // Get key num
        key_num = pinControlMap->value(keys.at(i));

        // Check if already exists in map
        if (pinRangeMap->contains(key_num))
        {
            // Delete old if already exists
            delete pinRangeMap->value(key_num);
        }

        // Insert new value
        pinRangeMap->insert(key_num, values.at(i));
    }
}

void GUI_IO_CONTROL::setValues(uint8_t minorKey, QByteArray values)
{
    // Get pin information
    PinTypeInfo pInfo;
    if (!getPinTypeInfo(minorKey, &pInfo)) return;

    // Get & verify maps
    QMap<QString, uint8_t> *pinControlMap = controlMap.value(pInfo.pinType);
    QMap<uint8_t, RangeList*> *pinRangeMap = rangeMap.value(pInfo.pinType);
    QList<uint8_t> *pinDisabledSet = disabledValueSet.value(pInfo.pinType);
    if (!(pinControlMap && pinRangeMap && pinDisabledSet)) return;

    // Allocate loop variables
    QHBoxLayout *pin = nullptr;
    QComboBox *comboBox = nullptr;
    RangeList *rList = nullptr;
    uint8_t combo_value;
    uint16_t value;
    int newVAL;

    // Set loop values
    uint8_t pin_num = 0;
    uint8_t val_len = values.length();

    switch (minorKey)
    {
        // If read_all data
        case MINOR_KEY_IO_AIO_READ_ALL:
        case MINOR_KEY_IO_DIO_READ_ALL:
        {
            // Setup variables
            uint8_t i = 0, j = 0;
            QList<QHBoxLayout*> *pins = pinMap.value(pInfo.pinType);

            // Verify values
            if (!pins || ((2*pins->length()) != val_len)) break;

            // Loop over all pins and set their value
            foreach (pin, *pins)
            {
                // Get combo
                comboBox = (QComboBox*) pin->itemAt(io_combo_pos)->widget();
                combo_value = pinControlMap->value(comboBox->currentText());

                // Only update value if not controllable
                if (pinDisabledSet->contains(combo_value))
                {
                    // Get value from list (value is big endian)
                    value = 0;
                    for (j = 0; j < bytesPerPin; j++)
                    {
                        value = (value << 8) | ((uchar) values.at(i+j));
                    }

                    // Scale value
                    rList = pinRangeMap->value(combo_value);
                    newVAL = qRound((float) value + (rList->min * rList->div));

                    // Set slider
                    set_pin_io(pin, io_slider_pos, newVAL);

                    // Set lineEdit
                    set_pin_io(pin, io_line_edit_pos, QString::number(((float) newVAL) / rList->div));
                }

                // Move to next pin
                i += bytesPerPin;
            }

            // Leave parse loop
            break;
        }
        // If set pin data
        case MINOR_KEY_IO_AIO_SET:
        case MINOR_KEY_IO_DIO_SET:
        {
            // Parse & verify info from set data
            // Formatted as [pinNum, val_high, val_low, io_combo]
            if (val_len != 4) break;
            pin_num = values.at(s2_io_pin_num_loc);

            // Find pin layout on GUI
            if (!get_pin_layout(pInfo.pinType, pin_num, &pin)) break;

            // Set new combo
            set_pin_io(pin, io_combo_pos, values.at(s2_io_combo_loc));

            // Propogate combo value update
            comboBox = (QComboBox*) pin->itemAt(io_combo_pos)->widget();
            inputsChanged(pInfo.pinType, comboBox, io_combo_pos);

            // Subtract one from val_len (combo pos)
            // and fall through to set value
            val_len -= 1;
        }
        // If read/write pin data
        case MINOR_KEY_IO_AIO_READ:
        case MINOR_KEY_IO_DIO_READ:
        case MINOR_KEY_IO_AIO_WRITE:
        case MINOR_KEY_IO_DIO_WRITE:
        {
            // Parse & verify info from set data
            // Formatted as [pinNum, val_high, val_low]
            if (val_len != 3) break;
            pin_num = values.at(s2_io_pin_num_loc);
            value = ((uint16_t) values.at(s2_io_value_high_loc) << 8) | ((uchar) values.at(s2_io_value_low_loc));

            // Find pin layout on GUI (if not already found)
            // Break out of parse if not found
            if (!pin && !get_pin_layout(pInfo.pinType, pin_num, &pin)) break;

            // Get combo
            comboBox = (QComboBox*) pin->itemAt(io_combo_pos)->widget();
            combo_value = pinControlMap->value(comboBox->currentText());

            // Check if was a read (need to check pin disabled if so)
            bool exitWrite = false;
            switch (minorKey)
            {
                case MINOR_KEY_IO_AIO_READ:
                case MINOR_KEY_IO_DIO_READ:
                {
                    exitWrite = !pinDisabledSet->contains(combo_value);
                    break;
                }
            }

            // If exitWrite set, break out of parse
            if (exitWrite) break;

            // Scale value
            rList = pinRangeMap->value(combo_value);
            newVAL = qRound((float) value + (rList->min * rList->div));

            // Set slider
            set_pin_io(pin, io_slider_pos, newVAL);

            // Set lineEdit
            set_pin_io(pin, io_line_edit_pos, QString::number(((float) newVAL) / rList->div));

            // Break out after writing new value
            break;
        }
    }
}

bool GUI_IO_CONTROL::set_pin_io(QHBoxLayout *pin, uint8_t io_pos, QVariant value)
{
    // Verify pin
    if (!pin) return false;

    // Get io widget
    QWidget *pin_io = pin->itemAt(io_pos)->widget();

    // Block signals
    bool prev_block_io = pin_io->blockSignals(true);

    // Parse how to set
    bool set_success = false;
    switch (io_pos)
    {
        case io_combo_pos:
        {
            uint8_t combo_value = value.toUInt();
            QComboBox *pin_combo = (QComboBox*) pin_io;

            // Verify value in range (break if not)
            if (pin_combo->count() <= combo_value) break;

            // Set value
            pin_combo->setCurrentIndex(combo_value);
            set_success = (pin_combo->currentIndex() == combo_value);
            break;
        }
        case io_slider_pos:
        {
            int slider_value = value.toInt();
            QSlider *pin_slider = (QSlider*) pin_io;
            pin_slider->setValue(slider_value);
            set_success = (pin_slider->value() == slider_value);
            break;
        }
        case io_line_edit_pos:
        {
            QString lineEdit_value = value.toString();
            QLineEdit *pin_lineEdit = (QLineEdit*) pin_io;
            pin_lineEdit->setText(lineEdit_value);
            set_success = (pin_lineEdit->text() == lineEdit_value);
            break;
        }
    }

    // Unblock signals
    pin_io->blockSignals(prev_block_io);

    // Return if combo set
    return set_success;
}

bool GUI_IO_CONTROL::getPinTypeInfo(uint8_t pinType, PinTypeInfo *infoPtr)
{
    infoPtr->minorKey = pinType;

    // Set pin type variables
    switch (pinType)
    {
        case MINOR_KEY_IO_AIO:
        case MINOR_KEY_IO_AIO_SET:
        case MINOR_KEY_IO_AIO_WRITE:
        case MINOR_KEY_IO_AIO_READ:
        case MINOR_KEY_IO_AIO_READ_ALL:
            infoPtr->cols = num_AIOcols;
            infoPtr->grid = AIO_Grid;
            infoPtr->pinType = MINOR_KEY_IO_AIO;
            return true;
        case MINOR_KEY_IO_DIO:
        case MINOR_KEY_IO_DIO_SET:
        case MINOR_KEY_IO_DIO_WRITE:
        case MINOR_KEY_IO_DIO_READ:
        case MINOR_KEY_IO_DIO_READ_ALL:
            infoPtr->cols = num_DIOcols;
            infoPtr->grid = DIO_Grid;
            infoPtr->pinType = MINOR_KEY_IO_DIO;
            return true;
        case MINOR_KEY_IO_REMOTE_CONN:
        case MINOR_KEY_IO_REMOTE_CONN_SET:
        case MINOR_KEY_IO_REMOTE_CONN_READ:
            infoPtr->pinType = MINOR_KEY_IO_REMOTE_CONN;
            return true;
        default:
            delete infoPtr;
            *infoPtr = EMPTY_PIN_TYPE_INFO;
            return false;
    }
}

RangeList *GUI_IO_CONTROL::makeRangeList(QString rangeInfo)
{
    // Split range info string into values
    QStringList range_split = rangeInfo.split(':');
    if (range_split.length() != 4) return new EMPTY_RANGE;

    return new RangeList({
                             .min=range_split.at(0).toInt(),
                             .max=range_split.at(1).toInt(),
                             .step=range_split.at(2).toInt(),
                             .div=range_split.at(3).toFloat()
                         });
}

QHBoxLayout *GUI_IO_CONTROL::create_pin()
{
    // Create memebrs
    QHBoxLayout *pin = new QHBoxLayout();
    QLabel *pin_label = new QLabel(this);
    QComboBox *pin_combo = new QComboBox(this);
    QSlider *pin_slider = new QSlider(Qt::Horizontal, this);
    QLineEdit *pin_edit = new QLineEdit(this);

    // Verify memebrs
    if (!(pin && pin_label && pin_combo && pin_slider && pin_edit))
    {
        if (pin) delete pin;
        if (pin_label) delete pin_label;
        if (pin_combo) delete pin_combo;
        if (pin_slider) delete pin_slider;
        if (pin_edit) delete pin_edit;
        return nullptr;
    }

    // Set size attributes
    pin_label->setMinimumSize(20, 20);
    pin_combo->setMinimumSize(75, 20);
    pin_slider->setMinimumSize(85, 20);
    pin_edit->setMinimumSize(50, 20);
    pin_label->setMaximumSize(20, 20);
    pin_combo->setMaximumSize(75, 20);
    pin_slider->setMaximumSize(85, 20);
    pin_edit->setMaximumSize(50, 20);

    // Set size policy
    pin->setSizeConstraint(QLayout::SetMaximumSize);
    pin_label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    pin_combo->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    pin_slider->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    pin_edit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    // Set pin attributes
    pin_label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    pin_edit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

    // Add elements to pin layout
    pin->addWidget(pin_label);
    pin->addWidget(pin_combo);
    pin->addWidget(pin_slider);
    pin->addWidget(pin_edit);

    // Return the newly created pin
    return pin;
}

void GUI_IO_CONTROL::destroy_pin(QHBoxLayout *pin)
{
    // Verify valid input
    if (!pin) return;

    // Disconnect from signals
    disconnect_pin(pin);

    // Delete all members of layout
    QLayoutItem *item;
    while ((item = pin->takeAt(0)) != nullptr)
    {
        delete item->widget();
        delete item;
    }

    // Delete the layout
    delete pin;
}

void GUI_IO_CONTROL::destroy_unused_pins(PinTypeInfo *pInfo)
{
    // Verify valid input
    if (!pInfo) return;

    // Get current pins list
    QList<QHBoxLayout*> *pin_layouts = pinMap.value(pInfo->pinType);
    if (!pin_layouts) return;

    // Set pinType_str prepend
    QString pinType_str;
    if (pInfo->pinType == MINOR_KEY_IO_AIO) pinType_str = "AIO_";
    else if (pInfo->pinType == MINOR_KEY_IO_DIO) pinType_str = "DIO_";
    else pinType_str = "_";

    // Check all pins
    QLabel *pinLabel;
    foreach (QHBoxLayout *pin_layout, *pin_layouts)
    {
        // Get label text
        pinLabel = (QLabel*) pin_layout->itemAt(io_label_pos)->widget();

        // If not member of pinList, destroy pin and remove from map
        if (!pinList.contains(pinType_str + pinLabel->text()))
        {
            pin_layouts->removeAll(pin_layout);
            destroy_pin(pin_layout);
        }
    }
}

void GUI_IO_CONTROL::connect_pin(uint8_t pinType, QHBoxLayout *pin)
{
    // Verify valid input
    if (!pin) return;

    // Connect entries
    if (pinType == MINOR_KEY_IO_AIO)
    {
        connect((GUI_IO_CONTROL*) pin->itemAt(io_combo_pos)->widget(), SIGNAL(currentIndexChanged(int)),
                this, SLOT(AIO_ComboValueChanged()),
                Qt::DirectConnection);
        connect((GUI_IO_CONTROL*) pin->itemAt(io_slider_pos)->widget(), SIGNAL(valueChanged(int)),
                this, SLOT(AIO_SliderValueChanged()),
                Qt::DirectConnection);
        connect((GUI_IO_CONTROL*) pin->itemAt(io_line_edit_pos)->widget(), SIGNAL(editingFinished()),
                this, SLOT(AIO_LineEditValueChanged()),
                Qt::DirectConnection);
    } else if (pinType == MINOR_KEY_IO_DIO)
    {
        connect((GUI_IO_CONTROL*) pin->itemAt(io_combo_pos)->widget(), SIGNAL(currentIndexChanged(int)),
                this, SLOT(DIO_ComboValueChanged()),
                Qt::DirectConnection);
        connect((GUI_IO_CONTROL*) pin->itemAt(io_slider_pos)->widget(), SIGNAL(valueChanged(int)),
                this, SLOT(DIO_SliderValueChanged()),
                Qt::DirectConnection);
        connect((GUI_IO_CONTROL*) pin->itemAt(io_line_edit_pos)->widget(), SIGNAL(editingFinished()),
                this, SLOT(DIO_LineEditValueChanged()),
                Qt::DirectConnection);
    }
}

void GUI_IO_CONTROL::disconnect_pin(QHBoxLayout *pin)
{
    // Verify valid input
    if (!pin) return;

    // Disconnect entries
    disconnect(pin->itemAt(io_combo_pos)->widget(), 0, 0, 0);
    disconnect(pin->itemAt(io_slider_pos)->widget(), 0, 0, 0);
    disconnect(pin->itemAt(io_line_edit_pos)->widget(), 0, 0, 0);
}

void GUI_IO_CONTROL::update_pin_grid(PinTypeInfo *pInfo)
{
    // Verify valid input
    if (!pInfo) return;

    // Get grid info
    int grid_cols = pInfo->cols - 1;

    // Get pin list
    QList<QHBoxLayout*> *pins = pinMap.value(pInfo->pinType);
    if (!pins) return;

    // Setup loop variables
    int curr_col = 0;
    int curr_row = 0;
    int num_pins = pins->length();
    QLayout *new_item;
    QLayoutItem *old_item;

    // Replace items in grid if changed
    // (assumes pins is already in order)
    for (int i = 0; i < num_pins; i++)
    {
        // Set grid item to current pin
        new_item = pins->at(i);
        old_item = pInfo->grid->itemAtPosition(curr_row, curr_col);
        if (!old_item || (new_item != old_item->layout()))
        {
            pInfo->grid->removeItem(new_item);
            pInfo->grid->addLayout(new_item, curr_row, curr_col);
        }

        // Increment column and adjust row if overflows
        curr_col += 1;
        if (grid_cols < curr_col)
        {
            // Increment row and reset column
            curr_row += 1;
            curr_col = 0;
        }
    }
}

void GUI_IO_CONTROL::clear_all_maps()
{
    // Clear pin list
    pinList.clear();

    // Clear pin map
    foreach (uint8_t pinType, pinMap.keys())
    {
        foreach (QHBoxLayout *pin, *pinMap.value(pinType))
        {
            destroy_pin(pin);
        }
        delete pinMap.value(pinType);
    }
    pinMap.clear();

    // Clear control map
    foreach (uint8_t pinType, controlMap.keys())
    {
        delete controlMap.value(pinType);
    }
    controlMap.clear();

    // Clear disabled value set map
    foreach (uint8_t pinType, disabledValueSet.keys())
    {
        delete disabledValueSet.value(pinType);
    }
    disabledValueSet.clear();

    // Clear range map
    QMap<uint8_t, RangeList*> *rmap;
    foreach (uint8_t pinType, rangeMap.keys())
    {
        rmap = rangeMap.value(pinType);
        foreach (uint8_t rangeType, rmap->keys())
        {
            delete rmap->value(rangeType);
        }
        delete rmap;
    }
    rangeMap.clear();
}
