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

void GUI_CUSTOM_CMD_TESTS::test_complex_cmd()
{
    // Fetch data
    QFETCH(QString, send_file_data);
    QFETCH(QString, send_input_data);
    QFETCH(QList<QByteArray>, rcvd_fill_data);
    QFETCH(QString, rcvd_expected_display_data);
    QFETCH(QByteArray, rcvd_expected_file_data);
    QFETCH(QString, instructions);
    QFETCH(QList<QString>, key_and_base_fills);
    QFETCH(QList<uint8_t>, bases_after_send);
    QFETCH(QList<QByteArray>, send_expected_packets);
    QFETCH(QList<bool>, boolean_selections);
    QFETCH(quint32, expected_recv_len);
    QFETCH(bool, click_send_button);
    QFETCH(bool, click_reset_button);

    // Reset the gui before (direct call slot)
    custom_cmd_tester->reset_gui();

    // Set key fields
    QCOMPARE(key_and_base_fills.length(), (int) 4);
    custom_cmd_tester->set_major_key_test(key_and_base_fills.at(0));
    custom_cmd_tester->set_minor_key_test(key_and_base_fills.at(1));
    custom_cmd_tester->set_key_base_test(key_and_base_fills.at(2));
    custom_cmd_tester->set_cmd_base_test(key_and_base_fills.at(3));

    // Set basic info
    custom_cmd_tester->set_instructions_text_test(instructions);

    // Set send file info
    custom_cmd_tester->set_cmd_input_radio_test(true);
    custom_cmd_tester->set_file_input_text_test(send_file_data);

    // Set send input info
    custom_cmd_tester->set_cmd_input_radio_test(false);
    custom_cmd_tester->set_user_input_text_test(send_input_data);

    // Set boolen selections
    QCOMPARE(boolean_selections.length(), (int) 5);
    custom_cmd_tester->set_cmd_input_radio_test(boolean_selections.at(0));
    custom_cmd_tester->set_keys_in_input_test(boolean_selections.at(1));
    custom_cmd_tester->set_feedback_log_all_cmds_test(boolean_selections.at(2));
    custom_cmd_tester->set_feedback_append_newline_test(boolean_selections.at(3));
    custom_cmd_tester->set_feedback_clear_on_set_test(boolean_selections.at(4));

    // Set rcvd values
    custom_cmd_tester->set_expected_recv_length_test(expected_recv_len);
    foreach (QByteArray rcvd, rcvd_fill_data)
    {
        custom_cmd_tester->receive_gui_test(rcvd);
    }
    QCOMPARE(custom_cmd_tester->get_displayed_feedback_test(), rcvd_expected_display_data);
    QCOMPARE(custom_cmd_tester->rcvd_formatted_readAll_test(), rcvd_expected_file_data);

    // Check if sending
    if (click_send_button)
    {
        // Setup spy to catch tranmit signal
        QList<QVariant> spy_args;
        QSignalSpy transmit_chunk_spy(custom_cmd_tester, custom_cmd_tester->transmit_chunk);
        QVERIFY(transmit_chunk_spy.isValid());

        // Click send
        custom_cmd_tester->click_send_test();

        // Verify key base after send
        QCOMPARE(bases_after_send.length(), (int) 4);
        QCOMPARE(custom_cmd_tester->get_send_key_base_test(), bases_after_send.at(0));
        QCOMPARE(custom_cmd_tester->get_send_cmd_base_test(), bases_after_send.at(1));
        QCOMPARE(custom_cmd_tester->get_recv_key_base_test(), bases_after_send.at(2));
        QCOMPARE(custom_cmd_tester->get_recv_cmd_base_test(), bases_after_send.at(3));

        // Verify spy signal after send
        QCOMPARE(transmit_chunk_spy.count(), send_expected_packets.length());
        foreach (QByteArray expected_send, send_expected_packets)
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

    // Check if reseting with button
    if (click_reset_button)
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
    QTest::addColumn<QString>("send_file_data");
    QTest::addColumn<QString>("send_input_data");
    QTest::addColumn<QList<QByteArray>>("rcvd_fill_data");
    QTest::addColumn<QString>("rcvd_expected_display_data");
    QTest::addColumn<QByteArray>("rcvd_expected_file_data");
    QTest::addColumn<QString>("instructions");

    // Key & Base fill column
    // Ordering as follows:
    //  0) Major Key
    //  1) Minor Key
    //  2) Key Base
    //  3) CMD Base
    QTest::addColumn<QList<QString>>("key_and_base_fills");

    // Base values column
    // Ordering as follows:
    //  0) Send Key Base
    //  1) Send CMD Base
    //  2) Recv Key Base
    //  3) Recv CMD Base
    QTest::addColumn<QList<uint8_t>>("bases_after_send");

    // Send Expected column
    // ByteArray arranged as follows:
    //  0) Major Key
    //  1) Minor Key
    //  2) CMD Base
    //  3-end) Data Packet
    QTest::addColumn<QList<QByteArray>>("send_expected_packets");

    // Boolean selection column
    // Ordering as follows:
    //  0) send_file_radio
    //  1) keys_in_input
    //  2) log_all_cmds
    //  3) append_newline
    //  4) clear_on_set
    QTest::addColumn<QList<bool>>("boolean_selections");

    // Misc columns
    QTest::addColumn<quint32>("expected_recv_len");
    QTest::addColumn<bool>("click_send_button");
    QTest::addColumn<bool>("click_reset_button");

    // Setup helper fields
    QString gui_major_key_str = QString::number(MAJOR_KEY_CUSTOM_CMD, 16);
    QString gui_minor_key_str = QString::number(MINOR_KEY_CUSTOM_CMD_CMD, 16);
    QList<QByteArray> expected_send_list;
    QList<QByteArray> rcvd_list;
    QString expected_feedback;

    // Setup Basic test data
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
    expected_feedback += gui_major_key_str + " ";
    expected_feedback += QString::number(MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE, 16) + " 10 0";
    expected_feedback += gui_major_key_str + " ";
    expected_feedback += gui_minor_key_str + " ";
    expected_feedback += "Feedback_PlainText";

    // Enter basic test:
    //  Sets text in: Filepath, input, and instructions
    //  Checks: File_radio, keys in input, and log all cmds
    //  Calls Receive for: Feedback_PlainText
    QTest::newRow("Basic") \
            << "File_Path" << "Input_PlainText" \
            << rcvd_list << expected_feedback << expected_feedback.toLatin1() \
            << "Instructions" \
            << QList<QString>({"0", "0", "0", "0"}) \
            << QList<uint8_t>() \
            << QList<QByteArray>() \
            << QList<bool>({true, true, true, false, false}) \
            << (quint32) 100 << false << true;

    // Setup data for send loading
    rcvd_list.clear();
    rcvd_list.append("Feedback_PlainText");

    expected_feedback.clear();

    expected_send_list.clear();
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_SET_CMD_BASE,
                     0, 16, 16}));
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_CUSTOM_CMD, MINOR_KEY_CUSTOM_CMD_CMD, 16})
                .append("Input_PlainText"));

    // Load in
    QTest::newRow("Send_test") \
            << "File_Path" << "Input_PlainText" \
            << rcvd_list << expected_feedback << expected_feedback.toLatin1() \
            << "Instructions" \
            << QList<QString>({gui_major_key_str, gui_minor_key_str, "16", "16"}) \
            << QList<uint8_t>({16, 16, 16, 16}) \
            << expected_send_list \
            << QList<bool>({false, false, false, true, true}) \
            << (quint32) 100 << true << true;
}

