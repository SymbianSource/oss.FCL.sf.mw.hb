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

#include "hbinputqwertynumerictouchkeyboard.h"
#include "hbinputqwertynumerictouchkeyboard_p.h"
#include "hbinputvkbwidget_p.h"
#include "hbinputbuttongroup.h"
#include "hbinputbutton.h"
#include "hbinputmodeindicator.h"

const qreal HbKeyboardHeightInUnits = 16.4;
const qreal HbKeyboardWidthInUnits = 95.5;

const int HbVirtualQwertyNumberOfRows = 2;
const int HbVirtualQwertyNumberOfColumns = 10;
const int HbButtonKeyCodeTable[HbVirtualQwertyNumberOfRows * HbVirtualQwertyNumberOfColumns] =
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
    HbInputButton::ButtonKeyCodeShift,
    HbInputButton::ButtonKeyCodeSymbol,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeDelete,
    HbInputButton::ButtonKeyCodeSettings,
    HbInputButton::ButtonKeyCodeCustom
};

HbQwertyNumericKeyboardPrivate::HbQwertyNumericKeyboardPrivate()
{
}

HbQwertyNumericKeyboardPrivate::~HbQwertyNumericKeyboardPrivate()
{
}

void HbQwertyNumericKeyboardPrivate::init()
{
    Q_Q(HbQwertyNumericKeyboard);

    HbInputVkbWidgetPrivate::init();

    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup*>(q->contentItem());
    if (buttonGroup) {
        buttonGroup->setGridSize(QSize(HbVirtualQwertyNumberOfColumns, HbVirtualQwertyNumberOfRows));

        QList<HbInputButton*> buttons;
        for (int i = 0; i < HbVirtualQwertyNumberOfColumns * HbVirtualQwertyNumberOfRows; ++i) {
            HbInputButton *item = new HbInputButton(HbButtonKeyCodeTable[i], QPoint(i % HbVirtualQwertyNumberOfColumns, i / HbVirtualQwertyNumberOfColumns));
            buttons.append(item);

            if (HbButtonKeyCodeTable[i] == HbInputButton::ButtonKeyCodeSettings) {
                mInputModeIndicator = new HbInputModeIndicator(item, q);
            } else if (HbButtonKeyCodeTable[i] == HbInputButton::ButtonKeyCodeShift ||
                       HbButtonKeyCodeTable[i] == HbInputButton::ButtonKeyCodeSymbol) {
                item->setState(HbInputButton::ButtonStateDisabled);
            }
        }
        buttonGroup->setButtons(buttons);
        buttonGroup->setButtonPreviewEnabled(HbInputSettingProxy::instance()->isCharacterPreviewForQwertyEnabled());

        QObject::connect(buttonGroup, SIGNAL(buttonPressed(const QKeyEvent&)), q, SLOT(sendKeyPressEvent(const QKeyEvent&)));
        QObject::connect(buttonGroup, SIGNAL(buttonDoublePressed(const QKeyEvent&)), q, SLOT(sendKeyDoublePressEvent(const QKeyEvent&)));
        QObject::connect(buttonGroup, SIGNAL(buttonReleased(const QKeyEvent&)), q, SLOT(sendKeyReleaseEvent(const QKeyEvent&)));
        QObject::connect(buttonGroup, SIGNAL(buttonLongPressed(const QKeyEvent&)), q, SLOT(sendLongPressEvent(const QKeyEvent&)));
        QObject::connect(buttonGroup, SIGNAL(pressedButtonChanged(const QKeyEvent&, const QKeyEvent&)), q, SLOT(sendKeyChangeEvent(const QKeyEvent&, const QKeyEvent&)));
    }
    QObject::connect(q, SIGNAL(flickEvent(HbInputVkbWidget::HbFlickDirection)), buttonGroup, SLOT(cancelButtonPress()));
}

int HbQwertyNumericKeyboardPrivate::keyCode(int buttonId)
{
    return HbButtonKeyCodeTable[buttonId];
}

void HbQwertyNumericKeyboardPrivate::applyEditorConstraints()
{
    Q_Q(HbQwertyNumericKeyboard);

    HbInputFocusObject *focusedObject = mOwner->focusObject();
    if (!focusedObject) {
        return;
    }

    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup*>(q->contentItem());
    if (buttonGroup) {
        QList<HbInputButton*> buttons = buttonGroup->buttons();
        for (int i = 0; i < buttons.count(); ++i) {
            if (keyCode(i) == HbInputButton::ButtonKeyCodeCharacter) {
                HbInputButton *item = buttons.at(i);

                HbInputButton::HbInputButtonState state = item->state();
                QString data = item->text(HbInputButton::ButtonTextIndexPrimary);
                if (data.isEmpty() || !focusedObject->characterAllowedInEditor(data.at(0))) {
                    state = HbInputButton::ButtonStateDisabled;
                } else if (item->state() == HbInputButton::ButtonStateDisabled) {
                    state = HbInputButton::ButtonStateReleased;
                }
                item->setState(state);
            }
        }
        buttonGroup->setButtons(buttons);
    }
}

