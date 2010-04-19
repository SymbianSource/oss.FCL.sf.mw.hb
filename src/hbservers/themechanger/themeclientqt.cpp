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
#include <QLocalSocket>
#include <QDebug>
#include <hbinstance.h>

#include "themeclientqt.h"
#include "themechangerdefs.h"
#include "hbthemecommon_p.h"

/**
 * Constructor
 */
ThemeClientQt::ThemeClientQt()
{
#ifdef THEME_CHANGER_TRACES
    qDebug() << Q_FUNC_INFO;
#endif
    localSocket = new QLocalSocket();
    connected = false;
}

bool ThemeClientQt::connectToServer()
{
    localSocket->connectToServer(THEME_SERVER_NAME);
    bool success = localSocket->waitForConnected(3000);
#ifdef THEME_CHANGER_TRACES
    qDebug() << "ThemeClientQt:: Socket Connect status: " <<success;
#endif
    connected = success;
    return success;
}

/**
 * Destructor
 */
ThemeClientQt::~ThemeClientQt()
{
    if (connected) {
        localSocket->disconnectFromServer();
    }
    delete localSocket;
}

/**
 * changeTheme
 */
void ThemeClientQt::changeTheme(const QString &newtheme)
{
#ifdef THEME_CHANGER_TRACES
    qDebug() <<"ThemeClientQt::changeTheme("<<newtheme<<") called";
#endif
    if( (themeName==newtheme) || (newtheme =="") ) {
#ifdef THEME_CHANGER_TRACES
        qDebug() <<"ThemeClientQt:: return Sametheme applied";
#endif
        return;
    }
    QByteArray outputByteArray;
    QDataStream outputDataStream(&outputByteArray, QIODevice::WriteOnly);
    HbThemeServerRequest requestType = EThemeSelection;
    outputDataStream << (int)requestType;
    outputDataStream << newtheme;
    themeName = newtheme;
    localSocket->write(outputByteArray);
#ifdef THEME_CHANGER_TRACES
    qDebug()<<"ThemeClientQt::ThemeName written to server";
#endif
    localSocket->flush();
}


/**
 * isConnected
 */
bool ThemeClientQt::isConnected()
{
    return connected;
}
