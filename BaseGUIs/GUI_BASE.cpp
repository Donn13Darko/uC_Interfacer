#include "GUI_BASE.h"

#include <QMessageBox>
#include <QFile>
#include <QTimer>
#include <QFileDialog>
#include <QTextStream>

float GUI_BASE::S2MS = 1000.0;

GUI_BASE::GUI_BASE(QStringList params, QWidget *parent)
{
    connParams = params;

    bool connected = false;
    if (connParams[1] == "RS-232")
    {
        serial_rs232 = new Serial_RS232(connParams[3], connParams[2]);
        serial_rs232->open();
        connected = serial_rs232->isConnected();

        // First write operations for new connection always fail
        if (connected)
        {
            send_connect();

            send({JSON_RESET, JSON_START});
            send({JSON_RESET, JSON_END});
        }
    }

    // Error out of can't connect to hardware
    if (!connected)
    {
        showMessage("Error: Unable to connect to target!");
        QTimer::singleShot(0, this, SLOT(close()));
    }
}

GUI_BASE::~GUI_BASE()
{
    if (connParams[1] == "RS-232")
    {
        serial_rs232->close();
        delete serial_rs232;
    }
}

bool GUI_BASE::showMessage(QString msg)
{
    QMessageBox n;
    n.setText(msg);
    return n.exec();
}

void GUI_BASE::send_connect()
{
    if ((connParams[1] == "RS-232") && serial_rs232->isConnected())
    {
        connect(this, SIGNAL(write_data(QByteArray)),
                serial_rs232, SLOT(write(QByteArray)));
        connect(this, SIGNAL(write_data(std::initializer_list<uint8_t>)),
                serial_rs232, SLOT(write(std::initializer_list<uint8_t>)));
    }
}

void GUI_BASE::send_disconnect()
{
    if (connParams[1] == "RS-232")
    {
        disconnect(this, SIGNAL(write_data(QByteArray)),
                serial_rs232, SLOT(write(QByteArray)));
        disconnect(this, SIGNAL(write_data(std::initializer_list<uint8_t>)),
                serial_rs232, SLOT(write(std::initializer_list<uint8_t>)));
    }
}

void GUI_BASE::send(QByteArray data)
{
    emit write_data(data);
}

void GUI_BASE::send(std::initializer_list<uint8_t> data)
{
    emit write_data(data);
}

void GUI_BASE::sendFile(QString filePath, size_t chunkSize)
{
    uint32_t enumFlags = QIODevice::ReadOnly;
    QFile sFile(filePath);
    if (!sFile.open((QIODevice::OpenModeFlag) enumFlags)) return;

    send({
             JSON_FILE,
             JSON_START,
         });

    while (!sFile.atEnd())
    {
        send(sFile.read(chunkSize));
    }
    sFile.close();

    send({
             JSON_FILE,
             JSON_END
         });
}

bool GUI_BASE::getOpenFilePath(QString *filePath)
{
    *filePath = QFileDialog::getOpenFileName(this, tr("Open"), "", tr("All Files (*)"));;

    if (filePath->length() <= 0)
        return false;
    return true;
}

bool GUI_BASE::getSaveFilePath(QString *filePath)
{
    *filePath = QFileDialog::getSaveFileName(this, tr("Save Location"),
                                             "", tr("All Files (*)"));;

    return !filePath->isEmpty();
}

bool GUI_BASE::saveFile(QString filePath, QByteArray data)
{
    uint32_t enumFlags = QIODevice::WriteOnly;
    QFile sFile(filePath);
    if (!sFile.open((QIODevice::OpenModeFlag) enumFlags))
        return false;

    qint64 res = sFile.write(data);
    sFile.close();

    if (res < 0)
        return false;
    return true;
}
