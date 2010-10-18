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

#include "hbinputserver_p.h"
#include <QDebug>

#include "hbinputserversettings_p.h"
#include "hbinputservermethods_p.h"

HbInputServer::HbInputServer()
    : mSettings(new HbInputServerSettings(this)),
      mMethods(new HbInputServerMethods(this))
{
}

HbInputServer::~HbInputServer()
{
    debugPrint("Stopping input server");
    delete mSettings;
    delete mMethods;
}

/*
Starts the server. Returns true if all services started ok, false otherwise.
*/
bool HbInputServer::start()
{
    debugPrint("Starting input server");
    bool startedOk = false;
    if (mMethods->initialize() && mSettings->initialize()) {
        startedOk = true;
        debugPrint("Input server up and running");
    } else {
        debugPrint("Input server startup failed");
    }
    return startedOk;
}

/*
Prints out the message in a manner dependent on platform and configuration flags.
*/
void HbInputServer::debugPrint(const QString& message)
{
#if defined(ENABLE_INPUTSRVDEBUG)
    qDebug() << "HbInputServer: " << message;
#else
    Q_UNUSED(message);
#endif // ENABLE_INPUTSRVDEBUG

#if !(defined(Q_OS_SYMBIAN))
    emit debugMessage("HbInputServer: " + message);
#endif // Q_OS_SYMBIAN
}
