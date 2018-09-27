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

#include "gui-arduino-uno-io.h"
#include "ui_gui-8aio-16dio-comm.h"

#include <QMessageBox>
#include <QTimer>

ArduinoUno_IO::ArduinoUno_IO(QWidget *parent) :
    GUI_8AIO_16DIO_COMM(parent)
{
    // Add new pin settings
    addNewPinSettings(JSON_DIO, {
                          "Input,true,0-1-1-1.0",
                          "Output,false,0-1-1-1.0",
                          "PWM,false,0-100-5-1",
                          "Servo Deg,false,0-360-5-1",
                          "Servo uS,false,0-3000-5-1"
                      });

    addNewPinSettings(JSON_AIO, {
                          "Input,true,0-500-50-100.0"
                      });

    addNewPinSettings(REMOTE_CONN_REMOTE, {
                          "UART,false,",
                          "I2C,false,",
                          "SPI,true,"
                      });

    // Set combo values for pins
    setCombos(  JSON_AIO, {"Input"});
    setCombos(  JSON_DIO,
                {"Input", "Output", "Servo Deg", "Servo uS"});
    setCombos(  JSON_DIO,
                {"Input", "Output", "PWM", "Servo Deg", "Servo uS"},
                {3, 5, 6, 9, 10, 11});
    setCombos(REMOTE_CONN_REMOTE, {"UART", "I2C", "SPI"});

    // Set device pins (Automatically disables extra pins)
    setNumPins(JSON_AIO, 6);
    setNumPins(JSON_DIO, 14);

    qDebug() << controlMap.keys();
    qDebug() << controlMap.value(JSON_DIO)->keys();
}

ArduinoUno_IO::~ArduinoUno_IO()
{
}
