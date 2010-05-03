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

#include <QGraphicsGridLayout>
#include <hbinputmethod.h>
#include <hbinputkeymap.h>
#include <hbinputsettingproxy.h>
#include <hbstyleoptionlabel.h>
#include <hbaction.h>
#include <hbtextitem.h>
#include <hbframeitem.h>
#include <hbcolorscheme.h>

#include "hbinputcharpreviewpane.h"
#include "hbinputtouchkeypadbutton.h"
#include "hbinputqwertytouchkeyboard.h"
#include "hbinputqwertytouchkeyboard_p.h"
#include "hbinputvkbwidget_p.h"

const int HbVirtualQwertyNumberOfColumns = 10;
const int HbVirtualQwertyNumberOfRows = 4;
const int HbVirtualQwerty4x10MaxKeysCount = 32;
const int HbVirtualQwerty4x11MaxKeysCount = 36;
const int HbVirtualQwertyFunctionButtonCount = 5;
const int HbVirtualQwertyNumericKeypadButtonCount = 18;
const qreal HbVirtualQwertyButtonPreferredHeight = 58.0;
const QSizeF HbVirtualQwerty4x10LayoutDimensions(64.0, HbVirtualQwertyButtonPreferredHeight);
const QSizeF HbVirtualQwerty4x11LayoutDimensions(58.18, HbVirtualQwertyButtonPreferredHeight);
const int HbVirtualQwertyNumberOfRowsNumberMode = 2;

const QString HbButtonObjName = "qwerty ";
const QString HbCustomButtonObjName = "qwerty custom button ";
const QString HbEnterObjName = "qwerty enter";
const QString HbShiftObjName = "qwerty shift";
const QString HbControlObjName = "qwerty control";
const QString HbBackspaceObjName = "qwerty backspace";
const QString HbSpaceObjName = "qwerty space";

const QString HbQwertyButtonTextLayout = "_hb_qwerty_button_text_layout";
const QString HbQwertyButtonIconLayout = "_hb_qwerty_button_icon_layout";

/*!
\deprecated class HbQwertyKeyboard
*/

HbQwertyKeyboardPrivate::HbQwertyKeyboardPrivate()
:mCtrlBtnIndex(-1),
mPressedButtonIndex(-1),
mPreviewPane(0),
mInStickyRegion(false),
mLongKeyPressCharsShown(false),
mKeypadCreated(false),
mKeymapChanged(false),
mKeyboardSize(HbQwerty4x10), 
mSize(QSizeF())
{
}

