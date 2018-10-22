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

#ifndef CHECKSUMS_H
#define CHECKSUMS_H

/*
 * Provides a single file to include in the QT GUI project
 * To include a new checksum in project:
 *  1) Add a new include for the checksums .h below
 *  2) Add a corresponding checksum map entry in MainWindow.cpp
*/

// CRC checksums - LUT = Lookup table, Poly = Live Compute using Polynomial
#include "crc-8-lut.h"
#include "crc-8-poly.h"
#include "crc-16-lut.h"
#include "crc-16-poly.h"
#include "crc-32-lut.h"
#include "crc-32-poly.h"

// Calls an executabe to produce the checksum
#include "checksum-exe.h"

#endif // CHECKSUMS_H
