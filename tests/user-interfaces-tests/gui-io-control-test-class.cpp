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

void GUI_IO_CONTROL_TEST_CLASS::set_connType_combo_test(QString combo_value)
{
    ui_ptr->ConnType_Combo->setCurrentText(combo_value);
    qApp->processEvents();
}

QString GUI_IO_CONTROL_TEST_CLASS::get_connType_combo_test()
{
    return ui_ptr->ConnType_Combo->currentText();
}

void GUI_IO_CONTROL_TEST_CLASS::set_connDevice_combo_test(QString combo_value)
{
    ui_ptr->ConnDeviceNum_Combo->setCurrentText(combo_value);
    qApp->processEvents();
}

QString GUI_IO_CONTROL_TEST_CLASS::get_connDevice_combo_test()
{
    return ui_ptr->ConnDeviceNum_Combo->currentText();
}

void GUI_IO_CONTROL_TEST_CLASS::set_connSpeed_combo_test(QString combo_value)
{
    ui_ptr->ConnSpeed_Combo->setCurrentText(combo_value);
    qApp->processEvents();
}

QString GUI_IO_CONTROL_TEST_CLASS::get_connSpeed_combo_test()
{
    return ui_ptr->ConnSpeed_Combo->currentText();
}

void GUI_IO_CONTROL_TEST_CLASS::set_connAddr_combo_test(QString combo_value)
{
    ui_ptr->ConnAddr_Combo->setCurrentText(combo_value);
    qApp->processEvents();
}

QString GUI_IO_CONTROL_TEST_CLASS::get_connAddr_combo_test()
{
    return ui_ptr->ConnAddr_Combo->currentText();
}

void GUI_IO_CONTROL_TEST_CLASS::set_conn_msg_data_test(QString msg)
{
    ui_ptr->ConnMsg_PlainText->clear();
    QTest::keyClicks(ui_ptr->ConnMsg_PlainText, msg);
    qApp->processEvents();
}

QByteArray GUI_IO_CONTROL_TEST_CLASS::get_conn_recv_data_test()
{
    return ui_ptr->ConnRecv_PlainText->toPlainText().toLatin1();
}

QString GUI_IO_CONTROL_TEST_CLASS::get_conn_connectButton_text_test()
{
    return ui_ptr->ConnSend_Button->text();
}

void GUI_IO_CONTROL_TEST_CLASS::conn_connect_clicked_test()
{
    QTest::mouseClick(ui_ptr->ConnConnect_Button, Qt::LeftButton);
    qApp->processEvents();
}

void GUI_IO_CONTROL_TEST_CLASS::conn_send_clicked_test()
{
    QTest::mouseClick(ui_ptr->ConnSend_Button, Qt::LeftButton);
    qApp->processEvents();
}

void GUI_IO_CONTROL_TEST_CLASS::set_aio_update_rate_test(float rate)
{
    ui_ptr->AIO_UR_LineEdit->clear();
    QTest::keyClicks(ui_ptr->AIO_UR_LineEdit, QString::number(rate));
    qApp->processEvents();
}

float GUI_IO_CONTROL_TEST_CLASS::get_aio_update_rate_test()
{
    return ui_ptr->AIO_UR_LineEdit->text().toFloat();
}

void GUI_IO_CONTROL_TEST_CLASS::set_dio_update_rate_test(float rate)
{
    ui_ptr->DIO_UR_LineEdit->clear();
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
    ui_ptr->LogSaveLoc_LineEdit->clear();
    QTest::keyClicks(ui_ptr->LogSaveLoc_LineEdit, filePath);
    qApp->processEvents();
}

QString GUI_IO_CONTROL_TEST_CLASS::get_log_file_save_path_test()
{
    return ui_ptr->LogSaveLoc_LineEdit->text();
}