void HbQwertyKeyboardPrivate::constructKeypad()
{
    Q_Q(HbQwertyKeyboard);

    if (mMode == EModeAbc) {
        const HbKeyboardMap *keyboardmap = mKeymap->keyboard(HbKeyboardVirtualQwerty);
        for (int i = 0; i < HbVirtualQwerty4x11MaxKeysCount; i++) {
            HbTouchKeypadButton *button = new HbTouchKeypadButton(q, textForKey(i), q);
            if (keyboardmap && (i < keyboardmap->keys.count())) {
                button->setKeyCode(keyboardmap->keys.at(i)->keycode.unicode());
            }
            button->setProperty(HbStyleRulesCacheId::hbStyleRulesForNodeCache, HbQwertyButtonTextLayout);
            button->setAsStickyButton(true);
            q->connect(button, SIGNAL(pressed()), mPressMapper, SLOT(map()));
            q->connect(button, SIGNAL(released()), mReleaseMapper, SLOT(map()));
            q->connect(button, SIGNAL(enteredInNonStickyRegion()), q, SLOT(enteredInNonStickyRegion()));
            if (i >= HbVirtualQwerty4x10MaxKeysCount) {
                button->hide();
            }
            mButtons.append(button);
        }
        //Create the function buttons
        for (int i = 0; i < HbVirtualQwertyFunctionButtonCount; ++i) {
            HbTouchKeypadButton *button = 0;
            switch ( i ) {
            case 0: {
                HbIcon icon("qtg_mono_backspace1");
                button = new HbTouchKeypadButton(q, icon, QString(), q);
                button->setAutoRepeatDelay(HbRepeatTimeout);
                button->setAutoRepeatInterval(HbRepeatTimeoutShort);
                button->setAutoRepeat(true);
                button->setProperty(HbStyleRulesCacheId::hbStyleRulesForNodeCache, HbQwertyButtonIconLayout);
                break;
                }
            case 1: {
                HbIcon icon("qtg_mono_enter");
                button = new HbTouchKeypadButton(q, icon, QString(), q);
                button->setProperty(HbStyleRulesCacheId::hbStyleRulesForNodeCache, HbQwertyButtonIconLayout);
                break;
                }
            case 2: {
                HbIcon icon("qtg_mono_shift");
                button = new HbTouchKeypadButton(q, icon, QString(), q);
                button->setProperty(HbStyleRulesCacheId::hbStyleRulesForNodeCache, HbQwertyButtonIconLayout);
                break;
                }
            case 3: {
                HbIcon icon("qtg_mono_sym_qwerty");
                button = new HbTouchKeypadButton(q, icon, QString(), q);
                button->setProperty(HbStyleRulesCacheId::hbStyleRulesForNodeCache, HbQwertyButtonIconLayout);
                mCtrlBtnIndex = mButtons.count()-1;
                break;
                }
            case 4: {
                button = new HbTouchKeypadButton(q, QString(), q);
                button->setFrameIcon("qtg_mono_space");
                button->setAutoRepeatDelay(HbRepeatTimeout);
                button->setAutoRepeatInterval(HbRepeatTimeoutShort);
                button->setAutoRepeat(true);
                break;
                }
            default:
                break;
            }
            button->setButtonType(HbTouchKeypadButton::HbTouchButtonFunction);
            button->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
            button->setAsStickyButton(false);
            q->connect(button, SIGNAL(pressed()), mPressMapper, SLOT(map()));
            q->connect(button, SIGNAL(released()), mReleaseMapper, SLOT(map()));
            q->connect(button, SIGNAL(enteredInNonStickyRegion()), q, SLOT(enteredInNonStickyRegion()));
            mButtons.append(button);
        }
    } else {
        QString allowedSctCharacters;
        getAllowedSctCharcters(allowedSctCharacters);
        int sctIndex = 1;
        // Construct Numeric Keypad
        for(int jj = 0; jj < HbVirtualQwertyNumericKeypadButtonCount ; jj++) {
            HbTouchKeypadButton *button = 0;
            if (jj < 10) {
                button = new HbTouchKeypadButton(q, QString::number((jj+1)%10), q);
                button->setAsStickyButton(true);
                setButtonObjectName(*button, 0, jj, Qt::Key_unknown);
            } else if(jj>= 12 && jj<=16) {
                // One todo is what if their are more than 5 sct characters that are allowed 
                // in to the editor. UI Concept is not clear for this. Need to revisit this.
                // But as of now, there are not numeric editors which have more than 5 sct 
                // characters allowed in to them.
                QString buttonText;
                if(allowedSctCharacters.length() >= sctIndex) {
                    buttonText = allowedSctCharacters[sctIndex-1];
                }
                button = new HbTouchKeypadButton(q, buttonText, q);
                button->setAsStickyButton(true);
                setButtonObjectName(*button, 1, jj%10 , Qt::Key_unknown);
                sctIndex++;
            } else {
                //construct the function button in numeric keypad
                switch(jj) {
                    case 10: {
                        HbIcon icon("qtg_mono_shift");
                        button = new HbTouchKeypadButton(q, icon, QString(), q);
                        setButtonObjectName(*button, 1, 0, Qt::Key_Shift);
                        button->setButtonType(HbTouchKeypadButton::HbTouchButtonFunction);
                        button->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
                        button->setAsStickyButton(false);
                        button->setEnabled(false);
                        }
                        break;
                    case 11: {
                        HbIcon icon("qtg_mono_sym_qwerty");
                        button = new HbTouchKeypadButton(q, icon, QString(), q);
                        setButtonObjectName(*button, 1, 1, Qt::Key_Control);
                        button->setButtonType(HbTouchKeypadButton::HbTouchButtonFunction);
                        button->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
                        button->setAsStickyButton(false);
                        button->setEnabled(false);
                        }
                        break;
                    case 17: {
                        HbIcon icon("qtg_mono_backspace1");
                        button = new HbTouchKeypadButton(q, icon, QString(), q);
                        setButtonObjectName(*button, 1, 7, Qt::Key_Backspace);
                        button->setButtonType(HbTouchKeypadButton::HbTouchButtonFunction);
                        button->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
                        button->setAsStickyButton(false);
                        button->setAutoRepeatDelay(HbRepeatTimeout);
                        button->setAutoRepeatInterval(HbRepeatTimeoutShort);
                        button->setAutoRepeat(true);
                        }
                        break;
                    default:
                        break;
                    }
                }

            q->connect(button, SIGNAL(pressed()), mPressMapper, SLOT(map()));
            q->connect(button, SIGNAL(released()), mReleaseMapper, SLOT(map()));
            q->connect(button, SIGNAL(enteredInNonStickyRegion()), q, SLOT(enteredInNonStickyRegion()));
            mButtons.append(button);
        }
    }


    // intercepting signal before passing to mOwner
    q->connect(mPressMapper, SIGNAL(mapped(int)), q, SLOT(mappedKeyPress(int)));
    q->connect(mReleaseMapper, SIGNAL(mapped(int)), q, SLOT(mappedKeyRelease(int)));
}

void HbQwertyKeyboardPrivate::getAllowedSctCharcters(QString & allowedSctCharacters)
{
    QString sctCharacters;
    if (mKeymap) {
        const HbKeyboardMap* keymap = mKeymap->keyboard(HbKeyboardSctLandscape);
        if (keymap == 0) {
            return;
        }
        foreach (const HbMappedKey* mappedKey, keymap->keys) {
            sctCharacters.append(mappedKey->characters(HbModifierNone));
        }
    }

    HbInputFocusObject *focusedObject = mOwner->focusObject();
    QString tempAllowedSctCharacters;
    if(focusedObject) {
        focusedObject->filterStringWithEditorFilter(sctCharacters,tempAllowedSctCharacters);
    }

    // Remove digits from it ( digits always come in the first row )
    allowedSctCharacters.clear();
    for(int i=0; i<tempAllowedSctCharacters.length() ;i++) {
        if(!(tempAllowedSctCharacters[i]>='0' && tempAllowedSctCharacters[i] <= '9' )) {
            // dont add duplicates to the list
            if(!allowedSctCharacters.contains(tempAllowedSctCharacters[i])) {
                allowedSctCharacters.append(tempAllowedSctCharacters[i]);
            }
        }
    }
}

