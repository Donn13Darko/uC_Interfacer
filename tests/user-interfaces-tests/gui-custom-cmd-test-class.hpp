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
    void rcvd_formatted_clear_test();

    void set_expected_recv_length_test(uint32_t expected_length);
    uint32_t get_expected_recv_length_test();

    void update_current_recv_length_test(uint32_t recv_len);
    uint32_t get_current_recv_length_test();

    void on_ResetGUI_Button_clicked_test();

    uint8_t get_send_key_base_test();
    uint8_t get_send_cmd_base_test();
    uint8_t get_recv_key_base_test();
    uint8_t get_recv_cmd_base_test();

    void cmd_input_radio_select_test(bool select_file);
    void cmd_keys_in_input_checked_test(bool b);
    void cmd_user_input_enter_text_test(QString input);
    void cmd_file_input_enter_text_test(QString filePath);
    void cmd_send_click_test();

    void cmd_set_major_key_test(QString key);
    void cmd_set_minor_key_test(QString key);
    void cmd_set_key_base_test(QString base);
    void cmd_set_cmd_base_test(QString base);

    int get_cmd_progress_value_test();
    QString get_cmd_progress_string_test();

    int get_feedback_progress_value_test();
    QString get_feedback_progress_string_test();

    void log_all_cmds_checked_test(bool b);
    void display_rcvd_click_test(bool b);

    QByteArray get_displayed_feedback_test();

private:
    Ui::GUI_CUSTOM_CMD *ui_ptr;

    void set_checked_click_test(QCheckBox *check, bool b);
};

#endif // GUI_CUSTOM_CMD_TEST_CLASS_H
