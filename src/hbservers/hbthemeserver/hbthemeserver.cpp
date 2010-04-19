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
#include <QLabel>
#include <QDebug>

#include "hbthemecommon_p.h"
#include <hbmemoryutils_p.h>
#ifdef Q_OS_SYMBIAN
#include "hbthemeserver_symbian_p_p.h"
#include "hbthemecommon_symbian_p.h"
#else
#include "hbthemeserver_generic_p_p.h"
#endif

#ifdef THEME_SERVER_TRACES
QLabel *testLabel = 0;
#endif

/**
 * Constructor
 */

#ifdef Q_OS_SYMBIAN
HbThemeServer::HbThemeServer(): themeServer(0)
#else
HbThemeServer::HbThemeServer(QWidget *parent): QMainWindow(parent), themeServer(0)
#endif
{
#ifndef Q_OS_SYMBIAN
    statusLabel = new QLabel;
    statusLabel->setText("Theme Server Started");
    setCentralWidget(statusLabel);
    setWindowTitle("Theme Server");
#ifdef THEME_SERVER_TRACES
    testLabel = statusLabel;
#endif
#endif
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
#ifdef Q_OS_SYMBIAN
    TRAPD(err, themeServer =  HbThemeServerPrivate::NewL(CActive::EPriorityStandard));
    if (KErrNone != err) {
        return success;
    }
    TRAPD(error, themeServer->StartL(KThemeServerName));
    if (KErrNone == error) {
        success = true;
    } else {
#ifdef THEME_SERVER_TRACES
        qDebug() << Q_FUNC_INFO << "Error Starting SERVER";
#endif
    }
#else
    QT_TRY {
        themeServer = new HbThemeServerPrivate();
    } QT_CATCH(const std::bad_alloc &) {
        return success;
    }
    success = themeServer->start();
    if (!success) {
#ifdef THEME_SERVER_TRACES
        qDebug() << Q_FUNC_INFO << "Error Starting SERVER";
#endif
    }
#endif

    // Parses the device profiles and device modes and stores in the
    // shared memory.
    HbThemeServerUtils::createDeviceProfileDatabase();

    //Creates the secondary Cache.
    HbThemeServerUtils::createSharedCache();
    return success;
}

/**
 * Destructor
 */
HbThemeServer::~HbThemeServer()
{
    delete themeServer;
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory)
    if (manager) {
        manager->releaseInstance(HbMemoryManager::SharedMemory);
    }
}
