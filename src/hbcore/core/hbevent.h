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

#ifndef HB_EVENT_H
#define HB_EVENT_H
#include <QEvent>
#include <hbglobal.h>
#include <hbdeviceprofile.h>

QT_BEGIN_NAMESPACE

class QGraphicsWidget;

QT_END_NAMESPACE

class HB_CORE_EXPORT HbEvent: public QEvent
{
public:
    // Hb specific events
    static const int ChildFocusIn;
    static const int ChildFocusOut;
    static const int ThemeChanged;
    static const int DeviceProfileChanged;
    static const int SleepModeEnter;
    static const int SleepModeExit;
    static const int WindowLayoutDirectionChanged;
    static const int InputMethodFocusIn;
    static const int InputMethodFocusOut;
    static const int WindowObscuredChanged;
    HbEvent(int eventType);
};


class HB_CORE_EXPORT HbDeviceProfileChangedEvent: public HbEvent
{
public:
    HbDeviceProfileChangedEvent(const HbDeviceProfile &profile, const HbDeviceProfile &oldProfile);
    ~HbDeviceProfileChangedEvent();

    inline const HbDeviceProfile &profile() const {
        return mProfile;
    }
    inline const HbDeviceProfile &oldProfile()const {
        return mOldProfile;
    }
protected:
    HbDeviceProfile mProfile, mOldProfile;
};

class HB_CORE_EXPORT HbWindowObscuredChangedEvent: public HbEvent
{
public:
    HbWindowObscuredChangedEvent(bool obscuredState);
    ~HbWindowObscuredChangedEvent();

    inline bool obscuredState() const {
        return mObscuredState;
    }
protected:
    bool mObscuredState;
};

#endif // HB_EVENTS_H

