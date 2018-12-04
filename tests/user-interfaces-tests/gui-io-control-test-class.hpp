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

#ifndef GUI_IO_CONTROL_TEST_CLASS_H
#define GUI_IO_CONTROL_TEST_CLASS_H

// Testing class
#include "../../src/user-interfaces/gui-io-control.hpp"

#include <QCheckBox>

class GUI_IO_CONTROL_TEST_CLASS : public GUI_IO_CONTROL
{
    Q_OBJECT

public:
    GUI_IO_CONTROL_TEST_CLASS(QWidget *parent = 0);
    ~GUI_IO_CONTROL_TEST_CLASS();

    /*** Defines any needed setters & accessors for testing ***/
    QByteArray rcvd_formatted_readAll_test();
    qint64 rcvd_formatted_size_test();

    void set_aio_update_rate_test(float rate);
    float get_aio_update_rate_test();

    void set_dio_update_rate_test(float rate);
    float get_dio_update_rate_test();

    QString get_update_rate_start_text_test();

    void update_rate_start_clicked_test();
    void update_rate_stop_clicked_test();

    void set_log_file_save_path_test(QString filePath);
    QString get_log_file_save_path_test();

    void set_log_file_update_rate_test(float rate);
    float get_log_file_update_rate_test();

    void set_log_append_checked_test(bool b);
    bool get_log_append_checked_test();

    QString get_log_start_text_test();

    void log_start_clicked_test();
    void log_stop_clicked_test();

    bool reset_clicked_test();

    bool check_pins_test(QStringList expected_pin_list,
                         QList<QStringList> expected_combo_list,
                         QList<QList<int>> expected_slider_list,
                         QStringList expected_lineEdit_list);
    bool check_pin_test(QString pin_str, QStringList expected_combo,
                        QList<int> expected_slider, QString expected_value);

private:
    Ui::GUI_IO_CONTROL *ui_ptr;

    bool check_pin_list_test(QStringList expected_pin_list);
    QHBoxLayout *get_pin_test(QString pin_str);
    QHBoxLayout *get_pin_test(uint8_t pinType, uint8_t pinNum);

    void set_checked_click_test(QCheckBox *check, bool b);
};

#endif // GUI_IO_CONTROL_TEST_CLASS_H
