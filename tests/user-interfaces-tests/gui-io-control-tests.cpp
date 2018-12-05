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

    // Verify no pins
    QVERIFY(io_control_tester->check_pins_test(QStringList(), QList<QStringList>(),
                                               QList<QList<int>>(), QStringList(),
                                               QList<bool>()));
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
    QFETCH(QList<bool>, expected_disabled);

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
                                               expected_sliders, expected_lineEdits,
                                               expected_disabled));
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
    QTest::addColumn<QList<bool>>("expected_disabled");

    // Helper variables
    QString config_str;
    QString curr_gui_name = io_control_tester->get_gui_name();
    QStringList pin_list;
    QList<QStringList> combo_list;
    QStringList pin_combo_list;
    QList<QList<int>> slider_list;
    QList<int> pin_slider_list;
    QStringList lineEdit_list;
    QList<bool> disabled_list;

    // Setup Basic config str
    config_str.clear();
    config_str += "[" + curr_gui_name + "]\n";
    config_str += "tab_name=\"Tab A\"\n";
    config_str += "closable=\"false\"\n";

    pin_list.clear();
    combo_list.clear();
    slider_list.clear();
    lineEdit_list.clear();
    disabled_list.clear();

    // Load in Basic
    QTest::newRow("Basic") << config_str \
                           << "Tab A" \
                           << false \
                           << pin_list \
                           << combo_list \
                           << slider_list \
                           << lineEdit_list \
                           << disabled_list;

    // Setup Basic DIO/AIO test data
    config_str.clear();
    pin_list.clear();
    combo_list.clear();
    slider_list.clear();
    lineEdit_list.clear();
    disabled_list.clear();

    // Setup Basic DIO/AIO config_str
    config_str += "[" + curr_gui_name + "]\n";
    config_str += "tab_name=\"IO\"\n";
    config_str += "closable=\"true\"\n";
    config_str += "dio_combo_settings = \\\n";
    config_str += "\"Input,true,0:1:1:1.0\"\n";
    config_str += "dio_pin_settings = \\\n";
    config_str += "\"0=Input\"\n";
    config_str += "aio_combo_settings = \\\n";
    config_str += "\"Input,true,0:1:1:1.0\"\n";
    config_str += "aio_pin_settings = \\\n";
    config_str += "\"0=Input\"\n";

    // DIO 0 are INPUT
    pin_list << "DIO_00";

    pin_combo_list.clear();
    pin_combo_list << "Input";
    combo_list << pin_combo_list;

    pin_slider_list.clear();
    pin_slider_list << 0 << 0 << 1 << 1;
    slider_list << pin_slider_list;

    lineEdit_list << "0";
    disabled_list << true;

    // AIO 0 are INPUT
    pin_list << "AIO_00";

    pin_combo_list.clear();
    pin_combo_list << "Input";
    combo_list << pin_combo_list;

    pin_slider_list.clear();
    pin_slider_list << 0 << 0 << 1 << 1;
    slider_list << pin_slider_list;

    lineEdit_list << "0";
    disabled_list << true;

    // Load in Basic DIO/AIO
    QTest::newRow("Basic DIO/AIO") << config_str \
                                   << "IO" \
                                   << true \
                                   << pin_list \
                                   << combo_list \
                                   << slider_list \
                                   << lineEdit_list \
                                   << disabled_list;

    // Setup Complex DIO/AIO config_str data
    config_str.clear();
    pin_list.clear();
    combo_list.clear();
    slider_list.clear();
    lineEdit_list.clear();
    disabled_list.clear();

    // Setup config_str
    config_str += "[" + curr_gui_name + "]\n";
    config_str += "tab_name=\"IO\"\n";
    config_str += "closable=\"true\"\n";
    config_str += "dio_combo_settings = \\\n";
    config_str += "\"Input,true,0:1:1:1.0\",\\\n";
    config_str += "\"Output,false,0:5:1:1.0\"\n";
    config_str += "dio_pin_settings = \\\n";
    config_str += "\"0,1=Input,Output\",\\\n";
    config_str += "\"4=Output\"\n";
    config_str += "aio_combo_settings = \\\n";
    config_str += "\"Input,true,0:500:50:100.0\"\n";
    config_str += "aio_pin_settings = \\\n";
    config_str += "\"0:1=Input\"\n";

    // DIO 0/1 are INPUT & OUTPUT
    pin_list << "DIO_00" << "DIO_01";

    pin_combo_list.clear();
    pin_combo_list << "Input" << "Output";
    combo_list << pin_combo_list << pin_combo_list;

    pin_slider_list.clear();
    pin_slider_list << 0 << 0 << 1 << 1;
    slider_list << pin_slider_list << pin_slider_list;

    lineEdit_list << "0" << "0";
    disabled_list << true << true;

    // DIO 4 are OUTPUT
    pin_list << "DIO_04";

    pin_combo_list.clear();
    pin_combo_list << "Output";
    combo_list << pin_combo_list;

    pin_slider_list.clear();
    pin_slider_list << 0 << 0 << 5 << 1;
    slider_list << pin_slider_list;

    lineEdit_list << "0";
    disabled_list << false;

    // AIO 0/1 are INPUT
    pin_list << "AIO_00" << "AIO_01";

    pin_combo_list.clear();
    pin_combo_list << "Input";
    combo_list << pin_combo_list << pin_combo_list;

    pin_slider_list.clear();
    pin_slider_list << 0 << 0 << 500 << 50;
    slider_list << pin_slider_list << pin_slider_list;

    lineEdit_list << "0" << "0";
    disabled_list << true << true;

    // Load in Complex DIO/AIO data
    QTest::newRow("Complex DIO/AIO") << config_str \
                                     << "IO" \
                                     << true \
                                     << pin_list \
                                     << combo_list \
                                     << slider_list \
                                     << lineEdit_list \
                                     << disabled_list;

    // Setup Verify GUI Interactions Test config_str test data
    config_str.clear();
    pin_list.clear();
    combo_list.clear();
    slider_list.clear();
    lineEdit_list.clear();
    disabled_list.clear();

    // Setup config_str
    config_str += "[" + curr_gui_name + "]\n";
    config_str += "tab_name=\"IO\"\n";
    config_str += "closable=\"true\"\n";
    config_str += "dio_combo_settings = \\\n";
    config_str += "\"Input,true,0:1:1:1.0\",\\\n";
    config_str += "\"Output,false,0:5:1:1.0\",\\\n";
    config_str += "\"Neg_Out,false,-5:5:1:1.0\"\n";
    config_str += "dio_pin_settings = \\\n";
    config_str += "\"0,1=Input,Output\",\\\n";
    config_str += "\"3=Input\",\\\n";
    config_str += "\"4=Output\",\\\n";
    config_str += "\"5=Input,Neg_Out\"\n";
    config_str += "aio_combo_settings = \\\n";
    config_str += "\"Input,true,0:500:50:100.0\",\\\n";
    config_str += "\"Output,false,0:500:50:100.0\"\n";
    config_str += "aio_pin_settings = \\\n";
    config_str += "\"0:3=Input\",\\\n";
    config_str += "\"4=Output\",\\\n";
    config_str += "\"5=Input,Output\"\n";

    // DIO 0/1 are INPUT & OUTPUT
    pin_list << "DIO_00" << "DIO_01";

    pin_combo_list.clear();
    pin_combo_list << "Input" << "Output";
    combo_list << pin_combo_list << pin_combo_list;

    pin_slider_list.clear();
    pin_slider_list << 0 << 0 << 1 << 1;
    slider_list << pin_slider_list << pin_slider_list;

    lineEdit_list << "0" << "0";
    disabled_list << true << true;

    // DIO 3 are INPUT
    pin_list << "DIO_03";

    pin_combo_list.clear();
    pin_combo_list << "Input";
    combo_list << pin_combo_list;

    pin_slider_list.clear();
    pin_slider_list << 0 << 0 << 1 << 1;
    slider_list << pin_slider_list;

    lineEdit_list << "0";
    disabled_list << true;

    // DIO 4 are OUTPUT
    pin_list << "DIO_04";

    pin_combo_list.clear();
    pin_combo_list << "Output";
    combo_list << pin_combo_list;

    pin_slider_list.clear();
    pin_slider_list << 0 << 0 << 5 << 1;
    slider_list << pin_slider_list;

    lineEdit_list << "0";
    disabled_list << false;

    // DIO 5 are INPUT & NEG_OUT
    pin_list << "DIO_05";

    pin_combo_list.clear();
    pin_combo_list << "Input" << "Neg_Out";
    combo_list << pin_combo_list;

    pin_slider_list.clear();
    pin_slider_list << 0 << 0 << 1 << 1;
    slider_list << pin_slider_list;

    lineEdit_list << "0";
    disabled_list << true;

    // AIO 0/1/2/3 are INPUT
    pin_list << "AIO_00" << "AIO_01" << "AIO_02" << "AIO_03";

    pin_combo_list.clear();
    pin_combo_list << "Input";
    combo_list << pin_combo_list << pin_combo_list << pin_combo_list << pin_combo_list;

    pin_slider_list.clear();
    pin_slider_list << 0 << 0 << 500 << 50;
    slider_list << pin_slider_list << pin_slider_list << pin_slider_list << pin_slider_list;

    lineEdit_list << "0" << "0" << "0" << "0";
    disabled_list << true << true << true << true;

    // AIO 4 are OUTPUT
    pin_list << "AIO_04";

    pin_combo_list.clear();
    pin_combo_list << "Output";
    combo_list << pin_combo_list;

    pin_slider_list.clear();
    pin_slider_list << 0 << 0 << 500 << 50;
    slider_list << pin_slider_list;

    lineEdit_list << "0";
    disabled_list << false;

    // AIO 5 are INPUT & OUTPUT
    pin_list << "AIO_05";

    pin_combo_list.clear();
    pin_combo_list << "Input" << "Output";
    combo_list << pin_combo_list;

    pin_slider_list.clear();
    pin_slider_list << 0 << 0 << 500 << 50;
    slider_list << pin_slider_list;

    lineEdit_list << "0";
    disabled_list << true;

    // Load in Verify GUI Interactions Test config_str test data
    QTest::newRow("Verify test_gui_interactions config_str") \
            << config_str \
            << "IO" \
            << true \
            << pin_list \
            << combo_list \
            << slider_list \
            << lineEdit_list \
            << disabled_list;

    // Setup Centered 0 Range test data
    config_str.clear();
    pin_list.clear();
    combo_list.clear();
    slider_list.clear();
    lineEdit_list.clear();
    disabled_list.clear();

    // Setup config_str
    config_str += "[" + curr_gui_name + "]\n";
    config_str += "tab_name=\"IO\"\n";
    config_str += "closable=\"true\"\n";
    config_str += "dio_combo_settings = \\\n";
    config_str += "\"Input,true,-1:1:1:1.0\"\n";
    config_str += "dio_pin_settings = \\\n";
    config_str += "\"0=Input\"\n";

    // DIO 0 are INPUT
    pin_list << "DIO_00";

    pin_combo_list.clear();
    pin_combo_list << "Input";
    combo_list << pin_combo_list;

    pin_slider_list.clear();
    pin_slider_list << 0 << -1 << 1 << 1;
    slider_list << pin_slider_list;

    lineEdit_list << "0";
    disabled_list << true;

    // Load Centered 0 Range data
    QTest::newRow("Centered 0 Range") << config_str \
                                      << "IO" \
                                      << true \
                                      << pin_list \
                                      << combo_list \
                                      << slider_list \
                                      << lineEdit_list \
                                      << disabled_list;

    // Setup Full Positive Range test data
    config_str.clear();
    pin_list.clear();
    combo_list.clear();
    slider_list.clear();
    lineEdit_list.clear();
    disabled_list.clear();

    // Setup config_str
    config_str += "[" + curr_gui_name + "]\n";
    config_str += "tab_name=\"IO\"\n";
    config_str += "closable=\"true\"\n";
    config_str += "dio_combo_settings = \\\n";
    config_str += "\"Input,true,1:5:1:1.0\"\n";
    config_str += "dio_pin_settings = \\\n";
    config_str += "\"0=Input\"\n";

    // DIO 0 are INPUT
    pin_list << "DIO_00";

    pin_combo_list.clear();
    pin_combo_list << "Input";
    combo_list << pin_combo_list;

    pin_slider_list.clear();
    pin_slider_list << 1 << 1 << 5 << 1;
    slider_list << pin_slider_list;

    lineEdit_list << "1";
    disabled_list << true;

    // Load Full Positive Range test data
    QTest::newRow("Full Positive Range") << config_str \
                                         << "IO" \
                                         << true \
                                         << pin_list \
                                         << combo_list \
                                         << slider_list \
                                         << lineEdit_list \
                                         << disabled_list;

    // Setup Full Negative Range test data
    config_str.clear();
    pin_list.clear();
    combo_list.clear();
    slider_list.clear();
    lineEdit_list.clear();
    disabled_list.clear();

    // Setup config_str
    config_str += "[" + curr_gui_name + "]\n";
    config_str += "tab_name=\"IO\"\n";
    config_str += "closable=\"true\"\n";
    config_str += "dio_combo_settings = \\\n";
    config_str += "\"Input,true,-5:-1:1:1.0\"\n";
    config_str += "dio_pin_settings = \\\n";
    config_str += "\"0=Input\"\n";

    // DIO 0 are INPUT
    pin_list << "DIO_00";

    pin_combo_list.clear();
    pin_combo_list << "Input";
    combo_list << pin_combo_list;

    pin_slider_list.clear();
    pin_slider_list << -1 << -5 << -1 << 1;
    slider_list << pin_slider_list;

    lineEdit_list << "-1";
    disabled_list << true;

    // Load Full Negative Range test data
    QTest::newRow("Full Negative Range") << config_str \
                                         << "IO" \
                                         << true \
                                         << pin_list \
                                         << combo_list \
                                         << slider_list \
                                         << lineEdit_list \
                                         << disabled_list;
}

