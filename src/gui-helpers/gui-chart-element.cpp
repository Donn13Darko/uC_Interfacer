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

#include <QDateTime>

// Charts & helpers
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>

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
                                           "3D Bar",
                                           "3D Surface"
                                       });

GUI_CHART_ELEMENT::GUI_CHART_ELEMENT(int type, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GUI_CHART_ELEMENT)
{
    // Setup UI
    ui->setupUi(this);

    // Set intial values
    curr_time = ((double) QDateTime::currentMSecsSinceEpoch() / GUI_HELPER::S2MS);
    chart_type = type;
    chart_element = nullptr;
    ui->Legend_CheckBox->setChecked(false);

    // Set & update chart ranges
    ui->yMin_LineEdit->setText("0.0");
    ui->yMax_LineEdit->setText("1.0");
    ui->xDuration_LineEdit->setText("60");
    on_yMin_LineEdit_editingFinished();
    on_yMax_LineEdit_editingFinished();
    on_xDuration_LineEdit_editingFinished();

    // Registar types
    qRegisterMetaType< QList<QString> >( "QList<QString>" );
    qRegisterMetaType< QList<double*> >( "QList<double*>" );

    // Connect update receiver
    connect(this, SIGNAL(update_receive(QList<double*>)),
            this, SLOT(process_update(QList<double*>)),
            Qt::QueuedConnection);

    // Setup, connect, and start the update rate
    ui->UpdateRate_LineEdit->setText("1");
    connect(&update_timer, SIGNAL(timeout()),
            this, SLOT(update_data_series()),
            Qt::DirectConnection);
    on_UpdateRate_LineEdit_editingFinished();

    // Create the chart element
    create_chart_element();
}

GUI_CHART_ELEMENT::~GUI_CHART_ELEMENT()
{
    // Delete chart element
    destroy_chart_element();
    destroy_data_map();

    // Delete ui
    delete ui;
}

int GUI_CHART_ELEMENT::get_chart_type()
{
    return chart_type;
}

QStringList GUI_CHART_ELEMENT::get_supported_chart_types()
{
    return supportedChartsList;
}

void GUI_CHART_ELEMENT::update_series_combo(QStringList new_data_series_list)
{
    // Remove added elements that don't exist anymore
    foreach (QString key, addded_data_series_map.keys())
    {
        // Remove series that don't exist anymore
        if (!new_data_series_list.contains(key))
        {
            ui->SeriesSelection_ComboBox->setCurrentText(key);
            on_Remove_Button_clicked();
        }
    }

    // Clear combo and re-add items
    ui->SeriesSelection_ComboBox->clear();
    ui->SeriesSelection_ComboBox->addItems(new_data_series_list);
}

void GUI_CHART_ELEMENT::on_UpdateRate_LineEdit_editingFinished()
{
    // Get new value
    double update_interval = ui->UpdateRate_LineEdit->text().toDouble();

    // If timeout zero, stop the timer
    // Else, start timer with timeout
    if (update_interval == 0.0)
    {
        update_timer.stop();
    } else
    {
        update_timer.start(qRound(GUI_HELPER::S2MS * update_interval));
    }
}

void GUI_CHART_ELEMENT::on_Legend_CheckBox_stateChanged(int)
{
    // Verify chart element
    if (!chart_element) return;

    // Get show value
    bool showLegend = ui->Legend_CheckBox->isChecked();

    // Create the new element
    switch (chart_type)
    {
        case CHART_TYPE_2D_LINE:
        case CHART_TYPE_2D_SCATTER:
        case CHART_TYPE_2D_BAR:
        case CHART_TYPE_2D_AREA:
        case CHART_TYPE_3D_LINE:
        case CHART_TYPE_3D_SCATTER:
        case CHART_TYPE_3D_BAR:
        case CHART_TYPE_3D_SURFACE:
        {
            ((QChartView*) chart_element)->chart()->legend()->setVisible(showLegend);
            break;
        }
        default:
            return;
    }
}

void GUI_CHART_ELEMENT::on_Exit_Button_clicked()
{
    // Parent handles exiting
    emit exit_clicked();
}

