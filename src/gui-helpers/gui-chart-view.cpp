/*
 * uC Interface - A GUI for Programming & Interfacing with Microcontrollers
 * Copyright (C) 2018  Mitchell Oleson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "gui-chart-view.h"
#include "ui_gui-chart-view.h"

GUI_CHART_VIEW::GUI_CHART_VIEW(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GUI_CHART_VIEW)
{
    // Setup ui
    ui->setupUi(this);

    // Reset GUI
    reset_gui();
}

GUI_CHART_VIEW::~GUI_CHART_VIEW()
{
    delete ui;
}

void GUI_CHART_VIEW::reset_gui()
{
}