void GUI_IO_CONTROL_TEST_CLASS::set_log_file_update_rate_test(float rate)
{
    ui_ptr->LOG_UR_LineEdit->clear();
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

void GUI_IO_CONTROL_TEST_CLASS::log_start_clicked_force_test()
{
    // Get if button was enabled
    bool wasEnabled = ui_ptr->StartLog_Button->isEnabled();

    // Set enabled and click
    ui_ptr->StartLog_Button->setEnabled(true);
    QTest::mouseClick(ui_ptr->StartLog_Button, Qt::LeftButton);
    qApp->processEvents();

    // Reset to previous state
    ui_ptr->StartLog_Button->setEnabled(wasEnabled);
}

void GUI_IO_CONTROL_TEST_CLASS::log_stop_clicked_test()
{
    QTest::mouseClick(ui_ptr->StopLog_Button, Qt::LeftButton);
    qApp->processEvents();
}

void GUI_IO_CONTROL_TEST_CLASS::request_read_all_test(uint8_t pinType)
{
    request_read_all(pinType);
}

void GUI_IO_CONTROL_TEST_CLASS::request_read_pin_test(uint8_t pinType, uint8_t pinNum)
{
    request_read_pin(pinType, pinNum);
}

bool GUI_IO_CONTROL_TEST_CLASS::set_pin_test(QString pin_str, QString combo_value, int slider_value)
{
    // Get pin
    QHBoxLayout *pin = get_pin_test(pin_str);
    if (!pin) return false;

    // Get items
    QComboBox *pin_combo = (QComboBox*) pin->itemAt(io_combo_pos)->widget();
    QSlider *pin_slider = (QSlider*) pin->itemAt(io_slider_pos)->widget();
    if (!(pin_combo && pin_slider))
    {
        show_warning("Failed to retrieve pin combo and/or slider!");
        return false;
    }

    // Set & process combo info
    pin_combo->setCurrentText(combo_value);
    qApp->processEvents();

    // Set & process slider info
    pin_slider->setValue(slider_value);
    qApp->processEvents();

    // Pin set complete
    return true;
}

bool GUI_IO_CONTROL_TEST_CLASS::perform_action_test(QString pin_str, uint8_t button, QVariant value)
{
    // Get pin
    QHBoxLayout *pin = get_pin_test(pin_str);
    if (!pin) return false;

    // Perform action on button
    switch (button)
    {
        case io_combo_pos:
        {
            QComboBox *pin_combo = (QComboBox*) pin->itemAt(io_combo_pos)->widget();
            if (!pin_combo)
            {
                show_warning("Failed to retrieve pin combo!");
                return false;
            }

            pin_combo->setCurrentText(value.toString());
            qApp->processEvents();

            break;
        }
        case io_slider_pos:
        {
            QSlider *pin_slider = (QSlider*) pin->itemAt(io_slider_pos)->widget();
            if (!pin_slider)
            {
                show_warning("Failed to retrieve pin slider!");
                return false;
            } else if (pin_slider->testAttribute(Qt::WA_TransparentForMouseEvents))
            {
                show_warning("Pin slider disabled!");
                return false;
            }

            pin_slider->setValue(value.toInt());
            qApp->processEvents();

            break;
        }
        case io_line_edit_pos:
        {
            QLineEdit *pin_lineEdit = (QLineEdit*) pin->itemAt(io_line_edit_pos)->widget();
            if (!pin_lineEdit)
            {
                show_warning("Failed to retrieve pin line edit!");
                return false;
            } else if (pin_lineEdit->testAttribute(Qt::WA_TransparentForMouseEvents))
            {
                show_warning("pin lineEdit disabled!");
                return false;
            }

            pin_lineEdit->clear();
            QTest::keyClicks(pin_lineEdit, value.toString());
            QTest::keyClick(pin_lineEdit, Qt::Key_Enter);
            qApp->processEvents();

            break;
        }
        default:
        {
            show_warning("Unknown button position",
                         QString::number(button));
            return false;
        }
    }

    // Action completed successfully
    return true;
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
        show_warning("Pin list length", QString::number(expected_pin_list.length()));
        show_warning("Combo list length", QString::number(expected_combo_list.length()));
        show_warning("Slider list length", QString::number(expected_slider_list.length()));
        show_warning("LineEdit list length", QString::number(expected_lineEdit_list.length()));
        show_warning("Disabled list length", QString::number(expected_disabled_list.length()));
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
            show_warning("Bad pin", expected_pin_list.at(i));
            return false;
        }
    }

    // If reached here, all pins passed (return true)
    return true;
}

bool GUI_IO_CONTROL_TEST_CLASS::check_pin_test(QString pin_str, QStringList expected_combos,
                                               QList<int> expected_slider, QString expected_lineEdit,
                                               bool isDisabled)
{
    // Get the pin
    QHBoxLayout *pin = get_pin_test(pin_str);
    if (!pin) return false;

    // Check pin elements
    if (!check_pin_label_test(pin, pin_str.split("_").at(1))
            || !check_pin_combo_test(pin, expected_combos)
            || !check_pin_slider_test(pin, expected_slider, isDisabled)
            || !check_pin_lineEdit_test(pin, expected_lineEdit, isDisabled))
    {
        show_warning("Bad pin", pin_str);
        return false;
    }

    // All elements passed (return true)
    return true;
}

