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

    delete ui;
}

void GUI_8AIO_16DIO_COMM::reset_gui()
{
    PinTypeInfo pInfo;
    QWidget *item;
    uint8_t rowNum, colNum;

    // Disconnect sending slot for setup
    emit connect_signals(false);

    // Stop logging and updating if running
    on_StopLog_Button_clicked();
    on_StopUpdater_Button_clicked();

    // Reset pin settings
    QList<uint8_t> pinTypes({MINOR_KEY_IO_AIO, MINOR_KEY_IO_DIO});
    foreach (uint8_t pinType, pinTypes)
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
                    item->blockSignals(true);
                    ((QComboBox*) item)->setCurrentIndex(0);
                    inputsChanged(&pInfo, item, io_combo_pos);
                    item->blockSignals(false);
                }
            }
        }
    }

    // Clear any rcvd data
    rcvd_formatted.clear();

    // Reconnect sending slot
    emit connect_signals(true);
}

void GUI_8AIO_16DIO_COMM::parseConfigMap(QMap<QString, QVariant> *configMap)
{
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

    ui->ConnType_Combo->blockSignals(true);
    ui->ConnType_Combo->clear();
    ui->ConnType_Combo->addItems(controlMap.value(pInfo.pinType)->keys());
    ui->ConnType_Combo->blockSignals(false);

    // Pass to parent for additional parsing
    GUI_PIN_BASE::parseConfigMap(configMap);
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
    send_io(&pInfo, data);
}

void GUI_8AIO_16DIO_COMM::DIO_SliderValueChanged()
{
    // Grab pin info
    PinTypeInfo pInfo;
    if (!getPinTypeInfo(MINOR_KEY_IO_DIO_SET, &pInfo)) return;

    // Set message for clicked button
    QByteArray data;
    inputsChanged(&pInfo, sender(), io_slider_pos, &data);

    // Send update
    send_io(&pInfo, data);
}

void GUI_8AIO_16DIO_COMM::DIO_LineEditValueChanged()
{
    // Grab pin info
    PinTypeInfo pInfo;
    if (!getPinTypeInfo(MINOR_KEY_IO_DIO_SET, &pInfo)) return;

    // Set message for clicked button
    QByteArray data;
    inputsChanged(&pInfo, sender(), io_line_edit_pos, &data);

    // Send update
    send_io(&pInfo, data);
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
    send_io(&pInfo, data);
}

void GUI_8AIO_16DIO_COMM::AIO_SliderValueChanged()
{
    // Grab pin info
    PinTypeInfo pInfo;
    if (!getPinTypeInfo(MINOR_KEY_IO_AIO_SET, &pInfo)) return;

    // Send message for edited button
    QByteArray data;
    inputsChanged(&pInfo, sender(), io_slider_pos, &data);

    // Send update
    send_io(&pInfo, data);
}

void GUI_8AIO_16DIO_COMM::AIO_LineEditValueChanged()
{
    // Grab pin info
    PinTypeInfo pInfo;
    if (!getPinTypeInfo(MINOR_KEY_IO_AIO_SET, &pInfo)) return;

    // Send message for edited button
    QByteArray data;
    inputsChanged(&pInfo, sender(), io_line_edit_pos, &data);

    // Send update
    send_io(&pInfo, data);
}

void GUI_8AIO_16DIO_COMM::updateValues()
{
    uint8_t requestType;
    QTimer *caller = (QTimer*) sender();
    if (caller == &DIO_READ) requestType = MINOR_KEY_IO_DIO_READ;
    else if (caller == &AIO_READ) requestType = MINOR_KEY_IO_AIO_READ;
    else return;

    send({
             gui_type,
             requestType,
             0
         });
}

void GUI_8AIO_16DIO_COMM::recordLogData()
{
    if (!logIsRecording) return;

    PinTypeInfo pInfo;
    if (getPinTypeInfo(MINOR_KEY_IO_AIO_READ, &pInfo)) recordPinValues(&pInfo);
    if (getPinTypeInfo(MINOR_KEY_IO_DIO_READ, &pInfo)) recordPinValues(&pInfo);

}

