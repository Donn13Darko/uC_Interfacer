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
    ui->HexFile_LineEdit->setText("");
    ui->HexPreview_Edit->clear();
    ui->HexPreview_Edit->appendPlainText("");
    ui->ReadData_Edit->clear();
    ui->ReadData_Edit->appendPlainText("");
}

void GUI_PROGRAMMER::on_HexFile_Button_clicked()
{
    // Select programmer file
    QString file;
    if (getOpenFilePath(&file, tr("HEX (*.hex);;All Files (*.*)")))
    {
        // Set file text
        ui->HexFile_LineEdit->setText(file);

        // Load in file preview
        on_RefreshPreview_Button_clicked();
    }
}

void GUI_PROGRAMMER::on_RefreshPreview_Button_clicked()
{
    // Get file path
    QString filePath = ui->HexFile_LineEdit->text();
    if (filePath.isEmpty()) return;

    // Reload file if it exists
    loadedHex = loadFile(filePath);

    // Reset preview
    on_HexFormat_Combo_currentIndexChanged(0);
}

void GUI_PROGRAMMER::on_BurnData_Button_clicked()
{
    // Send start of programming
    send({
             JSON_PROGRAM,
             PROGRAMNING_INFO_START
         });

    QStringList formattedHexList = ui->HexPreview_Edit->toPlainText().split('\n');
    QStringList curr;
    for (int i = 0; i < formattedHexList.length(); i++)
    {
        curr = formattedHexList[i].split(' ');
        if (curr.isEmpty() || (curr.length() < 5)) continue;

        // Send across address
        if (!curr[3].isEmpty())// && (curr[3].length() == 4))
        {
            // Clear buffers
            rcvd.clear();

            send({
                     JSON_PROGRAM,
                     PROGRAMNING_INFO_ADDRESS,
                     2
                 });
            send(QByteArray::fromHex(curr[3].toUtf8()));

            // Wait for ack back
            waitForResponse(2, 1000);

            // Check ack
            if (!checkAck(rcvd)) break;
        }

        // Send across data
        if (!curr[4].isEmpty())// && (1 < curr[4].length()))
        {
            // Clear buffers
            rcvd.clear();

            // Send next program line
            send({
                     JSON_PROGRAM,
                     PROGRAMNING_INFO_DATA,
                     (uint8_t) (curr[4].length() / 2)
                 });
            send(QByteArray::fromHex(curr[4].toUtf8()));

            // Wait for ack back
            waitForResponse(2, 1000);

            // Check ack
            if (!checkAck(rcvd)) break;
        }
    }

    // Send end of programming
    send({
             JSON_PROGRAM,
             PROGRAMNING_INFO_END
         });
}

void GUI_PROGRAMMER::on_HexFormat_Combo_currentIndexChanged(int)
{
    // Only update format if exists
    if (loadedHex.isEmpty()) return;

    // Update Hex
    ui->HexPreview_Edit->clear();
    ui->HexPreview_Edit->appendPlainText(format_hex(loadedHex));

    // Set cursor to top
    ui->HexPreview_Edit->moveCursor(QTextCursor::Start);
}

QString GUI_PROGRAMMER::format_hex(QByteArray rawHex)
{
    QStringList hexList = QString(rawHex).split('\n');

    bool ok;
    int len_data;
    QString final, curr, nn, aaaa, tt, _dd_, cc;
    for (int i = 0; i < hexList.length(); i++)
    {
        // Grab next line
        curr = hexList[i];
        if (curr.isEmpty()) continue;

        // Sort line [nnaaaatt_dd_cc]
        nn = curr.mid(0+1,2); // First two characters is length of data
        aaaa = curr.mid(2+1, 4); // Next four characters is data address
        tt = curr.mid(2+4+1,2); // Next two characters is type of record

        // Compute command length
        len_data = 2 * nn.toInt(&ok, 16);
        if (!ok) continue;

        // Finish sorting line
        _dd_ = curr.mid(2+4+2+1, len_data); // Next len_data characters is data
        cc = curr.mid(2+4+2+len_data+1, 2); // Last two characters is checksum (after data)

        // Append to final string
        final += nn + " " + tt + " " + cc + " " + aaaa + " " + _dd_ + "\n";
    }

    return final;
}

