#-------------------------------------------------
#
# Project created by QtCreator 2018-11-22T11:22:44
#
#-------------------------------------------------
QT += gui testlib serialport network charts widgets

CONFIG += qt console warn_on depend_includepath testcase
#CONFIG -= app_bundle

TARGET = uc-interface-tests
TEMPLATE = app

SOURCES += \
    main.cpp \
    gui-tests/gui-base-tests.cpp

HEADERS += \
    gui-tests/gui-base-tests.hpp
