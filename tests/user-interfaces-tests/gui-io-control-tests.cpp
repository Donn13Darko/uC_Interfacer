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

// Object includes
#include <QFile>
#include <QTemporaryFile>
#include <QMessageBox>

// Setup global_config_str
const QString
GUI_IO_CONTROL_TESTS::generic_config_str = \
        "[IO]\n" \
        "tab_name=\"IO\"\n" \
        "closable=\"true\"\n" \
        "dio_combo_settings = \\\n" \
        "\"Input,true,0:1:1:1.0\",\\\n" \
        "\"Output,false,0:5:1:1.0\",\\\n" \
        "\"Neg_Out,false,-5:5:1:1.0\"\n" \
        "dio_pin_settings = \\\n" \
        "\"0,1=Input,Output\",\\\n" \
        "\"3=Input\",\\\n" \
        "\"4=Output\",\\\n" \
        "\"5=Input,Neg_Out\"\n" \
        "aio_combo_settings = \\\n" \
        "\"Input,true,0:500:50:100.0\",\\\n" \
        "\"Output,false,0:500:50:100.0\"\n" \
        "aio_pin_settings = \\\n" \
        "\"0:3=Input\",\\\n" \
        "\"4=Output\",\\\n" \
        "\"5=Input,Output\"\n";

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

    // Create temporary fileName
    QTemporaryFile *tmp_file = new QTemporaryFile();
    tmp_file->setAutoRemove(true);
    QVERIFY(tmp_file->open());
    tmp_file->close();
    temp_filename = tmp_file->fileName();
    delete tmp_file;
    QVERIFY(!temp_filename.isEmpty());
}

void GUI_IO_CONTROL_TESTS::cleanupTestCase()
{
    // Cleanup tempFile
    QVERIFY(remove_tempFile());

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
    QVERIFY(io_control_tester->waitForDevice(MINOR_KEY_IO_REMOTE_CONN_SET));
    QVERIFY(io_control_tester->waitForDevice(MINOR_KEY_IO_REMOTE_CONN_READ));
    QVERIFY(io_control_tester->waitForDevice(MINOR_KEY_IO_REMOTE_CONN_SEND));
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

    // Clear existing config & reset GUI
    clear_gui_config();
    io_control_tester->reset_gui();

    // Get default gui values
    uint8_t curr_gui_key = io_control_tester->get_gui_key();
    QString curr_gui_name = io_control_tester->get_gui_name();

    // Set to new config
    QVERIFY(set_gui_config(config_str));

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
    QStringList pin_list;
    QList<QStringList> combo_list;
    QStringList pin_combo_list;
    QList<QList<int>> slider_list;
    QList<int> pin_slider_list;
    QStringList lineEdit_list;
    QList<bool> disabled_list;
    QString curr_gui_name = io_control_tester->get_gui_name();

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

    // Setup Verify generic_config_str test data
    config_str.clear();
    pin_list.clear();
    combo_list.clear();
    slider_list.clear();
    lineEdit_list.clear();
    disabled_list.clear();

    // Setup config_str
    config_str += generic_config_str;

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
    QFETCH(QList<QList<QVariant>>, actions);
    QFETCH(QList<QList<QVariant>>, send_expected_signals);
    QFETCH(QList<QList<QVariant>>, expected_pin_settings);

    // Verify length match
    int num_actions = actions.length();
    QCOMPARE(send_expected_signals.length(), num_actions);
    QCOMPARE(expected_pin_settings.length(), num_actions);

    // Set new config & reset GUI
    QVERIFY(set_gui_config(generic_config_str));
    io_control_tester->reset_gui();

    // Setup spy to catch tranmit signal
    QList<QVariant> spy_args;
    QSignalSpy transmit_chunk_spy(io_control_tester, io_control_tester->transmit_chunk);
    QVERIFY(transmit_chunk_spy.isValid());

    // Setup action loop variables
    QList<QVariant> action;
    QList<QVariant> action_signal;
    QList<QVariant> action_pin_values;

    // Perform all actions
    for (int i = 0; i < num_actions; i++)
    {
        // Get current action info
        action = actions.at(i);
        action_signal = send_expected_signals.at(i);
        action_pin_values = expected_pin_settings.at(i);

        // Verify action lengths
        QVERIFY(action.length() == 3);
        QVERIFY(action_pin_values.length() == 5);

        // Perform action
        io_control_tester->perform_action_test(action.at(0).toString(),
                                               action.at(1).toUInt(),
                                               action.at(2));

        // Get & verify signals
        if (!action_signal.isEmpty())
        {
            // Verify signals
            QVERIFY(transmit_chunk_spy.count() == 1);
            QVERIFY(action_signal.length() == 3);

            // Get next spy signal
            spy_args = transmit_chunk_spy.takeFirst();

            // Verify signal values
            QVERIFY(action_signal.length() == 3);
            QCOMPARE(spy_args.at(0).toUInt(), action_signal.at(0).toUInt());
            QCOMPARE(spy_args.at(1).toUInt(), action_signal.at(1).toUInt());
            QCOMPARE(spy_args.at(2).toByteArray(), action_signal.at(2).toByteArray());

            // Verify GUI values
            QVERIFY(io_control_tester->check_pin_test(action_pin_values.at(0).toString(),
                                                      action_pin_values.at(1).toString(),
                                                      action_pin_values.at(2).toInt(),
                                                      action_pin_values.at(3).toString(),
                                                      action_pin_values.at(4).toBool()));
        }
    }
}

