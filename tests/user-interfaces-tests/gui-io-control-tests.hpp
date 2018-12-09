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

#ifndef GUI_IO_CONTROL_TESTS_H
#define GUI_IO_CONTROL_TESTS_H

#include <QObject>

// Testing class
#include "gui-io-control-test-class.hpp"

class GUI_IO_CONTROL_TESTS : public QObject
{
    Q_OBJECT

public:
    GUI_IO_CONTROL_TESTS();
    ~GUI_IO_CONTROL_TESTS();

private slots:
    // Setup and cleanup functions
    void initTestCase();
    void cleanupTestCase();

    // Member tests
    void test_init_vals();
    void test_basic_features();

    void test_gui_config();
    void test_gui_config_data();

    void test_gui_interactions();
    void test_gui_interactions_data();

    void test_updates();
    void test_updates_data();

    void test_logging_errors();

    void test_logging();
    void test_logging_data();

    void test_recv();
    void test_recv_data();

    void test_conn_type();
    void test_conn_type_data();

    void test_basic_chart_features();
    void test_complex_chart_features();

    void test_chart_update_features();
    void test_chart_update_features_data();

private:
    GUI_IO_CONTROL_TEST_CLASS *io_control_tester;
    QString temp_filename;

    static const QString generic_config_str;

    bool set_gui_config(QString config_str);
    void clear_gui_config();

    bool reset_tempFile();
    bool remove_tempFile();

    void verify_reset_values();
};

#endif // GUI_IO_CONTROL_TESTS_H
