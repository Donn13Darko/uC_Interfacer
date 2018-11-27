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

#ifndef GUI_WELCOME_TESTS_H
#define GUI_WELCOME_TESTS_H

#include <QObject>

// Testing class
#include "gui-welcome-test-class.hpp"

class GUI_WELCOME_TESTS : public QObject
{
    Q_OBJECT

public:
    GUI_WELCOME_TESTS();
    ~GUI_WELCOME_TESTS();

private slots:
    // Setup and cleanup functions
    void initTestCase();
    void cleanupTestCase();

    // Member tests
    void test_init_vals();

    void test_basic_features();
    void test_basic_features_data();

    void test_gui_config();
    void test_gui_config_data();

private:
    GUI_WELCOME_TEST_CLASS *welcome_tester;
};

#endif // GUI_WELCOME_TESTS_H
