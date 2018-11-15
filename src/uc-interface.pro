#-------------------------------------------------
#
# Project created by QtCreator 2018-01-10T20:41:44
#
#-------------------------------------------------

QT += core gui serialport network charts widgets

TARGET = uc-interface
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
    mainwindow.cpp \
    user-interfaces/gui-base.cpp \
    user-interfaces/gui-data-transmit.cpp \
    user-interfaces/gui-programmer.cpp \
    user-interfaces/gui-welcome.cpp \
    communication/tcp-client.cpp \
    communication/tcp-server.cpp \
    communication/udp-socket.cpp \
    checksums/crc-8-lut.c \
    checksums/crc-8-poly.c \
    checksums/crc-16-lut.c \
    checksums/crc-16-poly.c \
    checksums/crc-32-lut.c \
    checksums/crc-32-poly.c \
    user-interfaces/gui-custom-cmd.cpp \
    communication/comms-base.cpp \
    checksums/checksum-exe.c \
    gui-helpers/gui-more-options.cpp \
    communication/serial-com-port.cpp \
    gui-helpers/gui-comm-bridge.cpp \
    gui-helpers/gui-create-new-tabs.cpp \
    user-interfaces/gui-io-control.cpp \
    gui-helpers/gui-generic-helper.cpp \
    gui-helpers/gui-chart-element.cpp \
    gui-helpers/gui-chart-view.cpp

HEADERS += \
    mainwindow.h \
    user-interfaces/gui-base.h \
    user-interfaces/gui-data-transmit.h \
    user-interfaces/gui-programmer.h \
    user-interfaces/gui-welcome.h \
    communication/tcp-client.h \
    communication/tcp-server.h \
    communication/udp-socket.h \
    checksums/checksums.h \
    checksums/crc-8-lut.h \
    checksums/crc-8-poly.h \
    checksums/crc-16-lut.h \
    checksums/crc-16-poly.h \
    checksums/crc-32-lut.h \
    checksums/crc-32-poly.h \
    user-interfaces/gui-programmer-minor-keys.h \
    user-interfaces/gui-custom-cmd.h \
    user-interfaces/gui-data-transmit-minor-keys.h \
    user-interfaces/gui-custom-cmd-minor-keys.h \
    user-interfaces/gui-welcome-minor-keys.h \
    communication/comms-base.h \
    user-interfaces/gui-base-major-keys.h \
    checksums/checksum-exe.h \
    gui-helpers/gui-more-options.h \
    communication/serial-com-port.h \
    gui-helpers/gui-comm-bridge.h \
    gui-helpers/gui-create-new-tabs.h \
    user-interfaces/gui-io-control.h \
    user-interfaces/gui-io-control-minor-keys.h \
    gui-helpers/gui-generic-helper.h \
    gui-helpers/gui-chart-element.h \
    gui-helpers/gui-chart-view.h

FORMS += \
    mainwindow.ui \
    user-interfaces/gui-data-transmit.ui \
    user-interfaces/gui-programmer.ui \
    user-interfaces/gui-welcome.ui \
    user-interfaces/gui-custom-cmd.ui \
    gui-helpers/gui-more-options.ui \
    gui-helpers/gui-create-new-tabs.ui \
    user-interfaces/gui-io-control.ui \
    gui-helpers/gui-chart-element.ui \
    gui-helpers/gui-chart-view.ui

RESOURCES += \
    uc-interfaces.qrc
