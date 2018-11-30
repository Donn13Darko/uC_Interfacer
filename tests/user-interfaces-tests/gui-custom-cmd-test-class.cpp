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

#include "gui-custom-cmd-test-class.hpp"
#include "ui_gui-custom-cmd.h"

#include <QTest>
#include <QSignalSpy>

GUI_CUSTOM_CMD_TEST_CLASS::GUI_CUSTOM_CMD_TEST_CLASS(QWidget *parent) :
    GUI_CUSTOM_CMD(parent)
{
    // Retrieve ui object
    ui_ptr = get_ui();

    // Show UI (needs to be visible for UI events)
    show();
}

GUI_CUSTOM_CMD_TEST_CLASS::~GUI_CUSTOM_CMD_TEST_CLASS()
{
    /* DO NOTHING */
}

QByteArray GUI_CUSTOM_CMD_TEST_CLASS::rcvd_formatted_readAll_test()
{
    return rcvd_formatted_readAll();
}

qint64 GUI_CUSTOM_CMD_TEST_CLASS::rcvd_formatted_size_test()
{
    return rcvd_formatted_size();
}

int GUI_CUSTOM_CMD_TEST_CLASS::get_progress_update_send_value_test()
{
    return ui_ptr->CustomCMD_ProgressBar->value();
}

QString GUI_CUSTOM_CMD_TEST_CLASS::get_progress_update_send_string_test()
{
    return ui_ptr->CustomCMDProgress_Label->text();
}

int GUI_CUSTOM_CMD_TEST_CLASS::get_progress_update_recv_value_test()
{
    return ui_ptr->Feedback_ProgressBar->value();
}

QString GUI_CUSTOM_CMD_TEST_CLASS::get_progress_update_recv_string_test()
{
    return ui_ptr->FeedbackProgress_Label->text();
}

void GUI_CUSTOM_CMD_TEST_CLASS::set_cmd_input_radio_test(bool select_file)
{
    if (select_file) QTest::mouseClick(ui_ptr->CustomCMDFile_Radio, Qt::LeftButton);
    else QTest::mouseClick(ui_ptr->CustomCMDManual_Radio, Qt::LeftButton);

    qApp->processEvents();
}

bool GUI_CUSTOM_CMD_TEST_CLASS::get_cmd_input_radio_test()
{
    return ui_ptr->CustomCMDFile_Radio->isChecked();
}

void GUI_CUSTOM_CMD_TEST_CLASS::set_keys_in_input_test(bool b)
{
    set_checked_click_test(ui_ptr->CustomCMDKeysInInput_CheckBox, b);
}

bool GUI_CUSTOM_CMD_TEST_CLASS::get_keys_in_input_test()
{
    return ui_ptr->CustomCMDKeysInInput_CheckBox->isChecked();
}

void GUI_CUSTOM_CMD_TEST_CLASS::send_clicked_test()
{
    QTest::mouseClick(ui_ptr->CustomCMDSend_Button, Qt::LeftButton);
    qApp->processEvents();
}

void GUI_CUSTOM_CMD_TEST_CLASS::set_user_input_text_test(QString input)
{
    // Verify set correctly
    QVERIFY(ui_ptr->CustomCMDManual_Radio->isChecked());

    // Verify input
    ui_ptr->CustomCMD_PlainText->clear();
    if (input.isEmpty()) return;

    // Set input
    foreach (QString line, input.split('\n'))
    {
        QTest::keyClicks(ui_ptr->CustomCMD_PlainText, line);
        QTest::keyClick(ui_ptr->CustomCMD_PlainText, Qt::Key_Enter);
    }

    // Process events
    qApp->processEvents();
}

QString GUI_CUSTOM_CMD_TEST_CLASS::get_user_input_text_test()
{
    return ui_ptr->CustomCMD_PlainText->toPlainText();
}

void GUI_CUSTOM_CMD_TEST_CLASS::set_file_input_text_test(QString filePath)
{
    QVERIFY(ui_ptr->CustomCMDFile_Radio->isChecked());
    QTest::keyClicks(ui_ptr->CustomCMDFilePath_LineEdit, filePath);
    qApp->processEvents();
}

