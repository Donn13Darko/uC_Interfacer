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

#include "gui-8aio-16dio-comm.h"
#include "ui_gui-8aio-16dio-comm.h"

#include <QDateTime>
#include <QList>

GUI_8AIO_16DIO_COMM::GUI_8AIO_16DIO_COMM(QWidget *parent) :
    GUI_PIN_BASE(parent),
    ui(new Ui::GUI_8AIO_16DIO_COMM)
{
    // Setup ui
    ui->setupUi(this);
    ui->StartUpdater_Button->setText("Start");
    ui->StartLog_Button->setText("Start Log");

    // Call initializers
    initialize();
    connectUniversalSlots();
    setupUpdaters();

    // Reset GUI
    reset_gui();
}

GUI_8AIO_16DIO_COMM::~GUI_8AIO_16DIO_COMM()
{
    on_StopLog_Button_clicked();
    on_StopUpdater_Button_clicked();

    delete ui;
}

void GUI_8AIO_16DIO_COMM::parseConfigMap(QMap<QString, QVariant> *configMap)
{
    // Pass to parent for additional parsing
    GUI_PIN_BASE::parseConfigMap(configMap);

    // Setup pintypes variable
    PinTypeInfo pInfo;

    // Add DIO controls
    if (!getPinTypeInfo(MINOR_KEY_IO_DIO, &pInfo)) return;
    setNumPins(&pInfo, configMap->value("dio_num").toInt(),
                          configMap->value("dio_start_num").toInt());
    addNewPinSettings(&pInfo, configMap->value("dio_pin_settings").toStringList());
    setCombos(&pInfo, configMap->value("dio_combo_settings").toStringList());

    // Add AIO controls
    if (!getPinTypeInfo(MINOR_KEY_IO_AIO, &pInfo)) return;
    setNumPins(&pInfo, configMap->value("aio_num").toInt(),
                          configMap->value("aio_start_num").toInt());
    addNewPinSettings(&pInfo, configMap->value("aio_pin_settings").toStringList());
    setCombos(&pInfo, configMap->value("aio_combo_settings").toStringList());

    // Add Remote controls
    if (!getPinTypeInfo(MINOR_KEY_IO_REMOTE_CONN, &pInfo)) return;
    addNewPinSettings(&pInfo, configMap->value("remote_pin_settings").toStringList());

    bool prev_block_status = ui->ConnType_Combo->blockSignals(true);
    ui->ConnType_Combo->clear();
    ui->ConnType_Combo->addItems(controlMap.value(pInfo.pinType)->keys());
    ui->ConnType_Combo->blockSignals(prev_block_status);
}

void GUI_8AIO_16DIO_COMM::reset_gui()
{
    // Reset pin base
    GUI_PIN_BASE::reset_gui();

    // Setup loop variables
    PinTypeInfo pInfo;
    QWidget *item;
    uint8_t rowNum, colNum;
    bool prev_block_status;

    // Stop logging and updating if running
    on_StopLog_Button_clicked();
    on_StopUpdater_Button_clicked();

    // Reset pin settings
    foreach (uint8_t pinType,
             QList<uint8_t>({MINOR_KEY_IO_AIO, MINOR_KEY_IO_DIO}))
    {
        if (getPinTypeInfo(pinType, &pInfo))
        {
            // Set combo to start
            for (uint8_t i = 0; i < pInfo.numPins_DEV; i++)
            {
                getPinLocation(&rowNum, &colNum, &pInfo, i);

                // Reset Combo
                if (getItemWidget(&item, pInfo.grid, rowNum, colNum+io_combo_pos))
                {
                    prev_block_status = item->blockSignals(true);
                    ((QComboBox*) item)->setCurrentIndex(0);
                    inputsChanged(&pInfo, item, io_combo_pos);
                    item->blockSignals(prev_block_status);
                }
            }
        }
    }
}

void GUI_8AIO_16DIO_COMM::DIO_ComboChanged()
{
    // Grab pin info
    PinTypeInfo pInfo;
    if (!getPinTypeInfo(MINOR_KEY_IO_DIO_SET, &pInfo)) return;

    // Propogate updates
    QByteArray data;
    inputsChanged(&pInfo, sender(), io_combo_pos, &data);

    // Send update
    emit transmit_chunk(gui_key, pInfo.minorKey, data);
}

