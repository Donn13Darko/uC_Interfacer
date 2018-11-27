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
    // Delete tester if allocated
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
    // Verify reset defaults
    verify_reset_values();

    // Verify non-reset memebers
    QCOMPARE(custom_cmd_tester->get_instructions_text_test(), QString(""));
}


void GUI_CUSTOM_CMD_TESTS::test_basic_features()
{
    // Test accept all cmds (returns whether accept all checkbox is checks)
    custom_cmd_tester->set_feedback_log_all_cmds_test(false);
    QVERIFY(!custom_cmd_tester->acceptAllCMDs());
    custom_cmd_tester->set_feedback_log_all_cmds_test(true);
    QVERIFY(custom_cmd_tester->acceptAllCMDs());
    custom_cmd_tester->set_feedback_log_all_cmds_test(false);
    QVERIFY(!custom_cmd_tester->acceptAllCMDs());


    // Test waitForDevice (always returns false)
    QVERIFY(!custom_cmd_tester->waitForDevice(0));
    QVERIFY(!custom_cmd_tester->waitForDevice((uint8_t) qrand()));
}

void GUI_CUSTOM_CMD_TESTS::test_gui_config()
{
    // Fetch data
    QFETCH(QString, config_str);
    QFETCH(QString, gui_tab_name);
    QFETCH(bool, isClosable);

    // Clear current config
    QMap<QString, QVariant> reset_map;
    custom_cmd_tester->parseConfigMap(&reset_map);

    // Get gui values
    uint8_t curr_gui_key = custom_cmd_tester->get_gui_key();
    QString curr_gui_name = custom_cmd_tester->get_gui_name();

    // Generate new config
    CONFIG_MAP *gui_config = \
            GUI_GENERIC_HELPER::decode_configMap(config_str);

    // Parse new config
    custom_cmd_tester->parseConfigMap(gui_config->value(curr_gui_name, nullptr));

    // Check values
    QCOMPARE(custom_cmd_tester->get_gui_key(), curr_gui_key);
    QCOMPARE(custom_cmd_tester->get_gui_name(), curr_gui_name);
    QCOMPARE(custom_cmd_tester->get_gui_tab_name(), gui_tab_name);
    QCOMPARE(custom_cmd_tester->isClosable(), isClosable);
}

void GUI_CUSTOM_CMD_TESTS::test_gui_config_data()
{
    // Setup data columns
    QTest::addColumn<QString>("config_str");
    QTest::addColumn<QString>("gui_tab_name");
    QTest::addColumn<bool>("isClosable");

    // Helper variables
    QString config_str;
    QString curr_gui_name = custom_cmd_tester->get_gui_name();

    // Setup basic config str
    config_str.clear();
    config_str += "[" + curr_gui_name + "]\n";
    config_str += "tab_name=\"Tab A\"\n\n";
    config_str += "closable=\"false\"\n\n";

    // Load in basic
    QTest::newRow("Basic") << config_str \
                           << "Tab A" \
                           << false;

    // Setup RESET config str
    config_str.clear();
    config_str += "[" + curr_gui_name + "]\n\n";

    // Load in RESET
    QTest::newRow("RESET") << config_str \
                           << curr_gui_name \
                           << true;
}

void GUI_CUSTOM_CMD_TESTS::test_send()
{
    // Fetch data
    QFETCH(QString, send_fill_data);
    QFETCH(QList<QString>, key_and_base_fills);
    QFETCH(QList<QByteArray>, send_expected_signals);
    QFETCH(QList<bool>, click_buttons);

    // Reset the gui before (direct call slot)
    custom_cmd_tester->reset_gui();

    // Verify field lengths
    QCOMPARE(click_buttons.length(), (int) 2);

    // Setup & perform send
    perform_cmd_send(send_fill_data, key_and_base_fills,
                     click_buttons.at(0), click_buttons.at(1),
                     true, true,
                     send_expected_signals);
}

