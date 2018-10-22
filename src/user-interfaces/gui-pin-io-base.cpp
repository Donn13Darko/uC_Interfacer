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
    // Set GUI Type
    gui_type = GUI_TYPE_IO;
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
    QMap<uint8_t, RangeList*>* rmap;
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
        getItemWidget((QWidget**) &textValue, pInfo->grid, rowNum, colNum+textValuePos);

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
    // Check known major key
    if (recvData.at(s1_major_key_loc) != (char) MAJOR_KEY_READ_RESPONSE)
        return;

    // Set values with minor key
    setValues(recvData.at(s1_minor_key_loc), recvData.right(recvData.at(s1_num_s2_bytes_loc)));
}

void GUI_PIN_BASE::inputsChanged(PinTypeInfo *pInfo, uint8_t colOffset)
{
    // Get info of set button clicked
    QObject *caller = sender();
    int index, col, row, rowSp, colSp;
    index = pInfo->grid->indexOf((QWidget*) caller);
    pInfo->grid->getItemPosition(index, &row, &col, &rowSp, &colSp);
    col = col - colOffset;

    // Get all pin info
    QLayoutItem *itemLabel = pInfo->grid->itemAtPosition(row, col+labelPos);
    QLayoutItem *itemCombo = pInfo->grid->itemAtPosition(row, col+comboPos);
    QLayoutItem *itemSliderValue = pInfo->grid->itemAtPosition(row, col+slideValuePos);
    QLayoutItem *itemTextValue = pInfo->grid->itemAtPosition(row, col+textValuePos);
    if (!(itemLabel && itemCombo && itemSliderValue && itemTextValue)) return;

    // Get mapping info
    QMap<QString, uint8_t>* pinMap = controlMap.value(pInfo->pinType);
    QMap<uint8_t, RangeList*>* pinRangeMap = rangeMap.value(pInfo->pinType);
    QList<uint8_t>* pinDisabledSet = disabledValueSet.value(pInfo->pinType);
    if (!pinMap || !pinRangeMap || !pinDisabledSet) return;

    // Find widgets of interest
    QLabel *label = (QLabel*) itemLabel->widget();
    QComboBox *combo = (QComboBox*) itemCombo->widget();
    QSlider *sliderValue = (QSlider*) itemSliderValue->widget();
    QLineEdit *textValue = (QLineEdit*) itemTextValue->widget();
    if (!(label && combo && sliderValue && textValue)) return;

    // Set Pin Num
    QString pinNum = QString::number(label->text().toInt());

    // Clarify IO selection
    uint8_t IO = pinMap->value(combo->currentText());

    // Get range list for use in next sections
    RangeList* rList = pinRangeMap->value(IO);

    // Set IO if combo changed
    if (caller == combo)
    {
        // Enable/Disable pins if selection changed
        if (pinDisabledSet->contains(IO))
        {
            sliderValue->setAttribute(Qt::WA_TransparentForMouseEvents, true);
            textValue->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        } else
        {
            sliderValue->setAttribute(Qt::WA_TransparentForMouseEvents, false);
            textValue->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        }

        // Reset slider range if selection changed
        int prevPos = sliderValue->sliderPosition();
        updateSliderRange(sliderValue, rList);

        int curPos = sliderValue->sliderPosition();
        if (prevPos == curPos) emit sliderValue->valueChanged(curPos);

        return;
    }

    QString VAL;
    if (caller == sliderValue)
    {
        // Update values if slider moved
        int newVal = sliderValue->value();
        float tempVAL = (float) newVal / rList->div;

        if (pInfo->pinType != MINOR_KEY_IO_AIO) tempVAL = qRound(tempVAL);

        VAL = QString::number(tempVAL);
        textValue->setText(VAL);
    } else if (caller == textValue)
    {
        // Update values if text box changed
        VAL = textValue->text();
        float tempVAL = rList->div * VAL.toFloat();

        if (pInfo->pinType != MINOR_KEY_IO_AIO) tempVAL = qRound(tempVAL);

        sliderValue->setSliderPosition(tempVAL);
        VAL = QString::number(tempVAL);
        return;
    } else
    {
        // No change or error
        return;
    }

    // Send major/minor keys & data to uC
    uint16_t v = (uint16_t) VAL.toInt();
    send_chunk({
                   gui_type,                    // Major Key
                   pInfo->minorKey,             // Minor Key
               },
               {
                   (uint8_t) pinNum.toInt(),    // Pin Num
                   IO,                          // Combo setting
                   (uint8_t) ((v >> 8) & 0xFF), // Value High
                   (uint8_t) (v & 0xFF)         // Value Low
               });
}