void GUI_8AIO_16DIO_COMM::DIO_SliderValueChanged()
{
    // Grab pin info
    PinTypeInfo pInfo;
    if (!getPinTypeInfo(MINOR_KEY_IO_DIO_WRITE, &pInfo)) return;

    // Set message for clicked button
    QByteArray data;
    inputsChanged(&pInfo, sender(), io_slider_pos, &data);

    // Remove combo setting (unneeded)
    data.remove(s2_io_combo_loc, 1);

    // Send update
    emit transmit_chunk(gui_key, pInfo.minorKey, data);
}

void GUI_8AIO_16DIO_COMM::DIO_LineEditValueChanged()
{
    // Grab pin info
    PinTypeInfo pInfo;
    if (!getPinTypeInfo(MINOR_KEY_IO_DIO_WRITE, &pInfo)) return;

    // Set message for clicked button
    QByteArray data;
    inputsChanged(&pInfo, sender(), io_line_edit_pos, &data);

    // Remove combo setting (unneeded)
    data.remove(s2_io_combo_loc, 1);

    // Send update
    emit transmit_chunk(gui_key, pInfo.minorKey, data);
}

void GUI_8AIO_16DIO_COMM::AIO_ComboChanged()
{
    // Grab pin info
    PinTypeInfo pInfo;
    if (!getPinTypeInfo(MINOR_KEY_IO_AIO_SET, &pInfo)) return;

    // Send message for edited button
    QByteArray data;
    inputsChanged(&pInfo, sender(), io_combo_pos, &data);

    // Send update
    emit transmit_chunk(gui_key, pInfo.minorKey, data);
}

void GUI_8AIO_16DIO_COMM::AIO_SliderValueChanged()
{
    // Grab pin info
    PinTypeInfo pInfo;
    if (!getPinTypeInfo(MINOR_KEY_IO_AIO_WRITE, &pInfo)) return;

    // Send message for edited button
    QByteArray data;
    inputsChanged(&pInfo, sender(), io_slider_pos, &data);

    // Remove combo setting (unneeded)
    data.remove(s2_io_combo_loc, 1);

    // Send update
    emit transmit_chunk(gui_key, pInfo.minorKey, data);
}

void GUI_8AIO_16DIO_COMM::AIO_LineEditValueChanged()
{
    // Grab pin info
    PinTypeInfo pInfo;
    if (!getPinTypeInfo(MINOR_KEY_IO_AIO_WRITE, &pInfo)) return;

    // Send message for edited button
    QByteArray data;
    inputsChanged(&pInfo, sender(), io_line_edit_pos, &data);

    // Remove combo setting (unneeded)
    data.remove(s2_io_combo_loc, 1);

    // Send update
    emit transmit_chunk(gui_key, pInfo.minorKey, data);
}

void GUI_8AIO_16DIO_COMM::updateValues()
{
    uint8_t requestType;
    QTimer *caller = (QTimer*) sender();
    if (caller == &DIO_READ) requestType = MINOR_KEY_IO_DIO_READ_ALL;
    else if (caller == &AIO_READ) requestType = MINOR_KEY_IO_AIO_READ_ALL;
    else return;

    emit transmit_chunk(gui_key, requestType);
}

void GUI_8AIO_16DIO_COMM::recordLogData()
{
    if (!logIsRecording) return;

    PinTypeInfo pInfo;
    if (getPinTypeInfo(MINOR_KEY_IO_AIO_READ_ALL, &pInfo)) recordPinValues(&pInfo);
    if (getPinTypeInfo(MINOR_KEY_IO_DIO_READ_ALL, &pInfo)) recordPinValues(&pInfo);

}

void GUI_8AIO_16DIO_COMM::on_StartUpdater_Button_clicked()
{
    ui->StartUpdater_Button->setText("Reset");

    DIO_READ.start((int) (GUI_HELPER::S2MS * ui->DIO_UR_LineEdit->text().toFloat()));
    AIO_READ.start((int) (GUI_HELPER::S2MS * ui->AIO_UR_LineEdit->text().toFloat()));
}

void GUI_8AIO_16DIO_COMM::on_StopUpdater_Button_clicked()
{
    ui->StartUpdater_Button->setText("Start");

    DIO_READ.stop();
    AIO_READ.stop();
}

