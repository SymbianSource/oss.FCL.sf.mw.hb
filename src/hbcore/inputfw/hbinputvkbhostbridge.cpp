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
#include <QVariant>
#include <QPointer>

#include "hbinputvkbhostbridge.h"

/*!
\proto
\class HbVkbHostBridge
\brief Provides one connection point to receive signals from all the virtual keyboard hosts.

The virtual keyboard host provides signals when the keyboard changes state. Usually there are many active virtual
keyboard hosts in the system so if an application is interested to know when any of them changes state
(ie. keyboard opens or closes), it is difficult to know which one to connect to.

This singleton class acts as a gateway and delivers signals when any of the virtual keyboard hosts
in the system changes state. It also forwards HbVkbHost API function calls to active host.

Note that there is active host only when editor widget is focused.

The bridge virtual keyboard host also has has an important internal role in synchronising state transitions between several
active virtual keyboard hosts.  

\sa HbVkbHost
*/

/// @cond

class HbVkbHostBridgePrivate
{
public:
    QPointer<HbVkbHost> mActiveHost;
};

/// @endcond

HbVkbHostBridge *HbVkbHostBridge::instance()
{
    static HbVkbHostBridge theInstance;
    return &theInstance;
}

HbVkbHostBridge::HbVkbHostBridge() : d_ptr(new HbVkbHostBridgePrivate())
{
}

HbVkbHostBridge::~HbVkbHostBridge()
{
    delete d_ptr;
}

/*!
\reimp
*/
void HbVkbHostBridge::openKeypad(HbVirtualKeyboard *vkb, HbInputMethod* owner, bool animationAllowed)
{
    Q_D(HbVkbHostBridge);
    if (d->mActiveHost) {
        d->mActiveHost->openKeypad(vkb, owner, animationAllowed);
    }
}

/*!
\reimp
*/
void HbVkbHostBridge::openMinimizedKeypad(HbVirtualKeyboard *vkb, HbInputMethod* owner)
{
    Q_D(HbVkbHostBridge);
    if (d->mActiveHost) {
        d->mActiveHost->openMinimizedKeypad(vkb, owner);
    }
}

/*!
\reimp
*/
void HbVkbHostBridge::closeKeypad(bool animationAllowed)
{
    Q_D(HbVkbHostBridge);

    if (d->mActiveHost) {
        d->mActiveHost->closeKeypad(animationAllowed);
    }
}

/*!
\reimp
*/
void HbVkbHostBridge::minimizeKeypad(bool animationAllowed)
{
    Q_D(HbVkbHostBridge);
    if (d->mActiveHost) {
        d->mActiveHost->minimizeKeypad(animationAllowed);
    }
}

/*!
\reimp
*/
HbVkbHost::HbVkbStatus HbVkbHostBridge::keypadStatus() const
{
    Q_D(const HbVkbHostBridge);
    if (d->mActiveHost) {
        return d->mActiveHost->keypadStatus();
    }

    return HbVkbHost::HbVkbStatusClosed;
}

/*!
\reimp
*/ 
QSizeF HbVkbHostBridge::keyboardArea() const
{
    Q_D(const HbVkbHostBridge);
    if (d->mActiveHost) {
        return d->mActiveHost->keyboardArea();
    }

    return QSizeF();
}

/*!
\reimp
*/
HbVirtualKeyboard* HbVkbHostBridge::activeKeypad() const
{
    Q_D(const HbVkbHostBridge);
    if (d->mActiveHost) {
        return d->mActiveHost->activeKeypad();
    }

    return 0;
}

/*!
\reimp
*/
QRectF HbVkbHostBridge::applicationArea() const
{
    Q_D(const HbVkbHostBridge);
    if (d->mActiveHost) {
        return d->mActiveHost->applicationArea();
    }

    return QRectF();
}

/*!
\reimp
*/
HbVkbHost::HbVkbStatus HbVkbHostBridge::keypadStatusBeforeOrientationChange() const
{
    Q_D(const HbVkbHostBridge);
    if (d->mActiveHost) {
        return d->mActiveHost->keypadStatusBeforeOrientationChange();
    }

    return HbVkbHost::HbVkbStatusClosed;
}

/*!
\reimp
*/
void HbVkbHostBridge::ensureCursorVisibility()
{
    Q_D(HbVkbHostBridge);
    if (d->mActiveHost) {
        d->mActiveHost->ensureCursorVisibility();
    }
}

/*!
Connects vkb host to the bridge. Returns false if there is an existing host and
it has ongoing state transition.

Only vkb host implementors need this method.
*/
bool HbVkbHostBridge::connectHost(HbVkbHost *host)
{
    Q_D(HbVkbHostBridge);

    if (d->mActiveHost && d->mActiveHost->stateTransitionOngoing()) {     
        return false;
    }

    if (d->mActiveHost != host) {
        if (d->mActiveHost) {
            disconnect(d->mActiveHost, SIGNAL(aboutToOpen()), this, SIGNAL(aboutToOpen()));
            disconnect(d->mActiveHost, SIGNAL(aboutToClose()), this, SIGNAL(aboutToClose()));
            disconnect(d->mActiveHost, SIGNAL(keypadOpened()), this, SIGNAL(keypadOpened()));
            disconnect(d->mActiveHost, SIGNAL(keypadClosed()), this, SIGNAL(keypadClosed()));
            disconnect(d->mActiveHost, SIGNAL(keypadOpened()), this, SIGNAL(stateTransitionCompleted()));
            disconnect(d->mActiveHost, SIGNAL(keypadClosed()), this, SIGNAL(stateTransitionCompleted()));
        }  

        d->mActiveHost = host;
        if (d->mActiveHost) {
            connect(d->mActiveHost, SIGNAL(aboutToOpen()), this, SIGNAL(aboutToOpen()));
            connect(d->mActiveHost, SIGNAL(aboutToClose()), this, SIGNAL(aboutToClose()));
            connect(d->mActiveHost, SIGNAL(keypadOpened()), this, SIGNAL(keypadOpened()));
            connect(d->mActiveHost, SIGNAL(keypadClosed()), this, SIGNAL(keypadClosed()));
            connect(d->mActiveHost, SIGNAL(keypadOpened()), this, SIGNAL(stateTransitionCompleted()));
            connect(d->mActiveHost, SIGNAL(keypadClosed()), this, SIGNAL(stateTransitionCompleted()));
        }
    }

    return true;
}

/*!
Returns currently active vkb host. Note that there is active vkb host only when an editor is focused.
*/
HbVkbHost *HbVkbHostBridge::activeHost() const
{
    Q_D(const HbVkbHostBridge);
    return d->mActiveHost;
}

/*!
\reimp
*/
void HbVkbHostBridge::refresh()
{
}

/*!
\reimp
*/
bool HbVkbHostBridge::stateTransitionOngoing() const
{
    Q_D(const HbVkbHostBridge);
    if (d->mActiveHost) {
        return d->mActiveHost->stateTransitionOngoing();
    }

    return false;
}

// End of file

