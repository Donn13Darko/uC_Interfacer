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

#include "gui-base.h"

GUI_BASE::GUI_BASE(QWidget *parent) :
    QWidget(parent)
{
    // Init progress bars
    set_expected_recv_length(0);
    update_current_recv_length(0);

    // Connect read signals and slots
    // This gets emited by a reference in comm-bridge
    // Use queued connection to allow thread compatability
    connect(this, SIGNAL(readyRead(QByteArray)),
            this, SLOT(receive_gui(QByteArray)),
            Qt::QueuedConnection);

    // Connect progress signals and slots
    // Wait till return to main event loop to process slots
    // Additionally, this gets emited by a reference in comm-bridge
    // Use queued connection to allow thread compatability
    connect(this, SIGNAL(progress_update_recv(int, QString)),
            this, SLOT(set_progress_update_recv(int,QString)),
            Qt::QueuedConnection);
    connect(this, SIGNAL(progress_update_send(int, QString)),
            this, SLOT(set_progress_update_send(int,QString)),
            Qt::QueuedConnection);

    // Connect wait loop to exit signals
}

GUI_BASE::~GUI_BASE()
{
}

void GUI_BASE::reset_gui()
{
    // Clear formatted
    rcvd_formatted.clear();

    // Reset progress bars info
    current_recv_length = 0;
    expected_recv_length = 1;

    emit progress_update_recv(0, "");
    emit progress_update_send(0, "");
}

uint8_t GUI_BASE::get_GUI_key()
{
    return gui_key;
}

QString GUI_BASE::get_GUI_name()
{
    return gui_name;
}

void GUI_BASE::set_GUI_name(QString new_name)
{
    gui_name = new_name;
}

void GUI_BASE::parseConfigMap(QMap<QString, QVariant> *configMap)
{
    // Reset name if present
    gui_name = configMap->value("tab_name", gui_name).toString();
}

void GUI_BASE::receive_gui(QByteArray)
{
    // Default do nothing
}

void GUI_BASE::on_ResetGUI_Button_clicked()
{
    // Load reset CMD into sendList
    emit transmit_chunk(MAJOR_KEY_RESET, 0);

    /* Exiting here returngs control to the main event loop
     * Required in order for the next packet to be sent
     * if conn_bridge currently in waitForAck or waitForData
     * conn_bridge will emit reset() once CMD is sent which
     * then calls reset_gui()
    */
}

void GUI_BASE::on_RemoveTab_Button_clicked()
{
    // Emit remove_tab signal
    emit remove_tab();

    /* Exiting here to return control to main loop to handle
     * removing the gui from the qtabwidget and disconnect
     * its slots/signals
     */
}

void GUI_BASE::set_progress_update_recv(int, QString)
{
    // Default do nothing
}

void GUI_BASE::set_progress_update_send(int, QString)
{
    // Default do nothing
}

void GUI_BASE::send_chunk(uint8_t major_key, uint8_t minor_key, std::initializer_list<uint8_t> chunk)
{
    emit transmit_chunk(major_key, minor_key,
                        GUI_HELPER::initList_to_byteArray(chunk));
}

bool GUI_BASE::isDataRequest(uint8_t)
{
    return false;
}

void GUI_BASE::save_rcvd_formatted()
{
    // Select file save location
    QString fileName;
    if (!GUI_HELPER::getSaveFilePath(&fileName))
        return;

    // Save file
    if (!GUI_HELPER::saveFile(fileName, rcvd_formatted))
        GUI_HELPER::showMessage("ERROR: Failed to save file!");
}

void GUI_BASE::set_expected_recv_length(uint32_t expected_length)
{
    // Convert and set qbytearray
    if (expected_length) expected_recv_length = expected_length;
    else expected_recv_length = 1;

    // Reset expected recv length str
    expected_recv_length_str = "/" + QString::number(expected_recv_length / 1000.0f) + "KB";

    // Reset progress bar
    emit progress_update_recv(0, "");
}

void GUI_BASE::update_current_recv_length(uint32_t recv_len)
{
    // Start and stop sent by
    if (recv_len == 0)
    {
        // See if starting new file
        if (current_recv_length == expected_recv_length)
        {
            emit progress_update_recv(100, "Done!");
        } else
        {
            emit progress_update_recv(0, "");
        }

        // Reset recv length
        current_recv_length = 0;

        // Exit after setting
        return;
    }

    // Update received length
    current_recv_length += recv_len;

    // Update progress bar if total recv length known
    if (expected_recv_length != 0)
        emit progress_update_recv(qRound(((float) current_recv_length/expected_recv_length) * 100.0f),
                                  QString::number((float) current_recv_length / 1000.0f) + expected_recv_length_str);
}