void GUI_8AIO_16DIO_COMM::on_LogSaveLocSelect_Button_clicked()
{
    // Get file
    QString filePath;
    if (GUI_HELPER::getSaveFilePath(&filePath))
        ui->LogSaveLoc_LineEdit->setText(filePath);
}

void GUI_8AIO_16DIO_COMM::on_StartLog_Button_clicked()
{
    bool error = false;
    if (logIsRecording)
        error = GUI_HELPER::showMessage("Error: Already recording!");
    else if (ui->LogSaveLoc_LineEdit->text().isEmpty())
        error = GUI_HELPER::showMessage("Error: Must provide log file!");
    if (error) return;

    uint32_t enumFlags = QIODevice::WriteOnly | QIODevice::Text;
    if (ui->AppendLog_CheckBox->isChecked()) enumFlags |= QIODevice::Append;
    else enumFlags |= QIODevice::Truncate;

    logFile = new QFile(ui->LogSaveLoc_LineEdit->text());
    if (!logFile->open((QIODevice::OpenModeFlag) enumFlags))
        error = GUI_HELPER::showMessage("Error: Couldn't open log file!");
    if (error) return;

    logStream = new QTextStream(logFile);
    *logStream << "Started: " << QDateTime::currentDateTimeUtc().toString() << " ";
    *logStream << "with update rate " << ui->LOG_UR_LineEdit->text() << " seconds\n";
    logStream->flush();

    logTimer.start((int) (GUI_HELPER::S2MS * ui->LOG_UR_LineEdit->text().toFloat()));
    ui->StartLog_Button->setText("Running");
    ui->StartLog_Button->setEnabled(false);
    ui->AppendLog_CheckBox->setEnabled(false);
    ui->LOG_UR_LineEdit->setEnabled(false);
    logIsRecording = true;
}

void GUI_8AIO_16DIO_COMM::on_StopLog_Button_clicked()
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

void GUI_8AIO_16DIO_COMM::on_ConnConnect_Button_clicked()
{
    QByteArray msg;
    if (devConnected)
    {
        ui->ConnConnect_Button->setText("Connect");
        ui->ConnSend_Button->setEnabled(false);
        devConnected = false;
    } else
    {
        ui->ConnConnect_Button->setText("Disconnect");
        ui->ConnSend_Button->setEnabled(true);
        devConnected = true;
    }
    msg.append(ui->ConnSpeed_Combo->currentText());
    msg.append(ui->ConnAddr_Combo->currentText());

    emit transmit_chunk(gui_key, MINOR_KEY_IO_REMOTE_CONN, msg);
}

void GUI_8AIO_16DIO_COMM::on_ConnSend_Button_clicked()
{
    QByteArray msg;
    msg.append(ui->ConnMsg_LineEdit->text());

    emit transmit_chunk(gui_key, MINOR_KEY_IO_REMOTE_CONN, msg);
}

void GUI_8AIO_16DIO_COMM::on_ConnClearRecv_Button_clicked()
{
    ui->ConnRecv_PlainText->clear();
    rcvd_formatted.clear();
}

void GUI_8AIO_16DIO_COMM::on_ConnSaveRecv_Button_clicked()
{
    save_rcvd_formatted();
}

void GUI_8AIO_16DIO_COMM::on_ConnType_Combo_currentIndexChanged(int)
{
    QString currVal = ui->ConnType_Combo->currentText();
    uint8_t type = controlMap.value(MINOR_KEY_IO_REMOTE_CONN)->value(currVal);
    if (disabledValueSet.value(MINOR_KEY_IO_REMOTE_CONN)->contains(type)) ui->ConnSpeed_Combo->setEnabled(false);
    else ui->ConnSpeed_Combo->setEnabled(true);

    QStringList deviceConns = devSettings.value(currVal);
    ui->ConnAddr_Combo->clear();
    ui->ConnAddr_Combo->addItems(deviceConns);

    if (deviceConns.length() == 0) ui->ConnAddr_Combo->setEditable(true);
    else ui->ConnAddr_Combo->setEditable(false);
}

void GUI_8AIO_16DIO_COMM::setConTypes(QStringList connTypes, QList<char> mapValues)
{
    if (connTypes.length() != mapValues.length()) return;

    QMap<QString, uint8_t> *pinMap = controlMap.value(MINOR_KEY_IO_REMOTE_CONN);
    for (uint8_t i = 0; i < connTypes.length(); i++)
    {
        pinMap->insert(connTypes[i], mapValues[i]);
    }

    ui->ConnType_Combo->clear();
    ui->ConnType_Combo->addItems(connTypes);

    on_ConnType_Combo_currentIndexChanged(ui->ConnType_Combo->currentIndex());
}

