#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QMap>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_DeviceCombo_currentIndexChanged(int);
    void on_ConnTypeCombo_currentIndexChanged(int);
    void on_SpeedCombo_currentIndexChanged(int);
    void on_GUITypeCombo_currentIndexChanged(int);

    void on_DeviceConnect_clicked();

private:
    Ui::MainWindow *ui;

    QString deviceType;
    QString connType;
    QString speed;
    QString connInfo;
    QString guiType;

    QString connSel = "Connection Info";
    QString guiSel = "GUI Info";

    QMap<QString, QMap<QString, QMap<QString, QStringList>>> supportMapper;

    void updateTypeCombos();
    void updateSpeedCombo();
};

#endif // MAINWINDOW_H
