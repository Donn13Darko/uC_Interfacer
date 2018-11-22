#-------------------------------------------------
#
# Project created by QtCreator 2018-11-22T11:22:44
#
#-------------------------------------------------
QT += core gui testlib serialport network charts widgets

CONFIG += qt console warn_on depend_includepath testcase

TARGET = uc-interface-tests
TEMPLATE = app

SOURCES += \
    main.cpp

# Include main project src files
include(../src/mainwindow.pri)
include(../src/checksums/checksums.pri)
include(../src/communication/communication.pri)
include(../src/gui-helpers/gui-helpers.pri)
include(../src/user-interfaces/user-interfaces.pri)

# Include local test files
include(gui-tests/gui-base-tests.pri)
