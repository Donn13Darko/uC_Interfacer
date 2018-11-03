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
    set_expected_recv_length(QByteArray());
    update_current_recv_length(QByteArray());

    // Connect read signals and slots
    connect(this, SIGNAL(readyRead(QByteArray)),
            this, SLOT(receive_gui(QByteArray)),
            Qt::QueuedConnection);

    // Connect progress signals and slots
    // Wait till return to main event loop to process slots
    connect(this, SIGNAL(progress_update_recv(int, QString)),
            this, SLOT(set_progress_update_recv(int,QString)),
            Qt::QueuedConnection);
    connect(this, SIGNAL(progress_update_send(int, QString)),
            this, SLOT(set_progress_update_send(int,QString)),
            Qt::QueuedConnection);
}

GUI_BASE::~GUI_BASE()
{
}

void GUI_BASE::reset_remote()
{
    // Load reset CMD into msgList
    send_chunk(MAJOR_KEY_RESET, 0);

    /* Exiting here returngs control to the main event loop
     * Required in order for the next packet to be sent
     * if currently in waitForAck or waitForData
     * conn_bridge will emit reset() once CMD is set which
     * is tied to reset_gui()
    */
}

void GUI_BASE::reset_gui()
{
    // Clear formatted
    rcvd_formatted.clear();

    // Reset progress bars info
    start_data = true;
    current_recv_length = 0;
    expected_recv_length = 1;

    emit progress_update_recv(0, "");
    emit progress_update_send(0, "");
}

uint8_t GUI_BASE::get_GUI_key()
{
    return gui_key;
}

void GUI_BASE::parseConfigMap(QMap<QString, QVariant>*)
{
    // Default do nothing
}

void GUI_BASE::receive_gui(QByteArray)
{
    // Default do nothing
}

void GUI_BASE::on_ResetGUI_Button_clicked()
{
    // Reset the Remote
    // (reset_remote() enforces a GUI reset)
    reset_remote();
}

void GUI_BASE::set_progress_update_recv(int, QString)
{
    // Default do nothing
}

void GUI_BASE::set_progress_update_send(int, QString)
{
    // Default do nothing
}

void GUI_BASE::send_file(uint8_t major_key, uint8_t minor_key, QString filePath)
{
    emit transmit_file(major_key, minor_key, filePath);
}

void GUI_BASE::send_file_chunked(uint8_t major_key, uint8_t minor_key, QString filePath, char sep)
{
    emit transmit_file_chunked(major_key, minor_key, filePath, sep);
}

void GUI_BASE::send_chunk(uint8_t major_key, uint8_t minor_key, QByteArray chunk, bool force_envelope)
{
    emit transmit_chunk(major_key, minor_key, chunk, force_envelope);
}

void GUI_BASE::send_chunk(uint8_t major_key, uint8_t minor_key, std::initializer_list<uint8_t> chunk, bool force_envelope)
{
    send_chunk(
                major_key,
                minor_key,
                GUI_HELPER::initList_to_byteArray(chunk),
                force_envelope
                );
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

void GUI_BASE::set_expected_recv_length(QByteArray recv_length)
{
    // Convert and set qbytearray
    expected_recv_length = GUI_HELPER::byteArray_to_uint32(recv_length);

    // Reset progress bar
    emit progress_update_recv(0, "");
}

void GUI_BASE::update_current_recv_length(QByteArray recvData)
{
    // Find length of data
    int data_len = recvData.length();

    // Start and stop sent by
    if (data_len == 0)
    {
        // See if starting new file
        if (start_data)
        {
            current_recv_length = 0;
            emit progress_update_recv(0, "");
        } else
        {
            expected_recv_length = 0;
            emit progress_update_recv(100, "Done!");
        }

        // Toggle start_data
        start_data = !start_data;

        // Exit after setting
        return;
    }

    // Update received length
    current_recv_length += data_len;

    // Update progress bar if total recv length known
    if (expected_recv_length != 0)
        emit progress_update_recv(qRound(((float) current_recv_length/expected_recv_length) * 100.0f), "");
}
