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

#include "gui-io-control-test-class.hpp"
#include "ui_gui-io-control.h"

#include <QTest>
#include <QSignalSpy>

GUI_IO_CONTROL_TEST_CLASS::GUI_IO_CONTROL_TEST_CLASS(QWidget *parent) :
    GUI_IO_CONTROL(parent)
{
    // Retrieve ui object
    ui_ptr = get_ui();

    // Show UI (needs to be visible for UI events)
    show();
}

GUI_IO_CONTROL_TEST_CLASS::~GUI_IO_CONTROL_TEST_CLASS()
{
    /* DO NOTHING */
}

QByteArray GUI_IO_CONTROL_TEST_CLASS::rcvd_formatted_readAll_test()
{
    return rcvd_formatted_readAll();
}

qint64 GUI_IO_CONTROL_TEST_CLASS::rcvd_formatted_size_test()
{
    return rcvd_formatted_size();
}

void GUI_IO_CONTROL_TEST_CLASS::set_aio_update_rate_test(float rate)
{
    QTest::keyClicks(ui_ptr->AIO_UR_LineEdit, QString::number(rate));
    qApp->processEvents();
}

float GUI_IO_CONTROL_TEST_CLASS::get_aio_update_rate_test()
{
    return ui_ptr->AIO_UR_LineEdit->text().toFloat();
}

void GUI_IO_CONTROL_TEST_CLASS::set_dio_update_rate_test(float rate)
{
    QTest::keyClicks(ui_ptr->DIO_UR_LineEdit, QString::number(rate));
    qApp->processEvents();
}

float GUI_IO_CONTROL_TEST_CLASS::get_dio_update_rate_test()
{
    return ui_ptr->DIO_UR_LineEdit->text().toFloat();
}

QString GUI_IO_CONTROL_TEST_CLASS::get_update_rate_start_text_test()
{
    return ui_ptr->StartUpdater_Button->text();
}

void GUI_IO_CONTROL_TEST_CLASS::update_rate_start_clicked_test()
{
    QTest::mouseClick(ui_ptr->StartUpdater_Button, Qt::LeftButton);
    qApp->processEvents();
}

void GUI_IO_CONTROL_TEST_CLASS::update_rate_stop_clicked_test()
{
    QTest::mouseClick(ui_ptr->StopUpdater_Button, Qt::LeftButton);
    qApp->processEvents();
}

void GUI_IO_CONTROL_TEST_CLASS::set_log_file_save_path_test(QString filePath)
{
    QTest::keyClicks(ui_ptr->LogSaveLoc_LineEdit, filePath);
    qApp->processEvents();
}

QString GUI_IO_CONTROL_TEST_CLASS::get_log_file_save_path_test()
{
    return ui_ptr->LogSaveLoc_LineEdit->text();
}

void GUI_IO_CONTROL_TEST_CLASS::set_log_file_update_rate_test(float rate)
{
    QTest::keyClicks(ui_ptr->LOG_UR_LineEdit, QString::number(rate));
    qApp->processEvents();
}

float GUI_IO_CONTROL_TEST_CLASS::get_log_file_update_rate_test()
{
    return ui_ptr->LOG_UR_LineEdit->text().toFloat();
}

void GUI_IO_CONTROL_TEST_CLASS::set_log_append_checked_test(bool b)
{
    set_checked_click_test(ui_ptr->AppendLog_CheckBox, b);
}

bool GUI_IO_CONTROL_TEST_CLASS::get_log_append_checked_test()
{
    return ui_ptr->AppendLog_CheckBox->isChecked();
}

QString GUI_IO_CONTROL_TEST_CLASS::get_log_start_text_test()
{
    return ui_ptr->StartLog_Button->text();
}

void GUI_IO_CONTROL_TEST_CLASS::log_start_clicked_test()
{
    QTest::mouseClick(ui_ptr->StartLog_Button, Qt::LeftButton);
    qApp->processEvents();
}

void GUI_IO_CONTROL_TEST_CLASS::log_stop_clicked_test()
{
    QTest::mouseClick(ui_ptr->StopLog_Button, Qt::LeftButton);
    qApp->processEvents();
}

