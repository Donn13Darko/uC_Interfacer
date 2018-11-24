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

#ifndef GUI_PROGRAMMER_TESTS_H
#define GUI_PROGRAMMER_TESTS_H

#include <QObject>

// Testing class
#include "gui-programmer-test-class.hpp"

class GUI_PROGRAMMER_TESTS : public QObject
{
    Q_OBJECT

public:
    GUI_PROGRAMMER_TESTS();
    ~GUI_PROGRAMMER_TESTS();

private slots:
    // Setup and cleanup functions
    void initTestCase();
    void cleanupTestCase();

    // Member tests

private:
    GUI_PROGRAMMER_TEST_CLASS *programmer_tester;
};

#endif // GUI_PROGRAMMER_TESTS_H
