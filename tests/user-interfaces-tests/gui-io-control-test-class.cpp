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

void GUI_IO_CONTROL_TEST_CLASS::set_aio_update_rate_test(float rate)
{
    QTest::keyClicks(ui_ptr->AIO_UR_LineEdit, QString::number(rate));
    qApp->processEvents();
}

void GUI_IO_CONTROL_TEST_CLASS::set_dio_update_rate_test(float rate)
{
    QTest::keyClicks(ui_ptr->DIO_UR_LineEdit, QString::number(rate));
    qApp->processEvents();
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

void GUI_IO_CONTROL_TEST_CLASS::set_log_file_update_rate_test(float rate)
{
    QTest::keyClicks(ui_ptr->LOG_UR_LineEdit, QString::number(rate));
    qApp->processEvents();
}

void GUI_IO_CONTROL_TEST_CLASS::set_log_append_checked_test(bool b)
{
    set_checked_click_test(ui_ptr->AppendLog_CheckBox, b);
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

void GUI_IO_CONTROL_TEST_CLASS::reset_clicked_test()
{
    // Setup spy to catch tranmit signal
    QList<QVariant> spy_args;
    QSignalSpy transmit_chunk_spy(this, transmit_chunk);
    QVERIFY(transmit_chunk_spy.isValid());

    // Click the reset button
    QTest::mouseClick(ui_ptr->ResetGUI_Button, Qt::LeftButton);
    qApp->processEvents();

    // Verify that reset signal emitted
    QCOMPARE(transmit_chunk_spy.count(), 1);
    spy_args = transmit_chunk_spy.takeFirst();
    QCOMPARE(spy_args.at(0).toInt(), (int) MAJOR_KEY_RESET);
    QCOMPARE(spy_args.at(1).toInt(), (int) 0);
}

void GUI_IO_CONTROL_TEST_CLASS::set_checked_click_test(QCheckBox *check, bool b)
{
    if (b && !check->isChecked())
        QTest::mouseClick(check, Qt::LeftButton);
    else if (!b && check->isChecked())
        QTest::mouseClick(check, Qt::LeftButton);

    qApp->processEvents();
}
