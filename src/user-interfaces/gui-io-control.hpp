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

#ifndef GUI_IO_CONTROL_H
#define GUI_IO_CONTROL_H

// Inheritance & keys
#include "gui-base.hpp"
#include "gui-io-control-minor-keys.h"

// Mapping
#include <QMap>
#include <QList>

// UI Elements
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLayoutItem>
#include <QLabel>
#include <QComboBox>
#include <QSlider>
#include <QLineEdit>

// File & saving
#include <QFile>
#include <QTextStream>
#include <QTimer>

// Graphs
#include "../gui-helpers/gui-chart-view.hpp"

namespace Ui {
class GUI_IO_CONTROL;
}

typedef struct {
    int min;
    int max;
    int step;
    float div;
} RangeList;
#define EMPTY_RANGE RangeList{.min=0, .max=0, .step=0, .div=1.0}

typedef struct {
    QGridLayout *grid;
    uint8_t pinType;
    uint8_t minorKey;
    uint8_t cols;
} PinTypeInfo;
#define EMPTY_PIN_TYPE_INFO PinTypeInfo{.grid=nullptr, .pinType=0, .minorKey=0, .cols=0}

class GUI_IO_CONTROL : public GUI_BASE
{
    Q_OBJECT

public:
    GUI_IO_CONTROL(QWidget *parent = 0);
    ~GUI_IO_CONTROL();

    virtual void parseConfigMap(QMap<QString, QVariant> *configMap);
    virtual bool waitForDevice(uint8_t minorKey);

signals:
    void pin_update(QStringList pin_list);

public slots:
    virtual void reset_gui();
    void chart_update_request(QList<QString> data_points, GUI_CHART_ELEMENT *target_element);

protected slots:
    void recordPinValues(PinTypeInfo *pInfo);
    virtual void receive_gui(QByteArray recvData);

private slots:
    // DIO slots
    void DIO_ComboValueChanged();
    void DIO_SliderValueChanged();
    void DIO_LineEditValueChanged();

    // AIO slots
    void AIO_ComboValueChanged();
    void AIO_SliderValueChanged();
    void AIO_LineEditValueChanged();

    // Recording handlers
    void updateValues();
    void recordLogData();

    // Update handlers
    void on_StartUpdater_Button_clicked();
    void on_StopUpdater_Button_clicked();

    // Log button handlers
    void on_LogSaveLocSelect_Button_clicked();
    void on_StartLog_Button_clicked();
    void on_StopLog_Button_clicked();

    // Connection button handlers
    void on_ConnConnect_Button_clicked();
    void on_ConnSend_Button_clicked();
    void on_ConnClearRecv_Button_clicked();
    void on_ConnSaveRecv_Button_clicked();

    // Connection combo handler
    void on_ConnType_Combo_currentIndexChanged(int);

    // Graphing handler
    void on_CreatePlots_Button_clicked();

private:
    // UI element
    Ui::GUI_IO_CONTROL *ui;

    // Maps for setting control values
    QStringList pinList;
    QMap<uint8_t, QList<QHBoxLayout*>*> pinMap;
    QMap<uint8_t, QMap<QString, uint8_t>*> controlMap;
    QMap<uint8_t, QList<uint8_t>*> disabledValueSet;
    QMap<uint8_t, QMap<uint8_t, RangeList*>*> rangeMap;

    // Read variables
    QTimer DIO_READ;
    bool dio_read_requested;
    bool dio_read_requested_double;
    QTimer AIO_READ;
    bool aio_read_requested;
    bool aio_read_requested_double;

    // Log variables
    QFile *logFile;
    QTextStream *logStream;
    QTimer logTimer;
    bool logIsRecording;

    // Used for parsing read data
    uint8_t bytesPerPin;

    // AIO pin information
    QGridLayout *AIO_Grid;
    uint8_t num_AIOcols;

    // DIO pin information
    QGridLayout *DIO_Grid;
    uint8_t num_DIOcols;

    // IO grid positions
    typedef enum {
        io_label_pos = 0,
        io_combo_pos,
        io_slider_pos,
        io_line_edit_pos,
        io_num_entries
    } io_gui_positions;

    // Connection settings
    bool devConnected;
    QMap<QString, QStringList> devSettings;

    // Handle changes on the GUI
    void inputsChanged(uint8_t pinType, QObject *caller, uint8_t io_pos, QByteArray *data = nullptr);
    void updateSliderRange(QSlider *slider, RangeList *rList);

    // Get pin layout
    bool getPinLayout(uint8_t pinType, uint8_t pin_num, QLayout **itemLayout);
    bool getWidgetLayout(uint8_t pinType, QWidget *item, QLayout **itemLayout);

    // Set pin settings
    void setPinCombos(PinTypeInfo *pInfo, QList<QString> combos);
    void setConTypes(QStringList connTypes, QList<char> mapValues);

    // Add info to maps and settings
    void addPinType(uint8_t pinType);
    void addPinLayout(QHBoxLayout *pin, QList<QHBoxLayout*> *pins);
    void addComboSettings(PinTypeInfo *pInfo, QList<QString> newSettings);
    void addPinControls(uint8_t pinType, QList<QString> keys);
    void addPinRangeMap(uint8_t pinType, QList<QString> keys, QList<RangeList*> values);

    // Set GUI values
    void setValues(uint8_t minorKey, QByteArray values);

    // Get information
    bool getPinTypeInfo(uint8_t pinType, PinTypeInfo *infoPtr);

    // Range list creation
    RangeList *makeRangeList(QString rangeInfo);

    // Pin creation & deletion
    QHBoxLayout *create_pin();
    void destroy_pin(QHBoxLayout *pin);
    void destroy_unused_pins(PinTypeInfo *pInfo);
    void connect_pin(uint8_t pinType, QHBoxLayout *pin);
    void disconnect_pin(QHBoxLayout *pin);
    void update_pin_grid(PinTypeInfo *pInfo);

    // Clear maps
    void clear_all_maps();
};

#endif // GUI_IO_CONTROL_H
