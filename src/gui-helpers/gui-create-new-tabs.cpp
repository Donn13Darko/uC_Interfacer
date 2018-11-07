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

#include "gui-create-new-tabs.h"
#include "ui_gui-create-new-tabs.h"

#include <QTemporaryFile>
#include <QByteArray>

GUI_CREATE_NEW_TABS::GUI_CREATE_NEW_TABS(QMap<QString, QMap<QString, QVariant>*> **configMap, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GUI_CREATE_NEW_TABS)
{
    // Setup ui
    ui->setupUi(this);
    setWindowFlags(Qt::WindowCloseButtonHint
                   | Qt::MSWindowsFixedSizeDialogHint);

    // Link config maps
    local_configMap = configMap;

    // Reset GUI
    reset_gui();
}

GUI_CREATE_NEW_TABS::~GUI_CREATE_NEW_TABS()
{
    // Delete the config map
    if (*local_configMap) GUI_HELPER::deleteConfigMap(local_configMap);

    // Delete the ui
    delete ui;
}

void GUI_CREATE_NEW_TABS::reset_gui()
{
    // Clear fields
    on_Clear_Button_clicked();

    // Delete config map if populated
    if (*local_configMap) GUI_HELPER::deleteConfigMap(local_configMap);
}

void GUI_CREATE_NEW_TABS::on_Clear_Button_clicked()
{
    // Clear the plaintext
    ui->CreateNewTabs_PlainText->clear();
}

void GUI_CREATE_NEW_TABS::on_Cancel_Button_clicked()
{
    // Call reject to close
    reject();
}

void GUI_CREATE_NEW_TABS::on_OK_Button_clicked()
{
    // Save input string
    QString input_ini = ui->CreateNewTabs_PlainText->toPlainText();
    if (input_ini.isEmpty())
    {
        // If nothing to add, ignore
        reject();

        // Exit
        return;
    }

    // Create temporary file
    QTemporaryFile tmpINI;
    tmpINI.setAutoRemove(true);
    if (!tmpINI.open())
    {
        // Show error message
        GUI_HELPER::showMessage("Error: Unable to create temp file!");

        // Reject changes
        reject();

        // Return out of function
        return;
    }

    // Write data to temporary file & close
    tmpINI.write(input_ini.toLatin1());
    QString tmpName = tmpINI.fileName();
    tmpINI.close();

    // Verify local configs is null
    if (*local_configMap) GUI_HELPER::deleteConfigMap(local_configMap);

    // Read in new settings with helper
    *local_configMap = GUI_HELPER::readConfigINI(tmpName);

    // If success, accept changes
    // (tells MainWindow to add new data)
    if (*local_configMap) accept();
    else reject();
}
