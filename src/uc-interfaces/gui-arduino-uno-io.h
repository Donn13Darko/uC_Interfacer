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

#ifndef ARDUINOUNO_IO_H
#define ARDUINOUNO_IO_H

#include "../user-interfaces/gui-8aio-16dio-comm.h"
#include <QMap>

namespace Ui {
class ArduinoUno_IO;
}

class ArduinoUno_IO : public GUI_8AIO_16DIO_COMM
{
    Q_OBJECT

public:
    explicit ArduinoUno_IO(QWidget *parent = 0);
    ~ArduinoUno_IO();
};

#endif // ARDUINOUNO_IO_H
