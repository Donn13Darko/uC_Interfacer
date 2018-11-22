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

#include "gui-base-tests.hpp"

// Testing infrastructure includes
#include <QtTest>
#include <QCoreApplication>

GUI_BASE_TESTS::GUI_BASE_TESTS()
{
    /* Do Nothing */
}

GUI_BASE_TESTS::~GUI_BASE_TESTS()
{
    // Delete base test if allocated
    if (base) delete base;
}

void GUI_BASE_TESTS::initTestCase()
{
    // Create object for testing
    base = new GUI_BASE();
    QVERIFY(base);
}

void GUI_BASE_TESTS::cleanupTestCase()
{
    // Delete test class
    if (base)
    {
        delete base;
        base = nullptr;
    }
}

void GUI_BASE_TESTS::test_closable()
{
    // Test closable settings
    QVERIFY(base->isClosable());
    base->setClosable(false);
    QVERIFY(!base->isClosable());
    base->setClosable(true);
    QVERIFY(base->isClosable());
}

void GUI_BASE_TESTS::test_gui_key()
{
    // Verify base gui key is error
    QCOMPARE(base->get_GUI_key(), (uint8_t) MAJOR_KEY_ERROR);
}