bool GUI_IO_CONTROL_TEST_CLASS::check_pin_test(QString pin_str, QString combo_value,
                                               int slider_value, QString lineEdit_value,
                                               bool isDisabled)
{
    // Get the pin
    QHBoxLayout *pin = get_pin_test(pin_str);
    if (!pin) return false;

    // Check pin elements
    if (!check_pin_label_test(pin, pin_str.split("_").at(1))
            || !check_pin_combo_test(pin, combo_value)
            || !check_pin_slider_test(pin, slider_value, isDisabled)
            || !check_pin_lineEdit_test(pin, lineEdit_value, isDisabled))
    {
        show_warning("Bad pin", pin_str);
        return false;
    }

    // All elements passed (return true)
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
        show_warning("Expected pins length mismatch",
                     QString::number(pin_list.length()),
                     QString::number(expected_pin_list.length()));
        return false;
    }

    // Verify items
    foreach (QString pin, expected_pin_list)
    {
        if (!pin_list.contains(pin))
        {
            show_warning("Pin not found", pin);
            return false;
        }
    }

    // If reached, expected matches actual (return true)
    return true;
}

bool GUI_IO_CONTROL_TEST_CLASS::check_pin_label_test(QHBoxLayout *pin, QString expected_label_value)
{
    // Get combo
    QLabel *pin_label = (QLabel*) get_pin_element(pin, io_label_pos, "check_pin_label_test");
    if (!pin_label) return false;

    // Check label element
    if (pin_label->text() != expected_label_value)
    {
        show_warning("Bad pin label",
                     pin_label->text(),
                     expected_label_value);
        return false;
    }

    // Label passed
    return true;
}

bool GUI_IO_CONTROL_TEST_CLASS::check_pin_combo_test(QHBoxLayout *pin, QString expected_combo_value)
{
    // Get combo
    QComboBox *pin_combo = (QComboBox*) get_pin_element(pin, io_combo_pos, "combo value test");
    if (!pin_combo) return false;

    // Check combo element
    if (pin_combo->currentText() != expected_combo_value)
    {
        show_warning("Bad current combo",
                     pin_combo->currentText(),
                     expected_combo_value);
        return false;
    }

    // Combo passed
    return true;
}

bool GUI_IO_CONTROL_TEST_CLASS::check_pin_combo_test(QHBoxLayout *pin, QStringList expected_combos)
{
    // Get combo
    QComboBox *pin_combo = (QComboBox*) get_pin_element(pin, io_combo_pos, "combo list test");
    if (!pin_combo) return false;

    // Check combo length
    if (pin_combo->count() != expected_combos.length())
    {
        show_warning("Bad combo length",
                     QString::number(pin_combo->count()),
                     QString::number(expected_combos.length()));
        return false;
    }

    // Check combo elements
    foreach (QString combo_entry, expected_combos)
    {
        if (pin_combo->findText(combo_entry) == -1)
        {
            show_warning("Combo entry not found", combo_entry);
            return false;
        }
    }

    // Combo passed
    return true;
}

bool GUI_IO_CONTROL_TEST_CLASS::check_pin_slider_test(QHBoxLayout *pin, int expected_slider_value, bool isDisabled)
{
    // Get slider
    QSlider *pin_slider = (QSlider*) get_pin_element(pin, io_slider_pos, "slider value test");
    if (!pin_slider) return false;

    // Check slider clickable
    if (pin_slider->testAttribute(Qt::WA_TransparentForMouseEvents) != isDisabled)
    {
        show_warning("Bad clicking slider",
                     QString::number(pin_slider->testAttribute(Qt::WA_TransparentForMouseEvents)),
                     QString::number(isDisabled));
        return false;
    }

    // Check slider value
    if (pin_slider->value() != expected_slider_value)
    {
        show_warning("Bad Slider Value",
                     QString::number(pin_slider->value()),
                     QString::number(expected_slider_value));
        return false;
    }

    // Slider passed
    return true;
}

