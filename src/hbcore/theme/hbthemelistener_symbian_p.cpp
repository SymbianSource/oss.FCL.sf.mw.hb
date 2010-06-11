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
#include "hbthemelistener_symbian_p.h"
#include "hbthemeclient_p_p.h"
#include "hbthemecommon_symbian_p.h"

#include <qglobal.h>
#include <QDebug>
#include <hbinstance.h>
#include <hbtheme_p.h>
#include <hbthemeutils_p.h>

#ifdef THEME_LISTENER_TRACES
#include <hbmemoryutils_p.h>
#endif

/**
 * Constructor
 */
CHbThemeListenerPrivate::CHbThemeListenerPrivate(HbThemeClientPrivate *themeClient)
                      : CActive(EPriorityNormal), themeClient(themeClient)
{
    mRepository = CRepository::NewL(KServerUid3);
    mRepository->NotifyRequest(HbThemeUtils::CurrentThemeSetting, iStatus);
    CActiveScheduler::Add(this);
    SetActive();
}

/**
 * Destructor
 */
CHbThemeListenerPrivate::~CHbThemeListenerPrivate()
{
    Cancel();
    delete mRepository;
}

/**
 * RunL
 */
void CHbThemeListenerPrivate::RunL()
{
#ifdef THEME_LISTENER_TRACES
    qDebug() << "CHbThemeListenerPrivate::RunL: start\n appname"<<HbMemoryUtils::getCleanAppName();
#endif
    mRepository->NotifyRequest(HbThemeUtils::CurrentThemeSetting, iStatus);
    SetActive();    
    HbThemeIndexInfo info = HbThemeUtils::getThemeIndexInfo(ActiveTheme);
    if (info.themeIndexOffset > 0) {
        themeClient->handleThemeChange(info.name);
        return;
    }
    // Fallback to reading cenrep
    TBuf<256> newTheme;
    if (KErrNone == mRepository->Get(HbThemeUtils::CurrentThemeSetting, newTheme)) {
        QString qnewTheme((QChar*)newTheme.Ptr(), newTheme.Length());
        themeClient->handleThemeChange(qnewTheme);
    }
}

/**
 * DoCancel
 */
void CHbThemeListenerPrivate::DoCancel()
{
    mRepository->NotifyCancelAll();
}
