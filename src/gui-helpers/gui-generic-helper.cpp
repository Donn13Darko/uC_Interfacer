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

#include "gui-generic-helper.hpp"

#include <QMessageBox>
#include <QInputDialog>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QSettings>
#include <QTemporaryFile>
#include <QTextStream>

const float GUI_GENERIC_HELPER::S2MS = 1000.0f;

GUI_GENERIC_HELPER::GUI_GENERIC_HELPER(QObject *parent) :
    QObject(parent)
{
}

GUI_GENERIC_HELPER::~GUI_GENERIC_HELPER()
{
}

bool GUI_GENERIC_HELPER::showMessage(QString msg)
{
    // Create a new message box & set text
    QMessageBox *n = new QMessageBox();
    if (n)
    {
        // Set messagebox attributes
        n->setText(msg);
        n->setModal(true);

        // Set delete on close attribute
        n->setAttribute(Qt::WA_DeleteOnClose);

        // Show message box
        n->show();
    }

    // Return if successful
    return n;
}

bool GUI_GENERIC_HELPER::getUserString(QString *str, QString title, QString label)
{
    bool ok;
    *str = QInputDialog::getText(nullptr, title, label,
                                 QLineEdit::Normal, *str, &ok);
    return (ok && !str->isEmpty());
}

bool GUI_GENERIC_HELPER::getOpenFilePath(QString *filePath, QString fileTypes)
{
    *filePath = QFileDialog::getOpenFileName(nullptr, tr("Open"),
                                             "", fileTypes);

    return !filePath->isEmpty();
}

bool GUI_GENERIC_HELPER::getSaveFilePath(QString *filePath, QString fileTypes)
{
    *filePath = QFileDialog::getSaveFileName(nullptr, tr("Save Location"),
                                             "", fileTypes);

    return !filePath->isEmpty();
}

bool GUI_GENERIC_HELPER::saveFile(QString filePath, QByteArray data)
{
    // Check to make sure path valid
    if (filePath.isEmpty())
        return false;

    // Open file
    QFile sFile(filePath);
    if (!sFile.open(QIODevice::WriteOnly))
        return false;

    // Write data to file
    qint64 res = sFile.write(data);
    sFile.close();

    // Check result
    return (0 <= res);
}

bool GUI_GENERIC_HELPER::copyFile(QString pathFrom, QString pathTo, bool overwrite)
{
    // Verify pathFrom exists
    if (!QFile::exists(pathFrom)) return false;

    // Check if exits (delete it before copy so copy doesn't overwrite)
    if (QFile::exists(pathTo))
    {
        // Check that we want to overwrite
        if (!overwrite) return false;

        // Delete old file
        QFile::remove(pathTo);
    }

    // Copy file
    return QFile::copy(pathFrom, pathTo);
}

qint64 GUI_GENERIC_HELPER::getFileSize(QString filePath)
{
    QFileInfo file(filePath);
    return file.size();
}

QByteArray GUI_GENERIC_HELPER::loadFile(QString filePath)
{
    // Check to make sure path valid
    if (filePath.isEmpty() || !QFile::exists(filePath))
        return QByteArray();

    // Open file
    QFile sFile(filePath);
    if (!sFile.open(QIODevice::ReadOnly))
        return QByteArray();

    // Read file
    QByteArray data = sFile.readAll();

    // Close file
    sFile.close();

    // Return read data
    return data;
}

CONFIG_MAP *GUI_GENERIC_HELPER::read_configMap(QString config)
{
    // Reset & load the GUI settings file
    QSettings config_settings(config, QSettings::IniFormat);

    QMap<QString, QVariant> *groupMap = nullptr;
    CONFIG_MAP *configMap;
    configMap = new CONFIG_MAP();
    if (!configMap) return nullptr;

    // Loop through all child groups
    foreach (QString childGroup, config_settings.childGroups())
    {
        // Create new group map
        groupMap = new QMap<QString, QVariant>();
        if (!groupMap) continue;

        // Begin GUI group settings
        config_settings.beginGroup(childGroup);
        foreach (QString childKey, config_settings.childKeys())
        {
            groupMap->insert(childKey, config_settings.value(childKey));
        }

        // Exit GUI group settings
        config_settings.endGroup();

        // Add groupMap to configMap
        configMap->insert(childGroup, groupMap);
    }

    // Setup and open file (for parsing empty keys)
    QFile in_file(config);
    if (!in_file.open(QIODevice::ReadWrite))
    {
        return configMap;
    }

    // Setup variables
    QString line, group_str;
    QTextStream in(&in_file);

    // Search for empty groups (auto filtered by QSettings)
    do
    {
        // Get next line
        line = in.readLine().trimmed();

        // Check if matches group line
        if (line.startsWith('[')
                && line.endsWith(']'))
        {
            // Get group name
            group_str = line.mid(1, line.length()-2);

            // Check if in map
            if (!configMap->contains(group_str))
            {
                // Create new group map
                groupMap = new QMap<QString, QVariant>();
                if (!groupMap) continue;

                // Add groupMap to configMap
                configMap->insert(group_str, groupMap);
            }
        }
    } while (!line.isNull());

    // Close file
    in_file.close();

    // Return parsed data
    return configMap;
}

