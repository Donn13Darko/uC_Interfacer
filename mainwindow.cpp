#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "uCInterfaces/arduinouno_io_gui.h"
#include "BaseGUIs/GUI_DATA_TRANSMIT.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Set default support values
    supportMapper = {
        {"Arduino Uno/Genuino",
         {{connSel,
          {{"RS-232",
           Serial_RS232::Baudrate_Defaults
          }}
         },
         {guiSel,
          {{guiSel,
           {"I/O Controller", "Data Transmitter"}
          }}
         }}
        }
    };

    // Add specified values to combos
    ui->DeviceCombo->clear();
    ui->DeviceCombo->addItems(supportMapper.keys());

    // Set Initial values
    on_DeviceCombo_currentIndexChanged(ui->DeviceCombo->currentIndex());
    on_ConnTypeCombo_currentIndexChanged(ui->ConnTypeCombo->currentIndex());
    on_SpeedCombo_currentIndexChanged(ui->SpeedCombo->currentIndex());
    on_GUITypeCombo_currentIndexChanged(ui->GUITypeCombo->currentIndex());

    ui->ConnInfoEdit->clear();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_DeviceCombo_currentIndexChanged(int)
{
    // Update Changed Data
    deviceType = ui->DeviceCombo->currentText();

    // Update Main GUI
    updateTypeCombos();
}

void MainWindow::on_ConnTypeCombo_currentIndexChanged(int)
{
    // Update Changed Data
    connType = ui->ConnTypeCombo->currentText();

    // Process changes to the speed combo
    updateSpeedCombo();
}

void MainWindow::on_SpeedCombo_currentIndexChanged(int)
{
    // Update Changed Data
    speed = ui->SpeedCombo->currentText();
}

void MainWindow::on_GUITypeCombo_currentIndexChanged(int)
{
    // Update Changed Data
    guiType = ui->GUITypeCombo->currentText();
}

void MainWindow::on_DeviceConnect_clicked()
{
    // Get connection info
    connInfo = ui->ConnInfoEdit->text();
    QStringList uCConn;
    uCConn << deviceType << connType;
    uCConn << speed << connInfo << guiType;

    // Create and attempt to run instance of device class
    QDialog *gui;
    if (deviceType == "Arduino Uno/Genuino")
    {
        if (guiType == "I/O Controller")
            gui = (QDialog*) new ArduinoUno_IO(uCConn, this);
        else if (guiType == "Data Transmitter")
            gui = (QDialog*) new GUI_DATA_TRANSMIT(uCConn, 48, this);
        else
            return;
    } else
    {
        GUI_BASE::showMessage("Error: Unsupported configuration selected!");
        return;
    }

    gui->setAttribute(Qt::WA_DeleteOnClose, true);
    gui->show();
}

void MainWindow::updateTypeCombos()
{
    QStringList connTypes = supportMapper.value(deviceType).value(connSel).keys();
    ui->ConnTypeCombo->clear();
    ui->ConnTypeCombo->addItems(connTypes);

    QStringList devGUIs = supportMapper.value(deviceType).value(guiSel).value(guiSel);
    ui->GUITypeCombo->clear();
    ui->GUITypeCombo->addItems(devGUIs);

    on_ConnTypeCombo_currentIndexChanged(ui->ConnTypeCombo->currentIndex());
    on_GUITypeCombo_currentIndexChanged(ui->GUITypeCombo->currentIndex());
}

void MainWindow::updateSpeedCombo()
{
    ui->SpeedCombo->clear();
    ui->SpeedCombo->setEnabled(true);

    QStringList newItems = supportMapper.value(deviceType).value(connSel).value(connType);
    if (newItems.length() == 0) ui->SpeedCombo->setEnabled(false);
    else ui->SpeedCombo->addItems(newItems);

    on_SpeedCombo_currentIndexChanged(ui->SpeedCombo->currentIndex());
}
