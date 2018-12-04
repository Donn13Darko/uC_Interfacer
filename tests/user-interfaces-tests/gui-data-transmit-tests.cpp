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

#include "gui-data-transmit-tests.hpp"

// Testing infrastructure includes
#include <QtTest>
#include <QSignalSpy>

#include "../../src/gui-helpers/gui-generic-helper.hpp"

GUI_DATA_TRANSMIT_TESTS::GUI_DATA_TRANSMIT_TESTS()
{
    /* DO NOTHING */
}

GUI_DATA_TRANSMIT_TESTS::~GUI_DATA_TRANSMIT_TESTS()
{
    // Delete tester if allocated
    if (data_transmit_tester) delete data_transmit_tester;
}

void GUI_DATA_TRANSMIT_TESTS::initTestCase()
{
    // Create object for testing
    data_transmit_tester = new GUI_DATA_TRANSMIT_TEST_CLASS();
    QVERIFY(data_transmit_tester);
}

void GUI_DATA_TRANSMIT_TESTS::cleanupTestCase()
{
    // Delete test class
    if (data_transmit_tester)
    {
        delete data_transmit_tester;
        data_transmit_tester = nullptr;
    }
}

void GUI_DATA_TRANSMIT_TESTS::test_init_vals()
{
    // Verify reset defaults
    verify_reset_values();

    // Verify non-reset & defualt members
    QVERIFY(data_transmit_tester->isClosable());
    QCOMPARE(data_transmit_tester->get_gui_tab_name(), data_transmit_tester->get_gui_name());
    QCOMPARE(data_transmit_tester->get_gui_config(),
             data_transmit_tester->get_gui_name().prepend("[").append("]\n"));
}

void GUI_DATA_TRANSMIT_TESTS::test_basic_features()
{
    // Test waitForDevice
    QVERIFY(!data_transmit_tester->waitForDevice(0));
    QVERIFY(data_transmit_tester->waitForDevice(MINOR_KEY_DATA_TRANSMIT_DATA));
}

void GUI_DATA_TRANSMIT_TESTS::test_gui_config()
{
    // Fetch data
    QFETCH(QString, config_str);
    QFETCH(QString, gui_tab_name);
    QFETCH(bool, isClosable);

    // Clear current config
    QMap<QString, QVariant> reset_map;
    data_transmit_tester->parseConfigMap(&reset_map);

    // Get gui values
    uint8_t curr_gui_key = data_transmit_tester->get_gui_key();
    QString curr_gui_name = data_transmit_tester->get_gui_name();

    // Generate new config
    CONFIG_MAP *gui_config = \
            GUI_GENERIC_HELPER::decode_configMap(config_str);

    // Parse new config
    data_transmit_tester->parseConfigMap(gui_config->value(curr_gui_name, nullptr));

    // Check values
    QCOMPARE(data_transmit_tester->get_gui_key(), curr_gui_key);
    QCOMPARE(data_transmit_tester->get_gui_name(), curr_gui_name);
    QCOMPARE(data_transmit_tester->get_gui_tab_name(), gui_tab_name);
    QCOMPARE(data_transmit_tester->isClosable(), isClosable);
}

void GUI_DATA_TRANSMIT_TESTS::test_gui_config_data()
{
    // Setup data columns
    QTest::addColumn<QString>("config_str");
    QTest::addColumn<QString>("gui_tab_name");
    QTest::addColumn<bool>("isClosable");

    // Helper variables
    QString config_str;
    QString curr_gui_name = data_transmit_tester->get_gui_name();

    // Setup basic config str
    config_str.clear();
    config_str += "[" + curr_gui_name + "]\n";
    config_str += "tab_name=\"Tab A\"\n";
    config_str += "closable=\"false\"\n";

    // Load in basic
    QTest::newRow("Basic") << config_str \
                           << "Tab A" \
                           << false;

    // Setup RESET config str
    config_str.clear();
    config_str += "[" + curr_gui_name + "]\n";

    // Load in RESET
    QTest::newRow("RESET") << config_str \
                           << curr_gui_name \
                           << true;
}

