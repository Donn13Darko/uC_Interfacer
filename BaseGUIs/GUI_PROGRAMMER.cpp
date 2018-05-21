#include "GUI_PROGRAMMER.h"
#include "ui_GUI_PROGRAMMER.h"

GUI_PROGRAMMER::GUI_PROGRAMMER(QWidget *parent) :
    GUI_BASE(parent),
    ui(new Ui::GUI_PROGRAMMER)
{
    ui->setupUi(this);
}

GUI_PROGRAMMER::~GUI_PROGRAMMER()
{
    delete ui;
}
