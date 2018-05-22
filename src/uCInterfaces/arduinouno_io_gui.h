#ifndef ARDUINOUNO_IO_H
#define ARDUINOUNO_IO_H

#include "../baseGUIs/GUI_8AIO_16DIO_COMM.h"
#include <QMap>

namespace Ui {
class ArduinoUno_IO;
}

class ArduinoUno_IO : public GUI_8AIO_16DIO_COMM
{
    Q_OBJECT

public:
    explicit ArduinoUno_IO(QWidget *parent = 0);
    ~ArduinoUno_IO();
};

#endif // ARDUINOUNO_IO_H
