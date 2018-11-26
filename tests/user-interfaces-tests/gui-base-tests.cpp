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
#include <QTimer>
#include <QEventLoop>
#include <QFile>
#include <QTemporaryFile>

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
    QCOMPARE(base_tester->get_gui_tab_name(), QString("GUI Base"));
    QCOMPARE(base_tester->get_gui_config(), QString("[GUI Base]\n\n"));
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

    // Set and check result
    base_tester->set_gui_name_test(set_name);
    QCOMPARE(base_tester->get_gui_name(), set_name);
}

void GUI_BASE_TESTS::test_set_gui_name_data()
{
    // Setup data columns
    QTest::addColumn<QString>("set_name");

    // Load in data
    QTest::newRow("Blank") << "";
    QTest::newRow("Short") << "IO";
    QTest::newRow("Shimple") << "Welcome";
    QTest::newRow("Space") << "Data Transmit";
    QTest::newRow("Random Caps") << "RandOM GLESN";
    QTest::newRow("Random Chars") << "'-1923'/,.l][qw2487923-1!(*@!*$@!_";
    QTest::newRow("RESET") << base_tester->get_gui_name();
}

void GUI_BASE_TESTS::test_set_gui_tab_name()
{
    // Fetch name
    QFETCH(QString, set_name);

    // Set and check result
    base_tester->set_gui_tab_name(set_name);
    QCOMPARE(base_tester->get_gui_tab_name(), set_name);
}

