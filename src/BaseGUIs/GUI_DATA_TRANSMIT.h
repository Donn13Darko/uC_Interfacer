#ifndef GUI_DATA_TRANSMIT_H
#define GUI_DATA_TRANSMIT_H

#include "GUI_BASE.h"

namespace Ui {
class GUI_DATA_TRANSMIT;
}

class GUI_DATA_TRANSMIT : public GUI_BASE
{
    Q_OBJECT

public:
    explicit GUI_DATA_TRANSMIT(uint8_t chunk, QWidget *parent = 0);
    ~GUI_DATA_TRANSMIT();

    void reset_gui();

private slots:
    void MSG_Sel_buttonClicked(int);
    void TX_RX_Sel_buttonClicked(int);
    void on_SendMSG_Button_clicked();

    void on_OpenFile_Button_clicked();
    void on_SaveAs_Button_clicked();

    void on_ClearReceived_Button_clicked();

    void receive(QByteArray recvData);

private:
    Ui::GUI_DATA_TRANSMIT *ui;

    QByteArray received;
    uint8_t chunkSize;

    void input_select(bool fileIN, bool plainIN);

    void TX_enable();
    void TX_disable();

    void RX_enable();
    void RX_disable();
};

#endif // GUI_DATA_TRANSMIT_H
