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

GUI_CUSTOM_CMD_TEST_CLASS::GUI_CUSTOM_CMD_TEST_CLASS(QWidget *parent) :
    GUI_CUSTOM_CMD(parent)
{
    // Retrieve ui object from inheritance
    ui_ptr = get_ui();
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

void GUI_CUSTOM_CMD_TEST_CLASS::rcvd_formatted_clear_test()
{
    rcvd_formatted_clear();
}

void GUI_CUSTOM_CMD_TEST_CLASS::set_expected_recv_length_test(uint32_t expected_length)
{
    set_expected_recv_length(expected_length);
}

uint32_t GUI_CUSTOM_CMD_TEST_CLASS::get_expected_recv_length_test()
{
    return expected_recv_length;
}

void GUI_CUSTOM_CMD_TEST_CLASS::update_current_recv_length_test(uint32_t recv_len)
{
    update_current_recv_length(recv_len);
}

uint32_t GUI_CUSTOM_CMD_TEST_CLASS::get_current_recv_length_test()
{
    return current_recv_length;
}

void GUI_CUSTOM_CMD_TEST_CLASS::on_ResetGUI_Button_clicked_test()
{
    on_ResetGUI_Button_clicked();
}

uint8_t GUI_CUSTOM_CMD_TEST_CLASS::get_send_key_base_test()
{
    return get_send_key_base();
}

uint8_t GUI_CUSTOM_CMD_TEST_CLASS::get_send_cmd_base_test()
{
    return get_send_cmd_base();
}

uint8_t GUI_CUSTOM_CMD_TEST_CLASS::get_recv_key_base_test()
{
    return get_recv_key_base();
}

uint8_t GUI_CUSTOM_CMD_TEST_CLASS::get_recv_cmd_base_test()
{
    return get_recv_cmd_base();
}

void GUI_CUSTOM_CMD_TEST_CLASS::cmd_input_radio_select_test(bool select_file)
{
    if (select_file)
        QTest::mouseClick(ui_ptr->CustomCMDFile_Radio, Qt::LeftButton);
    else
        QTest::mouseClick(ui_ptr->CustomCMDManual_Radio, Qt::LeftButton);
}

void GUI_CUSTOM_CMD_TEST_CLASS::cmd_keys_in_input_checked_test(bool b)
{
    set_checked_click_test(ui_ptr->CustomCMDKeysInInput_CheckBox, b);
}

void GUI_CUSTOM_CMD_TEST_CLASS::cmd_user_input_enter_text_test(QString input)
{
    QVERIFY(ui_ptr->CustomCMDManual_Radio->isChecked());
    QTest::keyClicks(ui_ptr->CustomCMD_PlainText, input);
}

void GUI_CUSTOM_CMD_TEST_CLASS::cmd_file_input_enter_text_test(QString filePath)
{
    QVERIFY(ui_ptr->CustomCMDFile_Radio->isChecked());
    QTest::keyClicks(ui_ptr->CustomCMDFilePath_LineEdit, filePath);
}

void GUI_CUSTOM_CMD_TEST_CLASS::cmd_send_click_test()
{
    QTest::mouseClick(ui_ptr->CustomCMDSend_Button, Qt::LeftButton);
}

void GUI_CUSTOM_CMD_TEST_CLASS::cmd_set_major_key_test(QString key)
{
    ui_ptr->CustomCMDMajorKey_LineEdit->setText(key);
}

void GUI_CUSTOM_CMD_TEST_CLASS::cmd_set_minor_key_test(QString key)
{
    ui_ptr->CustomCMDMinorKey_LineEdit->setText(key);
}

void GUI_CUSTOM_CMD_TEST_CLASS::cmd_set_key_base_test(QString base)
{
    ui_ptr->CustomCMDKeyBase_LineEdit->setText(base);
}

void GUI_CUSTOM_CMD_TEST_CLASS::cmd_set_cmd_base_test(QString base)
{
    ui_ptr->CustomCMDBase_LineEdit->setText(base);
}

int GUI_CUSTOM_CMD_TEST_CLASS::get_cmd_progress_value_test()
{
    return ui_ptr->CustomCMD_ProgressBar->value();
}

QString GUI_CUSTOM_CMD_TEST_CLASS::get_cmd_progress_string_test()
{
    return ui_ptr->CustomCMDProgress_Label->text();
}

int GUI_CUSTOM_CMD_TEST_CLASS::get_feedback_progress_value_test()
{
    return ui_ptr->Feedback_ProgressBar->value();
}

QString GUI_CUSTOM_CMD_TEST_CLASS::get_feedback_progress_string_test()
{
    return ui_ptr->FeedbackProgress_Label->text();
}

void GUI_CUSTOM_CMD_TEST_CLASS::log_all_cmds_checked_test(bool b)
{
    set_checked_click_test(ui_ptr->FeedbackLogAllCMDs_CheckBox, b);
}

QByteArray GUI_CUSTOM_CMD_TEST_CLASS::get_displayed_feedback_test()
{
    return ui_ptr->Feedback_PlainText->toPlainText().toLatin1();
}

void GUI_CUSTOM_CMD_TEST_CLASS::set_checked_click_test(QCheckBox *check, bool b)
{
    if (b && !check->isChecked())
        QTest::mouseClick(check, Qt::LeftButton);
    else if (!b && check->isChecked())
        QTest::mouseClick(check, Qt::LeftButton);
}
