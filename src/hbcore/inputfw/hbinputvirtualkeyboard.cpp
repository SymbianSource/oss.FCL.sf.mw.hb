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

#include "hbinputvirtualkeyboard.h"

/*!
\proto
\class HbVirtualKeyboard
\brief Abstract base class for virtual keyboards.

This class is an abstract base class for virtual keyaboards. It is
not reponsible for the visual appearance of the keyboard, but contains methods that HbVkbHost
class needs when interacting with the keyboard. The actual Qt widget inheriting from this
class will fill and layout the keyboard contents.

The virtual keyboard host calls aboutToOpen() method when it is about to open the keyboard
and aboutToClose() when it is about to close it. Similarily, it calls keyboardOpened()
and keyboardClosed() methods when open and close operations have been completed.

Every time the host runs keyboard related animations, it calls keyboardAnimationFrame()
method after each frame so that that the keyboard widget has possibility to run local animation
effects inside the widget if needed.

\sa HbVkbHost
*/

/*!
\fn void HbVirtualKeyboard::aboutToOpen(HbVkbHost *host)

HbVkbHost calls this method right before keyboard becomes visible.
Virtual keyboard widget can then do any last minute initializations
if needed. The default implementation is empty.

\sa HbVkbHost
*/

/*!
\fn void HbVirtualKeyboard::aboutToClose(HbVkbHost *host)

HbVkbHost calls this method right before keyboard becomes visible.
Virtual keyboard widget can then do any cleaning operations it needs to do.
The default implemntation is empty.

\sa HbVkbHost
*/

/*!
\fn void HbVirtualKeyboard::keyboardOpened(HbVkbHost *host)

HbVkbHost calls this method when the keyboard open operation is finished and the keyboard is
visible on the screen in its final position. The default implementation is empty.

\sa HbVkbHost
*/

/*!
\fn void HbVirtualKeyboard::keyboardClosed(HbVkbHost *host)

HbVkbHost calls this method when keyboard closing operation is finished and the keyboard is not
visible on the screen anymore. The deafult implementation is empty.
*/

/*!
\fn void HbVirtualKeyboard::keyboardMinimized(HbVkbHost *host)

HbVkbHost calls this method when keyboard minimizing operation is finished and only the close bar
is visible on the keypad. The deafult implementation is empty.
*/

/*!
\fn QSizeF HbVirtualKeyboard::minimizedKeyboardSize()
The vkb host queries the size of minimized keyboard through this method.
*/

/*!
\fn void HbVirtualKeyboard::keyboardAnimationFrame(HbVkbAnimationType type, qreal x)

Vkb host calls this method when keyboard animation frame is drawn. Keyboard widget may then
animate its contents if needed. Parameter type specifies animation type and value
x is animation phase (between 0.0 and 1.0). The default implementation is empty.
*/

// End of file