void GUI_8AIO_16DIO_COMM::on_StartUpdater_Button_clicked()
{
    ui->StartUpdater_Button->setText("Reset");

    DIO_READ.start((int) (S2MS * ui->DIO_UR_LineEdit->text().toFloat()));
    AIO_READ.start((int) (S2MS * ui->AIO_UR_LineEdit->text().toFloat()));
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

    logTimer.start((int) (S2MS * ui->LOG_UR_LineEdit->text().toFloat()));
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
    msg.append(controlMap.value(MINOR_KEY_IO_REMOTE_CONN)->value(ui->ConnType_Combo->currentText()));

    if (devConnected)
    {
        msg.append(MINOR_KEY_IO_REMOTE_CONN);

        ui->ConnConnect_Button->setText("Connect");
        ui->ConnSend_Button->setEnabled(false);
        devConnected = false;
    } else
    {
        msg.append(MINOR_KEY_IO_REMOTE_CONN);

        ui->ConnConnect_Button->setText("Disconnect");
        ui->ConnSend_Button->setEnabled(true);
        devConnected = true;
    }
    msg.append(ui->ConnSpeed_Combo->currentText());
    msg.append(ui->ConnAddr_Combo->currentText());

    send(msg);
}

void GUI_8AIO_16DIO_COMM::on_ConnSend_Button_clicked()
{
    QByteArray msg;
    msg.append(controlMap.value(MINOR_KEY_IO_REMOTE_CONN)->value(ui->ConnType_Combo->currentText()));
    msg.append(MINOR_KEY_IO_REMOTE_CONN);
    msg.append(ui->ConnMsg_LineEdit->text());
    msg.append(MINOR_KEY_IO_REMOTE_CONN);
    msg.append(MINOR_KEY_IO_REMOTE_CONN);

    send(msg);
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

    QMap<QString, uint8_t>* pinMap = controlMap.value(MINOR_KEY_IO_REMOTE_CONN);
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
    connect(&DIO_READ, SIGNAL(timeout()), this, SLOT(updateValues()));
    connect(&AIO_READ, SIGNAL(timeout()), this, SLOT(updateValues()));
    connect(&logTimer, SIGNAL(timeout()), this, SLOT(recordLogData()));
}

void GUI_8AIO_16DIO_COMM::connectUniversalSlots()
{
    PinTypeInfo pInfo;
    QWidget *item;
    uint8_t rowNum, colNum;

    // Get AIO pin info
    if (!getPinTypeInfo(MINOR_KEY_IO_AIO_SET, &pInfo))
    {
        GUI_HELPER::showMessage("Error: Unable to connect AIO!");
        QTimer::singleShot(0, this, SLOT(close()));
    }

    // Connect AIO combo changes to the universal slot
    for (uint8_t i = 0; i < pInfo.numPins_GUI; i++)
    {
        getPinLocation(&rowNum, &colNum, &pInfo, i);

        // Connect AIO Combo
        if (getItemWidget(&item, pInfo.grid, rowNum, colNum+io_combo_pos))
        {
            connect((QComboBox*) item, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(AIO_ComboChanged()),
                    Qt::QueuedConnection);
        }

        // Connect AIO Slider
        if (getItemWidget(&item, pInfo.grid, rowNum, colNum+io_slider_pos))
        {
            connect((QSlider*) item, SIGNAL(valueChanged(int)),
                    this, SLOT(AIO_SliderValueChanged()),
                    Qt::QueuedConnection);
        }

        // Connect AIO Text
        if (getItemWidget(&item, pInfo.grid, rowNum, colNum+io_line_edit_pos))
        {
            connect((QLineEdit*) item, SIGNAL(editingFinished()),
                    this, SLOT(AIO_LineEditValueChanged()),
                    Qt::QueuedConnection);
        }
    }

    // Get DIO pin info
    if (!getPinTypeInfo(MINOR_KEY_IO_DIO_SET, &pInfo))
    {
        GUI_HELPER::showMessage("Error: Unable to connect DIO!");
        QTimer::singleShot(0, this, SLOT(close()));
    }

    // Connect DIO combo changes to the universal slot
    for (uint8_t i = 0; i < pInfo.numPins_GUI; i++)
    {
        getPinLocation(&rowNum, &colNum, &pInfo, i);

        // Connect DIO Combo
        if (getItemWidget(&item, pInfo.grid, rowNum, colNum+io_combo_pos))
        {
            connect((QComboBox*) item, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(DIO_ComboChanged()),
                    Qt::QueuedConnection);
        }

        // Connect DIO Slider
        if (getItemWidget(&item, pInfo.grid, rowNum, colNum+io_slider_pos))
        {
            connect((QSlider*) item, SIGNAL(valueChanged(int)),
                    this, SLOT(DIO_SliderValueChanged()),
                    Qt::QueuedConnection);
        }

        // Connect DIO Text
        if (getItemWidget(&item, pInfo.grid, rowNum, colNum+io_line_edit_pos))
        {
            connect((QLineEdit*) item, SIGNAL(editingFinished()),
                    this, SLOT(DIO_LineEditValueChanged()),
                    Qt::QueuedConnection);
        }
    }
}

