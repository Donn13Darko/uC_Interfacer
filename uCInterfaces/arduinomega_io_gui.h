#ifndef ARDUINOMEGA_H
#define ARDUINOMEGA_H

#include "BaseGUIs/GUI_8AIO_16DIO_COMM.h"
#include <QMap>

namespace Ui {
class ArduinoMega;
}

class ArduinoMega : public GUI_8AIO_16DIO_COMM
{
    Q_OBJECT

public:
    explicit ArduinoMega(QWidget *parent = 0);
    ~ArduinoMega();
};

#endif // ARDUINOMEGA_H
