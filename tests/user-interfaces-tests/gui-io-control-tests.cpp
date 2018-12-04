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

    // Verify init num pins
    QVERIFY(io_control_tester->check_pins_test(QStringList(), QList<QStringList>(),
                                               QList<QList<int>>(), QStringList()));
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
    QFETCH(QStringList, expected_pins);
    QFETCH(QList<QStringList>, expected_combos);
    QFETCH(QList<QList<int>>, expected_sliders);
    QFETCH(QStringList, expected_lineEdits);

    // Verify input arguments
    QCOMPARE(expected_pins.length(), expected_combos.length());

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
    QVERIFY(io_control_tester->check_pins_test(expected_pins, expected_combos,
                                               expected_sliders, expected_lineEdits));
}

void GUI_IO_CONTROL_TESTS::test_gui_config_data()
{
    // Setup data columns
    QTest::addColumn<QString>("config_str");
    QTest::addColumn<QString>("gui_tab_name");
    QTest::addColumn<bool>("isClosable");
    QTest::addColumn<QStringList>("expected_pins");
    QTest::addColumn<QList<QStringList>>("expected_combos");

    // The slider qlist<int> is setup as follows:
    //   0) Expected current value
    //   1) Expected minimum value
    //   2) Expected maximum value
    //   3) Expected step size
    QTest::addColumn<QList<QList<int>>>("expected_sliders");

    QTest::addColumn<QStringList>("expected_lineEdits");

    // Helper variables
    QString config_str;
    QString curr_gui_name = io_control_tester->get_gui_name();
    QStringList pin_list;
    QList<QStringList> combo_list;
    QStringList pin_combo_list;
    QList<QList<int>> slider_list;
    QList<int> pin_slider_list;
    QStringList lineEdit_list;

    // Setup Basic config str
    config_str.clear();
    config_str += "[" + curr_gui_name + "]\n";
    config_str += "tab_name=\"Tab A\"\n";
    config_str += "closable=\"false\"\n";

    pin_list.clear();
    combo_list.clear();
    slider_list.clear();
    lineEdit_list.clear();

    // Load in Basic
    QTest::newRow("Basic") << config_str \
                           << "Tab A" \
                           << false \
                           << pin_list \
                           << combo_list \
                           << slider_list \
                           << lineEdit_list;

    // Setup Basic DIO/AIO config str
    config_str.clear();
    config_str += "[" + curr_gui_name + "]\n";
    config_str += "tab_name=\"IO\"\n";
    config_str += "closable=\"true\"\n";
    config_str += "dio_combo_settings = \\\n";
    config_str += "\"Input,true,0-1-1-1.0\"\n";
    config_str += "dio_pin_settings = \\\n";
    config_str += "\"0=Input\"\n";
    config_str += "aio_combo_settings = \\\n";
    config_str += "\"Input,true,0-1-1-1.0\"\n";
    config_str += "aio_pin_settings = \\\n";
    config_str += "\"0=Input\"\n";

    pin_list.clear();
    pin_list << "DIO_00" << "AIO_00";

    combo_list.clear();
    pin_combo_list.clear();
    pin_combo_list << "Input";
    combo_list << pin_combo_list << pin_combo_list;

    slider_list.clear();
    pin_slider_list.clear();
    pin_slider_list << 0 << 0 << 1 << 1;
    slider_list << pin_slider_list << pin_slider_list;

    lineEdit_list.clear();
    lineEdit_list << "0" << "0";

    // Load in Basic DIO/AIO
    QTest::newRow("Basic DIO/AIO") << config_str \
                           << "IO" \
                           << true \
                           << pin_list \
                           << combo_list \
                           << slider_list \
                           << lineEdit_list;

    // Setup Complex DIO/AIO config str
    config_str.clear();
    config_str += "[" + curr_gui_name + "]\n";
    config_str += "tab_name=\"IO\"\n";
    config_str += "closable=\"true\"\n";
    config_str += "dio_combo_settings = \\\n";
    config_str += "\"Input,true,0-1-1-1.0\",\\\n";
    config_str += "\"Output,false,0-5-1-1.0\"\n";
    config_str += "dio_pin_settings = \\\n";
    config_str += "\"0,1=Input,Output\",\\\n";
    config_str += "\"3=Input\",\\\n";
    config_str += "\"4=Output\"\n";
    config_str += "aio_combo_settings = \\\n";
    config_str += "\"Input,true,0-500-50-100.0\"\n";
    config_str += "aio_pin_settings = \\\n";
    config_str += "\"0-3=Input\"\n";

    // Clear and setup pin list
    pin_list.clear();
    pin_list << "DIO_00" << "DIO_01" << "DIO_03" << "DIO_04" \
             << "AIO_00" << "AIO_01" << "AIO_02" << "AIO_03";

    // Clear combo list
    combo_list.clear();

    // DIO 0/1 are INPUT & OUTPUT combo
    pin_combo_list.clear();
    pin_combo_list << "Input" << "Output";
    combo_list << pin_combo_list << pin_combo_list;

    // DIO 3 is INPUT combo
    pin_combo_list.clear();
    pin_combo_list << "Input";
    combo_list << pin_combo_list;

    // DIO 4 is OUTPUT combo
    pin_combo_list.clear();
    pin_combo_list << "Output";
    combo_list << pin_combo_list;

    // AIO 0/1/2/3 are INPUT combo
    pin_combo_list.clear();
    pin_combo_list << "Input";
    combo_list << pin_combo_list << pin_combo_list << pin_combo_list << pin_combo_list;

    // Clear slider list
    slider_list.clear();

    // DIO 0/1/3 are INPUT slider
    pin_slider_list.clear();
    pin_slider_list << 0 << 0 << 1 << 1;
    slider_list << pin_slider_list << pin_slider_list << pin_slider_list;

    // DIO 4 is OUTPUT slider
    pin_slider_list.clear();
    pin_slider_list << 0 << 0 << 5 << 1;
    slider_list << pin_slider_list;

    // AIO 0/1/2/3 are INPUT slider
    pin_slider_list.clear();
    pin_slider_list << 0 << 0 << 500 << 50;
    slider_list << pin_slider_list << pin_slider_list << pin_slider_list << pin_slider_list;

    lineEdit_list.clear();
    lineEdit_list << "0" << "0" << "0" << "0" \
                  << "0" << "0" << "0" << "0";

    // Load in Complex DIO/AIO
    QTest::newRow("Complex DIO/AIO") << config_str \
                           << "IO" \
                           << true \
                           << pin_list \
                           << combo_list \
                           << slider_list \
                           << lineEdit_list;
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
