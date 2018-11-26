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
