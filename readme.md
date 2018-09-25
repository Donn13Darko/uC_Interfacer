# Microcontroller Interface
This project aims at providing a simple interface for working with various microcontrollers. The ultimate goal is to aid in rapid hardware prototyping by removing the burden of intial software bring up and device programming.


# Primary Controls
Currently there are 3 primary panels supported when connected to a correctly programmed device:

## I/O Controller
This GUI enables command & control of all the pins on the connected device.

## File Transmit
Transmits a given file to the target device. In the default application, the code simply steps through the FSM for this without actually retaining the file information.

## Programmer
Connects to a pre-programmed microctontroller and utilizes it to program other devices.

### Helpful Sites
The programmer in this application was greatly aided and loosely based off of these sites and their very helpful documentation:
1. [Ard Pic Prog](http://rweather.github.io/ardpicprog/)
2. [Picprog](http://hyvatti.iki.fi/~jaakko/pic/picprog.html)
3. [High Spark](https://sites.google.com/site/thehighspark/arduino-pic18f)
4. [Arduino](https://www.arduino.cc/) and their [github](https://github.com/arduino)


# Secondary Controls
There are also a couple of sub windows that act as standalone features. These can be reached by clicking the ... button directly to the right of the Disconnect button. These controls include:

## USB Programmer
Adds baseline programming to some microcontrollers without the need for a man in the middle. This is limited to controllers that can either be directly programmed from a USB connection or using a USB to TTL converter. For additional functionality, programmer chainning may be required (i.e. program an Arduino or other AVR device with this and then use that with the main controls panel to program the next device).


# General Sources
The following were heavily used across the entire project:
1. [Qt](https://www.qt.io/): Qt Creator & accompanying software


# License
uC Interface - A GUI for Programming & Interfacing with Microcontrollers
Copyright (C) 2018  Mitchell Oleson

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
