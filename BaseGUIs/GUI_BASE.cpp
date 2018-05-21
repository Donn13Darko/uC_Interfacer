#include "GUI_BASE.h"

#include <QMessageBox>
#include <QFile>
#include <QTimer>
#include <QFileDialog>
#include <QTextStream>

float GUI_BASE::S2MS = 1000.0;

GUI_BASE::GUI_BASE(QWidget *parent) :
    QWidget(parent)
{
}

GUI_BASE::~GUI_BASE()
{
}

bool GUI_BASE::showMessage(QString msg)
{
    QMessageBox n;
    n.setText(msg);
    return n.exec();
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
    *filePath = QFileDialog::getOpenFileName(this, tr("Open"),
                                             "", tr("All Files (*)"));;

    return !filePath->isEmpty();
}

bool GUI_BASE::getSaveFilePath(QString *filePath)
{
    *filePath = QFileDialog::getSaveFileName(this, tr("Save Location"),
                                             "", tr("All Files (*)"));

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

void GUI_BASE::reset_remote()
{
    send({JSON_RESET, JSON_START});
    send({JSON_RESET, JSON_END});
}
