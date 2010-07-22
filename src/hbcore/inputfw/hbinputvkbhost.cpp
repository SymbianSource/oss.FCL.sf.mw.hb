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
#include "hbinputvkbhost.h"

#include <QVariant>

#include "hbinputmethod.h"

const char HbVkbHostPropertyName[] = "HbVkbHost";

/*!
\proto
\class HbVkbHost
\brief Virtual keyboard host base class.

A virtual keyboard host knows how to animate and display given virtual keyboard widget.
Its purpose is to isolate virtual keyboard's interaction in application space from
the widget and the input method.

While the input method is responsible for creating virtual keyboard widget,
it is the virtual keyboard host who places it on the screen. The host decides
where the keyboard is shown and what is the maximum area it is allowed to cover.

Virtual keyboard host also knows how to keep the editor cursor visible at all times
so that the virtual keyboard widget never covers it. Application developer or editor widget
developer doesn't need to care about that.

There can be several vkb hosts in the system because in some situations the keyboard
needs to behave differently than in others.

The input framework finds active vkb host by traversing editor widget's parent chain
to see if there is a host attached to editor widget or any of its parents. If
it doesn't find a suitable host, it uses the default host.

Application developer can override the default vkb host if needed but in typical cases
there is no need to care about vkb hosting system in application code.

\sa HbVirtualKeyboard
\sa HbEditorInterface
*/

/*!
\fn virtual void HbVkbHost::openKeypad(HbVirtualKeyboard *vkb, HbInputMethod *owner, bool animationAllowed = true)
Opens given virtual keyboard. If animation is used, emits signal
keypadOpened after animation is completed.
*/

/*!
\fn virtual void HbVkbHost::closeKeypad(bool animationAllowed = true)
Closes keyboard. If animation is used, emits signal keypadClosed.
*/

/*!
\fn virtual bool HbVkbHost::isKeypadOpen() const
Returns true if virtual keypad is open on the screen.
*/

/*!
\fn virtual QSizeF HbVkbHost::keyboardArea() const
Returns the size of a rectangular area of the screen where virtual keyboard will be displayed. If
virtual keyboard's preferredKeyboardSize method returns larger area than returned by this method,
the vkb host will shrink keyboard to fit into this rectangle.
*/

/*!
\fn virtual HbVirtualKeyboard* activeKeypad() const = 0
Returns active virtual keyboard. Returns zero if there is no active keyboard.
*/

/*!
\fn virtual void ensureCursorVisibility() = 0
Checks whether the keypad is obscuring cursor in the active editor and moves the background if so.
*/

/*!
\fn virtual QRectF applicationArea() const = 0
Returns the area inside application window (the one where active editor resides in case there are
several application windows) that remains visible when virtual keyboard is active on screen.
Returns zero sized rectangle if virtual keyboard is not active.
*/

/*!
\fn virtual void refresh() = 0
The framework calls this method when it suspects that the host should
refresh the split view status.
*/

/*!
\fn bool stateTransitionOngoing() const = 0
Returns true if the vkb host has state transition ongoing, ie. it is running open or close
animation or updating the split-view status.
*/

/*!
Attaches given host to given object. Deletes previously attached host.
*/
void HbVkbHost::attachHost(HbVkbHost *host, QObject *object)
{
    if (object) {
        delete getVkbHost(object);  // delete previous host.
        QObject *hostObject = host;
        QVariant hostData = qVariantFromValue(hostObject);
        object->setProperty(HbVkbHostPropertyName, hostData);
    }
}

/*!
Removes (possible) vkb host from given object.
*/
void HbVkbHost::detachHost(QObject *object)
{
    if (object) {
        QObject *hostObject = 0;
        QVariant hostData = qVariantFromValue(hostObject);
        object->setProperty(HbVkbHostPropertyName, hostData);
    }
}

/*!
Returns vkb host attached to given object. Returns 0 if no hosts are attached
to object.
*/
HbVkbHost *HbVkbHost::getVkbHost(QObject *object)
{
    if (object) {
        QVariant variant = object->property(HbVkbHostPropertyName);
        if (variant.isValid()) {
            QObject *hostObject = variant.value<QObject *>();
            HbVkbHost *host = static_cast<HbVkbHost *>(hostObject);
            return host;
        }
    }

    return 0;
}

/*!
Returns active virtual keyboard host if there is one currently available.
*/
HbVkbHost *HbVkbHost::activeVkbHost()
{
    HbInputMethod *activeMethod = HbInputMethod::activeInputMethod();
    if (activeMethod && activeMethod->focusObject()) {
        return activeMethod->focusObject()->editorInterface().vkbHost();
    }

    return 0;
}

// End of file
