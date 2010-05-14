/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbInput module of the UI Extensions for Mobile.
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

#include <math.h>

#include <hbdeviceprofile.h>

#include <hbinputmethod.h>
#include <hbinputkeymap.h>
#include <hbinpututils.h>
#include <hbframedrawer.h>
#include <hbinputsettingproxy.h>

#include "hbinputsctkeyboard.h"
#include "hbinputsctkeyboard_p.h"
#include "hbinputbuttongroup.h"
#include "hbinputbutton.h"

const qreal HbPortraitKeyboardHeightInUnits = 46.8;
const qreal HbPortraitKeyboardWidthInUnits = 53.8;
const qreal HbLandscapeKeyboardHeightInUnits = 34.6;
const qreal HbLandscapeKeyboardWidthInUnits = 95.5;

const int HbSctPortraitNumberOfRows = 5;
const int HbSctPortraitNumberOfColumns = 5;
const int HbPortraitButtonKeyCodeTable[HbSctPortraitNumberOfRows * HbSctPortraitNumberOfColumns] =
{
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeDelete,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeAlphabet,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodePageChange,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeSmiley,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCustom
};

const int HbSctLandscapeNumberOfRows = 4;
const int HbSctLandscapeNumberOfColumns = 10;
const int HbLandscapeButtonKeyCodeTable[HbSctLandscapeNumberOfRows * HbSctLandscapeNumberOfColumns] =
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
    HbInputButton::ButtonKeyCodeSmiley,
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
    HbInputButton::ButtonKeyCodeEnter,
    HbInputButton::ButtonKeyCodePageChange,
    HbInputButton::ButtonKeyCodeAlphabet,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeSpace,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCustom
};

/*!
@proto
@hbinput
\class HbSctKeyboard
\brief Touch keyboard for special characters

Implements special character keyboard. The keyboard knows how to set up button titles according to
given key map data object and it also supports editor specific custom buttons.

\sa HbInputVkbWidget
*/

HbSctKeyboardPrivate::HbSctKeyboardPrivate()
 : mType(HbKeyboardSctPortrait), mColumns(0), mRows(0),
   mCharacterButtons(0), mPages(0), mActivePage(0)
{
}

HbSctKeyboardPrivate::~HbSctKeyboardPrivate()
{
}

void HbSctKeyboardPrivate::init()
{
    Q_Q(HbSctKeyboard);

    HbInputVkbWidgetPrivate::init();

    if (HbInputSettingProxy::instance()->activeKeyboard() & HbQwertyKeyboardMask) {
        mType = HbKeyboardSctLandscape;
    }

    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup*>(q->contentItem());
    if (buttonGroup) {
        mColumns = HbSctPortraitNumberOfColumns;
        mRows = HbSctPortraitNumberOfRows;
        if (mType == HbKeyboardSctLandscape) {
            mColumns = HbSctLandscapeNumberOfColumns;
            mRows = HbSctLandscapeNumberOfRows;

            buttonGroup->setButtonPreviewEnabled(HbInputSettingProxy::instance()->isCharacterPreviewForQwertyEnabled());
        }

        buttonGroup->setGridSize(QSize(mColumns, mRows));

        int key = 0;
        QList<HbInputButton*> buttons;
        for (int i = 0; i < mColumns * mRows; ++i) {
            HbInputButton *item = new HbInputButton(keyCode(i), QPoint(key % mColumns, key / mColumns));
            buttons.append(item);
 
            if (keyCode(i) == HbInputButton::ButtonKeyCodeCharacter) {
                ++mCharacterButtons;
            } else if (keyCode(i) == HbInputButton::ButtonKeyCodeSpace) {
                item->setSize(QSize(2, 1));
                ++key;
            } else if (keyCode(i) == HbInputButton::ButtonKeyCodeDelete &&
                       mType == HbKeyboardSctPortrait) {
                item->setIcon(HbIcon(HbInputButtonIconDelete2), HbInputButton::ButtonIconIndexPrimary);
            } else if (keyCode(i) == HbInputButton::ButtonKeyCodePageChange &&
                       mType == HbKeyboardSctPortrait) {
                item->setIcon(HbIcon(HbInputButtonIconPageChange2), HbInputButton::ButtonIconIndexPrimary);
            }
            ++key;
        }
        buttonGroup->setButtons(buttons);

        QObject::connect(buttonGroup, SIGNAL(buttonPressed(const QKeyEvent&)), q, SLOT(sendKeyPressEvent(const QKeyEvent&)));
        QObject::connect(buttonGroup, SIGNAL(buttonDoublePressed(const QKeyEvent&)), q, SLOT(sendKeyDoublePressEvent(const QKeyEvent&)));
        QObject::connect(buttonGroup, SIGNAL(buttonReleased(const QKeyEvent&)), q, SLOT(sendKeyReleaseEvent(const QKeyEvent&)));
        QObject::connect(buttonGroup, SIGNAL(buttonLongPressed(const QKeyEvent&)), q, SLOT(sendLongPressEvent(const QKeyEvent&)));
        QObject::connect(buttonGroup, SIGNAL(pressedButtonChanged(const QKeyEvent&, const QKeyEvent&)), q, SLOT(sendKeyChangeEvent(const QKeyEvent&, const QKeyEvent&)));
    }
 
    QObject::connect(q, SIGNAL(flickEvent(HbInputVkbWidget::HbFlickDirection)), buttonGroup, SLOT(cancelButtonPress()));
}

