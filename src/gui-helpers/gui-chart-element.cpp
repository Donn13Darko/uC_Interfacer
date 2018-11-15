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

#include "gui-chart-element.h"
#include "ui_gui-chart-element.h"

QT_CHARTS_USE_NAMESPACE

// Setup supported Charts list
QStringList
GUI_CHART_ELEMENT::supportedChartsList({
                                           "2D Line",
                                           "2D Scatter",
                                           "2D Bar",
                                           "2D Area",
                                           "3D Line",
                                           "3D Scatter",
                                           "3D Surface",
                                           "3D Surface"
                                       });

GUI_CHART_ELEMENT::GUI_CHART_ELEMENT(int type, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GUI_CHART_ELEMENT)
{
    // Setup UI
    ui->setupUi(this);

    // Set start update rate
    ui->UpdateRate_LineEdit->setText("1");

    // Set and create the desired chart
    chart_type = type;
    create_chart_element();
}

GUI_CHART_ELEMENT::~GUI_CHART_ELEMENT()
{
    // Delete chart element
    destroy_chart_element();

    // Delete ui
    delete ui;
}

QStringList GUI_CHART_ELEMENT::get_chart_types()
{
    return supportedChartsList;
}

void GUI_CHART_ELEMENT::on_Exit_Button_clicked()
{
    emit exit_clicked();
}

void GUI_CHART_ELEMENT::create_chart_element()
{
    switch (chart_type)
    {
        case CHART_TYPE_2D_LINE:
        case CHART_TYPE_2D_SCATTER:
        case CHART_TYPE_2D_BAR:
        case CHART_TYPE_3D_LINE:
        case CHART_TYPE_3D_SCATTER:
        case CHART_TYPE_3D_BAR:
        case CHART_TYPE_3D_SURFACE:
            chart_element = (QWidget*) new QChartView(new QChart(), this);
            break;
        default:
            return;
    }
    chart_element->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->ChartGridLayout->addWidget(chart_element, 0, 0, 0, 0);
}

void GUI_CHART_ELEMENT::destroy_chart_element()
{
    switch (chart_type)
    {
        case CHART_TYPE_2D_LINE:
        case CHART_TYPE_2D_SCATTER:
        case CHART_TYPE_2D_BAR:
        case CHART_TYPE_3D_LINE:
        case CHART_TYPE_3D_SCATTER:
        case CHART_TYPE_3D_BAR:
        case CHART_TYPE_3D_SURFACE:
        {
            delete ((QChartView*) chart_element)->chart();
            delete chart_element;
            break;
        }
        default:
            return;
    }
}
