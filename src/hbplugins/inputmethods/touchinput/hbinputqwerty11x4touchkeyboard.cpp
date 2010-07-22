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

#include <hbdeviceprofile.h>

#include <hbinputmethod.h>
#include <hbinputkeymap.h>
#include <hbinputsettingproxy.h>

#include "hbinputqwerty11x4touchkeyboard.h"
#include "hbinputqwerty11x4touchkeyboard_p.h"
#include "hbinputvkbwidget_p.h"
#include "hbinputbuttongroup.h"
#include "hbinputbutton.h"
#include "hbinputmodeindicator.h"

const qreal HbKeyboardHeightInUnits = 33.7;
const qreal HbKeyboardWidthInUnits = 95.5;

const int HbVirtualQwertyNumberOfRows = 4;
const int HbVirtualQwertyNumberOfColumns = 11;
const int HbButtonKeyCodeTable[HbVirtualQwertyNumberOfRows * HbVirtualQwertyNumberOfColumns - 1] =
{
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeDelete,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeEnter,
    HbInputButton::ButtonKeyCodeShift,
    HbInputButton::ButtonKeyCodeSymbol,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeSpace,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeSettings,
    HbInputButton::ButtonKeyCodeCustom
};

HbQwerty11x4KeyboardPrivate::HbQwerty11x4KeyboardPrivate()
{
}

HbQwerty11x4KeyboardPrivate::~HbQwerty11x4KeyboardPrivate()
{
}

void HbQwerty11x4KeyboardPrivate::init()
{
    Q_Q(HbQwerty11x4Keyboard);

    HbInputVkbWidgetPrivate::init();

    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup*>(q->contentItem());
    if (buttonGroup) {
        buttonGroup->setGridSize(QSize(HbVirtualQwertyNumberOfColumns, HbVirtualQwertyNumberOfRows));

        int key = 0;
        QList<HbInputButton*> buttons;
        for (int i = 0; i < HbVirtualQwertyNumberOfColumns * HbVirtualQwertyNumberOfRows - 1; ++i) {
            HbInputButton *item = new HbInputButton(HbButtonKeyCodeTable[i], QPoint(key % HbVirtualQwertyNumberOfColumns, key / HbVirtualQwertyNumberOfColumns));
            buttons.append(item);

            if (HbButtonKeyCodeTable[i] == HbInputButton::ButtonKeyCodeSettings) {
                mInputModeIndicator = new HbInputModeIndicator(item, q);
            } else if (HbButtonKeyCodeTable[i] == HbInputButton::ButtonKeyCodeSpace) {
                item->setSize(QSize(2, 1));
                ++key;
            }
            ++key;
        }
        buttonGroup->setButtons(buttons);
        buttonGroup->setButtonPreviewEnabled(HbInputSettingProxy::instance()->isCharacterPreviewForQwertyEnabled());
        buttonGroup->setCharacterSelectionPreviewEnabled(true);

        QObject::connect(buttonGroup, SIGNAL(buttonPressed(const QKeyEvent&)), q, SLOT(sendKeyPressEvent(const QKeyEvent&)));
        QObject::connect(buttonGroup, SIGNAL(buttonDoublePressed(const QKeyEvent&)), q, SLOT(sendKeyDoublePressEvent(const QKeyEvent&)));
        QObject::connect(buttonGroup, SIGNAL(buttonReleased(const QKeyEvent&)), q, SLOT(sendKeyReleaseEvent(const QKeyEvent&)));
        QObject::connect(buttonGroup, SIGNAL(buttonLongPressed(const QKeyEvent&)), q, SLOT(sendLongPressEvent(const QKeyEvent&)));
        QObject::connect(buttonGroup, SIGNAL(pressedButtonChanged(const QKeyEvent&, const QKeyEvent&)), q, SLOT(sendKeyChangeEvent(const QKeyEvent&, const QKeyEvent&)));
    }

    QObject::connect(q, SIGNAL(flickEvent(HbInputVkbWidget::HbFlickDirection)), buttonGroup, SLOT(cancelButtonPress()));
}

int HbQwerty11x4KeyboardPrivate::keyCode(int buttonId)
{
    return HbButtonKeyCodeTable[buttonId];
}

/*!
Constructs the object. owner is the owning input method implementation. Keymap
is key mapping data to be used to display button texts. Key mapping data can be
changed later (for example when the input language changes) by calling
setKeymap.
*/
HbQwerty11x4Keyboard::HbQwerty11x4Keyboard(HbInputMethod *owner, const HbKeymap *keymap, QGraphicsItem *parent)
 : HbQwerty10x4Keyboard(*new HbQwerty11x4KeyboardPrivate, owner, keymap, parent)
{
    Q_D(HbQwerty11x4Keyboard);
    d->mOwner = owner;
    setKeymap(keymap);

    QObject::connect(HbInputSettingProxy::instance(), SIGNAL(characterPreviewStateForQwertyChanged(bool)), this, SLOT(updateButtonPreviewStatus(bool)));
}

/*!
Constructs the object. owner is the owning input method implementation. Keymap
is key mapping data to be used to display button texts. Key mapping data can be
changed later (for example when the input language changes) by calling
setKeymap.
*/
HbQwerty11x4Keyboard::HbQwerty11x4Keyboard(HbQwerty11x4KeyboardPrivate &dd, HbInputMethod *owner,
                                           const HbKeymap *keymap, QGraphicsItem *parent)
 : HbQwerty10x4Keyboard(dd, owner, keymap, parent)
{
    Q_D(HbQwerty11x4Keyboard);
    d->mOwner = owner;
    setKeymap(keymap);

    QObject::connect(HbInputSettingProxy::instance(), SIGNAL(characterPreviewStateForQwertyChanged(bool)), this, SLOT(updateButtonPreviewStatus(bool)));
}

/*!
Destructs the object.
*/
HbQwerty11x4Keyboard::~HbQwerty11x4Keyboard()
{
}

// End of file