void GUI_CUSTOM_CMD_TESTS::verify_reset_values()
{
    // Check class memebrs
    QVERIFY(custom_cmd_tester->isClosable());
    QVERIFY(!custom_cmd_tester->acceptAllCMDs());
    QCOMPARE(custom_cmd_tester->get_gui_key(), (uint8_t) MAJOR_KEY_CUSTOM_CMD);
    QCOMPARE(custom_cmd_tester->get_gui_name(), QString("Custom CMD"));
    QCOMPARE(custom_cmd_tester->rcvd_formatted_size_test(), (qint64) 0);
    QCOMPARE(custom_cmd_tester->get_expected_recv_length_test(), (uint32_t) 0);
    QCOMPARE(custom_cmd_tester->get_current_recv_length_test(), (uint32_t) 0);

    // Check encoding keys
    QCOMPARE(custom_cmd_tester->get_major_key_test(), QString::number(MAJOR_KEY_CUSTOM_CMD, 16));
    QCOMPARE(custom_cmd_tester->get_minor_key_test(), QString::number(MINOR_KEY_CUSTOM_CMD_CMD, 16));
    QCOMPARE(custom_cmd_tester->get_key_base_test(), QString("16"));
    QCOMPARE(custom_cmd_tester->get_cmd_base_test(), QString("16"));
    QCOMPARE(custom_cmd_tester->get_send_key_base_test(), (uint8_t) 16);
    QCOMPARE(custom_cmd_tester->get_send_cmd_base_test(), (uint8_t) 16);
    QCOMPARE(custom_cmd_tester->get_recv_key_base_test(), (uint8_t) 16);
    QCOMPARE(custom_cmd_tester->get_recv_cmd_base_test(), (uint8_t) 16);

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
