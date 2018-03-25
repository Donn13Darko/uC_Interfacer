#ifndef SERIAL_RS232_H
#define SERIAL_RS232_H

#include <QObject>
#include <QMutex>
#include <QSerialPort>
#include <QSerialPortInfo>

class Serial_RS232 : public QObject
{
    Q_OBJECT

public:
    Serial_RS232(QString port, QString baudrate = "9600", QObject *parent = NULL);
    ~Serial_RS232();

    void open();
    void close();
    bool isConnected();
    static QStringList Baudrate_Defaults;

signals:
    void readyRead(QByteArray readData);

public slots:
    void write(QByteArray writeData);
    void write(std::initializer_list<uint8_t> writeData);

private slots:
    void read();

private:
    QSerialPort *rs232;
    QMutex *readLock;
    QMutex *writeLock;

    bool connected;
    qint64 maxSize;
};

#endif // SERIAL_RS232_H