void GUI_CUSTOM_CMD_TESTS::test_send_data()
{
    // Input data columns
    QTest::addColumn<QString>("send_fill_data");

    // Send expected signals column (emit transmit_chunk())
    // Each QByteArray must be arranged as follows:
    //  0) Major Key
    //  1) Minor Key
    //  2) CMD Base
    //  3-end) Data Packet
    QTest::addColumn<QList<QByteArray>>("send_expected_signals");

    // Key & Base fill column
    // Ordering as follows:
    //  0) Major Key
    //  1) Minor Key
    //  2) Key Base
    //  3) CMD Base
    QTest::addColumn<QList<QString>>("key_and_base_fills");

    // Boolean selection column
    // Ordering as follows:
    //  0) check_send_file_radio
    //  1) check_keys_in_input
    QTest::addColumn<QList<bool>>("click_buttons");

    // Setup helper variables
    QString input_data;
    QString gui_major_key_str = QString::number(MAJOR_KEY_CUSTOM_CMD, 16);
    QString gui_minor_key_str = QString::number(MINOR_KEY_CUSTOM_CMD_CMD, 16);
    QList<QByteArray> expected_send_list;

    // Setup Simple Send test data (no data)
    input_data.clear();
    expected_send_list.clear();
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE,
                     0, 16, 0}));

    // Enter Simple Send test:
    //  send_fill_data = ""
    //  send_expected_signals = {"6 2 10 0"}
    //  key_and_base_fills = {major_key, minor_key, key_base, cmd_base}
    //                     = {"6", "3", "16", "0"}
    //  click_buttons = {send_file_radio, keys_in_input}
    //                     = {false, false}
    QTest::newRow("Simple Send Input") \
            << input_data \
            << expected_send_list \
            << QList<QString>({gui_major_key_str, gui_minor_key_str, "16", "0"}) \
            << QList<bool>({false, false});

    // Setup simple keys test data
    input_data.clear();
    input_data += "6 3 Read\n";

    expected_send_list.clear();
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE,
                     0, 16, 0}));
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_CMD, 0})
                .append("Read"));

    // Enter simple keys test:
    //  send_fill_data = "6 3 Read\n"
    //  send_expected_signals = {"6 2 10 0", "6 3 Read"}
    //  key_and_base_fills = {major_key, minor_key, key_base, cmd_base}
    //                     = {"6", "3", "16", "0"}
    //  click_buttons = {send_file_radio, keys_in_input}
    //                     = {false, true}
    QTest::newRow("Simple Keys In Input") \
            << input_data \
            << expected_send_list \
            << QList<QString>({gui_major_key_str, gui_minor_key_str, "16", "0"}) \
            << QList<bool>({false, true});

    // Setup set base v1 test data
    input_data.clear();
    input_data += "6 2 2 0\n110 11 read\n";

    expected_send_list.clear();
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE,
                     0, 16, 16}));
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE,
                     16, 50, 0, 48}));
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_CMD, 0})
                .append("read"));

    // Enter set base v1 test:
    //  send_fill_data = "6 2 2 0\n6 3 read\n"
    //  send_expected_signals = {"6 2 10 10", "6 2 2 0", "110 11 read"}
    //  key_and_base_fills = {major_key, minor_key, key_base, cmd_base}
    //                     = {"6", "3", "16", "16"}
    //  click_buttons = {send_file_radio, keys_in_input}
    //                     = {false, true}
    QTest::newRow("Set Bases V1") \
            << input_data \
            << expected_send_list \
            << QList<QString>({gui_major_key_str, gui_minor_key_str, "16", "16"}) \
            << QList<bool>({false, true});

    // Setup double set base test data
    input_data.clear();
    input_data += "6 2 10 10\n6 3 52 45 41 44\n";

    expected_send_list.clear();
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE,
                     0, 16, 16}));
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_CMD, 16,
                     53, 50, 0, 52, 53, 0, 52, 49, 0, 52, 52}));

    // Enter double set base test:
    //  send_fill_data = "6 2 10 0\n6 3 52 45 41 44\n"
    //  send_expected_signals = {"6 2 10 10", "6 3 52 45 41 44"}
    //  key_and_base_fills = {major_key, minor_key, key_base, cmd_base}
    //                     = {"6", "3", "16", "16"}
    //  click_buttons = {send_file_radio, keys_in_input}
    //                     = {false, true}
    QTest::newRow("Double set Bases") \
            << input_data \
            << expected_send_list \
            << QList<QString>({gui_major_key_str, gui_minor_key_str, "16", "16"}) \
            << QList<bool>({false, true});

    // Setup Space in CMD test data
    input_data.clear();
    input_data += "6 3 Re ad\n";

    expected_send_list.clear();
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE,
                     0, 16, 0}));
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_CMD, 0})
                .append("Re ad"));

    // Enter Space in CMD test:
    //  send_fill_data = "6 3 Re ad\n"
    //  send_expected_signals = {"6 2 10 0", "6 3 Re ad"}
    //  key_and_base_fills = {major_key, minor_key, key_base, cmd_base}
    //                     = {"6", "3", "16", "0"}
    //  click_buttons = {send_file_radio, keys_in_input}
    //                     = {false, true}
    QTest::newRow("Space in CMD") \
            << input_data \
            << expected_send_list \
            << QList<QString>({gui_major_key_str, gui_minor_key_str, "16", "0"}) \
            << QList<bool>({false, true});

    // Setup Malformed Input V1 test data
    input_data.clear();
    input_data += "6 \n";

    expected_send_list.clear();
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE,
                     0, 16, 16}));

    // Enter Malformed Input V1 test:
    //  send_fill_data = "6 \n"
    //  send_expected_signals = {"6 2 10 10"}
    //  key_and_base_fills = {major_key, minor_key, key_base, cmd_base}
    //                     = {"6", "3", "16", "16"}
    //  click_buttons = {send_file_radio, keys_in_input}
    //                     = {false, true}
    QTest::newRow("Malformed Input V1") \
            << input_data \
            << expected_send_list \
            << QList<QString>({gui_major_key_str, gui_minor_key_str, "16", "16"}) \
            << QList<bool>({false, true});

    // Setup Malformed Input V2 test data
    input_data.clear();
    input_data += "6 2 1 \n";

    expected_send_list.clear();
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE,
                     0, 16, 16}));

    // Enter Malformed Input V2 test:
    //  send_fill_data = "6 2 1 \n"
    //  send_expected_signals = {"6 2 10 10"}
    //  key_and_base_fills = {major_key, minor_key, key_base, cmd_base}
    //                     = {"6", "3", "16", "16"}
    //  click_buttons = {send_file_radio, keys_in_input}
    //                     = {false, true}
    QTest::newRow("Malformed Input V2") \
            << input_data \
            << expected_send_list \
            << QList<QString>({gui_major_key_str, gui_minor_key_str, "16", "16"}) \
            << QList<bool>({false, true});

    // Setup KB_0_CB_0 test data
    input_data.clear();
    input_data += "6 3 Read\n";

    expected_send_list.clear();
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE,
                     0, 0, 0}));
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {54, 51, 0})
                .append("Read"));

    // Enter KB_0_CB_0 test:
    //  send_fill_data = "54 51 Read\n"
    //  send_expected_signals = {"6 2 0 0", "6 3 Read"}
    //  key_and_base_fills = {major_key, minor_key, key_base, cmd_base}
    //                     = {"6", "3", "0", "0"}
    //  click_buttons = {send_file_radio, keys_in_input}
    //                     = {false, true}
    QTest::newRow("KB_0_CB_0") \
            << input_data \
            << expected_send_list \
            << QList<QString>({gui_major_key_str, gui_minor_key_str, "0", "0"}) \
            << QList<bool>({false, true});

    // Setup KB_16_CB_16 test data
    input_data.clear();
    input_data += "6 3 52 45 41 44\n";

    expected_send_list.clear();
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE,
                     0, 16, 16}));
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_CMD, 16,
                    53, 50, 0, 52, 53, 0, 52, 49, 0, 52, 52}));

    // Enter KB_16_CB_16 test:
    //  send_fill_data = "54 51 52 45 41 44\n"
    //  send_expected_signals = {"6 2 10 10", "6 3 52 45 41 44"}
    //  key_and_base_fills = {major_key, minor_key, key_base, cmd_base}
    //                     = {"6", "3", "16", "16"}
    //  click_buttons = {send_file_radio, keys_in_input}
    //                     = {false, true}
    QTest::newRow("KB_16_CB_16") \
            << input_data \
            << expected_send_list \
            << QList<QString>({gui_major_key_str, gui_minor_key_str, "16", "16"}) \
            << QList<bool>({false, true});

    // Setup Simple Send File test data (no data)
    input_data.clear();

    expected_send_list.clear();
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE,
                     0, 16, 0}));

    // Enter Simple Send File test:
    //  send_fill_data = ""
    //  send_expected_signals = {"6 2 10 0"}
    //  key_and_base_fills = {major_key, minor_key, key_base, cmd_base}
    //                     = {"6", "3", "16", "0"}
    //  click_buttons = {send_file_radio, keys_in_input}
    //                     = {false, false}
    QTest::newRow("Simple Send File") \
            << input_data \
            << expected_send_list \
            << QList<QString>({gui_major_key_str, gui_minor_key_str, "16", "0"}) \
            << QList<bool>({true, false});

    // Setup Send File V1 test data (no data)
    input_data.clear();
    input_data += "6 3 Read\n";

    expected_send_list.clear();
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE,
                     0, 16, 0}));
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_CMD, 0})
                .append("Read"));

    // Enter Send File V1 test:
    //  send_fill_data = ""
    //  send_expected_signals = {"6 2 10 0", "6 3 Read"}
    //  key_and_base_fills = {major_key, minor_key, key_base, cmd_base}
    //                     = {"6", "3", "16", "0"}
    //  click_buttons = {send_file_radio, keys_in_input}
    //                     = {false, false}
    QTest::newRow("Send File V1") \
            << input_data \
            << expected_send_list \
            << QList<QString>({gui_major_key_str, gui_minor_key_str, "16", "0"}) \
            << QList<bool>({true, false});
}

