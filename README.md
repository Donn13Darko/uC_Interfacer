# Microcontroller Interface
This project aims at providing a simple user interface for controlling various microcontrollers. By utlizing the provided gui and defining the desired functions from the base file (required to map CMDs to hardware),  The ultimate goal is to aid in rapid hardware prototyping by eliminating the burden of intial software bring up and device programming.


# Primary Interfaces
The following user interfaces are available for connected devices that have been programmed using the provided state machines or an equivalent program.

## I/O Controller
Enables control & readback of all the pins and some hardware on the connected device. The pin combos, scale, and numbering are dynamically customizable using an INI file as described later in this document.

## Data Transmit
Transmits a given file or input to the target device. The data is precluded with a byte length CMD that notifies of the total expected data. Additionally, before and after the data is sent, a packet with zero data length is sent to signify start and end.

## Programmer [In Work]
Utilizes the connected device to program other devices. Basic layouts for some programming methods are included in the uc-interfaces folder.

### Helpful Sites
The programmer in this application was greatly aided and loosely based off of the following sites and their very helpful documentation:
1. [avrdude](https://www.nongnu.org/avrdude/)
2. [Ard Pic Prog](http://rweather.github.io/ardpicprog/)
3. [Picprog](http://hyvatti.iki.fi/~jaakko/pic/picprog.html)
4. [High Spark](https://sites.google.com/site/thehighspark/arduino-pic18f)
5. [Arduino](https://www.arduino.cc/) and their [github](https://github.com/arduino)
6. [quaxio](https://www.quaxio.com/programming_an_at89s4051_with_an_arduino/)
7. Code and Life articles [AVR ATiny USB Parts 1-4](http://codeandlife.com/2012/01/22/avr-attiny-usb-tutorial-part-1/) and [Arduino Uno as ISP](http://codeandlife.com/2012/03/21/using-arduino-uno-as-isp/).

## Custom CMD
Allows sending of custom major key, minor key, and data as well as choosing an input base for the keys and data. Additionally, supports sending a file chunked by each newline. This is ideal for sending multiple verified commands and provides a base for scripting procedures as any major key, minor key, and data can be sent and encoded with this GUI. *NOTE:* The checksum will default to the Custom CMD gui checksum even if sending a differet major key (GUI has no knowledge of the other tabs/GUIs).


# Secondary Interfaces
There are also a couple of sub windows that act as standalone features. These can be reached by clicking the ... button directly to the right of the Disconnect button. These controls include:

## More Options [In Work]
Houses a multitude of settings that can be dynamically adjusted during runtime. This includes: Not resetting between tab switches, adjusting the chunk size, setting the GUI checksums, as well as setting detailed connection parameters (these must be set prior to connection in order for them to take effect).

## USB Programmer [In Work]
Adds baseline programming for some microcontrollers without the need for a man in the middle. This is limited to controllers that can either be directly programmed from a USB connection or using a USB to RS232/SPI converter. For expanded devices, it might be required to do programmer chainning (i.e. program an Arduino or other device using this and then use the programmed device in conjunction with the primary interface programmer to program the next device).


# CMD Structure
1. Major Key & Bits: Upper six bits are the major key which is used for checksum selection and first layer parsing. The lower two bits are used to set number of date length bytes and are masked away on receive by the provided FSM. *Note:* A bit value of three (11) corresponds to four bytes (adjusts from a uint24_t to a uint32_t).
2. Minor Key: Sent by the CMD gui and describes what type of action was done or needs to be carried out. This is passed as the first argument to the default FSM functions.
3. Data Length: Use the lower two bits of the first byte to determine number of bytes in this section (can be up to 4 bytes long). This section decribes how long the sent data section is. This is passed as a uint32_t for the third argument for the default FSM functions.
4. Data: This section directly follows the data length bytes and is data length bytes long. This is passed as a uint8_t* as the second argument for the default FSM functions.
5. Checksum: Used for CMD verification, this is transparently added to all CMDs and removed in the provided FSM.


# Transmission Structure
1. First packet is CMD or request
2. Second packet is an ACK for the CMD from the receiving device
3. If data was requested, the third packet is the device returning the requested data
4. If data was requested and received, the original sending device acks received for the data


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
1. [Qt](https://www.qt.io/): Qt Creator & its accompanying software/documentation
2. [UPX](https://upx.github.io/): Used to compress the static release executables (will be done starting with V1.0)


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
