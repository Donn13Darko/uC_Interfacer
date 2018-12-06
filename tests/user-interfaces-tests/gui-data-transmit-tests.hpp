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

#ifndef GUI_DATA_TRANSMIT_TESTS_H
#define GUI_DATA_TRANSMIT_TESTS_H

#include <QObject>

// Testing class
#include "gui-data-transmit-test-class.hpp"

class GUI_DATA_TRANSMIT_TESTS : public QObject
{
    Q_OBJECT

public:
    GUI_DATA_TRANSMIT_TESTS();
    ~GUI_DATA_TRANSMIT_TESTS();

private slots:
    // Setup and cleanup functions
    void initTestCase();
    void cleanupTestCase();

    // Member tests
    void test_init_vals();
    void test_basic_features();

    void test_gui_config();
    void test_gui_config_data();

    void test_send();
    void test_send_data();

    void test_rcvd();
    void test_rcvd_data();

    void test_complex_transmit();
    void test_complex_transmit_data();

private:
    GUI_DATA_TRANSMIT_TEST_CLASS *data_transmit_tester;

    void verify_reset_values();

    void perform_data_rcvd(QList<QByteArray> rcvd_fill_data,
                           bool display_recv = true, bool clear_on_set = true,
                           bool click_clear_button = true,
                           bool click_save_button = false, bool check_rcvd = false,
                           QString rcvd_expected_display_data = "",
                           QByteArray rcvd_expected_file_data = QByteArray());

    void perform_data_send(QString send_fill_data, bool send_file_radio,
                           bool click_send = false, bool check_send = false,
                           QList<QList<QVariant>> send_expected_signals = QList<QList<QVariant>>());
};

#endif // GUI_DATA_TRANSMIT_TESTS_H