void HbQwertyKeyboardPrivate::updateButtonsTextAndMappers()
{
    if (mMode == EModeNumeric) {
        QString allowedSctCharacters;
        getAllowedSctCharcters(allowedSctCharacters);
        int sctIndex = 1;
        for (int jj = 0; jj < HbVirtualQwertyNumericKeypadButtonCount ; jj++) {
            if (jj>=12 && jj<=16) {
                QString buttonText;
                if (allowedSctCharacters.length() >= sctIndex) {
                    buttonText = allowedSctCharacters[sctIndex-1];
                }
                mButtons[jj]->setText(buttonText);
                // Update press and release mapper.
                mReleaseMapper->removeMappings(mButtons.at(jj));
                mPressMapper->removeMappings(mButtons.at(jj));
                if(!mButtons.at(jj)->text().isEmpty()) {
                    mReleaseMapper->setMapping(mButtons.at(jj), mButtons.at(jj)->text().at(0).unicode());
                    mPressMapper->setMapping(mButtons.at(jj), mButtons.at(jj)->text().at(0).unicode());
                } 
                sctIndex++;
            }
        }
    } else { // mMode == EModeAbc
        const HbKeyboardMap* keymap = mKeymap->keyboard(HbKeyboardVirtualQwerty);
        if (keymap == 0) {
            return;
        }
        int keymapCount = keymap->keys.count();

        for (int i = 0; i < mButtons.count(); i++) {
            if (i < keymapCount) {
                mButtons.at(i)->setText(textForKey(i));
                mButtons.at(i)->setKeyCode(keymap->keys.at(i)->keycode.unicode());
                if (mKeymapChanged) {
                    HbTouchKeypadButton *button = mButtons.at(i);
                    mReleaseMapper->removeMappings(button);
                    mPressMapper->removeMappings(button);
                    mReleaseMapper->setMapping(button, keymap->keys.at(i)->keycode.unicode());
                    mPressMapper->setMapping(button, keymap->keys.at(i)->keycode.unicode());
                }
            }
        }
    }
}

HbQwertyKeyboardPrivate::~HbQwertyKeyboardPrivate()
{
    delete mPreviewPane;
    mPreviewPane = 0;

    while (!mKeypadButtonOption.isEmpty()) {
        delete mKeypadButtonOption.takeFirst();
    }
}

void HbQwertyKeyboardPrivate::launchPreviewPane(const QStringList& list)
{
    if (mPressedButtonIndex > -1 && mPressedButtonIndex < mButtons.size()) {
        mPreviewPane->showCharacters(list, mButtons.at(mPressedButtonIndex)->sceneBoundingRect());
    }
}

int HbQwertyKeyboardPrivate::indexForKeycode(int keycode)
{
    int index = -1;
    if (mMode == EModeNumeric) {
        switch(keycode) {
            case Qt::Key_Control:
                index = 10; // First key of second row
                break;
            case Qt::Key_Shift:
                index = 11;
                break;
            case Qt::Key_Backspace:
                index = 17;
                break;
            case '0':
                index = 9;
                break;
            default:
                if (keycode >= '1' && keycode <= '9') {
                    index = keycode - '1';
                } else {
                    QString sctChars;
                    getAllowedSctCharcters(sctChars);
                    sctChars.truncate(5);
                    if (sctChars.contains(QChar(keycode))) {
                        index = sctChars.indexOf(QChar(keycode)) + 12;
                    }
                }
                break;
        }
    } else {
        const HbKeyboardMap* keymap = mKeymap->keyboard(HbKeyboardVirtualQwerty);
        for (int i = 0; i < keymap->keys.count(); ++i) {
            if (keymap->keys.at(i)->keycode == keycode) {
                index = i;
                break;
            }
        }
    }
    return index;
}

int HbQwertyKeyboardPrivate::keyCode(int buttonId)
{
    return buttonId;
}

int HbQwertyKeyboardPrivate::keyCode(HbTouchKeypadButton *button)
{
    int code = -1;
    if(button) {
        code = button->keyCode();
    }
    return code;
}

void HbQwertyKeyboardPrivate::handleStandardButtonPress(int buttonId)
{
    HbInputVkbWidgetPrivate::handleStandardButtonPress(buttonId);

    // A new button is pressed so we should close
    // preview pane on the previous button.
    if (mPreviewPane->isVisible()) {
        mPreviewPane->hide();
    }
    // a new button is pressed so we should reset the state of the
    // long press character preview pane.
    mLongKeyPressCharsShown = false;

    if (buttonId < 0) {
        return;
    }

    mPressedButtonIndex = indexForKeycode(buttonId);

    if (!(buttonId & 0xffff0000) && showPreview(buttonId)) {
        mInStickyRegion = false;
        // Show character preview only incase of setting proxy allows us to do.
        if (HbInputSettingProxy::instance()->isCharacterPreviewForQwertyEnabled()) {
            if (mPressedButtonIndex >= 0 && mPressedButtonIndex < mButtons.count()) {
                if(mButtons.at(mPressedButtonIndex)->isFaded()) {
                    return;  // if the button is inactive, dont show character preview popup.
                }
                const QString &text = mButtons.at(mPressedButtonIndex)->text();
                if (text.size()) {
                    QStringList list;
                    list.append(text);
                    // let's show the character preview.
                    launchPreviewPane(list);
                    return;
                }
            }
        }
    }
}