bool GUI_IO_CONTROL_TEST_CLASS::check_pin_slider_test(QHBoxLayout *pin, QList<int> expected_slider, bool isDisabled)
{
    // Get slider
    QSlider *pin_slider = (QSlider*) get_pin_element(pin, io_slider_pos, "slider complete test");
    if (!pin_slider) return false;

    // Check expected input length
    int expected_len = 4;
    if (expected_slider.length() != expected_len)
    {
        show_warning("Slider incorrect num elements",
                     QString::number(expected_slider.length()),
                     QString::number(expected_len));
        return false;
    }

    // Check basic slider info
    if (!check_pin_slider_test(pin, expected_slider.at(0), isDisabled)) return false;

    // Check other slider values
    if ((pin_slider->minimum() != expected_slider.at(1))
            || (pin_slider->maximum() != expected_slider.at(2))
            || (pin_slider->tickInterval() != expected_slider.at(3))
            || (pin_slider->singleStep() != expected_slider.at(3))
            || (pin_slider->pageStep() != expected_slider.at(3)))
    {
        show_warning("Slider mismatch!");
        show_warning("Minimum",
                     QString::number(pin_slider->minimum()),
                     QString::number(expected_slider.at(1)));
        show_warning("Maximum",
                     QString::number(pin_slider->maximum()),
                     QString::number(expected_slider.at(2)));
        show_warning("Tick Interval",
                     QString::number(pin_slider->tickInterval()),
                     QString::number(expected_slider.at(3)));
        show_warning("Single Step",
                     QString::number(pin_slider->singleStep()),
                     QString::number(expected_slider.at(3)));
        show_warning("Page Step",
                     QString::number(pin_slider->pageStep()),
                     QString::number(expected_slider.at(3)));
        return false;
    }

    // Slider passed
    return true;
}

bool GUI_IO_CONTROL_TEST_CLASS::check_pin_lineEdit_test(QHBoxLayout *pin, QString lineEdit_value, bool isDisabled)
{
    // Get slider
    QLineEdit *pin_lineEdit = (QLineEdit*) get_pin_element(pin, io_line_edit_pos, "lineEdit value test");
    if (!pin_lineEdit) return false;

    // Check line edit clickable
    if (pin_lineEdit->testAttribute(Qt::WA_TransparentForMouseEvents) != isDisabled)
    {
        show_warning("Bad clicking line edit",
                     QString::number(pin_lineEdit->testAttribute(Qt::WA_TransparentForMouseEvents)),
                     QString::number(isDisabled));
        return false;
    }

    // Check line edit value
    if (pin_lineEdit->text() != lineEdit_value)
    {
        show_warning("Bad line edit",
                     pin_lineEdit->text(),
                     lineEdit_value);
        return false;
    }

    // LineEdit passed
    return true;
}

QWidget *GUI_IO_CONTROL_TEST_CLASS::get_pin_element(QHBoxLayout *pin, uint8_t pos, QString test)
{
    // Verify pin
    if (!pin)
    {
        show_warning("NULL pin passed into get_pin_element. Test Name", test);
        return nullptr;
    } else if (pin->count() < pos)
    {
        show_warning("Widget pos out of bounds. Test Name", test);
        return nullptr;
    }

    // Get widget
    QWidget *pin_elem = pin->itemAt(pos)->widget();
    if (!pin_elem)
    {
        show_warning("Failed to retreive pin element from pin. Test Name", test);
        return nullptr;
    }

    // Otherwise return widget
    return pin_elem;
}

QHBoxLayout *GUI_IO_CONTROL_TEST_CLASS::get_pin_test(QString pin_str)
{
    // Parse pin str
    QStringList pin_vals = pin_str.split("_");
    if (pin_vals.length() != 2)
    {
        show_warning("Bad pin string format", pin_str);
        return nullptr;
    }

    // Set values from pin str
    uint8_t pinNum = pin_vals.at(1).toInt();
    uint8_t pinType;
    if (pin_vals.at(0) == "AIO") pinType = MINOR_KEY_IO_AIO;
    else if (pin_vals.at(0) == "DIO") pinType = MINOR_KEY_IO_DIO;
    else
    {
        show_warning("Unknown pinType", pin_vals.at(0));
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
            show_warning("Unknown pinType",
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
    show_warning("Pin not found", QString::number(pinNum));
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
    if (!value.isEmpty()) hint += ": \"" + value + "\"";
    qWarning().noquote().nospace() << "\n\t" << hint;
}

void GUI_IO_CONTROL_TEST_CLASS::show_warning(QString hint, QString got, QString expected)
{
    qWarning().noquote().nospace() \
            << "\n\t" << hint \
            << "\n\tGot: \"" << got << "\"" \
            << "\n\tExpected: \"" << expected << "\"";
}
