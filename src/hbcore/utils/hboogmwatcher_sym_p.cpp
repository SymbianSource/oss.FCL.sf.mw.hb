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
#include <hbinstance.h>
#include <hbmainwindow.h>
#include <coecntrl.h>
#include <e32debug.h>
#include <QMetaObject>

HbOogmWatcherPrivate::HbOogmWatcherPrivate()
{
#ifdef HB_OOGM_ALF
    mInitialized = false;
    setupListener(); // there may be no mainwindow at this point but we must try anyway
#endif
}

void HbOogmWatcherPrivate::mainWindowReady()
{
    // This function is invoked by the framework when a mainwindow is registered to the
    // HbInstance. It has to invoke setupListener() asynchronously otherwise the creation
    // of the CAlfCompositionSource may fail.
    if (!mInitialized) {
        QMetaObject::invokeMethod(this, "setupListener", Qt::QueuedConnection);
    }
}

void HbOogmWatcherPrivate::setupListener()
{
#ifdef HB_OOGM_ALF
    mCompositionSource = 0;
    QList<HbMainWindow *> mainWindows = hbInstance->allMainWindows();
    if (!mainWindows.isEmpty()) {
        RWindow *win = static_cast<RWindow *>(mainWindows.at(0)->effectiveWinId()->DrawableWindow());
        TInt err = win ? KErrNone : KErrNotFound;
        if (win)
            TRAP(err, mCompositionSource = CAlfCompositionSource::NewL(*win));
        if (err == KErrNone) {
            TRAP(err, mCompositionSource->AddCompositionObserverL(*this));
            if (err != KErrNone) {
                RDebug::Printf("HbOogmWatcher: Cannot add composition observer (%d)", err);
                delete mCompositionSource;
                mCompositionSource = 0;
            } else {
                mInitialized = true;
                RDebug::Printf("HbOogmWatcher: Initialized successfully");
            }
        } else {
            RDebug::Printf("HbOogmWatcher: Cannot create CAlfCompositionSource (%d, RWindow=%x)", err, (int) win);
        }
    } else {
        RDebug::Printf("HbOogmWatcher: No mainwindows available");
    }
#endif
}

HbOogmWatcherPrivate::~HbOogmWatcherPrivate()
{
#ifdef HB_OOGM_ALF
    delete mCompositionSource;
#endif
}

#ifdef HB_OOGM_ALF
void HbOogmWatcherPrivate::RunningLowOnGraphicsMemory()
{
    RDebug::Printf("HbOogmWatcher: RunningLowOnGraphicsMemory()");
    graphicsMemoryLow();
}
#endif