void HbQwertyKeyboardPrivate::handleStandardButtonRelease(int buttonId)
{
    // mLongKeyPressCharsShown will be true in case there is a long key press
    // detected and preview pane is showing some character(s) to be selected
    // by user. so when mLongKeyPressCharsShown is true we should not close
    // the preview pane.
    if (!mLongKeyPressCharsShown) {
        if (mPreviewPane->isVisible()) {
            mPreviewPane->hide();
        }

	/* Release Event is handled in Button Release as we do not get Click event from
	pushButton on longpress of the button
	*/

    // handle keypress only if there was no flick
    if (mFlickDirection==HbInputVkbWidget::HbFlickDirectionNone && buttonId >= 0){
            QKeyEvent releaseEvent(QEvent::KeyRelease, buttonId, Qt::NoModifier);
            if (mOwner) {
                mOwner->filterEvent(&releaseEvent);
            }
        }
    }
}

void HbQwertyKeyboardPrivate::setLayoutDimensions(QSizeF dimensions)
{
    // only update the dimensions if they are not previously set and buttons have been created
    if (mSize == dimensions || !mButtons.count()) {
        return;
    }
    mSize = dimensions;

    mButtonLayout->setContentsMargins(0.0, 0.0, 0.0, 0.0);
    int numberOfRows = HbVirtualQwertyNumberOfRows;

    foreach (HbTouchKeypadButton* button, mButtons) {
        button->setInitialSize(dimensions);
    }
    if (mSettingsButton && mApplicationButton) {
        mSettingsButton->setInitialSize(dimensions);
        mApplicationButton->setInitialSize(dimensions);
    }

    if ( EModeNumeric == mMode ) {
        numberOfRows = HbVirtualQwertyNumberOfRowsNumberMode;
    }
    if (mKeyboardSize == HbQwerty4x10) {
        for (int jj = 0; jj < HbVirtualQwertyNumberOfColumns; jj++) {
            mButtonLayout->setColumnFixedWidth(jj, dimensions.width() - HorizontalSpacing);
        }
    } else {
        for (int jj = 0; jj < HbVirtualQwertyNumberOfColumns + 1; jj++) {
            mButtonLayout->setColumnFixedWidth(jj, dimensions.width() - HorizontalSpacing);
        }
    }
    for (int jj = 0; jj < numberOfRows; jj++) {
        mButtonLayout->setRowFixedHeight(jj, dimensions.height() - VerticalSpacing);
    }

    mButtonLayout->setHorizontalSpacing(HorizontalSpacing);
    mButtonLayout->setVerticalSpacing(VerticalSpacing);
}

QSizeF HbQwertyKeyboardPrivate::calculateDimensions(QSizeF size)
{
    QSizeF dimensions;
    if (mKeyboardSize == HbQwertyKeyboardPrivate::HbQwerty4x10) {
        dimensions.setWidth(size.width() / (qreal)HbVirtualQwertyNumberOfColumns);
    } else {
        dimensions.setWidth(size.width() / (qreal)(HbVirtualQwertyNumberOfColumns + 1));
    }
    if (mMode == EModeNumeric) {
        dimensions.setHeight(size.height() / (qreal)HbVirtualQwertyNumberOfRowsNumberMode);
    } else {
        dimensions.setHeight(size.height() / (qreal)HbVirtualQwertyNumberOfRows);
    }
    return dimensions;
}

QString HbQwertyKeyboardPrivate::textForKey(int key)
{
    QString keydata;
    if (mKeymap->keyboard(HbKeyboardVirtualQwerty)->keys.count() <= key) {
        return QString();
    }
    if (mModifiers & HbModifierShiftPressed) {
        keydata = mKeymap->keyboard(HbKeyboardVirtualQwerty)->keys.at(key)->characters(HbModifierShiftPressed);
    } else {
        keydata = mKeymap->keyboard(HbKeyboardVirtualQwerty)->keys.at(key)->characters(HbModifierNone);
    }
    return keydata.left(1);
}

void HbQwertyKeyboardPrivate::initializeNumericKeyboard()
{
    Q_Q(HbQwertyKeyboard);
    removeExistingSignalMappings();
    const HbKeyboardMap* keymap = mKeymap->keyboard(HbKeyboardVirtualQwerty);
    if (keymap == 0) {
        return;
    }
    for (int i = 0; i < HbVirtualQwertyNumericKeypadButtonCount; ++i) {
        if (i <= 9) {
            mButtonLayout->addItem(mButtons.at(i), 0, i);
            mReleaseMapper->setMapping(mButtons.at(i), mButtons.at(i)->text().at(0).unicode());
            mPressMapper->setMapping(mButtons.at(i), mButtons.at(i)->text().at(0).unicode());
        } else if(i>=12 && i<= 16) {
            mButtonLayout->addItem(mButtons.at(i), 1, i%10);
            if(!mButtons.at(i)->text().isEmpty()) {
                mReleaseMapper->setMapping(mButtons.at(i), mButtons.at(i)->text().at(0).unicode());
                mPressMapper->setMapping(mButtons.at(i), mButtons.at(i)->text().at(0).unicode());
            } 
        } else  {
            switch(i) {
            case 10: {
                mButtonLayout->addItem(mButtons.at(i), 1, 0);
                mReleaseMapper->setMapping(mButtons.at(i), Qt::Key_Shift);
                mPressMapper->setMapping(mButtons.at(i), Qt::Key_Shift);
                }
                break;
            case 11: {
                mButtonLayout->addItem(mButtons.at(i), 1, 1);
                mReleaseMapper->setMapping(mButtons.at(i), Qt::Key_Control);
                mPressMapper->setMapping(mButtons.at(i), Qt::Key_Control);
                }
                break;
            case 17: {
                mButtonLayout->addItem(mButtons.at(i), 1, 7);
                mReleaseMapper->setMapping(mButtons.at(i), Qt::Key_Backspace);
                mPressMapper->setMapping(mButtons.at(i), Qt::Key_Backspace);
                }
                break;
            default:
                break;
            }
        }
    }

    q->setupToolCluster();
    if (mSettingsButton) {
        mSettingsButton->setObjectName(HbCustomButtonObjName + QString::number(2));
        mButtonLayout->addItem(mSettingsButton, 1, 8);
    }
    if (mApplicationButton) {
        mApplicationButton->setObjectName(HbCustomButtonObjName + QString::number(3));
        mButtonLayout->addItem(mApplicationButton, 1, 9);
    }

    setLayoutDimensions(calculateDimensions(q->keypadButtonAreaSize()));
    mKeypadCreated = true;
}