void GUI_8AIO_16DIO_COMM::initialize()
{
    // Set class pin variables
    bytesPerPin = 2;

    num_AIOpins_GUI = 8;
    num_AIOpins_DEV = num_AIOpins_GUI;
    num_AIOcols = 1;
    num_AIOrows = num_AIOpins_GUI / num_AIOcols;
    num_AIObuttons = 4;
    num_AIOpins_START = 0;

    num_DIOpins_GUI = 16;
    num_DIOpins_DEV = num_DIOpins_GUI;
    num_DIOcols = 2;
    num_DIOrows = num_DIOpins_GUI / num_DIOcols;
    num_DIObuttons = 4;
    num_DIOpins_START = 0;

    // Set log file parameters
    logFile = NULL;
    logStream = NULL;
    logIsRecording = false;
}

void GUI_8AIO_16DIO_COMM::setupUpdaters()
{
    // Connect updaters
    // All internal object connections so direct is okay
    connect(&DIO_READ, SIGNAL(timeout()),
            this, SLOT(updateValues()),
            Qt::DirectConnection);
    connect(&AIO_READ, SIGNAL(timeout()),
            this, SLOT(updateValues()),
            Qt::DirectConnection);
    connect(&logTimer, SIGNAL(timeout()),
            this, SLOT(recordLogData()),
            Qt::DirectConnection);
}

void GUI_8AIO_16DIO_COMM::connectUniversalSlots()
{
    PinTypeInfo pInfo;
    QWidget *item;
    uint8_t rowNum, colNum;

    // Get AIO pin info
    if (!getPinTypeInfo(MINOR_KEY_IO_AIO, &pInfo))
    {
        GUI_HELPER::showMessage("Error: Unable to connect AIO!");
        QTimer::singleShot(0, this, SLOT(close()));
    }

    // Connect AIO combo changes to the universal slot
    // All internal object connections so direct is okay
    for (uint8_t i = 0; i < pInfo.numPins_GUI; i++)
    {
        getPinLocation(&rowNum, &colNum, &pInfo, i);

        // Connect AIO Combo
        if (getItemWidget(&item, pInfo.grid, rowNum, colNum+io_combo_pos))
        {
            connect((QComboBox*) item, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(AIO_ComboChanged()),
                    Qt::DirectConnection);
        }

        // Connect AIO Slider
        if (getItemWidget(&item, pInfo.grid, rowNum, colNum+io_slider_pos))
        {
            connect((QSlider*) item, SIGNAL(valueChanged(int)),
                    this, SLOT(AIO_SliderValueChanged()),
                    Qt::DirectConnection);
        }

        // Connect AIO Text
        if (getItemWidget(&item, pInfo.grid, rowNum, colNum+io_line_edit_pos))
        {
            connect((QLineEdit*) item, SIGNAL(editingFinished()),
                    this, SLOT(AIO_LineEditValueChanged()),
                    Qt::DirectConnection);
        }
    }

    // Get DIO pin info
    if (!getPinTypeInfo(MINOR_KEY_IO_DIO, &pInfo))
    {
        GUI_HELPER::showMessage("Error: Unable to connect DIO!");
        QTimer::singleShot(0, this, SLOT(close()));
    }

    // Connect DIO combo changes to the universal slot
    // All internal object connections so direct is okay
    for (uint8_t i = 0; i < pInfo.numPins_GUI; i++)
    {
        getPinLocation(&rowNum, &colNum, &pInfo, i);

        // Connect DIO Combo
        if (getItemWidget(&item, pInfo.grid, rowNum, colNum+io_combo_pos))
        {
            connect((QComboBox*) item, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(DIO_ComboChanged()),
                    Qt::DirectConnection);
        }

        // Connect DIO Slider
        if (getItemWidget(&item, pInfo.grid, rowNum, colNum+io_slider_pos))
        {
            connect((QSlider*) item, SIGNAL(valueChanged(int)),
                    this, SLOT(DIO_SliderValueChanged()),
                    Qt::DirectConnection);
        }

        // Connect DIO Text
        if (getItemWidget(&item, pInfo.grid, rowNum, colNum+io_line_edit_pos))
        {
            connect((QLineEdit*) item, SIGNAL(editingFinished()),
                    this, SLOT(DIO_LineEditValueChanged()),
                    Qt::DirectConnection);
        }
    }
}