int HbSctKeyboardPrivate::keyCode(int buttonId)
{
    if (mType == HbKeyboardSctPortrait) {
        return HbPortraitButtonKeyCodeTable[buttonId];
    } else {
        return HbLandscapeButtonKeyCodeTable[buttonId];
    }
}

void HbSctKeyboardPrivate::applyEditorConstraints()
{
    Q_Q(HbSctKeyboard);

    HbInputFocusObject *focusedObject = mOwner->focusObject();
    if (!focusedObject) {
        return;
    }

    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup*>(q->contentItem());
    if (buttonGroup) {
        QList<HbInputButton*> buttons = buttonGroup->buttons();
        for (int i = 0; i < buttons.count(); ++i) {
            HbInputButton *item = buttons.at(i);

            HbInputButton::HbInputButtonState state = item->state();
            if (keyCode(i) == HbInputButton::ButtonKeyCodeCharacter) {
                QString data = item->text(HbInputButton::ButtonTextIndexPrimary);
                if (data.isEmpty() || !focusedObject->characterAllowedInEditor(data.at(0))) {
                    state = HbInputButton::ButtonStateDisabled;
                } else if (item->state() == HbInputButton::ButtonStateDisabled) {
                    state = HbInputButton::ButtonStateReleased;
                }
            } else if (keyCode(i) == HbInputButton::ButtonKeyCodeSmiley) {
                if (focusedObject->editorInterface().isNumericEditor() ||
                    !focusedObject->editorInterface().editorClass() == HbInputEditorClassUnknown ||
                    !isSmileysEnabled()) {
                    state = HbInputButton::ButtonStateDisabled;
                } else if (item->state() == HbInputButton::ButtonStateDisabled) {
                    state = HbInputButton::ButtonStateReleased;
                }
            }
            item->setState(state);
        }
        buttonGroup->setButtons(buttons);
    }
}

void HbSctKeyboardPrivate::updateKeyCodes()
{
    Q_Q(HbSctKeyboard);

    mPages = 0;
    const HbKeyboardMap *keyboardMap = mKeymap->keyboard(q->keyboardType());
    if (keyboardMap) {
        mPages = (int)ceil((float)(keyboardMap->keys.count() / mCharacterButtons));
    }

    if (mPages > 1) {
        mFlickAnimation = true;
    }

    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup*>(q->contentItem());
    if (buttonGroup) {
        int key = mActivePage * mCharacterButtons;
        QList<HbInputButton*> buttons = buttonGroup->buttons();
        for (int i = 0; i < buttons.count(); ++i) {
            if (keyCode(i) == HbInputButton::ButtonKeyCodeCharacter) {
                HbInputButton *item = buttons.at(i);

                if (keyboardMap && key < keyboardMap->keys.count()) {
                    item->setKeyCode(keyboardMap->keys.at(key)->keycode.unicode());
                } else {
                    item->setKeyCode(-1);
                }
                ++key;
            }
        }
    }
}

