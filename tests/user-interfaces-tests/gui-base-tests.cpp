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
#include <QSignalSpy>

// Object includes
#include <QFile>
#include <QTemporaryFile>
#include <QMessageBox>

#include "../../src/gui-helpers/gui-generic-helper.hpp"

GUI_BASE_TESTS::GUI_BASE_TESTS()
{
    /* DO NOTHING */
}

GUI_BASE_TESTS::~GUI_BASE_TESTS()
{
    // Delete tester if allocated
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

void GUI_BASE_TESTS::test_init_vals()
{
    // Verify all init values
    QVERIFY(base_tester->isClosable());
    QCOMPARE(base_tester->get_gui_key(), (uint8_t) MAJOR_KEY_ERROR);
    QCOMPARE(base_tester->get_gui_name(), QString("GUI Base"));
    QCOMPARE(base_tester->get_gui_tab_name(), base_tester->get_gui_name());
    QCOMPARE(base_tester->get_gui_config(),
             base_tester->get_gui_name().prepend("[").append("]\n"));
}

void GUI_BASE_TESTS::test_basic_features()
{
    // Test toggle closable settings
    QVERIFY(base_tester->isClosable());
    base_tester->setClosable(false);
    QVERIFY(!base_tester->isClosable());
    base_tester->setClosable(true);
    QVERIFY(base_tester->isClosable());

    // Test accept all cmds (should return false)
    QVERIFY(!base_tester->acceptAllCMDs());

    // Test waitForDevice (should always return false)
    QVERIFY(!base_tester->waitForDevice(0));
    QVERIFY(!base_tester->waitForDevice((uint8_t) qrand()));

    // Test getting and setting gui_map values
    base_tester->set_gui_map_value_test("tab_name", QVariant("Red"));
    QCOMPARE(base_tester->get_gui_tab_name(), QString("Red"));
    QCOMPARE(base_tester->get_gui_map_value("tab_name").toString(), QString("Red"));
    base_tester->set_gui_map_value_test("closable", QVariant("false"));
    QCOMPARE(base_tester->isClosable(), false);
    QCOMPARE(base_tester->get_gui_map_value("closable").toBool(), false);

    // Verify readyRead & receive connection (does nothing in base)
    emit base_tester->readyRead(GUI_GENERIC_HELPER::qList_to_byteArray({0, 1, 2, 3}));
    qApp->processEvents();

    // Clear map
    QMap<QString, QVariant> reset_map;
    base_tester->parseConfigMap(&reset_map);
}

void GUI_BASE_TESTS::test_gui_key()
{
    // Fetch keys
    QFETCH(quint8, set_key);

    // Set and check result
    base_tester->set_gui_key_test(set_key);
    QCOMPARE(base_tester->get_gui_key(), set_key);
}

void GUI_BASE_TESTS::test_gui_key_data()
{
    // Setup data columns
    QTest::addColumn<quint8>("set_key");

    // Load in data
    QTest::newRow("Major Error Key") << (quint8) MAJOR_KEY_ERROR;
    QTest::newRow("General Settings Key") << (quint8) MAJOR_KEY_GENERAL_SETTINGS;
    QTest::newRow("IO Key") << (quint8) MAJOR_KEY_IO;
    QTest::newRow("Custom CMD Key") << (quint8) MAJOR_KEY_CUSTOM_CMD;
    QTest::newRow("RESET") << (quint8) base_tester->get_gui_key();
}

void GUI_BASE_TESTS::test_set_gui_name()
{
    // Fetch name
    QFETCH(QString, set_name);
    QFETCH(QString, expected_name);

    // Set name
    base_tester->set_gui_name_test(set_name);

    // Verify name
    QCOMPARE(base_tester->get_gui_name(), expected_name);
}

void GUI_BASE_TESTS::test_set_gui_name_data()
{
    // Setup data columns
    QTest::addColumn<QString>("set_name");
    QTest::addColumn<QString>("expected_name");

    // Helper variable
    QString curr_name = base_tester->get_gui_name();

    // Load in data
    QTest::newRow("Same") << curr_name << curr_name;
    QTest::newRow("Blank") << "" << curr_name;
    QTest::newRow("Short") << "IO" << "IO";
    QTest::newRow("Simple") << "Welcome" << "Welcome";
    QTest::newRow("Space") << "Data Transmit" << "Data Transmit";
    QTest::newRow("Random Caps") << "RandOM GLESN" << "RandOM GLESN";
    QTest::newRow("Random Chars") << "'-1923'/,.l][qw2487923-1!(*@!*$@!_" \
                                  << "'-1923'/,.l][qw2487923-1!(*@!*$@!_";
    QTest::newRow("RESET") << curr_name << curr_name;
}

void GUI_BASE_TESTS::test_set_gui_tab_name()
{
    // Fetch name
    QFETCH(QString, set_name);
    QFETCH(QString, expected_name);

    // Set and check result
    base_tester->set_gui_tab_name(set_name);
    QCOMPARE(base_tester->get_gui_tab_name(), expected_name);
}

void GUI_BASE_TESTS::test_set_gui_tab_name_data()
{
    // Setup data columns
    QTest::addColumn<QString>("set_name");
    QTest::addColumn<QString>("expected_name");

    // Helper variable
    QString curr_name = base_tester->get_gui_tab_name();

    // Load in data
    QTest::newRow("Same") << curr_name << curr_name;
    QTest::newRow("Blank") << "" << "";
    QTest::newRow("Simple") << "New Tab" << "New Tab";
    QTest::newRow("Random Caps") << "RandOM GLESN" << "RandOM GLESN";
    QTest::newRow("Random Chars") << "'-1923'/,.l][qw2487923-1!(*@!*$@!_" \
                                  << "'-1923'/,.l][qw2487923-1!(*@!*$@!_";
    QTest::newRow("RESET") << curr_name << curr_name;
}

void GUI_BASE_TESTS::test_gui_config_1()
{
    // Get current config string
    QString config_str_before = base_tester->get_gui_config();

    // Check handling of null config maps (no changes)
    base_tester->parseConfigMap(nullptr);
    QCOMPARE(base_tester->get_gui_config(), config_str_before);

    // Setup empty variables
    CONFIG_MAP empty_gui_config;
    QMap<QString, QVariant> empty_gui_map;
    empty_gui_config.insert(base_tester->get_gui_name(), &empty_gui_map);
    QString expected_config_str = GUI_GENERIC_HELPER::encode_configMap(&empty_gui_config);

    // Check handling of empty config maps (clears all)
    base_tester->parseConfigMap(&empty_gui_map);
    QCOMPARE(base_tester->get_gui_config(), expected_config_str);
}

void GUI_BASE_TESTS::test_gui_config_2()
{
    // Set tab name before change
    QString t_name = "This is a tab";
    base_tester->set_gui_tab_name(t_name);
    QCOMPARE(base_tester->get_gui_tab_name(), t_name);

    // Build config string
    // String is set to how GUI_GENERIC_HELPER::encode_configMap() would
    // generate it from a map object as get_gui_config() calls this
    QString config_str = "[";
    config_str += base_tester->get_gui_name() + "]\n";
    config_str += "tab_name=\"\"\n";

    // Create config map
    QMap<QString, QMap<QString, QVariant>*> *config_map = \
            GUI_GENERIC_HELPER::decode_configMap(config_str);

    // Set config map
    base_tester->parseConfigMap(config_map->value(base_tester->get_gui_name()));

    // Delete config map
    GUI_GENERIC_HELPER::delete_configMap(&config_map);

    // Verify changes (tab name cleared)
    QCOMPARE(base_tester->get_gui_config(), config_str);
    QCOMPARE(base_tester->get_gui_tab_name(), QString(""));
}

void GUI_BASE_TESTS::test_gui_config()
{
    // Fetch data
    QFETCH(QString, config_str);
    QFETCH(QString, gui_tab_name);
    QFETCH(bool, isClosable);

    // Clear current config
    QMap<QString, QVariant> reset_map;
    base_tester->parseConfigMap(&reset_map);

    // Get unchanging values
    uint8_t curr_gui_key = base_tester->get_gui_key();
    QString curr_gui_name = base_tester->get_gui_name();

    // Generate new config
    CONFIG_MAP *gui_config = \
            GUI_GENERIC_HELPER::decode_configMap(config_str);

    // Parse new config
    base_tester->parseConfigMap(gui_config->value(curr_gui_name, nullptr));

    // Check values
    QCOMPARE(base_tester->get_gui_key(), curr_gui_key);
    QCOMPARE(base_tester->get_gui_name(), curr_gui_name);
    QCOMPARE(base_tester->get_gui_tab_name(), gui_tab_name);
    QCOMPARE(base_tester->isClosable(), isClosable);
}

void GUI_BASE_TESTS::test_gui_config_data()
{
    // Setup data columns
    QTest::addColumn<QString>("config_str");
    QTest::addColumn<QString>("gui_tab_name");
    QTest::addColumn<bool>("isClosable");

    // Helper variables
    QString config_str;
    QString curr_gui_name = base_tester->get_gui_name();

    // Setup defaults config str
    config_str.clear();
    config_str += "[" + curr_gui_name + "]\n";

    // Load in defaults
    QTest::newRow("Empty") << config_str \
                           << curr_gui_name \
                           << true;

    // Setup only tab name config str
    config_str.clear();
    config_str += "[" + curr_gui_name + "]\n";
    config_str += "tab_name=\"Only Tab\"\n";

    // Load in only tab name
    QTest::newRow("Only Tab Name") << config_str \
                                    << "Only Tab" \
                                    << true;

    // Setup only closable config str
    config_str.clear();
    config_str += "[" + curr_gui_name + "]\n";
    config_str += "closable=\"false\"\n";

    // Load in only tab name
    QTest::newRow("Only Closable") << config_str \
                                    << curr_gui_name \
                                    << false;

    // Setup basic config str
    config_str.clear();
    config_str += "[" + curr_gui_name + "]\n";
    config_str += "tab_name=\"Tab A\"\n";
    config_str += "closable=\"false\"\n";

    // Load in basic
    QTest::newRow("Basic") << config_str \
                           << "Tab A" \
                           << false;

    // Setup empty tab name config str
    config_str.clear();
    config_str += "[" + curr_gui_name + "]\n";
    config_str += "tab_name=\"\"\n";
    config_str += "closable=\"true\"\n";

    // Load in empty tab name
    QTest::newRow("Empty Tab Name") << config_str \
                                    << "" \
                                    << true;

    // Setup incorrect group name config str
    config_str.clear();
    config_str += "[Bad_Name]\n";
    config_str += "tab_name=\"\"\n";
    config_str += "closable=\"false\"\n";

    // Load in incorrect group name
    QTest::newRow("Incorrect Group Name") << config_str \
                                          << curr_gui_name \
                                          << true;
}

void GUI_BASE_TESTS::test_recv_length()
{
    // Fetch data
    QFETCH(quint32, expected_recv_len);
    QFETCH(QList<quint32>, recv_len_data);

    // Setup signal spy
    QList<QVariant> spy_args;
    QSignalSpy progress_spy(base_tester, base_tester->progress_update_recv);
    QVERIFY(progress_spy.isValid());

    // Set start info
    base_tester->set_expected_recv_length_test(expected_recv_len);
    qApp->processEvents();

    // Verify set signal
    QCOMPARE(progress_spy.count(), (int) 1);
    spy_args = progress_spy.takeFirst();
    QCOMPARE(spy_args.at(0).toInt(), (int) 0);
    QCOMPARE(spy_args.at(1).toString(), QString(""));

    // Send packets & verify signals
    bool recv_len_done = false;
    quint32 target_recv = 0;
    QString expected_recv_len_str = "/" + QString::number(expected_recv_len / 1000.0f) + "KB";
    foreach (quint32 recv_len, recv_len_data)
    {
        // Send update to base class
        base_tester->update_current_recv_length_test(recv_len);

        // Continue if already received expected_recv_len
        if (recv_len_done)
        {
            QCOMPARE(progress_spy.count(), (int) 0);
            continue;
        }

        // Increment target
        target_recv += recv_len;

        // Verify signal
        if (!expected_recv_len || (expected_recv_len <= target_recv))
        {
            // Verify update signal
            if (expected_recv_len && (expected_recv_len <= target_recv))
            {
                // Verify progress signal
                QCOMPARE(progress_spy.count(), (int) 1);
                spy_args = progress_spy.takeFirst();
                QCOMPARE(spy_args.at(0).toInt(), (int) 100);
                QCOMPARE(spy_args.at(1).toString(), QString("Done!"));
            } else
            {
                QCOMPARE(progress_spy.count(), (int) 0);
            }

            // Clear expected values
            target_recv = 0;
            expected_recv_len = 0;
            expected_recv_len_str.clear();

            // Set to stop incrementing
            recv_len_done = true;
        } else if (recv_len)
        {
            // Verify progress signal
            QCOMPARE(progress_spy.count(), (int) 1);
            spy_args = progress_spy.takeFirst();
            QCOMPARE(spy_args.at(0).toInt(), qRound(((float) target_recv / expected_recv_len) * 100.0f));
            QCOMPARE(spy_args.at(1).toString(), QString::number((float) target_recv / 1000.0f) + expected_recv_len_str);
        }
    }
}

void GUI_BASE_TESTS::test_recv_length_data()
{
    // Setup data columns
    QTest::addColumn<quint32>("expected_recv_len");
    QTest::addColumn<QList<quint32>>("recv_len_data");

    // Load in data
    QTest::newRow("E0_S0") << (quint32) 0x00 << QList<quint32>({0x00});
    QTest::newRow("E0_S1") << (quint32) 0x00 << QList<quint32>({0x01});
    QTest::newRow("E1_S0") << (quint32) 0x01 << QList<quint32>({0x00});
    QTest::newRow("E1_S1 Start") << (quint32) 0x01 << QList<quint32>({0x01, 0x00, 0x00});
    QTest::newRow("E1_S1 Middle") << (quint32) 0x01 << QList<quint32>({0x00, 0x01, 0x00});
    QTest::newRow("E1_S1 End") << (quint32) 0x01 << QList<quint32>({0x00, 0x00, 0x01});
    QTest::newRow("E1_S3 V1") << (quint32) 0x01 << QList<quint32>({0x03});
    QTest::newRow("E1_S3 V2") << (quint32) 0x01 << QList<quint32>({0x01, 0x02});
    QTest::newRow("E1_S3 V3") << (quint32) 0x01 << QList<quint32>({0x01, 0x00, 0x02});
    QTest::newRow("E5_S6 V1") << (quint32) 0x05 << QList<quint32>({0x00, 0x01, 0x02, 0x03});
    QTest::newRow("E500_S0") << (quint32) 0x01F4 << QList<quint32>({0x00});
    QTest::newRow("E500_S500") << (quint32) 0x01F4 << QList<quint32>({0x01F4});
    QTest::newRow("E500_S5") << (quint32) 0x01F4 << QList<quint32>({0x05});
    QTest::newRow("E500_S6 Sum") << (quint32) 0x01F4 << QList<quint32>({0x00, 0x01, 0x02, 0x03});
    QTest::newRow("E500_S500 Sum V1") << (quint32) 0x01F4 << QList<quint32>({0xFA, 0xFA});
    QTest::newRow("E500_S500 Sum V1") << (quint32) 0x01F4 << QList<quint32>({0x64, 0x64, 0x64, 0x64, 0x64});
    QTest::newRow("E1000_S500 Sum") << (quint32) 0x03E8 << QList<quint32>({0xFA, 0xFA});
    QTest::newRow("E1000_S1000 Sum") << (quint32) 0x03E8 << QList<quint32>({0xFA, 0xFA, 0x01F4});
    QTest::newRow("E1234_S0") << (quint32) 0x04D2 << QList<quint32>({0x00});
    QTest::newRow("E12345_S0") << (quint32) 0x3039 << QList<quint32>({0x00});
    QTest::newRow("E123456_S0") << (quint32) 0x01E240 << QList<quint32>({0x00});
    QTest::newRow("RESET") << (quint32) 0x00 << QList<quint32>({0x00});
}

void GUI_BASE_TESTS::test_reset_gui_1()
{
    // Set some info
    base_tester->set_expected_recv_length_test(500);
    base_tester->update_current_recv_length_test(100);

    QByteArray rcvd_data = QByteArray("Test Dataaaaa");
    base_tester->rcvd_formatted_append_test(rcvd_data);

    // Verify the updates
    QCOMPARE(base_tester->rcvd_formatted_readAll_test(), rcvd_data);
    QCOMPARE(base_tester->rcvd_formatted_size_test(), rcvd_data.size());

    // Get static values
    uint8_t gui_key = base_tester->get_gui_key();
    QString gui_name = base_tester->get_gui_name();
    QString gui_config = base_tester->get_gui_config();

    // Setup signal spy
    QList<QVariant> spy_args;
    QSignalSpy rcvd_reset_spy(base_tester, base_tester->progress_update_recv);
    QSignalSpy send_reset_spy(base_tester, base_tester->progress_update_send);
    QVERIFY(rcvd_reset_spy.isValid());
    QVERIFY(send_reset_spy.isValid());

    // Reset the gui
    base_tester->reset_gui();
    qApp->processEvents();

    // Verify the reset values
    QCOMPARE(base_tester->rcvd_formatted_readAll_test(), QByteArray());
    QCOMPARE(base_tester->rcvd_formatted_size_test(), (int) 0);

    // Verify the static values haven't changed
    QCOMPARE(base_tester->get_gui_key(), gui_key);
    QCOMPARE(base_tester->get_gui_name(), gui_name);
    QCOMPARE(base_tester->get_gui_config(), gui_config);

    // Verify signal spy
    QCOMPARE(rcvd_reset_spy.count(), (int) 1);
    spy_args = rcvd_reset_spy.takeFirst();
    QCOMPARE(spy_args.at(0).toInt(), (int) 0);
    QCOMPARE(spy_args.at(1).toString(), QString(""));

    QCOMPARE(send_reset_spy.count(), (int) 1);
    spy_args = send_reset_spy.takeFirst();
    QCOMPARE(spy_args.at(0).toInt(), (int) 0);
    QCOMPARE(spy_args.at(1).toString(), QString(""));
}

void GUI_BASE_TESTS::test_reset_gui_2()
{
    // Setup signal spy
    QList<QVariant> spy_args;
    QSignalSpy transmit_chunk_spy(base_tester, base_tester->transmit_chunk);
    QVERIFY(transmit_chunk_spy.isValid());

    // Call slot
    base_tester->on_ResetGUI_Button_clicked_test();

    // Verify signal spy
    QCOMPARE(transmit_chunk_spy.count(), (int) 1);
    spy_args = transmit_chunk_spy.takeFirst();
    QCOMPARE(spy_args.at(0).toInt(), (int) MAJOR_KEY_RESET);
    QCOMPARE(spy_args.at(1).toInt(), (int) 0);
}

void GUI_BASE_TESTS::test_rcvd_formatted()
{
    // Fetch data
    QFETCH(QList<QByteArray>, rcvd_formatted);
    QFETCH(bool, rcvd_formatted_test_save);
    QFETCH(bool, rcvd_formatted_test_clear);

    // Clear existing data
    base_tester->rcvd_formatted_clear_test();

    // Send all data
    QByteArray rcvd_data;
    foreach (QByteArray data, rcvd_formatted)
    {
        rcvd_data.append(data);
        base_tester->rcvd_formatted_append_test(data);
    }

    // Verify set data
    QCOMPARE(base_tester->rcvd_formatted_readAll_test(), rcvd_data);
    QCOMPARE(base_tester->rcvd_formatted_size_test(), rcvd_data.size());

    // Check if testing save
    if (rcvd_formatted_test_save)
    {
        // Create, open, and close save file
        // (opening forces creation on disk)
        QTemporaryFile *tmp_file = new QTemporaryFile();
        tmp_file->setAutoRemove(true);
        QVERIFY(tmp_file->open());
        tmp_file->close();
        QString fileName = tmp_file->fileName();
        delete tmp_file;

        // Write data to file
        base_tester->rcvd_formatted_save_test(fileName);

        // Read data from file & remove file
        QByteArray data = GUI_GENERIC_HELPER::loadFile(fileName);
        QFile::remove(fileName);

        // Verify read data
        QCOMPARE(data, rcvd_data);
        QCOMPARE(data.size(), rcvd_data.size());
    }

    // Check if testing clear
    if (rcvd_formatted_test_clear)
    {
        // Clear data
        base_tester->rcvd_formatted_clear_test();

        // Verify cleared
        QCOMPARE(base_tester->rcvd_formatted_readAll_test(), QByteArray());
        QCOMPARE(base_tester->rcvd_formatted_size_test(), (int) 0);
    }
}

void GUI_BASE_TESTS::test_rcvd_formatted_data()
{
    // Setup data columns
    QTest::addColumn<QList<QByteArray>>("rcvd_formatted");
    QTest::addColumn<bool>("rcvd_formatted_test_save");
    QTest::addColumn<bool>("rcvd_formatted_test_clear");

    // Load in data
    QByteArray data = GUI_GENERIC_HELPER::qList_to_byteArray({'d', 'a', 't', 'a'});
    QTest::newRow("Basic") << QList<QByteArray>({data}) << false << false;
    QTest::newRow("Clear") << QList<QByteArray>({data}) << false << true;
    QTest::newRow("Save") << QList<QByteArray>({data}) << true << true;
    QTest::newRow("Rcvd Multiple V1") << QList<QByteArray>({data, data, data}) << false << false;
    QTest::newRow("Save Multiple V1") << QList<QByteArray>({data, data, data}) << true << false;
    QTest::newRow("String Basic V1") << QList<QByteArray>({"ReadMe"}) << false << false;
    QTest::newRow("RESET") << QList<QByteArray>({""}) << false << true;
}

void GUI_BASE_TESTS::test_rcvd_formatted_save_fail()
{
    // Create temporary file
    QTemporaryFile tmp;
    tmp.setAutoRemove(true);

    // Open (to prevent writing)
    tmp.open();

    // Try calling copy (will fail and call showMessage)
    base_tester->rcvd_formatted_save_test(tmp.fileName());
    qApp->processEvents();

    // Verify active window is error message
    QMessageBox *error_msg = \
            qobject_cast<QMessageBox*>(QApplication::activeWindow());
    QVERIFY(error_msg);
    QCOMPARE(error_msg->text(), QString("Error: Failed to save file!"));

    // Close error_msg
    error_msg->close();

    // Close file
    tmp.close();
}

void GUI_BASE_TESTS::test_send_chunk_qlist()
{
    // Fetch data
    QFETCH(quint8, major_key);
    QFETCH(quint8, minor_key);
    QFETCH(QList<quint8>, send_chunk);

    // Setup signal spy
    QList<QVariant> spy_args;
    QSignalSpy transmit_chunk_spy(base_tester, base_tester->transmit_chunk);
    QVERIFY(transmit_chunk_spy.isValid());

    // Send the chunk
    base_tester->send_chunk_test(major_key, minor_key, send_chunk);

    // Verify signal spy
    QCOMPARE(transmit_chunk_spy.count(), (int) 1);
    spy_args = transmit_chunk_spy.takeFirst();
    QCOMPARE(spy_args.at(0).toInt(), (int) major_key);
    QCOMPARE(spy_args.at(1).toInt(), (int) minor_key);
    QCOMPARE(spy_args.at(2).toByteArray(), GUI_GENERIC_HELPER::qList_to_byteArray(send_chunk));
}

void GUI_BASE_TESTS::test_send_chunk_qlist_data()
{
    // Setup data columns
    QTest::addColumn<quint8>("major_key");
    QTest::addColumn<quint8>("minor_key");
    QTest::addColumn<QList<quint8>>("send_chunk");

    // Load in data
    QTest::newRow("Empty") << (quint8) 0x00 << (quint8) 0x00 << QList<quint8>();
    QTest::newRow("Basic") << (quint8) 0x04 << (quint8) 0x03 << QList<quint8>({0x00, 0x01, 0x02});
    QTest::newRow("Random") << (quint8) qrand() << (quint8) qrand() \
                            << QList<quint8>({(quint8) qrand(), (quint8) qrand(), (quint8) qrand()});
}
