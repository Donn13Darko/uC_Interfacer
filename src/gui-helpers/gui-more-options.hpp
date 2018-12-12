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

#ifndef GUI_MORE_OPTIONS_H
#define GUI_MORE_OPTIONS_H

#include <QDialog>
#include <QMap>
#include <QString>

#include "gui-generic-helper.hpp"

typedef struct MoreOptions_struct {
    bool reset_on_tab_switch;
    bool send_little_endian;
    uint32_t chunk_size;
    // (1) checksum_name, (2) checksum_start, (3) checksum_start_base, (4) checksum_exe
    QMap<QString, QStringList> checksum_map;
    QStringList custom;
} MoreOptions_struct;

#define DEFAULT_CHECKSUM QStringList({"CRC_8_LUT", "", "", ""})

namespace Ui {
class GUI_MORE_OPTIONS;
}

class GUI_MORE_OPTIONS : public QDialog
{
    Q_OBJECT

public:
    explicit GUI_MORE_OPTIONS(MoreOptions_struct *main_options, MoreOptions_struct **local_options_ptr, QStringList GUIs, QStringList checksums, QWidget *parent = 0);
    ~GUI_MORE_OPTIONS();

public slots:
    void reset_gui();

private slots:
    void on_GUI_Combo_activated(int);
    void on_Checksum_Combo_activated(int);
    void on_ChecksumSet_Button_clicked();
    void on_BrowseEXE_Button_clicked();

    void on_Undo_Button_clicked();
    void on_Apply_Button_clicked();
    void on_Cancel_Button_clicked();
    void on_OK_Button_clicked();

private:
    Ui::GUI_MORE_OPTIONS *ui;
    bool updated;

    MoreOptions_struct *main_options_ptr;
    MoreOptions_struct local_options;

    void save_updates();
    void reset_updates();
};

#endif // GUI_MORE_OPTIONS_H
