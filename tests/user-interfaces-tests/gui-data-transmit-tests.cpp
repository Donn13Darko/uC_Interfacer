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
             data_transmit_tester->get_gui_name().prepend("[").append("]\n\n"));
}

void GUI_DATA_TRANSMIT_TESTS::test_basic_features()
{
    // Test waitForDevice
    QVERIFY(!data_transmit_tester->waitForDevice(0));
    QVERIFY(data_transmit_tester->waitForDevice(MINOR_KEY_DATA_TRANSMIT_DATA));
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