void GUI_PIN_BASE::updateSliderRange(QSlider *slider, RangeList *rList)
{
    slider->setMinimum(rList->min);
    slider->setMaximum(rList->max);
    slider->setTickInterval(rList->step);
    slider->setSingleStep(rList->step);
    slider->setPageStep(rList->step);

    if (0 < rList->min) slider->setSliderPosition(rList->min);
    else if (rList->max < 0) slider->setSliderPosition(rList->max);
    else slider->setSliderPosition(0);
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
        if (getItemWidget(&item, pInfo->grid, rowNum, colNum+labelPos))
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

bool GUI_PIN_BASE::getItemWidget(QWidget** itemWidget, QGridLayout *grid, uint8_t row, uint8_t col)
{
    *itemWidget = 0;
    QLayoutItem *item = grid->itemAtPosition(row, col);
    if (item) *itemWidget = item->widget();
    return (*itemWidget != 0);
}

void GUI_PIN_BASE::getPinLocation(uint8_t *row, uint8_t *col, PinTypeInfo *pInfo, uint8_t pin)
{
    *row = pin / pInfo->cols;
    *col = pInfo->numButtons * (pin % pInfo->cols);
}

void GUI_PIN_BASE::setCombos(PinTypeInfo *pInfo, QList<QString> combos)
{
    // Retrieve & verify pin type maps
    QMap<QString, uint8_t>* pinMap = controlMap.value(pInfo->pinType);
    QMap<uint8_t, RangeList*>* pinRangeMap = rangeMap.value(pInfo->pinType);
    QList<uint8_t>* pinDisabledSet = disabledValueSet.value(pInfo->pinType);
    if (!pinMap || !pinRangeMap || !pinDisabledSet) return;

    // Setup arrays & constructs for use in the loop
    uint8_t IO;
    uint8_t rowNum, colNum;
    QList<uint8_t> pinNums;
    QList<QString> listValues;
    QStringList comboStr_split;

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
            if (getItemWidget((QWidget**) &itemCombo, pInfo->grid, rowNum, colNum+comboPos))
            {
                itemCombo->blockSignals(true);

                itemCombo->clear();
                itemCombo->addItems(listValues);

                IO = pinMap->value(itemCombo->currentText());
                if (getItemWidget(&sliderWidget, pInfo->grid, rowNum, colNum+slideValuePos)
                        && getItemWidget(&textWidget, pInfo->grid, rowNum, colNum+textValuePos))
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

                    RangeList* rList = pinRangeMap->value(IO);
                    updateSliderRange((QSlider*) sliderWidget, rList);
                }

                itemCombo->blockSignals(false);
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

    QMap<QString, uint8_t>* pinMap = controlMap.value(pinType);
    QList<uint8_t>* pinDisabledSet = disabledValueSet.value(pinType);
    foreach (QString i, pinSetDisabled)
    {
        pinDisabledSet->append(pinMap->value(i));
    }
}

bool GUI_PIN_BASE::isDataRequest(uint8_t minorKey)
{
    switch (minorKey)
    {
        case MINOR_KEY_IO_AIO_READ:
        case MINOR_KEY_IO_DIO_READ:
            return true;
        default:
            return GUI_BASE::isDataRequest(minorKey);
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
        case MINOR_KEY_IO_AIO_READ:
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
        case MINOR_KEY_IO_DIO_READ:
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

RangeList* GUI_PIN_BASE::makeRangeList(QString rangeInfo)
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
    QMap<QString, uint8_t>* pinMap = controlMap.value(pinType);
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

    QMap<QString, uint8_t>* pinMap = controlMap.value(pinType);
    QMap<uint8_t, RangeList*>* pinRangeMap = rangeMap.value(pinType);
    for (uint8_t i = 0; i < keys.length(); i++)
    {
        pinRangeMap->insert(pinMap->value(keys[i]), values[i]);
    }
}
