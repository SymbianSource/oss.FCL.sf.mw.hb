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

#ifndef HBSPLASHBLACKLIST_P_H
#define HBSPLASHBLACKLIST_P_H

#include <QList>

// List of apps that should use Hb::NoSplash but they don't.
// Showing a splash screen for these does not make sense anyway.
inline QList<quint32> hbsplash_blacklist()
{
    return QList<quint32>()

           // started on phone boot
           << 0x20022f35 // app/homescreen/homescreenapp/hsapplication
           << 0x100058b3 // app/phone/phoneapp/phoneui2
           << 0x2002e67a // app/phone/phoneengine/networkhandlingstarter

           // uses hbapplication but not hbmainwindow
           << 0x1028339d // app/devicecontrol/deviceupdateui/deviceupdateqtsp
           << 0x2001fe74 // app/messaging/messagingapp/msgnotifications/msgerrornotifier
           << 0x2002ff8a // mw/phonesrv/telutils/keysequencerecognitionservice

           // started to background and may not need splash anyway
           << 0x2002e669 // mw/webruntime/app/widget/wrtwidgetui

           // probably launched on first boot, including for now to prevent mess in startup sequence
           << 0x20026f95 // app/firsttimeuse/ftuapplication

           ;
}

#endif
