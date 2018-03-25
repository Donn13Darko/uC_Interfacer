#include "uCInterfaces/arduinouno_io_gui.h"
#include "ui_GUI_8AIO_16DIO_COMM.h"

#include <QMessageBox>
#include <QTimer>

ArduinoUno_IO::ArduinoUno_IO(QStringList params, QWidget *parent) :
    GUI_8AIO_16DIO_COMM(params, this)
{
    // Add new pin settings
    addNewPinSettings(  {JSON_DIO},
                        {"PWM", "Servo Deg", "Servo uS"},
                        {IO_PWM, IO_SERVO_DEG, IO_SERVO_US},
                        {false, false, false},
                        {
                          {.min=0, .max=100, .step=5, .div=1},
                          {.min=0, .max=360, .step=5, .div=1},
                          {.min=0, .max=3000, .step=5, .div=1}
                        }
                      );


    // Set combo values for pins
    setCombos(  JSON_AIO, {"Input"});
    setCombos(  JSON_DIO,
                {"Input", "Output", "Servo Deg", "Servo uS"});
    setCombos(  JSON_DIO,
                {"Input", "Output", "PWM", "Servo Deg", "Servo uS"},
                {3, 5, 6, 9, 10, 11});

    // Remove Extra Pins
    disablePins(JSON_AIO, {6, 7});
    disablePins(JSON_DIO, {14, 15});

    // Set Device pin count
    num_AIOpins_DEV = 6;
    num_DIOpins_DEV = 14;
}

ArduinoUno_IO::~ArduinoUno_IO()
{
}
