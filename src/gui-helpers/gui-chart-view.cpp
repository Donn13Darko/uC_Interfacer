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

#include "gui-chart-view.hpp"
#include "ui_gui-chart-view.h"

GUI_CHART_VIEW::GUI_CHART_VIEW(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GUI_CHART_VIEW)
{
    // Setup ui
    ui->setupUi(this);
    setWindowFlag(Qt::WindowMinMaxButtonsHint, true);

    // Register QList<QString> metaType
    qRegisterMetaType<QList<QString>>("QList<QString>");

    // Load graph types
    ui->ChartType_Combo->addItems(GUI_CHART_ELEMENT::get_supported_chart_types());

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

    // Empty data list
    data_series_list.clear();
}

void GUI_CHART_VIEW::destroy_chart_element()
{
    // Get & check the requesting item
    GUI_CHART_ELEMENT *item = (GUI_CHART_ELEMENT*) sender();
    if (!item) return;

    // Remove item from charts lists
    charts.removeAll(item);

    // Remove item from grid view
    ui->ChartGridLayout->removeWidget((QWidget*) item);

    // Delete item
    delete item;

    // Update chart grid
    update_chart_grid();
}

void GUI_CHART_VIEW::set_data_list(QStringList new_data_series_list)
{
    // Clear existing list
    data_series_list.clear();

    // Copy new list
    data_series_list.append(new_data_series_list);

    // Update each chart with new valid list
    foreach (GUI_CHART_ELEMENT *chart_elem, charts)
    {
        chart_elem->update_series_combo(data_series_list);
    }
}

void GUI_CHART_VIEW::element_data_request(QList<QString> data_points)
{
    // Get sending element
    GUI_CHART_ELEMENT *elem = (GUI_CHART_ELEMENT*) sender();
    if (!elem) return;

    // Emit update to parent
    emit update_request(data_points, elem);
}

void GUI_CHART_VIEW::on_AddChart_Button_clicked()
{
    // Create new chart element
    GUI_CHART_ELEMENT *new_elem = new GUI_CHART_ELEMENT(ui->ChartType_Combo->currentIndex() + 1, this);
    if (!new_elem) return;

    // Set element info
    new_elem->update_series_combo(data_series_list);
    new_elem->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Connect to disconnect & remove slot
    connect(new_elem, SIGNAL(exit_clicked()),
            this, SLOT(destroy_chart_element()),
            Qt::QueuedConnection);

    // Connect to update slots
    connect(new_elem, SIGNAL(update_request(QList<QString>)),
            this, SLOT(element_data_request(QList<QString>)),
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

    // Disable spacer if charts
    if (num_charts != 0)
    {
        ui->ChartVSpacerBottom->changeSize(20, 0,
                                          QSizePolicy::Expanding, QSizePolicy::Maximum);
    } else
    {
        ui->ChartVSpacerBottom->changeSize(20, 16777215,
                                          QSizePolicy::Expanding, QSizePolicy::Maximum);;
    }

    /** Update entries in grid **/

    // Replace items in grid if changed
    for (int i = 0; i < num_charts; i++)
    {
        // Set column stretchs during first row
        if (curr_row == 0)
        {
            ui->ChartGridLayout->setColumnStretch(curr_col, 1);
        }

        // Set row stretchs at first column
        if (curr_col == 0)
        {
            ui->ChartGridLayout->setRowStretch(curr_row, 1);
        }

        // Set grid item to current chart
        new_item = (QWidget*) charts.at(i);
        old_item = ui->ChartGridLayout->itemAtPosition(curr_row, curr_col);
        if (!old_item)
        {
            ui->ChartGridLayout->addWidget(new_item, curr_row, curr_col, 1, 1);
        } else if (new_item != old_item->widget())
        {
            ui->ChartGridLayout->removeItem(old_item);
            ui->ChartGridLayout->addWidget(new_item, curr_row, curr_col, 1, 1);
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

    /** Handle streching & sizing **/

    // Get last used row
    if (curr_col != 0) curr_row += 1;

    // Set unused row strech to zero
    int grid_rows = ui->ChartGridLayout->rowCount();
    while (curr_row < grid_rows)
    {
        ui->ChartGridLayout->setRowStretch(curr_row++, 0);
    }

    // Set unused column strech to zero
    curr_col = num_chart_cols;
    int grid_cols = ui->ChartGridLayout->columnCount();
    while (curr_col < grid_cols)
    {
        ui->ChartGridLayout->setColumnStretch(curr_col++, 0);
    }
}

void GUI_CHART_VIEW::destroy_chart_elements()
{
    // Clear charts list
    charts.clear();

    // Get and delete all grid elements
    QLayoutItem *item;
    QWidget *itemWidget;
    while ((item = ui->ChartGridLayout->takeAt(0)))
    {
        if (item != NULL)
        {
            itemWidget = item->widget();
            if (itemWidget != NULL) delete itemWidget;
            delete item;
        }
    }
}