void GUI_IO_CONTROL_TESTS::test_gui_interactions_data()
{
    // Setup data columns
    // Each action item is composed as follows:
    //   0) pin_str
    //   1) button pos
    //   2) set value
    QTest::addColumn<QList<QList<QVariant>>>("actions");

    // Send expected signals column (emit transmit_chunk())
    // Each QList<QVariant> must be arranged as follows:
    //  0) Major Key (uint8_t)
    //  1) Minor Key (uint8_t)
    //  2) Data Packet (QByteArray)
    QTest::addColumn<QList<QList<QVariant>>>("send_expected_signals");

    // Each pin setting item is composed as follows:
    //   0) pin_str
    //   1) Combo Value
    //   2) Slider Value
    //   3) Line Edit Value
    //   4) isDisabled
    QTest::addColumn<QList<QList<QVariant>>>("expected_pin_settings");

    // Helper variables
    QList<QList<QVariant>> action_list;
    QList<QList<QVariant>> action_signals_list;
    QList<QVariant> action_signal;
    QList<QList<QVariant>> action_settings_list;

    // Setup Verify test (performs no actions)
    action_list.clear();
    action_signals_list.clear();
    action_settings_list.clear();

    // Load Verify test (performs no actions)
    QTest::newRow("Verify") << action_list \
                            << action_signals_list \
                            << action_settings_list;

    // Setup basic DIO verify test
    action_list.clear();
    action_signals_list.clear();
    action_settings_list.clear();

    // Add first action (Change DIO_00 to Output)
    action_list << QList<QVariant>({"DIO_00", io_combo_pos, "Output"});
    action_signal.clear();
    action_signal << QVariant(MAJOR_KEY_IO) \
                  << QVariant(MINOR_KEY_IO_DIO_SET) \
                  << GUI_GENERIC_HELPER::qList_to_byteArray({0x00, 0x00, 0x00, 0x01});
    action_signals_list << action_signal;
    action_settings_list << QList<QVariant>({"DIO_00", "Output", 0, "0", false});

    // Add second action (Set DIO_00 to 1 using slider)
    action_list << QList<QVariant>({"DIO_00", io_slider_pos, 1});
    action_signal.clear();
    action_signal << QList<QVariant>({
                                         QVariant(MAJOR_KEY_IO),
                                         QVariant(MINOR_KEY_IO_DIO_WRITE),
                                         GUI_GENERIC_HELPER::qList_to_byteArray({0x00, 0x00, 0x01})
                                     });
    action_signals_list << action_signal;
    action_settings_list << QList<QVariant>({"DIO_00", "Output", 1, "1", false});

    // Load basic DIO verify test.
    // Performs the following actions:
    //   1) Change DIO_00 to Output
    //   2) Set DIO_00 to 1 using slider
    QTest::newRow("Basic DIO Verify") << action_list \
                                      << action_signals_list \
                                      << action_settings_list;

    // Setup basic AIO verify test
    action_list.clear();
    action_signals_list.clear();
    action_settings_list.clear();

    // Add first action (Set AIO_04 to 1 using line edit)
    action_list << QList<QVariant>({"AIO_04", io_line_edit_pos, "1"});
    action_signal.clear();
    action_signal << QVariant(MAJOR_KEY_IO) \
                  << QVariant(MINOR_KEY_IO_AIO_WRITE) \
                  << GUI_GENERIC_HELPER::qList_to_byteArray({0x04, 0x00, 0x64});
    action_signals_list << action_signal;
    action_settings_list << QList<QVariant>({"AIO_04", "Output", 100, "1", false});

    // Load basic AIO verify test.
    // Performs the following actions:
    //   1) Set AIO_04 to 1 using line edit
    QTest::newRow("Basic AIO Verify") << action_list \
                                      << action_signals_list \
                                      << action_settings_list;

    // Setup Complex DIO/AIO Actions test data
    action_list.clear();
    action_signals_list.clear();
    action_settings_list.clear();

    // Add first action (Change DIO_00 to Output)
    action_list << QList<QVariant>({"DIO_00", io_combo_pos, "Output"});
    action_signal.clear();
    action_signal << QVariant(MAJOR_KEY_IO) \
                  << QVariant(MINOR_KEY_IO_DIO_SET) \
                  << GUI_GENERIC_HELPER::qList_to_byteArray({0x00, 0x00, 0x00, 0x01});
    action_signals_list << action_signal;
    action_settings_list << QList<QVariant>({"DIO_00", "Output", 0, "0", false});

    // Add second action (Change DIO_05 to Neg_Out)
    action_list << QList<QVariant>({"DIO_05", io_combo_pos, "Neg_Out"});
    action_signal.clear();
    action_signal << QVariant(MAJOR_KEY_IO) \
                  << QVariant(MINOR_KEY_IO_DIO_SET) \
                  << GUI_GENERIC_HELPER::qList_to_byteArray({0x05, 0x00, 0x05, 0x02});
    action_signals_list << action_signal;
    action_settings_list << QList<QVariant>({"DIO_05", "Neg_Out", 0, "0", false});

    // Add third action (Set DIO_00 to 1 using slider)
    action_list << QList<QVariant>({"DIO_00", io_slider_pos, 1});
    action_signal.clear();
    action_signal << QVariant(MAJOR_KEY_IO) \
                  << QVariant(MINOR_KEY_IO_DIO_WRITE) \
                  << GUI_GENERIC_HELPER::qList_to_byteArray({0x00, 0x00, 0x01});
    action_signals_list << action_signal;
    action_settings_list << QList<QVariant>({"DIO_00", "Output", 1, "1", false});


    // Add fourth action (Set DIO_05 to -1 using line edit)
    action_list << QList<QVariant>({"DIO_05", io_line_edit_pos, "-1"});
    action_signal.clear();
    action_signal << QVariant(MAJOR_KEY_IO) \
                  << QVariant(MINOR_KEY_IO_DIO_WRITE) \
                  << GUI_GENERIC_HELPER::qList_to_byteArray({0x05, 0x00, 0x04});
    action_signals_list << action_signal;
    action_settings_list << QList<QVariant>({"DIO_05", "Neg_Out", -1, "-1", false});

    // Add fifth action (Set DIO_05 to -2 using slider)
    action_list << QList<QVariant>({"DIO_05", io_slider_pos, -2});
    action_signal.clear();
    action_signal << QVariant(MAJOR_KEY_IO) \
                  << QVariant(MINOR_KEY_IO_DIO_WRITE) \
                  << GUI_GENERIC_HELPER::qList_to_byteArray({0x05, 0x00, 0x03});
    action_signals_list << action_signal;
    action_settings_list << QList<QVariant>({"DIO_05", "Neg_Out", -2, "-2", false});

    // Add sixth action (Set AIO_04 to 1.5 using slider)
    action_list << QList<QVariant>({"AIO_04", io_slider_pos, 150});
    action_signal.clear();
    action_signal << QVariant(MAJOR_KEY_IO) \
                  << QVariant(MINOR_KEY_IO_AIO_WRITE) \
                  << GUI_GENERIC_HELPER::qList_to_byteArray({0x04, 0x00, 0x96});
    action_signals_list << action_signal;
    action_settings_list << QList<QVariant>({"AIO_04", "Output", 150, "1.5", false});

    // Add seventh action (Set AIO_04 to 2.75 using lineEdit)
    action_list << QList<QVariant>({"AIO_04", io_line_edit_pos, "2.75"});
    action_signal.clear();
    action_signal << QVariant(MAJOR_KEY_IO) \
                  << QVariant(MINOR_KEY_IO_AIO_WRITE) \
                  << GUI_GENERIC_HELPER::qList_to_byteArray({0x04, 0x01, 0x13});
    action_signals_list << action_signal;
    action_settings_list << QList<QVariant>({"AIO_04", "Output", 275, "2.75", false});

    // Load Complex DIO/AIO Actions test data.
    // Performs the following actions:
    //   1) Change DIO_00 to Output
    //   2) Change DIO_05 to Neg_Out
    //   3) Set DIO_00 to 1 using slider
    //   4) Set DIO_05 to -1 using line edit
    //   5) Set DIO_05 to -2 using slider
    //   6) Set AIO_04 to 1.5 using slider
    //   7) Set AIO_04 to 2.75 using lineEdit
    QTest::newRow("Complex DIO/AIO Actions") << action_list \
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

void GUI_IO_CONTROL_TESTS::test_logging_errors()
{
    // Set new config & reset GUI
    QVERIFY(set_gui_config(generic_config_str));
    io_control_tester->reset_gui();
    QVERIFY(remove_tempFile());

    // Setup local variables
    QMessageBox *error_msg;

    // Try clicking start (should show missing filePath error)
    io_control_tester->set_log_file_save_path_test("");
    io_control_tester->log_start_clicked_test();

    // Check & close error
    error_msg = qobject_cast<QMessageBox*>(QApplication::activeWindow());
    QVERIFY(error_msg);
    QCOMPARE(error_msg->text(), QString("Error: Must provide log file!"));
    error_msg->close();

    // Set filePath to a bad path
    io_control_tester->set_log_file_save_path_test(":/BAD PATH");

    // Try clicking start (gives failed to open error)
    io_control_tester->log_start_clicked_test();

    // Check & close error
    error_msg = qobject_cast<QMessageBox*>(QApplication::activeWindow());
    QVERIFY(error_msg);
    QCOMPARE(error_msg->text(), QString("Error: Couldn't open log file!"));
    error_msg->close();

    // Set filePath to a good path
    io_control_tester->set_log_file_save_path_test(temp_filename);

    // Try clicking start (should be no error)
    io_control_tester->log_start_clicked_test();

    // Check no error, button text change, and file exists
    error_msg = qobject_cast<QMessageBox*>(QApplication::activeWindow());
    QVERIFY(!error_msg);
    QCOMPARE(io_control_tester->get_log_start_text_test(), QString("Running"));
    QVERIFY(QFile(temp_filename).exists());

    // Double click start (nothing should happen since button disabled)
    io_control_tester->log_start_clicked_test();

    // Check no error, button text same, and file exists
    error_msg = qobject_cast<QMessageBox*>(QApplication::activeWindow());
    QVERIFY(!error_msg);
    QCOMPARE(io_control_tester->get_log_start_text_test(), QString("Running"));
    QVERIFY(QFile(temp_filename).exists());

    // Force clicking of button (should show already recording error)
    io_control_tester->log_start_clicked_force_test();

    // Check & close error
    error_msg = qobject_cast<QMessageBox*>(QApplication::activeWindow());
    QVERIFY(error_msg);
    QCOMPARE(error_msg->text(), QString("Error: Already recording!"));
    error_msg->close();

    // Click stop log
    io_control_tester->log_stop_clicked_test();
    QCOMPARE(io_control_tester->get_log_start_text_test(), QString("Start Log"));
}

void GUI_IO_CONTROL_TESTS::test_logging()
{
    // Fetch data
    QFETCH(QList<QList<QList<QList<QVariant>>>>, pin_settings);
    QFETCH(QList<QList<QVariant>>, log_settings);
    QFETCH(QList<QByteArray>, expected_log_lines);

    // Verify input data
    int num_entries = pin_settings.length();
    QCOMPARE(log_settings.length(), num_entries);

    // Set new config & reset GUI
    QVERIFY(set_gui_config(generic_config_str));
    io_control_tester->reset_gui();
    QVERIFY(remove_tempFile());

    // Set log path
    io_control_tester->set_log_file_save_path_test(temp_filename);

    // Connect log wait signals
    QEventLoop log_waiter;
    connect(io_control_tester, SIGNAL(log_updated()),
            &log_waiter, SLOT(quit()),
            Qt::QueuedConnection);

    // Setup loop variables
    QList<QList<QList<QVariant>>> pin_entry;
    QList<QVariant> log_entry;
    bool log_running = false;
    int last_reset = 0;
    int num_pin_entries = 0;
    int num_expected_log_lines = 0;
    QList<int> start_line_pos;

    // Loop over all entries
    for (int i = 0; i < num_entries; i++)
    {
        // Get current values
        pin_entry = pin_settings.at(i);
        log_entry = log_settings.at(i);

        // Verify and set log settings
        QVERIFY(log_entry.length() == 3);
        io_control_tester->set_log_file_update_rate_test(log_entry.at(0).toFloat());
        io_control_tester->set_log_append_checked_test(log_entry.at(1).toBool());

        // If not appending, reset values
        if (!log_entry.at(1).toBool())
        {
            // Reset values
            last_reset = i;
            start_line_pos.clear();
            num_expected_log_lines = 0;
        }

        // Loop until button entries empty
        num_pin_entries = pin_entry.length();
        for (int j = 0; j < num_pin_entries; j++)
        {
            // Set pin group
            foreach (QList<QVariant> button, pin_entry.at(j))
            {
                QVERIFY(button.length() == 3);
                io_control_tester->set_pin_test(button.at(0).toString(), button.at(1).toString(),
                                                button.at(2).toInt());
            }

            // Start log if not running
            if (!log_running)
            {
                // Set values
                start_line_pos << num_expected_log_lines;
                num_expected_log_lines += 1;

                // Click start
                io_control_tester->log_start_clicked_test();
                log_running = true;

                // Verify start log button text
                QCOMPARE(io_control_tester->get_log_start_text_test(), QString("Running"));
            }

            // Wait for log updated
            log_waiter.exec();
            num_expected_log_lines += 2;
        }

        // Check if clicking stop after test
        if (log_entry.at(2).toBool() && log_running)
        {
            // Click the stop button
            io_control_tester->log_stop_clicked_test();
            log_running = false;

            // Verify start log button text
            QCOMPARE(io_control_tester->get_log_start_text_test(), QString("Start Log"));
        }
    }

    // If log running, click stop log
    if (log_running) io_control_tester->log_start_clicked_test();

    // Read log file
    QList<QByteArray> log_data = GUI_GENERIC_HELPER::loadFile(temp_filename).split('\n');

    // Setup log check loop data
    QByteArray expected_start_line;
    QByteArray curr_data_line;
    int next_start_line = 0, curr_start_line = 0;
    QCOMPARE(log_data.length(), num_expected_log_lines+1);

    // Handle empty log file case
    if (num_expected_log_lines == 0)
    {
        QVERIFY(log_data.length() == 1);
        QVERIFY(log_data.at(0).isEmpty());
        return;
    }

    // Check log file
    for (int i = 0; i < num_expected_log_lines; i++)
    {
        // Get next log line
        curr_data_line = log_data.at(i);

        // Check if should be a start line
        if (i == next_start_line)
        {
            // Build expected start line (minus time)
            expected_start_line.clear();
            expected_start_line += "with update rate ";
            expected_start_line += QString::number(log_settings.at(last_reset+curr_start_line)
                                                   .at(0).toFloat());
            expected_start_line += " seconds";

            // Compare start line
            QVERIFY(curr_data_line.startsWith("Started: "));
            QCOMPARE(curr_data_line.split(' ').mid(7),
                     expected_start_line.split(' ').mid(0));

            // Set next expected start
            curr_start_line += 1;
            if (curr_start_line < start_line_pos.length())
            {
                next_start_line = start_line_pos.at(curr_start_line);
            }
        } else
        {
            // Compare data log line
            QCOMPARE(curr_data_line, expected_log_lines.at(i - curr_start_line));
        }
    }
}

void GUI_IO_CONTROL_TESTS::test_logging_data()
{
    // Setup data columns
    // Gets parsed after each log timeout
    // Each button item is set as follows:
    //   0) pin_str
    //   1) combo value
    //   2) slider value
    QTest::addColumn<QList<QList<QList<QList<QVariant>>>>>("pin_settings");

    // Parsed before each start click
    // Each log item is set as follows:
    //   0) log update_rate (s - float)
    //   1) append if exists? (bool)
    //   2) reset log (click stop after run - bool)
    QTest::addColumn<QList<QList<QVariant>>>("log_settings");

    QTest::addColumn<QList<QByteArray>>("expected_log_lines");

    // Helper variables
    QList<QList<QList<QList<QVariant>>>> pin_settings_list;
    QList<QList<QList<QVariant>>> pin_settings_entry;
    QList<QList<QVariant>> button_group;
    QList<QList<QVariant>> log_settings_list;
    QList<QByteArray> log_data_lines;

    // Setup verify test data (does nothing)
    pin_settings_list.clear();
    log_settings_list.clear();
    log_data_lines.clear();

    // Load verify test data (does nothing)
    QTest::newRow("Verify") << pin_settings_list \
                            << log_settings_list \
                            << log_data_lines;

    // Setup basic test data
    pin_settings_list.clear();
    pin_settings_entry.clear();
    pin_settings_entry << QList<QList<QVariant>>();
    pin_settings_list << pin_settings_entry;

    log_settings_list.clear();
    button_group.clear();
    log_settings_list << QList<QVariant>({0.05, true, false});

    log_data_lines.clear();
    log_data_lines << QString("1,0,0,0,0,0,0").toLatin1();
    log_data_lines << QString("6,0,0,0,0,0").toLatin1();

    // Load basic test data
    // Reads one set of values before calling stop
    QTest::newRow("Basic") << pin_settings_list \
                           << log_settings_list \
                           << log_data_lines;

    // Setup Basic Append test data
    pin_settings_list.clear();
    pin_settings_entry.clear();
    pin_settings_entry << QList<QList<QVariant>>();
    pin_settings_entry << QList<QList<QVariant>>();
    pin_settings_list << pin_settings_entry;
    pin_settings_list << pin_settings_entry;

    log_settings_list.clear();
    button_group.clear();
    log_settings_list << QList<QVariant>({0.05, true, true});
    log_settings_list << QList<QVariant>({0.05, true, false});

    log_data_lines.clear();
    log_data_lines << QString("1,0,0,0,0,0,0").toLatin1();
    log_data_lines << QString("6,0,0,0,0,0").toLatin1();
    log_data_lines << QString("1,0,0,0,0,0,0").toLatin1();
    log_data_lines << QString("6,0,0,0,0,0").toLatin1();
    log_data_lines << QString("1,0,0,0,0,0,0").toLatin1();
    log_data_lines << QString("6,0,0,0,0,0").toLatin1();
    log_data_lines << QString("1,0,0,0,0,0,0").toLatin1();
    log_data_lines << QString("6,0,0,0,0,0").toLatin1();

    // Load Basic Append test data
    //   1) Saves two rows then clicks stop
    //   2) Saves two more rows (appends to existing)
    QTest::newRow("Basic Append") << pin_settings_list \
                                  << log_settings_list \
                                  << log_data_lines;

    // Setup Basic Append test data
    pin_settings_list.clear();
    pin_settings_entry.clear();
    pin_settings_entry << QList<QList<QVariant>>();
    pin_settings_entry << QList<QList<QVariant>>();
    pin_settings_list << pin_settings_entry;
    pin_settings_list << pin_settings_entry;

    log_settings_list.clear();
    button_group.clear();
    log_settings_list << QList<QVariant>({0.05, true, true});
    log_settings_list << QList<QVariant>({0.05, false, false});

    log_data_lines.clear();
    log_data_lines << QString("1,0,0,0,0,0,0").toLatin1();
    log_data_lines << QString("6,0,0,0,0,0").toLatin1();
    log_data_lines << QString("1,0,0,0,0,0,0").toLatin1();
    log_data_lines << QString("6,0,0,0,0,0").toLatin1();

    // Load Basic No Append test data
    //   1) Saves two rows then clicks stop
    //   2) Saves two more rows (erases existing)
    QTest::newRow("Basic No Append") << pin_settings_list \
                                     << log_settings_list \
                                     << log_data_lines;
}

void GUI_IO_CONTROL_TESTS::test_recv()
{
    // Fetch data
    QFETCH(QList<QList<QList<QVariant>>>, pin_settings);
    QFETCH(QList<QByteArray>, recv_data);
    QFETCH(QList<QList<QList<QVariant>>>, expected_pin_settings);
    QFETCH(QList<QList<QList<QVariant>>>, send_expected_signals);

    // Verify input data
    int num_entries = pin_settings.length();
    QCOMPARE(recv_data.length(), num_entries);
    QCOMPARE(expected_pin_settings.length(), num_entries);
    QCOMPARE(send_expected_signals.length(), num_entries);

    // Set new config & reset GUI
    QVERIFY(set_gui_config(generic_config_str));
    io_control_tester->reset_gui();

    // Setup spy to catch update signal
    QList<QVariant> spy_args;
    QSignalSpy transmit_chunk_spy(io_control_tester, io_control_tester->transmit_chunk);
    QVERIFY(transmit_chunk_spy.isValid());

    // Setup loop variables
    QList<QList<QVariant>> curr_signal;
    QByteArray curr_recv_data;

    // Loop through all buttons, calls, and verify
    for (int i = 0; i < num_entries; i++)
    {
        // Set current buttons
        foreach (QList<QVariant> button, pin_settings.at(i))
        {
            QVERIFY(button.length() == 3);
            io_control_tester->set_pin_test(button.at(0).toString(), button.at(1).toString(),
                                            button.at(2).toInt());
        }

        // Setup current recv
        curr_recv_data = recv_data.at(i);

        // Check if read write (need to have read request)
        switch (curr_recv_data.at(s1_minor_key_loc))
        {
            case MINOR_KEY_IO_AIO_READ:
            case MINOR_KEY_IO_DIO_READ:
            {
                if (curr_recv_data.length() != (s1_end_loc + s2_io_read_end))
                {
                    // Send read request
                    io_control_tester->request_read_pin_test(
                                curr_recv_data.at(s1_minor_key_loc),
                                curr_recv_data.at(s1_end_loc + s2_io_pin_num_loc));
                }
                break;
            }
            case MINOR_KEY_IO_AIO_READ_ALL:
            case MINOR_KEY_IO_DIO_READ_ALL:
            {
                if (curr_recv_data.length() != s1_end_loc)
                {
                    // Send read request
                    io_control_tester->request_read_all_test(curr_recv_data.at(s1_minor_key_loc));
                }
                break;
            }
        }

        // Send data
        emit io_control_tester->readyRead(curr_recv_data);
        qApp->processEvents();

        // Verify same number of signals
        curr_signal = send_expected_signals.at(i);
        QCOMPARE(transmit_chunk_spy.count(), curr_signal.length());

        // Check each signal
        foreach (QList<QVariant> action_signal, curr_signal)
        {
            // Get next spy signal
            spy_args = transmit_chunk_spy.takeFirst();

            // Verify signal values
            QVERIFY(action_signal.length() == 3);
            QCOMPARE(spy_args.at(0).toUInt(), action_signal.at(0).toUInt());
            QCOMPARE(spy_args.at(1).toUInt(), action_signal.at(1).toUInt());
            QCOMPARE(spy_args.at(2).toByteArray(), action_signal.at(2).toByteArray());

        }

        // Check requested buttons
        foreach (QList<QVariant> button_group, expected_pin_settings.at(i))
        {
            QVERIFY(button_group.length() == 5);
            QVERIFY(io_control_tester->check_pin_test(button_group.at(0).toString(),
                                                      button_group.at(1).toString(),
                                                      button_group.at(2).toInt(),
                                                      button_group.at(3).toString(),
                                                      button_group.at(4).toBool()));
        }
    }
}

void GUI_IO_CONTROL_TESTS::test_recv_data()
{
    // Setup data columns
    // Parsed before each recv call
    // Each button item is set as follows:
    //   0) pin_str
    //   1) combo value
    //   2) slider value
    QTest::addColumn<QList<QList<QList<QVariant>>>>("pin_settings");

    QTest::addColumn<QList<QByteArray>>("recv_data");

    // Parsed after each recv call
    // Each pin setting item is composed as follows
    //   0) pin_str
    //   1) Combo Value
    //   2) Slider Value
    //   3) Line Edit Value
    //   4) isDisabled
    QTest::addColumn<QList<QList<QList<QVariant>>>>("expected_pin_settings");

    // Send expected signals column (emit transmit_chunk())
    // Each QList<QVariant> must be arranged as follows:
    //  0) Major Key (uint8_t)
    //  1) Minor Key (uint8_t)
    //  2) Data Packet (QByteArray)
    QTest::addColumn<QList<QList<QList<QVariant>>>>("send_expected_signals");

    // Helper variables
    QList<QList<QList<QVariant>>> button_pre_settings_list;
    QList<QByteArray> recv_data;
    QByteArray data;
    QList<QList<QList<QVariant>>> button_post_settings_list;
    QList<QList<QList<QVariant>>> expected_signals_list;
    QList<QList<QVariant>> button_group;

    // Setup verify test data (does nothing)
    button_pre_settings_list.clear();
    recv_data.clear();
    button_post_settings_list.clear();
    expected_signals_list.clear();

    // Load verify test data (does nothing)
    QTest::newRow("Verify") << button_pre_settings_list \
                            << recv_data \
                            << button_post_settings_list \
                            << expected_signals_list;

    /** Start of DIO READ ALL Tests **/

    // Setup Basic DIO Read All test data
    button_pre_settings_list.clear();
    recv_data.clear();
    button_post_settings_list.clear();
    expected_signals_list.clear();

    // Set first action
    button_group.clear();
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_DIO_READ_ALL,
                                                        0x00, 0x01, 0x00, 0x01, 0x00, 0x01,
                                                        0x00, 0x01, 0x00, 0x01}));
    recv_data << data;

    button_group.clear();
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_DIO_READ_ALL,
                                     QByteArray()
                                    });
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"DIO_00", "Input", 1, "1", true});
    button_group << QList<QVariant>({"DIO_01", "Input", 1, "1", true});
    button_group << QList<QVariant>({"DIO_03", "Input", 1, "1", true});
    button_group << QList<QVariant>({"DIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"DIO_05", "Input", 1, "1", true});
    button_post_settings_list << button_group;

    // Load Basic DIO Read All test data
    // Sets all outputs to 1
    QTest::newRow("Basic DIO Read All") \
            << button_pre_settings_list \
            << recv_data \
            << button_post_settings_list \
            << expected_signals_list;

    // Setup Change DIO Read All V1 test data
    button_pre_settings_list.clear();
    recv_data.clear();
    button_post_settings_list.clear();
    expected_signals_list.clear();

    // Set first action
    button_group.clear();
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_DIO_READ_ALL,
                                                        0x00, 0x01, 0x00, 0x01, 0x00, 0x01,
                                                        0x00, 0x01, 0x00, 0x01}));
    recv_data << data;

    button_group.clear();
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_DIO_READ_ALL,
                                     QByteArray()
                                    });
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"DIO_00", "Input", 1, "1", true});
    button_group << QList<QVariant>({"DIO_01", "Input", 1, "1", true});
    button_group << QList<QVariant>({"DIO_03", "Input", 1, "1", true});
    button_group << QList<QVariant>({"DIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"DIO_05", "Input", 1, "1", true});
    button_post_settings_list << button_group;

    // Set second action
    button_group.clear();
    button_group << QList<QVariant>({"DIO_00", "Output", 0});
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_DIO_READ_ALL,
                                                        0x00, 0x01, 0x00, 0x01, 0x00, 0x01,
                                                        0x00, 0x01, 0x00, 0x01}));
    recv_data << data;

    button_group.clear();
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_DIO_SET,
                                     GUI_GENERIC_HELPER::qList_to_byteArray(
                                        {0x00, 0x00, 0x00, 0x01})
                                    });
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_DIO_READ_ALL,
                                     QByteArray()
                                    });
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"DIO_00", "Output", 0, "0", false});
    button_group << QList<QVariant>({"DIO_01", "Input", 1, "1", true});
    button_group << QList<QVariant>({"DIO_03", "Input", 1, "1", true});
    button_group << QList<QVariant>({"DIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"DIO_05", "Input", 1, "1", true});
    button_post_settings_list << button_group;

    // Load Change DIO Read All V1 test data
    // Two actions: Set DIO_00 while in output, then change to input
    QTest::newRow("Change DIO Read All V1") \
            << button_pre_settings_list \
            << recv_data \
            << button_post_settings_list \
            << expected_signals_list;


    // Setup DIO Read All Bad Length test data
    button_pre_settings_list.clear();
    recv_data.clear();
    button_post_settings_list.clear();
    expected_signals_list.clear();

    // Set first action
    button_group.clear();
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_DIO_READ_ALL,
                                                        0x00, 0x01, 0x00, 0x01, 0x00, 0x01}));
    recv_data << data;

    button_group.clear();
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_DIO_READ_ALL,
                                     QByteArray()
                                    });
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"DIO_00", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"DIO_05", "Input", 0, "0", true});
    button_post_settings_list << button_group;

    // Load DIO Read All Bad Length test data
    QTest::newRow("DIO Read All Bad Length") \
            << button_pre_settings_list \
            << recv_data \
            << button_post_settings_list \
            << expected_signals_list;

    // Setup DIO Read All Request test data
    button_pre_settings_list.clear();
    recv_data.clear();
    button_post_settings_list.clear();
    expected_signals_list.clear();

    // Set first action
    button_group.clear();
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_DIO_READ_ALL}));
    recv_data << data;

    button_group.clear();
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_DIO_READ_ALL,
                                     GUI_GENERIC_HELPER::qList_to_byteArray(
                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00})
                                    });
    button_group << QList<QVariant>({MAJOR_KEY_DEV_READY,
                                     0,
                                     QByteArray()
                                    });
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"DIO_00", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"DIO_05", "Input", 0, "0", true});
    button_post_settings_list << button_group;

    // Set second action
    button_group.clear();
    button_group << QList<QVariant>({"DIO_01", "Output", 1});
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_DIO_READ_ALL}));
    recv_data << data;

    button_group.clear();
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_DIO_SET,
                                     GUI_GENERIC_HELPER::qList_to_byteArray(
                                        {0x01, 0x00, 0x00, 0x01})
                                    });
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_DIO_WRITE,
                                     GUI_GENERIC_HELPER::qList_to_byteArray(
                                        {0x01, 0x00, 0x01})
                                    });
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_DIO_READ_ALL,
                                     GUI_GENERIC_HELPER::qList_to_byteArray(
                                        {0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00})
                                    });
    button_group << QList<QVariant>({MAJOR_KEY_DEV_READY,
                                     0,
                                     QByteArray()
                                    });
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"DIO_00", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_01", "Output", 1, "1", false});
    button_group << QList<QVariant>({"DIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"DIO_05", "Input", 0, "0", true});
    button_post_settings_list << button_group;

    // Load DIO Read All Request test data
    // Performs the following actions:
    //   1) Reads all default values (0 for all pins)
    //   2) Sets DIO_01 to Output with Value 1, then reads all values again
    QTest::newRow("DIO Read All Request") \
            << button_pre_settings_list \
            << recv_data \
            << button_post_settings_list \
            << expected_signals_list;

    /** Start of DIO READ SINGLE Tests **/

    // Setup Basic DIO Read Single test data
    button_pre_settings_list.clear();
    recv_data.clear();
    button_post_settings_list.clear();
    expected_signals_list.clear();

    // Set first action
    button_group.clear();
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_DIO_READ,
                                                        0x00, 0x00, 0x01}));
    recv_data << data;

    button_group.clear();
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_DIO_READ,
                                     GUI_GENERIC_HELPER::qList_to_byteArray(
                                        {0x00})
                                    });
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"DIO_00", "Input", 1, "1", true});
    button_group << QList<QVariant>({"DIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"DIO_05", "Input", 0, "0", true});
    button_post_settings_list << button_group;

    // Set second action
    button_group.clear();
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_DIO_READ,
                                                        0x05, 0x00, 0x01}));
    recv_data << data;

    button_group.clear();
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_DIO_READ,
                                     GUI_GENERIC_HELPER::qList_to_byteArray(
                                        {0x05})
                                    });
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"DIO_00", "Input", 1, "1", true});
    button_group << QList<QVariant>({"DIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"DIO_05", "Input", 1, "1", true});
    button_post_settings_list << button_group;

    // Load Basic DIO Read Single test data
    // Performs the following actions:
    //   1) Sets DIO_00 to 1 (checks that its only set)
    //   2) Sets DIO_05 to 1 (checks that it and DIO_00 set)
    QTest::newRow("Basic DIO Read Single") \
            << button_pre_settings_list \
            << recv_data \
            << button_post_settings_list \
            << expected_signals_list;

    // Setup DIO Read Single Request test data
    button_pre_settings_list.clear();
    recv_data.clear();
    button_post_settings_list.clear();
    expected_signals_list.clear();

    // Set first action
    button_group.clear();
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_DIO_READ,
                                                        0x05}));
    recv_data << data;

    button_group.clear();
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_DIO_READ,
                                     GUI_GENERIC_HELPER::qList_to_byteArray(
                                        {0x05, 0x00, 0x00})
                                    });
    button_group << QList<QVariant>({MAJOR_KEY_DEV_READY,
                                     0,
                                     QByteArray()
                                    });
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"DIO_00", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"DIO_05", "Input", 0, "0", true});
    button_post_settings_list << button_group;

    // Set second action
    button_group.clear();
    button_group << QList<QVariant>({"DIO_05", "Input", 1});
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_DIO_READ,
                                                        0x05}));
    recv_data << data;

    button_group.clear();
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_DIO_WRITE,
                                     GUI_GENERIC_HELPER::qList_to_byteArray(
                                        {0x05, 0x00, 0x01})
                                    });
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_DIO_READ,
                                     GUI_GENERIC_HELPER::qList_to_byteArray(
                                        {0x05, 0x00, 0x01})
                                    });
    button_group << QList<QVariant>({MAJOR_KEY_DEV_READY,
                                     0,
                                     QByteArray()
                                    });
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"DIO_00", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"DIO_05", "Input", 1, "1", true});
    button_post_settings_list << button_group;

    // Load DIO Read Single Request test data
    // Performs the following actions:
    //   1) Reads DIO_05 default value (0)
    //   2) Sets DIO_05 to 1, then reads value again
    QTest::newRow("DIO Read Single Request") \
            << button_pre_settings_list \
            << recv_data \
            << button_post_settings_list \
            << expected_signals_list;

    /** Start of DIO SET Tests **/

    // Setup Basic DIO Set V1 test data
    button_pre_settings_list.clear();
    recv_data.clear();
    button_post_settings_list.clear();
    expected_signals_list.clear();

    // Set first action
    button_group.clear();
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_DIO_SET,
                                                        0x00, 0x00, 0x00, 0x01}));
    recv_data << data;

    button_group.clear();
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"DIO_00", "Output", 0, "0", false});
    button_group << QList<QVariant>({"DIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"DIO_05", "Input", 0, "0", true});
    button_post_settings_list << button_group;

    // Set second action
    button_group.clear();
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_DIO_SET,
                                                        0x00, 0x00, 0x01, 0x00}));
    recv_data << data;

    button_group.clear();
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"DIO_00", "Input", 1, "1", true});
    button_group << QList<QVariant>({"DIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"DIO_05", "Input", 0, "0", true});
    button_post_settings_list << button_group;

    // Load Basic DIO Set V1 test data
    // Performs the following actions:
    //   1) Sets DIO_00 to Output with value 0
    //   2) Sets DIO_00 to Input with value 1
    QTest::newRow("Basic DIO Set V1") \
            << button_pre_settings_list \
            << recv_data \
            << button_post_settings_list \
            << expected_signals_list;

    // Setup DIO Set Bad Pin V1 test data
    button_pre_settings_list.clear();
    recv_data.clear();
    button_post_settings_list.clear();
    expected_signals_list.clear();

    // Set first action
    button_group.clear();
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_DIO_SET,
                                                        0x7F, 0x00, 0x00, 0x01}));
    recv_data << data;

    button_group.clear();
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"DIO_00", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"DIO_05", "Input", 0, "0", true});
    button_post_settings_list << button_group;

    // Load DIO Set Bad Pin V1 test data
    // Performs the following actions:
    //   1) Trys setting DIO_127 to Output with value 0
    QTest::newRow("DIO Set Bad Pin V1") \
            << button_pre_settings_list \
            << recv_data \
            << button_post_settings_list \
            << expected_signals_list;

    /** Start of DIO WRITE Tests **/

    // Setup Basic DIO Write V1 test data
    button_pre_settings_list.clear();
    recv_data.clear();
    button_post_settings_list.clear();
    expected_signals_list.clear();

    // Set first action
    button_group.clear();
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_DIO_WRITE,
                                                        0x00, 0x00, 0x01}));
    recv_data << data;

    button_group.clear();
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"DIO_00", "Input", 1, "1", true});
    button_group << QList<QVariant>({"DIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"DIO_05", "Input", 0, "0", true});
    button_post_settings_list << button_group;

    // Set second action
    button_group.clear();
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_DIO_WRITE,
                                                        0x00, 0x00, 0x00}));
    recv_data << data;

    button_group.clear();
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"DIO_00", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"DIO_05", "Input", 0, "0", true});
    button_post_settings_list << button_group;

    // Set third action
    button_group.clear();
    button_group << QList<QVariant>({"DIO_00", "Output", 0});
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_DIO_WRITE,
                                                        0x00, 0x00, 0x01}));
    recv_data << data;

    button_group.clear();
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_DIO_SET,
                                     GUI_GENERIC_HELPER::qList_to_byteArray(
                                        {0x00, 0x00, 0x00, 0x01})
                                    });
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"DIO_00", "Output", 1, "1", false});
    button_group << QList<QVariant>({"DIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"DIO_05", "Input", 0, "0", true});
    button_post_settings_list << button_group;

    // Load Basic DIO Write V1 test data
    // Performs the following actions:
    //   1) Sets DIO_00 to 1
    //   2) Sets DIO_00 to 0
    //   3) Sets DIO_00 to 1 (after chaning to Output)
    QTest::newRow("Basic DIO Write V1") \
            << button_pre_settings_list \
            << recv_data \
            << button_post_settings_list \
            << expected_signals_list;

    // Setup DIO Write Bad Pin V1 test data
    button_pre_settings_list.clear();
    recv_data.clear();
    button_post_settings_list.clear();
    expected_signals_list.clear();

    // Set first action
    button_group.clear();
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_DIO_WRITE,
                                                        0x7F, 0x00, 0x01}));
    recv_data << data;

    button_group.clear();
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"DIO_00", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"DIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"DIO_05", "Input", 0, "0", true});
    button_post_settings_list << button_group;

    // Load DIO Write Bad Pin V1 test data
    // Performs the following actions:
    //   1) Trys writing DIO_127 with value 1
    QTest::newRow("DIO Write Bad Pin V1") \
            << button_pre_settings_list \
            << recv_data \
            << button_post_settings_list \
            << expected_signals_list;

    /** Start of AIO READ ALL Tests **/

    // Setup Basic AIO Read All test data
    button_pre_settings_list.clear();
    recv_data.clear();
    button_post_settings_list.clear();
    expected_signals_list.clear();

    // Set first action
    button_group.clear();
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_AIO_READ_ALL,
                                                        0x00, 0x64, 0x00, 0x64, 0x00, 0x64,
                                                        0x00, 0x64, 0x00, 0x64, 0x00, 0x64}));
    recv_data << data;

    button_group.clear();
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_AIO_READ_ALL,
                                     QByteArray()
                                    });
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"AIO_00", "Input", 100, "1", true});
    button_group << QList<QVariant>({"AIO_01", "Input", 100, "1", true});
    button_group << QList<QVariant>({"AIO_02", "Input", 100, "1", true});
    button_group << QList<QVariant>({"AIO_03", "Input", 100, "1", true});
    button_group << QList<QVariant>({"AIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"AIO_05", "Input", 100, "1", true});
    button_post_settings_list << button_group;

    // Load Basic AIO Read All test data
    // Sets all outputs to 1
    QTest::newRow("Basic AIO Read All") \
            << button_pre_settings_list \
            << recv_data \
            << button_post_settings_list \
            << expected_signals_list;

    // Setup Change AIO Read All V1 test data
    button_pre_settings_list.clear();
    recv_data.clear();
    button_post_settings_list.clear();
    expected_signals_list.clear();

    // Set first action
    button_group.clear();
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_AIO_READ_ALL,
                                                        0x00, 0x64, 0x00, 0x64, 0x00, 0x64,
                                                        0x00, 0x64, 0x00, 0x64, 0x00, 0x64}));
    recv_data << data;

    button_group.clear();
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_AIO_READ_ALL,
                                     QByteArray()
                                    });
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"AIO_00", "Input", 100, "1", true});
    button_group << QList<QVariant>({"AIO_01", "Input", 100, "1", true});
    button_group << QList<QVariant>({"AIO_02", "Input", 100, "1", true});
    button_group << QList<QVariant>({"AIO_03", "Input", 100, "1", true});
    button_group << QList<QVariant>({"AIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"AIO_05", "Input", 100, "1", true});
    button_post_settings_list << button_group;

    // Set second action
    button_group.clear();
    button_group << QList<QVariant>({"AIO_05", "Output", 0});
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_AIO_READ_ALL,
                                                        0x00, 0x64, 0x00, 0x64, 0x00, 0x64,
                                                        0x00, 0x64, 0x00, 0x64, 0x00, 0x64}));
    recv_data << data;

    button_group.clear();
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_AIO_SET,
                                     GUI_GENERIC_HELPER::qList_to_byteArray(
                                        {0x05, 0x00, 0x00, 0x01})
                                    });
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_AIO_READ_ALL,
                                     QByteArray()
                                    });
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"AIO_00", "Input", 100, "1", true});
    button_group << QList<QVariant>({"AIO_01", "Input", 100, "1", true});
    button_group << QList<QVariant>({"AIO_02", "Input", 100, "1", true});
    button_group << QList<QVariant>({"AIO_03", "Input", 100, "1", true});
    button_group << QList<QVariant>({"AIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"AIO_05", "Output", 0, "0", false});
    button_post_settings_list << button_group;

    // Load Change AIO Read All V1 test data
    // Two actions:
    //   1) Set AIO_05 value with read_all while input
    //   2) Change AIO_05 to output and verify no change for read all
    QTest::newRow("Change AIO Read All V1") \
            << button_pre_settings_list \
            << recv_data \
            << button_post_settings_list \
            << expected_signals_list;

    // Setup AIO Read All Request test data
    button_pre_settings_list.clear();
    recv_data.clear();
    button_post_settings_list.clear();
    expected_signals_list.clear();

    // Set first action
    button_group.clear();
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_AIO_READ_ALL}));
    recv_data << data;

    button_group.clear();
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_AIO_READ_ALL,
                                     GUI_GENERIC_HELPER::qList_to_byteArray(
                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00})
                                    });
    button_group << QList<QVariant>({MAJOR_KEY_DEV_READY,
                                     0,
                                     QByteArray()
                                    });
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"AIO_00", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_02", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"AIO_05", "Input", 0, "0", true});
    button_post_settings_list << button_group;

    // Set second action
    button_group.clear();
    button_group << QList<QVariant>({"AIO_02", "Input", 500});
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_AIO_READ_ALL}));
    recv_data << data;

    button_group.clear();
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_AIO_WRITE,
                                     GUI_GENERIC_HELPER::qList_to_byteArray(
                                        {0x02, 0x01, 0xF4})
                                    });
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_AIO_READ_ALL,
                                     GUI_GENERIC_HELPER::qList_to_byteArray(
                                        {0x00, 0x00, 0x00, 0x00, 0x01, 0xF4,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00})
                                    });
    button_group << QList<QVariant>({MAJOR_KEY_DEV_READY,
                                     0,
                                     QByteArray()
                                    });
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"AIO_00", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_02", "Input", 500, "5", true});
    button_group << QList<QVariant>({"AIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"AIO_05", "Input", 0, "0", true});
    button_post_settings_list << button_group;

    // Load AIO Read All Request test data
    // Performs the following actions:
    //   1) Reads all default values (0 for all pins)
    //   2) Sets AIO_02 to 500, then reads all values again
    QTest::newRow("AIO Read All Request") \
            << button_pre_settings_list \
            << recv_data \
            << button_post_settings_list \
            << expected_signals_list;

    /** Start of AIO READ SINGLE Tests **/

    // Setup Basic AIO Read Single test data
    button_pre_settings_list.clear();
    recv_data.clear();
    button_post_settings_list.clear();
    expected_signals_list.clear();

    // Set first action
    button_group.clear();
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_AIO_READ,
                                                        0x00, 0x00, 0xDC}));
    recv_data << data;

    button_group.clear();
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_AIO_READ,
                                     GUI_GENERIC_HELPER::qList_to_byteArray(
                                        {0x00})
                                    });
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"AIO_00", "Input", 220, "2.2", true});
    button_group << QList<QVariant>({"AIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_02", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"AIO_05", "Input", 0, "0", true});
    button_post_settings_list << button_group;

    // Set second action
    button_group.clear();
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_AIO_READ,
                                                        0x03, 0x01, 0x4A}));
    recv_data << data;

    button_group.clear();
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_AIO_READ,
                                     GUI_GENERIC_HELPER::qList_to_byteArray(
                                        {0x03})
                                    });
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"AIO_00", "Input", 220, "2.2", true});
    button_group << QList<QVariant>({"AIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_02", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_03", "Input", 330, "3.3", true});
    button_group << QList<QVariant>({"AIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"AIO_05", "Input", 0, "0", true});
    button_post_settings_list << button_group;

    // Load Basic AIO Read Single test data
    // Performs the following actions:
    //   1) Sets AIO_00 to 2.2 (checks that its only set)
    //   2) Sets AIO_03 to 3.3 (checks that it and AIO_00 set)
    QTest::newRow("Basic AIO Read Single") \
            << button_pre_settings_list \
            << recv_data \
            << button_post_settings_list \
            << expected_signals_list;

    // Setup AIO Read Single Request test data
    button_pre_settings_list.clear();
    recv_data.clear();
    button_post_settings_list.clear();
    expected_signals_list.clear();

    // Set first action
    button_group.clear();
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_AIO_READ,
                                                        0x04}));
    recv_data << data;

    button_group.clear();
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_AIO_READ,
                                     GUI_GENERIC_HELPER::qList_to_byteArray(
                                        {0x04, 0x00, 0x00})
                                    });
    button_group << QList<QVariant>({MAJOR_KEY_DEV_READY,
                                     0,
                                     QByteArray()
                                    });
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"AIO_00", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_02", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"AIO_05", "Input", 0, "0", true});
    button_post_settings_list << button_group;

    // Set second action
    button_group.clear();
    button_group << QList<QVariant>({"AIO_04", "Input", 210});
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_AIO_READ,
                                                        0x04}));
    recv_data << data;

    button_group.clear();
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_AIO_WRITE,
                                     GUI_GENERIC_HELPER::qList_to_byteArray(
                                        {0x04, 0x00, 0xD2})
                                    });
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_AIO_READ,
                                     GUI_GENERIC_HELPER::qList_to_byteArray(
                                        {0x04, 0x00, 0xD2})
                                    });
    button_group << QList<QVariant>({MAJOR_KEY_DEV_READY,
                                     0,
                                     QByteArray()
                                    });
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"AIO_00", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_02", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_04", "Output", 210, "2.1", false});
    button_group << QList<QVariant>({"AIO_05", "Input", 0, "0", true});
    button_post_settings_list << button_group;

    // Load AIO Read Single Request test data
    // Performs the following actions:
    //   1) Reads AIO_04 default value (0)
    //   2) Sets AIO_04 to 2.1, then reads value again
    QTest::newRow("AIO Read Single Request") \
            << button_pre_settings_list \
            << recv_data \
            << button_post_settings_list \
            << expected_signals_list;

    /** Start of AIO SET Tests **/

    // Setup Basic AIO Set V1 test data
    button_pre_settings_list.clear();
    recv_data.clear();
    button_post_settings_list.clear();
    expected_signals_list.clear();

    // Set first action
    button_group.clear();
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_AIO_SET,
                                                        0x05, 0x00, 0x00, 0x01}));
    recv_data << data;

    button_group.clear();
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"AIO_00", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_02", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"AIO_05", "Output", 0, "0", false});
    button_post_settings_list << button_group;

    // Set second action
    button_group.clear();
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_AIO_SET,
                                                        0x05, 0x00, 0x64, 0x00}));
    recv_data << data;

    button_group.clear();
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"AIO_00", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_02", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"AIO_05", "Input", 100, "1", true});
    button_post_settings_list << button_group;

    // Load Basic AIO Set V1 test data
    // Performs the following actions:
    //   1) Sets AIO_05 to Output with value 0
    //   2) Sets AIO_05 to Input with value 1
    QTest::newRow("Basic AIO Set V1") \
            << button_pre_settings_list \
            << recv_data \
            << button_post_settings_list \
            << expected_signals_list;

    // Setup AIO Set Bad Pin V1 test data
    button_pre_settings_list.clear();
    recv_data.clear();
    button_post_settings_list.clear();
    expected_signals_list.clear();

    // Set first action
    button_group.clear();
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_AIO_SET,
                                                        0x7F, 0x00, 0x00, 0x01}));
    recv_data << data;

    button_group.clear();
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"AIO_00", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_02", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"AIO_05", "Input", 0, "0", true});
    button_post_settings_list << button_group;

    // Load AIO Set Bad Pin V1 test data
    // Performs the following actions:
    //   1) Trys setting AIO_127 to Output with value 0
    QTest::newRow("AIO Set Bad Pin V1") \
            << button_pre_settings_list \
            << recv_data \
            << button_post_settings_list \
            << expected_signals_list;

    /** Start of AIO WRITE Tests **/

    // Setup Basic AIO Write V1 test data
    button_pre_settings_list.clear();
    recv_data.clear();
    button_post_settings_list.clear();
    expected_signals_list.clear();

    // Set first action
    button_group.clear();
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_AIO_WRITE,
                                                        0x05, 0x00, 0x64}));
    recv_data << data;

    button_group.clear();
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"AIO_00", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_02", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"AIO_05", "Input", 100, "1", true});
    button_post_settings_list << button_group;

    // Set second action
    button_group.clear();
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_AIO_WRITE,
                                                        0x05, 0x00, 0x00}));
    recv_data << data;

    button_group.clear();
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"AIO_00", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_02", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"AIO_05", "Input", 0, "0", true});
    button_post_settings_list << button_group;

    // Set third action
    button_group.clear();
    button_group << QList<QVariant>({"AIO_05", "Output", 0});
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_AIO_WRITE,
                                                        0x05, 0x00, 0x64}));
    recv_data << data;

    button_group.clear();
    button_group << QList<QVariant>({MAJOR_KEY_IO,
                                     MINOR_KEY_IO_AIO_SET,
                                     GUI_GENERIC_HELPER::qList_to_byteArray(
                                        {0x05, 0x00, 0x00, 0x01})
                                    });
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"AIO_00", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_02", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"AIO_05", "Output", 100, "1", false});
    button_post_settings_list << button_group;

    // Load Basic AIO Write V1 test data
    // Performs the following actions:
    //   1) Sets AIO_05 to 1
    //   2) Sets AIO_05 to 0
    //   3) Sets AIO_05 to 1 (after chaning to Output)
    QTest::newRow("Basic AIO Write V1") \
            << button_pre_settings_list \
            << recv_data \
            << button_post_settings_list \
            << expected_signals_list;

    // Setup AIO Write Bad Pin V1 test data
    button_pre_settings_list.clear();
    recv_data.clear();
    button_post_settings_list.clear();
    expected_signals_list.clear();

    // Set first action
    button_group.clear();
    button_pre_settings_list << button_group;

    data.clear();
    data.append(GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO, MINOR_KEY_IO_AIO_WRITE,
                                                        0x7F, 0x00, 0x01}));
    recv_data << data;

    button_group.clear();
    expected_signals_list << button_group;

    button_group.clear();
    button_group << QList<QVariant>({"AIO_00", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_01", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_02", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_03", "Input", 0, "0", true});
    button_group << QList<QVariant>({"AIO_04", "Output", 0, "0", false});
    button_group << QList<QVariant>({"AIO_05", "Input", 0, "0", true});
    button_post_settings_list << button_group;

    // Load AIO Write Bad Pin V1 test data
    // Performs the following actions:
    //   1) Trys writing AIO_127 with value 1
    QTest::newRow("AIO Write Bad Pin V1") \
            << button_pre_settings_list \
            << recv_data \
            << button_post_settings_list \
            << expected_signals_list;
}

void GUI_IO_CONTROL_TESTS::test_conn_type()
{
    // Fetch data
    QFETCH(QString, config_str);
    QFETCH(QList<QList<QString>>, conn_settings);
    QFETCH(QList<QList<QString>>, send_fill_data);
    QFETCH(QList<QList<QByteArray>>, recv_fill_data);
    QFETCH(QList<QList<QList<QVariant>>>, send_expected_signals);

    // Verify input data
    int num_entries = conn_settings.length();
    QCOMPARE(send_fill_data.length(), num_entries);
    QCOMPARE(recv_fill_data.length(), num_entries);
    QCOMPARE(send_expected_signals.length(), num_entries);

    // Set new config & reset GUI
    QVERIFY(!config_str.isEmpty());
    QVERIFY(set_gui_config(config_str));
    io_control_tester->reset_gui();
    QVERIFY(remove_tempFile());

    // Setup loop variables
    int num_values;
    QList<QString> curr_settings;
    QList<QString> curr_send_data;
    QList<QByteArray> curr_recv_data;
    QList<QList<QVariant>> curr_signals;
    QByteArray dev_connected = \
            GUI_GENERIC_HELPER::qList_to_byteArray({MAJOR_KEY_IO,
                                                    MINOR_KEY_IO_REMOTE_CONN_CONNECTED,
                                                    0x01});

    // Loop over all entries
    for (int i = 0; i < num_entries; i++)
    {
        // Get current values
        curr_settings = conn_settings.at(i);
        curr_send_data = send_fill_data.at(i);
        curr_recv_data = recv_fill_data.at(i);
        curr_signals = send_expected_signals.at(i);

        // Check current values
        num_values = curr_send_data.length();
        QVERIFY(curr_send_data.length() == curr_recv_data.length());
        QVERIFY(curr_settings.length() == 4);

        // Set settings
        io_control_tester->set_connType_combo_test(curr_settings.at(0));
        io_control_tester->set_connDevice_combo_test(curr_settings.at(1));
        io_control_tester->set_connSpeed_combo_test(curr_settings.at(2));
        io_control_tester->set_connAddr_combo_test(curr_settings.at(3));

        // Click connect
        io_control_tester->conn_connect_clicked_test();

        // emit readyRead device connected
        // Required to establish connection
        emit io_control_tester->readyRead(dev_connected);

        // Loop over current values
        for (int j = 0; j < num_values; j++)
        {
            // Fill send plaintext
            io_control_tester->set_conn_msg_data_test(curr_send_data.at(i));

            // Click send
            io_control_tester->conn_send_clicked_test();

            // Emit ready read
            emit io_control_tester->readyRead(curr_recv_data.at(j));
        }

        // Click disconnect
        QCOMPARE(io_control_tester->get_conn_connectButton_text_test(), QString("Disconnect"));
        io_control_tester->conn_connect_clicked_test();
        QCOMPARE(io_control_tester->get_conn_connectButton_text_test(), QString("Connect"));
    }
}

void GUI_IO_CONTROL_TESTS::test_conn_type_data()
{
    // Setup data columns
    QTest::addColumn<QString>("config_str");

    // Parsed between call groups (data set, connect called, then
    // Each item is set as follows:
    //   0) connType
    //   1) connDevice
    //   2) connSpeed
    //   3) connAddr
    QTest::addColumn<QList<QList<QString>>>("conn_settings");

    // Send fill data column
    // Each byte array is loaded into the msg box before send is clicked
    QTest::addColumn<QList<QList<QString>>>("send_fill_data");

    // Read fill data column
    // Each byte array is passed in via the readyRead signal
    //   0) Major Key
    //   1) Minor Key
    //   2-end) Data packet
    QTest::addColumn<QList<QList<QByteArray>>>("recv_fill_data");

    // Send expected signals column (emit transmit_chunk())
    // Each QList<QVariant> must be arranged as follows:
    //  0) Major Key (uint8_t)
    //  1) Minor Key (uint8_t)
    //  2) Data Packet (QByteArray)
    QTest::addColumn<QList<QList<QList<QVariant>>>>("send_expected_signals");

    // Helper variables
    QString config_str;
    QList<QList<QString>> conn_settings_list;
    QList<QList<QString>> send_data_list;
    QList<QList<QByteArray>> recv_data_list;
    QList<QByteArray> packet;
    QList<QList<QList<QVariant>>> expected_signals_list;
    QList<QList<QVariant>> button_group;

    // Setup config str
    config_str = \
            "[IO]\n" \
            "remote_combo_settings = \\\n" \
            "\"UART,false,\",\\\n" \
            "\"I2C,false,\",\\\n" \
            "\"SPI,true,\"\n";

    // Setup verify test data (does nothing)
    conn_settings_list.clear();
    send_data_list.clear();
    recv_data_list.clear();
    expected_signals_list.clear();

    // Load verify test data (does nothing)
    QTest::newRow("Verify") << config_str \
                            << conn_settings_list \
                            << send_data_list \
                            << recv_data_list \
                            << expected_signals_list;

    // Setup basic test data
    conn_settings_list.clear();
    send_data_list.clear();
    recv_data_list.clear();
    expected_signals_list.clear();

    // Set first action

    // Load Basic test data
    // Sets settings and performs simple send/recv
    QTest::newRow("Basic") << config_str \
                           << conn_settings_list \
                           << send_data_list \
                           << recv_data_list \
                           << expected_signals_list;
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
    // Fetch data
    QFETCH(QList<QList<QList<QVariant>>>, pin_settings);
    QFETCH(QList<QList<QString>>, request_pins);
    QFETCH(QList<QList<QVariant>>, expected_signals);

    // Verify input data
    int num_entries = pin_settings.length();
    QCOMPARE(request_pins.length(), num_entries);
    QCOMPARE(expected_signals.length(), num_entries);

    // Set new config & reset GUI
    QVERIFY(set_gui_config(generic_config_str));
    io_control_tester->reset_gui();

    // Create helper chart element
    GUI_CHART_ELEMENT chart_elem_test(CHART_TYPE_2D_LINE);

    // Setup spy to catch update signal
    QList<QVariant> spy_args;
    QSignalSpy update_receive_spy(&chart_elem_test, chart_elem_test.update_receive);
    QVERIFY(update_receive_spy.isValid());

    // Setup loop variables
    QList<QVariant> curr_signal, spy_signal;

    // Loop through all buttons, calls, and verify
    for (int i = 0; i < num_entries; i++)
    {
        // Set current buttons
        foreach (QList<QVariant> button, pin_settings.at(i))
        {
            QVERIFY(button.length() == 3);
            io_control_tester->set_pin_test(button.at(0).toString(), button.at(1).toString(),
                                            button.at(2).toInt());
        }

        // Call update request
        io_control_tester->chart_update_request(request_pins.at(i), &chart_elem_test);
        qApp->processEvents();

        // Get emitted signal
        QVERIFY(update_receive_spy.count() == 1);
        spy_args = update_receive_spy.takeFirst();
        spy_signal = spy_args.at(0).toList();

        // Verify signal with expected
        curr_signal = expected_signals.at(i);
        QCOMPARE(spy_signal.length(), curr_signal.length());
        QCOMPARE(spy_signal, curr_signal);
    }
}

void GUI_IO_CONTROL_TESTS::test_chart_update_features_data()
{
    // Setup data columns
    // Parsed before each chart_update_request
    // Each button item is set as follows:
    //   0) pin_str
    //   1) combo value
    //   2) slider value
    QTest::addColumn<QList<QList<QList<QVariant>>>>("pin_settings");

    QTest::addColumn<QList<QList<QString>>>("request_pins");
    QTest::addColumn<QList<QList<QVariant>>>("expected_signals");

    // Helper variables
    QList<QList<QList<QVariant>>> pin_settings_list;
    QList<QList<QVariant>> button_group;
    QList<QList<QString>> request_pins_list;
    QList<QList<QVariant>> expected_signals_list;

    // Setup verify test data (does nothing)
    pin_settings_list.clear();
    request_pins_list.clear();
    expected_signals_list.clear();

    // Load verify test data (does nothing)
    QTest::newRow("Verify") << pin_settings_list \
                            << request_pins_list \
                            << expected_signals_list;

    // Setup basic DIO test data
    pin_settings_list.clear();
    request_pins_list.clear();
    expected_signals_list.clear();

    // Set DIO_00 to Output with value 0
    button_group.clear();
    button_group << QList<QVariant>({"DIO_00", "Output", 0});
    pin_settings_list << button_group;
    request_pins_list << QList<QString>({"DIO_00"});
    expected_signals_list << QList<QVariant>({0.0});

    // Set DIO_00 to Output with value 1
    button_group.clear();
    button_group << QList<QVariant>({"DIO_00", "Output", 1});
    pin_settings_list << button_group;
    request_pins_list << QList<QString>({"DIO_00"});
    expected_signals_list << QList<QVariant>({1.0});

    // Set DIO_00 to Output with value 0
    button_group.clear();
    button_group << QList<QVariant>({"DIO_00", "Output", 0});
    pin_settings_list << button_group;
    request_pins_list << QList<QString>({"DIO_00"});
    expected_signals_list << QList<QVariant>({0.0});

    // Load basic DIO test data
    // Performs and checks the following
    //   1) DIO_00 set to 0
    //   2) DIO_00 set to 1
    //   3) DIO_00 set to 0
    QTest::newRow("Basic DIO") << pin_settings_list \
                               << request_pins_list \
                               << expected_signals_list;

    // Setup basic AIO test data
    pin_settings_list.clear();
    request_pins_list.clear();
    expected_signals_list.clear();

    // Set AIO_04 to Output with value 0
    button_group.clear();
    button_group << QList<QVariant>({"AIO_04", "Output", 0});
    pin_settings_list << button_group;
    request_pins_list << QList<QString>({"AIO_04"});
    expected_signals_list << QList<QVariant>({0.0});

    // Set AIO_04 to Output with value 3.3
    button_group.clear();
    button_group << QList<QVariant>({"AIO_04", "Output", 330});
    pin_settings_list << button_group;
    request_pins_list << QList<QString>({"AIO_04"});
    expected_signals_list << QList<QVariant>({3.3});

    // Set AIO_04 to Output with value 0
    button_group.clear();
    button_group << QList<QVariant>({"AIO_04", "Output", 0});
    pin_settings_list << button_group;
    request_pins_list << QList<QString>({"AIO_04"});
    expected_signals_list << QList<QVariant>({0.0});

    // Load basic AIO test data
    // Performs and checks the following
    //   1) AIO_04 set to 0.0
    //   2) AIO_04 set to 3.3
    //   3) AIO_04 set to 0.0
    QTest::newRow("Basic AIO") << pin_settings_list \
                               << request_pins_list \
                               << expected_signals_list;

    // Setup bad pin test data
    pin_settings_list.clear();
    request_pins_list.clear();
    expected_signals_list.clear();

    // Try reading non-existent pin 127
    button_group.clear();
    pin_settings_list << button_group;
    request_pins_list << QList<QString>({"AIO_127"});
    expected_signals_list << QList<QVariant>({-1.0});

    // Try reading bad pin_str V1
    button_group.clear();
    pin_settings_list << button_group;
    request_pins_list << QList<QString>({"V1"});
    expected_signals_list << QList<QVariant>({-1.0});

    // Try reading bad pin_str V2
    button_group.clear();
    pin_settings_list << button_group;
    request_pins_list << QList<QString>({"V2_00"});
    expected_signals_list << QList<QVariant>({-1.0});

    // Try reading bad pin_str V3
    button_group.clear();
    pin_settings_list << button_group;
    request_pins_list << QList<QString>({"DIO_V3"});
    expected_signals_list << QList<QVariant>({-1.0});

    // Load bad pin test data
    QTest::newRow("Test bad pins") << pin_settings_list \
                                   << request_pins_list \
                                   << expected_signals_list;
}

bool GUI_IO_CONTROL_TESTS::set_gui_config(QString config_str)
{
    // Clear current config
    clear_gui_config();

    // Deode new config map
    CONFIG_MAP *gui_config = \
            GUI_GENERIC_HELPER::decode_configMap(config_str);
    if (!gui_config) return false;

    // Get new gui_map
    QMap<QString, QVariant> *gui_map = \
            gui_config->value(io_control_tester->get_gui_name(),
                              nullptr);
    if (!gui_map) return false;

    // Parse new config
    io_control_tester->parseConfigMap(gui_map);

    // Config parsed
    return true;
}

void GUI_IO_CONTROL_TESTS::clear_gui_config()
{
    // Clear current config
    QMap<QString, QVariant> reset_map;
    io_control_tester->parseConfigMap(&reset_map);
}

bool GUI_IO_CONTROL_TESTS::reset_tempFile()
{
    // Check temp_filename
    if (temp_filename.isEmpty()) return false;

    // Reset temp file (creates if doesn't exist)
    QFile file(temp_filename);
    if (!file.open(QIODevice::ReadWrite)) return false;

    // Try resizing file then close
    bool resize_success = file.resize(0);
    file.close();

    // Return if resized correctlr
    return resize_success;
}

bool GUI_IO_CONTROL_TESTS::remove_tempFile()
{
    // Check temp_filename
    if (temp_filename.isEmpty()) return false;

    // Remove temp file if exists
    QFile temp_file(temp_filename);
    if (temp_file.exists())
    {
        return temp_file.remove();
    } else
    {
        // Return true (file doesn't exist)
        return true;
    }
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
