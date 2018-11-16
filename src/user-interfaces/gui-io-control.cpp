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

#include "gui-io-control.h"
#include "ui_gui-io-control.h"

#include <QDateTime>

GUI_IO_CONTROL::GUI_IO_CONTROL(QWidget *parent) :
    GUI_BASE(parent),
    ui(new Ui::GUI_IO_CONTROL)
{
    // Setup ui
    ui->setupUi(this);

    // Set GUI Type & Default Name
    gui_key = MAJOR_KEY_IO;
    gui_name = "IO";

    // Set buttons
    ui->StartUpdater_Button->setText("Start");
    ui->StartLog_Button->setText("Start Log");

    // Call initializers
    initialize();
    setupUpdaters();

    // Reset GUI
    reset_gui();
}

GUI_IO_CONTROL::~GUI_IO_CONTROL()
{
    // Stop loggers & updaters
    on_StopLog_Button_clicked();
    on_StopUpdater_Button_clicked();

    // Destroy and created charts
    emit destroy_charts();

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

    // Clear all maps
    clear_all_maps();

    // Setup pintypes variable
    PinTypeInfo pInfo;

    // Add DIO controls
    if (!getPinTypeInfo(MINOR_KEY_IO_DIO, &pInfo)) return;
    addPinType(pInfo.pinType);
    addComboSettings(&pInfo, configMap->value("dio_combo_settings").toStringList());
    setPinCombos(&pInfo, configMap->value("dio_pin_settings").toStringList());
    update_pin_grid(&pInfo);

    // Add AIO controls
    if (!getPinTypeInfo(MINOR_KEY_IO_AIO, &pInfo)) return;
    addPinType(pInfo.pinType);
    addComboSettings(&pInfo, configMap->value("aio_combo_settings").toStringList());
    setPinCombos(&pInfo, configMap->value("aio_pin_settings").toStringList());
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

    // Reset after parse (forces updates)
    reset_gui();
}

bool GUI_IO_CONTROL::waitForDevice(uint8_t minorKey)
{
    switch (minorKey)
    {
        case MINOR_KEY_IO_REMOTE_CONN_READ:
            return true;
        case MINOR_KEY_IO_AIO_READ:
        case MINOR_KEY_IO_AIO_READ_ALL:
            return aio_read_requested;
        case MINOR_KEY_IO_DIO_READ:
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

void GUI_IO_CONTROL::recordPinValues(PinTypeInfo *pInfo)
{
    if (logStream == nullptr) return;

    *logStream << pInfo->pinType;

    QLineEdit *textValue;
    foreach (QHBoxLayout* pin, *pinMap.value(pInfo->pinType))
    {
        // Make sure pin valid
        if (!pin) continue;

        // Add comma
        *logStream << ",";

        // Get text value
        textValue = (QLineEdit*) pin->itemAt(io_line_edit_pos)->widget();

        // Append pin value
        *logStream << textValue->text();
    }
    // Add new line
    *logStream << "\n";

    // Flush data to file
    logStream->flush();
}

void GUI_IO_CONTROL::receive_gui(QByteArray recvData)
{
    // Ignore commands not meant for this GUI
    if (recvData.at(s1_major_key_loc) != (char) gui_key)
        return;

    uint8_t minor_key = recvData.at(s1_minor_key_loc);
    switch (minor_key)
    {
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
                    pinValues.append(GUI_HELPER::uint32_to_byteArray(pinValue).right(2));
                }

                // Send values back
                emit transmit_chunk(gui_key, minor_key, pinValues);

                // Send device ready
                emit transmit_chunk(MAJOR_KEY_DEV_READY, 0);

                // Exit out of parse
                break;
            }

            // Otherwise, packet is a response so clear data request
            // and fall through to set values
            if (minor_key == MINOR_KEY_IO_AIO_READ_ALL) aio_read_requested = false;
            else if (minor_key == MINOR_KEY_IO_DIO_READ_ALL) dio_read_requested = false;
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
    }
}

