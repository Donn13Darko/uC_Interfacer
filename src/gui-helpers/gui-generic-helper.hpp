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

#ifndef GUI_GENERIC_HELPER_H
#define GUI_GENERIC_HELPER_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include <QDebug>

typedef enum {
    checksum_name_pos = 0,
    checksum_start_pos,
    checksum_start_base_pos,
    checksum_exe_pos
} checksum_locations;

// Typedef for line length & simplicity
typedef QMap<QString, QMap<QString, QVariant>*> CONFIG_MAP;

class GUI_GENERIC_HELPER : public QObject
{
    Q_OBJECT

public:
    GUI_GENERIC_HELPER(QObject *parent = 0);
    ~GUI_GENERIC_HELPER();

    // Show a message
    static bool showMessage(QString msg);

    // Get user input
    static bool getUserString(QString *str, QString title, QString label);

    // Get file info
    static bool getOpenFilePath(QString *filePath, QString fileTypes = tr("All Files (*)"));
    static bool getSaveFilePath(QString *filePath, QString fileTypes = tr("All Files (*)"));
    static bool saveFile(QString filePath, QByteArray data);
    static bool copyFile(QString pathFrom, QString pathTo, bool overwrite = false);
    static qint64 getFileSize(QString filePath);
    static QByteArray loadFile(QString filePath);

    // Read. parse, delete a config INI
    static CONFIG_MAP *read_configMap(QString config);
    static QString encode_configMap(CONFIG_MAP *configMap);
    static CONFIG_MAP *decode_configMap(QString configMap);
    static CONFIG_MAP *copy_configMap(CONFIG_MAP *configMap);
    static void delete_configMap(CONFIG_MAP **configMap);

    // Conversions
    static QByteArray initList_to_byteArray(std::initializer_list<uint8_t> initList);
    static uint32_t byteArray_to_uint32(QByteArray data);
    static QByteArray uint32_to_byteArray(uint32_t data);
    static QByteArray encode_byteArray(QByteArray data, uint8_t base = 0, char sep = 0);
    static QByteArray decode_byteArray(QByteArray data, uint8_t base = 0, char sep = 0);

    // Variables
    static const float S2MS;
};

#endif // GUI_GENERIC_HELPER_H
