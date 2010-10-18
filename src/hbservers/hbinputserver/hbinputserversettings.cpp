/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbServers module of the UI Extensions for Mobile.
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at developer.feedback@nokia.com.
**
****************************************************************************/

#include "hbinputserversettings_p.h"

#include <QFile>
#include <QDir>
#include <hbinputsettingproxy.h>
#include <hbinputsettingproxy_p.h>

HbInputServerSettings::HbInputServerSettings(HbInputServer *server)
    : mServer(server),
      mSettings(0)
{
}

HbInputServerSettings::~HbInputServerSettings()
{
    if (mSettings) {
        mSettings->lock();
        saveToDisk();
        mSettings->unlock();
        delete mSettings;
    }
}

/*
Initializes input settings shared memory. If the shared memory already exists,
attaches to it and refreshes the settings from disk.

Returns true on success.
*/
bool HbInputServerSettings::initialize()
{
    mServer->debugPrint("Initializing settings");
    mSettings = new QSharedMemory(HbInputSettingsSharedMemoryKey);

    if (!mSettings->create(sizeof(HbSettingProxyInternalData))) {
        mServer->debugPrint("Settings shared memory already exists!");
        mSettings->attach();
    }

    mSettings->lock();
    if (!initializeFromDisk()) {
        initializeWithDefaults();
    }
    mSettings->unlock();
    mServer->debugPrint("Settings initialized");
    return true;
}

/*
Reads input settings from disk. Returns true if successful,
false if the file cannot be read.
*/
bool HbInputServerSettings::initializeFromDisk()
{
    mServer->debugPrint("Reading input settings from disk");

    if (HbInputSettingProxyPrivate::load(settingsData())) {
        mServer->debugPrint("Input settings successfully read from disk");
        return true;
    } else {
        mServer->debugPrint("Input settings could not be read from disk");
        return false;
    }
}

/*
Saves input settings to disk.
*/
void HbInputServerSettings::saveToDisk()
{
    mServer->debugPrint("Saving input settings to disk");
    HbInputSettingProxyPrivate::save(settingsData());
    mServer->debugPrint("Input settings saved");
}

/*
Fills shared data with default settings values.
*/
void HbInputServerSettings::initializeWithDefaults()
{
    HbSettingProxyInternalData *data = settingsData();
    if (data) {
        HbInputSettingProxyPrivate::writeDefaultValuesToData(data);
        mServer->debugPrint("Input settings initialized with defaults");
    }
}

/*
Returns pointer to the settings data inside shared memory.
*/
HbSettingProxyInternalData *HbInputServerSettings::settingsData() const
{
    return static_cast<HbSettingProxyInternalData *>(mSettings->data());
}
