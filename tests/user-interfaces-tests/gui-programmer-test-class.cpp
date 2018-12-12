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

#include "gui-programmer-test-class.hpp"
#include "ui_gui-programmer.h"

#include <QTest>
#include <QSignalSpy>

GUI_PROGRAMMER_TEST_CLASS::GUI_PROGRAMMER_TEST_CLASS(QWidget *parent) :
    GUI_PROGRAMMER(parent)
{
    // Retrieve ui object
    ui_ptr = get_ui();

    // Show UI (needs to be visible for UI events)
    show();
}

GUI_PROGRAMMER_TEST_CLASS::~GUI_PROGRAMMER_TEST_CLASS()
{
    /* DO NOTHING */
}

QByteArray GUI_PROGRAMMER_TEST_CLASS::rcvd_formatted_readAll_test()
{
    return rcvd_formatted_readAll();
}

qint64 GUI_PROGRAMMER_TEST_CLASS::rcvd_formatted_size_test()
{
    return rcvd_formatted_size();
}

bool GUI_PROGRAMMER_TEST_CLASS::reset_clicked_test()
{
    // Setup spy to catch tranmit signal
    QList<QVariant> spy_args;
    QSignalSpy transmit_chunk_spy(this, transmit_chunk);
    if (!transmit_chunk_spy.isValid()) return false;

    // Click the reset button
    QTest::mouseClick(ui_ptr->ResetGUI_Button, Qt::LeftButton);
    qApp->processEvents();

    // Verify that reset signal emitted
    if (transmit_chunk_spy.count() != 1) return false;
    spy_args = transmit_chunk_spy.takeFirst();
    return ((spy_args.at(0).toInt() == MAJOR_KEY_RESET)
            && (spy_args.at(1).toInt() == 0));
}

void GUI_PROGRAMMER_TEST_CLASS::set_checked_click_test(QCheckBox *check, bool b)
{
    if (b && !check->isChecked())
        QTest::mouseClick(check, Qt::LeftButton);
    else if (!b && check->isChecked())
        QTest::mouseClick(check, Qt::LeftButton);

    qApp->processEvents();
}