void HbSctKeyboardPrivate::updateButtons()
{
    Q_Q(HbSctKeyboard);

    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup*>(q->contentItem());
    if (buttonGroup) {
        int key = mActivePage * mCharacterButtons;
        QList<HbInputButton*> buttons = buttonGroup->buttons();
        for (int i = 0; i < buttons.count(); ++i) {
            if (keyCode(i) == HbInputButton::ButtonKeyCodeCharacter) {
                HbInputButton *item = buttons.at(i);

                const HbKeyboardMap *keyboardMap = mKeymap->keyboard(q->keyboardType());
                if (keyboardMap && key < keyboardMap->keys.count()) {
                    QString keydata = keyboardMap->keys.at(key)->characters(HbModifierNone);
                    item->setText(keydata.at(0), HbInputButton::ButtonTextIndexPrimary);
                } else {
                    item->setText("", HbInputButton::ButtonTextIndexPrimary);
                }

                ++key;
            }
        }
        buttonGroup->setButtons(buttons);
    }
}

/*!
Constructs the object. owner is the owning input method implementation. Keymap
is key mapping data to be used to display button texts. Key mapping data can be
changed later (for example when the input language changes) by calling
setKeymap.
*/
HbSctKeyboard::HbSctKeyboard(HbInputMethod *owner, const HbKeymap *keymap, QGraphicsItem *parent)
 : HbInputVkbWidget(*new HbSctKeyboardPrivate, parent)
{
    if (!owner) {
        return;
    }
    Q_D(HbSctKeyboard);
    d->mOwner = owner;    
    setKeymap(keymap);

    const HbKeyboardMap *keyboardMap = keymap->keyboard(keyboardType());
    if (keyboardMap) {
        d->mPages = (int)ceil((float)(keyboardMap->keys.count() / d->mCharacterButtons));
    }

    if (d->mPages > 1) {
        d->mFlickAnimation = true;
    }

    if (d->mType == HbKeyboardSctLandscape) {
        connect(HbInputSettingProxy::instance(), SIGNAL(characterPreviewStateForQwertyChanged(bool)), this, SLOT(updateButtonPreviewStatus(bool)));
    }
    connect(this, SIGNAL(flickEvent(HbInputVkbWidget::HbFlickDirection)), this, SLOT(changePage(HbInputVkbWidget::HbFlickDirection)));
}

/*!
Constructs the object. owner is the owning input method implementation. Keymap
is key mapping data to be used to display button texts. Key mapping data can be
changed later (for example when the input language changes) by calling
setKeymap.
*/
HbSctKeyboard::HbSctKeyboard(HbSctKeyboardPrivate &dd, HbInputMethod *owner,
                             const HbKeymap *keymap, QGraphicsItem* parent)
 : HbInputVkbWidget(dd, parent)
{
    if (!owner) {
        return;
    }
    Q_D(HbSctKeyboard);
    d->mOwner = owner;    
    setKeymap(keymap);

    const HbKeyboardMap *keyboardMap = keymap->keyboard(keyboardType());
    if (keyboardMap) {
        d->mPages = (int)ceil((float)(keyboardMap->keys.count() / d->mCharacterButtons));
    }

    if (d->mPages > 1) {
        d->mFlickAnimation = true;
    }

    if (d->mType == HbKeyboardSctLandscape) {
        connect(HbInputSettingProxy::instance(), SIGNAL(characterPreviewStateForQwertyChanged(bool)), this, SLOT(updateButtonPreviewStatus(bool)));
    }
    connect(this, SIGNAL(flickEvent(HbInputVkbWidget::HbFlickDirection)), this, SLOT(changePage(HbInputVkbWidget::HbFlickDirection)));
}

