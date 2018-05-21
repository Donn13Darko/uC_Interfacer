#-------------------------------------------------
#
# Project created by QtCreator 2018-01-10T20:41:44
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = uC_Interfacer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
    BaseGUIs/GUI_8AIO_16DIO_COMM.cpp \
    Communuication/serial_rs232.cpp \
    uCInterfaces/arduinouno_io_gui.cpp \
    BaseGUIs/GUI_DATA_TRANSMIT.cpp \
    BaseGUIs/GUI_PIN_BASE.cpp \
    BaseGUIs/GUI_BASE.cpp \
    uCInterfaces/arduinomega_io_gui.cpp \
    BaseGUIs/GUI_PROGRAMMER.cpp \
    mainwindow.cpp

HEADERS += \
    BaseGUIs/GUI_8AIO_16DIO_COMM.h \
    Communuication/serial_rs232.h \
    uCInterfaces/arduinouno_io_gui.h \
    BaseGUIs/GUI_DATA_TRANSMIT.h \
    BaseGUIs/GUI_PIN_BASE.h \
    BaseGUIs/GUI_BASE.h \
    uCInterfaces/arduinomega_io_gui.h \
    Communuication/json_info.h \
    BaseGUIs/GUI_PROGRAMMER.h \
    mainwindow.h

FORMS += \
    BaseGUIs/GUI_DATA_TRANSMIT.ui \
    BaseGUIs/GUI_8AIO_16DIO_COMM.ui \
    BaseGUIs/GUI_PROGRAMMER.ui \
    mainwindow.ui

DISTFILES += \
    uCInterfaces/arduinouno_io_uart/arduinouno_io_uart.ino \
    uCInterfaces/arduinomega_io_uart/arduinomega_io_uart.ino
