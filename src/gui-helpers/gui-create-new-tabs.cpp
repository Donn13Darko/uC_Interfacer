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

#include "gui-create-new-tabs.hpp"
#include "ui_gui-create-new-tabs.h"

#include <QTemporaryFile>
#include <QByteArray>

GUI_CREATE_NEW_TABS::GUI_CREATE_NEW_TABS(QMap<QString, QMap<QString, QVariant>*> **configMap, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GUI_CREATE_NEW_TABS)
{
    // Setup ui
    ui->setupUi(this);
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);

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

void GUI_CREATE_NEW_TABS::set_title(QString title)
{
    ui->Title_Label->setText(title);
}

void GUI_CREATE_NEW_TABS::set_config_tab(int index, QString new_config_str)
{
    // Set information
    local_index = index;
    local_config_str = new_config_str;

    // Update display
    on_Undo_Button_clicked();
}

QString GUI_CREATE_NEW_TABS::get_tab_config_str()
{
    return local_config_str;
}

int GUI_CREATE_NEW_TABS::get_tab_index()
{
    return local_index;
}

void GUI_CREATE_NEW_TABS::reset_gui()
{
    // Clear fields
    on_Clear_Button_clicked();
    local_config_str.clear();

    // Reset tab_pos
    local_index = -1;

    // Delete config map if populated
    if (*local_configMap) GUI_HELPER::deleteConfigMap(local_configMap);
}

void GUI_CREATE_NEW_TABS::on_Undo_Button_clicked()
{
    // Set plaintext to start value
    ui->CreateNewTabs_PlainText->setPlainText(local_config_str);
}

void GUI_CREATE_NEW_TABS::on_Clear_Button_clicked()
{
    // Clear the plaintext (leave config string for undo)
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
        // If nothing to add/change, ignore
        reject();

        // Exit
        return;
    }

    // Verify local configs is null
    if (*local_configMap) GUI_HELPER::deleteConfigMap(local_configMap);

    // Read in new settings with helper
    *local_configMap = GUI_HELPER::decode_configMap(input_ini);

    // If success, accept changes
    // (tells MainWindow to add new data)
    if (*local_configMap) accept();
    else reject();
}
