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

#include "../communication/serial-rs232.h"

GUI_8AIO_16DIO_COMM::GUI_8AIO_16DIO_COMM(QWidget *parent) :
    GUI_PIN_BASE(parent),
    ui(new Ui::GUI_8AIO_16DIO_COMM)
{
    ui->setupUi(this);
    ui->updateStarter->setText("Start");
    ui->startLog->setText("Start Log");

    initialize();
    connectUniversalSlots();

    setupUpdaters();
}

GUI_8AIO_16DIO_COMM::~GUI_8AIO_16DIO_COMM()
{
    on_stopLog_clicked();

    delete ui;
}

void GUI_8AIO_16DIO_COMM::reset_gui()
{
    PinTypeInfo pInfo;
    QWidget *item;
    int rowNum, colNum;

    // Disconnect sending slot
    emit connect_signals(false);

    // Stop logging and updating if running
    on_stopLog_clicked();
    on_updateStopper_clicked();

    // Get AIO pin info
    if (getPinTypeInfo(JSON_AIO, &pInfo))
    {
        // Set AIO combo to start
        for (int i = 0; i < pInfo.numPins; i++)
        {
            getPinLocation(&rowNum, &colNum, &pInfo, i);

            // Set AIO Combo
            if (getItemWidget(&item, pInfo.grid, rowNum, colNum+comboPos))
            {
                ((QComboBox*) item)->setCurrentIndex(0);
            }

            // Set AIO Slider
            if (getItemWidget(&item, pInfo.grid, rowNum, colNum+slideValuePos))
            {
                ((QSlider*) item)->setValue(0);
            }

            // Set AIO Text
            if (getItemWidget(&item, pInfo.grid, rowNum, colNum+textValuePos))
            {
                ((QLineEdit*) item)->setText("0.0");
            }
        }
    }

    // Get DIO pin info
    if (getPinTypeInfo(JSON_DIO, &pInfo))
    {
        // Set DIO combo to start
        for (int i = 0; i < pInfo.numPins; i++)
        {
            getPinLocation(&rowNum, &colNum, &pInfo, i);

            // Set DIO Combo
            if (getItemWidget(&item, pInfo.grid, rowNum, colNum+comboPos))
            {
                ((QComboBox*) item)->setCurrentIndex(0);
            }

            // Set DIO Slider
            if (getItemWidget(&item, pInfo.grid, rowNum, colNum+slideValuePos))
            {
                ((QSlider*) item)->setValue(0);
            }

            // Set DIO Text
            if (getItemWidget(&item, pInfo.grid, rowNum, colNum+textValuePos))
            {
                ((QLineEdit*) item)->setText("0");
            }
        }
    }

    currData.clear();

    // Reconnect sending slot
    emit connect_signals(true);
}

void GUI_8AIO_16DIO_COMM::initialize()
{
    // Set class pin variables
    bytesPerPin = 3;

    num_AIOpins_GUI = 8;
    num_AIOpins_DEV = num_AIOpins_GUI;
    num_AIOcols = 1;
    num_AIOrows = num_AIOpins_GUI / num_AIOcols;
    num_AIObuttons = 4;

    num_DIOpins_GUI = 16;
    num_DIOpins_DEV = num_DIOpins_GUI;
    num_DIOcols = 2;
    num_DIOrows = num_DIOpins_GUI / num_DIOcols;
    num_DIObuttons = 4;

    labelPos = 0;
    comboPos = 1;
    slideValuePos = 2;
    textValuePos = 3;

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
    int rowNum, colNum;

    // Get AIO pin info
    if (!getPinTypeInfo(JSON_AIO, &pInfo))
    {
        showMessage("Error: Unable to connect AIO!");
        QTimer::singleShot(0, this, SLOT(close()));
    }

    // Connect AIO combo changes to the universal slot
    for (int i = 0; i < pInfo.numPins; i++)
    {
        getPinLocation(&rowNum, &colNum, &pInfo, i);

        // Connect AIO Combo
        if (getItemWidget(&item, pInfo.grid, rowNum, colNum+comboPos))
        {
            connect((QComboBox*) item, SIGNAL(currentIndexChanged(int)), this, SLOT(AIO_ComboChanged()));
        }

        // Connect AIO Slider
        if (getItemWidget(&item, pInfo.grid, rowNum, colNum+slideValuePos))
        {
            connect((QSlider*) item, SIGNAL(valueChanged(int)), this, SLOT(AIO_SliderValueChanged()));
        }

        // Connect AIO Text
        if (getItemWidget(&item, pInfo.grid, rowNum, colNum+textValuePos))
        {
            connect((QLineEdit*) item, SIGNAL(editingFinished()), this, SLOT(AIO_TextValueChanged()));
        }
    }

    // Get DIO pin info
    if (!getPinTypeInfo(JSON_DIO, &pInfo))
    {
        showMessage("Error: Unable to connect DIO!");
        QTimer::singleShot(0, this, SLOT(close()));
    }

    // Connect DIO combo changes to the universal slot
    for (int i = 0; i < pInfo.numPins; i++)
    {
        getPinLocation(&rowNum, &colNum, &pInfo, i);

        // Connect DIO Combo
        if (getItemWidget(&item, pInfo.grid, rowNum, colNum+comboPos))
        {
            connect((QComboBox*) item, SIGNAL(currentIndexChanged(int)), this, SLOT(DIO_ComboChanged()));
        }

        // Connect DIO Slider
        if (getItemWidget(&item, pInfo.grid, rowNum, colNum+slideValuePos))
        {
            connect((QSlider*) item, SIGNAL(valueChanged(int)), this, SLOT(DIO_SliderValueChanged()));
        }

        // Connect DIO Text
        if (getItemWidget(&item, pInfo.grid, rowNum, colNum+textValuePos))
        {
            connect((QLineEdit*) item, SIGNAL(editingFinished()), this, SLOT(DIO_TextValueChanged()));
        }
    }
}

