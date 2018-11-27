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

#include "gui-welcome.hpp"
#include "ui_gui-welcome.h"

GUI_WELCOME::GUI_WELCOME(QWidget *parent) :
    GUI_BASE(parent),
    ui(new Ui::GUI_WELCOME)
{
    // Setup UI
    ui->setupUi(this);

    // Set GUI Type & Default Name
    set_gui_key(MAJOR_KEY_WELCOME);
    set_gui_name("Welcome");
    set_header(get_gui_name());
}

GUI_WELCOME::~GUI_WELCOME()
{
    delete ui;
}

void GUI_WELCOME::set_header(QString text)
{
    set_gui_map_value("header", text);
    ui->header_label->setText(get_gui_map_value("header").toString());
}

QString GUI_WELCOME::get_header()
{
    return get_gui_map_value("header").toString();
}

void GUI_WELCOME::set_msg(QString text)
{
    set_gui_map_value("msg", text);
    ui->msg_label->setText(get_gui_map_value("msg").toString());
}

QString GUI_WELCOME::get_msg()
{
    return get_gui_map_value("msg").toString();
}

void GUI_WELCOME::set_buttons_enabled(bool enabled)
{
    // Set map
    set_gui_map_value("enable_buttons", enabled);
    bool map_value = get_gui_map_value("enable_buttons").toBool();

    // Set buttons
    ui->ResetGUI_Button->setVisible(map_value);
}

bool GUI_WELCOME::get_buttons_enabled()
{
    return get_gui_map_value("enable_buttons").toBool();
}

void GUI_WELCOME::parseConfigMap(QMap<QString, QVariant> *configMap)
{
    // Pass to parent for additional parsing
    GUI_BASE::parseConfigMap(configMap);

    // Parse individual values
    set_header(configMap->value("header", get_gui_name()).toString());
    set_msg(configMap->value("msg").toString());
    set_buttons_enabled(configMap->value("enable_buttons", "false").toBool());
}