void GUI_DATA_TRANSMIT_TESTS::test_send()
{
    // Fetch data
    QFETCH(QString, send_fill_data);
    QFETCH(QList<QByteArray>, send_expected_signals);
    QFETCH(bool, send_file);

    // Reset the gui before (direct call slot)
    data_transmit_tester->reset_gui();

    // Setup & perform send
    perform_data_send(send_fill_data, send_file,
                     true, true,
                     send_expected_signals);
}

void GUI_DATA_TRANSMIT_TESTS::test_send_data()
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

    // Add misc columns
    QTest::addColumn<bool>("send_file");

    // Setup helper variables
    QString input_data;
    QString gui_major_key_str = QString::number(MAJOR_KEY_DATA_TRANSMIT, 16);
    QString gui_minor_key_str = QString::number(MINOR_KEY_DATA_TRANSMIT_DATA, 16);
    QList<QByteArray> expected_send_list;

    // Setup Simple Send Empty Input test data (no data)
    input_data.clear();
    expected_send_list.clear();
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_DATA_TRANSMIT, MINOR_KEY_DATA_TRANSMIT_SET_TRANS_SIZE,
                     0, 0, 0, 0, 0}));
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_DATA_TRANSMIT, MINOR_KEY_DATA_TRANSMIT_DATA,
                     0}));

    // Enter Simple Send Empty Input test:
    //  send_fill_data = ""
    //  send_expected_signals = {"4 1 0 0 0 0", "4 2 "}
    //  send_file = false
    QTest::newRow("Simple Send Empty Input") \
            << input_data \
            << expected_send_list \
            << false;

    // Setup Simple Send Empty File test data (no data)
    input_data.clear();
    expected_send_list.clear();
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_DATA_TRANSMIT, MINOR_KEY_DATA_TRANSMIT_SET_TRANS_SIZE,
                     0, 0, 0, 0, 0}));
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_DATA_TRANSMIT, MINOR_KEY_DATA_TRANSMIT_DATA,
                     0}));

    // Enter Simple Send Empty File test:
    //  send_fill_data = ""
    //  send_expected_signals = {"4 1 0 0 0 0", "4 2 **TEMP_FNAME**"}
    //  send_file = true
    QTest::newRow("Simple Send Empty File") \
            << input_data \
            << expected_send_list \
            << true;

    // Setup Simple Send Input test data (no data)
    input_data.clear();
    input_data += "Hello World!";

    expected_send_list.clear();
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_DATA_TRANSMIT, MINOR_KEY_DATA_TRANSMIT_SET_TRANS_SIZE,
                     0, 0, 0, 0, 13}));
    expected_send_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_DATA_TRANSMIT, MINOR_KEY_DATA_TRANSMIT_DATA,
                     0})
                .append("Hello World!\n"));

    // Enter Simple Send Input test:
    //  send_fill_data = "Hello World!\n"
    //  send_expected_signals = {"4 1 0 0 0 0", "4 2 Hello World!\n"}
    //  send_file = false
    QTest::newRow("Simple Send Input") \
            << input_data \
            << expected_send_list \
            << false;
}

void GUI_DATA_TRANSMIT_TESTS::test_rcvd()
{
    // Fetch data
    QFETCH(QList<QByteArray>, rcvd_fill_data);
    QFETCH(QString, rcvd_expected_display_data);
    QFETCH(QByteArray, rcvd_expected_file_data);
    QFETCH(QList<bool>, click_buttons);

    // Reset the gui before (direct call slot)
    data_transmit_tester->reset_gui();

    // Verify field lengths
    QCOMPARE(click_buttons.length(), (int) 4);

    // Setup & perform rcvd
    perform_data_rcvd(rcvd_fill_data,
                      click_buttons.at(0), click_buttons.at(1),
                      click_buttons.at(2),
                      click_buttons.at(3), true,
                      rcvd_expected_display_data,
                      rcvd_expected_file_data);
}