void GUI_8AIO_16DIO_COMM::on_RESET_BUTTON_clicked()
{
    // Reset the GUI
    reset_gui();

    // Reset the Remote
    reset_remote();
}

void GUI_8AIO_16DIO_COMM::DIO_ComboChanged()
{
    // Grab pin info
    PinTypeInfo pInfo;
    if (!getPinTypeInfo(JSON_DIO, &pInfo)) return;

    // Set message for clicked button
    inputsChanged(&pInfo, comboPos);
}

void GUI_8AIO_16DIO_COMM::DIO_SliderValueChanged()
{
    // Grab pin info
    PinTypeInfo pInfo;
    if (!getPinTypeInfo(JSON_DIO, &pInfo)) return;

    // Set message for clicked button
    inputsChanged(&pInfo, slideValuePos);
}

void GUI_8AIO_16DIO_COMM::DIO_TextValueChanged()
{
    // Grab pin info
    PinTypeInfo pInfo;
    if (!getPinTypeInfo(JSON_DIO, &pInfo)) return;

    // Set message for clicked button
    inputsChanged(&pInfo, textValuePos);
}

void GUI_8AIO_16DIO_COMM::AIO_ComboChanged()
{
    // Grab pin info
    PinTypeInfo pInfo;
    if (!getPinTypeInfo(JSON_AIO, &pInfo)) return;

    // Send message for edited button
    inputsChanged(&pInfo, comboPos);
}

void GUI_8AIO_16DIO_COMM::AIO_SliderValueChanged()
{
    // Grab pin info
    PinTypeInfo pInfo;
    if (!getPinTypeInfo(JSON_AIO, &pInfo)) return;

    // Send message for edited button
    inputsChanged(&pInfo, slideValuePos);
}

void GUI_8AIO_16DIO_COMM::AIO_TextValueChanged()
{
    // Grab pin info
    PinTypeInfo pInfo;
    if (!getPinTypeInfo(JSON_AIO, &pInfo)) return;

    // Send message for edited button
    inputsChanged(&pInfo, textValuePos);
}

void GUI_8AIO_16DIO_COMM::updateValues()
{
    uint8_t pinType;
    QTimer *caller = (QTimer*) sender();
    if (caller == &DIO_READ) pinType = JSON_DIO;
    else if (caller == &AIO_READ) pinType = JSON_AIO;
    else return;

    send({
             JSON_READ,
             pinType
         });
}