void HbQwertyKeyboardPrivate::initializeKeyboard(bool refreshButtonText)
{
    const HbKeyboardMap* keymap = mKeymap->keyboard(HbKeyboardVirtualQwerty);
    if (mKeymap == 0) {
        return;
    }

    int keymapCount = keymap->keys.count();
    if (mMode == EModeNumeric) {
        initializeNumericKeyboard();
    } else if (keymapCount <= HbVirtualQwerty4x10MaxKeysCount) {
        initialize4x10Keypad(refreshButtonText);
    } else {
        initialize4x11Keypad(refreshButtonText);
    }
    mKeypadCreated = true;
}

void HbQwertyKeyboardPrivate::setRockerPosition()
{
    Q_Q(HbQwertyKeyboard);

    // Set rocker position.
    QSizeF padArea = q->keypadButtonAreaSize();
    QPointF point((padArea.width() * 0.5) - (mRocker->size().width() * 0.5),
        (padArea.height() * 0.5) - (mRocker->size().height() * 0.5));
    point.setY(point.y() + HbCloseHandleHeight);

    if (q->keypadLayout() && q->keypadLayout()->geometry().height()) {
        if(mKeyboardSize == HbQwerty4x10) {
            point.setX((padArea.width() * 0.5) - (mRocker->size().width() * 0.5));
            point.setY((q->keypadLayout()->geometry().height() * 0.5) - (mRocker->size().height() * 0.5) + HbCloseHandleHeight);
        } else {
            point.setX(((padArea.width() * 0.5) - (mRocker->size().width() * 0.5)) + HbVirtualQwerty4x11LayoutDimensions.width()/2);
            point.setY((q->keypadLayout()->geometry().height() * 0.5) - (mRocker->size().height() * 0.5) + HbCloseHandleHeight);
        }
    }
    mRocker->setPos(point);
}

void HbQwertyKeyboardPrivate::removeExistingSignalMappings()
{
    // removes all mappings except settings button and application button
    if (mButtonLayout) {
        int count = mButtonLayout->count();
        for (int jj=0; jj < count-2 ; jj++) {
            HbTouchKeypadButton * button = mButtons.at(jj);
            mReleaseMapper->removeMappings(button);
            mPressMapper->removeMappings(button);
        }
        for (int i = mButtonLayout->count() - 1; i >= 0; i--) {
            mButtonLayout->removeAt(i);
        }
    }
}

void HbQwertyKeyboardPrivate::initialize4x10Keypad(bool refreshButtonText)
{
    removeExistingSignalMappings();
    mKeyboardSize = HbQwerty4x10;
    Q_Q(HbQwertyKeyboard);
    const HbKeyboardMap* keymap = mKeymap->keyboard(HbKeyboardVirtualQwerty);
    if (keymap == 0) {
        return;
    }

    for (int i = 0; i < keymap->keys.count() && i < HbVirtualQwerty4x11MaxKeysCount; ++i) {
        if (refreshButtonText) {
            mButtons[i]->setText(textForKey(i));
            mButtons[i]->setKeyCode(keymap->keys.at(i)->keycode.unicode());
        }
        mReleaseMapper->setMapping(mButtons.at(i), keymap->keys.at(i)->keycode.unicode());
        mPressMapper->setMapping(mButtons.at(i), keymap->keys.at(i)->keycode.unicode());
        int row = 0;
        int column = 0;
        if (i < 10) {
            row = 0;
            column = i;
        } else if (i < 19) {
            row = 1;
            column = i-10;
        } else if (i < 28) {
            row = 2;
            column = i-19;
        } else {
            row = 3;
            column = i-28;
            if (column == 0) {
                column = 2;
            } else if (column == 1) {
                column = 3;
            } else if (column == 2) {
                column = 6;
            } else if (column == 3) {
                column = 7;
            }
        }
        mButtonLayout->addItem(mButtons.at(i), row, column);
        setButtonObjectName(*mButtons.at(i), row, column, Qt::Key_unknown);
    }

    for (int i = HbVirtualQwerty4x10MaxKeysCount; i < HbVirtualQwerty4x11MaxKeysCount; ++i) {
        mButtons[i]->hide();
    }

    for (int i = 0; i < 5; ++i) {
        switch ( i ) {
        case 0: {
            mButtonLayout->addItem(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), 1, 9);
            setButtonObjectName(*mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), 1, 9, Qt::Key_Backspace);
            mReleaseMapper->setMapping(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), Qt::Key_Backspace);
            mPressMapper->setMapping(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), Qt::Key_Backspace);
            break;
            }
        case 1: {
            mButtonLayout->addItem(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), 2, 9);
            setButtonObjectName(*mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), 2, 9, Qt::Key_Enter);
            mReleaseMapper->setMapping(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), Qt::Key_Enter);
            mPressMapper->setMapping(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), Qt::Key_Enter);
            break;
            }
        case 2: {
            mButtonLayout->addItem(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), 3, 0);
            setButtonObjectName(*mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), 3, 0, Qt::Key_Shift);
            mReleaseMapper->setMapping(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), Qt::Key_Shift);
            mPressMapper->setMapping(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), Qt::Key_Shift);
            break;
            }
        case 3: {
            mButtonLayout->addItem(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), 3, 1);
            setButtonObjectName(*mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), 3, 1, Qt::Key_Control);
            mReleaseMapper->setMapping(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), Qt::Key_Control);
            mPressMapper->setMapping(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), Qt::Key_Control);
            break;
            }
        case 4: {
            mButtonLayout->addItem(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), 3, 4, 1, 2);
            setButtonObjectName(*mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), 3, 4, Qt::Key_Space);
            mReleaseMapper->setMapping(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), Qt::Key_Space);
            mPressMapper->setMapping(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), Qt::Key_Space);
            break;
            }
        default:
            break;
        }
    }
    q->setupToolCluster();
    if (mSettingsButton) {
        mSettingsButton->setObjectName(HbCustomButtonObjName + QString::number(2));
        mButtonLayout->addItem(mSettingsButton, 3, 8);
    }
    if (mApplicationButton) {
        mApplicationButton->setObjectName(HbCustomButtonObjName + QString::number(3));
        mButtonLayout->addItem(mApplicationButton, 3, 9);
    }

    setLayoutDimensions(calculateDimensions(q->keypadButtonAreaSize()));
}

