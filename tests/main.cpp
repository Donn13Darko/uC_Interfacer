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

// Testing infrastructure includes
#include <QtTest>
#include <QCoreApplication>

// Testing classes
#include "user-interfaces-tests/gui-base-tests.hpp"
#include "user-interfaces-tests/gui-welcome-tests.hpp"
#include "user-interfaces-tests/gui-io-control-tests.hpp"
#include "user-interfaces-tests/gui-data-transmit-tests.hpp"
#include "user-interfaces-tests/gui-programmer-tests.hpp"
#include "user-interfaces-tests/gui-custom-cmd-tests.hpp"

int main(int argc, char *argv[])
{
    // Create application (for use in widgets)
    QApplication a(argc, argv);

    // Setup status variable
    int status = 0;

    // Parse input & force modes
    QStringList argList = QString(*argv).split(" ");

    // Force parameters unless told not to
    if (!argList.contains("-interfaceForceParamsOff"))
    {
        // Force silent mode
        if (!argList.contains("-silent")) argList.append("-silent");
    } else
    {
        // Remove force off argument if present
        argList.removeAll("-interfaceForceParamsOff");
    }

    /* GUI Base Tests */
    GUI_BASE_TESTS gui_base_tester;
    status |= QTest::qExec(&gui_base_tester, argList);

    /* GUI Welcome Tests */
    GUI_WELCOME_TESTS gui_welcome_tester;
    status |= QTest::qExec(&gui_welcome_tester, argList);

    /* GUI IO Control Tests */
    GUI_IO_CONTROL_TESTS gui_io_control_tester;
    status |= QTest::qExec(&gui_io_control_tester, argList);

    /* GUI Data Transmit Tests */
    GUI_DATA_TRANSMIT_TESTS gui_data_transmit_tester;
    status |= QTest::qExec(&gui_data_transmit_tester, argList);

    /* GUI Programmer Tests */
    GUI_PROGRAMMER_TESTS gui_programmer_tester;
    status |= QTest::qExec(&gui_programmer_tester, argList);

    /* GUI Custom CMD Tests */
    GUI_CUSTOM_CMD_TESTS gui_custom_cmd_tester;
    status |= QTest::qExec(&gui_custom_cmd_tester, argList);

    /* Show if pass or fail */
    if (status) qInfo() << "Tests Failed!";
    else qInfo() << "All Tests Passed!";

    // Return if error
    return status;
}