bool GUI_8AIO_16DIO_COMM::getPinTypeInfo(uint8_t pinType, PinTypeInfo *infoPtr)
{
    if (!GUI_PIN_BASE::getPinTypeInfo(pinType, infoPtr))
        return false;

    // Set ui pin type variables
    switch (pinType)
    {
        case MINOR_KEY_IO_AIO:
        case MINOR_KEY_IO_AIO_SET:
        case MINOR_KEY_IO_AIO_READ:
            infoPtr->grid = ui->AIO_Grid;
            return true;
        case MINOR_KEY_IO_DIO:
        case MINOR_KEY_IO_DIO_SET:
        case MINOR_KEY_IO_DIO_READ:
            infoPtr->grid = ui->DIO_Grid;
            return true;
        case MINOR_KEY_IO_REMOTE_CONN:
        case MINOR_KEY_IO_REMOTE_CONN_SET:
        case MINOR_KEY_IO_REMOTE_CONN_READ:
            infoPtr->pinType = MINOR_KEY_IO_REMOTE_CONN;
            return true;
        default:
            return false;
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

void GUI_8AIO_16DIO_COMM::setValues(uint8_t pinType, QByteArray values)
{
    PinTypeInfo pInfo;
    if (!getPinTypeInfo(pinType, &pInfo)) return;

    QComboBox *comboBox;
    QSlider *sliderValue;
    QLineEdit *textValue;
    uint32_t value;
    uint8_t rowNum, colNum, comboVal, divisor;

    // Get & verify maps
    QMap<QString, uint8_t>* pinMap = controlMap.value(pInfo.pinType);
    QMap<uint8_t, RangeList*>* pinRangeMap = rangeMap.value(pInfo.pinType);
    QList<uint8_t>* pinDisabledSet = disabledValueSet.value(pInfo.pinType);
    if (!pinMap || !pinRangeMap || !pinDisabledSet) return;

    uint8_t i = 0, j = 0, pin_num = 0;
    uint8_t val_len = values.length();
    while (i < val_len)
    {
        // Value is big endian
        value = 0;
        for (j = bytesPerPin; 0 < j; j--)
        {
            value = ((value << 8) | (values[i++] & 0xFF));
        }

        getPinLocation(&rowNum, &colNum, &pInfo, pin_num++);

        if (getItemWidget((QWidget**) &comboBox, pInfo.grid, rowNum, colNum+io_combo_pos))
        {
            comboVal = pinMap->value(comboBox->currentText());
            if (pinDisabledSet->contains(comboVal)
                    && getItemWidget((QWidget**) &sliderValue, pInfo.grid, rowNum, colNum+io_slider_pos)
                    && getItemWidget((QWidget**) &textValue, pInfo.grid, rowNum, colNum+io_line_edit_pos))
            {
                sliderValue->blockSignals(true);
                textValue->blockSignals(true);

                divisor = pinRangeMap->value(comboVal)->div;
                sliderValue->setSliderPosition(value);
                textValue->setText(QString::number(((float) value) / divisor));

                sliderValue->blockSignals(false);
                textValue->blockSignals(false);
            }
        }
    }
}