void GUI_BASE_TESTS::test_set_gui_tab_name_data()
{
    // Setup data columns
    QTest::addColumn<QString>("set_name");

    // Load in data
    QTest::newRow("Blank") << "";
    QTest::newRow("Simple") << "New Tab";
    QTest::newRow("Random Caps") << "RandOM GLESN";
    QTest::newRow("Random Chars") << "'-1923'/,.l][qw2487923-1!(*@!*$@!_";
    QTest::newRow("RESET") << base_tester->get_gui_tab_name();
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
    config_str += "tab_name=\"\"\n\n";

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

void GUI_BASE_TESTS::test_recv_length()
{
    // Fetch data
    QFETCH(quint32, expected_recv_len);
    QFETCH(QList<quint32>, recv_len_data);
    QFETCH(QString, expected_recv_len_str);

    // Setup signal spy
    QList<QVariant> spy_args;
    QSignalSpy progress_spy(base_tester, base_tester->progress_update_recv);
    QVERIFY(progress_spy.isValid());

    // Set start info
    base_tester->set_expected_recv_length_test(expected_recv_len);

    // Verify start info
    QCOMPARE(base_tester->get_expected_recv_length_test(), expected_recv_len);
    QCOMPARE(base_tester->get_expected_recv_length_str_test(), expected_recv_len_str);

    // Verify signals
    QCOMPARE(progress_spy.count(), 1);
    spy_args = progress_spy.takeFirst();
    QCOMPARE(spy_args.at(0).toInt(), 0);
    QCOMPARE(spy_args.at(1).toString(), QString(""));

    // Send packets & verify signals
    bool recv_len_done = false;
    quint32 target_recv = 0;
    foreach (quint32 recv_len, recv_len_data)
    {
        // Send update to base class
        base_tester->update_current_recv_length_test(recv_len);

        // Continue if alread received expected_recv_len
        if (recv_len_done) continue;

        // Increment target and verify
        target_recv += recv_len;
        if (!expected_recv_len || (expected_recv_len <= target_recv))
        {
            // Verify final signal
            if (expected_recv_len && (expected_recv_len <= target_recv))
            {
                QCOMPARE(progress_spy.count(), 1);
                spy_args = progress_spy.takeFirst();
                QCOMPARE(spy_args.at(0).toInt(), 100);
                QCOMPARE(spy_args.at(1).toString(), QString("Done!"));
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
            QCOMPARE(progress_spy.count(), 1);
            spy_args = progress_spy.takeFirst();
            QCOMPARE(spy_args.at(0).toInt(), qRound(((float) target_recv / expected_recv_len) * 100.0f));
            QCOMPARE(spy_args.at(1).toString(), QString::number((float) target_recv / 1000.0f) + expected_recv_len_str);
        }
    }

    // Verify the updates
    QCOMPARE(base_tester->get_current_recv_length_test(), target_recv);
    QCOMPARE(base_tester->get_expected_recv_length_test(), expected_recv_len);
    QCOMPARE(base_tester->get_expected_recv_length_str_test(), expected_recv_len_str);
}

void GUI_BASE_TESTS::test_recv_length_data()
{
    // Setup data columns
    QTest::addColumn<quint32>("expected_recv_len");
    QTest::addColumn<QList<quint32>>("recv_len_data");
    QTest::addColumn<QString>("expected_recv_len_str");

    // Load in data
    QTest::newRow("E0_S0") << (quint32) 0 << QList<quint32>({0}) << "";
    QTest::newRow("E0_S1") << (quint32) 0 << QList<quint32>({1}) << "";
    QTest::newRow("E1_S0") << (quint32) 1 << QList<quint32>({0}) << "/0.001KB";
    QTest::newRow("E1_S1 Start") << (quint32) 1 << QList<quint32>({1, 0, 0}) << "/0.001KB";
    QTest::newRow("E1_S1 Middle") << (quint32) 1 << QList<quint32>({0, 1, 0}) << "/0.001KB";
    QTest::newRow("E1_S1 End") << (quint32) 1 << QList<quint32>({0, 0, 1}) << "/0.001KB";
    QTest::newRow("E1_S3 V1") << (quint32) 1 << QList<quint32>({3}) << "/0.001KB";
    QTest::newRow("E1_S3 V2") << (quint32) 1 << QList<quint32>({1, 2}) << "/0.001KB";
    QTest::newRow("E1_S3 V3") << (quint32) 1 << QList<quint32>({1, 0, 2}) << "/0.001KB";
    QTest::newRow("E5_S6 V1") << (quint32) 5 << QList<quint32>({0, 1, 2, 3}) << "/0.005KB";
    QTest::newRow("E500_S0") << (quint32) 500 << QList<quint32>({0}) << "/0.5KB";
    QTest::newRow("E500_S500") << (quint32) 500 << QList<quint32>({500}) << "/0.5KB";
    QTest::newRow("E500_S5") << (quint32) 500 << QList<quint32>({5}) << "/0.5KB";
    QTest::newRow("E500_S6 Sum") << (quint32) 500 << QList<quint32>({0, 1, 2, 3}) << "/0.5KB";
    QTest::newRow("E500_S500 Sum V1") << (quint32) 500 << QList<quint32>({250, 250}) << "/0.5KB";
    QTest::newRow("E500_S500 Sum V1") << (quint32) 500 << QList<quint32>({100, 100, 100, 100, 100}) << "/0.5KB";
    QTest::newRow("E1000_S500 Sum") << (quint32) 1000 << QList<quint32>({250, 250}) << "/1KB";
    QTest::newRow("E1000_S1000 Sum") << (quint32) 1000 << QList<quint32>({250, 250, 500}) << "/1KB";
    QTest::newRow("E1234_S0") << (quint32) 1234 << QList<quint32>({0}) << "/1.234KB";
    QTest::newRow("E12345_S0") << (quint32) 12345 << QList<quint32>({0}) << "/12.345KB";
    QTest::newRow("E123456_S0") << (quint32) 123456 << QList<quint32>({0}) << "/123.456KB";
    QTest::newRow("RESET") << base_tester->get_expected_recv_length_test() \
                           << QList<quint32>({0}) \
                           << base_tester->get_expected_recv_length_str_test();
}

void GUI_BASE_TESTS::test_reset_gui_1()
{
    // Set some info
    base_tester->set_expected_recv_length_test(500);
    base_tester->update_current_recv_length_test(100);

    QByteArray rcvd_data = QByteArray("Test Dataaaaa");
    base_tester->rcvd_formatted_append_test(rcvd_data);

    // Verify the updates
    QCOMPARE(base_tester->get_expected_recv_length_test(), (uint32_t) 500);
    QCOMPARE(base_tester->get_current_recv_length_test(), (uint32_t) 100);
    QCOMPARE(base_tester->get_expected_recv_length_str_test(), QString("/0.5KB"));
    QCOMPARE(base_tester->rcvd_formatted_readAll_test(), rcvd_data);
    QCOMPARE(base_tester->rcvd_formatted_size_test(), rcvd_data.size());

    // Get static values
    uint8_t gui_key = base_tester->get_gui_key();
    QString gui_name = base_tester->get_gui_name();
    CONFIG_MAP *gui_config = base_tester->get_gui_config_test();
    QMap<QString, QVariant> *gui_map = base_tester->get_gui_map_test();

    // Setup signal spy
    QList<QVariant> spy_args;
    QSignalSpy rcvd_reset_spy(base_tester, base_tester->progress_update_recv);
    QSignalSpy send_reset_spy(base_tester, base_tester->progress_update_send);
    QVERIFY(rcvd_reset_spy.isValid());
    QVERIFY(send_reset_spy.isValid());

    // Reset the gui
    base_tester->reset_gui();

    // Verify the reset values
    QCOMPARE(base_tester->get_expected_recv_length_test(), (uint32_t) 0);
    QCOMPARE(base_tester->get_current_recv_length_test(), (uint32_t) 0);
    QCOMPARE(base_tester->get_expected_recv_length_str_test(), QString(""));
    QCOMPARE(base_tester->rcvd_formatted_readAll_test(), QByteArray());
    QCOMPARE(base_tester->rcvd_formatted_size_test(), 0);

    // Verify the static values haven't changed
    QCOMPARE(base_tester->get_gui_key(), gui_key);
    QCOMPARE(base_tester->get_gui_name(), gui_name);
    QCOMPARE(base_tester->get_gui_config_test(), gui_config);
    QCOMPARE(base_tester->get_gui_map_test(), gui_map);

    // Verify signal spy
    QCOMPARE(rcvd_reset_spy.count(), 1);
    QCOMPARE(send_reset_spy.count(), 1);

    spy_args = rcvd_reset_spy.takeFirst();
    QCOMPARE(spy_args.at(0).toInt(), 0);
    QCOMPARE(spy_args.at(1).toString(), QString(""));

    spy_args = send_reset_spy.takeFirst();
    QCOMPARE(spy_args.at(0).toInt(), 0);
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
    QCOMPARE(transmit_chunk_spy.count(), 1);
    spy_args = transmit_chunk_spy.takeFirst();
    QCOMPARE(spy_args.at(0).toInt(), (int) MAJOR_KEY_RESET);
    QCOMPARE(spy_args.at(1).toInt(), 0);
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
        QCOMPARE(base_tester->rcvd_formatted_size_test(), 0);
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
    QCOMPARE(transmit_chunk_spy.count(), 1);
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
    QTest::newRow("Empty") << (quint8) 0 << (quint8) 0 << QList<quint8>();
    QTest::newRow("Basic") << (quint8) 4 << (quint8) 3 << QList<quint8>({(quint8) 0, (quint8) 1, (quint8) 2});
    QTest::newRow("Random") << (quint8) qrand() << (quint8) qrand() << QList<quint8>({(quint8) qrand(), (quint8) qrand(), (quint8) qrand()});
}
