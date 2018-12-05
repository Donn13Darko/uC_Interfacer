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

bool GUI_IO_CONTROL_TEST_CLASS::check_pins_test(QStringList expected_pin_list,
                                                QList<QStringList> expected_combo_list,
                                                QList<QList<int>> expected_slider_list,
                                                QStringList expected_lineEdit_list,
                                                QList<bool> expected_disabled_list)
{
    // Verify all sizes are same & expected pin list matches
    if ((expected_pin_list.length() != expected_combo_list.length())
            || (expected_combo_list.length() != expected_slider_list.length())
            || (expected_slider_list.length() != expected_lineEdit_list.length())
            || (expected_lineEdit_list.length() != expected_disabled_list.length()))
    {
        show_warning("Lengths mismatch!");
        show_warning("Pin list length:", QString::number(expected_pin_list.length()));
        show_warning("Combo list length:", QString::number(expected_combo_list.length()));
        show_warning("Slider list length:", QString::number(expected_slider_list.length()));
        show_warning("LineEdit list length:", QString::number(expected_lineEdit_list.length()));
        show_warning("Disabled list length:", QString::number(expected_disabled_list.length()));
        return false;
    }

    // Verify pin list
    if (!check_pin_list_test(expected_pin_list)) return false;

    // Verify each pin
    int num_pins = expected_pin_list.length();
    for (int i = 0; i < num_pins; i++)
    {
        if (!check_pin_test(expected_pin_list.at(i), expected_combo_list.at(i),
                            expected_slider_list.at(i), expected_lineEdit_list.at(i),
                            expected_disabled_list.at(i)))
        {
            show_warning("Bad pin:", expected_pin_list.at(i));
            return false;
        }
    }

    // If reached here, all pins passed (return true)
    return true;
}

bool GUI_IO_CONTROL_TEST_CLASS::check_pin_test(QString pin_str, QStringList expected_combo,
                                               QList<int> expected_slider, QString expected_value,
                                               bool expected_disabled)
{
    // Get the pin
    QHBoxLayout *pin = get_pin_test(pin_str);
    if (!pin) return false;

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
        show_warning("Bad pin label:",
                     pin_label->text(),
                     pin_str);
        return false;
    }

    // Check combo element
    if (pin_combo->count() != expected_combo.length())
    {
        show_warning("Bad combo length:",
                     QString::number(pin_combo->count()),
                     QString::number(expected_combo.length()));
        return false;
    }

    if (pin_combo->currentText() != expected_combo.at(0))
    {
        show_warning("Bad current combo:",
                     pin_combo->currentText(),
                     expected_combo.at(0));
        return false;
    }

    foreach (QString combo_entry, expected_combo)
    {
        if (pin_combo->findText(combo_entry) == -1)
        {
            show_warning("Combo entry not found:", combo_entry);
            return false;
        }
    }

    // Check slider element
    if (pin_slider->testAttribute(Qt::WA_TransparentForMouseEvents) != expected_disabled)
    {
        show_warning("Bad clicking slider:",
                     QString::number(pin_slider->testAttribute(Qt::WA_TransparentForMouseEvents)),
                     QString::number(expected_disabled));
        return false;
    }

    int expected_len = 4;
    if ((expected_slider.length() != expected_len)
            || (pin_slider->value() != expected_slider.at(0))
            || (pin_slider->minimum() != expected_slider.at(1))
            || (pin_slider->maximum() != expected_slider.at(2))
            || (pin_slider->tickInterval() != expected_slider.at(3))
            || (pin_slider->singleStep() != expected_slider.at(3))
            || (pin_slider->pageStep() != expected_slider.at(3)))
    {
        show_warning("Slider mismatch!");
        show_warning("Expected Length:",
                     QString::number(expected_slider.length()),
                     QString::number(expected_len));
        show_warning("Value:",
                     QString::number(pin_slider->value()),
                     QString::number(expected_slider.at(0)));
        show_warning("Minimum:",
                     QString::number(pin_slider->minimum()),
                     QString::number(expected_slider.at(1)));
        show_warning("Maximum:",
                     QString::number(pin_slider->maximum()),
                     QString::number(expected_slider.at(2)));
        show_warning("Tick Interval:",
                     QString::number(pin_slider->tickInterval()),
                     QString::number(expected_slider.at(3)));
        show_warning("Single Step:",
                     QString::number(pin_slider->singleStep()),
                     QString::number(expected_slider.at(3)));
        show_warning("Page Step:",
                     QString::number(pin_slider->pageStep()),
                     QString::number(expected_slider.at(3)));
        return false;
    }

    // Check line edit element
    if (pin_lineEdit->testAttribute(Qt::WA_TransparentForMouseEvents) != expected_disabled)
    {
        show_warning("Bad clicking line edit:",
                     QString::number(pin_lineEdit->testAttribute(Qt::WA_TransparentForMouseEvents)),
                     QString::number(expected_disabled));
        return false;
    }

    if (pin_lineEdit->text() != expected_value)
    {
        show_warning("Bad line edit:",
                     pin_lineEdit->text(),
                     expected_value);
        return false;
    }

    // All elements passed (return true)
    return true;
}