bool GUI_8AIO_16DIO_COMM::getPinTypeInfo(uint8_t pinType, PinTypeInfo *infoPtr)
{
    bool base = GUI_PIN_BASE::getPinTypeInfo(pinType, infoPtr);

    // Set ui pin type variables
    switch (pinType)
    {
        case MINOR_KEY_IO_AIO:
        case MINOR_KEY_IO_AIO_SET:
        case MINOR_KEY_IO_AIO_WRITE:
        case MINOR_KEY_IO_AIO_READ:
        case MINOR_KEY_IO_AIO_READ_ALL:
            infoPtr->grid = ui->AIO_Grid;
            return base;
        case MINOR_KEY_IO_DIO:
        case MINOR_KEY_IO_DIO_SET:
        case MINOR_KEY_IO_DIO_WRITE:
        case MINOR_KEY_IO_DIO_READ:
        case MINOR_KEY_IO_DIO_READ_ALL:
            infoPtr->grid = ui->DIO_Grid;
            return base;
        case MINOR_KEY_IO_REMOTE_CONN:
        case MINOR_KEY_IO_REMOTE_CONN_SET:
        case MINOR_KEY_IO_REMOTE_CONN_READ:
            // No info in pin base, set here
            infoPtr->pinType = MINOR_KEY_IO_REMOTE_CONN;
            return true;
        default:
            return base;
    }
}

bool GUI_8AIO_16DIO_COMM::isDataRequest(uint8_t minorKey)
{
    switch (minorKey)
    {
        case MINOR_KEY_IO_REMOTE_CONN_READ:
            return true;
        default:
            return GUI_PIN_BASE::isDataRequest(minorKey);
    }
}

