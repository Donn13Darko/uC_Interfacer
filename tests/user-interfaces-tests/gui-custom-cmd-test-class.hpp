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

#ifndef GUI_CUSTOM_CMD_TEST_CLASS_H
#define GUI_CUSTOM_CMD_TEST_CLASS_H

// Testing class
#include "../../src/user-interfaces/gui-custom-cmd.hpp"

#include <QCheckBox>

class GUI_CUSTOM_CMD_TEST_CLASS : public GUI_CUSTOM_CMD
{
    Q_OBJECT

public:
    GUI_CUSTOM_CMD_TEST_CLASS(QWidget *parent = 0);
    ~GUI_CUSTOM_CMD_TEST_CLASS();

    /*** Defines any needed setters & accessors for testing ***/
    QByteArray rcvd_formatted_readAll_test();
    qint64 rcvd_formatted_size_test();

    int get_progress_update_send_value_test();
    QString get_progress_update_send_string_test();

    int get_progress_update_recv_value_test();
    QString get_progress_update_recv_string_test();

    void set_cmd_input_radio_test(bool select_file);
    bool get_cmd_input_radio_test();

    void set_keys_in_input_test(bool b);
    bool get_keys_in_input_test();

    void send_clicked_test();

    void set_user_input_text_test(QString input);
    QString get_user_input_text_test();

    void set_file_input_text_test(QString filePath);
    QString get_file_input_text_test();

    void set_major_key_test(QString key);
    QString get_major_key_test();

    void set_minor_key_test(QString key);
    QString get_minor_key_test();

    void set_key_base_test(QString base);
    QString get_key_base_test();

    void set_cmd_base_test(QString base);
    QString get_cmd_base_test();

    void set_feedback_log_all_cmds_test(bool b);
    bool get_feedback_log_all_cmds_test();

    void set_feedback_append_newline_test(bool b);
    bool get_feedback_append_newline_test();

    void set_feedback_clear_on_set_test(bool b);
    bool get_feedback_clear_on_set_test();

    QString get_displayed_feedback_test();
    void feedback_clear_clicked_test();

    void set_instructions_text_test(QString inst);
    QString get_instructions_text_test();

    void reset_clicked_test();

private:
    Ui::GUI_CUSTOM_CMD *ui_ptr;

    void set_checked_click_test(QCheckBox *check, bool b);
};

#endif // GUI_CUSTOM_CMD_TEST_CLASS_H
