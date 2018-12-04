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

#include "gui-welcome-tests.hpp"

// Testing infrastructure includes
#include <QtTest>
#include <QSignalSpy>

#include "../../src/gui-helpers/gui-generic-helper.hpp"

GUI_WELCOME_TESTS::GUI_WELCOME_TESTS()
{
    /* DO NOTHING */
}

GUI_WELCOME_TESTS::~GUI_WELCOME_TESTS()
{
    // Delete tester if allocated
    if (welcome_tester) delete welcome_tester;
}

void GUI_WELCOME_TESTS::initTestCase()
{
    // Create object for testing
    welcome_tester = new GUI_WELCOME_TEST_CLASS();
    QVERIFY(welcome_tester);
}

void GUI_WELCOME_TESTS::cleanupTestCase()
{
    // Delete test class
    if (welcome_tester)
    {
        delete welcome_tester;
        welcome_tester = nullptr;
    }
}

void GUI_WELCOME_TESTS::test_init_vals()
{
    // Verify all init values
    QVERIFY(welcome_tester->isClosable());
    QCOMPARE(welcome_tester->get_gui_key(), (uint8_t) MAJOR_KEY_WELCOME);
    QCOMPARE(welcome_tester->get_gui_name(), QString("Welcome"));
    QCOMPARE(welcome_tester->get_gui_tab_name(), welcome_tester->get_gui_name());
    QCOMPARE(welcome_tester->get_header(),
             welcome_tester->get_gui_name());
    QCOMPARE(welcome_tester->get_msg(), QString(""));
    QCOMPARE(welcome_tester->get_buttons_enabled(), false);

    QString config_str = welcome_tester->get_gui_name().prepend("[")
            + "]\nheader=\"" + welcome_tester->get_gui_name() + "\"\n";
    QCOMPARE(welcome_tester->get_gui_config(), config_str);
}

void GUI_WELCOME_TESTS::test_basic_features()
{
    // Fetch data
    QFETCH(QString, header);
    QFETCH(QString, msg);
    QFETCH(bool, buttons);

    // Test header
    welcome_tester->set_header(header);
    QCOMPARE(welcome_tester->get_header(), QString(header));

    // Test msg
    welcome_tester->set_header(msg);
    QCOMPARE(welcome_tester->get_header(), QString(msg));

    // Test buttons
    welcome_tester->set_buttons_enabled(buttons);
    QCOMPARE(welcome_tester->get_buttons_enabled(), buttons);
}

void GUI_WELCOME_TESTS::test_basic_features_data()
{
    // Setup data columns
    QTest::addColumn<QString>("header");
    QTest::addColumn<QString>("msg");
    QTest::addColumn<bool>("buttons");

    // Load in data
    QTest::newRow("Basic") << "HEADER" << "MESSAGE" << false;
    QTest::newRow("Welcome V2.0") << "Welcome V2.0" << "Hello World!" << true;
    QTest::newRow("RESET") << welcome_tester->get_gui_name() << "" \
                           << welcome_tester->get_buttons_enabled();
}

void GUI_WELCOME_TESTS::test_gui_config()
{
    // Fetch data
    QFETCH(QString, config_str);
    QFETCH(QString, gui_tab_name);
    QFETCH(QString, header);
    QFETCH(QString, msg);
    QFETCH(bool, buttons);
    QFETCH(bool, isClosable);

    // Clear current config
    QMap<QString, QVariant> reset_map;
    welcome_tester->parseConfigMap(&reset_map);

    // Get unchanging values
    uint8_t curr_gui_key = welcome_tester->get_gui_key();
    QString curr_gui_name = welcome_tester->get_gui_name();

    // Generate new config
    CONFIG_MAP *gui_config = \
            GUI_GENERIC_HELPER::decode_configMap(config_str);

    // Parse new config
    welcome_tester->parseConfigMap(gui_config->value(curr_gui_name, nullptr));

    // Check values
    QCOMPARE(welcome_tester->get_gui_key(), curr_gui_key);
    QCOMPARE(welcome_tester->get_gui_name(), curr_gui_name);
    QCOMPARE(welcome_tester->get_gui_tab_name(), gui_tab_name);
    QCOMPARE(welcome_tester->isClosable(), isClosable);
}

void GUI_WELCOME_TESTS::test_gui_config_data()
{
    // Setup data columns
    QTest::addColumn<QString>("config_str");
    QTest::addColumn<QString>("gui_tab_name");
    QTest::addColumn<QString>("header");
    QTest::addColumn<QString>("msg");
    QTest::addColumn<bool>("buttons");
    QTest::addColumn<bool>("isClosable");

    // Helper variables
    QString config_str, header, msg;
    QString curr_gui_name = welcome_tester->get_gui_name();

    // Setup empty config str
    config_str.clear();
    config_str += "[" + curr_gui_name + "]\n";

    // Load in defaults
    QTest::newRow("Empty") << config_str \
                           << curr_gui_name \
                           << curr_gui_name \
                           << "" \
                           << false << true;

    // Setup basic config str
    header = "New Name";
    msg = "Hello World!";
    config_str.clear();
    config_str += "[" + curr_gui_name + "]\n";
    config_str += "header=\"" + header + "\"\n";
    config_str += "msg=\"" + msg + "\"\n";

    // Load in basic
    QTest::newRow("basic") << config_str \
                           << curr_gui_name \
                           << header \
                           << msg \
                           << false << true;
}


