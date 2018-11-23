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

#ifndef GUI_BASE_TESTS_H
#define GUI_BASE_TESTS_H

#include <QObject>

// Testing class
#include "gui-base-test-class.hpp"

class GUI_BASE_TESTS : public QObject
{
    Q_OBJECT

public:
    GUI_BASE_TESTS();
    ~GUI_BASE_TESTS();

private slots:
    // Setup and cleanup functions
    void initTestCase();
    void cleanupTestCase();

    // Data generators


    // Basic public member tests
    void test_init_vals();
    void test_basic_features();

    void test_gui_key();
    void test_gui_key_data();

    void test_set_gui_name();
    void test_set_gui_name_data();

    void test_set_gui_tab_name();
    void test_set_gui_tab_name_data();

    void test_gui_config_1();
    void test_gui_config_2();

    void test_recv_length();
    void test_recv_length_data();

    void test_reset_gui_1();

    void test_rcvd_formatted();
    void test_rcvd_formatted_data();

    // Basic protected member tests

private:
    GUI_BASE_TEST_CLASS *base_tester;
};

#endif // GUI_BASE_TESTS_H
