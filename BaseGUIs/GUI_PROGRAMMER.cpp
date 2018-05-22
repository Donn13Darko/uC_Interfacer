#include "GUI_PROGRAMMER.h"
#include "ui_GUI_PROGRAMMER.h"

// Setup static burn methods list
QMap<QString, QStringList> GUI_PROGRAMMER::burnMethods({
                                                           {"Arduino Uno/Genuino",
                                                            {"ICSP"}
                                                           }
                                                       });

// Setup static hex format list
QStringList GUI_PROGRAMMER::hexFormats({"Atmel 16-bit"});

GUI_PROGRAMMER::GUI_PROGRAMMER(QString deviceType, size_t chunk, QWidget *parent) :
    GUI_BASE(parent),
    ui(new Ui::GUI_PROGRAMMER)
{
    ui->setupUi(this);
    chunkSize = chunk;
    loadedHex = QByteArray();

    ui->HexFormat_Combo->addItems(GUI_PROGRAMMER::hexFormats);
    ui->BurnMethod_Combo->addItems(GUI_PROGRAMMER::burnMethods.value(deviceType));
}

GUI_PROGRAMMER::~GUI_PROGRAMMER()
{
    delete ui;
}

void GUI_PROGRAMMER::reset_gui()
{
    ui->HexFile_LineEdit->clear();
    ui->HexPreview_Edit->clear();
    ui->ReadData_Edit->clear();
}

void GUI_PROGRAMMER::on_HexFile_Button_clicked()
{
    // Select programmer file
    QString file;
    if (getOpenFilePath(&file, tr("HEX (*.hex);;All Files (*.*)")))
        ui->HexFile_LineEdit->setText(file);
}

void GUI_PROGRAMMER::on_RefreshPreview_Button_clicked()
{
    // Get file path
    QString filePath = ui->HexFile_LineEdit->text();
    if (filePath.isEmpty()) return;

    // Load in file if it exists
    loadedHex = loadFile(filePath);
    on_HexFormat_Combo_currentIndexChanged(0);

    // Temp for debugging
    ui->ReadData_Edit->clear();
    ui->ReadData_Edit->appendPlainText(QString(loadedHex));
    ui->ReadData_Edit->moveCursor(QTextCursor::Start);
}

void GUI_PROGRAMMER::on_BurnData_Button_clicked()
{
    sendFile(ui->HexFile_LineEdit->text(), chunkSize);
}

void GUI_PROGRAMMER::on_HexFormat_Combo_currentIndexChanged(int)
{
    // Update Hex
    ui->HexPreview_Edit->clear();
    ui->HexPreview_Edit->appendPlainText(format_hex());

    // Set cursor to top
    ui->HexPreview_Edit->moveCursor(QTextCursor::Start);
}

QString GUI_PROGRAMMER::format_hex()
{
    QStringList hexList = QString(loadedHex).split('\n');

    int len_data;
    QString curr;
    QString final = "";
    for (int i = 0; i < hexList.length(); i++)
    {
        curr = hexList[i];
        if (curr.isEmpty()) continue;

        len_data = 2 * curr.mid(1,2).toInt(nullptr, 16);
        final += curr.mid(2+1, 4) + ": " + curr.mid(2+4+2+1, len_data)
                + " (" + curr.mid(2+4+1,2) + "," + curr.mid(2+4+2+len_data+1, 2) + ")\n";
    }

    return final;
}