void GUI_CHART_ELEMENT::on_Add_Button_clicked()
{
    // Verify chart element
    if (!chart_element) return;

    // Get new series id
    QString series_uid = ui->SeriesSelection_ComboBox->currentText();

    // Verify not already in map
    if (series_uid.isEmpty() ||
            addded_data_series_map.contains(series_uid))
    {
        return;
    }

    // Add to chart element & data series map
    switch (chart_type)
    {
        case CHART_TYPE_2D_LINE:
        case CHART_TYPE_2D_SCATTER:
        case CHART_TYPE_2D_BAR:
        case CHART_TYPE_2D_AREA:
        case CHART_TYPE_3D_LINE:
        case CHART_TYPE_3D_SCATTER:
        case CHART_TYPE_3D_BAR:
        case CHART_TYPE_3D_SURFACE:
        {
            // Get chart and create new series
            QChart *chart = ((QChartView*) chart_element)->chart();
            QLineSeries *n_series = new QLineSeries();
            if (!chart || !n_series) break;

            // Add series to map and chart
            addded_data_series_map.insert(series_uid, n_series);
            chart->addSeries(n_series);

            // Link chart axis to series
            n_series->attachAxis(chart->axisX());
            n_series->attachAxis(chart->axisY());

            // Set series name
            n_series->setName(series_uid);
            break;
        }
        default:
            return;
    }
}

void GUI_CHART_ELEMENT::on_Remove_Button_clicked()
{
    // Verify chart element
    if (!chart_element) return;

    // Get the series id from combo (if not provided)
    QString series_uid = ui->SeriesSelection_ComboBox->currentText();

    // Verify map contains series
    if (!addded_data_series_map.contains(series_uid)) return;

    // Remove from chart element & data series map
    switch (chart_type)
    {
        case CHART_TYPE_2D_LINE:
        case CHART_TYPE_2D_SCATTER:
        case CHART_TYPE_2D_BAR:
        case CHART_TYPE_2D_AREA:
        case CHART_TYPE_3D_LINE:
        case CHART_TYPE_3D_SCATTER:
        case CHART_TYPE_3D_BAR:
        case CHART_TYPE_3D_SURFACE:
        {
            QLineSeries *n_series = (QLineSeries*) addded_data_series_map.take(series_uid);
            ((QChartView*) chart_element)->chart()->removeSeries(n_series);
            break;
        }
        default:
            return;
    }
}

void GUI_CHART_ELEMENT::on_yMin_LineEdit_editingFinished()
{
    // Set new ymin
    y_min = ui->yMin_LineEdit->text().toDouble();

    // Verify if chart exists
    if (!chart_element) return;

    // Decide how to set y min
    switch (chart_type)
    {
        case CHART_TYPE_2D_LINE:
        case CHART_TYPE_2D_SCATTER:
        case CHART_TYPE_2D_BAR:
        case CHART_TYPE_2D_AREA:
        case CHART_TYPE_3D_LINE:
        case CHART_TYPE_3D_SCATTER:
        case CHART_TYPE_3D_BAR:
        case CHART_TYPE_3D_SURFACE:
        {
            // Set y min of chart
            ((QChartView*) chart_element)->chart()->axisY()->setMin(y_min);
            break;
        }
        default:
            return;
    }
}

void GUI_CHART_ELEMENT::on_yMax_LineEdit_editingFinished()
{
    // Set new ymin
    y_max = ui->yMax_LineEdit->text().toDouble();

    // Verify if chart exists
    if (!chart_element) return;

    // Decide how to set y min
    switch (chart_type)
    {
        case CHART_TYPE_2D_LINE:
        case CHART_TYPE_2D_SCATTER:
        case CHART_TYPE_2D_BAR:
        case CHART_TYPE_2D_AREA:
        case CHART_TYPE_3D_LINE:
        case CHART_TYPE_3D_SCATTER:
        case CHART_TYPE_3D_BAR:
        case CHART_TYPE_3D_SURFACE:
        {
            // Set y min of chart
            ((QChartView*) chart_element)->chart()->axisY()->setMax(y_max);
            break;
        }
        default:
            return;
    }
}

void GUI_CHART_ELEMENT::on_xDuration_LineEdit_editingFinished()
{
    // Set new ymin
    x_duration = ui->xDuration_LineEdit->text().toDouble();

    // Verify if chart exists
    if (!chart_element) return;

    // Decide how to set y min
    switch (chart_type)
    {
        case CHART_TYPE_2D_LINE:
        case CHART_TYPE_2D_SCATTER:
        case CHART_TYPE_2D_BAR:
        case CHART_TYPE_2D_AREA:
        case CHART_TYPE_3D_LINE:
        case CHART_TYPE_3D_SCATTER:
        case CHART_TYPE_3D_BAR:
        case CHART_TYPE_3D_SURFACE:
        {
            // Set x duration for chart
            ((QChartView*) chart_element)->chart()->axisX()->setRange(curr_time - x_duration, curr_time);
            break;
        }
        default:
            return;
    }
}