void GUI_IO_CONTROL::DIO_ComboValueChanged()
{
    // Propogate updates
    QByteArray data;
    inputsChanged(MINOR_KEY_IO_DIO, sender(), io_combo_pos, &data);

    // Send update
    emit transmit_chunk(gui_key, MINOR_KEY_IO_DIO_SET, data);
}

void GUI_IO_CONTROL::DIO_SliderValueChanged()
{
    // Set message for clicked button
    QByteArray data;
    inputsChanged(MINOR_KEY_IO_DIO, sender(), io_slider_pos, &data);

    // Remove combo setting (unneeded)
    data.remove(s2_io_combo_loc, 1);

    // Send update
    emit transmit_chunk(gui_key, MINOR_KEY_IO_DIO_WRITE, data);
}

void GUI_IO_CONTROL::DIO_LineEditValueChanged()
{
    // Set message for clicked button
    QByteArray data;
    inputsChanged(MINOR_KEY_IO_DIO, sender(), io_line_edit_pos, &data);

    // Remove combo setting (unneeded)
    data.remove(s2_io_combo_loc, 1);

    // Send update
    emit transmit_chunk(gui_key, MINOR_KEY_IO_DIO_WRITE, data);
}

void GUI_IO_CONTROL::AIO_ComboValueChanged()
{
    // Send message for edited button
    QByteArray data;
    inputsChanged(MINOR_KEY_IO_AIO, sender(), io_combo_pos, &data);

    // Send update
    emit transmit_chunk(gui_key, MINOR_KEY_IO_AIO_SET, data);
}

void GUI_IO_CONTROL::AIO_SliderValueChanged()
{
    // Send message for edited button
    QByteArray data;
    inputsChanged(MINOR_KEY_IO_AIO, sender(), io_slider_pos, &data);

    // Remove combo setting (unneeded)
    data.remove(s2_io_combo_loc, 1);

    // Send update
    emit transmit_chunk(gui_key, MINOR_KEY_IO_AIO_WRITE, data);
}

void GUI_IO_CONTROL::AIO_LineEditValueChanged()
{
    // Send message for edited button
    QByteArray data;
    inputsChanged(MINOR_KEY_IO_AIO, sender(), io_line_edit_pos, &data);

    // Remove combo setting (unneeded)
    data.remove(s2_io_combo_loc, 1);

    // Send update
    emit transmit_chunk(gui_key, MINOR_KEY_IO_AIO_WRITE, data);
}

void GUI_IO_CONTROL::updateValues()
{
    // Get caller to find request type
    QTimer *caller = (QTimer*) sender();

    // Find request type & set read_requested
    uint8_t requestType;
    if (caller == &DIO_READ)
    {
        dio_read_requested = true;
        requestType = MINOR_KEY_IO_DIO_READ_ALL;
    } else if (caller == &AIO_READ)
    {
        aio_read_requested = true;
        requestType = MINOR_KEY_IO_AIO_READ_ALL;
    } else
    {
        return;
    }

    // Emit request for data
    emit transmit_chunk(gui_key, requestType);
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

    DIO_READ.start((int) (GUI_HELPER::S2MS * ui->DIO_UR_LineEdit->text().toFloat()));
    AIO_READ.start((int) (GUI_HELPER::S2MS * ui->AIO_UR_LineEdit->text().toFloat()));
}

void GUI_IO_CONTROL::on_StopUpdater_Button_clicked()
{
    ui->StartUpdater_Button->setText("Start");

    DIO_READ.stop();
    AIO_READ.stop();
}

void GUI_IO_CONTROL::on_LogSaveLocSelect_Button_clicked()
{
    // Get file
    QString filePath;
    if (GUI_HELPER::getSaveFilePath(&filePath))
        ui->LogSaveLoc_LineEdit->setText(filePath);
}

void GUI_IO_CONTROL::on_StartLog_Button_clicked()
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

void GUI_IO_CONTROL::on_ConnSend_Button_clicked()
{
    QByteArray msg;
    msg.append(ui->ConnMsg_PlainText->toPlainText());

    emit transmit_chunk(gui_key, MINOR_KEY_IO_REMOTE_CONN_SEND, msg);
}

