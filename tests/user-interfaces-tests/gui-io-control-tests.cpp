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

#include "gui-io-control-tests.hpp"

// Testing infrastructure includes
#include <QtTest>
#include <QSignalSpy>

#include "../../src/gui-helpers/gui-generic-helper.hpp"

GUI_IO_CONTROL_TESTS::GUI_IO_CONTROL_TESTS()
{
    /* DO NOTHING */
}

GUI_IO_CONTROL_TESTS::~GUI_IO_CONTROL_TESTS()
{
    // Delete tester if allocated
    if (io_control_tester) delete io_control_tester;
}

void GUI_IO_CONTROL_TESTS::initTestCase()
{
    // Create object for testing
    io_control_tester = new GUI_IO_CONTROL_TEST_CLASS();
    QVERIFY(io_control_tester);
}

void GUI_IO_CONTROL_TESTS::cleanupTestCase()
{
    // Delete test class
    if (io_control_tester)
    {
        delete io_control_tester;
        io_control_tester = nullptr;
    }
}

void GUI_IO_CONTROL_TESTS::test_init_vals()
{
    // Verify reset defaults
    verify_reset_values();

    // Verify non-reset & defualt members
    QVERIFY(io_control_tester->isClosable());
    QCOMPARE(io_control_tester->get_gui_tab_name(), io_control_tester->get_gui_name());
    QCOMPARE(io_control_tester->get_gui_config(),
             io_control_tester->get_gui_name().prepend("[").append("]\n"));

    // Verify init update info
    QCOMPARE(io_control_tester->get_aio_update_rate_test(), 1.0f);
    QCOMPARE(io_control_tester->get_dio_update_rate_test(), 1.0f);

    // Verify init log info
    QCOMPARE(io_control_tester->get_log_file_update_rate_test(), 1.0f);
    QCOMPARE(io_control_tester->get_log_file_save_path_test(), QString(""));
    QCOMPARE(io_control_tester->get_log_append_checked_test(), true);
}

void GUI_IO_CONTROL_TESTS::test_basic_features()
{
    // Test waitForDevice
    QVERIFY(!io_control_tester->waitForDevice(0));
    QVERIFY(io_control_tester->waitForDevice(MINOR_KEY_IO_REMOTE_CONN_READ));
    QVERIFY(!io_control_tester->waitForDevice(MINOR_KEY_IO_AIO_READ));
    QVERIFY(!io_control_tester->waitForDevice(MINOR_KEY_IO_DIO_READ));
}

void GUI_IO_CONTROL_TESTS::test_gui_config()
{
    // Fetch data
    QFETCH(QString, config_str);
    QFETCH(QString, gui_tab_name);
    QFETCH(bool, isClosable);

    // Clear current config
    QMap<QString, QVariant> reset_map;
    io_control_tester->parseConfigMap(&reset_map);

    // Get gui values
    uint8_t curr_gui_key = io_control_tester->get_gui_key();
    QString curr_gui_name = io_control_tester->get_gui_name();

    // Generate new config
    CONFIG_MAP *gui_config = \
            GUI_GENERIC_HELPER::decode_configMap(config_str);

    // Parse new config
    io_control_tester->parseConfigMap(gui_config->value(curr_gui_name, nullptr));

    // Check values
    QCOMPARE(io_control_tester->get_gui_key(), curr_gui_key);
    QCOMPARE(io_control_tester->get_gui_name(), curr_gui_name);
    QCOMPARE(io_control_tester->get_gui_tab_name(), gui_tab_name);
    QCOMPARE(io_control_tester->isClosable(), isClosable);
}

void GUI_IO_CONTROL_TESTS::test_gui_config_data()
{
    // Setup data columns
    QTest::addColumn<QString>("config_str");
    QTest::addColumn<QString>("gui_tab_name");
    QTest::addColumn<bool>("isClosable");

    // Helper variables
    QString config_str;
    QString curr_gui_name = io_control_tester->get_gui_name();

    // Setup Basic config str
    config_str.clear();
    config_str += "[" + curr_gui_name + "]\n";
    config_str += "tab_name=\"Tab A\"\n";
    config_str += "closable=\"false\"\n";

    // Load in Basic
    QTest::newRow("Basic") << config_str \
                           << "Tab A" \
                           << false;

    // Setup Basic DIO/AIO config str
    config_str.clear();
    config_str += "[" + curr_gui_name + "]\n";
    config_str += "tab_name=\"IO\"\n";
    config_str += "closable=\"true\"\n";
    config_str += "dio_combo_settings=\"Input,true,0-1-1-1.0\"\n";
    config_str += "dio_pin_settings=\"0=Input\"\n";
    config_str += "aio_combo_settings=\"Input,true,0-1-1-1.0\"\n";
    config_str += "aio_pin_settings=\"0=Input\"\n";

    // Load in Basic DIO/AIO
    QTest::newRow("Basic DIO/AIO") << config_str \
                           << "IO" \
                           << true;
}

void GUI_IO_CONTROL_TESTS::verify_reset_values()
{
    // Check class memebrs
    QCOMPARE(io_control_tester->get_gui_key(), (uint8_t) MAJOR_KEY_IO);
    QCOMPARE(io_control_tester->get_gui_name(), QString("IO"));
    QCOMPARE(io_control_tester->rcvd_formatted_size_test(), (qint64) 0);

    // Check update rate
    QCOMPARE(io_control_tester->get_update_rate_start_text_test(), QString("Start"));
}
