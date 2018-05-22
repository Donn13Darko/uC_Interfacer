#include "uCInterfaces/arduinomega_io_gui.h"
#include "ui_GUI_8AIO_16DIO_COMM.h"

#include <QMessageBox>
#include <QTimer>

ArduinoMega::ArduinoMega(QWidget *parent) :
    GUI_8AIO_16DIO_COMM(parent)
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
}

ArduinoMega::~ArduinoMega()
{
}
