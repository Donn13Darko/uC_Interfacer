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

#include "gui-helper.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QSettings>

const float GUI_HELPER::S2MS = 1000.0f;

GUI_HELPER::GUI_HELPER(QObject *parent) :
    QObject(parent)
{
}

GUI_HELPER::~GUI_HELPER()
{
}

bool GUI_HELPER::showMessage(QString msg)
{
    QMessageBox n;
    n.setText(msg);
    return n.exec();
}

bool GUI_HELPER::getUserString(QString *str, QString title, QString label)
{
    bool ok;
    *str = QInputDialog::getText(nullptr, title, label,
                                 QLineEdit::Normal, *str, &ok);
    return (ok && !str->isEmpty());
}

bool GUI_HELPER::getOpenFilePath(QString *filePath, QString fileTypes)
{
    *filePath = QFileDialog::getOpenFileName(nullptr, tr("Open"),
                                             "", fileTypes);

    return !filePath->isEmpty();
}

bool GUI_HELPER::getSaveFilePath(QString *filePath, QString fileTypes)
{
    *filePath = QFileDialog::getSaveFileName(nullptr, tr("Save Location"),
                                             "", fileTypes);

    return !filePath->isEmpty();
}

bool GUI_HELPER::saveFile(QString filePath, QByteArray data)
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

uint32_t GUI_HELPER::getFileSize(QString filePath)
{
    QFileInfo file(filePath);
    return file.size();
}

QByteArray GUI_HELPER::loadFile(QString filePath)
{
    // Check to make sure path valid
    if (filePath.isEmpty())
        return QByteArray();

    // Open file
    QFile sFile(filePath);
    if (!sFile.open(QIODevice::ReadOnly))
        return QByteArray();

    // Read file
    QByteArray data = sFile.readAll();
    sFile.close();

    // Return read data
    return data;
}

QMap<QString, QMap<QString, QVariant>*>* GUI_HELPER::readConfigINI(QString config)
{
    // Reset & load the GUI settings file
    QSettings config_settings(config, QSettings::IniFormat);

    QMap<QString, QVariant>* groupMap;
    QMap<QString, QMap<QString, QVariant>*>* configMap;
    configMap = new QMap<QString, QMap<QString, QVariant>*>();

    // Loop through all child groups
    foreach (QString childGroup, config_settings.childGroups())
    {
        // Create new group map
        groupMap = new QMap<QString, QVariant>();

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

    return configMap;
}

void GUI_HELPER::deleteConfigMap(QMap<QString, QMap<QString, QVariant> *>* configMap)
{
    QMap<QString, QVariant>* groupMap;
    foreach (QString group, configMap->keys())
    {
        groupMap = configMap->value(group);
        configMap->remove(group);
        delete groupMap;
    }
    delete configMap;
}

QByteArray GUI_HELPER::initList_to_byteArray(std::initializer_list<uint8_t> initList)
{
    QByteArray init_array;
    foreach (char i, initList)
    {
        init_array.append(i);
    }

    return init_array;
}

uint32_t GUI_HELPER::byteArray_to_uint32(QByteArray data)
{
    uint32_t ret_data = 0;
    uint8_t data_len = data.length();
    for (uint8_t i = 0; ((i < 4) && (i < data_len)); i++)
    {
        ret_data = (ret_data << 8) | (uchar) data.at(i);
    }
    return ret_data;
}

QByteArray GUI_HELPER::uint32_to_byteArray(uint32_t data)
{
    QByteArray ret_data;
    for (uint8_t i = 0; i < 4; i++)
    {
        ret_data.prepend((char) (data & 0xFF));
        data = data >> 8;
    }
    return ret_data;
}