void HbQwertyKeyboardPrivate::initialize4x11Keypad(bool refreshButtonText)
{
    removeExistingSignalMappings();
    mKeyboardSize = HbQwerty4x11;
    Q_Q(HbQwertyKeyboard);
    const HbKeyboardMap* keymap = mKeymap->keyboard(HbKeyboardVirtualQwerty);
    if (keymap == 0) {
        return;
    }

    for (int i = 0; i < keymap->keys.count() && i <= HbVirtualQwerty4x11MaxKeysCount; ++i) {
        if (refreshButtonText) {
            mButtons[i]->setText(textForKey(i));
            mButtons[i]->setKeyCode(keymap->keys.at(i)->keycode.unicode());
        }
        mReleaseMapper->setMapping(mButtons.at(i), keymap->keys.at(i)->keycode.unicode());
        mPressMapper->setMapping(mButtons.at(i), keymap->keys.at(i)->keycode.unicode());
        int row = 0;
        int column = 0;
        if (i < 11) {
            row = 0;
            column = i;
        } else if (i < 21) {
            row = 1;
            column = i-11;
        } else if (i < 31) {
            row = 2;
            column = i-21;
        } else {
            row = 3;
            column = i-31;
            if (column == 0) {
                column = 2;
            } else if (column == 1) {
                column = 3;
            } else if (column == 2) {
                column = 6;
            } else if (column == 3) {
                column = 7;
            } else if (column == 4) {
                column = 8;
            }
        }
        mButtonLayout->addItem(mButtons.at(i), row, column);
        setButtonObjectName(*mButtons.at(i), row, column, Qt::Key_unknown);
        if (i >= HbVirtualQwerty4x10MaxKeysCount) {
            mButtons[i]->show();
        }
    }

    for (int i = 0; i < 5; ++i) {
        switch ( i ) {
        case 0: {
            mButtonLayout->addItem(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), 1, 10);
            setButtonObjectName(*mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), 1, 10, Qt::Key_Backspace);
            mReleaseMapper->setMapping(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), Qt::Key_Backspace);
            mPressMapper->setMapping(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), Qt::Key_Backspace);
            break;
            }
        case 1: {
            mButtonLayout->addItem(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), 2, 10);
            setButtonObjectName(*mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), 2, 10, Qt::Key_Enter);
            mReleaseMapper->setMapping(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), Qt::Key_Enter);
            mPressMapper->setMapping(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), Qt::Key_Enter);
            break;
            }
        case 2: {
            mButtonLayout->addItem(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), 3, 0);
            setButtonObjectName(*mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), 3, 0, Qt::Key_Shift);
            mReleaseMapper->setMapping(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), Qt::Key_Shift);
            mPressMapper->setMapping(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), Qt::Key_Shift);
            break;
            }
        case 3: {
            mButtonLayout->addItem(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), 3, 1);
            setButtonObjectName(*mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), 3, 1, Qt::Key_Control);
            mReleaseMapper->setMapping(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), Qt::Key_Control);
            mPressMapper->setMapping(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), Qt::Key_Control);
            break;
            }
        case 4: {
            mButtonLayout->addItem(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), 3, 4, 1, 2);
            setButtonObjectName(*mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), 3, 4, Qt::Key_Space);
            mReleaseMapper->setMapping(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), Qt::Key_Space);
            mPressMapper->setMapping(mButtons.at(HbVirtualQwerty4x11MaxKeysCount+i), Qt::Key_Space);
            break;
            }
        default:
            break;
        }
    }

    q->setupToolCluster();
    if (mSettingsButton) {
        mSettingsButton->setObjectName(HbCustomButtonObjName + QString::number(2));
        mButtonLayout->addItem(mSettingsButton, 3, 9);
    }
    if (mApplicationButton) {
        mApplicationButton->setObjectName(HbCustomButtonObjName + QString::number(3));
        mButtonLayout->addItem(mApplicationButton, 3, 10);
    }

    setLayoutDimensions(calculateDimensions(q->keypadButtonAreaSize()));
}