void GUI_IO_CONTROL::on_ConnClearRecv_Button_clicked()
{
    ui->ConnRecv_PlainText->clear();
    rcvd_formatted.resize(0);
}

void GUI_IO_CONTROL::on_ConnSaveRecv_Button_clicked()
{
    save_rcvd_formatted();
}

void GUI_IO_CONTROL::on_ConnType_Combo_currentIndexChanged(int)
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

void GUI_IO_CONTROL::on_CreatePlots_Button_clicked()
{
    // Setup graph
    GUI_CHART_VIEW *chart_view = new GUI_CHART_VIEW();
    chart_view->set_data_list({});
    chart_view->setAttribute(Qt::WA_DeleteOnClose);
    chart_view->setModal(false);

    // Connect destroy signal to close signal
    connect(this, SIGNAL(destroy_charts()),
            chart_view, SLOT(close()),
            Qt::QueuedConnection);

    // Show the chart view
    chart_view->show();
}

void GUI_IO_CONTROL::initialize()
{
    // Set class pin variables
    bytesPerPin = 2;

    // Setup AIO info
    AIO_Grid = new QGridLayout();
    ui->AIOVLayout->insertLayout(1, AIO_Grid);
    num_AIOcols = 1;
    num_AIOrows = 0 / num_AIOcols;

    // Setup DIO info
    DIO_Grid = new QGridLayout();
    ui->DIOVLayout->insertLayout(1, DIO_Grid);
    num_DIOcols = 2;
    num_DIOrows = 0 / num_DIOcols;

    // Set log file parameters
    logFile = NULL;
    logStream = NULL;
    logIsRecording = false;
}

void GUI_IO_CONTROL::setupUpdaters()
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
    QLayout *pin;
    if (!getWidgetLayout(pInfo.pinType, (QWidget*) caller, &pin)) return;

    // Get widgets
    QLabel *label = (QLabel*) pin->itemAt(io_label_pos)->widget();
    QComboBox *comboBox = (QComboBox*) pin->itemAt(io_combo_pos)->widget();
    QSlider *sliderValue = (QSlider*) pin->itemAt(io_slider_pos)->widget();
    QLineEdit *textValue = (QLineEdit*) pin->itemAt(io_line_edit_pos)->widget();
    if (!(label && comboBox && sliderValue && textValue)) return;

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
            textValue->setAttribute(Qt::WA_TransparentForMouseEvents, disableClicks);

            // Update slider range (will reset slider value)
            updateSliderRange(sliderValue, rList);

            // Fall through to next case to update info
        }
        case io_slider_pos:
        {
            // Update values based on slider
            newVAL = ((float) sliderValue->value()) / rList->div;
            if (pInfo.pinType == MINOR_KEY_IO_DIO) newVAL = qRound(newVAL);

            // Update text value
            prev_block_status = textValue->blockSignals(true);
            textValue->setText(QString::number(newVAL));
            textValue->blockSignals(prev_block_status);
            break;
        }
        case io_line_edit_pos:
        {
            // Update values if text box changed
            newVAL = rList->div * textValue->text().toFloat();
            if (pInfo.pinType == MINOR_KEY_IO_DIO) newVAL = qRound(newVAL);

            // Update slider value
            prev_block_status = sliderValue->blockSignals(true);
            sliderValue->setSliderPosition(newVAL);
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
        // Build pin data array
        uint16_t v = (uint16_t) QString::number(newVAL).toInt();
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
    if (0 < rList->min) slider->setSliderPosition(rList->min);
    else if (rList->max < 0) slider->setSliderPosition(rList->max);
    else slider->setSliderPosition(0);

    // Enable signals
    slider->blockSignals(prev_block_status);
}

void GUI_IO_CONTROL::setPinAttribute(PinTypeInfo *pInfo, uint8_t pinNum, Qt::WidgetAttribute attribute, bool on)
{
    // Go through each element and set attributes
    QLayout *itemLayout;
    if (getPinLayout(pInfo->pinType, pinNum, &itemLayout))
    {
        int num_members = itemLayout->count();
        for (int i = 0; i < num_members; i++)
        {
            itemLayout->itemAt(i)->widget()->setAttribute(attribute, on);
        }
    }
}

