#ifndef GUI_PROGRAMMER_H
#define GUI_PROGRAMMER_H

#include "GUI_BASE.h"

namespace Ui {
class GUI_PROGRAMMER;
}

class GUI_PROGRAMMER : public GUI_BASE
{
    Q_OBJECT

public:
    explicit GUI_PROGRAMMER(QString deviceType, size_t chunk, QWidget *parent = 0);
    ~GUI_PROGRAMMER();

    void reset_gui();

private slots:
    void on_HexFile_Button_clicked();
    void on_RefreshPreview_Button_clicked();
    void on_BurnData_Button_clicked();
    void on_HexFormat_Combo_currentIndexChanged(int);

private:
    Ui::GUI_PROGRAMMER *ui;

    size_t chunkSize;
    QByteArray loadedHex;

    static QMap<QString, QStringList> burnMethods;
    static QStringList hexFormats;

    QString format_hex();
};

#endif // GUI_PROGRAMMER_H
