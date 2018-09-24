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

#include "GUI_PIN_BASE.h"

GUI_PIN_BASE::GUI_PIN_BASE(QWidget *parent) :
    GUI_BASE(parent)
{
}

GUI_PIN_BASE::~GUI_PIN_BASE()
{
}

void GUI_PIN_BASE::setRangeDefaults(RangeList DIO_range, RangeList AIO_range)
{
    DIO_rangeDefault = DIO_range;
    AIO_rangeDefault = AIO_range;
}

RangeList GUI_PIN_BASE::getDIORangeDefault()
{
    return DIO_rangeDefault;
}

RangeList GUI_PIN_BASE::getAIORangeDefault()
{
    return AIO_rangeDefault;
}

void GUI_PIN_BASE::inputsChanged(PinTypeInfo *pInfo, int colOffset)
{
    // Get info of set button clicked
    QObject *caller = sender();
    int in, col, row, rowSp, colSp;
    in = pInfo->grid->indexOf((QWidget*) caller);
    pInfo->grid->getItemPosition(in, &row, &col, &rowSp, &colSp);
    col = col - colOffset;

    // Get all pin info
    QLayoutItem *itemLabel = pInfo->grid->itemAtPosition(row, col+labelPos);
    QLayoutItem *itemCombo = pInfo->grid->itemAtPosition(row, col+comboPos);
    QLayoutItem *itemSliderValue = pInfo->grid->itemAtPosition(row, col+slideValuePos);
    QLayoutItem *itemTextValue = pInfo->grid->itemAtPosition(row, col+textValuePos);

    // Compile & send request for uC
    if (itemLabel && itemCombo && itemSliderValue && itemTextValue)
    {
        QLabel *label = (QLabel*) itemLabel->widget();
        QComboBox *combo = (QComboBox*) itemCombo->widget();
        QSlider *sliderValue = (QSlider*) itemSliderValue->widget();
        QLineEdit *textValue = (QLineEdit*) itemTextValue->widget();
        if (label && combo && sliderValue && textValue)
        {
            // Set Pin Num
            QString pinNum = QString::number(label->text().toInt());

            // Clarify IO selection
            uint8_t IO = controlMap.value(combo->currentText());
            RangeList rList = rangeMap.value(IO, pInfo->rangeDefault);

            // Set IO if combo changed
            if (caller == combo)
            {
                // Enable/Disable pins if selection changed
                if (disabledValueSet.value(pInfo->pinType).contains(IO))
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
                updateSliderRange(sliderValue, &rList);

                int curPos = sliderValue->sliderPosition();
                if (prevPos == curPos) emit sliderValue->valueChanged(curPos);

                return;
            }

            QString VAL;
            if (caller == sliderValue)
            {
                // Update values if slider moved
                int newVal = sliderValue->value();
                float tempVAL = (float) newVal / rList.div;

                if (pInfo->pinType == JSON_AIO) tempVAL = qRound(tempVAL);

                VAL = QString::number(tempVAL);
                textValue->setText(VAL);
            } else if (caller == textValue)
            {
                // Update values if text box changed
                VAL = textValue->text();
                float tempVAL = rList.div * VAL.toFloat();

                if (pInfo->pinType != JSON_AIO) tempVAL = qRound(tempVAL);

                sliderValue->setSliderPosition(tempVAL);
                VAL = QString::number(tempVAL);
                return;
            } else
            {
                // No change or error
                return;
            }

            // Send request to uC
            uint16_t v = (uint16_t) VAL.toInt();
            send({
                     pInfo->pinType,
                     (uint8_t) pinNum.toInt(),
                     IO,
                     (uint8_t) ((v >> 8) & 0xFF),
                     (uint8_t) (v & 0xFF)
                 });
        }
    }
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

void GUI_PIN_BASE::setPinAttribute(PinTypeInfo *pInfo, int pinNum, Qt::WidgetAttribute attribute, bool on)
{
    // Find row & column of desired buttons
    int rowNum, colNum;
    getPinLocation(&rowNum, &colNum, pInfo, pinNum);

    // Go through each element and set attributes
    QWidget *itemWidget;
    for (int i = 0; i < pInfo->numButtons; i++)
    {
        if (getItemWidget(&itemWidget, pInfo->grid, rowNum, colNum+i))
        {
            itemWidget->setAttribute(attribute, on);
        }
    }
}

bool GUI_PIN_BASE::getItemWidget(QWidget** itemWidget, QGridLayout *grid, int row, int col)
{
    *itemWidget = 0;
    QLayoutItem *item = grid->itemAtPosition(row, col);
    if (item) *itemWidget = item->widget();
    return (*itemWidget != 0);
}

void GUI_PIN_BASE::getPinLocation(int *row, int *col, PinTypeInfo *pInfo, int pin)
{
    *row = pin / pInfo->cols;
    *col = pInfo->numButtons * (pin % pInfo->cols);
}

bool GUI_PIN_BASE::getPinTypeInfo(uint8_t pinType, PinTypeInfo *infoPtr)
{
    infoPtr->pinType = pinType;

    // Set pin type variables
    switch (pinType)
    {
        case JSON_AIO:
            infoPtr->numButtons = num_AIObuttons;
            infoPtr->numPins = num_AIOpins_GUI;
            infoPtr->cols = num_AIOcols;
            infoPtr->rows = num_AIOrows;
            infoPtr->rangeDefault = AIO_rangeDefault;
            return true;
        case JSON_DIO:
            infoPtr->numButtons = num_DIObuttons;
            infoPtr->numPins = num_DIOpins_GUI;
            infoPtr->cols = num_DIOcols;
            infoPtr->rows = num_DIOrows;
            infoPtr->rangeDefault = DIO_rangeDefault;
            return true;
        default:
            *infoPtr = EMPTY_PINTYPEINFO;
            return false;
    }
}
