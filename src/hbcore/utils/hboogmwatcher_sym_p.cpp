/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbCore module of the UI Extensions for Mobile.
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

#include "hboogmwatcher_sym_p.h"
#include "hbiconloader_p.h"
#include <QApplication>
#include <QSymbianEvent>
#include <e32debug.h>
#include <w32std.h>

// Constants copied from goommonitorplugin.hrh.  Including the file is
// not possible because older versions of the file do not have the
// constants and thus would cause a build break with older Symbian
// envs which is not desirable.
#define KGoomMemoryLowEvent 0x10282DBF
#define KGoomMemoryGoodEvent 0x20026790

static QCoreApplication::EventFilter prevEventFilter;

static bool goomEventFilter(void *message, long *result)
{
    QSymbianEvent *symEvent = static_cast<QSymbianEvent *>(message);
    if (symEvent->type() == QSymbianEvent::WindowServerEvent) {
        const TWsEvent *wsEvent = symEvent->windowServerEvent();
        switch (wsEvent->Type()) {

        case KGoomMemoryLowEvent:
            RDebug::Printf("HbOogmWatcher: Goom memory low event");
            HbOogmWatcherPrivate::d_ptr(HbOogmWatcher::instance())->graphicsMemoryLow();
            break;

        case KGoomMemoryGoodEvent:
            RDebug::Printf("HbOogmWatcher: Goom memory good event");
            HbOogmWatcherPrivate::d_ptr(HbOogmWatcher::instance())->graphicsMemoryGood();
            break;

        case EEventWindowVisibilityChanged:
            if (wsEvent->VisibilityChanged()->iFlags & TWsVisibilityChangedEvent::ENotVisible) {
                HbIconLoader::global()->handleForegroundLost();
            }
            break;
        }
    }
    // If there was an event filter set previously then call it. Otherwise
    // return false (let through any events, even the ones handled above).
    return prevEventFilter ? prevEventFilter(message, result) : false;
}

HbOogmWatcherPrivate::HbOogmWatcherPrivate()
{
    // Standard app event filtering is not suitable here, must use
    // setEventFilter to catch the symbian events.
    prevEventFilter = qApp->setEventFilter(goomEventFilter);
}
