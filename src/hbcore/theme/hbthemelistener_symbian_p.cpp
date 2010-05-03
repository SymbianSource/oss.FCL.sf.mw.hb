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


#include <qglobal.h>
#include <QDebug>
#include <hbinstance.h>
#include <hbtheme_p.h>

#ifdef THEME_LISTENER_TRACES
#include <hbmemoryutils_p.h>
#endif

#include "hbthemeclient_p_p.h"
#include "hbthemelistener_symbian_p.h"
#include "hbthemecommon_symbian_p.h"

// app uid of theme application
const TInt KThemeName = 0;

/**
 * Constructor
 */
CHbThemeListenerPrivate::CHbThemeListenerPrivate(HbThemeClientPrivate *themeClient)
                      : CActive(EPriorityNormal), themeClient(themeClient)
{
    User::LeaveIfError(themeState.Attach(KServerUid3,KThemeName));
    CActiveScheduler::Add(this);
    // initial subscription
    themeState.Subscribe(iStatus);
    SetActive();
}

/**
 * Destructor
 */
CHbThemeListenerPrivate::~CHbThemeListenerPrivate()
{
    Cancel();
    themeState.Close();
}

/**
 * RunL
 */
void CHbThemeListenerPrivate::RunL()
{
#ifdef THEME_LISTENER_TRACES
    qDebug() << "CHbThemeListenerPrivate::RunL: start\n appname"<<HbMemoryUtils::getCleanAppName();
#endif
    themeState.Subscribe(iStatus);
    SetActive();    
    // Added RProperty::Get() as workaround for QSettings bug on symbian
    // bug: QSettings does not get synched if values are changes fast at server side
    // @todo: XQsettingManager instead of QSetting for symbian
    TBuf<80> name;
    TInt r = themeState.Get(name);
    QString str((QChar*)name.Ptr(),name.Length());
#ifdef THEME_LISTENER_TRACES
    if (r==KErrNone) {
        qDebug() << "CHbThemeListenerPrivate::RunL: Get() Themename" <<str; 
    } else {
        qDebug() << "CHbThemeListenerPrivate::RunL: Get() Error!!!!!!!!";
    }
#endif
    themeClient->handleThemeChange(str);
}


/**
 * DoCancel
 */
void CHbThemeListenerPrivate::DoCancel()
{
    themeState.Cancel();
}

