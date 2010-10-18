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
@stable
@hbcore
\class HbVirtualKeyboard
\brief The HbVirtualKeyboard class is an abstract base class for virtual keyboards.

This class is an abstract base class for virtual keyboards. It is not responsible
for the visual appearance of the keyboard, but contains methods that the HbVkbHost
class needs when interacting with the keyboard. The concrete widget inheriting
from this class will fill and define the layout for the keyboard contents.

The virtual keyboard host calls aboutToOpen() when it is about to open the keyboard
and aboutToClose() when it is about to close it. Similarly, it calls keyboardOpened()
and keyboardClosed() when the open and close operations have been completed.

Every time the host runs keyboard related animations, it calls keyboardAnimationFrame()
after each frame so that that the keyboard widget has the possibility of running
local animation effects inside the widget if needed.

See \ref vkbHandling "virtual keyboard handling guide" for more information

\sa HbVkbHost
*/


/*!
\enum HbVirtualKeyboard::HbVkbAnimationType
Specifies known virtual keyboard animation types.
*/

/*!
\var HbVirtualKeyboard::HbVkbAnimOpen
Animation type for virtual keyboard opening.
*/

/*!
\var HbVirtualKeyboard::HbVkbAnimClose
Animation type for virtual keyboard closing.
*/

/*!
\fn void HbVirtualKeyboard::aboutToOpen(HbVkbHost *host)

HbVkbHost calls this function right before the keyboard becomes visible.
The virtual keyboard widget can then do any last minute initializations
if needed.

\sa HbVkbHost
*/

/*!
\fn void HbVirtualKeyboard::aboutToClose(HbVkbHost *host)

HbVkbHost calls this function right before keyboard is closed.
The virtual keyboard widget can then do any cleaning operations it needs to do.

\sa HbVkbHost
*/

/*!
\fn void HbVirtualKeyboard::keyboardOpened(HbVkbHost *host)

HbVkbHost calls this function when the keyboard opening operation is finished
and the keyboard is visible on the screen in its final position.

\sa HbVkbHost
*/

/*!
\fn void HbVirtualKeyboard::keyboardClosed(HbVkbHost *host)

HbVkbHost calls this function when the keyboard closing operation is finished
and the keyboard is no longer visible on the screen.
*/

/*!
\fn QSizeF HbVirtualKeyboard::preferredKeyboardSize()
Returns the size of the preferred keyboard.
*/
 
/*!
\fn void HbVirtualKeyboard::keyboardAnimationFrame(HbVkbAnimationType type, qreal x)

HbVkbHost calls this function when the keyboard animation frame is drawn.
The keyboard widget may then animate its contents if needed. Parameter \a type
specifies the animation type and value \a x is the animation phase (between 0.0 and 1.0).
*/

/*!
\fn HbKeyboardType HbVirtualKeyboard::keyboardType() const

Returns the keyboard type. See file \c hbinputdef.h for the possible HbKeyboardType values. 
The default implementation returns HbKeyboardNone.
*/

/*!
\fn QWidget * HbVirtualKeyboard::asWidget()

Returns a QWidget pointer to the virtual keyboard widget.
*/

/*!
\fn QGraphicsWidget * HbVirtualKeyboard::asGraphicsWidget()

Returns a QGraphicsWidget pointer to the virtual keyboard widget.
*/

// End of file
