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

#ifndef GUI_CHART_VIEW_H
#define GUI_CHART_VIEW_H

#include <QDialog>

#include "gui-chart-element.hpp"

namespace Ui {
class GUI_CHART_VIEW;
}

class GUI_CHART_VIEW : public QDialog
{
    Q_OBJECT

public:
    explicit GUI_CHART_VIEW(QWidget *parent = 0);
    ~GUI_CHART_VIEW();

signals:
    void update_request(QList<QString> data_points, GUI_CHART_ELEMENT *target_element);

public slots:
    void reset_gui();
    void destroy_chart_element();
    void set_data_list(QStringList new_data_series_list);
    void element_data_request(QList<QString> data_points);

private slots:
    void on_AddChart_Button_clicked();
    void on_NumColumns_LineEdit_editingFinished();

private:
    Ui::GUI_CHART_VIEW *ui;

    int num_chart_cols;
    QList<GUI_CHART_ELEMENT*> charts;

    QStringList data_series_list;

    void update_chart_grid();
    void destroy_chart_elements();
};

#endif // GUI_CHART_VIEW_H
