#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QMap>
#include <QTimer>

#include "Communuication/serial_rs232.h"
#include "Communuication/json_info.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static bool showMessage(QString msg);

signals:
    void write_data(QByteArray data);
    void write_data(std::initializer_list<uint8_t> data);

public slots:
    void connect_signals(bool connect);

private slots:
    void on_DeviceCombo_currentIndexChanged(int);
    void on_ConnTypeCombo_currentIndexChanged(int);
    void on_SpeedCombo_currentIndexChanged(int);

    void on_DeviceConnect_clicked();
    void on_DeviceDisconnect_clicked();

    void updateConnInfoCombo();

    void on_ucOptions_currentChanged(int index);
    void receive(QByteArray) {}

private:
    Ui::MainWindow *ui;
    int prev_tab;

    QString deviceType;
    QString connType;
    QString speed;
    QString connInfo;

    QMap<QString, QMap<QString, QStringList>> supportMapper;

    QTimer updateConnInfo;
    Serial_RS232 *serial_rs232;

    size_t arduino_chunk_size = 32;

    void updateTypeCombos();
    void updateSpeedCombo();
    void setConnecting(bool conn);
    void reset_remote();
    void connect2sender(QObject* obj, bool conn);
};

#endif // MAINWINDOW_H