void GUI_IO_CONTROL_TESTS::test_gui_interactions()
{
    // Fetch data
    QFETCH(QString, config_str);
    QFETCH(QList<QList<QString>>, actions);
    QFETCH(QList<QList<QByteArray>>, expected_signals);
    QFETCH(QList<QList<QString>>, expected_pin_settings);

    // Verify length match
    QCOMPARE(actions.length(), expected_signals.length());
    QCOMPARE(actions.length(), expected_pin_settings.length());

    // Clear current config
    QMap<QString, QVariant> reset_map;
    io_control_tester->parseConfigMap(&reset_map);

    // Get gui values
    QString curr_gui_name = io_control_tester->get_gui_name();

    // Generate new config
    CONFIG_MAP *gui_config = \
            GUI_GENERIC_HELPER::decode_configMap(config_str);

    // Parse new config
    io_control_tester->parseConfigMap(gui_config->value(curr_gui_name, nullptr));

    // Setup spy to catch tranmit signal
    QList<QVariant> spy_args;
    QSignalSpy transmit_chunk_spy(io_control_tester, io_control_tester->transmit_chunk);
    QVERIFY(transmit_chunk_spy.isValid());

    // Setup action loop variables
    QList<QString> action;
    QList<QByteArray> action_signals;
    QList<QString> action_pin_settings;
    int num_actions = actions.length();

    // Perform all actions
    for (int i = 0; i < num_actions; i++)
    {
        // Get current action info
        action = actions.at(i);
        action_signals = expected_signals.at(i);
        action_pin_settings = expected_pin_settings.at(i);

        // Verify action lengths
        QCOMPARE(action.length(), (int) 3);
        QCOMPARE(action_pin_settings.length(), (int) 4);

        // Perform action
        io_control_tester->perform_action_test(action.at(0),
                                               (uint8_t) action.at(1).toUInt(),
                                               action.at(2));

        // Get & verify signals
        QCOMPARE(transmit_chunk_spy.count(), action_signals.length());
        foreach (QByteArray signal, action_signals)
        {
            // Get next spy signal
            spy_args = transmit_chunk_spy.takeFirst();

            // Verify signal values
            QVERIFY(2 <= signal.length());
            QCOMPARE(spy_args.at(0).toInt(), (int) signal.at(0));
            QCOMPARE(spy_args.at(1).toInt(), (int) signal.at(1));
            QCOMPARE(spy_args.at(2).toByteArray(), signal.mid(2));
        }

        // Verify GUI values
        QVERIFY(io_control_tester->check_pin_test(action.at(0),
                                                  action_pin_settings.at(0),
                                                  action_pin_settings.at(1).toInt(),
                                                  action_pin_settings.at(2),
                                                  action_pin_settings.at(3).toLower() == "true"));
    }
}

