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

#ifndef GUI_CREATE_NEW_TABS_H
#define GUI_CREATE_NEW_TABS_H

#include <QDialog>
#include <QMap>
#include <QString>
#include <QVariant>

#include "gui-helper.h"

namespace Ui {
class GUI_CREATE_NEW_TABS;
}

class GUI_CREATE_NEW_TABS : public QDialog
{
    Q_OBJECT

public:
    explicit GUI_CREATE_NEW_TABS(QMap<QString, QMap<QString, QVariant>*> **configMap, QWidget *parent = 0);
    ~GUI_CREATE_NEW_TABS();

    void set_config_tab(int index, QString new_config_str);
    QString get_tab_config_str();
    int get_tab_index();

public slots:
    void reset_gui();

private slots:
    void on_Undo_Button_clicked();
    void on_Clear_Button_clicked();
    void on_Cancel_Button_clicked();
    void on_OK_Button_clicked();

private:
    Ui::GUI_CREATE_NEW_TABS *ui;

    QMap<QString, QMap<QString, QVariant>*> **local_configMap;

    QString local_config_str;
    int local_index;

    void save_updates();
    void reset_updates();
};

#endif // GUI_CREATE_NEW_TABS_H
