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

#ifndef GUI_CHART_ELEMENT_H
#define GUI_CHART_ELEMENT_H

// Inheritance
#include <QWidget>

// Data members
#include <QStringList>

// Helpers
#include <QTimer>
#include <QMap>
#include "gui-generic-helper.h"

// Needs to be in same order as supportedChartsList
typedef enum {
    CHART_TYPE_ERROR = 0,

    CHART_TYPE_2D_LINE,
    CHART_TYPE_2D_SCATTER,
    CHART_TYPE_2D_BAR,
    CHART_TYPE_2D_AREA,
    CHART_TYPE_3D_LINE,
    CHART_TYPE_3D_SCATTER,
    CHART_TYPE_3D_BAR,
    CHART_TYPE_3D_SURFACE
} chart_types;

namespace Ui {
class GUI_CHART_ELEMENT;
}

class GUI_CHART_ELEMENT : public QWidget
{
    Q_OBJECT

public:
    explicit GUI_CHART_ELEMENT(int type = CHART_TYPE_ERROR, QStringList data_series = {}, QWidget *parent = 0);
    ~GUI_CHART_ELEMENT();

    static QStringList get_chart_types();

signals:
    void exit_clicked();
    void update_request(QList<QString> data_points);
    void update_receive(QList<void*> data_values);

private slots:
    void on_UpdateRate_LineEdit_editingFinished();
    void on_Legend_CheckBox_stateChanged(int);

    void on_Exit_Button_clicked();
    void on_Add_Button_clicked();
    void on_Remove_Button_clicked();

    void update_data_series();
    void process_update(QList<void*> data_values);

private:
    Ui::GUI_CHART_ELEMENT *ui;

    int chart_type;
    QWidget *chart_element;
    QMap<QString, void*> addded_data_series_map;

    double update_time;
    double update_interval;
    QTimer update_timer;

    static QStringList supportedChartsList;

    // Create or destroy the chart
    void create_chart_element();
    void destroy_chart_element();
    void destroy_data_map();
};

#endif // GUI_CHART_ELEMENT_H
