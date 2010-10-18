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

#ifndef HBSTATUSBAR_P_P_H
#define HBSTATUSBAR_P_P_H

#include "hbstatusbar_p.h"
#include <hbwidget_p.h>

class HbSignalIndicator;
class HbBatteryIndicator;
class HbIndicatorGroup;
class HbIndicatorPrivate;
class HbDeviceDialogServerStatus;

#if defined(Q_OS_SYMBIAN)
class CEnvironmentChangeNotifier;       // Receive system event notifications
#endif

class HbStatusBarPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC(HbStatusBar)

public:
    HbStatusBarPrivate();
    virtual ~HbStatusBarPrivate();

    void delayedConstruction();
    void init();
    void updateTime();
    QString timeFormat();
    void startClockTimer();
    void killClockTimer();
    
#if defined(Q_OS_SYMBIAN)
    static TInt EnvChangeCallback(TAny *aObject);
    TInt DoEnvChange();
#endif    
    
    int mClockTimerId;
    QString mTimeText;
    QGraphicsItem *mTimeTextItem;

    HbSignalIndicator *mSignalIndicator;
    HbBatteryIndicator *mBatteryIndicator;
    HbIndicatorGroup *mNotificationIndicatorGroup;
    HbIndicatorGroup *mSettingsIndicatorGroup;

    HbMainWindow *mMainWindow;
    int mPreviousProperties;

    HbIndicatorPrivate *mIndicatorPrivate;

#if defined(Q_OS_SYMBIAN)    
    // Notifications about locale and time changes
    CEnvironmentChangeNotifier *mEnvChangeNotifier;
#endif    
    
    HbDeviceDialogServerStatus *mServerStatus;

    void _q_signalLevelChanged();
    void _q_batteryLevelChanged();
    void _q_indicatorsChanged();
};


#endif // HBSTATUSBAR_P_P_H
