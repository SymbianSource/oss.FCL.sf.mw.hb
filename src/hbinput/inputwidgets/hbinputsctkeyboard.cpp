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
#include "hbinputsctkeyboard.h"
#include "hbinputsctkeyboard_p.h"

#include <math.h>

#include <hbdeviceprofile.h>

#include <hbinputmethod.h>
#include <hbinputkeymap.h>
#include <hbinpututils.h>
#include <hbframedrawer.h>
#include <hbinputsettingproxy.h>
#include <hbmainwindow.h>
#include <hbwidgetfeedback.h>

#include "hbinputbuttongroup.h"
#include "hbinputbutton.h"

const qreal HbPortraitKeyboardHeightInUnits = 45.9;
const qreal HbPortraitKeyboardWidthInUnits = 53.8;
const qreal HbLandscapeKeyboardHeightInUnits = 27.0;
const qreal HbLandscapeKeyboardWidthInUnits = 95.5;

const int HbSctPortraitNumberOfRows = 5;
const int HbSctPortraitNumberOfColumns = 5;
const int HbPortraitButtonCount = HbSctPortraitNumberOfRows * HbSctPortraitNumberOfColumns;
const int HbPortraitButtonKeyCodeTable[HbPortraitButtonCount] = {
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
const int HbLandscapeButtonCount = HbSctLandscapeNumberOfRows * HbSctLandscapeNumberOfColumns - 1;
const int HbLandscapeButtonKeyCodeTable[HbLandscapeButtonCount] = {
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

const int HbSctSpaceMarker = 0xE118;
const int HbSctEnterMarker = 0xE125;

#if defined(Q_OS_SYMBIAN)
const QChar HbSctSpaceGlyph = 0xF800;
const QChar HbSctEnterGlyph = 0xF801;
#else
const QChar HbSctSpaceGlyph = ' ';
const QChar HbSctEnterGlyph = ' ';
#endif

/*!
@stable
@hbinput
\class HbSctKeyboard
\brief The HbSctKeyboard class provides a touch keyboard for special characters.

The class implements a special character keyboard. The keyboard supports setting up
button titles according to a given keymap data object, and it also supports
editor-specific custom buttons.

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

    Qt::Orientation orientation = Qt::Horizontal;
    if (q->mainWindow()) {
        orientation = q->mainWindow()->orientation();
    }

    if (HbInputSettingProxy::instance()->activeKeyboard(orientation) & HbQwertyKeyboardMask) {
        mType = HbKeyboardSctLandscape;
    } else {
        mType = HbKeyboardSctPortrait;
    }

    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup *>(q->contentItem());
    if (buttonGroup) {
        mColumns = HbSctPortraitNumberOfColumns;
        mRows = HbSctPortraitNumberOfRows;
        int count = HbPortraitButtonCount;
        if (mType == HbKeyboardSctLandscape) {
            mColumns = HbSctLandscapeNumberOfColumns;
            mRows = HbSctLandscapeNumberOfRows;
            count = HbLandscapeButtonCount;

            buttonGroup->setButtonPreviewEnabled(HbInputSettingProxy::instance()->isCharacterPreviewForQwertyEnabled());
        }

        buttonGroup->setGridSize(QSize(mColumns, mRows));

        int key = 0;
        QList<HbInputButton *> buttons;
        for (int i = 0; i < count; ++i) {
            HbInputButton *item = new HbInputButton(keyCode(i), QPoint(key % mColumns, key / mColumns));
            buttons.append(item);

            if (keyCode(i) == HbInputButton::ButtonKeyCodeCharacter) {
                ++mCharacterButtons;
            } else if (keyCode(i) == HbInputButton::ButtonKeyCodeSpace) {
                item->setSize(QSize(2, 1));
                ++key;
            } else if (keyCode(i) == HbInputButton::ButtonKeyCodeDelete &&
                       (mType == HbKeyboardSctPortrait || mType == HbKeyboardSctEmail || mType == HbKeyboardSctUrl )) {
                // Portrait SCT has different delete icon from the default one
                item->setIcon(HbIcon(HbInputButtonIconDelete2), HbInputButton::ButtonIconIndexPrimary);
            } else if (keyCode(i) == HbInputButton::ButtonKeyCodeAlphabet) {
                // Button that is used to return to normal keypad should be shown as latched
                item->setState(HbInputButton::ButtonStateLatched);
                if (mType == HbKeyboardSctPortrait || mType == HbKeyboardSctEmail || mType == HbKeyboardSctUrl ) {
                    // Portrait SCT has different symbol icon from the default one
                    item->setIcon(HbIcon(HbInputButtonIconSymbol2), HbInputButton::ButtonIconIndexPrimary);
                }
            }
            ++key;
        }
        buttonGroup->setButtons(buttons);

        QObject::connect(buttonGroup, SIGNAL(buttonPressed(const QKeyEvent &)), q, SLOT(sendKeyPressEvent(const QKeyEvent &)));
        QObject::connect(buttonGroup, SIGNAL(buttonDoublePressed(const QKeyEvent &)), q, SLOT(sendKeyDoublePressEvent(const QKeyEvent &)));
        QObject::connect(buttonGroup, SIGNAL(buttonReleased(const QKeyEvent &)), q, SLOT(sendKeyReleaseEvent(const QKeyEvent &)));
        QObject::connect(buttonGroup, SIGNAL(buttonLongPressed(const QKeyEvent &)), q, SLOT(sendLongPressEvent(const QKeyEvent &)));
        QObject::connect(buttonGroup, SIGNAL(pressedButtonChanged(const QKeyEvent &, const QKeyEvent &)), q, SLOT(sendKeyChangeEvent(const QKeyEvent &, const QKeyEvent &)));
    }

    QObject::connect(q, SIGNAL(flickEvent(HbInputVkbWidget::HbFlickDirection)), buttonGroup, SLOT(cancelButtonPress()));
}

int HbSctKeyboardPrivate::keyCode(int buttonId)
{
    if (mType == HbKeyboardSctLandscape) {
        return HbLandscapeButtonKeyCodeTable[buttonId];
    } else {
        return HbPortraitButtonKeyCodeTable[buttonId];
    }
}


void HbSctKeyboardPrivate::applyEditorConstraints()
{
    Q_Q(HbSctKeyboard);
    HbInputFocusObject *focusedObject = mOwner->focusObject();
    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup *>(q->contentItem());
    applyEditorConstraints(focusedObject, buttonGroup);
}

void HbSctKeyboardPrivate::applyEditorConstraints(HbInputFocusObject *focusedObject, HbInputButtonGroup *buttonGroup)
{
    if (!focusedObject) {
        return;
    }
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
                    !isSmileysEnabled()) {
                    state = HbInputButton::ButtonStateDisabled;
                } else if (item->state() == HbInputButton::ButtonStateDisabled) {
                    state = HbInputButton::ButtonStateReleased;
                }
            } else if (keyCode(i) == HbInputButton::ButtonKeyCodeAlphabet) {
                state = HbInputButton::ButtonStateLatched;
            } else if (keyCode(i) == HbInputButton::ButtonKeyCodeSpace) {
                bool allowed = focusedObject->characterAllowedInEditor(QChar(' '));
                if (!allowed) {
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

    HbInputFocusObject *focusedObject = mOwner->focusObject();
    if (!focusedObject) {
        return;
    }

    Qt::Orientation orientation = Qt::Horizontal;
    if (q->mainWindow()) {
        orientation = q->mainWindow()->orientation();
    }

    if (HbInputSettingProxy::instance()->activeKeyboard(orientation) & HbQwertyKeyboardMask){
        mType = HbKeyboardSctLandscape;
    } else if (focusedObject && focusedObject->editorInterface().editorClass() == HbInputEditorClassEmail &&
               mKeymap->keyboard(HbKeyboardSctEmail)) {
        mType = HbKeyboardSctEmail;
    } else if (focusedObject && focusedObject->editorInterface().editorClass() == HbInputEditorClassUrl &&
               mKeymap->keyboard(HbKeyboardSctUrl)) {
        mType = HbKeyboardSctUrl;
    } else {
        mType = HbKeyboardSctPortrait;
    }
    
    mPages = 0;
    const HbKeyboardMap *keyboardMap = mKeymap->keyboard(q->keyboardType());
    if (keyboardMap) {
        mPages = (int)ceil((float)countActiveKeys() / mCharacterButtons);
    }
    
    mFlickAnimation = mPages > 1;
    
    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup *>(q->contentItem());
    if (buttonGroup) {
        int key = keyAtIndex(mActivePage * mCharacterButtons);
        QList<HbInputButton *> buttons = buttonGroup->buttons();
        for (int i = 0; i < buttons.count(); ++i) {
            if (keyCode(i) == HbInputButton::ButtonKeyCodeCharacter) {
                HbInputButton *item = buttons.at(i);

                if (keyboardMap && key < keyboardMap->keys.count()) {
                    // Replace space and enter markers with correct keycodes
                    if (focusedObject->characterAllowedInEditor(keyboardMap->keys.at(key)->characters(HbModifierNone).at(0)) || mType == HbKeyboardSctLandscape){
                        if (keyboardMap->keys.at(key)->keycode.unicode() == HbSctSpaceMarker) {
                            item->setKeyCode(HbInputButton::ButtonKeyCodeSpace);
                        } else if (keyboardMap->keys.at(key)->keycode.unicode() == HbSctEnterMarker) {
                            item->setKeyCode(HbInputButton::ButtonKeyCodeEnter);
                        } else {
                            item->setKeyCode(keyboardMap->keys.at(key)->keycode.unicode());
                        }
                    } else {
                        i--;
                    }
                } else {
                    item->setKeyCode(-1);
                }
                ++key;
            }
        }
    }
    applyEditorConstraints();           
}

void HbSctKeyboardPrivate::updateButtons()
{
    Q_Q(HbSctKeyboard);

    HbInputFocusObject *focusedObject = mOwner->focusObject();
    if (!focusedObject) {
        return;
    }

    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup *>(q->contentItem());
    if (buttonGroup) {
        int key = keyAtIndex(mActivePage * mCharacterButtons);
        QList<HbInputButton *> buttons = buttonGroup->buttons();
        for (int i = 0; i < buttons.count(); ++i) {
            if (keyCode(i) == HbInputButton::ButtonKeyCodeCharacter) {
                HbInputButton *item = buttons.at(i);
                const HbKeyboardMap *keyboardMap = mKeymap->keyboard(q->keyboardType());
                if (keyboardMap && key < keyboardMap->keys.count() && keyboardMap->keys.at(key)->characters(HbModifierNone) != QString("")) {
                    if (focusedObject->characterAllowedInEditor(keyboardMap->keys.at(key)->characters(HbModifierNone).at(0)) || mType == HbKeyboardSctLandscape){
                        QString keydata = keyboardMap->keys.at(key)->characters(HbModifierNone);
                        // Replace space and enter markers with correct glyphs.
                        // These only exist in symbian fonts, so if we are not using symbian, use blank.
                        if (keydata.at(0) == HbSctSpaceMarker) {
                            item->setText(HbSctSpaceGlyph, HbInputButton::ButtonTextIndexPrimary);
                        } else if (keydata.at(0) == HbSctEnterMarker) {
                            item->setText(HbSctEnterGlyph, HbInputButton::ButtonTextIndexPrimary);
                        } else {
                            item->setText(keydata.at(0), HbInputButton::ButtonTextIndexPrimary);
                        }
                    } else {
                        i--;
                    }
                } else {
                    item->setText("", HbInputButton::ButtonTextIndexPrimary);
                }

                ++key;
            } else if (keyCode(i) == HbInputButton::ButtonKeyCodePageChange) {
                HbInputButton *item = buttons.at(i);
                item->setText(QString::number(mActivePage+1) + '/' + QString::number(mPages), HbInputButton::ButtonTextIndexSecondaryFirstRow);
            }
        }
        buttonGroup->setButtons(buttons);
    }
}

int HbSctKeyboardPrivate::keyAtIndex(int index)
{
    Q_Q(HbSctKeyboard);
    
    const HbKeyboardMap *keyboardMap = mKeymap->keyboard(q->keyboardType());
    if (mType == HbKeyboardSctLandscape){
        return index;
    }
    int totalKeys=0;
    HbInputFocusObject *focusedObject = mOwner->focusObject();
    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup*>(q->contentItem());    
    if (buttonGroup) {
        if (keyboardMap && totalKeys < keyboardMap->keys.count()) {
            while (index){
                if (focusedObject->characterAllowedInEditor(keyboardMap->keys.at(totalKeys)->characters(HbModifierNone).at(0))){
                    index--;                    
                }
                totalKeys++;
            }            
        }
    }
    return totalKeys;
}

int HbSctKeyboardPrivate::countActiveKeys()
{
    Q_Q(HbSctKeyboard);

    const HbKeyboardMap *keyboardMap = mKeymap->keyboard(q->keyboardType());
    if (mType == HbKeyboardSctLandscape){
        return keyboardMap->keys.count();
    }

    int totalKeys=0;
    HbInputFocusObject *focusedObject = mOwner->focusObject();
    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup*>(q->contentItem());
    if (buttonGroup) {
        if (keyboardMap && totalKeys < keyboardMap->keys.count()) {
            foreach(HbMappedKey *key, keyboardMap->keys){  
                if (focusedObject->characterAllowedInEditor(key->characters(HbModifierNone).at(0))){
                    totalKeys++;
                }
            }    
        }
    }
    return totalKeys;
}

void HbSctKeyboardPrivate::changePage(HbInputVkbWidget::HbFlickDirection flickDirection)
{
    if (mFlickAnimation) {
        int direction = 1;
        if (flickDirection == HbInputVkbWidget::HbFlickDirectionRight) {
            direction = -1;
        }

        mActivePage = (mActivePage + direction) % mPages;
        if (mActivePage < 0) {
            mActivePage = mPages - 1;
        }
        updateButtons();
        updateKeyCodes();
    }
}

/*!
Constructor. \a owner is the owning input method implementation, and
\a keymap is the key mapping data that is used to display the button texts. Key mapping
data can be changed later (for example when the input language changes) by calling
the base class function HbInputVkbWidget::setKeymap.
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
        d->mPages = (int)ceil((float)d->countActiveKeys() / d->mCharacterButtons);
    }

    d->mFlickAnimation = d->mPages > 1;

    if (d->mType == HbKeyboardSctLandscape) {
        connect(HbInputSettingProxy::instance(), SIGNAL(characterPreviewStateForQwertyChanged(bool)), this, SLOT(updateButtonPreviewStatus(bool)));
    }
    connect(this, SIGNAL(flickEvent(HbInputVkbWidget::HbFlickDirection)), this, SLOT(changePage(HbInputVkbWidget::HbFlickDirection)));
}

/*!
Constructor. \a owner is the owning input method implementation, and
\a keymap is the key mapping data that is used to display the button texts. Key mapping
data can be changed later (for example when the input language changes) by calling
the base class function HbInputVkbWidget::setKeymap.
*/
HbSctKeyboard::HbSctKeyboard(HbSctKeyboardPrivate &dd, HbInputMethod *owner,
                             const HbKeymap *keymap, QGraphicsItem *parent)
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
        d->mPages = (int)ceil((float)d->countActiveKeys() / d->mCharacterButtons);
    }

    d->mFlickAnimation = d->mPages > 1;
    

    if (d->mType == HbKeyboardSctLandscape) {
        connect(HbInputSettingProxy::instance(), SIGNAL(characterPreviewStateForQwertyChanged(bool)), this, SLOT(updateButtonPreviewStatus(bool)));
    }
    connect(this, SIGNAL(flickEvent(HbInputVkbWidget::HbFlickDirection)), this, SLOT(changePage(HbInputVkbWidget::HbFlickDirection)));
}