void GUI_DATA_TRANSMIT_TESTS::test_rcvd_data()
{
    // Input data columns
    QTest::addColumn<QList<QByteArray>>("rcvd_fill_data");
    QTest::addColumn<QString>("rcvd_expected_display_data");
    QTest::addColumn<QByteArray>("rcvd_expected_file_data");

    // Boolean selection column
    // Ordering as follows:
    //  0) check_display_recv
    //  1) check_clear_on_set
    //  2) click_clear
    //  3) click_save
    QTest::addColumn<QList<bool>>("click_buttons");

    // Setup helper variables
    QString gui_major_key_str = QString::number(MAJOR_KEY_DATA_TRANSMIT, 16);
    QString gui_minor_key_str = QString::number(MINOR_KEY_DATA_TRANSMIT_DATA, 16);
    QList<QByteArray> rcvd_list;
    QString expected_feedback;

    // Setup basic test data (everything empty)
    rcvd_list.clear();
    expected_feedback.clear();

    // Load basic test data
    //  rcvd_fill_data = {}
    //  rcvd_expected_display_data = ""
    //  rcvd_expected_file_data = ""
    //  click_buttons = {display_recv, clear_on_set, click_clear, click_save}
    //                = {true, false, true, true}
    QTest::newRow("Basic Recv Verify") \
            << rcvd_list << expected_feedback << expected_feedback.toLatin1() \
            << QList<bool>({true, false, true, true});

    // Setup basic transmit test data
    rcvd_list.clear();
    rcvd_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_DATA_TRANSMIT, MINOR_KEY_DATA_TRANSMIT_DATA})
                .append("Hello "));
    rcvd_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_DATA_TRANSMIT, MINOR_KEY_DATA_TRANSMIT_DATA})
                .append("World!"));

    expected_feedback.clear();
    expected_feedback += "Hello World!";

    // Load basic transmit test data
    //  rcvd_fill_data = {"Hello ", "World!"}
    //  rcvd_expected_display_data = "Hello World!"
    //  rcvd_expected_file_data = "Hello World!"
    //  click_buttons = {display_recv, clear_on_set, click_clear, click_save}
    //                = {true, false, true, true}
    QTest::newRow("Basic Transmit Packets") \
            << rcvd_list << expected_feedback << expected_feedback.toLatin1() \
            << QList<bool>({true, false, true, true});

    // Setup test trans size test data
    rcvd_list.clear();
    rcvd_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_DATA_TRANSMIT, MINOR_KEY_DATA_TRANSMIT_SET_TRANS_SIZE,
                    100}));
    rcvd_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_DATA_TRANSMIT, MINOR_KEY_DATA_TRANSMIT_DATA})
                .append("Test set trans size"));

    expected_feedback.clear();
    expected_feedback += "Test set trans size";

    // Load set trans size test data
    //  rcvd_fill_data = {"Test set trans size"}
    //  rcvd_expected_display_data = "Test set trans size"
    //  rcvd_expected_file_data = "Test set trans size"
    //  click_buttons = {display_recv, clear_on_set, click_clear, click_save}
    //                = {true, false, true, true}
    QTest::newRow("Set Trans Size") \
            << rcvd_list << expected_feedback << expected_feedback.toLatin1() \
            << QList<bool>({true, false, true, true});

    // Setup test trans size test data
    rcvd_list.clear();
    rcvd_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_DATA_TRANSMIT, MINOR_KEY_DATA_TRANSMIT_SET_TRANS_SIZE,
                    24}));
    rcvd_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_DATA_TRANSMIT, MINOR_KEY_DATA_TRANSMIT_DATA})
                .append("Test progress bar."));
    rcvd_list.append(
                GUI_GENERIC_HELPER::qList_to_byteArray(
                    {MAJOR_KEY_DATA_TRANSMIT, MINOR_KEY_DATA_TRANSMIT_DATA})
                .append(" Done!"));

    expected_feedback.clear();
    expected_feedback += "Test progress bar. Done!";

    // Load complete transmit test data
    //  rcvd_fill_data = {"Test progress bar.", " Done!"}
    //  rcvd_expected_display_data = "Test progress bar. Done!"
    //  rcvd_expected_file_data = "Test progress bar. Done!"
    //  click_buttons = {display_recv, clear_on_set, click_clear, click_save}
    //                = {true, false, true, true}
    QTest::newRow("Complete Transmit") \
            << rcvd_list << expected_feedback << expected_feedback.toLatin1() \
            << QList<bool>({true, true, true, true});
}

