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

#include "gui-custom-cmd-tests.hpp"

// Testing infrastructure includes
#include <QtTest>
#include <QSignalSpy>

#include "../../src/gui-helpers/gui-generic-helper.hpp"

GUI_CUSTOM_CMD_TESTS::GUI_CUSTOM_CMD_TESTS()
{
    /* DO NOTHING */
}

GUI_CUSTOM_CMD_TESTS::~GUI_CUSTOM_CMD_TESTS()
{
    // Delete base test if allocated
    if (custom_cmd_tester) delete custom_cmd_tester;
}

void GUI_CUSTOM_CMD_TESTS::initTestCase()
{
    // Create object for testing
    custom_cmd_tester = new GUI_CUSTOM_CMD_TEST_CLASS();
    QVERIFY(custom_cmd_tester);
}

void GUI_CUSTOM_CMD_TESTS::cleanupTestCase()
{
    // Delete test class
    if (custom_cmd_tester)
    {
        delete custom_cmd_tester;
        custom_cmd_tester = nullptr;
    }
}

void GUI_CUSTOM_CMD_TESTS::test_init_vals()
{
    // Verify all init values
    QVERIFY(custom_cmd_tester->isClosable());
    QCOMPARE(custom_cmd_tester->get_gui_key(), (uint8_t) MAJOR_KEY_CUSTOM_CMD);
    QCOMPARE(custom_cmd_tester->get_gui_name(), QString("Custom CMD"));
    QCOMPARE(custom_cmd_tester->get_gui_tab_name(), QString("Custom CMD"));
    QCOMPARE(custom_cmd_tester->get_send_key_base_test(), (uint8_t) 16);
    QCOMPARE(custom_cmd_tester->get_send_cmd_base_test(), (uint8_t) 16);
    QCOMPARE(custom_cmd_tester->get_recv_key_base_test(), (uint8_t) 16);
    QCOMPARE(custom_cmd_tester->get_recv_cmd_base_test(), (uint8_t) 16);
}

void GUI_CUSTOM_CMD_TESTS::test_basic_features()
{
    // Test accept all cmds (returns whether accept all checkbox is checks)
    QVERIFY(!custom_cmd_tester->acceptAllCMDs());

    // Test waitForDevice (always returns false)
    QVERIFY(!custom_cmd_tester->waitForDevice(0));
    QVERIFY(!custom_cmd_tester->waitForDevice((uint8_t) qrand()));
}
