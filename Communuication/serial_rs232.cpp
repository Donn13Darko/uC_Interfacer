#include "serial_rs232.h"
#include <QDebug>

QStringList Serial_RS232::Baudrate_Defaults({"1200", "2400", "4800", "9600", "19200", "39400", "57600", "115200"});

Serial_RS232::Serial_RS232(QString port, QString baudrate, QObject *parent) :
    QObject(parent)
{
    rs232 = new QSerialPort(this);
    rs232->setPortName(port);
    rs232->setBaudRate(baudrate.toInt());
    rs232->setDataBits(QSerialPort::Data8);

    maxSize = 1024;
    readLock = new QMutex(QMutex::Recursive);
    writeLock = new QMutex(QMutex::Recursive);

    connect(rs232, SIGNAL(readyRead()), this, SLOT(read()));
}

Serial_RS232::~Serial_RS232()
{
    delete rs232;
}

void Serial_RS232::open()
{
    connected = rs232->open(QIODevice::ReadWrite);
}

void Serial_RS232::close()
{
    rs232->close();
    connected = false;
}

bool Serial_RS232::isConnected()
{
    return connected;
}

QStringList Serial_RS232::getDevices()
{
    QStringList portNames;
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for (auto i = ports.begin(); i != ports.end(); i++)
        portNames.append((*i).portName());

    return portNames;
}

void Serial_RS232::waitOnRead(int msecs)
{
    if (!isConnected()) return;
    rs232->waitForReadyRead(msecs);
}

void Serial_RS232::write(QByteArray writeData)
{
    writeLock->lock();

    qDebug() << writeData;
    rs232->write((const QByteArray) writeData);
    rs232->waitForBytesWritten();

    writeLock->unlock();
}

void Serial_RS232::write(std::initializer_list<uint8_t> writeData)
{
    QByteArray data;
    for (auto i = writeData.begin(); i != writeData.end(); i++)
    {
        data.append((char) (*i));
    }

    write(data);
}

void Serial_RS232::read()
{
    readLock->lock();
    emit readyRead(rs232->readAll());
    readLock->unlock();
}
