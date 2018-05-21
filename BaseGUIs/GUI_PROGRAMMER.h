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
    explicit GUI_PROGRAMMER(QWidget *parent = 0);
    ~GUI_PROGRAMMER();

private:
    Ui::GUI_PROGRAMMER *ui;
};

#endif // GUI_PROGRAMMER_H
