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

    // Reset number of columns
    ui->NumColumns_LineEdit->setText("2");
    on_NumColumns_LineEdit_editingFinished();
}

void GUI_CHART_VIEW::destroy_chart_element()
{
    // Check that item is valid
    GUI_CHART_ELEMENT *item = (GUI_CHART_ELEMENT*) sender();
    if (!item) return;

    // Get index of item & number of entries in grid
    int index = ui->ChartGridLayout->indexOf((QWidget*) item);

    // Remove from charts lists
    charts.removeOne(item);

    // Delete item of interest from grid
    QLayoutItem *item_layout = ui->ChartGridLayout->takeAt(index);
    if (item_layout)
    {
        delete item_layout->widget();
        delete item_layout;
    }

    // Update chart grid
    update_chart_grid();
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

    // Append to charts
    charts.append(new_elem);

    // Update grid layout
    update_chart_grid();
}

void GUI_CHART_VIEW::on_NumColumns_LineEdit_editingFinished()
{
    // Update number of cols
    num_chart_cols = ui->NumColumns_LineEdit->text().toInt(nullptr, 10);

    // Update the grid layout
    update_chart_grid();
}

void GUI_CHART_VIEW::update_chart_grid()
{
    // Setup loop variables
    int curr_col = 0;
    int curr_row = 0;
    int num_charts = charts.length();
    QWidget *new_item;
    QLayoutItem *old_item;

    // Replace items in grid if changed
    for (int i = 0; i < num_charts; i++)
    {
        // Set grid item to current chart
        new_item = (QWidget*) charts.at(i);
        old_item = ui->ChartGridLayout->itemAtPosition(curr_row, curr_col);
        if (!old_item || (new_item != old_item->widget()))
        {
            ui->ChartGridLayout->addWidget(new_item, curr_row, curr_col);
        }

        // Increment column and adjust row if overflows
        curr_col += 1;
        if ((num_chart_cols - 1) < curr_col)
        {
            // Increment row and reset column
            curr_row += 1;
            curr_col = 0;
        }
    }
}

void GUI_CHART_VIEW::destroy_chart_elements()
{
    // Clear charts list
    charts.clear();

    // Get and delete all grid elements
    QLayoutItem *item;
    while ((item = ui->ChartGridLayout->itemAt(0)) != nullptr)
    {
        delete item->widget();
        delete item;
    }
}
