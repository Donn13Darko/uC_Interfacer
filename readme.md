# Microcontroller Interface
This project aims at providing a simple interface for working with various microcontrollers. The ultimate goal is to aid in rapid hardware prototyping by eliminating the burden of intial software bring up and device programming.


# Primary Interfaces
Currently there are 4 primary panels supported when connected to a correctly programmed device:

## I/O Controller
This GUI enables command & control of all the pins on the connected device.

## Data Transmit
Transmits a given file or input to the target device. The data is precluded with a byte length CMD that notifies of the total expected data. Additionally, before and after the data is sent, a packet with zero data length is sent to signify start and end.

## Programmer
[In Work]
Connects to a pre-programmed microctontroller and utilizes it to program other devices.

### Helpful Sites
The programmer in this application was greatly aided and loosely based off of these sites and their very helpful documentation:
1. [Ard Pic Prog](http://rweather.github.io/ardpicprog/)
2. [Picprog](http://hyvatti.iki.fi/~jaakko/pic/picprog.html)
3. [High Spark](https://sites.google.com/site/thehighspark/arduino-pic18f)
4. [Arduino](https://www.arduino.cc/) and their [github](https://github.com/arduino)

## Custom CMD
Allows sending of custom major key, minor key and data. The backend will add the correct data length bits/bytes and checksum. Also supports sending a command file line by line 


# Secondary Interfaces
There are also a couple of sub windows that act as standalone features. These can be reached by clicking the ... button directly to the right of the Disconnect button. These controls include:

## USB Programmer
[In Work]
Adds baseline programming to some microcontrollers without the need for a man in the middle. This is limited to controllers that can either be directly programmed from a USB connection or using a USB to TTL converter. For additional functionality, programmer chainning may be required (i.e. program an Arduino or other AVR device with this and then use that with the main controls panel to program the next device).


# CMD Structure
1. Major Key & Bits: Upper six bits are the major key, lower two bits are used to set number of date length bytes (lower bits removed on receive)
2. Minor Key: Send by the CMD gui and describes what type of action was done
3. Data Length: Use the lower two bits of the first byte to determine number of bytes in this section (can be up to 4 bytes long). This section decribes how long the sent data section is.
4. Data: This section directly follows data length and is data length bytes
5. Checksum: Used for CMD verification, this is transparently added to all CMDs.


# Transmission Structure
1. First packet is CMD or request
2. This is then followed by an ack from the receiving device
3. If data was requested, the device returns the data
4. Finally, if data was requested and received, the original device acks received


# Configuration Files
These allow for dynamic setting of many of the gui values and structures. See examples in src/uc-interface/\*.ini.
## Settings
Suround these with brakets number them to preserve their order when connected.
1. GENERAL SETTINGS [In Work]
2. Welcome: A basic template to explain any special functions or unique features of the ini or backend controller if desigend.
3. IO: Control the combo boxes and ranges for the given pins. Can use multiple instances to control more than 16DIO or 8AIO by using the start_num argument.
4. Data Transmit: Set tab name
5. Programmer [In Work]: Set tab name, burn methods, hex file parsing, and device specific instructions for each burn method
6. Custom CMD: Set tab name


# General Sources
The following were utilized across the project:
1. [Qt](https://www.qt.io/): Qt Creator & accompanying software
2. [UPX](https://upx.github.io/): Used to compress the static release executables


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