void HbQwertyNumericKeyboardPrivate::updateKeyCodes()
{
    Q_Q(HbQwertyNumericKeyboard);

    QString characters;
    getCharacters(characters);

    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup*>(q->contentItem());
    if (buttonGroup) {
        int key = 0;
        int charKeyCount = 0;

        QList<HbInputButton*> buttons = buttonGroup->buttons();
        for (int i = 0; i < buttons.count(); ++i) {
            if (keyCode(i) == HbInputButton::ButtonKeyCodeCharacter) {
                HbInputButton *item = buttons.at(i);
                // digits always comes in the first row
                if (charKeyCount < 10) {
                    item->setKeyCode(numberCharacterBoundToKey((charKeyCount + 1) % 10).unicode());
                    charKeyCount++;
                } else if (key < characters.count()) {
                    item->setKeyCode(characters.at(key).unicode());
                    ++key;
                }
            }
        }
    }
}

void HbQwertyNumericKeyboardPrivate::updateButtons()
{
    Q_Q(HbQwertyNumericKeyboard);

    QString characters;
    getCharacters(characters);

    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup*>(q->contentItem());
    if (buttonGroup) {
        int key = 0;
        int charKeyCount = 0;

        QList<HbInputButton*> buttons = buttonGroup->buttons();
        for (int i = 0; i < buttons.count(); ++i) {
            if (keyCode(i) == HbInputButton::ButtonKeyCodeCharacter) {
                HbInputButton *item = buttons.at(i);
                if (charKeyCount < 10) {
                    item->setText(numberCharacterBoundToKey((charKeyCount + 1) % 10),
                    HbInputButton::ButtonTextIndexPrimary);  
                    charKeyCount++;					
                } else {
                    if (key < characters.count()) {
                        item->setText(characters.at(key), HbInputButton::ButtonTextIndexPrimary);
                    } else {
                        item->setText(QString(), HbInputButton::ButtonTextIndexPrimary);
                    }
                    ++key;
                }					
            }
        }
        buttonGroup->setButtons(buttons);
    }
}

void HbQwertyNumericKeyboardPrivate::getCharacters(QString &characters)
{
    if (mKeymap) {
        const HbKeyboardMap* keyboardMap = mKeymap->keyboard(HbKeyboardSctLandscape);
        if (!keyboardMap) {
            return;
        }
        QString chars;
		
        foreach (const HbMappedKey* mappedKey, keyboardMap->keys) {
            chars.append(mappedKey->characters(HbModifierNone));
        }          
            HbInputFocusObject *focusedObject = mOwner->focusObject();
            QString allowedChars;
            if (focusedObject) {
                focusedObject->filterStringWithEditorFilter(chars, allowedChars);
            }
            // Remove digits from it ( digits always come in the first row )
            for (int i=0; i < 10; i++) {
                allowedChars.remove(numberCharacterBoundToKey(i));
            }	
			
            foreach (QChar sctChar, allowedChars) {
                if (!characters.contains(sctChar)) {
                    characters.append(sctChar);
                }
            }
        
    }
}


/*!
Constructs the object. owner is the owning input method implementation. Keymap
is key mapping data to be used to display button texts. Key mapping data can be
changed later (for example when the input language changes) by calling
setKeymap.
*/
HbQwertyNumericKeyboard::HbQwertyNumericKeyboard(HbInputMethod *owner, const HbKeymap *keymap, QGraphicsItem *parent)
 : HbInputVkbWidget(*new HbQwertyNumericKeyboardPrivate, parent)
{
    Q_D(HbQwertyNumericKeyboard);
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
HbQwertyNumericKeyboard::HbQwertyNumericKeyboard(HbQwertyNumericKeyboardPrivate &dd, HbInputMethod *owner,
                                                 const HbKeymap *keymap, QGraphicsItem* parent)
 : HbInputVkbWidget(dd, parent)
{
    Q_D(HbQwertyNumericKeyboard);
    d->mOwner = owner;
    setKeymap(keymap);

    QObject::connect(HbInputSettingProxy::instance(), SIGNAL(characterPreviewStateForQwertyChanged(bool)), this, SLOT(updateButtonPreviewStatus(bool)));
}

/*!
Destructs the object.
*/
HbQwertyNumericKeyboard::~HbQwertyNumericKeyboard()
{
}

/*!
Returns keyboard type.
*/
HbKeyboardType HbQwertyNumericKeyboard::keyboardType() const
{
    return HbKeyboardVirtualQwerty;
}

/*!
Returns preferred keyboard size. HbVkbHost uses this information when it opens the keyboard.
*/
QSizeF HbQwertyNumericKeyboard::preferredKeyboardSize()
{
    Q_D(HbQwertyNumericKeyboard);

    QSizeF result;
    qreal unitValue = HbDeviceProfile::profile(mainWindow()).unitValue();

    result.setHeight(HbKeyboardHeightInUnits * unitValue + d->mCloseHandleHeight);
    result.setWidth(HbKeyboardWidthInUnits * unitValue);

    return QSizeF(result);
}

/*!
Updates button preview status.
*/
void HbQwertyNumericKeyboard::updateButtonPreviewStatus(bool status)
{
    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup*>(contentItem());
    if (buttonGroup) {
        buttonGroup->setButtonPreviewEnabled(status);   
    }
}

// End of file