bool GUI_IO_CONTROL_TEST_CLASS::perform_action_test(QString pin_str, uint8_t button, QString value)
{
    // Get pin
    QHBoxLayout *pin = get_pin_test(pin_str);
    if (!pin) return false;

    // Perform action on button
    switch (button)
    {
        case io_combo_pos:
        {
            QComboBox *combo = (QComboBox*) pin->itemAt(io_combo_pos)->widget();
            if (!combo)
            {
                show_warning("Failed to retrieve combo!");
                return false;
            }

            combo->setCurrentText(value);
            qApp->processEvents();
            return (combo->currentText() == value);
        }
        case io_slider_pos:
        {
            QSlider *slider = (QSlider*) pin->itemAt(io_slider_pos)->widget();
            if (!slider)
            {
                show_warning("Failed to retrieve slider!");
                return false;
            } else if (slider->testAttribute(Qt::WA_TransparentForMouseEvents))
            {
                show_warning("Slider disabled!");
                return false;
            }

            int value_int = value.toInt();
            slider->setSliderPosition(value_int);
            qApp->processEvents();
            return (slider->value() == value_int);
        }
        case io_line_edit_pos:
        {
            QLineEdit *lineEdit = (QLineEdit*) pin->itemAt(io_line_edit_pos)->widget();
            if (!lineEdit)
            {
                show_warning("Failed to retrieve line edit!");
                return false;
            } else if (lineEdit->testAttribute(Qt::WA_TransparentForMouseEvents))
            {
                show_warning("Line edit disabled!");
                return false;
            }

            lineEdit->clear();
            QTest::keyClicks(lineEdit, value);
            QTest::keyClick(lineEdit, Qt::Key_Enter);
            qApp->processEvents();
            return (lineEdit->text() == value);
        }
        default:
        {
            show_warning("Unknown button position:",
                         QString::number(button));
            return false;
        }
    }

    // Action completed
    return true;
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

bool GUI_IO_CONTROL_TEST_CLASS::check_pin_list_test(QStringList expected_pin_list)
{
    // Get Current pint list
    QStringList pin_list = get_pin_list();

    // Verify length
    if (pin_list.length() != expected_pin_list.length())
    {
        show_warning("Expected pins length mismatch:",
                     QString::number(pin_list.length()),
                     QString::number(expected_pin_list.length()));
        return false;
    }

    // Verify items
    foreach (QString pin, expected_pin_list)
    {
        if (!pin_list.contains(pin))
        {
            show_warning("Pin not found:", pin);
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
    if (pin_vals.length() != 2)
    {
        show_warning("Pin not found:", pin_str);
        return nullptr;
    }

    // Set values from pin str
    uint8_t pinNum = pin_vals.at(1).toInt();
    uint8_t pinType;
    if (pin_vals.at(0) == "AIO") pinType = MINOR_KEY_IO_AIO;
    else if (pin_vals.at(0) == "DIO") pinType = MINOR_KEY_IO_DIO;
    else
    {
        show_warning("Unknown pinType:", pin_vals.at(0));
        return nullptr;
    }

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
            show_warning("Unknown pinType:",
                         QString::number(pinType),
                         QString::number(MINOR_KEY_IO_AIO) + "-or-" \
                         + QString::number(MINOR_KEY_IO_DIO));
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

    // Warn and return null if not found
    show_warning("Pin not found:", QString::number(pinNum));
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

void GUI_IO_CONTROL_TEST_CLASS::show_warning(QString hint, QString value)
{
    qWarning() << hint << value;
}

void GUI_IO_CONTROL_TEST_CLASS::show_warning(QString hint, QString got, QString expected)
{
    qWarning() << hint << "Got:" << got << "Expected:" << expected;
}
