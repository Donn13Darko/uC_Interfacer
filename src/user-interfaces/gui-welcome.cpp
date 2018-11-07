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

#include "gui-welcome.h"
#include "ui_gui-welcome.h"

GUI_WELCOME::GUI_WELCOME(QWidget *parent) :
    GUI_BASE(parent),
    ui(new Ui::GUI_WELCOME)
{
    // Setup UI
    ui->setupUi(this);
    ui->ResetGUI_Button->hide();

    // Set GUI Type & Default Name
    gui_key = MAJOR_KEY_WELCOME;
    gui_name = "GUI Welcome";
}

GUI_WELCOME::~GUI_WELCOME()
{
    delete ui;
}

void GUI_WELCOME::setHeader(QString text)
{
    ui->header_label->setText(text);
}

QString GUI_WELCOME::getHeader()
{
    return ui->header_label->text();
}

void GUI_WELCOME::setMsg(QString text)
{
    ui->msg_label->setText(text);
}

QString GUI_WELCOME::getMsg()
{
    return ui->msg_label->text();
}

void GUI_WELCOME::parseConfigMap(QMap<QString, QVariant> *configMap)
{
    // Parse individual values
    setHeader(configMap->value("header", "Welcome").toString());
    setMsg(configMap->value("msg").toString());

    // Pass to parent for additional parsing
    GUI_BASE::parseConfigMap(configMap);
}