bool GUI_IO_CONTROL_TEST_CLASS::reset_clicked_test()
{
    // Setup spy to catch tranmit signal
    QList<QVariant> spy_args;
    QSignalSpy transmit_chunk_spy(this, transmit_chunk);
    if (!transmit_chunk_spy.isValid()) return false;

    // Click the reset button
    QTest::mouseClick(ui_ptr->ResetGUI_Button, Qt::LeftButton);
    qApp->processEvents();

    // Verify that reset signal emitted
    if (transmit_chunk_spy.count() != 1) return false;
    spy_args = transmit_chunk_spy.takeFirst();
    return ((spy_args.at(0).toInt() == MAJOR_KEY_RESET)
            && (spy_args.at(1).toInt() == 0));
}

bool GUI_IO_CONTROL_TEST_CLASS::check_pins_test(QStringList expected_pin_list,
                                                QList<QStringList> expected_combo_list,
                                                QList<QList<int>> expected_slider_list,
                                                QStringList expected_lineEdit_list)
{
    // Verify all sizes are same & expected pin list matches
    if ((expected_pin_list.length() != expected_combo_list.length())
            || (expected_combo_list.length() != expected_slider_list.length())
            || (expected_slider_list.length() != expected_lineEdit_list.length()))
    {
        qWarning() << "Lengths mismatch.";
        qWarning() << "Pin list length:" << QString::number(expected_pin_list.length());
        qWarning() << "Combo list length:" << QString::number(expected_combo_list.length());
        qWarning() << "Slider list length:" << QString::number(expected_slider_list.length());
        qWarning() << "LineEdit list length:" << QString::number(expected_lineEdit_list.length());
        return false;
    }

    // Verify pin list
    if (!check_pin_list_test(expected_pin_list))
    {
        qWarning() << "Eccpected pin mismatch.";
        return false;
    }

    // Verify each pin
    int num_pins = expected_pin_list.length();
    for (int i = 0; i < num_pins; i++)
    {
        if (!check_pin_test(expected_pin_list.at(i), expected_combo_list.at(i),
                            expected_slider_list.at(i), expected_lineEdit_list.at(i)))
        {
            qWarning() << "Bad pin:" << expected_pin_list.at(i);
            return false;
        }
    }

    // If reached here, all pins passed (return true)
    return true;
}

bool GUI_IO_CONTROL_TEST_CLASS::check_pin_test(QString pin_str, QStringList expected_combo,
                                               QList<int> expected_slider, QString expected_value)
{
    // Get the pin
    QHBoxLayout *pin = get_pin_test(pin_str);
    if (!pin)
    {
        qWarning() << "Pin not found:" << pin_str;
        return false;
    }

    // Setup member widget holders
    QLabel *pin_label = (QLabel*) pin->itemAt(io_label_pos)->widget();
    QComboBox *pin_combo = (QComboBox*) pin->itemAt(io_combo_pos)->widget();;
    QSlider *pin_slider = (QSlider*) pin->itemAt(io_slider_pos)->widget();;
    QLineEdit *pin_lineEdit = (QLineEdit*) pin->itemAt(io_line_edit_pos)->widget();;

    // Generated expected values helpers
    QString pin_num_str = pin_str.split("_").at(1);

    // Check label element
    if (pin_label->text() != pin_num_str)
    {
        qWarning() << "Pin label bad.";
        qWarning() << "Got:" << pin_label->text() << "Expected:" << pin_str;
        return false;
    }

    // Check combo element
    if (pin_combo->count() != expected_combo.length())
    {
        qWarning() << "Bad combo length: Got:" << QString::number(pin_combo->count()) \
                   << "Expected:" << QString::number(expected_combo.length());
        return false;
    }

    if (pin_combo->currentText() != expected_combo.at(0))
    {
        qWarning() << "Bad current element: Got:" << pin_combo->currentText() \
                   << "Expected:" << expected_combo.at(0);
        return false;
    }

    foreach (QString combo_entry, expected_combo)
    {
        if (pin_combo->findText(combo_entry) == -1)
        {
            qWarning() << "Combo entry not found:" << combo_entry;
            return false;
        }
    }

    // Check slider element
    if ((expected_slider.length() != 4)
            || (pin_slider->value() != expected_slider.at(0))
            || (pin_slider->minimum() != expected_slider.at(1))
            || (pin_slider->maximum() != expected_slider.at(2))
            || (pin_slider->tickInterval() != expected_slider.at(3))
            || (pin_slider->singleStep() != expected_slider.at(3))
            || (pin_slider->pageStep() != expected_slider.at(3)))
    {
        qWarning() << "Slider mismatch.";
        qWarning() << "Expected Length: Got:" << expected_slider.length() \
                   << "Expected: 4";
        qWarning() << "Value: Got:" << QString::number(pin_slider->value()) \
                   << "Expected:" << QString::number(expected_slider.at(0));
        qWarning() << "Minimum: Got:" << QString::number(pin_slider->minimum()) \
                   << "Expected:" << QString::number(expected_slider.at(1));
        qWarning() << "Maximum: Got:" << QString::number(pin_slider->maximum()) \
                   << "Expected:" << QString::number(expected_slider.at(2));
        qWarning() << "Tick Interval: Got:" << QString::number(pin_slider->tickInterval()) \
                   << "Expected:" << QString::number(expected_slider.at(3));
        qWarning() << "Single Step: Got:" << QString::number(pin_slider->singleStep()) \
                   << "Expected:" << QString::number(expected_slider.at(3));
        qWarning() << "Page Step: Got:" << QString::number(pin_slider->pageStep()) \
                   << "Expected:" << QString::number(expected_slider.at(3));
        return false;
    }

    // Check line edit element
    if (pin_lineEdit->text() != expected_value)
    {
        qWarning() << "Pin line edit bad.";
        qWarning() << "Got:" << pin_lineEdit->text() << "Expected:" << expected_value;
        return false;
    }

    // All elements passed (return true)
    return true;
}