void GUI_CUSTOM_CMD_TESTS::test_rcvd()
{
    // Fetch data
    QFETCH(QList<QByteArray>, rcvd_fill_data);
    QFETCH(QString, rcvd_expected_display_data);
    QFETCH(QByteArray, rcvd_expected_file_data);
    QFETCH(QList<bool>, click_buttons);

    // Reset the gui before (direct call slot)
    custom_cmd_tester->reset_gui();

    // Verify field lengths
    QCOMPARE(click_buttons.length(), (int) 5);

    // Setup & perform rcvd
    perform_cmd_rcvd(rcvd_fill_data,
                     click_buttons.at(0), click_buttons.at(1),
                     click_buttons.at(2), click_buttons.at(3),
                     click_buttons.at(4), true,
                     rcvd_expected_display_data,
                     rcvd_expected_file_data);
}

void GUI_CUSTOM_CMD_TESTS::test_rcvd_data()
{
    // Input data columns
    QTest::addColumn<QList<QByteArray>>("rcvd_fill_data");
    QTest::addColumn<QString>("rcvd_expected_display_data");
    QTest::addColumn<QByteArray>("rcvd_expected_file_data");

    // Boolean selection column
    // Ordering as follows:
    //  0) check_log_all_cmds
    //  1) check_append_newline
    //  2) check_clear_on_set
    //  3) click_clear
    //  4) click_save
    QTest::addColumn<QList<bool>>("click_buttons");

    // Setup helper variables
    QString gui_major_key_str = QString::number(MAJOR_KEY_CUSTOM_CMD, 16);
    QString gui_minor_key_str = QString::number(MINOR_KEY_CUSTOM_CMD_CMD, 16);
    QList<QByteArray> rcvd_list;
    QString expected_feedback;

    // Setup basic test data (everything empty)
    rcvd_list.clear();
    expected_feedback.clear();

    // Load basic test data
    //  rcvd_fill_data = {}
    //  rcvd_expected_display_data = ""
    //  rcvd_expected_file_data = ""
    //  click_buttons = {log_all_cmds, append_newline, clear_on_set,
    //                      click_clear, click_save}
    //                = {true, false, false,
    //                      true, true}
    QTest::newRow("Basic Recv Verify") \
            << rcvd_list << expected_feedback << expected_feedback.toLatin1() \
            << QList<bool>({true, false, false,
                            true, true});

    // Setup trans size test data
    rcvd_list.clear();
    rcvd_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE,
                     16, 0}));
    rcvd_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_SET_TRANS_SIZE,
                     100}));
    rcvd_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_CMD})
                     .append("Feed"));

    expected_feedback.clear();
    expected_feedback += gui_major_key_str + " " \
            + gui_minor_key_str + " Feed\n";


    // Load trans size test data
    //  rcvd_fill_data = {"6 2 10 0", "6 1 64", "6 3 Feed"}
    //  rcvd_expected_display_data = "6 3 Feed\n"
    //  rcvd_expected_file_data = ^^Same as above
    //  click_buttons = {log_all_cmds, append_newline, clear_on_set,
    //                      click_clear, click_save}
    //                = {true, false, false,
    //                      true, true}
    QTest::newRow("Recv Verify Trans Size Set") \
            << rcvd_list << expected_feedback << expected_feedback.toLatin1() \
            << QList<bool>({true, true, true,
                            false, false});

    // Setup ignore cmds test data
    rcvd_list.clear();
    rcvd_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE,
                     16, 0}));
    rcvd_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD+1, MINOR_KEY_CUSTOM_CMD_SET_TRANS_SIZE,
                     100}));
    rcvd_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_CMD})
                     .append("Feed"));

    expected_feedback.clear();
    expected_feedback += gui_major_key_str + " " \
            + QString::number(MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE, 16) + " 10 0\n" \
            + gui_major_key_str + " " + gui_minor_key_str + " Feed\n";


    // Load ignore cmds test data
    //  rcvd_fill_data = {"6 2 10 0", "10 1 64", "6 3 Feed"}
    //  rcvd_expected_display_data = "6 2 10 0\n6 3 Feed\n"
    //  rcvd_expected_file_data = ^^Same as above
    //  click_buttons = {log_all_cmds, append_newline, clear_on_set,
    //                      click_clear, click_save}
    //                = {false, true, true,
    //                      false, false}
    QTest::newRow("Recv Verify Ignore CMDs") \
            << rcvd_list << expected_feedback << expected_feedback.toLatin1() \
            << QList<bool>({false, true, true,
                            false, false});

    // Setup complete packet test data
    rcvd_list.clear();
    rcvd_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE,
                     16, 0}));
    rcvd_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_SET_TRANS_SIZE,
                     5}));
    rcvd_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_CMD})
                     .append("12345"));

    expected_feedback.clear();
    expected_feedback += gui_major_key_str + " " \
            + gui_minor_key_str + " 12345\n";


    // Load complete packet test data
    //  rcvd_fill_data = {"6 2 10 0", "6 1 5", "6 3 12345"}
    //  rcvd_expected_display_data = "6 3 12345\n"
    //  rcvd_expected_file_data = ^^Same as above
    //  click_buttons = {log_all_cmds, append_newline, clear_on_set,
    //                      click_clear, click_save}
    //                = {true, true, false,
    //                      true, true}
    QTest::newRow("Recv Complete Packet") \
            << rcvd_list << expected_feedback << expected_feedback.toLatin1() \
            << QList<bool>({false, true, true,
                            false, false});

    // Setup complete packet test data
    rcvd_list.clear();
    rcvd_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE,
                     16, 0}));
    rcvd_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_SET_TRANS_SIZE,
                     5}));
    rcvd_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_CMD})
                     .append("12345"));
    rcvd_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_SET_TRANS_SIZE,
                     4}));
    rcvd_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_CMD})
                     .append("6789"));

    expected_feedback.clear();
    expected_feedback += gui_major_key_str + " " \
            + gui_minor_key_str + " 6789\n";


    // Load multiple packets test data
    //  rcvd_fill_data = {"6 2 10 0", "6 1 5", "6 3 12345", "6 1 4", "6 3 6789"}
    //  rcvd_expected_display_data = "6 3 6789\n"
    //  rcvd_expected_file_data = ^^Same as above
    //  click_buttons = {log_all_cmds, append_newline, clear_on_set,
    //                      click_clear, click_save}
    //                = {true, true, false,
    //                      true, true}
    QTest::newRow("Recv Multiple Packets") \
            << rcvd_list << expected_feedback << expected_feedback.toLatin1() \
            << QList<bool>({false, true, true,
                            false, false});
}

