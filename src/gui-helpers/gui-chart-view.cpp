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

#include "gui-chart-element.h"

GUI_CHART_VIEW::GUI_CHART_VIEW(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GUI_CHART_VIEW)
{
    // Setup ui
    ui->setupUi(this);
    setWindowFlag(Qt::WindowMinMaxButtonsHint, true);

    // Load graph types
    ui->ChartType_Combo->addItems(GUI_CHART_ELEMENT::get_chart_types());

    // Reset GUI
    reset_gui();
}

GUI_CHART_VIEW::~GUI_CHART_VIEW()
{
    // Add function to close all open charts
    destroy_chart_elements();

    // Delete local elements
    delete ui;
}

void GUI_CHART_VIEW::reset_gui()
{
    // Remove all charts
    destroy_chart_elements();
}

void GUI_CHART_VIEW::destroy_chart_element()
{
    // Check that item is valid
    GUI_CHART_ELEMENT *item = (GUI_CHART_ELEMENT*) sender();
    if (!item) return;

    // Get index of item & number of entries in grid
    int index = ui->ChartGridLayout->indexOf((QWidget*) item);
    int count = ui->ChartGridLayout->count() - 1;

    // Delete item of interest
    QLayoutItem *item_layout = ui->ChartGridLayout->takeAt(index);
    if (item_layout)
    {
        delete item_layout->widget();
        delete item_layout;
    }

    // Setup variables
    int row, col, rowSpan, colSpan, i;

    // Set i and loop until done complete
    i = index;
    while (i < count)
    {
        // Get row & column of current index
        ui->ChartGridLayout->getItemPosition(i, &row, &col, &rowSpan, &colSpan);

        // Replace current index with item at next index
        ui->ChartGridLayout->addItem(ui->ChartGridLayout->takeAt(++i),
                                     row, col, rowSpan, colSpan);
    }
}

void GUI_CHART_VIEW::on_AddChart_Button_clicked()
{
    // Create new chart element
    GUI_CHART_ELEMENT *new_elem = \
            new GUI_CHART_ELEMENT(ui->ChartType_Combo->currentIndex() + 1, this);

    // Connect to disconnect & remove slot
    connect(new_elem, SIGNAL(exit_clicked()),
            this, SLOT(destroy_chart_element()),
            Qt::QueuedConnection);

    // Get next grid position
    int row, col, rowSpan, colSpan;
    ui->ChartGridLayout->getItemPosition(ui->ChartGridLayout->count(),
                                         &row, &col, &rowSpan, &colSpan);

    // Add to grid
    ui->ChartGridLayout->addWidget((QWidget*) new_elem, row, col);
}

void GUI_CHART_VIEW::destroy_chart_elements()
{
    // Element holder
    QLayoutItem *item;

    // Get and delete all elements
    while ((item = ui->ChartGridLayout->itemAt(0)) != nullptr)
    {
        delete item->widget();
        delete item;
    }
}
