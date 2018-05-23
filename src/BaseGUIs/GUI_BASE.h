#ifndef GUI_BASE_H
#define GUI_BASE_H

#include <QObject>
#include <QWidget>

#include "../communication/json_info.h"
#include <QDebug>

class GUI_BASE : public QWidget
{
    Q_OBJECT

public:
    GUI_BASE(QWidget *parent = 0);
    ~GUI_BASE();

    static bool showMessage(QString msg);
    void reset_gui() {/*Default do nothing*/}

signals:
    void write_data(QByteArray data);
    void write_data(std::initializer_list<uint8_t> data);
    void connect_signals(bool connect);
    void readyRead();

private slots:
    void receive(QByteArray recvData);

protected:
    static float S2MS;
    QByteArray rcvd;

    void send(QString data);
    void send(QByteArray data);
    void send(std::initializer_list<uint8_t> data);
    void sendFile(QString filePath, uint8_t chunkSize);

    bool getOpenFilePath(QString *filePath, QString fileTypes = tr("All Files (*.*)"));
    bool getSaveFilePath(QString *filePath, QString fileTypes = tr("All Files (*.*)"));
    bool saveFile(QString filePath, QByteArray data);
    QByteArray loadFile(QString filePath);

    void reset_remote();
    void waitForResponse(int len, int msecs = 5000);
    bool checkAck(QByteArray ack);
};

#endif // GUI_BASE_H