void GUI_CUSTOM_CMD_TESTS::test_looped_send_rcvd_cmd()
{
    // Set load data
}

void GUI_CUSTOM_CMD_TESTS::test_looped_send_rcvd_cmd_data()
{
    // Set load data
}

void GUI_CUSTOM_CMD_TESTS::test_complex_cmd()
{
    // Fetch data
    QFETCH(QString, instructions);
    QFETCH(QString, send_fill_data);
    QFETCH(QList<QByteArray>, rcvd_fill_data);
    QFETCH(QString, rcvd_expected_display_data);
    QFETCH(QByteArray, rcvd_expected_file_data);
    QFETCH(QList<QByteArray>, send_expected_signals);
    QFETCH(QList<QString>, key_and_base_fills);
    QFETCH(QList<bool>, click_buttons);
    QFETCH(bool, check_send);
    QFETCH(bool, check_rcvd);

    // Reset the gui before (direct call slot)
    custom_cmd_tester->reset_gui();

    // Set basic info
    custom_cmd_tester->set_instructions_text_test(instructions);

    // Verify field lengths
    QCOMPARE(click_buttons.length(), (int) 9);

    // Setup & perform rcvd
    perform_cmd_rcvd(rcvd_fill_data,
                     click_buttons.at(3), click_buttons.at(4),
                     click_buttons.at(5), click_buttons.at(6),
                     click_buttons.at(7), check_rcvd,
                     rcvd_expected_display_data,
                     rcvd_expected_file_data);


    // Setup & perform send
    perform_cmd_send(send_fill_data, key_and_base_fills,
                     click_buttons.at(0), click_buttons.at(1),
                     click_buttons.at(2), check_send,
                     send_expected_signals);

    // Check if reseting with button
    if (click_buttons.at(8))
    {
        // Send Reset
        custom_cmd_tester->reset_clicked_test();

        // Verify reset values
        verify_reset_values();

        // Verify non-reset values
        QCOMPARE(custom_cmd_tester->get_instructions_text_test(), instructions);
    }
}