bool HbQwertyKeyboardPrivate::showPreview(int keycode)
{
    if (keycode == Qt::Key_Enter ||
        keycode == Qt::Key_Shift ||
        keycode == Qt::Key_Control ||
        keycode == Qt::Key_Backspace ||
        keycode == Qt::Key_Space) {
        return false;
    }
    return true;
}

void HbQwertyKeyboardPrivate::setButtonObjectName(HbTouchKeypadButton& button, int row, int column, Qt::Key specialKey)
{
    // bs, enter, more, pred, .com, space, 123sym, shift
    QString objName;
    switch(specialKey) {
    case Qt::Key_Enter:
        objName = HbEnterObjName;
        break;
    case Qt::Key_Shift:
        objName = HbShiftObjName;
        break;
    case Qt::Key_Control:
        objName = HbControlObjName;
        break;
    case Qt::Key_Backspace:
        objName = HbBackspaceObjName;
        break;
    case Qt::Key_Space:
        objName = HbSpaceObjName;
        break;
    default:
        objName = (HbButtonObjName + QString::number(row+1) + "," + QString::number(column+1) );
        break;
    }
    button.setObjectName(objName);
}

/*!
Apply editor constraints to the vkb
*/
void HbQwertyKeyboardPrivate::applyEditorConstraints()
{
    HbInputFocusObject *focusedObject = 0;
    if (mOwner) {
        focusedObject = mOwner->focusObject();
    }

    if(!focusedObject || isKeyboardDimmed()) {
    // dont need to apply constraints when keypad is dimmed.
    // applyEditorConstraints will be called from setKeyboardDimmed(false)
        return;
    }

    for (int i = 0; i < mButtons.count(); i++) {
        if(Hb::ItemType_InputCharacterButton == mButtons.at(i)->type()) {
            QString buttonText = mButtons.at(i)->text();
            if (buttonText.isEmpty() || !focusedObject->characterAllowedInEditor(buttonText[0])) {
                mButtons.at(i)->setFade(true);
            } else {
                mButtons.at(i)->setFade(false);
            }
        }
    }
}

//
// HbQwertyKeyboard
//

/*!
\deprecated HbQwertyKeyboard::HbQwertyKeyboard(HbInputMethod*, const HbKeymap*, QGraphicsItem*, HbKeypadMode)
    is deprecated.
*/
HbQwertyKeyboard::HbQwertyKeyboard(HbInputMethod* owner,
                                   const HbKeymap* keymap,
                                   QGraphicsItem* aParent, HbKeypadMode mode)
                                   : HbInputVkbWidget(*new HbQwertyKeyboardPrivate, aParent)
{
    Q_D(HbQwertyKeyboard);
    d->mKeymap = keymap;
    d->mClickMapper = new QSignalMapper(this);
    d->q_ptr = this;
    d->mOwner = owner;

    d->mButtonLayout = new QGraphicsGridLayout();

    // character preview pane
    d->mPreviewPane = new HbCharPreviewPane();
    connect(d->mPreviewPane, SIGNAL(charFromPreviewSelected(QString)), this, SIGNAL(charFromPreviewSelected(QString)));
    // A QGraphicsItem bydefault is shown so we need to hide it.
    d->mPreviewPane->hide();

    d->mMode = mode;
}

/*!
\deprecated HbQwertyKeyboard::HbQwertyKeyboard(HbQwertyKeyboardPrivate&, QGraphicsItem*)
    is deprecated.
*/
HbQwertyKeyboard::HbQwertyKeyboard(HbQwertyKeyboardPrivate &dd, QGraphicsItem* parent)
: HbInputVkbWidget(dd, parent)
{
}

/*!
\deprecated HbQwertyKeyboard::~HbQwertyKeyboard()
    is deprecated.
*/
HbQwertyKeyboard::~HbQwertyKeyboard()
{
}

/*!
\reimp
\deprecated HbQwertyKeyboard::keyboardType() const
    is deprecated.
*/
HbKeyboardType HbQwertyKeyboard::keyboardType() const
{
    return HbKeyboardVirtualQwerty;
}

/*!
\reimp
\deprecated HbQwertyKeyboard::setMode(HbKeypadMode, QFlags<HbModifier>)
    is deprecated.
*/
void HbQwertyKeyboard::setMode(HbKeypadMode mode, HbModifiers modifiers)
{
    Q_D(HbQwertyKeyboard);

    if(d->mMode == EModeNumeric && d->mKeypadCreated) {
        // for numeric edito we need to update sct character button everytime
        // we move between editors. ( dialer editor, digits only, formatted editor ect)
        d->updateButtonsTextAndMappers();
    }

	setupToolCluster();
    if (d->mMode == mode && d->mModifiers == modifiers && d->mKeypadCreated && !d->mKeymapChanged) {
        d->applyEditorConstraints();
        return;
    }

    d->mMode = mode;
    d->mModifiers = modifiers;

    const HbKeyboardMap* keymap = d->mKeymap->keyboard(HbKeyboardVirtualQwerty);
    if (keymap == 0) {
        return;
    }

    int keymapCount = keymap->keys.count();

    if (!d->mKeypadCreated
        || (keymapCount == HbVirtualQwerty4x10MaxKeysCount && d->mKeyboardSize == HbQwertyKeyboardPrivate::HbQwerty4x11)
        || (keymapCount == HbVirtualQwerty4x11MaxKeysCount && d->mKeyboardSize == HbQwertyKeyboardPrivate::HbQwerty4x10)) {
        if (!d->mKeypadCreated) {
            d->constructKeypad();
            d->initializeKeyboard(false);
        } else {
            d->initializeKeyboard(true);
        }
        d->applyEditorConstraints();
        d->setRockerPosition();
        return;
    }
    if (d->mMode == EModeNumeric) {
        // Numeric keyboard does not change mode
        return;
    }
    d->updateButtonsTextAndMappers();
    d->applyEditorConstraints();
}

