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

#include "hbthemeserver_p.h"
#include "hbthemecommon_p.h"
#include "hbthemeserver_symbian_p_p.h"
#include "hbthemecommon_symbian_p.h"

#include <hbmemoryutils_p.h>

#include <QDebug>

/**
 * Constructor
 */

HbThemeServer::HbThemeServer() :
    themeServer(0)
{
}

/**
 * startServer
 */
bool HbThemeServer::startServer()
{
    bool success = false;
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory)
    if (!manager) {
        return success;
    }

    TRAPD(err, themeServer =  HbThemeServerPrivate::NewL(CActive::EPriorityStandard));
    if (KErrNone != err) {
        return success;
    }
    TRAPD(error, themeServer->StartL(KThemeServerName));
    if (KErrNone == error) {
        success = true;
    } else {
        THEME_GENERIC_DEBUG() << Q_FUNC_INFO << "Error Starting SERVER";
    }

    // Parses the device profiles and device modes and stores in the
    // shared memory.
    HbThemeServerUtils::createDeviceProfileDatabase();
    return success;
}

/**
 * Destructor
 */
HbThemeServer::~HbThemeServer()
{
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory)
    if (manager) {
        manager->releaseInstance(HbMemoryManager::SharedMemory);
    }
}