void GUI_CUSTOM_CMD_TESTS::test_complex_cmd_data()
{
    // Input data columns
    QTest::addColumn<QString>("instructions");
    QTest::addColumn<QString>("send_fill_data");
    QTest::addColumn<QList<QByteArray>>("rcvd_fill_data");
    QTest::addColumn<QString>("rcvd_expected_display_data");
    QTest::addColumn<QByteArray>("rcvd_expected_file_data");

    // Send expected signals column (emit transmit_chunk())
    // Each QByteArray must be arranged as follows:
    //  0) Major Key
    //  1) Minor Key
    //  2) CMD Base
    //  3-end) Data Packet
    QTest::addColumn<QList<QByteArray>>("send_expected_signals");

    // Key & Base fill column
    // Ordering as follows:
    //  0) Major Key
    //  1) Minor Key
    //  2) Key Base
    //  3) CMD Base
    QTest::addColumn<QList<QString>>("key_and_base_fills");

    // Click buttons column
    // Ordering as follows:
    //  0) check_send_file_radio
    //  1) check_keys_in_input
    //  2) click_send
    //  3) check_log_all_cmds
    //  4) check_append_newline
    //  5) check_clear_on_set
    //  6) click_clear
    //  7) click_save
    //  8) click_reset
    QTest::addColumn<QList<bool>>("click_buttons");

    // Misc columns
    QTest::addColumn<bool>("check_send");
    QTest::addColumn<bool>("check_rcvd");

    // Setup helper variables
    QString gui_major_key_str = QString::number(MAJOR_KEY_CUSTOM_CMD, 16);
    QString gui_minor_key_str = QString::number(MINOR_KEY_CUSTOM_CMD_CMD, 16);
    QList<QByteArray> expected_send_list;
    QList<QByteArray> rcvd_list;
    QString expected_feedback;

    // Setup basic test data (everything empty)
    rcvd_list.clear();
    expected_feedback.clear();
    expected_send_list.clear();

    // Load basic test data:
    //  instructions = "Instructions"
    //  send_fill_data = "File_Path"
    //  rcvd_fill_data = {}
    //  rcvd_expected_display_data = ""
    //  rcvd_expected_file_data = ""
    //  send_expected_signals = {}
    //  key_and_base_fills = {major_key, minor_key,
    //                          key_base, cmd_base}
    //                     = {"0", "0", "0", "0"}
    //  click_buttons = {send_file_radio, keys_in_input, click_send,
    //                      log_all_cmds, append_newline, clear_on_set,
    //                      click_clear, click_save, click_reset}
    //                = {true, true, false,
    //                      true, false, false,
    //                      false, false, true}
    //  check_send = false, check_rcvd = false
    QTest::newRow("Basic Complex Verify") \
            << "Instructions" << "File_Path" \
            << rcvd_list << expected_feedback << expected_feedback.toLatin1() \
            << expected_send_list \
            << QList<QString>({"0", "0", "0", "0"}) \
            << QList<bool>({true, true, false,
                            true, true, false,
                            false, false, true}) \
            << false << false;

    // Setup send test data
    rcvd_list.clear();
    expected_feedback.clear();

    expected_send_list.clear();
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE,
                     0, 16, 0}));
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_CMD, 0})
                .append("Input_PlainText"));

    // Load send test data:
    //  instructions = ""
    //  send_fill_data = "Input_PlainText"
    //  rcvd_fill_data = {}
    //  rcvd_expected_display_data = ""
    //  rcvd_expected_file_data = ""
    //  send_expected_signals = {"6 2 0 10 0", "6 3 0 Input_PlainText"}
    //  key_and_base_fills = {major_key, minor_key,
    //                          key_base, cmd_base}
    //                     = {"6", "3", "16", "0"}
    //  click_buttons = {send_file_radio, keys_in_input, click_send,
    //                      log_all_cmds, append_newline, clear_on_set,
    //                      click_clear, click_save, click_reset}
    //                = {false, false, true,
    //                      false, true, true,
    //                      false, false, true}
    //  check_send = true, check_rcvd = false
    QTest::newRow("Test Complex Send") \
            << "" << "Input_PlainText" \
            << rcvd_list << expected_feedback << expected_feedback.toLatin1() \
            << expected_send_list \
            << QList<QString>({gui_major_key_str, gui_minor_key_str, "16", "0"}) \
            << QList<bool>({false, false, true,
                            true, false, false,
                            false, false, true}) \
            << true << false;

    // Setup recv test data
    expected_send_list.clear();
    rcvd_list.clear();
    rcvd_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE,
                     16, 0}));
    rcvd_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_CMD})
                     .append("Feedback_PlainText"));

    expected_feedback.clear();
    expected_feedback += gui_major_key_str + " " \
            + QString::number(MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE, 16) + " 10 0" \
            + gui_major_key_str + " " + gui_minor_key_str + " Feedback_PlainText";

    // Load recv test data:
    //  instructions = ""
    //  send_fill_data = ""
    //  rcvd_fill_data = {"6 2 0 10 0", "6 3 0 Feedback_PlainText"}
    //  rcvd_expected_display_data = "6 2 0 10 06 3 0 Feedback_PlainText"
    //  rcvd_expected_file_data = ^^Same as above
    //  send_expected_signals = {}
    //  key_and_base_fills = {major_key, minor_key, key_base, cmd_base}
    //                     = {"0", "0", "0", "0"}
    //  click_buttons = {send_file_radio, keys_in_input, click_send,
    //                      log_all_cmds, append_newline, clear_on_set,
    //                      click_clear, click_save, click_reset}
    //                = {true, true, false,
    //                      true, false, false,
    //                      false, false, true}
    //  check_send = false, check_rcvd = true
    QTest::newRow("Test Complex recv, recv no newline") \
            << "" << "" \
            << rcvd_list << expected_feedback << expected_feedback.toLatin1() \
            << expected_send_list \
            << QList<QString>({"0", "0", "0", "0"}) \
            << QList<bool>({true, true, false,
                            true, false, false,
                            false, false, true}) \
            << false << true;

    // Setup send/recv test data
    expected_send_list.clear();
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE,
                     0, 16, 0}));
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_CMD, 0})
                .append("Input_PlainText"));

    rcvd_list.clear();
    rcvd_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE,
                     16, 0}));
    rcvd_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_CMD})
                     .append("Feedback_PlainText"));

    expected_feedback.clear();
    expected_feedback += gui_major_key_str + " " \
            + QString::number(MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE, 16) + " 10 0\n" \
            + gui_major_key_str + " " + gui_minor_key_str + " Feedback_PlainText\n";

    // Load send/recv test data:
    //  instructions = ""
    //  send_fill_data = "Input_PlainText"
    //  rcvd_fill_data = {"6 2 0 10 0", "6 3 0 Feedback_PlainText"}
    //  rcvd_expected_display_data = "6 2 0 10 0\n6 3 0 Feedback_PlainText\n"
    //  rcvd_expected_file_data = ^^Same as above
    //  send_expected_signals = {"6 2 0 10 0", "6 3 0 Feedback_PlainText"}
    //  key_and_base_fills = {major_key, minor_key, key_base, cmd_base}
    //                     = {"0", "0", "0", "0"}
    //  click_buttons = {send_file_radio, keys_in_input, click_send,
    //                      log_all_cmds, append_newline, clear_on_set,
    //                      click_clear, click_save, click_reset}
    //                = {false, true, true,
    //                      true, false, false,
    //                      false, false, true}
    //  check_send = true, check_rcvd = true
    QTest::newRow("Test Complex send/recv") \
            << "" << "Input_PlainText" \
            << rcvd_list << expected_feedback << expected_feedback.toLatin1() \
            << expected_send_list \
            << QList<QString>({gui_major_key_str, gui_minor_key_str, "16", "0"}) \
            << QList<bool>({false, false, true,
                            true, true, false,
                            false, false, true}) \
            << true << true;
}