/*!
\reimp
\deprecated HbQwertyKeyboard::setKeymap(const HbKeymap*)
    is deprecated.
*/
void HbQwertyKeyboard::setKeymap(const HbKeymap* keymap)
{
    Q_D(HbQwertyKeyboard);
    if (keymap) {
        d->mKeymap = keymap;
        d->mKeymapChanged = true;
        // let's change the button text depending on the new keymapping.
        HbInputState newState = d->mOwner->inputState();
        if (newState.textCase() == HbTextCaseUpper || newState.textCase() == HbTextCaseAutomatic) {
            setMode(d->mMode, HbModifierShiftPressed);
        } else {
            setMode(d->mMode, HbModifierNone);
        }
        d->mKeymapChanged = false;
    }
}

/*!
\reimp
\deprecated HbQwertyKeyboard::aboutToOpen(HbVkbHost*)
*/
void HbQwertyKeyboard::aboutToOpen(HbVkbHost *host)
{
    Q_D(HbQwertyKeyboard);

    HbInputVkbWidget::aboutToOpen(host);

    d->setLayoutDimensions(d->calculateDimensions(keypadButtonAreaSize()));
}

/*!
\reimp
\deprecated HbQwertyKeyboard::preferredKeyboardSize()
*/
QSizeF HbQwertyKeyboard::preferredKeyboardSize()
{
    Q_D(HbQwertyKeyboard);

    QSizeF result = HbInputVkbWidget::preferredKeyboardSize();

    if (d->mMode == EModeNumeric) {
        //We need to subtract the height of the close handle from prefered size of keypad
        //before calculating the height of each row.
        qreal height = (result.height() - HbCloseHandleHeight) / (qreal)HbVirtualQwertyNumberOfRows;
        result.setHeight(HbVirtualQwertyNumberOfRowsNumberMode * height + HbCloseHandleHeight);
    }

    return QSizeF(result);
}

/*!
\deprecated HbQwertyKeyboard::previewCharacters(const QStringList&)
    is deprecated.
*/
bool HbQwertyKeyboard::previewCharacters(const QStringList& characters)
{
    Q_D(HbQwertyKeyboard);

    // Don't do anything if the current button index is not in the range.
    if (d->mPressedButtonIndex < 0 || d->mPressedButtonIndex > d->mButtons.size()) {
        return false;
    }

    // let's set mLongKeyPressCharsShown. Since if the long press preview pane is On
    // we should not close the preview pane when the button is released.
    if (!d->mInStickyRegion) {
        if (characters.count()) {
            // we should not show the long press preview pane if the character size is 1 and matches with
            // the the button text.
            if (characters.count() == 1
                && (d->mButtons.at(d->mPressedButtonIndex)->text().compare(characters.at(0)) == 0)) {
                    d->mLongKeyPressCharsShown = false;
            } else {
                d->launchPreviewPane(characters);
                d->mLongKeyPressCharsShown = true;
            }
        } else {
            d->mLongKeyPressCharsShown = false;
        }
    } else {
        // This situation is not likely to happen as we might have closed
        // the preview pane in enteredInNonStickyRegion function.
        if (d->mPreviewPane->isVisible()) {
            d->mPreviewPane->hide();
        }
        d->mLongKeyPressCharsShown = false;
    }

    // let's inform the caller that there preview is not possible with the
    // character set sent.
    return d->mLongKeyPressCharsShown;
}

/*!
\reimp
\deprecated HbQwertyKeyboard::aboutToClose(HbVkbHost*)
*/
void HbQwertyKeyboard::aboutToClose(HbVkbHost *host)
{
    Q_UNUSED(host);
    Q_D(HbQwertyKeyboard);
    // Let's hide the preview pane.
    if (d->mPreviewPane->isVisible()) {
        d->mPreviewPane->hide();
    }
    // reset the states as the keypad is closing
    d->mLongKeyPressCharsShown = false;
    d->mInStickyRegion = true;

    HbInputVkbWidget::aboutToClose(host);
}

/*!
\deprecated HbQwertyKeyboard::initSctModeList()
    is deprecated. Sct mode list is not supported anymore.
*/
void HbQwertyKeyboard::initSctModeList()
{
}

/*!
\deprecated HbQwertyKeyboard::sctModeListClosed()
    is deprecated. Sct mode list is not supported anymore.
*/
void HbQwertyKeyboard::sctModeListClosed()
{
}

/*!
\deprecated HbQwertyKeyboard::enteredInNonStickyRegion()
    is deprecated.
*/
void HbQwertyKeyboard::enteredInNonStickyRegion()
{
    Q_D(HbQwertyKeyboard);
    if (d->mPreviewPane->isVisible()) {
        d->mPreviewPane->hide();
    }
    d->mInStickyRegion = true;
    d->mLongKeyPressCharsShown = false;
}

/*!
\deprecated HbQwertyKeyboard::mappedKeyClick(int)
  is deprecated and will be removed.
*/
void HbQwertyKeyboard::mappedKeyClick(int buttonid)
{
    Q_UNUSED(buttonid);
}
// End of file