void GUI_8AIO_16DIO_COMM::receive(QByteArray recvData)
{
    currData.append(recvData);
    int m = currData.length();
    if (m & 1) m = m - 1;
    if (m == 0) return;

    // Search received for valid key,value formations
    uint8_t key, value;
    for (int i = 0; i < (m - 1); i++)
    {
        key = (uint8_t) currData[0];
        value = (uint8_t) currData[1];

        int e = 0;
        switch (key)
        {
            case JSON_READ:
                {
                    switch (value)
                    {
                        case JSON_DIO:
                            if ((bytesPerPin*num_DIOpins_DEV) < m)
                                e = bytesPerPin*num_DIOpins_DEV;
                            else
                                return;
                            break;
                        case JSON_AIO:
                            if ((bytesPerPin*num_AIOpins_DEV) < m)
                                e = bytesPerPin*num_AIOpins_DEV;
                            else
                                return;
                            break;
                        default:
                            currData = currData.mid(1);
                            return;
                    }

                    if (e != 0)
                    {
                        // Update the value set & update i to remove from buffer
                        setValues(value, currData.mid(2, e));
                        currData = currData.mid(e+4);
                        return;
                    }
                }
                break;
            default:
                break;
        }

        // Remove ignored tokens
        currData = currData.mid(e + 1);
    }
}

void GUI_8AIO_16DIO_COMM::recordLogData()
{
    if (!logIsRecording) return;

    PinTypeInfo pInfo;
    if (getPinTypeInfo(JSON_AIO, &pInfo)) recordPinValues(&pInfo);
    if (getPinTypeInfo(JSON_DIO, &pInfo)) recordPinValues(&pInfo);

}

void GUI_8AIO_16DIO_COMM::recordPinValues(PinTypeInfo *pInfo)
{
    if (logStream == nullptr) return;

    *logStream << pInfo->pinType << ",";

    QLineEdit *textValue;
    int iend = pInfo->rows-1, jend = pInfo->cols-1;
    for (int i = 0; i < pInfo->rows; i++)
    {
        int colSel = textValuePos;
        for (int j = 0; j < pInfo->cols; j++)
        {
            getItemWidget((QWidget**) &textValue, pInfo->grid, i, colSel);
            *logStream << textValue->text();

            if ((i == iend) && (j == jend)) *logStream << "\n";
            else *logStream << ",";

            colSel += pInfo->numButtons;
        }
    }

    logStream->flush();
}

void GUI_8AIO_16DIO_COMM::on_updateStarter_clicked()
{
    ui->updateStarter->setText("Reset");

    DIO_READ.start((int) (S2MS * ui->DIO_UREdit->text().toFloat()));
    AIO_READ.start((int) (S2MS * ui->AIO_UREdit->text().toFloat()));
}

void GUI_8AIO_16DIO_COMM::on_updateStopper_clicked()
{
    ui->updateStarter->setText("Start");

    DIO_READ.stop();
    AIO_READ.stop();
}

void GUI_8AIO_16DIO_COMM::on_selectSaveLocation_clicked()
{
    // Get file
    QString filePath;
    if (getSaveFilePath(&filePath))
        ui->saveLocEdit->setText(filePath);
}

void GUI_8AIO_16DIO_COMM::on_startLog_clicked()
{
    bool error = false;
    if (logIsRecording) error = showMessage("Error: Already recording!");
    else if (ui->saveLocEdit->text().isEmpty()) error = showMessage("Error: Must provide log file!");
    if (error) return;

    uint32_t enumFlags = QIODevice::WriteOnly | QIODevice::Text;
    if (ui->appendLog->isChecked()) enumFlags |= QIODevice::Append;
    else enumFlags |= QIODevice::Truncate;

    logFile = new QFile(ui->saveLocEdit->text());
    if (!logFile->open((QIODevice::OpenModeFlag) enumFlags)) error = showMessage("Error: Couldn't open log file!");
    if (error) return;

    logStream = new QTextStream(logFile);
    *logStream << "Started: " << QDateTime::currentDateTimeUtc().toString() << " ";
    *logStream << "with update rate " << ui->LOG_UREdit->text() << " seconds\n";
    logStream->flush();

    logTimer.start((int) (S2MS * ui->LOG_UREdit->text().toFloat()));
    ui->startLog->setText("Running");
    ui->startLog->setEnabled(false);
    ui->appendLog->setEnabled(false);
    ui->LOG_UREdit->setEnabled(false);
    ui->logURLabel->setEnabled(false);
    logIsRecording = true;
}

void GUI_8AIO_16DIO_COMM::on_stopLog_clicked()
{
    if (!logIsRecording) return;
    logTimer.stop();

    logStream->flush();
    logFile->close();
    delete logStream;
    delete logFile;
    logStream = NULL;
    logFile = NULL;

    ui->startLog->setText("Start Log");
    ui->startLog->setEnabled(true);
    ui->appendLog->setEnabled(true);
    ui->LOG_UREdit->setEnabled(true);
    ui->logURLabel->setEnabled(true);
    logIsRecording = false;
}

