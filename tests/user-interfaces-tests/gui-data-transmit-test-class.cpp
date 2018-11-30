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

#include "gui-data-transmit-test-class.hpp"
#include "ui_gui-data-transmit.h"

#include <QTest>
#include <QSignalSpy>

GUI_DATA_TRANSMIT_TEST_CLASS::GUI_DATA_TRANSMIT_TEST_CLASS(QWidget *parent) :
    GUI_DATA_TRANSMIT(parent)
{
    // Retrieve ui object
    ui_ptr = get_ui();

    // Show UI (needs to be visible for UI events)
    show();
}

GUI_DATA_TRANSMIT_TEST_CLASS::~GUI_DATA_TRANSMIT_TEST_CLASS()
{
    /* DO NOTHING */
}

QByteArray GUI_DATA_TRANSMIT_TEST_CLASS::rcvd_formatted_readAll_test()
{
    return rcvd_formatted_readAll();
}

qint64 GUI_DATA_TRANSMIT_TEST_CLASS::rcvd_formatted_size_test()
{
    return rcvd_formatted_size();
}

int GUI_DATA_TRANSMIT_TEST_CLASS::get_progress_update_send_value_test()
{
    return ui_ptr->Send_ProgressBar->value();
}

QString GUI_DATA_TRANSMIT_TEST_CLASS::get_progress_update_send_string_test()
{
    return ui_ptr->SendProgress_Label->text();
}

int GUI_DATA_TRANSMIT_TEST_CLASS::get_progress_update_recv_value_test()
{
    return ui_ptr->Recv_ProgressBar->value();
}

QString GUI_DATA_TRANSMIT_TEST_CLASS::get_progress_update_recv_string_test()
{
    return ui_ptr->RecvProgress_Label->text();
}

void GUI_DATA_TRANSMIT_TEST_CLASS::set_user_input_text_test(QString input)
{
    // Verify set correctly
    QVERIFY(ui_ptr->SendInput_Radio->isChecked());

    // Verify input
    ui_ptr->Send_PlainText->clear();
    if (input.isEmpty()) return;

    // Set Input
    foreach (QString line, input.split('\n'))
    {
        QTest::keyClicks(ui_ptr->Send_PlainText, line);
        QTest::keyClick(ui_ptr->Send_PlainText, Qt::Key_Enter);
    }

    // Process events
    qApp->processEvents();
}

QString GUI_DATA_TRANSMIT_TEST_CLASS::get_user_input_text_test()
{
    return ui_ptr->Send_PlainText->toPlainText();
}

void GUI_DATA_TRANSMIT_TEST_CLASS::set_file_input_text_test(QString filePath)
{
    QVERIFY(ui_ptr->SendFile_Radio->isChecked());
    QTest::keyClicks(ui_ptr->SendFilePath_LineEdit, filePath);
    qApp->processEvents();
}

QString GUI_DATA_TRANSMIT_TEST_CLASS::get_file_input_text_test()
{
    return ui_ptr->SendFilePath_LineEdit->text();
}

void GUI_DATA_TRANSMIT_TEST_CLASS::set_data_input_radio_test(bool select_file)
{
    if (select_file) QTest::mouseClick(ui_ptr->SendFile_Radio, Qt::LeftButton);
    else QTest::mouseClick(ui_ptr->SendInput_Radio, Qt::LeftButton);

    qApp->processEvents();
}

bool GUI_DATA_TRANSMIT_TEST_CLASS::get_data_input_radio_test()
{
    return ui_ptr->SendFile_Radio->isChecked();
}

void GUI_DATA_TRANSMIT_TEST_CLASS::send_clicked_test()
{
    QTest::mouseClick(ui_ptr->Send_Button, Qt::LeftButton);
    qApp->processEvents();
}

void GUI_DATA_TRANSMIT_TEST_CLASS::set_show_recv_data_test(bool b)
{
    set_checked_click_test(ui_ptr->RecvShowRecv_CheckBox, b);
}

bool GUI_DATA_TRANSMIT_TEST_CLASS::get_show_recv_data_test()
{
    return ui_ptr->RecvShowRecv_CheckBox->isChecked();
}

void GUI_DATA_TRANSMIT_TEST_CLASS::set_recv_clear_on_set_test(bool b)
{
    set_checked_click_test(ui_ptr->RecvClearOnSet_CheckBox, b);
}

bool GUI_DATA_TRANSMIT_TEST_CLASS::get_recv_clear_on_set_test()
{
    return ui_ptr->RecvClearOnSet_CheckBox->isChecked();
}

QString GUI_DATA_TRANSMIT_TEST_CLASS::get_displayed_recv_test()
{
    return ui_ptr->Recv_PlainText->toPlainText();
}

void GUI_DATA_TRANSMIT_TEST_CLASS::recv_clear_clicked_test()
{
    QTest::mouseClick(ui_ptr->RecvClear_Button, Qt::LeftButton);
    qApp->processEvents();
}

void GUI_DATA_TRANSMIT_TEST_CLASS::reset_clicked_test()
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

void GUI_DATA_TRANSMIT_TEST_CLASS::set_checked_click_test(QCheckBox *check, bool b)
{
    if (b && !check->isChecked())
        QTest::mouseClick(check, Qt::LeftButton);
    else if (!b && check->isChecked())
        QTest::mouseClick(check, Qt::LeftButton);

    qApp->processEvents();
}
