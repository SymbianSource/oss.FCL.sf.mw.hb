/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbPlugins module of the UI Extensions for Mobile.
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

#ifndef HB_INPUT_12KEY_TOUCH_KEYBOARD_H
#define HB_INPUT_12KEY_TOUCH_KEYBOARD_H

#include <hbinputdef.h>
#include "hbinputvkbwidget.h"

class QGraphicsItem;
class Hb12KeyTouchKeyboardPrivate;

class Hb12KeyTouchKeyboard : public HbInputVkbWidget
{
    Q_OBJECT

public:
    Hb12KeyTouchKeyboard(HbInputMethod *owner, const HbKeymap *keymap, QGraphicsItem *parent = 0);
    ~Hb12KeyTouchKeyboard();

public:  // From HbVirtualKeyboard
    HbKeyboardType keyboardType() const;

public: // From HbInputVkbWidget
    QSizeF preferredKeyboardSize();

protected:
    Hb12KeyTouchKeyboard(Hb12KeyTouchKeyboardPrivate &dd, HbInputMethod *owner, const HbKeymap *keymap, QGraphicsItem* parent);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, Hb12KeyTouchKeyboard)
    Q_DISABLE_COPY(Hb12KeyTouchKeyboard)
};

#endif // HB_INPUT_12KEY_TOUCH_KEYBOARD_H

// End of file
