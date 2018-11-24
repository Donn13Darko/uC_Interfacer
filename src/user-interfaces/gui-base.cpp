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

#include "gui-base.hpp"

GUI_BASE::GUI_BASE(QWidget *parent) :
    QWidget(parent)
{
    // Init base variables
    gui_key = MAJOR_KEY_ERROR;
    gui_name = "GUI Base";
    gui_config = nullptr;
    gui_map = nullptr;

    // Setup empty config
    QMap<QString, QVariant> empty_cmap;
    parseConfigMap(&empty_cmap);

    // Open temporary file and set autoremove
    rcvd_formatted.open();
    rcvd_formatted.setAutoRemove(true);

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
}

GUI_BASE::~GUI_BASE()
{
    // Close temporary file
    // Gets deleted on close
    if (rcvd_formatted.isOpen())
        rcvd_formatted.close();
}

void GUI_BASE::reset_gui()
{
    // Clear formatted
    rcvd_formatted_clear();

    // Reset progress bars info
    current_recv_length = 0;
    expected_recv_length = 0;
    expected_recv_length_str.clear();

    emit progress_update_recv(0, "");
    emit progress_update_send(0, "");
}

bool GUI_BASE::isClosable()
{
    if (gui_map || init_maps()) return gui_map->value("closable", "true").toBool();
    else return true;
}

void GUI_BASE::setClosable(bool new_close)
{
    if (gui_map || init_maps()) gui_map->insert("closable", new_close);
}

uint8_t GUI_BASE::get_gui_key()
{
    return gui_key;
}

QString GUI_BASE::get_gui_name()
{
    return gui_name;
}

QString GUI_BASE::get_gui_tab_name()
{
    if (gui_map || init_maps()) return gui_map->value("tab_name", gui_name).toString();
    else return gui_name;
}

void GUI_BASE::set_gui_tab_name(QString new_name)
{
    // Set new name in gui map
    if (gui_map || init_maps()) gui_map->insert("tab_name", new_name);
}

QString GUI_BASE::get_gui_config()
{
    return GUI_GENERIC_HELPER::encode_configMap(gui_config);
}

bool GUI_BASE::acceptAllCMDs()
{
    return false;
}

void GUI_BASE::parseConfigMap(QMap<QString, QVariant> *configMap)
{
    // Verify input
    if (!configMap) return;

    // Delete current map
    if (gui_config)
    {
        // Will set gui_config to nullptr
        GUI_GENERIC_HELPER::delete_configMap(&gui_config);

        // Manually set gui_mapp to nullptr (deleted as part of gui_config)
        gui_map = nullptr;
    }

    // Create temporary full config
    CONFIG_MAP tmp_map;
    tmp_map.insert(gui_name, configMap);

    // Copy full config
    gui_config = GUI_GENERIC_HELPER::copy_configMap(&tmp_map);

    // Setup and verify maps
    init_maps();
}

bool GUI_BASE::waitForDevice(uint8_t)
{
    return false;
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

void GUI_BASE::set_progress_update_recv(int, QString)
{
    // Default do nothing
}

void GUI_BASE::set_progress_update_send(int, QString)
{
    // Default do nothing
}

void GUI_BASE::send_chunk(uint8_t major_key, uint8_t minor_key, QList<uint8_t> chunk)
{
    emit transmit_chunk(major_key, minor_key,
                        GUI_GENERIC_HELPER::qList_to_byteArray(chunk));
}

void GUI_BASE::rcvd_formatted_append(QByteArray data)
{
    rcvd_formatted.write(data);
}

void GUI_BASE::rcvd_formatted_save(QString fileName)
{
    // Select file save location if not provided
    if (fileName.isEmpty() && !GUI_GENERIC_HELPER::getSaveFilePath(&fileName))
        return;

    // Close temporary file (for copying)
    rcvd_formatted.close();

    // Copy file
    if (!GUI_GENERIC_HELPER::copyFile(rcvd_formatted.fileName(), fileName, true))
        GUI_GENERIC_HELPER::showMessage("ERROR: Failed to save file!");

    // Reopen temporary (and go to end)
    rcvd_formatted.open();
    rcvd_formatted.seek(rcvd_formatted.size());
}

QByteArray GUI_BASE::rcvd_formatted_readAll()
{
    // Seek to start
    rcvd_formatted.seek(0);

    // Read all
    QByteArray data = rcvd_formatted.readAll();

    // Seek to end
    rcvd_formatted.seek(rcvd_formatted.size());

    // Return read
    return data;
}

uint32_t GUI_BASE::rcvd_formatted_size()
{
    return rcvd_formatted.size();
}

void GUI_BASE::rcvd_formatted_clear()
{
    rcvd_formatted.resize(0);
}

void GUI_BASE::set_expected_recv_length(uint32_t expected_length)
{
    // Set expecting length
    expected_recv_length = expected_length;

    // Reset current_recv_len
    current_recv_length = 0;

    // Setup recv_length helpers
    expected_recv_length_str.clear();
    if (expected_length)
    {
        expected_recv_length_str = "/" + QString::number(expected_recv_length / 1000.0f) + "KB";
    }

    // Reset progress bar
    emit progress_update_recv(0, "");
}

void GUI_BASE::update_current_recv_length(uint32_t recv_len)
{
    // Exit if expecting not set -or- recv_len is zero (no change)
    if (!expected_recv_length || !recv_len) return;

    // Update received length
    current_recv_length += recv_len;

    // See if finished receiving
    if (expected_recv_length <= current_recv_length)
    {
        // Reset current & expected recv
        current_recv_length = 0;
        expected_recv_length = 0;
        expected_recv_length_str.clear();

        // Emit update
        emit progress_update_recv(100, "Done!");
    } else
    {
        // Update progress bar
        emit progress_update_recv(qRound(((float) current_recv_length / expected_recv_length) * 100.0f),
                                  QString::number((float) current_recv_length / 1000.0f) + expected_recv_length_str);
    }
}

bool GUI_BASE::init_maps()
{
    // Try creating gui_config if not present
    if (!gui_config)
    {
        // Create new config
        gui_config = new CONFIG_MAP();

        // Delete old gui_map if present
        // No config before this so shouldn't have
        // a gui_map object (if there is its old)
        if (gui_map)
        {
            delete gui_map;
            gui_map = nullptr;
        }

        // Verify success or return failure
        if (!gui_config) return false;
    }

    // See if gui_config already has gui_map
    QMap<QString, QVariant> *gui_map_config = gui_config->value(gui_name, nullptr);

    // Handle gui map selection/creation
    if (gui_map_config)
    {
        // If gui_map != gui_map_config, delete gui_map
        if (gui_map && (gui_map != gui_map_config)) delete gui_map;

        // Set gui_map to map from gui_config
        gui_map = gui_map_config;
    } else
    {
        // Create new, blank map if none
        if (!gui_map) gui_map = new QMap<QString, QVariant>();

        // Insert into gui_config
        gui_config->insert(gui_name, gui_map);
    }

    // Return true if both gui_config and gui_map created
    return (gui_config && gui_map);
}
