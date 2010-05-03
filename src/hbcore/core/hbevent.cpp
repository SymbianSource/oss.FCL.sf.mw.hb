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

#include "hbevent.h"

const int HbEvent::ChildFocusIn = registerEventType();
const int HbEvent::ChildFocusOut = registerEventType();
const int HbEvent::ThemeChanged = registerEventType();
const int HbEvent::DeviceProfileChanged = registerEventType();
const int HbEvent::SleepModeEnter = registerEventType();
const int HbEvent::SleepModeExit = registerEventType();
const int HbEvent::WindowLayoutDirectionChanged = registerEventType();

/*!
    @stable
    @hbcore
    \class HbEvent
    \brief HbEvent is a class having support for Hb specific events.

    This class supports following extra types of events besides those supported by QEvent


    ChildFocusIn - This event is sent from child widget to parent widget to notify that the child 
                   has got the focus. Normally the parent may show some visualization 
                   change when its child gets focus.

    ChildFocusOut - This event is sent from child widget to parent widget to notify that the child 
                   has lost the focus. Normally the parent may show some visualization 
                   change when its child gets focus.

    ThemeChanged - This event is sent to all QGraphicsWidget. The widget can handle this event by 
                   implementing changeEvent() virtual function.

    DeviceProfileChanged - This event is sent by the system when device's device profile 
                  (screen resolution) changes. The event causes automatic polish call and 
                  the event can be catched in event(QEvent *event) method.

    SleepModeEnter - This event is sent by the system when the phone enters to the sleep mode. 
                     If your application/component needs to take some action when entering the
                     sleep mode then you can do it in event(QEvent *event) method.
    
    SleepModeExit - This event is sent by the system when the phone exist from the sleep mode.
                    If your application/component needs to take some action when exiting the
                    sleep mode then you can do it in event(QEvent *event) method.

    DeviceProfileChanged - This event is sent by the system when layout direction of the main
                           window changes. If your application/component needs to take some
                           action upon this event you can do it in event(QEvent *event) method.

    Example of how to send HbEvent
    \snippet{gui/hbwidget.cpp,1}
    
    Example of how to receive HbEvent
    \snippet{gui/hbwidget.cpp,2}
*/

/*!
    Constructs a HbEvent with \a type.
    \a eventType - type of event
*/
HbEvent::HbEvent(int eventType)
: QEvent((QEvent::Type)eventType)
{
}


/*!
    \class HbDeviceProfileChangedEvent
    \brief The HbDeviceProfileChangedEvent class contains event parameters for device profile changed events.

    Device profile changed events are sent to all widgets in a main window when the device profile of
    that main window changes.
*/

/*!
    Constructs a device profile changed event with the new and old
    device profiles, \a profile and \a oldProfile respectively.
*/
HbDeviceProfileChangedEvent::HbDeviceProfileChangedEvent(
    const HbDeviceProfile &profile,
    const HbDeviceProfile &oldProfile)
    : HbEvent(DeviceProfileChanged), mProfile(profile), mOldProfile(oldProfile)
{
}

/*!
  \internal
*/
HbDeviceProfileChangedEvent::~HbDeviceProfileChangedEvent()
{
}

/*!
    \fn const HbDeviceProfile &HbDeviceProfileChangedEvent::profile() const

    Returns the new device profile.
*/

/*!
    \fn const HbDeviceProfile &HbDeviceProfileChangedEvent::oldProfile() const

    Returns the old device profile.
*/