void GUI_DATA_TRANSMIT_TESTS::test_complex_transmit()
{
    // Fetch data
    QFETCH(QString, send_fill_data);
    QFETCH(QList<QByteArray>, rcvd_fill_data);
    QFETCH(QString, rcvd_expected_display_data);
    QFETCH(QByteArray, rcvd_expected_file_data);
    QFETCH(QList<QByteArray>, send_expected_signals);
    QFETCH(QList<bool>, click_buttons);
    QFETCH(bool, check_send);
    QFETCH(bool, check_rcvd);

    // Reset the gui before (direct call slot)
    data_transmit_tester->reset_gui();

    // Verify field lengths
    QCOMPARE(click_buttons.length(), (int) 7);

    // Setup & perform rcvd
    perform_data_rcvd(rcvd_fill_data,
                      click_buttons.at(2), click_buttons.at(3),
                      click_buttons.at(4),
                      click_buttons.at(5), check_rcvd,
                      rcvd_expected_display_data,
                      rcvd_expected_file_data);

    // Setup & perform send
    perform_data_send(send_fill_data, click_buttons.at(0),
                      click_buttons.at(1), check_send,
                      send_expected_signals);

    // Check if reseting with button
    if (click_buttons.at(6))
    {
        // Send Reset
        data_transmit_tester->reset_clicked_test();

        // Verify reset values
        verify_reset_values();
    }
}

void GUI_DATA_TRANSMIT_TESTS::test_complex_transmit_data()
{
    // Input data columns
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

    // Click buttons column
    // Ordering as follows:
    //  0) check_send_file_radio
    //  1) click_send
    //  2) check_display_recv
    //  3) check_clear_on_set
    //  4) click_clear
    //  5) click_save
    //  6) click_reset
    QTest::addColumn<QList<bool>>("click_buttons");

    // Misc columns
    QTest::addColumn<bool>("check_send");
    QTest::addColumn<bool>("check_rcvd");

    // Setup helper variables
    QString gui_major_key_str = QString::number(MAJOR_KEY_DATA_TRANSMIT, 16);
    QString gui_minor_key_str = QString::number(MINOR_KEY_DATA_TRANSMIT_DATA, 16);
    QList<QByteArray> expected_send_list;
    QList<QByteArray> rcvd_list;
    QString expected_feedback;

    // Setup basic test data (everything empty)
    rcvd_list.clear();
    expected_feedback.clear();
    expected_send_list.clear();

    // Load basic test data:
    //  send_fill_data = "File_Path"
    //  rcvd_fill_data = {}
    //  rcvd_expected_display_data = ""
    //  rcvd_expected_file_data = ""
    //  send_expected_signals = {}
    //  click_buttons = {send_file_radio, click_send,
    //                      check_display_recv, clear_on_set,
    //                      click_clear, click_save, click_reset}
    //                = {true, false,
    //                      true, false,
    //                      false, false, true}
    //  check_send = false, check_rcvd = false
    QTest::newRow("Basic Complex Verify") \
            << "File_Path" \
            << rcvd_list << expected_feedback << expected_feedback.toLatin1() \
            << expected_send_list \
            << QList<bool>({true, false,
                            true, false,
                            false, false, true}) \
            << false << false;
}

