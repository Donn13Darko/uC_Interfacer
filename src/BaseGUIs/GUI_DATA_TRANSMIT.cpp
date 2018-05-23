#include "GUI_DATA_TRANSMIT.h"
#include "ui_gui_data_transmit.h"

#include <QFile>
#include <QFileDialog>

GUI_DATA_TRANSMIT::GUI_DATA_TRANSMIT(uint8_t chunk, QWidget *parent) :
    GUI_BASE(parent),
    ui(new Ui::GUI_DATA_TRANSMIT)
{
    ui->setupUi(this);
    chunkSize = chunk;

    // Connect radio group change signals
    connect(ui->MSG_Sel, SIGNAL(buttonClicked(int)), this, SLOT(input_RadioClicked(int)));
    connect(ui->TX_RX_Sel, SIGNAL(buttonClicked(int)), this, SLOT(TX_RX_RadioClicked(int)));

    // Set radio values
    ui->File_Radio->setChecked(true);
    ui->TX_RX_Radio->setChecked(true);
    input_RadioClicked(0);
    TX_RX_RadioClicked(0);
}

GUI_DATA_TRANSMIT::~GUI_DATA_TRANSMIT()
{
    delete ui;
}

void GUI_DATA_TRANSMIT::reset_gui()
{
    // Set radio values
    ui->File_Radio->setChecked(true);
    ui->TX_RX_Radio->setChecked(true);
    input_RadioClicked(0);
    TX_RX_RadioClicked(0);
}

void GUI_DATA_TRANSMIT::input_RadioClicked(int)
{
    if (ui->File_Radio->isChecked())
        input_select(true, false);
    else if (ui->Input_Radio->isChecked())
        input_select(false, true);
}

void GUI_DATA_TRANSMIT::TX_RX_RadioClicked(int)
{
    if (ui->RX_Radio->isChecked())
    {
        TX_disable();
        RX_enable();
        send({
                 JSON_REMOTE_CONN,
                 REMOTE_CONN_SET_RX
             });
    } else if (ui->TX_Radio->isChecked())
    {
        TX_enable();
        RX_disable();
        send({
                 JSON_REMOTE_CONN,
                 REMOTE_CONN_SET_TX
             });
    } else if (ui->TX_RX_Radio->isChecked())
    {
        TX_enable();
        RX_enable();
        send({
                 JSON_REMOTE_CONN,
                 REMOTE_CONN_SET_TX_RX
             });
    } else
    {
        TX_disable();
        RX_disable();
        send({
                 JSON_REMOTE_CONN,
                 REMOTE_CONN_SET_NONE
             });
    }
}

void GUI_DATA_TRANSMIT::on_SendMSG_Button_clicked()
{
    // Find which radio button selected
    if (ui->Input_Radio->isChecked())
        send(ui->msg_PlainText->toPlainText());
    else if (ui->File_Radio->isChecked())
        sendFile(ui->FilePathEdit->text(), chunkSize);
}

void GUI_DATA_TRANSMIT::on_OpenFile_Button_clicked()
{
    // Select file to send
    QString file;
    if (getOpenFilePath(&file))
        ui->FilePathEdit->setText(file);
}

void GUI_DATA_TRANSMIT::on_SaveAs_Button_clicked()
{
    // Select file save location
    QString fileName;
    if (!getSaveFilePath(&fileName))
        return;

    // Save file
    if (!saveFile(fileName, received))
        showMessage("ERROR: Failed to save file!");
}

void GUI_DATA_TRANSMIT::on_ClearReceived_Button_clicked()
{
    received.clear();
    ui->recv_PlainText->clear();
}

void GUI_DATA_TRANSMIT::receive(QByteArray recvData)
{
    if (!ui->TX_Radio->isChecked())
    {
        received.append(recvData);
        ui->recv_PlainText->appendPlainText(QString(recvData));
    }
}

void GUI_DATA_TRANSMIT::input_select(bool fileIN, bool plainIN)
{
    ui->FilePathEdit->setEnabled(fileIN);
    ui->OpenFile_Button->setEnabled(fileIN);
    ui->msg_PlainText->setEnabled(plainIN);
}

void GUI_DATA_TRANSMIT::TX_enable()
{
    ui->Send_Label->setEnabled(true);
    ui->File_Radio->setEnabled(true);
    ui->Input_Radio->setEnabled(true);
    ui->SendMSG_Button->setEnabled(true);

    input_RadioClicked(0);
}

void GUI_DATA_TRANSMIT::TX_disable()
{
    ui->Send_Label->setEnabled(false);
    ui->File_Radio->setEnabled(false);
    ui->Input_Radio->setEnabled(false);
    ui->SendMSG_Button->setEnabled(false);

    input_select(false, false);
}

void GUI_DATA_TRANSMIT::RX_enable()
{
    ui->Receive_Label->setEnabled(true);
    ui->recv_PlainText->setEnabled(true);
    ui->ClearReceived_Button->setEnabled(true);
    ui->SaveAs_Button->setEnabled(true);
}

void GUI_DATA_TRANSMIT::RX_disable()
{
    ui->Receive_Label->setEnabled(false);
    ui->recv_PlainText->setEnabled(false);
    ui->ClearReceived_Button->setEnabled(false);
    ui->SaveAs_Button->setEnabled(false);
}
