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

#include "gui-pin-io-base.h"

GUI_PIN_BASE::GUI_PIN_BASE(QWidget *parent) :
    GUI_BASE(parent)
{
    // Set GUI Type & Default Name
    gui_key = MAJOR_KEY_IO;
    gui_name = "IO";
}

GUI_PIN_BASE::~GUI_PIN_BASE()
{
    // Clear control map
    foreach (uint8_t pinType, controlMap.keys())
    {
        delete controlMap.value(pinType);
    }

    // Clear disabled value set map
    foreach (uint8_t pinType, disabledValueSet.keys())
    {
        delete disabledValueSet.value(pinType);
    }

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
}

void GUI_PIN_BASE::parseConfigMap(QMap<QString, QVariant> *configMap)
{
    // Pass to parent for additional parsing
    GUI_BASE::parseConfigMap(configMap);
}

bool GUI_PIN_BASE::waitForDevice(uint8_t minorKey)
{
    switch (minorKey)
    {
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

void GUI_PIN_BASE::reset_gui()
{
    // Reset request variables
    dio_read_requested = false;
    aio_read_requested = false;

    // Reset base
    GUI_BASE::reset_gui();
}

void GUI_PIN_BASE::recordPinValues(PinTypeInfo *pInfo)
{
    if (logStream == nullptr) return;

    *logStream << pInfo->pinType;

    QLineEdit *textValue;
    uint8_t rowNum, colNum;
    for (uint8_t i = 0; i < pInfo->numPins_DEV; i++)
    {
        // Add comma
        *logStream << ",";

        // Get pin location and line edit widget
        getPinLocation(&rowNum, &colNum, pInfo, i);
        getItemWidget((QWidget**) &textValue, pInfo->grid, rowNum, colNum+io_line_edit_pos);

        // Append pin value
        *logStream << textValue->text();
    }
    // Add new line
    *logStream << "\n";

    // Flush data to file
    logStream->flush();
}

void GUI_PIN_BASE::receive_gui(QByteArray recvData)
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
                uint8_t rowNum, colNum;

                // Get pin info
                PinTypeInfo pInfo;
                getPinTypeInfo(minor_key, &pInfo);

                // Parse all pin values
                for (uint8_t i = 0; i < pInfo.numPins_DEV; i++)
                {
                    // Get pin location and slider widget
                    getPinLocation(&rowNum, &colNum, &pInfo, i);
                    getItemWidget((QWidget**) &sliderValue, pInfo.grid, rowNum, colNum+io_slider_pos);

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

void GUI_PIN_BASE::inputsChanged(PinTypeInfo *pInfo, QObject *caller, uint8_t io_pos, QByteArray *data)
{
    // Get mapping info
    QMap<QString, uint8_t> *pinMap = controlMap.value(pInfo->pinType);
    QMap<uint8_t, RangeList*> *pinRangeMap = rangeMap.value(pInfo->pinType);
    QList<uint8_t> *pinDisabledSet = disabledValueSet.value(pInfo->pinType);
    if (!(pinMap && pinRangeMap && pinDisabledSet)) return;

    // Get info of button clicked
    int index, colNum, rowNum, rowSp, colSp;
    index = pInfo->grid->indexOf((QWidget*) caller);
    pInfo->grid->getItemPosition(index, &rowNum, &colNum, &rowSp, &colSp);
    colNum = colNum - io_pos;

    // Get widgets
    QLabel *label;
    QComboBox *comboBox;
    QSlider *sliderValue;
    QLineEdit *textValue;
    if (!(getItemWidget((QWidget**) &label, pInfo->grid, rowNum, colNum+io_label_pos)
          && getItemWidget((QWidget**) &comboBox, pInfo->grid, rowNum, colNum+io_combo_pos)
          && getItemWidget((QWidget**) &sliderValue, pInfo->grid, rowNum, colNum+io_slider_pos)
          && getItemWidget((QWidget**) &textValue, pInfo->grid, rowNum, colNum+io_line_edit_pos)))
    {
        return;
    }

    // Set Pin Num
    QString pinNum = QString::number(label->text().toInt());

    // Clarify IO selection
    uint8_t io_combo = pinMap->value(comboBox->currentText());

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
            if (pInfo->pinType == MINOR_KEY_IO_DIO) newVAL = qRound(newVAL);

            // If not combo change and number already correctly set,
            // return to prevent a double send
            if ((io_pos != io_combo_pos)
                    && (newVAL == textValue->text().toFloat()))
            {
                return;
            }

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
            if (pInfo->pinType == MINOR_KEY_IO_DIO) newVAL = qRound(newVAL);

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

void GUI_PIN_BASE::updateSliderRange(QSlider *slider, RangeList *rList)
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

void GUI_PIN_BASE::setNumPins(PinTypeInfo *pInfo, uint8_t num_dev_pins, uint8_t start_num)
{
    // Update dev pins
    switch (pInfo->pinType)
    {
        case MINOR_KEY_IO_AIO:
            num_AIOpins_DEV = num_dev_pins;
            break;
        case MINOR_KEY_IO_DIO:
            num_DIOpins_DEV = num_dev_pins;
            break;
    }

    // Enable each button set in the list
    for (uint8_t i = 0; i < pInfo->numPins_GUI; i++)
    {
        setPinAttribute(pInfo, i, Qt::WA_Disabled, false);
    }

    // Disable each button set not in the list
    for (uint8_t i = (pInfo->numPins_GUI-1); num_dev_pins <= i; i--)
    {
        setPinAttribute(pInfo, i, Qt::WA_Disabled, true);
    }

    // Set pin numbering
    setPinNumbers(pInfo, start_num);
}

void GUI_PIN_BASE::setPinAttribute(PinTypeInfo *pInfo, uint8_t pinNum, Qt::WidgetAttribute attribute, bool on)
{
    // Find row & column of desired buttons
    uint8_t rowNum, colNum;
    getPinLocation(&rowNum, &colNum, pInfo, pinNum);

    // Go through each element and set attributes
    QWidget *itemWidget;
    for (uint8_t i = 0; i < pInfo->numButtons; i++)
    {
        if (getItemWidget(&itemWidget, pInfo->grid, rowNum, colNum+i))
        {
            itemWidget->setAttribute(attribute, on);
        }
    }
}

void GUI_PIN_BASE::setPinNumbers(PinTypeInfo *pInfo, uint8_t start_num)
{
    uint8_t rowNum, colNum;
    QWidget *item;
    for (uint8_t i = 0; i < pInfo->numPins_GUI; i++)
    {
        // Find local pin (numbering starts at 0)
        getPinLocation(&rowNum, &colNum, pInfo, i);

        // Set the new text value (start_num+i)
        if (getItemWidget(&item, pInfo->grid, rowNum, colNum+io_label_pos))
        {
            ((QLabel*) item)->setText(QString("%1").arg(start_num+i, 2, 10, QChar('0')));
        }
    }

    // Set global startnum
    switch (pInfo->pinType)
    {
        case MINOR_KEY_IO_AIO:
            num_AIOpins_START = start_num;
            break;
        case MINOR_KEY_IO_DIO:
            num_DIOpins_START = start_num;
            break;
    }
}

bool GUI_PIN_BASE::getItemWidget(QWidget **itemWidget, QGridLayout *grid, uint8_t row, uint8_t col)
{
    *itemWidget = nullptr;
    QLayoutItem *item = grid->itemAtPosition(row, col);
    if (item) *itemWidget = item->widget();
    return (*itemWidget != nullptr);
}

void GUI_PIN_BASE::getPinLocation(uint8_t *row, uint8_t *col, PinTypeInfo *pInfo, uint8_t pin)
{
    *row = pin / pInfo->cols;
    *col = pInfo->numButtons * (pin % pInfo->cols);
}

void GUI_PIN_BASE::setCombos(PinTypeInfo *pInfo, QList<QString> combos)
{
    // Retrieve & verify pin type maps
    QMap<QString, uint8_t> *pinMap = controlMap.value(pInfo->pinType);
    QMap<uint8_t, RangeList*> *pinRangeMap = rangeMap.value(pInfo->pinType);
    QList<uint8_t> *pinDisabledSet = disabledValueSet.value(pInfo->pinType);
    if (!pinMap || !pinRangeMap || !pinDisabledSet) return;

    // Setup arrays & constructs for use in the loop
    uint8_t IO;
    uint8_t rowNum, colNum;
    QList<uint8_t> pinNums;
    QList<QString> listValues;
    QStringList comboStr_split;
    bool prev_block_status;

    // Setup widget holders
    QComboBox *itemCombo;
    QWidget *sliderWidget, *textWidget;

    foreach (QString comboStr, combos)
    {
        // Reset pinNums array
        pinNums.clear();

        // Parse inputs
        comboStr_split = comboStr.split('-');
        if ((comboStr[0] == '-') && (comboStr_split.length() == 2)
                && comboStr_split[0].isEmpty())
        {
            // Apply combo values to all pins
            for (uint8_t pinNum = 0; pinNum < pInfo->numPins_DEV; pinNum++)
            {
                pinNums.append(pinNum);
            }
        } else if (comboStr_split.length() == 2)
        {
            // Apply combo values to supplied pins
            foreach (QString pinNum, comboStr_split[0].split(','))
            {
                pinNums.append(pinNum.toInt());
            }
        } else
        {
            // Malformed so skip
            continue;
        }
        listValues = comboStr_split[1].split(',');

        // Set combo for each pin in the list
        foreach (uint8_t pin, pinNums)
        {
            // Find row & column of desired combo
            getPinLocation(&rowNum, &colNum, pInfo, pin);

            // Replace combo options
            if (getItemWidget((QWidget**) &itemCombo, pInfo->grid, rowNum, colNum+io_combo_pos))
            {
                prev_block_status = itemCombo->blockSignals(true);

                itemCombo->clear();
                itemCombo->addItems(listValues);

                IO = pinMap->value(itemCombo->currentText());
                if (getItemWidget(&sliderWidget, pInfo->grid, rowNum, colNum+io_slider_pos)
                        && getItemWidget(&textWidget, pInfo->grid, rowNum, colNum+io_line_edit_pos))
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

                    RangeList *rList = pinRangeMap->value(IO);
                    updateSliderRange((QSlider*) sliderWidget, rList);
                }

                itemCombo->blockSignals(prev_block_status);
            }
        }
    }
}

void GUI_PIN_BASE::addNewPinSettings(PinTypeInfo *pInfo, QList<QString> newSettings)
{
    QList<QString> settingValues;
    QList<QString> pinCombos = {}, pinSetDisabled = {};
    QList<RangeList*> pinRanges = {};
    foreach (QString i, newSettings)
    {
        // Split settings string into values [name,setEnabled,rangeList]
        settingValues = (i).split(',');

        pinCombos.append(settingValues[0]);

        if (settingValues[1].toLower() == "true")
            pinSetDisabled.append(settingValues[0]);

        pinRanges.append(makeRangeList(settingValues[2]));
    }

    // Add new pinTypes (if relevant)
    uint8_t pinType = pInfo->pinType;
    if (!controlMap.contains(pinType)) controlMap.insert(pinType, new QMap<QString, uint8_t>());
    if (!disabledValueSet.contains(pinType)) disabledValueSet.insert(pinType, new QList<uint8_t>());
    if (!rangeMap.contains(pinType)) rangeMap.insert(pinType, new QMap<uint8_t, RangeList*>());

    addPinControls(pinType, pinCombos);
    addPinRangeMap(pinType, pinCombos, pinRanges);

    QMap<QString, uint8_t> *pinMap = controlMap.value(pinType);
    QList<uint8_t> *pinDisabledSet = disabledValueSet.value(pinType);
    foreach (QString i, pinSetDisabled)
    {
        pinDisabledSet->append(pinMap->value(i));
    }
}

void GUI_PIN_BASE::setValues(uint8_t, QByteArray)
{
    // Default do nothing
}

bool GUI_PIN_BASE::getPinTypeInfo(uint8_t pinType, PinTypeInfo *infoPtr)
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
            infoPtr->numButtons = num_AIObuttons;
            infoPtr->numPins_GUI = num_AIOpins_GUI;
            infoPtr->numPins_DEV = num_AIOpins_DEV;
            infoPtr->numPins_START = num_AIOpins_START;
            infoPtr->cols = num_AIOcols;
            infoPtr->rows = num_AIOrows;
            infoPtr->pinType = MINOR_KEY_IO_AIO;
            return true;
        case MINOR_KEY_IO_DIO:
        case MINOR_KEY_IO_DIO_SET:
        case MINOR_KEY_IO_DIO_WRITE:
        case MINOR_KEY_IO_DIO_READ:
        case MINOR_KEY_IO_DIO_READ_ALL:
            infoPtr->numButtons = num_DIObuttons;
            infoPtr->numPins_GUI = num_DIOpins_GUI;
            infoPtr->numPins_DEV = num_DIOpins_DEV;
            infoPtr->numPins_START = num_DIOpins_START;
            infoPtr->cols = num_DIOcols;
            infoPtr->rows = num_DIOrows;
            infoPtr->pinType = MINOR_KEY_IO_DIO;
            return true;
        default:
            delete infoPtr;
            *infoPtr = EMPTY_PIN_TYPE_INFO;
            return false;
    }
}

RangeList *GUI_PIN_BASE::makeRangeList(QString rangeInfo)
{
    // Split range info string into values
    QStringList ril = rangeInfo.split('-');
    if (ril.length() != 4) return new EMPTY_RANGE;

    return new RangeList({
                             .min=ril[0].toInt(),
                             .max=ril[1].toInt(),
                             .step=ril[2].toInt(),
                             .div=ril[3].toFloat()
                         });
}

void GUI_PIN_BASE::addPinControls(uint8_t pinType, QList<QString> keys)
{
    QMap<QString, uint8_t> *pinMap = controlMap.value(pinType);
    uint8_t key_num = pinMap->keys().length();
    foreach (QString i, keys)
    {
        pinMap->insert(i, key_num);
        key_num += 1;
    }
}

void GUI_PIN_BASE::addPinRangeMap(uint8_t pinType, QList<QString> keys, QList<RangeList*> values)
{
    if (keys.length() != values.length()) return;

    QMap<QString, uint8_t> *pinMap = controlMap.value(pinType);
    QMap<uint8_t, RangeList*> *pinRangeMap = rangeMap.value(pinType);
    for (uint8_t i = 0; i < keys.length(); i++)
    {
        pinRangeMap->insert(pinMap->value(keys[i]), values[i]);
    }
}