void GUI_DATA_TRANSMIT_TESTS::verify_reset_values()
{
    // Check class memebrs
    QCOMPARE(data_transmit_tester->get_gui_key(), (uint8_t) MAJOR_KEY_DATA_TRANSMIT);
    QCOMPARE(data_transmit_tester->get_gui_name(), QString("Data Transmit"));
    QCOMPARE(data_transmit_tester->rcvd_formatted_size_test(), (qint64) 0);

    // Check input and recv
    QCOMPARE(data_transmit_tester->get_user_input_text_test(), QString(""));
    QCOMPARE(data_transmit_tester->get_file_input_text_test(), QString(""));
    QCOMPARE(data_transmit_tester->get_displayed_recv_test(), QString(""));

    // Check progress bars
    QCOMPARE(data_transmit_tester->get_progress_update_recv_value_test(), (int) 0);
    QCOMPARE(data_transmit_tester->get_progress_update_recv_string_test(), QString(""));
    QCOMPARE(data_transmit_tester->get_progress_update_send_value_test(), (int) 0);
    QCOMPARE(data_transmit_tester->get_progress_update_send_string_test(), QString(""));

    // Check check boxes
    QCOMPARE(data_transmit_tester->get_data_input_radio_test(), true);
    QCOMPARE(data_transmit_tester->get_show_recv_data_test(), false);
    QCOMPARE(data_transmit_tester->get_recv_clear_on_set_test(), true);
}