void GUI_CUSTOM_CMD_TESTS::verify_reset_values()
{
    // Check class memebrs
    QVERIFY(custom_cmd_tester->isClosable());
    QVERIFY(!custom_cmd_tester->acceptAllCMDs());
    QCOMPARE(custom_cmd_tester->get_gui_key(), (uint8_t) MAJOR_KEY_CUSTOM_CMD);
    QCOMPARE(custom_cmd_tester->get_gui_name(), QString("Custom CMD"));
    QCOMPARE(custom_cmd_tester->rcvd_formatted_size_test(), (qint64) 0);

    // Check encoding keys
    QCOMPARE(custom_cmd_tester->get_major_key_test(), QString::number(MAJOR_KEY_CUSTOM_CMD, 16));
    QCOMPARE(custom_cmd_tester->get_minor_key_test(), QString::number(MINOR_KEY_CUSTOM_CMD_CMD, 16));
    QCOMPARE(custom_cmd_tester->get_key_base_test(), QString("16"));
    QCOMPARE(custom_cmd_tester->get_cmd_base_test(), QString("16"));

    // Check input and recv
    QCOMPARE(custom_cmd_tester->get_user_input_text_test(), QString(""));
    QCOMPARE(custom_cmd_tester->get_file_input_text_test(), QString(""));
    QCOMPARE(custom_cmd_tester->get_displayed_feedback_test(), QString(""));

    // Check progress bars
    QCOMPARE(custom_cmd_tester->get_progress_update_recv_value_test(), (int) 0);
    QCOMPARE(custom_cmd_tester->get_progress_update_recv_string_test(), QString(""));
    QCOMPARE(custom_cmd_tester->get_progress_update_send_value_test(), (int) 0);
    QCOMPARE(custom_cmd_tester->get_progress_update_send_string_test(), QString(""));

    // Check check boxes
    QCOMPARE(custom_cmd_tester->get_cmd_input_radio_test(), true);
    QCOMPARE(custom_cmd_tester->get_keys_in_input_test(), false);
    QCOMPARE(custom_cmd_tester->get_feedback_log_all_cmds_test(), false);
    QCOMPARE(custom_cmd_tester->get_feedback_append_newline_test(), true);
    QCOMPARE(custom_cmd_tester->get_feedback_clear_on_set_test(), true);
}