bool GUI_IO_CONTROL::getPinLayout(uint8_t pinType, uint8_t pin_num, QLayout **itemLayout)
{
    // Set itemLayout to nullptr
    *itemLayout = nullptr;

    // Get pins
    QList<QHBoxLayout*> *pins = pinMap.value(pinType);

    // Verify pins
    if (!pins) return false;

    // Search pins for item
    QHBoxLayout *pin;
    int num_pins = pins->length();
    int curr_pin_num;
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

bool GUI_IO_CONTROL::getWidgetLayout(uint8_t pinType, QWidget *item, QLayout **itemLayout)
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

    // Setup arrays & constructs for use in the loop
    uint8_t IO;
    QList<uint8_t> pinNums;
    QList<QString> listValues;
    QStringList comboStr_split;
    bool prev_block_status;

    // Setup widget holders
    QComboBox *itemCombo;
    QWidget *sliderWidget, *textWidget;
    QLayout *pin_layout;
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
            if (pinNum.contains('-'))
            {
                // Split range list
                QStringList numStr_split = pinNum.split('-');
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
            // the map & list at the correct location (by number)
            if (!getPinLayout(pInfo->pinType, pin, &pin_layout))
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

            // Get all the combo
            itemCombo = (QComboBox*) pin_layout->itemAt(io_combo_pos)->widget();
            sliderWidget = pin_layout->itemAt(io_slider_pos)->widget();
            textWidget = pin_layout->itemAt(io_line_edit_pos)->widget();

            // Block signals to combo
            prev_block_status = itemCombo->blockSignals(true);

            // Update combo & get map information
            itemCombo->clear();
            itemCombo->addItems(listValues);
            IO = pinControlMap->value(itemCombo->currentText());

            // Enable or disable pin group
            if (pinDisabledSet->contains(IO))
            {
                sliderWidget->setAttribute(Qt::WA_TransparentForMouseEvents, true);
                textWidget->setAttribute(Qt::WA_TransparentForMouseEvents, true);
            } else
            {
                sliderWidget->setAttribute(Qt::WA_TransparentForMouseEvents, false);
                textWidget->setAttribute(Qt::WA_TransparentForMouseEvents, false);
            }

            // Update slider range
            RangeList *rList = pinRangeMap->value(IO);
            updateSliderRange((QSlider*) sliderWidget, rList);

            // Enble signals to combo
            itemCombo->blockSignals(prev_block_status);
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
    // Check edge case
    int num_pins = pins->length();
    if (num_pins == 0) pins->append(pin);

    // Get insert pin number
    int insert_pin_num = ((QLabel*) pin->itemAt(io_label_pos)->widget())->text().toInt();

    // Find insertion position (assume pins inserted in order until now)
    int curr_pin_num;
    for (int i = (num_pins-1); 0 <= i; i--)
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
}

void GUI_IO_CONTROL::addComboSettings(PinTypeInfo *pInfo, QList<QString> newSettings)
{
    // Setup lists to hold setting
    QList<QString> settingValues;
    QList<QString> pinCombos = {}, pinSetDisabled = {};
    QList<RangeList*> pinRanges = {};

    // Get each setting
    foreach (QString i, newSettings)
    {
        // Split settings string into values [name,setEnabled,rangeList]
        settingValues = i.split(',');

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
    int key_num = pinControlMap->keys().length();

    // Add each string to pinMap
    foreach (QString i, keys)
    {
        // Verify its a new value
        if (!pinControlMap->contains(i))
        {
            pinControlMap->insert(i, key_num);
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
    if (!pinControlMap || !pinRangeMap || !pinDisabledSet) return;

    // Allocate loop variables
    QLayout *pin;
    QComboBox *comboBox;
    QSlider *sliderValue;
    QLineEdit *textValue;
    uint16_t value;
    uint8_t comboVal, divisor;
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
                // Find pin layout on GUI
                if (!getPinLayout(pInfo.pinType, pin_num, &pin)) return;

                // Get all the widgets
                comboBox = (QComboBox*) pin->itemAt(io_combo_pos)->widget();

                // Get combo value
                comboVal = pinControlMap->value(comboBox->currentText());

                // Only update value if not controllable
                if (pinDisabledSet->contains(comboVal))
                {
                    // Get value from list
                    // Value is big endian
                    value = 0;
                    for (j = 0; j < bytesPerPin; j++)
                    {
                        value = (value << 8) | ((uchar) values.at(i++));
                    }

                    // Get other widgets
                    sliderValue = (QSlider*) pin->itemAt(io_slider_pos)->widget();
                    textValue = (QLineEdit*) pin->itemAt(io_line_edit_pos)->widget();

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
                } else
                {
                    // Skip value in list
                    i += bytesPerPin;
                }

                // Increment pin
                pin_num += 1;
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
            QString combo_text = pinControlMap->key(values.at(s2_io_combo_loc), "");
            if (combo_text.isEmpty()) return;

            // Find pin layout on GUI
            if (!getPinLayout(pInfo.pinType, pin_num, &pin)) return;

            // Get all the widgets
            comboBox = (QComboBox*) pin->itemAt(io_combo_pos)->widget();
            sliderValue = (QSlider*) pin->itemAt(io_slider_pos)->widget();
            textValue = (QLineEdit*) pin->itemAt(io_line_edit_pos)->widget();

            // Block combo signals before setting
            prev_block_combo = comboBox->blockSignals(true);
            prev_block_slider = sliderValue->blockSignals(true);
            prev_block_text = textValue->blockSignals(true);

            // Set combo to correct position
            comboBox->setCurrentText(combo_text);

            // Get combo value
            comboVal = pinControlMap->value(comboBox->currentText());

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

            // Find pin layout on GUI
            if (!getPinLayout(pInfo.pinType, pin_num, &pin)) return;

            // Get all the widgets
            comboBox = (QComboBox*) pin->itemAt(io_combo_pos)->widget();
            sliderValue = (QSlider*) pin->itemAt(io_slider_pos)->widget();
            textValue = (QLineEdit*) pin->itemAt(io_line_edit_pos)->widget();

            // Block combo signals before setting
            prev_block_combo = comboBox->blockSignals(true);
            prev_block_slider = sliderValue->blockSignals(true);
            prev_block_text = textValue->blockSignals(true);

            // Get combo value
            comboVal = pinControlMap->value(comboBox->currentText());

            // Set slider and divisor
            divisor = pinRangeMap->value(comboVal)->div;
            sliderValue->setSliderPosition(value);
            textValue->setText(QString::number(((float) value) / divisor));

            // Unblock signals now that they are set
            comboBox->blockSignals(prev_block_combo);
            sliderValue->blockSignals(prev_block_slider);
            textValue->blockSignals(prev_block_text);

            // Break out after writing new value
            break;
        }
    }
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
            infoPtr->rows = num_AIOrows;
            infoPtr->grid = AIO_Grid;
            infoPtr->pinType = MINOR_KEY_IO_AIO;
            return true;
        case MINOR_KEY_IO_DIO:
        case MINOR_KEY_IO_DIO_SET:
        case MINOR_KEY_IO_DIO_WRITE:
        case MINOR_KEY_IO_DIO_READ:
        case MINOR_KEY_IO_DIO_READ_ALL:
            infoPtr->cols = num_DIOcols;
            infoPtr->rows = num_DIOrows;
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
    QStringList ril = rangeInfo.split('-');
    if (ril.length() != 4) return new EMPTY_RANGE;

    return new RangeList({
                             .min=ril.at(0).toInt(),
                             .max=ril.at(1).toInt(),
                             .step=ril.at(2).toInt(),
                             .div=ril.at(3).toFloat()
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
    // Clear pin map
    foreach (uint8_t pinType, pinMap.keys())
    {
        foreach (QHBoxLayout *pin, *pinMap.value(pinType))
        {
            destroy_pin(pin);
        }
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
