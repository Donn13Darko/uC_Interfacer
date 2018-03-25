#ifndef GUI_BASE_H
#define GUI_BASE_H

#include <QObject>
#include <QDialog>

#include "Communuication/serial_rs232.h"
#include "Communuication/json_info.h"

class GUI_BASE : public QDialog
{
    Q_OBJECT

public:
    GUI_BASE(QStringList params, QWidget *parent = 0);
    ~GUI_BASE();

    static bool showMessage(QString msg);

signals:
    void write_data(QByteArray data);
    void write_data(std::initializer_list<uint8_t> data);

protected:
    QStringList connParams;
    Serial_RS232 *serial_rs232;
    static float S2MS;

    void send_connect();
    void send_disconnect();

    void send(QByteArray data);
    void send(std::initializer_list<uint8_t> data);
    void sendFile(QString filePath, size_t chunkSize);

    bool getOpenFilePath(QString *filePath);
    bool getSaveFilePath(QString *filePath);
    bool saveFile(QString filePath, QByteArray data);
};

#endif // GUI_BASE_H