void GUI_IO_CONTROL_TESTS::test_gui_interactions_data()
{
    // Setup data columns
    QTest::addColumn<QString>("config_str");

    // Each action item is composed as follows:
    //   0) pin_str
    //   1) button pos
    //   2) set value
    QTest::addColumn<QList<QList<QString>>>("actions");

    // Each expected signal item is composed as follows:
    //   0) Major Key
    //   1) Minor Key
    //   2-end) Data
    QTest::addColumn<QList<QList<QByteArray>>>("expected_signals");

    // Each pin setting item is composed as follows:
    //   0) Combo Value
    //   1) Slider Value
    //   2) Line Edit Value
    //   3) isDisabled
    QTest::addColumn<QList<QList<QString>>>("expected_pin_settings");

    // Helper variables
    QString config_str;
    QString curr_gui_name = io_control_tester->get_gui_name();
    QList<QList<QString>> action_list;
    QList<QList<QByteArray>> action_signals_list;
    QList<QByteArray> action_signal;
    QList<QList<QString>> action_settings_list;

    // Setup config_str for all tests
    config_str.clear();
    config_str += "[" + curr_gui_name + "]\n";
    config_str += "tab_name=\"IO\"\n";
    config_str += "closable=\"true\"\n";
    config_str += "dio_combo_settings = \\\n";
    config_str += "\"Input,true,0:1:1:1.0\",\\\n";
    config_str += "\"Output,false,0:5:1:1.0\",\\\n";
    config_str += "\"Neg_Out,false,-5:5:1:1.0\"\n";
    config_str += "dio_pin_settings = \\\n";
    config_str += "\"0,1=Input,Output\",\\\n";
    config_str += "\"3=Input\",\\\n";
    config_str += "\"4=Output\",\\\n";
    config_str += "\"5=Input,Neg_Out\"\n";
    config_str += "aio_combo_settings = \\\n";
    config_str += "\"Input,true,0:500:50:100.0\",\\\n";
    config_str += "\"Output,false,0:500:50:100.0\"\n";
    config_str += "aio_pin_settings = \\\n";
    config_str += "\"0:3=Input\",\\\n";
    config_str += "\"4=Output\",\\\n";
    config_str += "\"5=Input,Output\"\n";

    // Setup basic test (performs no actions)
    action_list.clear();
    action_signals_list.clear();
    action_settings_list.clear();

    // Load basic test (performs no actions)
    QTest::newRow("Basic") << config_str \
                           << action_list \
                           << action_signals_list \
                           << action_settings_list;

    // Setup basic DIO verify test
    action_list.clear();
    action_signals_list.clear();
    action_settings_list.clear();

    // Add first action (Change DIO_00 to Output)
    action_list << QList<QString>({"DIO_00", QString::number(io_combo_pos), "Output"});
    action_signal.clear();
    action_signal.append(GUI_GENERIC_HELPER::qList_to_byteArray(
                             {
                                 MAJOR_KEY_IO,
                                 MINOR_KEY_IO_DIO_SET,
                                 0x00, 0x00, 0x00, 0x01
                             }));
    action_signals_list << action_signal;
    action_settings_list << QList<QString>({"Output", "0", "0", "false"});

    // Add second action (Set DIO_00 to 1 using slider)
    action_list << QList<QString>({"DIO_00", QString::number(io_slider_pos), "1"});
    action_signal.clear();
    action_signal.append(GUI_GENERIC_HELPER::qList_to_byteArray(
                             {
                                 MAJOR_KEY_IO,
                                 MINOR_KEY_IO_DIO_WRITE,
                                 0x00, 0x00, 0x01
                             }));
    action_signals_list << action_signal;
    action_settings_list << QList<QString>({"Output", "1", "1", "false"});

    // Load basic DIO verify test.
    // Performs the following actions:
    //   1) Change DIO_00 to Output
    //   2) Set DIO_00 to 1 using slider
    QTest::newRow("Basic DIO Verify") << config_str \
                                      << action_list \
                                      << action_signals_list \
                                      << action_settings_list;

    // Setup basic AIO verify test
    action_list.clear();
    action_signals_list.clear();
    action_settings_list.clear();

    // Add first action (Set AIO_04 to 1 using line edit)
    action_list << QList<QString>({"AIO_04", QString::number(io_line_edit_pos), "1"});
    action_signal.clear();
    action_signal.append(GUI_GENERIC_HELPER::qList_to_byteArray(
                             {
                                 MAJOR_KEY_IO,
                                 MINOR_KEY_IO_AIO_WRITE,
                                 0x04, 0x00, 0x64
                             }));
    action_signals_list << action_signal;
    action_settings_list << QList<QString>({"Output", "100", "1", "false"});

    // Load basic AIO verify test.
    // Performs the following actions:
    //   1) Set AIO_04 to 1 using line edit
    QTest::newRow("Basic AIO Verify") << config_str \
                                      << action_list \
                                      << action_signals_list \
                                      << action_settings_list;

    // Setup Complex DIO/AIO Actions test data
    action_list.clear();
    action_signals_list.clear();
    action_settings_list.clear();

    // Add first action (Change DIO_00 to Output)
    action_list << QList<QString>({"DIO_00", QString::number(io_combo_pos), "Output"});
    action_signal.clear();
    action_signal.append(GUI_GENERIC_HELPER::qList_to_byteArray(
                             {
                                 MAJOR_KEY_IO,
                                 MINOR_KEY_IO_DIO_SET,
                                 0x00, 0x00, 0x00, 0x01
                             }));
    action_signals_list << action_signal;
    action_settings_list << QList<QString>({"Output", "0", "0", "false"});

    // Add second action (Change DIO_05 to Neg_Out)
    action_list << QList<QString>({"DIO_05", QString::number(io_combo_pos), "Neg_Out"});
    action_signal.clear();
    action_signal.append(GUI_GENERIC_HELPER::qList_to_byteArray(
                             {
                                 MAJOR_KEY_IO,
                                 MINOR_KEY_IO_DIO_SET,
                                 0x05, 0x00, 0x05, 0x02
                             }));
    action_signals_list << action_signal;
    action_settings_list << QList<QString>({"Neg_Out", "0", "0", "false"});

    // Add third action (Set DIO_00 to 1 using slider)
    action_list << QList<QString>({"DIO_00", QString::number(io_slider_pos), "1"});
    action_signal.clear();
    action_signal.append(GUI_GENERIC_HELPER::qList_to_byteArray(
                             {
                                 MAJOR_KEY_IO,
                                 MINOR_KEY_IO_DIO_WRITE,
                                 0x00, 0x00, 0x01
                             }));
    action_signals_list << action_signal;
    action_settings_list << QList<QString>({"Output", "1", "1", "false"});


    // Add fourth action (Set DIO_05 to -1 using line edit)
    action_list << QList<QString>({"DIO_05", QString::number(io_line_edit_pos), "-1"});
    action_signal.clear();
    action_signal.append(GUI_GENERIC_HELPER::qList_to_byteArray(
                             {
                                 MAJOR_KEY_IO,
                                 MINOR_KEY_IO_DIO_WRITE,
                                 0x05, 0x00, 0x04
                             }));
    action_signals_list << action_signal;
    action_settings_list << QList<QString>({"Neg_Out", "-1", "-1", "false"});

    // Add fifth action (Set DIO_05 to -2 using slider)
    action_list << QList<QString>({"DIO_05", QString::number(io_slider_pos), "-2"});
    action_signal.clear();
    action_signal.append(GUI_GENERIC_HELPER::qList_to_byteArray(
                             {
                                 MAJOR_KEY_IO,
                                 MINOR_KEY_IO_DIO_WRITE,
                                 0x05, 0x00, 0x03
                             }));
    action_signals_list << action_signal;
    action_settings_list << QList<QString>({"Neg_Out", "-2", "-2", "false"});

    // Add sixth action (Set AIO_04 to 1.5 using slider)
    action_list << QList<QString>({"AIO_04", QString::number(io_slider_pos), "150"});
    action_signal.clear();
    action_signal.append(GUI_GENERIC_HELPER::qList_to_byteArray(
                             {
                                 MAJOR_KEY_IO,
                                 MINOR_KEY_IO_AIO_WRITE,
                                 0x04, 0x00, 0x96
                             }));
    action_signals_list << action_signal;
    action_settings_list << QList<QString>({"Output", "150", "1.5", "false"});

    // Add seventh action (Set AIO_04 to 2.75 using lineEdit)
    action_list << QList<QString>({"AIO_04", QString::number(io_line_edit_pos), "2.75"});
    action_signal.clear();
    action_signal.append(GUI_GENERIC_HELPER::qList_to_byteArray(
                             {
                                 MAJOR_KEY_IO,
                                 MINOR_KEY_IO_AIO_WRITE,
                                 0x04, 0x01, 0x13
                             }));
    action_signals_list << action_signal;
    action_settings_list << QList<QString>({"Output", "275", "2.75", "false"});

    // Load Complex DIO/AIO Actions test data.
    // Performs the following actions:
    //   1) Change DIO_00 to Output
    //   2) Change DIO_05 to Neg_Out
    //   3) Set DIO_00 to 1 using slider
    //   4) Set DIO_05 to -1 using line edit
    //   5) Set DIO_05 to -2 using slider
    //   6) Set AIO_04 to 1.5 using slider
    //   7) Set AIO_04 to 2.75 using lineEdit
    QTest::newRow("Complex DIO/AIO Actions") << config_str \
                                             << action_list \
                                             << action_signals_list \
                                             << action_settings_list;
}

void GUI_IO_CONTROL_TESTS::test_updates()
{
    // Start the updater and wait for various amounts of time to verify signals
}

void GUI_IO_CONTROL_TESTS::test_updates_data()
{
    // Start the updater and wait for various amounts of time to verify signals
}

void GUI_IO_CONTROL_TESTS::test_logging()
{
    // Start the log and wait for various amounts of time to verify signals/file
}

void GUI_IO_CONTROL_TESTS::test_logging_data()
{
    // Start the log and wait for various amounts of time to verify signals/file
}

void GUI_IO_CONTROL_TESTS::test_complex_recv()
{
    // Recv data and verify results
}

void GUI_IO_CONTROL_TESTS::test_complex_recv_data()
{
    // Setup recv data and expected results
}

void GUI_IO_CONTROL_TESTS::test_basic_chart_features()
{
    // Chart creation and destruction
}

void GUI_IO_CONTROL_TESTS::test_complex_chart_features()
{
    // pin_update and config switches
}

void GUI_IO_CONTROL_TESTS::test_chart_update_features()
{
    // Populate expected data/pins
}

void GUI_IO_CONTROL_TESTS::test_chart_update_features_data()
{
    // Set expected data/pins
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