/*!
Destructor.
*/
HbSctKeyboard::~HbSctKeyboard()
{
}

/*!
Returns the keyboard type.
*/
HbKeyboardType HbSctKeyboard::keyboardType() const
{
    Q_D(const HbSctKeyboard);
    return d->mType;
}

/*!
Returns the preferred keyboard size. HbVkbHost uses this information when it opens the keyboard.
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
Sets the keypad to given \a mode. Possible values are EModeAbc and EModeNumeric.
\a modifiers is a set of flags for shift, chr, and fn keys (see details in file
<tt>hbinputdef.h</tt>).
*/
void HbSctKeyboard::setMode(HbKeypadMode mode, HbModifiers modifiers)
{
    Q_D(HbSctKeyboard);

    d->mActivePage = 0;
    d->updateKeyCodes();

    HbInputVkbWidget::setMode(mode, modifiers);
}

/*!
Updates the button preview status.
*/
void HbSctKeyboard::updateButtonPreviewStatus(bool status)
{
    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup *>(contentItem());
    if (buttonGroup) {
        buttonGroup->setButtonPreviewEnabled(status);
    }
}

/*!
Handles the flick gesture.
*/
void HbSctKeyboard::changePage(HbInputVkbWidget::HbFlickDirection flickDirection)
{
    Q_D(HbSctKeyboard);

    if (d->mFlickAnimation &&
        (flickDirection == HbInputVkbWidget::HbFlickDirectionRight ||
        flickDirection == HbInputVkbWidget::HbFlickDirectionLeft)) {
        
        HbWidgetFeedback::triggered(this, Hb::InstantFlicked);
        d->changePage(flickDirection);
    }
}