void GUI_DATA_TRANSMIT_TESTS::perform_data_rcvd(QList<QByteArray> rcvd_fill_data,
                                                bool display_recv, bool clear_on_set,
                                                bool click_clear_button,
                                                bool click_save_button, bool check_rcvd,
                                                QString rcvd_expected_display_data,
                                                QByteArray rcvd_expected_file_data)
{
    // Set boolen selections
    data_transmit_tester->set_show_recv_data_test(display_recv);
    data_transmit_tester->set_recv_clear_on_set_test(clear_on_set);

    // Setup spy to catch tranmit signal
    QList<QVariant> spy_args;
    QSignalSpy progress_spy(data_transmit_tester, data_transmit_tester->progress_update_recv);
    QVERIFY(progress_spy.isValid());

    // Load in rcvd values (emit readyRead() signal for tester)
    int progress_val;
    QString expected_recv_len_str, progress_str;
    uint32_t expected_recv_len = 0, target_recv = 0;
    foreach (QByteArray rcvd, rcvd_fill_data)
    {
        // Send command
        emit data_transmit_tester->readyRead(rcvd);
        qApp->processEvents();

        // Check if major key is gui key (progress bar updates)
        if (rcvd.at(s1_major_key_loc) == data_transmit_tester->get_gui_key())
        {
            // Process extra events since gui relevant
            // (required for progress_bar updates)
            qApp->processEvents();

            // Check minor key
            switch (rcvd.at(s1_minor_key_loc))
            {
                case MINOR_KEY_DATA_TRANSMIT_SET_TRANS_SIZE:
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
                    QCOMPARE(data_transmit_tester->get_progress_update_recv_value_test(), 0);
                    QCOMPARE(data_transmit_tester->get_progress_update_recv_string_test(), QString(""));
                    break;
                }
                case MINOR_KEY_DATA_TRANSMIT_DATA:
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
                            QCOMPARE(data_transmit_tester->get_progress_update_recv_value_test(), 100);
                            QCOMPARE(data_transmit_tester->get_progress_update_recv_string_test(), QString("Done!"));
                        } else
                        {
                            // Get values
                            progress_val = qRound(((float) target_recv / expected_recv_len) * 100.0f);
                            progress_str = QString::number((float) target_recv / 1000.0f) + expected_recv_len_str;

                            // Verify increment string
                            QCOMPARE(spy_args.at(0).toInt(), progress_val);
                            QCOMPARE(spy_args.at(1).toString(), progress_str);
                            QCOMPARE(data_transmit_tester->get_progress_update_recv_value_test(), progress_val);
                            QCOMPARE(data_transmit_tester->get_progress_update_recv_string_test(), progress_str);
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
        QCOMPARE(data_transmit_tester->get_displayed_recv_test(), rcvd_expected_display_data);
        QCOMPARE(data_transmit_tester->rcvd_formatted_readAll_test(), rcvd_expected_file_data);
    } else if (click_save_button)
    {
        // Can't manually click button (blocking pop-up dialog)
        // Button slot simply calls rcvd_formatted_save() which is tested in gui-base-tests.cpp
        QCOMPARE(data_transmit_tester->rcvd_formatted_readAll_test(), rcvd_expected_file_data);
    }

    // Check if clearing
    if (click_clear_button)
    {
        // Click clear button
        data_transmit_tester->recv_clear_clicked_test();

        // Verify clear
        QCOMPARE(data_transmit_tester->rcvd_formatted_size_test(), (int) 0);
        QCOMPARE(data_transmit_tester->rcvd_formatted_readAll_test(), QByteArray());
        QCOMPARE(data_transmit_tester->get_displayed_recv_test(), QString(""));

        // Verify set signal
        QCOMPARE(progress_spy.count(), 1);
        spy_args = progress_spy.takeFirst();
        QCOMPARE(spy_args.at(0).toInt(), 0);
        QCOMPARE(spy_args.at(1).toString(), QString(""));
    }
}

void GUI_DATA_TRANSMIT_TESTS::perform_data_send(QString send_fill_data, bool send_file_radio,
                                                bool click_send, bool check_send,
                                                QList<QByteArray> send_expected_signals)
{
    // Create temp file to store data in
    QTemporaryFile temp_file;
    temp_file.setAutoRemove(true);

    // Set input info
    data_transmit_tester->set_data_input_radio_test(send_file_radio);
    if (send_file_radio)
    {
        // Load in data
        QVERIFY(temp_file.open());
        temp_file.write(send_fill_data.toLatin1());
        temp_file.close();

        // Set file text
        data_transmit_tester->set_file_input_text_test(temp_file.fileName());
    } else
    {
        // Set input text
        data_transmit_tester->set_user_input_text_test(send_fill_data);
    }

    // Setup spy to catch tranmit signal
    QList<QVariant> spy_args;
    QSignalSpy transmit_chunk_spy(data_transmit_tester, data_transmit_tester->transmit_chunk);
    QSignalSpy transmit_chunk_pack_spy(data_transmit_tester, data_transmit_tester->transmit_chunk_pack);
    QSignalSpy transmit_file_pack_spy(data_transmit_tester, data_transmit_tester->transmit_file_pack);
    QVERIFY(transmit_chunk_spy.isValid());
    QVERIFY(transmit_chunk_pack_spy.isValid());
    QVERIFY(transmit_file_pack_spy.isValid());

    // See if clicking send
    if (click_send)
    {
        // Click send
        data_transmit_tester->send_clicked_test();
        qApp->processEvents();
    }

    // If checking, Verify spy signals after send
    if (check_send)
    {
        // Verify num signals equal to expected
        QCOMPARE(send_expected_signals.length(), (int) 2);

        // Send byte array holder
        QByteArray expected_send = send_expected_signals.takeFirst();
        QVERIFY(4 <= expected_send.length());

        // Get transmit_chunk signal
        QCOMPARE(transmit_chunk_spy.count(), (int) 1);
        spy_args = transmit_chunk_spy.takeFirst();

        // Verify transmit_chunk signal
        QCOMPARE(spy_args.at(0).toInt(), (int) expected_send.at(0));
        QCOMPARE(spy_args.at(1).toInt(), (int) expected_send.at(1));
        QCOMPARE(spy_args.at(3).toInt(), (int) expected_send.at(2));
        QCOMPARE(spy_args.at(2).toByteArray(), expected_send.mid(3));

        // Get next expected
        expected_send = send_expected_signals.takeFirst();
        QVERIFY(3 <= expected_send.length());

        // Get file or chunk pack signal
        if (send_file_radio)
        {
            QCOMPARE(transmit_file_pack_spy.count(), (int) 1);
            spy_args = transmit_file_pack_spy.takeFirst();
            QCOMPARE(spy_args.at(2).toByteArray(), temp_file.fileName().toLatin1());
        } else
        {
            QCOMPARE(transmit_chunk_pack_spy.count(), (int) 1);
            spy_args = transmit_chunk_pack_spy.takeFirst();
            QCOMPARE(spy_args.at(2).toByteArray(), expected_send.mid(3));
        }

        // Verify file or chunk pack signal
        QCOMPARE(spy_args.at(0).toInt(), (int) expected_send.at(0));
        QCOMPARE(spy_args.at(1).toInt(), (int) expected_send.at(1));
        QCOMPARE(spy_args.at(3).toInt(), (int) expected_send.at(2));
    }
}
