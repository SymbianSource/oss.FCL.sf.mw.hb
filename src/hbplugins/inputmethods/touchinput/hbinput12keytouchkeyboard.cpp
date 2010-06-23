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
#include <hbinpututils.h>
#include <hbframedrawer.h>

#include "hbinput12keytouchkeyboard.h"
#include "hbinput12keytouchkeyboard_p.h"
#include "hbinputbuttongroup.h"
#include "hbinputbutton.h"
#include "hbinputmodeindicator.h"

const qreal HbKeyboardHeightInUnits = 37.8;
const qreal HbKeyboardWidthInUnits = 53.8;

const int HbFirstRowIndex = 0;
const int HbSecondRowIndex = 2;
const int HbVirtual12KeyNumberOfRows = 4;
const int HbVirtual12KeyNumberOfColumns = 4;
const int HbButtonKeyCodeTable[HbVirtual12KeyNumberOfRows * HbVirtual12KeyNumberOfColumns] =
{
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeDelete,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeSymbol,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeSettings,
    HbInputButton::ButtonKeyCodeAsterisk,
    HbInputButton::ButtonKeyCodeCharacter,
    HbInputButton::ButtonKeyCodeShift,
    HbInputButton::ButtonKeyCodeCustom
};

Hb12KeyTouchKeyboardPrivate::Hb12KeyTouchKeyboardPrivate()
{
}

Hb12KeyTouchKeyboardPrivate::~Hb12KeyTouchKeyboardPrivate()
{
}