void GUI_CHART_ELEMENT::update_data_series()
{
    // Get time
    curr_time = ((double) QDateTime::currentMSecsSinceEpoch() / GUI_HELPER::S2MS);

    // Update chart range
    on_xDuration_LineEdit_editingFinished();

    // Check if anything to update
    if (addded_data_series_map.isEmpty()) return;

    // Send data list update
    emit update_request(addded_data_series_map.keys());
}

void GUI_CHART_ELEMENT::process_update(QList<double*> data_values)
{
    // Verify chart element
    if (!chart_element) return;

    // Get and verify data length
    QList<QString> added_keys = addded_data_series_map.keys();
    int added_len = added_keys.length();
    if (added_len != data_values.length()) return;

    // Get new element adding technique
    switch (chart_type)
    {
        case CHART_TYPE_2D_LINE:
        case CHART_TYPE_2D_SCATTER:
        case CHART_TYPE_2D_BAR:
        case CHART_TYPE_2D_AREA:
        case CHART_TYPE_3D_LINE:
        case CHART_TYPE_3D_SCATTER:
        case CHART_TYPE_3D_BAR:
        case CHART_TYPE_3D_SURFACE:
        {
            // Add values to series
            QLineSeries *data_series;
            double *data_point;
            for (int i = 0; i < added_len; i++)
            {
                // Parse values from info
                data_point = data_values.at(i);
                data_series = (QLineSeries*) addded_data_series_map.value(added_keys.at(i));
                if (!data_point)
                {
                    continue;
                } else if (!data_series)
                {
                    delete data_point;
                    continue;
                }

                // Append to series
                data_series->append(curr_time, *data_point);

                // Delete point
                delete data_point;
            }
            break;
        }
        default:
            return;
    }
}

void GUI_CHART_ELEMENT::create_chart_element()
{
    // Verify chart element empty
    if (chart_element) return;

    // Create the new element
    switch (chart_type)
    {
        case CHART_TYPE_2D_LINE:
        case CHART_TYPE_2D_SCATTER:
        case CHART_TYPE_2D_BAR:
        case CHART_TYPE_2D_AREA:
        case CHART_TYPE_3D_LINE:
        case CHART_TYPE_3D_SCATTER:
        case CHART_TYPE_3D_BAR:
        case CHART_TYPE_3D_SURFACE:
        {
            // Create chart and widget
            QChart *n_chart = new QChart();
            chart_element = (QWidget*) new QChartView(n_chart, this);

            // Set chart legend
            n_chart->legend()->setAlignment(Qt::AlignRight);
            on_Legend_CheckBox_stateChanged(0);

            // Add basic axis
            n_chart->setAxisX(new QValueAxis());
            n_chart->setAxisY(new QValueAxis());
            n_chart->axisX()->setRange(curr_time - x_duration, curr_time);
            n_chart->axisY()->setRange(y_min, y_max);
            break;
        }
        default:
            return;
    }
    if (!chart_element) return;

    chart_element->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->ChartGridLayout->addWidget(chart_element, 0, 0, 0, 0);
}

void GUI_CHART_ELEMENT::destroy_chart_element()
{
    // Verify element valid
    if (!chart_element) return;

    // Decide how to delete
    switch (chart_type)
    {
        case CHART_TYPE_2D_LINE:
        case CHART_TYPE_2D_SCATTER:
        case CHART_TYPE_2D_BAR:
        case CHART_TYPE_2D_AREA:
        case CHART_TYPE_3D_LINE:
        case CHART_TYPE_3D_SCATTER:
        case CHART_TYPE_3D_BAR:
        case CHART_TYPE_3D_SURFACE:
        {
            // Get and delete chart
            QChart *chart = ((QChartView*) chart_element)->chart();
            delete chart->axisX();
            delete chart->axisY();
            delete chart;

            // Delete element
            delete chart_element;
            break;
        }
        default:
            return;
    }
}

void GUI_CHART_ELEMENT::destroy_data_map()
{
    // Decide how to delete
    switch (chart_type)
    {
        case CHART_TYPE_2D_LINE:
        case CHART_TYPE_2D_SCATTER:
        case CHART_TYPE_2D_BAR:
        case CHART_TYPE_2D_AREA:
        case CHART_TYPE_3D_LINE:
        case CHART_TYPE_3D_SCATTER:
        case CHART_TYPE_3D_BAR:
        case CHART_TYPE_3D_SURFACE:
        {
            // Delete all the elements
            foreach (QString key, addded_data_series_map.keys())
            {
                delete (QLineSeries*) addded_data_series_map.take(key);
            }
            break;
        }
        default:
            return;
    }
}