/*!
Sends the key press event to the owning input method.
*/
void HbSctKeyboard::sendKeyPressEvent(const QKeyEvent &event)
{
    if (event.key() != HbInputButton::ButtonKeyCodePageChange &&
        event.key() != HbInputButton::ButtonKeyCodeSmiley) {
        HbInputVkbWidget::sendKeyPressEvent(event);
    }
}

/*!
Sends the key release event to the owning input method.
*/
void HbSctKeyboard::sendKeyReleaseEvent(const QKeyEvent &event)
{
    Q_D(HbSctKeyboard);

    if (event.key() == HbInputButton::ButtonKeyCodePageChange) {
        d->changePage(HbInputVkbWidget::HbFlickDirectionLeft);
    } else if (event.key() == HbInputButton::ButtonKeyCodeSmiley) {
        showSmileyPicker();
    } else {
        HbInputVkbWidget::sendKeyReleaseEvent(event);
    }
}

/*!
Sends the key change event to the owning input method.
The release event is ignored, and the press event is handled further.
*/
void HbSctKeyboard::sendKeyChangeEvent(const QKeyEvent &releaseEvent, const QKeyEvent &pressEvent)
{
    if (pressEvent.key() != HbInputButton::ButtonKeyCodePageChange &&
        pressEvent.key() != HbInputButton::ButtonKeyCodeSmiley) {
        HbInputVkbWidget::sendKeyChangeEvent(releaseEvent, pressEvent);
    }
}

// End of file