void Hb12KeyTouchKeyboardPrivate::init()
{
    Q_Q(Hb12KeyTouchKeyboard);

    HbInputVkbWidgetPrivate::init();

    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup*>(q->contentItem());
    if (buttonGroup) {
        buttonGroup->setGridSize(QSize(HbVirtual12KeyNumberOfColumns, HbVirtual12KeyNumberOfRows));

        QList<HbInputButton*> buttons;
        for (int i = 0; i < HbVirtual12KeyNumberOfColumns * HbVirtual12KeyNumberOfRows; ++i) {
            HbInputButton *item = new HbInputButton(HbButtonKeyCodeTable[i], QPoint(i % HbVirtual12KeyNumberOfColumns, i / HbVirtual12KeyNumberOfColumns));
            buttons.append(item);

            if (HbButtonKeyCodeTable[i] == HbInputButton::ButtonKeyCodeSettings) {
                mInputModeIndicator = new HbInputModeIndicator(item, q);
            } else if (HbButtonKeyCodeTable[i] == HbInputButton::ButtonKeyCodeDelete) {
                item->setIcon(HbIcon(HbInputButtonIconDelete2), HbInputButton::ButtonIconIndexPrimary);
            } else if (HbButtonKeyCodeTable[i] == HbInputButton::ButtonKeyCodeSymbol) {
                item->setIcon(HbIcon(HbInputButtonIconSymbol2), HbInputButton::ButtonIconIndexPrimary);
            } else if (HbButtonKeyCodeTable[i] == HbInputButton::ButtonKeyCodeAsterisk) {
                item->setText(QString("*"), HbInputButton::ButtonTextIndexPrimary);
                item->setText(QString("+"), HbInputButton::ButtonTextIndexSecondaryFirstRow);
                item->setType(HbInputButton::ButtonTypeNormal);
            } else if (HbButtonKeyCodeTable[i] == HbInputButton::ButtonKeyCodeShift) {
                item->setText(QString(" "), HbInputButton::ButtonTextIndexSecondaryFirstRow);
                item->setType(HbInputButton::ButtonTypeNormal);
            }
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

int Hb12KeyTouchKeyboardPrivate::keyCode(int buttonId)
{
    return HbButtonKeyCodeTable[buttonId];
}

void Hb12KeyTouchKeyboardPrivate::applyEditorConstraints()
{
    Q_Q(Hb12KeyTouchKeyboard);

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
                if (mMode == EModeNumeric) {
                    QString data = item->text(HbInputButton::ButtonTextIndexPrimary);
                    if (data.isEmpty() || !focusedObject->characterAllowedInEditor(data.at(0))) {
                        state = HbInputButton::ButtonStateDisabled;
                    } else if (item->state() == HbInputButton::ButtonStateDisabled) {
                        state = HbInputButton::ButtonStateReleased;
                    }
                } else if (mMode == EModeAbc) {
                    if (item->text(HbInputButton::ButtonTextIndexSecondaryFirstRow).isEmpty() &&
                        item->icon(HbInputButton::ButtonIconIndexSecondaryFirstRow).isNull()) {
                        state = HbInputButton::ButtonStateDisabled;
                    }  else if (item->state() == HbInputButton::ButtonStateDisabled) {
                        state = HbInputButton::ButtonStateReleased;
                    }
                }
            } else if (keyCode(i) == HbInputButton::ButtonKeyCodeSymbol) {
                if (mMode == EModeNumeric && focusedObject->editorInterface().isNumericEditor()) {
                    state = HbInputButton::ButtonStateDisabled;
                } else if (item->state() == HbInputButton::ButtonStateDisabled) {
                    state = HbInputButton::ButtonStateReleased;
                }
            } else if (keyCode(i) == HbInputButton::ButtonKeyCodeAsterisk ||
                       keyCode(i) == HbInputButton::ButtonKeyCodeShift) {
                QString sctCharacters;
                const HbKeyboardMap *keyboardMap = mKeymap->keyboard(HbKeyboardSctPortrait);
                if (keyboardMap) {
                    foreach (const HbMappedKey* mappedKey, keyboardMap->keys) {
                        focusedObject->filterStringWithEditorFilter(mappedKey->characters(HbModifierNone), sctCharacters);
                        if (sctCharacters.count()) {
                            break;
                        }
                    }
                }

                if (!sctCharacters.count() && mMode == EModeNumeric && focusedObject->editorInterface().isNumericEditor()) {
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

void Hb12KeyTouchKeyboardPrivate::updateButtons()
{
    Q_Q(Hb12KeyTouchKeyboard);

    HbInputButtonGroup *buttonGroup = static_cast<HbInputButtonGroup*>(q->contentItem());
    if (buttonGroup) {
        int key = 0;
        QList<HbInputButton*> buttons = buttonGroup->buttons();
        for (int i = 0; i < buttons.count(); ++i) {
            HbInputButton *item = buttons.at(i);
            if (keyCode(i) == HbInputButton::ButtonKeyCodeCharacter) {

                if (mMode == EModeNumeric) {
                    QChar numChr;
                    const HbKeyboardMap *labelMap = mKeymap->keyboard(HbKeyboardVirtual12KeyLabels);
                    const HbKeyboardMap *keyMap = mKeymap->keyboard(HbKeyboardVirtual12Key);
                    if (labelMap && key < labelMap->keys.count()) {
                        numChr = labelMap->keys.at(key)->keycode;
                    }

                    // Fallback to normal keymappings if key labels are not present
                    if (keyMap && key < keyMap->keys.count() && numChr.isNull()) {
                        numChr = numberCharacterBoundToKey(key);
                    }

                    item->setText(numChr, HbInputButton::ButtonTextIndexPrimary);
                    item->setText(QString(), HbInputButton::ButtonTextIndexSecondaryFirstRow);
                    item->setText(QString(), HbInputButton::ButtonTextIndexSecondarySecondRow);
                    item->setIcon(HbIcon(), HbInputButton::ButtonIconIndexPrimary);
                    item->setIcon(HbIcon(), HbInputButton::ButtonIconIndexSecondaryFirstRow);
                    item->setIcon(HbIcon(), HbInputButton::ButtonIconIndexSecondarySecondRow);
                } else if (mMode == EModeAbc) {
                    QString firstRow;
                    QString secondRow;
                    QChar numChr;
                    const HbKeyboardMap *labelMap = mKeymap->keyboard(HbKeyboardVirtual12KeyLabels);
                    const HbKeyboardMap *keyMap = mKeymap->keyboard(HbKeyboardVirtual12Key);
                    if (labelMap && key < labelMap->keys.count()) {
                        firstRow = keyLabel(labelMap->keys.at(key)->chars, HbFirstRowIndex | mModifiers);
                        secondRow = keyLabel(labelMap->keys.at(key)->chars, HbSecondRowIndex | mModifiers);
                        numChr = labelMap->keys.at(key)->keycode;
                    }

                    // Fallback to normal keymappings if key labels are not present
                    if (keyMap && key < keyMap->keys.count()) {
                        if (firstRow.isEmpty()) {
                            firstRow = keyMap->keys.at(key)->characters(mModifiers);
                            if (mOwner->focusObject()) {
                                QString allowedData;
                                mOwner->focusObject()->filterStringWithEditorFilter(firstRow, allowedData);
                                firstRow = allowedData.left(3);
                            } else {
                                firstRow = firstRow.left(3);
                           }
                        }
                        if (numChr.isNull()) {
                            numChr = numberCharacterBoundToKey(key);
                        }
                    }

                    if (key == 9) {
                        item->setText(numChr, HbInputButton::ButtonTextIndexPrimary);
                        item->setIcon(HbIcon(HbInputButtonIconSpace2), HbInputButton::ButtonIconIndexSecondaryFirstRow);
                    } else {
                        item->setText(firstRow, HbInputButton::ButtonTextIndexSecondaryFirstRow);
                        item->setText(secondRow, HbInputButton::ButtonTextIndexSecondarySecondRow);
                        item->setText(numChr, HbInputButton::ButtonTextIndexPrimary);
                    }
                }

                ++key;
            } else if (keyCode(i) == HbInputButton::ButtonKeyCodeShift) {
                if (mMode == EModeNumeric) {
                    item->setText(QString("#"), HbInputButton::ButtonTextIndexSecondaryFirstRow);
                } else if (mMode == EModeAbc) {
                    item->setText(QString(" "), HbInputButton::ButtonTextIndexSecondaryFirstRow);
                }
            }
        }
        buttonGroup->setButtons(buttons);
    }
}

QString Hb12KeyTouchKeyboardPrivate::keyLabel(const QStringList &labels, int index)
{
    if (index == HbFirstRowIndex && labels.count() >= 2) {
        return labels.at(1);
    } else if (index == (HbFirstRowIndex | HbModifierShiftPressed) && labels.count() >= 3) {
        return labels.at(2);
    } else if (index == HbSecondRowIndex && labels.count() >= 4) {
        return labels.at(3);
    } else if (index == (HbSecondRowIndex | HbModifierShiftPressed) && labels.count() >= 5) {
        return labels.at(4);
    }
    return QString();
}

/*!
Constructs the object. owner is the owning input method implementation. Keymap
is key mapping data to be used to display button texts. Key mapping data can be
changed later (for example when the input language changes) by calling
setKeymap.
*/
Hb12KeyTouchKeyboard::Hb12KeyTouchKeyboard(HbInputMethod *owner, const HbKeymap *keymap, QGraphicsItem *parent)
 : HbInputVkbWidget(*new Hb12KeyTouchKeyboardPrivate, parent)
{
    if (!owner) {
        return;
    }
    Q_D(Hb12KeyTouchKeyboard);
    d->mOwner = owner;
    setKeymap(keymap);
}

/*!
Constructs the object. owner is the owning input method implementation. Keymap
is key mapping data to be used to display button texts. Key mapping data can be
changed later (for example when the input language changes) by calling
setKeymap.
*/
Hb12KeyTouchKeyboard::Hb12KeyTouchKeyboard(Hb12KeyTouchKeyboardPrivate &dd, HbInputMethod *owner,
                                           const HbKeymap *keymap, QGraphicsItem *parent)
 : HbInputVkbWidget(dd, parent)
{
    if (!owner) {
        return;
    }
    Q_D(Hb12KeyTouchKeyboard);
    d->mOwner = owner;
    setKeymap(keymap);
}

/*!
Destructs the object.
*/
Hb12KeyTouchKeyboard::~Hb12KeyTouchKeyboard()
{
}

/*!
Returns keyboard type.
*/
HbKeyboardType Hb12KeyTouchKeyboard::keyboardType() const
{
    return HbKeyboardVirtual12Key;
}

/*!
Returns preferred keyboard size. HbVkbHost uses this information when it opens the keyboard.
*/
QSizeF Hb12KeyTouchKeyboard::preferredKeyboardSize()
{
    Q_D(Hb12KeyTouchKeyboard);

    QSizeF result;
    qreal unitValue = HbDeviceProfile::profile(mainWindow()).unitValue();

    result.setHeight(HbKeyboardHeightInUnits * unitValue + d->mCloseHandleHeight);
    result.setWidth(HbKeyboardWidthInUnits * unitValue);

    return QSizeF(result);
}

// End of file