bool GUI_IO_CONTROL_TEST_CLASS::check_pin_list_test(QStringList expected_pin_list)
{
    // Get Current pint list
    QStringList pin_list = get_pin_list();

    // Verify length
    if (pin_list.length() != expected_pin_list.length())
    {
        qWarning() << "Expected pins list length mismatch.";
        qWarning() << "Got:" << QString::number(pin_list.length()) \
                   << "Expected:" << QString::number(expected_pin_list.length());
        return false;
    }

    // Verify items
    foreach (QString pin, expected_pin_list)
    {
        if (!pin_list.contains(pin))
        {
            qWarning() << "Pin not found:" << pin;
            return false;
        }
    }

    // If reached, expected matches actual (return true)
    return true;
}

QHBoxLayout *GUI_IO_CONTROL_TEST_CLASS::get_pin_test(QString pin_str)
{
    // Parse pin str
    QStringList pin_vals = pin_str.split("_");
    if (pin_vals.length() != 2) return nullptr;

    // Set values from pin str
    uint8_t pinNum = pin_vals.at(1).toInt();
    uint8_t pinType;
    if (pin_vals.at(0) == "AIO") pinType = MINOR_KEY_IO_AIO;
    else if (pin_vals.at(0) == "DIO") pinType = MINOR_KEY_IO_DIO;
    else return nullptr;

    // Get and return the pin
    return get_pin_test(pinType, pinNum);
}

QHBoxLayout *GUI_IO_CONTROL_TEST_CLASS::get_pin_test(uint8_t pinType, uint8_t pinNum)
{
    // Get relevant grid
    QGridLayout *pin_grid;
    switch (pinType)
    {
        case MINOR_KEY_IO_AIO:
        {
            pin_grid = (QGridLayout*) ui_ptr->AIOVLayout->itemAt(1)->layout();
            break;
        }
        case MINOR_KEY_IO_DIO:
        {
            pin_grid = (QGridLayout*) ui_ptr->DIOVLayout->itemAt(1)->layout();
            break;
        }
        default:
        {
            return nullptr;
        }
    }

    // Get the pin from the grid
    int curr_pin_num;
    QHBoxLayout *pin;
    for (int i = 0; i < pin_grid->count(); i++)
    {
        pin = (QHBoxLayout*) pin_grid->itemAt(i)->layout();
        curr_pin_num = ((QLabel*) pin->itemAt(io_label_pos)->widget())->text().toInt(nullptr, 10);
        if (curr_pin_num == pinNum)
        {
            return pin;
        }
    }

    // Return null if not found
    return nullptr;
}

void GUI_IO_CONTROL_TEST_CLASS::set_checked_click_test(QCheckBox *check, bool b)
{
    if (b && !check->isChecked())
        QTest::mouseClick(check, Qt::LeftButton);
    else if (!b && check->isChecked())
        QTest::mouseClick(check, Qt::LeftButton);

    qApp->processEvents();
}
