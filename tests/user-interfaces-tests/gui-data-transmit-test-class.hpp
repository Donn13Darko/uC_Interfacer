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

#ifndef GUI_DATA_TRANSMIT_TEST_CLASS_H
#define GUI_DATA_TRANSMIT_TEST_CLASS_H

// Testing class
#include "../../src/user-interfaces/gui-data-transmit.hpp"

#include <QCheckBox>

class GUI_DATA_TRANSMIT_TEST_CLASS : public GUI_DATA_TRANSMIT
{
    Q_OBJECT

public:
    GUI_DATA_TRANSMIT_TEST_CLASS(QWidget *parent = 0);
    ~GUI_DATA_TRANSMIT_TEST_CLASS();

    /*** Defines any needed setters & accessors for testing ***/
    QByteArray rcvd_formatted_readAll_test();
    qint64 rcvd_formatted_size_test();

    void set_expected_recv_length_test(uint32_t expected_length);
    void update_current_recv_length_test(uint32_t recv_len);

    void set_progress_update_send_test(int progress, QString label);
    int get_progress_update_send_value_test();
    QString get_progress_update_send_string_test();

    void set_progress_update_recv_test(int progress, QString label);
    int get_progress_update_recv_value_test();
    QString get_progress_update_recv_string_test();

    void set_data_input_radio_test(bool select_file);
    bool get_data_input_radio_test();

    void send_clicked_test();

    void set_user_input_text_test(QString input);
    QString get_user_input_text_test();

    void set_file_input_text_test(QString filePath);
    QString get_file_input_text_test();

    void set_show_recv_data_test(bool b);
    bool get_show_recv_data_test();

    void set_recv_clear_on_set_test(bool b);
    bool get_recv_clear_on_set_test();

    QString get_displayed_recv_test();
    void recv_clear_clicked_test();
    void recv_save_test(QString filePath);

    void receive_gui_test(QByteArray data);
    void reset_clicked_test();

private:
    Ui::GUI_DATA_TRANSMIT *ui_ptr;

    void set_checked_click_test(QCheckBox *check, bool b);
};

#endif // GUI_DATA_TRANSMIT_TEST_CLASS_H