void GUI_8AIO_16DIO_COMM::setValues(uint8_t minorKey, QByteArray values)
{
    // Get pin information
    PinTypeInfo pInfo;
    if (!getPinTypeInfo(minorKey, &pInfo)) return;

    // Get & verify maps
    QMap<QString, uint8_t> *pinMap = controlMap.value(pInfo.pinType);
    QMap<uint8_t, RangeList*> *pinRangeMap = rangeMap.value(pInfo.pinType);
    QList<uint8_t> *pinDisabledSet = disabledValueSet.value(pInfo.pinType);
    if (!pinMap || !pinRangeMap || !pinDisabledSet) return;

    // Allocate loop variables
    QComboBox *comboBox;
    QSlider *sliderValue;
    QLineEdit *textValue;
    uint16_t value;
    uint8_t rowNum, colNum, comboVal, divisor;
    bool prev_block_combo, prev_block_slider, prev_block_text;

    // Set loop variables
    uint8_t pin_num = 0;

    switch (minorKey)
    {
        // If read data
        case MINOR_KEY_IO_AIO_READ_ALL:
        case MINOR_KEY_IO_DIO_READ_ALL:
        {
            // Setup variables
            uint8_t i = 0, j = 0, val_len = values.length();

            // Loop over all pins and set their value
            while (i < val_len)
            {
                // Value is big endian
                value = 0;
                for (j = 0; j < bytesPerPin; j++)
                {
                    value = (value << 8) | ((uchar) values[i++]);
                }

                // Find pin location on GUI
                getPinLocation(&rowNum, &colNum, &pInfo, pin_num++);

                // Get combo widget
                if (getItemWidget((QWidget**) &comboBox, pInfo.grid, rowNum, colNum+io_combo_pos))
                {
                    // Get combo value
                    comboVal = pinMap->value(comboBox->currentText());

                    // Only set if not GUI controllable and able to get other widgets
                    if (pinDisabledSet->contains(comboVal)
                            && getItemWidget((QWidget**) &sliderValue, pInfo.grid, rowNum, colNum+io_slider_pos)
                            && getItemWidget((QWidget**) &textValue, pInfo.grid, rowNum, colNum+io_line_edit_pos))
                    {
                        // Block signals before setting
                        prev_block_slider = sliderValue->blockSignals(true);
                        prev_block_text = textValue->blockSignals(true);

                        // Set values
                        divisor = pinRangeMap->value(comboVal)->div;
                        sliderValue->setSliderPosition(value);
                        textValue->setText(QString::number(((float) value) / divisor));

                        // Unblock signals now that they are set
                        sliderValue->blockSignals(prev_block_slider);
                        textValue->blockSignals(prev_block_text);
                    }
                }
            }

            // Leave parse loop
            break;
        }
        // If set data
        case MINOR_KEY_IO_AIO_SET:
        case MINOR_KEY_IO_DIO_SET:
        {
            // Parse & verify info from set data
            // Formatted as [pinNum, val_high, val_low, io_combo]
            pin_num = values.at(s2_io_pin_num_loc);
            value = ((uint16_t) values.at(s2_io_value_high_loc) << 8) | ((uchar) values.at(s2_io_value_low_loc));
            QString combo_text = pinMap->key(values.at(s2_io_combo_loc), "");
            if (combo_text.isEmpty()) return;

            // Find pin location on GUI
            getPinLocation(&rowNum, &colNum, &pInfo, pin_num);

            // Get all widgets for setting
            if (getItemWidget((QWidget**) &comboBox, pInfo.grid, rowNum, colNum+io_combo_pos)
                    && getItemWidget((QWidget**) &sliderValue, pInfo.grid, rowNum, colNum+io_slider_pos)
                    && getItemWidget((QWidget**) &textValue, pInfo.grid, rowNum, colNum+io_line_edit_pos))
            {
                // Block combo signals before setting
                prev_block_combo = comboBox->blockSignals(true);
                prev_block_slider = sliderValue->blockSignals(true);
                prev_block_text = textValue->blockSignals(true);

                // Set combo to correct position
                comboBox->setCurrentText(combo_text);

                // Get combo value
                comboVal = pinMap->value(comboBox->currentText());

                // Overwrite current values with inputs changed
                bool disableClicks = pinDisabledSet->contains(comboVal);
                sliderValue->setAttribute(Qt::WA_TransparentForMouseEvents, disableClicks);
                textValue->setAttribute(Qt::WA_TransparentForMouseEvents, disableClicks);

                // Get & update slider range list
                RangeList *rList = pinRangeMap->value(comboVal);
                updateSliderRange(sliderValue, rList);

                // Set slider and divisor
                divisor = pinRangeMap->value(comboVal)->div;
                sliderValue->setSliderPosition(value);
                textValue->setText(QString::number(((float) value) / divisor));

                // Unblock signals now that they are set
                comboBox->blockSignals(prev_block_combo);
                sliderValue->blockSignals(prev_block_slider);
                textValue->blockSignals(prev_block_text);
            }

            // Break out after setting and writing new value
            break;
        }
        // If write data
        case MINOR_KEY_IO_AIO_WRITE:
        case MINOR_KEY_IO_DIO_WRITE:
        {
            // Parse & verify info from set data
            // Formatted as [pinNum, val_high, val_low]
            pin_num = values.at(s2_io_pin_num_loc);
            value = ((uint16_t) values.at(s2_io_value_high_loc) << 8) | ((uchar) values.at(s2_io_value_low_loc));

            // Find pin location on GUI
            getPinLocation(&rowNum, &colNum, &pInfo, pin_num);

            // Get all widgets for setting
            if (getItemWidget((QWidget**) &comboBox, pInfo.grid, rowNum, colNum+io_combo_pos)
                    && getItemWidget((QWidget**) &sliderValue, pInfo.grid, rowNum, colNum+io_slider_pos)
                    && getItemWidget((QWidget**) &textValue, pInfo.grid, rowNum, colNum+io_line_edit_pos))
            {
                // Block combo signals before setting
                prev_block_combo = comboBox->blockSignals(true);
                prev_block_slider = sliderValue->blockSignals(true);
                prev_block_text = textValue->blockSignals(true);

                // Get combo value
                comboVal = pinMap->value(comboBox->currentText());

                // Set slider and divisor
                divisor = pinRangeMap->value(comboVal)->div;
                sliderValue->setSliderPosition(value);
                textValue->setText(QString::number(((float) value) / divisor));

                // Unblock signals now that they are set
                comboBox->blockSignals(prev_block_combo);
                sliderValue->blockSignals(prev_block_slider);
                textValue->blockSignals(prev_block_text);
            }

            // Break out after writing new value
            break;
        }
    }
}