QString GUI_GENERIC_HELPER::encode_configMap(CONFIG_MAP *configMap)
{
    // Verify configMap
    if (!configMap) return "";

    // Create qstring to hold config
    QString configMap_str;
    QVariant arg_str;

    // Write all values in the configMap
    QMap<QString, QVariant>* groupMap;
    foreach (QString group, configMap->keys())
    {
        // Get groupMap, skip if nullptr
        groupMap = configMap->value(group);
        if (!groupMap) continue;

        // Add group map to string conversion
        configMap_str += "[" + group + "]\n";

        // Write key and value for each entry in groupMap
        foreach (QString groupKey, groupMap->keys())
        {
            // Add setting key name and start assignment to string
            configMap_str += groupKey + "=\"";

            // Get the current value
            arg_str = groupMap->value(groupKey);

            // Parse value
            if (arg_str.canConvert(QMetaType::QString))
            {
                // Add value to assignment
                configMap_str += arg_str.toString();
            } else if (arg_str.canConvert(QMetaType::QStringList))
            {
                // Join the list with "," and add to assignment
                configMap_str += arg_str.toStringList().join("\",\"");
            }

            // Add end assignment to string
            configMap_str += "\"\n";
        }

        // Add ending newline
        configMap_str += "\n";
    }

    // Return the config data
    return configMap_str;
}

CONFIG_MAP *GUI_GENERIC_HELPER::decode_configMap(QString configMap)
{
    // Create temporary file & set autoremove
    QTemporaryFile tmpINI;
    tmpINI.setAutoRemove(true);

    // Open temporary file for writing data
    if (!tmpINI.open())
        return nullptr;

    // Write data to temporary file & close
    tmpINI.write(configMap.toLatin1());
    QString tmpName = tmpINI.fileName();
    tmpINI.close();

    // Read in file, create config, and return pointer
    return GUI_GENERIC_HELPER::read_configMap(tmpName);
}

CONFIG_MAP *GUI_GENERIC_HELPER::copy_configMap(CONFIG_MAP *configMap)
{
    // Verify valid pointer
    if (!configMap) return nullptr;

    QMap<QString, QVariant> *groupMap, *groupMap_copy;
    CONFIG_MAP *configMap_copy;
    configMap_copy = new CONFIG_MAP();
    if (!configMap_copy) return nullptr;

    // Loop through all child groups
    foreach (QString childGroup, configMap->keys())
    {
        // Get original child group
        groupMap = configMap->value(childGroup);
        if (!groupMap)
        {
            // Add nullptr to configMap_copy & continue
            configMap_copy->insert(childGroup, nullptr);
            continue;
        }

        // Create new group map
        groupMap_copy = new QMap<QString, QVariant>();
        if (!groupMap_copy) continue;

        // Read each group
        foreach (QString childKey, groupMap->keys())
        {
            groupMap_copy->insert(childKey, groupMap->value(childKey));
        }

        // Add groupMap_copy to configMap_copy
        configMap_copy->insert(childGroup, groupMap_copy);
    }

    // Return the copy
    return configMap_copy;
}

void GUI_GENERIC_HELPER::delete_configMap(CONFIG_MAP **configMap)
{
    // Verify valid pointer
    if (!configMap) return;

    // Get direct pointer (instead of pointer to a pointer
    CONFIG_MAP *configMap_ptr = *configMap;
    if (!configMap_ptr) return;

    // Loop through elements deleting
    QMap<QString, QVariant> *groupMap;
    foreach (QString group, configMap_ptr->keys())
    {
        groupMap = configMap_ptr->value(group);
        configMap_ptr->remove(group);
        if (groupMap) delete groupMap;
    }

    // Delete main config map
    delete *configMap;

    // Assign pointer to null
    *configMap = nullptr;
}

QByteArray GUI_GENERIC_HELPER::qList_to_byteArray(QList<uint8_t> initList)
{
    QByteArray init_array;
    foreach (uint8_t i, initList)
    {
        init_array.append(i);
    }

    return init_array;
}

uint32_t GUI_GENERIC_HELPER::byteArray_to_uint32(QByteArray data)
{
    uint32_t ret_data = 0;
    uint8_t data_len = data.length();
    for (uint8_t i = 0; ((i < 4) && (i < data_len)); i++)
    {
        ret_data = (ret_data << 8) | (uchar) data.at(i);
    }
    return ret_data;
}

QByteArray GUI_GENERIC_HELPER::uint32_to_byteArray(uint32_t data)
{
    QByteArray ret_data;
    for (uint8_t i = 0; i < 4; i++)
    {
        ret_data.prepend((char) (data & 0xFF));
        data = data >> 8;
    }
    return ret_data;
}

QByteArray GUI_GENERIC_HELPER::encode_byteArray(QByteArray data, uint8_t base, char sep)
{
    // If base is 0, return data as is
    if ((base == 0) || (data.length() == 0)) return data;

    // Otherwise, build return string from data
    QByteArray ret_data;
    foreach (uint8_t elem, data)
    {
        // Append sep + next element
        ret_data.append(sep);
        ret_data.append(QString::number(elem, base));
    }

    // Remove first sep char
    ret_data.remove(0, 1);

    // Return encoding
    return ret_data;
}

QByteArray GUI_GENERIC_HELPER::decode_byteArray(QByteArray data, uint8_t base, char sep)
{
    // If base is 0, return data as is
    if (base == 0) return data;

    // Otherwise, build return array from data
    // Splits array using sep char and trys to convert each string
    // into a number
    QByteArray ret_data;
    foreach (QString elem, data.split(sep))
    {
        ret_data.append((char) elem.toInt(nullptr, base));
    }
    return ret_data;
}
