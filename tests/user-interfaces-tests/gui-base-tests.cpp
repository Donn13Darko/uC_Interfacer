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

// Object includes
#include <QString>
#include <QTimer>
#include <QEventLoop>

GUI_BASE_TESTS::GUI_BASE_TESTS()
{
    /* DO NOTHING */
}

GUI_BASE_TESTS::~GUI_BASE_TESTS()
{
    // Delete base test if allocated
    if (base_tester) delete base_tester;
}

void GUI_BASE_TESTS::initTestCase()
{
    // Create object for testing
    base_tester = new GUI_BASE_TEST_CLASS();
    QVERIFY(base_tester);
}

void GUI_BASE_TESTS::cleanupTestCase()
{
    // Delete test class
    if (base_tester)
    {
        delete base_tester;
        base_tester = nullptr;
    }
}

void GUI_BASE_TESTS::test_closable()
{
    // Test toggle closable settings
    QVERIFY(base_tester->isClosable());
    base_tester->setClosable(false);
    QVERIFY(!base_tester->isClosable());
    base_tester->setClosable(true);
    QVERIFY(base_tester->isClosable());
    base_tester->setClosable(false);
    QVERIFY(!base_tester->isClosable());
    base_tester->setClosable(true);
    QVERIFY(base_tester->isClosable());
}

void GUI_BASE_TESTS::test_gui_key()
{
    // Verify base gui key setting
    QCOMPARE(base_tester->get_GUI_key(), (uint8_t) MAJOR_KEY_ERROR);
    base_tester->set_gui_key(MAJOR_KEY_ERROR + 1);
    QCOMPARE(base_tester->get_GUI_key(), (uint8_t) (MAJOR_KEY_ERROR + 1));
    base_tester->set_gui_key(MAJOR_KEY_ERROR);
}

void GUI_BASE_TESTS::test_set_GUI_tab_name()
{
    // Verify inits to blank then try setting
    QCOMPARE(base_tester->get_GUI_tab_name(), QString(""));
    base_tester->set_GUI_tab_name("New Tab");
    QCOMPARE(base_tester->get_GUI_tab_name(), QString("New Tab"));
    base_tester->set_GUI_tab_name("");
    QCOMPARE(base_tester->get_GUI_tab_name(), QString(""));
}

void GUI_BASE_TESTS::test_GUI_config()
{
    /* STUB */
}

void GUI_BASE_TESTS::test_acceptAllCMDs()
{
    /* STUB */
}

void GUI_BASE_TESTS::test_waitForDevice()
{
    /* STUB */
}

void GUI_BASE_TESTS::test_reset_gui()
{
    /* STUB */
}