/*!
Destructs the object.
*/
HbSctKeyboard::~HbSctKeyboard()
{
}

/*!
Returns keyboard type.
*/
HbKeyboardType HbSctKeyboard::keyboardType() const
{
    Q_D(const HbSctKeyboard);

    return d->mType;
}

/*!
Returns preferred keyboard size. HbVkbHost uses this information when it opens the keyboard.
*/
QSizeF HbSctKeyboard::preferredKeyboardSize()
{
    Q_D(HbSctKeyboard);

    QSizeF result;
    qreal unitValue = HbDeviceProfile::profile(mainWindow()).unitValue();

    if (d->mType == HbKeyboardSctLandscape) {
        result.setHeight(HbLandscapeKeyboardHeightInUnits * unitValue + d->mCloseHandleHeight);
        result.setWidth(HbLandscapeKeyboardWidthInUnits * unitValue);
    } else {
        result.setHeight(HbPortraitKeyboardHeightInUnits * unitValue + d->mCloseHandleHeight);
        result.setWidth(HbPortraitKeyboardWidthInUnits * unitValue);
    }

    return QSizeF(result);
}

/*!
Sets the keypad to given mode. Possible values are EModeAbc, EModeNumeric and EModeSct.
*/
void HbSctKeyboard::setMode(HbKeypadMode mode, HbModifiers modifiers)
{
    Q_D(HbSctKeyboard);

    d->mActivePage = 0;
    d->updateKeyCodes();

    HbInputVkbWidget::setMode(mode, modifiers);
}

/*!
Updates button preview status.
*/
void HbSctKeyboard::updateButtonPreviewStatus(bool status)
{
    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup*>(contentItem());
    if (buttonGroup) {
        buttonGroup->setButtonPreviewEnabled(status);   
    }
}

/*!
Handles flick gesture
*/
void HbSctKeyboard::changePage(HbInputVkbWidget::HbFlickDirection flickDirection)
{
    Q_D(HbSctKeyboard);

    if (flickDirection == HbInputVkbWidget::HbFlickDirectionRight ||
        flickDirection == HbInputVkbWidget::HbFlickDirectionLeft) {
        int direction = -1;
        if (flickDirection == HbInputVkbWidget::HbFlickDirectionRight) {
            direction = 1;
        }

        d->mActivePage = (d->mActivePage + direction) % d->mPages;
        if (d->mActivePage < 0) {
            d->mActivePage = d->mPages - 1;
        }
        d->updateKeyCodes();
        d->updateButtons();
        d->applyEditorConstraints();
    }
}

/*!
Sends key event to owning input method.
*/
void HbSctKeyboard::sendKeyPressEvent(const QKeyEvent &event)
{
    if (event.key() != HbInputButton::ButtonKeyCodePageChange &&
        event.key() != HbInputButton::ButtonKeyCodeSmiley) {
        HbInputVkbWidget::sendKeyPressEvent(event);
    }
}

/*!
Sends key event to owning input method.
*/
void HbSctKeyboard::sendKeyReleaseEvent(const QKeyEvent &event)
{
    Q_D(HbSctKeyboard);

    if (event.key() == HbInputButton::ButtonKeyCodePageChange) {
        changePage(HbInputVkbWidget::HbFlickDirectionRight);
    } else if (event.key() == HbInputButton::ButtonKeyCodeSmiley) {
        showSmileyPicker(d->mRows, d->mColumns);
    } else {
        HbInputVkbWidget::sendKeyReleaseEvent(event);
    }
}

/*!
Sends key event to owning input method.
Release event is ignored.
*/
void HbSctKeyboard::sendKeyChangeEvent(const QKeyEvent &releaseEvent, const QKeyEvent &pressEvent)
{
    if (pressEvent.key() != HbInputButton::ButtonKeyCodePageChange && 
        pressEvent.key() != HbInputButton::ButtonKeyCodeSmiley) {
        HbInputVkbWidget::sendKeyChangeEvent(releaseEvent, pressEvent);
    }
}

// End of file