void GUI_8AIO_16DIO_COMM::on_ConnectButton_clicked()
{
    QByteArray msg;
    msg.append(controlMap.value(JSON_REMOTE_CONN)->value(ui->ConnTypeCombo->currentText()));

    if (devConnected)
    {
        msg.append(REMOTE_CONN_DISCONNECT);

        ui->ConnectButton->setText("Connect");
        ui->SendButton->setEnabled(false);
        devConnected = false;
    } else
    {
        msg.append(REMOTE_CONN_CONNECT);

        ui->ConnectButton->setText("Disconnect");
        ui->SendButton->setEnabled(true);
        devConnected = true;
    }
    msg.append(ui->SpeedCombo->currentText());
    msg.append(ui->DeviceCombo->currentText());

    send(msg);
}

void GUI_8AIO_16DIO_COMM::on_SendButton_clicked()
{
    QByteArray msg;
    msg.append(controlMap.value(JSON_REMOTE_CONN)->value(ui->ConnTypeCombo->currentText()));
    msg.append(REMOTE_CONN_SEND);
    msg.append(ui->MessageEdit->text());
    msg.append(REMOTE_CONN_SEND);
    msg.append(JSON_END);

    send(msg);
}

void GUI_8AIO_16DIO_COMM::on_ClearRecvButton_clicked()
{
    ui->RecvTextBox->clear();
}

void GUI_8AIO_16DIO_COMM::on_ConnTypeCombo_currentIndexChanged(int)
{
    QString currVal = ui->ConnTypeCombo->currentText();
    uint8_t type = controlMap.value(JSON_REMOTE_CONN)->value(currVal);
    if (disabledValueSet.value(JSON_REMOTE_CONN)->contains(type)) ui->SpeedCombo->setEnabled(false);
    else ui->SpeedCombo->setEnabled(true);

    QStringList deviceConns = devSettings.value(currVal);
    ui->DeviceCombo->clear();
    ui->DeviceCombo->addItems(deviceConns);

    if (deviceConns.length() == 0) ui->DeviceCombo->setEditable(true);
    else ui->DeviceCombo->setEditable(false);
}

void GUI_8AIO_16DIO_COMM::setValues(uint8_t pinType, QByteArray values)
{
    PinTypeInfo pInfo;
    if (!getPinTypeInfo(pinType, &pInfo)) return;

    QComboBox *comboBox;
    QSlider *sliderValue;
    QLineEdit *textValue;
    char comboVal;
    uint32_t value;
    uint8_t pin_num;
    int rowNum, colNum, divisor;
    QMap<QString, uint8_t>* pinMap = controlMap.value(pInfo.pinType);
    QMap<uint8_t, RangeList>* pinRangeMap = rangeMap.value(pInfo.pinType);
    QList<uint8_t>* pinDisabledSet = disabledValueSet.value(pInfo.pinType);

    if (!pinMap || !pinRangeMap || !pinDisabledSet) return;

    for (int i = 0; i < values.length(); i++)
    {
        value = 0;
        pin_num = values[i];
        for (int j = (bytesPerPin - 1); 0 < j; j--)
        {
            i = i + 1;
            value = value | (values[i] & 0xFF) << (8 * (j - 1));
        }

        getPinLocation(&rowNum, &colNum, &pInfo, pin_num);

        if (getItemWidget((QWidget**) &comboBox, pInfo.grid, rowNum, colNum+comboPos))
        {
            comboVal = pinMap->value(comboBox->currentText());
            if (pinDisabledSet->contains(comboVal)
                    && getItemWidget((QWidget**) &sliderValue, pInfo.grid, rowNum, colNum+slideValuePos)
                    && getItemWidget((QWidget**) &textValue, pInfo.grid, rowNum, colNum+textValuePos))
            {
                sliderValue->blockSignals(true);
                textValue->blockSignals(true);

                divisor = pinRangeMap->value(comboVal).div;
                sliderValue->setSliderPosition(value);
                textValue->setText(QString::number(((float) value) / divisor));

                sliderValue->blockSignals(false);
                textValue->blockSignals(false);
            }
        }
    }
}