QString GUI_CUSTOM_CMD_TEST_CLASS::get_file_input_text_test()
{
    return ui_ptr->CustomCMDFilePath_LineEdit->text();
}

void GUI_CUSTOM_CMD_TEST_CLASS::set_major_key_test(QString key)
{
    ui_ptr->CustomCMDMajorKey_LineEdit->setText(key);
}

QString GUI_CUSTOM_CMD_TEST_CLASS::get_major_key_test()
{
    return ui_ptr->CustomCMDMajorKey_LineEdit->text();
}

void GUI_CUSTOM_CMD_TEST_CLASS::set_minor_key_test(QString key)
{
    ui_ptr->CustomCMDMinorKey_LineEdit->setText(key);
}

QString GUI_CUSTOM_CMD_TEST_CLASS::get_minor_key_test()
{
    return ui_ptr->CustomCMDMinorKey_LineEdit->text();
}

void GUI_CUSTOM_CMD_TEST_CLASS::set_key_base_test(QString base)
{
    ui_ptr->CustomCMDKeyBase_LineEdit->setText(base);
}

QString GUI_CUSTOM_CMD_TEST_CLASS::get_key_base_test()
{
    return ui_ptr->CustomCMDKeyBase_LineEdit->text();
}

void GUI_CUSTOM_CMD_TEST_CLASS::set_cmd_base_test(QString base)
{
    ui_ptr->CustomCMDBase_LineEdit->setText(base);
}

QString GUI_CUSTOM_CMD_TEST_CLASS::get_cmd_base_test()
{
    return ui_ptr->CustomCMDBase_LineEdit->text();
}

void GUI_CUSTOM_CMD_TEST_CLASS::set_feedback_log_all_cmds_test(bool b)
{
    set_checked_click_test(ui_ptr->FeedbackLogAllCMDs_CheckBox, b);
}

bool GUI_CUSTOM_CMD_TEST_CLASS::get_feedback_log_all_cmds_test()
{
    return ui_ptr->FeedbackLogAllCMDs_CheckBox->isChecked();
}

void GUI_CUSTOM_CMD_TEST_CLASS::set_feedback_append_newline_test(bool b)
{
    set_checked_click_test(ui_ptr->FeedbackAppendNewline_CheckBox, b);
}

bool GUI_CUSTOM_CMD_TEST_CLASS::get_feedback_append_newline_test()
{
    return ui_ptr->FeedbackAppendNewline_CheckBox->isChecked();
}

void GUI_CUSTOM_CMD_TEST_CLASS::set_feedback_clear_on_set_test(bool b)
{
    set_checked_click_test(ui_ptr->FeedbackClearOnSet_CheckBox, b);
}

bool GUI_CUSTOM_CMD_TEST_CLASS::get_feedback_clear_on_set_test()
{
    return ui_ptr->FeedbackClearOnSet_CheckBox->isChecked();
}

QString GUI_CUSTOM_CMD_TEST_CLASS::get_displayed_feedback_test()
{
    return ui_ptr->Feedback_PlainText->toPlainText();
}

void GUI_CUSTOM_CMD_TEST_CLASS::feedback_clear_clicked_test()
{
    QTest::mouseClick(ui_ptr->FeedbackClear_Button, Qt::LeftButton);
    qApp->processEvents();
}

void GUI_CUSTOM_CMD_TEST_CLASS::set_instructions_text_test(QString inst)
{
    ui_ptr->Instructions_PlainText->setPlainText(inst);
}

QString GUI_CUSTOM_CMD_TEST_CLASS::get_instructions_text_test()
{
    return ui_ptr->Instructions_PlainText->toPlainText();
}

void GUI_CUSTOM_CMD_TEST_CLASS::reset_clicked_test()
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

void GUI_CUSTOM_CMD_TEST_CLASS::set_checked_click_test(QCheckBox *check, bool b)
{
    if (b && !check->isChecked())
        QTest::mouseClick(check, Qt::LeftButton);
    else if (!b && check->isChecked())
        QTest::mouseClick(check, Qt::LeftButton);

    qApp->processEvents();
}
