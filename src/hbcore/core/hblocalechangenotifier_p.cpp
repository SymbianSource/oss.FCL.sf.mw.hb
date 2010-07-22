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
#include <QLocale>
#include <QCoreApplication>
#include <hbinstance.h>

#ifdef Q_OS_SYMBIAN
#include <e32std_private.h>
#endif // Q_OS_SYMBIAN

#include "hblocalechangenotifier_p.h"
#include "hbfeaturemanager_r.h"


#ifdef Q_OS_SYMBIAN
HbLocaleChangeNotifier::HbLocaleChangeNotifier()
    : CActive(CActive::EPriorityStandard)
{
    CActiveScheduler::Add(this);
    iStatus = KRequestPending;
    iProperty.Attach(KUidSystemCategory, KLocaleLanguageKey);
    iProperty.Subscribe(iStatus);
    SetActive();
}

#else // Q_OS_SYMBIAN

HbLocaleChangeNotifier::HbLocaleChangeNotifier()
{
}
#endif // Q_OS_SYMBIAN


HbLocaleChangeNotifier::~HbLocaleChangeNotifier()
{
#ifdef Q_OS_SYMBIAN
    Cancel();
    iProperty.Close();
#endif // Q_OS_SYMBIAN
}

#ifdef Q_OS_SYMBIAN
void HbLocaleChangeNotifier::RunL()
{

    iProperty.Subscribe(iStatus);
    SetActive();

    // To reset QLocale's cached data
    QSystemLocale dummy;
    
    if ( HbFeatureManager::instance()->featureStatus(HbFeatureManager::LanguageSwitch) ) {
        foreach (HbMainWindow *window, HbInstance::instance()->allMainWindows()) {
            QApplication::postEvent(window, new QEvent(QEvent::LocaleChange));
        }
    }
}
#endif // Q_OS_SYMBIAN

#ifdef Q_OS_SYMBIAN
void HbLocaleChangeNotifier::DoCancel()
{
    iProperty.Cancel();
}
#endif // Q_OS_SYMBIAN


