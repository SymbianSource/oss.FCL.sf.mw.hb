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

#ifndef HBSLEEPMODELISTENER_P_P_H
#define HBSLEEPMODELISTENER_P_P_H

#include <hbapplication.h>

#if defined(Q_OS_SYMBIAN)

#include <e32base.h>
#include <e32property.h>

/**
* Data structure used to handle light status information.
*/
struct THWRMStatusInfo {
    // Light target.
    TInt iTarget;

    // Status of the target.
    TInt iStatus;
};

/**
* Defines THWRMStatusInfo array.
*/
typedef RArray<THWRMStatusInfo> RLightStatusArray;

class CSleepModeListenerPrivate: public CActive
{
public:
    CSleepModeListenerPrivate();
    ~CSleepModeListenerPrivate();
private:
    void RunL();
    void DoCancel();

    TInt InitializeStatusArray(RLightStatusArray &aArray) const;

    static TInt FindByTarget(const TInt *aTarget, const THWRMStatusInfo &aItem);
private:
    RProperty sleepModeState;
};

#else

class HB_AUTOTEST_EXPORT HbSleepModeListenerPrivate
{
public:
    HbSleepModeListenerPrivate();
    ~HbSleepModeListenerPrivate();
};

#endif

#endif // HBSLEEPMODELISTENER_P_P_H