void GUI_CUSTOM_CMD_TESTS::perform_cmd_rcvd(QList<QByteArray> rcvd_fill_data,
                                            bool log_all_cmds, bool append_newline,
                                            bool clear_on_set, bool click_clear_button,
                                            bool click_save_button, bool check_rcvd,
                                            QString rcvd_expected_display_data,
                                            QByteArray rcvd_expected_file_data)
{
    // Set boolen selections
    custom_cmd_tester->set_feedback_log_all_cmds_test(log_all_cmds);
    custom_cmd_tester->set_feedback_append_newline_test(append_newline);
    custom_cmd_tester->set_feedback_clear_on_set_test(clear_on_set);

    // Setup spy to catch tranmit signal
    QList<QVariant> spy_args;
    QSignalSpy progress_spy(custom_cmd_tester, custom_cmd_tester->progress_update_recv);
    QVERIFY(progress_spy.isValid());

    // Load in rcvd values (emit readyRead() signal for tester)
    int progress_val;
    QString expected_recv_len_str, progress_str;
    uint32_t expected_recv_len = 0, target_recv = 0;
    foreach (QByteArray rcvd, rcvd_fill_data)
    {
        // Send command
        emit custom_cmd_tester->readyRead(rcvd);
        qApp->processEvents();

        // Check if major key is gui key (progress bar updates)
        if (rcvd.at(s1_major_key_loc) == custom_cmd_tester->get_gui_key())
        {
            // Process extra events since gui relevant
            // (required for progress_bar updates)
            qApp->processEvents();

            // Check minor key
            switch (rcvd.at(s1_minor_key_loc))
            {
                case MINOR_KEY_CUSTOM_CMD_SET_TRANS_SIZE:
                {
                    expected_recv_len = GUI_GENERIC_HELPER::byteArray_to_uint32(rcvd.mid(s1_end_loc));
                    expected_recv_len_str = "/" + QString::number(expected_recv_len / 1000.0f) + "KB";
                    target_recv = 0;

                    // Verify set signal
                    // Clear on set causes double set_progress to be emitted
                    if (clear_on_set)
                    {
                        // Handle two reset signals
                        QCOMPARE(progress_spy.count(), 2);
                        spy_args = progress_spy.takeFirst();
                        QCOMPARE(spy_args.at(0).toInt(), 0);
                        QCOMPARE(spy_args.at(1).toString(), QString(""));
                    } else
                    {
                        QCOMPARE(progress_spy.count(), 1);
                    }

                    // Verify spy signal and progress bar
                    spy_args = progress_spy.takeFirst();
                    QCOMPARE(spy_args.at(0).toInt(), 0);
                    QCOMPARE(spy_args.at(1).toString(), QString(""));
                    QCOMPARE(custom_cmd_tester->get_progress_update_recv_value_test(), 0);
                    QCOMPARE(custom_cmd_tester->get_progress_update_recv_string_test(), QString(""));
                    break;
                }
                case MINOR_KEY_CUSTOM_CMD_CMD:
                {
                    // Check if expected set
                    if (expected_recv_len)
                    {
                        // Increment target recv
                        target_recv += rcvd.length() - 2;

                        // Verify set signal
                        QCOMPARE(progress_spy.count(), 1);
                        spy_args = progress_spy.takeFirst();

                        // Decide what signal should be
                        if (expected_recv_len <= target_recv)
                        {
                            // Reset expected (recv done)
                            expected_recv_len = 0;

                            // Verify done signal & progress setting
                            QCOMPARE(spy_args.at(0).toInt(), 100);
                            QCOMPARE(spy_args.at(1).toString(), QString("Done!"));
                            QCOMPARE(custom_cmd_tester->get_progress_update_recv_value_test(), 100);
                            QCOMPARE(custom_cmd_tester->get_progress_update_recv_string_test(), QString("Done!"));
                        } else
                        {
                            // Get values
                            progress_val = qRound(((float) target_recv / expected_recv_len) * 100.0f);
                            progress_str = QString::number((float) target_recv / 1000.0f) + expected_recv_len_str;

                            // Verify increment string
                            QCOMPARE(spy_args.at(0).toInt(), progress_val);
                            QCOMPARE(spy_args.at(1).toString(), progress_str);
                            QCOMPARE(custom_cmd_tester->get_progress_update_recv_value_test(), progress_val);
                            QCOMPARE(custom_cmd_tester->get_progress_update_recv_string_test(), progress_str);
                        }
                    }
                    break;
                }
            }
        }
    }

    // If checking, verify that rcvd data matches the expected
    if (check_rcvd)
    {
        QCOMPARE(custom_cmd_tester->get_displayed_feedback_test(), rcvd_expected_display_data);
        QCOMPARE(custom_cmd_tester->rcvd_formatted_readAll_test(), rcvd_expected_file_data);
    } else if (click_save_button)
    {
        // Can't manually click button (blocking pop-up dialog)
        // Button slot simply calls rcvd_formatted_save() which is tested in gui-base-tests.cpp
        QCOMPARE(custom_cmd_tester->rcvd_formatted_readAll_test(), rcvd_expected_file_data);
    }

    // Check if clearing
    if (click_clear_button)
    {
        // Click clear button
        custom_cmd_tester->feedback_clear_clicked_test();

        // Verify clear
        QCOMPARE(custom_cmd_tester->rcvd_formatted_size_test(), (int) 0);
        QCOMPARE(custom_cmd_tester->rcvd_formatted_readAll_test(), QByteArray());
        QCOMPARE(custom_cmd_tester->get_displayed_feedback_test(), QString(""));

        // Verify set signal
        QCOMPARE(progress_spy.count(), 1);
        spy_args = progress_spy.takeFirst();
        QCOMPARE(spy_args.at(0).toInt(), 0);
        QCOMPARE(spy_args.at(1).toString(), QString(""));
    }
}