void GUI_8AIO_16DIO_COMM::setNumPins(uint8_t pinType, uint8_t num_dev_pins)
{
    PinTypeInfo pInfo;
    if (!getPinTypeInfo(pinType, &pInfo)) return;

    // Update dev pins
    switch (pInfo.pinType)
    {
        case JSON_AIO:
            num_AIOpins_DEV = num_dev_pins;
            break;
        case JSON_DIO:
            num_DIOpins_DEV = num_dev_pins;
            break;
    }

    // Disable each button set in the list
    for (int i = (pInfo.numPins-1); num_dev_pins <= i; i--)
    {
        setPinAttribute(&pInfo, i, Qt::WA_Disabled, true);
    }
}

void GUI_8AIO_16DIO_COMM::setCombos(uint8_t pinType, QList<QString> values, QList<int> pinNums)
{
    if (pinType == REMOTE_CONN_REMOTE)
    {
        ui->ConnTypeCombo->blockSignals(true);
        ui->ConnTypeCombo->clear();
        ui->ConnTypeCombo->addItem(values[0]);
        ui->ConnTypeCombo->blockSignals(false);
        return;
    }

    PinTypeInfo pInfo;
    if (!getPinTypeInfo(pinType, &pInfo)) return;

    if (pinNums.length() == 0)
    {
        pinNums.clear();
        for (int i = 0; i < pInfo.numPins; i++)
        {
            pinNums.append(i);
        }
    }

    // Set combo for each pin in the list
    int rowNum, colNum;
    QComboBox *itemCombo;
    QWidget *sliderWidget, *textWidget;
    QStringList listValues(values);
    QMap<QString, uint8_t>* pinMap = controlMap.value(pInfo.pinType);
    QMap<uint8_t, RangeList>* pinRangeMap = rangeMap.value(pInfo.pinType);
    QList<uint8_t>* pinDisabledSet = disabledValueSet.value(pInfo.pinType);

    if (!pinMap || !pinRangeMap || !pinDisabledSet) return;

    foreach (int i, pinNums)
    {
        // Find row & column of desired combo
        getPinLocation(&rowNum, &colNum, &pInfo, i);

        // Replace combo options
        if (getItemWidget((QWidget**) &itemCombo, pInfo.grid, rowNum, colNum+comboPos))
        {
            itemCombo->blockSignals(true);

            itemCombo->clear();
            itemCombo->addItems(listValues);

            char IO = pinMap->value(itemCombo->currentText());
            if (getItemWidget(&sliderWidget, pInfo.grid, rowNum, colNum+slideValuePos)
                    && getItemWidget(&textWidget, pInfo.grid, rowNum, colNum+textValuePos))
            {
                if (pinDisabledSet->contains(IO))
                {
                    sliderWidget->setAttribute(Qt::WA_TransparentForMouseEvents, true);
                    textWidget->setAttribute(Qt::WA_TransparentForMouseEvents, true);
                } else
                {
                    sliderWidget->setAttribute(Qt::WA_TransparentForMouseEvents, false);
                    textWidget->setAttribute(Qt::WA_TransparentForMouseEvents, false);
                }

                RangeList rList = pinRangeMap->value(IO);
                updateSliderRange((QSlider*) sliderWidget, &rList);
            }

            itemCombo->blockSignals(false);
        }
    }
}

void GUI_8AIO_16DIO_COMM::setConTypes(QStringList connTypes, QList<char> mapValues)
{
    if (connTypes.length() != mapValues.length()) return;

    QMap<QString, uint8_t>* pinMap = controlMap.value(JSON_REMOTE_CONN);
    for (int i = 0; i < connTypes.length(); i++)
    {
        pinMap->insert(connTypes[i], mapValues[i]);
    }

    ui->ConnTypeCombo->clear();
    ui->ConnTypeCombo->addItems(connTypes);

    on_ConnTypeCombo_currentIndexChanged(ui->ConnTypeCombo->currentIndex());
}

bool GUI_8AIO_16DIO_COMM::getPinTypeInfo(uint8_t pinType, PinTypeInfo *infoPtr)
{
    if (!GUI_PIN_BASE::getPinTypeInfo(pinType, infoPtr))
        return false;

    // Set ui pin type variables
    switch (pinType)
    {
        case JSON_AIO:
            infoPtr->grid = ui->AIO_Grid;
            return true;
        case JSON_DIO:
            infoPtr->grid = ui->DIO_Grid;
            return true;
        default:
            return false;
    }
}