void GUI_CUSTOM_CMD_TESTS::perform_cmd_send(QString send_fill_data, QList<QString> key_and_base_fills,
                                            bool send_file_radio, bool keys_in_input,
                                            bool click_send, bool check_send,
                                            QList<QByteArray> send_expected_signals)
{
    // Set key fields
    QCOMPARE(key_and_base_fills.length(), (int) 4);
    custom_cmd_tester->set_major_key_test(key_and_base_fills.at(0));
    custom_cmd_tester->set_minor_key_test(key_and_base_fills.at(1));
    custom_cmd_tester->set_key_base_test(key_and_base_fills.at(2));
    custom_cmd_tester->set_cmd_base_test(key_and_base_fills.at(3));

    // Create temp file to store data in
    QTemporaryFile temp_file;
    temp_file.setAutoRemove(true);

    // Set input info
    custom_cmd_tester->set_cmd_input_radio_test(send_file_radio);
    if (send_file_radio)
    {
        // Load in data
        QVERIFY(temp_file.open());
        temp_file.write(send_fill_data.toLatin1());
        temp_file.close();

        // Set input text
        custom_cmd_tester->set_file_input_text_test(temp_file.fileName());
    } else
    {
        custom_cmd_tester->set_user_input_text_test(send_fill_data);
    }

    // Set boolen selections
    custom_cmd_tester->set_keys_in_input_test(keys_in_input);

    // Setup spy to catch tranmit signal
    QList<QVariant> spy_args;
    QSignalSpy transmit_chunk_spy(custom_cmd_tester, custom_cmd_tester->transmit_chunk);
    QVERIFY(transmit_chunk_spy.isValid());

    // See if clicking send
    if (click_send)
    {
        // Click send
        custom_cmd_tester->send_clicked_test();
        qApp->processEvents();
    }

    // If checking, Verify spy signals after send
    if (check_send)
    {
        // Verify num signals equal to expected
        QCOMPARE(transmit_chunk_spy.count(), send_expected_signals.length());

        // Loop through variables
        foreach (QByteArray expected_send, send_expected_signals)
        {
            // Get signal
            spy_args = transmit_chunk_spy.takeFirst();

            // Verify values
            QVERIFY(4 <= expected_send.length());
            QCOMPARE(spy_args.at(0).toInt(), (int) expected_send.at(0));
            QCOMPARE(spy_args.at(1).toInt(), (int) expected_send.at(1));
            QCOMPARE(spy_args.at(3).toInt(), (int) expected_send.at(2));
            QCOMPARE(spy_args.at(2).toByteArray(), expected_send.mid(3));
        }
    }
}
