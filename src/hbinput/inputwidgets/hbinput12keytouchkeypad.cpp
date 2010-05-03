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
#include <hbinpututils.h>
#include <hbframedrawer.h>
#include <hbaction.h>
#include "hbinputvkbwidget_p.h"
#include "hbinput12keytouchkeypad.h"
#include "hbinput12keytouchkeypad_p.h"
#include "hbinputtouchkeypadbutton.h"

const int HbVirtual12KeyNumberOfRows = 4;
const int HbVirtual12KeyNumberOfColumn = 4;
const int HbKey7Location = 6;
const int HbKey9Location = 8;
const qreal HbVirtual12KeyButtonPreferredHeight = 70.0;
const QSizeF HbVirtual12KeyInitialLayoutDimensions(90.0, HbVirtual12KeyButtonPreferredHeight);
const int HbButtonToKeyCodeTable[HbNum12KeypadBaseButtons] =
{
    Qt::Key_1,
    Qt::Key_2,
    Qt::Key_3,
    Qt::Key_4,
    Qt::Key_5,
    Qt::Key_6,
    Qt::Key_7,
    Qt::Key_8,
    Qt::Key_9,
    Qt::Key_Asterisk,
    Qt::Key_0,
    Qt::Key_Shift,
    Qt::Key_Delete,
    Qt::Key_Control
};

const QString HbButtonObjName = "ITU ";
const QString HbDelButtonObjName = "ITU delete";
const QString HbCustomButtonObjName = "ITU custom button ";

const QString Hb12KeyButtonTextLayout = "_hb_12key_button_text_layout";
const QString Hb12KeyButtonIconLayout = "_hb_12key_button_icon_layout";
const QString Hb12KeyButtonNumberLayout = "_hb_12key_button_number_layout";

/*!
@proto
@hbinput
\class Hb12KeyTouchKeypad
\deprecated class Hb12KeyTouchKeypad
\brief Touch keypad for 12 key ITU-T layout

Implements touch key pad for 12 key ITU-T keypad. The key pad know how to operate
in alphabet, numeric modes. it knows how to set up button titles according to
given key map data object and it also supports editor specific custom buttons.

\sa HbInputVkbWidget
\sa HbTouchKeypadButton
*/

Hb12KeyTouchKeypadPrivate::Hb12KeyTouchKeypadPrivate()
: mKeypadCreated(false),
mKeymapChanged(false)
{
}

int Hb12KeyTouchKeypadPrivate::keyCode(int buttonId)
{
    return HbButtonToKeyCodeTable[buttonId];
}

Hb12KeyTouchKeypadPrivate::~Hb12KeyTouchKeypadPrivate()
{
}

void Hb12KeyTouchKeypadPrivate::setKeyMappingTitle(int key, HbTouchKeypadButton* button, HbModifiers modifiers)
{
    QString title;

    int numberOfCharacters = 3;
    if (key == HbKey7Location || key == HbKey9Location) {
        numberOfCharacters = 4;
    }

    QString keydata = mKeymap->keyboard(HbKeyboardVirtual12Key)->keys.at(key)->characters(modifiers);

    QChar numChr = findFirstNumberCharacterBoundToKey(key);

    if(mOwner && mOwner->focusObject()) {
        // First we filter all the data that is mapped to the button, then get the firt 3/4 allowed characters and set that string
        // as additionaltext to button.
        QString allowedData;
        mOwner->focusObject()->filterStringWithEditorFilter(keydata,allowedData);
        title.append(allowedData.left(numberOfCharacters));
    } else {
        title.append(keydata.left(numberOfCharacters));
    }

    button->setVisible(true);
    button->setText(QString(numChr));
    button->setAdditionalText(title);
    button->setProperty(HbStyleRulesCacheId::hbStyleRulesForNodeCache, Hb12KeyButtonTextLayout);
}

void Hb12KeyTouchKeypadPrivate::setKeyMappingTitleNumeric(int key, HbTouchKeypadButton* button, HbModifiers modifiers)
{
    Q_UNUSED(modifiers);
    QChar numChr = findFirstNumberCharacterBoundToKey(key);

    if (numChr > 0) {
        button->setText(numChr);
    } else {
        button->setText(QString());
    }
    button->setAdditionalText(QString());
    button->setProperty(HbStyleRulesCacheId::hbStyleRulesForNodeCache, Hb12KeyButtonNumberLayout);
}

void Hb12KeyTouchKeypadPrivate::createKeypad()
{
    Q_Q(Hb12KeyTouchKeypad);
    for (int i = 0; i < HbNum12KeypadBaseButtons; i++) {
        if (i == 13) {
            HbIcon icon("qtg_mono_sym_itut");
            mButtons[i] = new HbTouchKeypadButton(q, icon, textForKey(i), q);
            mButtons[i]->setProperty(HbStyleRulesCacheId::hbStyleRulesForNodeCache, Hb12KeyButtonIconLayout);
        } else if ( i == 12) {
            HbIcon icon("qtg_mono_backspace2");
            mButtons[i] = new HbTouchKeypadButton(q, icon, textForKey(i), q);
            mButtons[i]->setAutoRepeatDelay(HbRepeatTimeout);
            mButtons[i]->setAutoRepeatInterval(HbRepeatTimeoutShort);
            mButtons[i]->setAutoRepeat(true);
            mButtons[i]->setProperty(HbStyleRulesCacheId::hbStyleRulesForNodeCache, Hb12KeyButtonIconLayout);
        } else if (i == 11) {
            HbIcon icon("qtg_mono_shift");
            mButtons[i] = new HbTouchKeypadButton(q, icon, textForKey(i), q);
        } else {
            mButtons[i] = new HbTouchKeypadButton(q, textForKey(i), q);
        }
        mButtons[i]->setAdditionalText(additionalTextForKey(i));
        QObject::connect(mButtons[i], SIGNAL(pressed()), mPressMapper, SLOT(map()));
        QObject::connect(mButtons[i], SIGNAL(released()), mReleaseMapper, SLOT(map()));
        mPressMapper->setMapping(mButtons[i], i);
        mReleaseMapper->setMapping(mButtons[i], i);
    }

    QObject::connect(mPressMapper, SIGNAL(mapped(int)), q, SLOT(mappedKeyPress(int)));
    QObject::connect(mReleaseMapper, SIGNAL(mapped(int)), q, SLOT(mappedKeyRelease(int)));

    mKeypadCreated = true;
}

QString Hb12KeyTouchKeypadPrivate::textForKey(int key)
{
    // Key 10 is 0-key on keypad, which is defined as the ninth key
    // Key nine is the star key, which has "+" mapped to it
    if (key == 10) {
        key = 9;
    } else if (key == 9) {
        return QString("+");
    } else if(key ==11) {
        return QString("#");
    }
    if (key >= mKeymap->keyboard(HbKeyboardVirtual12Key)->keys.count()) {
        return QString();
    }
    QChar numChr = findFirstNumberCharacterBoundToKey(key);
    if (!numChr.isNull()) {
        return QString(numChr);
    } else {
        return QString();
    }
}

QString Hb12KeyTouchKeypadPrivate::additionalTextForKey(int key)
{
    // Key 10 is 0-key on keypad, which is defined as the ninth key
    // Key nine is the star key, "*" mapped to it
    if (key == 10) {
        key = 9;
    } else if (key == 9) {
        return QString("*");
    }  else if (key == 11) {
        return QString();
    }

    if (key >= mKeymap->keyboard(HbKeyboardVirtual12Key)->keys.count()) {
        return QString();
    }

    if (mMode == EModeNumeric) {
        return QString();
    } else {
        QString title;

        int numberOfCharacters = 3;
        if (key == 6 || key == 8) {
            numberOfCharacters = 4;
        }

        QString keydata = mKeymap->keyboard(HbKeyboardVirtual12Key)->keys.at(key)->characters(mModifiers);

        title.append(keydata.left(numberOfCharacters));

        return title;
    }
}

int Hb12KeyTouchKeypadPrivate::keyCode(HbTouchKeypadButton *button)
{
    int keycode = -1;
    for (int i = 0; i < HbNum12KeypadBaseButtons; i++) {
        if(button->text() == textForKey(i)) {
            keycode = i+1;
            break;
        }
    }
    return keycode;
}

void Hb12KeyTouchKeypadPrivate::createLayout()
{
    Q_Q(Hb12KeyTouchKeypad);

    // The layout is already created. So just return.
    if ( mButtonLayout ) {
        return;
    }

    mButtonLayout = new QGraphicsGridLayout();
    q->setContentsMargins(0.0, 0.0, 0.0, 0.0);
    mButtonLayout->setContentsMargins(0.0, 0.0, 0.0, 0.0);
    mButtonLayout->setHorizontalSpacing(HorizontalSpacing);
    mButtonLayout->setVerticalSpacing(VerticalSpacing);

    mButtonLayout->addItem(mButtons[0], 0, 0);    // key 1
    mButtonLayout->addItem(mButtons[1], 0, 1);    // key 2
    mButtonLayout->addItem(mButtons[2], 0, 2);    // key 3
    mButtonLayout->addItem(mButtons[12], 0, 3);   // key delete
    mButtonLayout->addItem(mButtons[3], 1, 0);    // key 4
    mButtonLayout->addItem(mButtons[4], 1, 1);    // key 5
    mButtonLayout->addItem(mButtons[5], 1, 2);    // key 6
    mButtonLayout->addItem(mButtons[6], 2, 0);    // key 7
    mButtonLayout->addItem(mButtons[7], 2, 1);    // key 8
    mButtonLayout->addItem(mButtons[8], 2, 2);    // key 9
    mButtonLayout->addItem(mButtons[9], 3, 0);    // key sym
    mButtonLayout->addItem(mButtons[10], 3, 1);   // key 0
    mButtonLayout->addItem(mButtons[11], 3, 2);   // key #
    mButtonLayout->addItem(mButtons[13], 1, 3);   // key sym (second)
    mButtonLayout->addItem(mSettingsButton, 2, 3);   // Settings key
    mButtonLayout->addItem(mApplicationButton, 3, 3);   // Application specific key

    mButtons[0]->setObjectName(HbButtonObjName + "1,1");
    mButtons[1]->setObjectName(HbButtonObjName + "1,2");
    mButtons[2]->setObjectName(HbButtonObjName + "1,3");
    mButtons[3]->setObjectName(HbButtonObjName + "2,1");
    mButtons[4]->setObjectName(HbButtonObjName + "2,2");
    mButtons[5]->setObjectName(HbButtonObjName + "2,3");
    mButtons[6]->setObjectName(HbButtonObjName + "3,1");
    mButtons[7]->setObjectName(HbButtonObjName + "3,2");
    mButtons[8]->setObjectName(HbButtonObjName + "3,3");
    mButtons[9]->setObjectName(HbButtonObjName + "4,1");
    mButtons[10]->setObjectName(HbButtonObjName + "4,2");
    mButtons[11]->setObjectName(HbButtonObjName + "4,3");
    mButtons[12]->setObjectName(HbDelButtonObjName);
    mButtons[13]->setObjectName(HbCustomButtonObjName + QString::number(1));
    if (mSettingsButton) {
        mSettingsButton->setObjectName(HbCustomButtonObjName + QString::number(2));
    }
    if (mApplicationButton) {
        mApplicationButton->setObjectName(HbCustomButtonObjName + QString::number(3));
    }

    mButtons[12]->setButtonType(HbTouchKeypadButton::HbTouchButtonFunction);
    mButtons[11]->setButtonType(HbTouchKeypadButton::HbTouchButtonFunction);
    mButtons[9]->setButtonType(HbTouchKeypadButton::HbTouchButtonFunction);
    mButtons[13]->setButtonType(HbTouchKeypadButton::HbTouchButtonFunction);
    mButtons[12]->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
    mButtons[11]->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
    mButtons[9]->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
    mButtons[13]->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
}

/*!
Apply editor constraints to the vkb
*/
void Hb12KeyTouchKeypadPrivate::applyEditorConstraints()
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

    for (int i = 0; i < HbNum12KeypadBaseButtons ; i++) {
        if(Hb::ItemType_InputCharacterButton == mButtons[i]->type()) {
            bool disableButton = false;
            if(EModeNumeric == mMode){
                QString data  = mButtons[i]->text();
                if (data.isEmpty() || !focusedObject->characterAllowedInEditor(data[0])) {
                    disableButton = true;
                }
            } else if(EModeAbc == mMode) {
                if((mButtons[i]->additionalText()).isEmpty() ) {
                    disableButton = true;
                }
            }
            mButtons[i]->setFade(disableButton);
        }
    }

	QString allowedSctCharacters;
	getAllowedSctCharcters(allowedSctCharacters);
	if (allowedSctCharacters.isNull() && (Qt::ImhDigitsOnly & focusedObject->inputMethodHints())) {
		mButtons[9]->setFade(true);
		mButtons[11]->setFade(true);
		mButtons[13]->setFade(true);
	}
	else if (Qt::ImhDialableCharactersOnly & focusedObject->inputMethodHints()) {
		mButtons[9]->setFade(false);
		mButtons[11]->setFade(false);
		mButtons[13]->setFade(true);
	}
	else {
		mButtons[9]->setFade(false);
		mButtons[11]->setFade(false);
		mButtons[13]->setFade(false);
	}
}
/*! returns first number character mapped bound to the key
*/

QChar Hb12KeyTouchKeypadPrivate::findFirstNumberCharacterBoundToKey(int key)
{
    QChar numChr = 0;
    if (!mKeymap) {
        return numChr;
    }
	
    HbInputLanguage language = mKeymap->language();
	HbInputFocusObject *focusObject = 0;

	if (mOwner) {
        focusObject = mOwner->focusObject();
	}
	bool isNumericEditor = false;

	if (focusObject) {
        isNumericEditor = focusObject->editorInterface().isNumericEditor();
	}
	
    HbInputDigitType digitType = HbInputUtils::inputDigitType(language);

    if (language.language()  != (QLocale::Language)0) {
        if (isNumericEditor) {
            QLocale::Language systemLanguage = QLocale::system().language();
            // show native digits only when the device language and writing language are same, 
            // else show latin digits
            if (language.language() != systemLanguage) {
                digitType = HbDigitTypeLatin;
            }	
        }	
        numChr = HbInputUtils::findFirstNumberCharacterBoundToKey(mKeymap->keyboard(HbKeyboardVirtual12Key)->keys.at(key),
            language, digitType);
    }
    return numChr;
}

/*!
Get the allowed sct Characters
*/
void Hb12KeyTouchKeypadPrivate::getAllowedSctCharcters(QString& allowedSctCharacters)
{
	QString sctCharacters;
	if (mKeymap) {
		const HbKeyboardMap* keymap = mKeymap->keyboard(HbKeyboardSctPortrait);
		if (keymap == 0) {
			return;
		}
		foreach (const HbMappedKey* mappedKey, keymap->keys) {
			sctCharacters.append(mappedKey->characters(HbModifierNone));
		}
	}
	HbInputFocusObject* focusObject = mOwner->focusObject();
	QString tempAllowedSctCharacters;
	if (focusObject) {
		focusObject->filterStringWithEditorFilter(sctCharacters,tempAllowedSctCharacters);
	}
	allowedSctCharacters.clear();
	for(int i=0; i<tempAllowedSctCharacters.length() ;i++) {
		// dont add duplicates to the list
		if(!allowedSctCharacters.contains(tempAllowedSctCharacters[i])) {
			allowedSctCharacters.append(tempAllowedSctCharacters[i]);
		}
	}
}

/*!
\deprecated Hb12KeyTouchKeypad::Hb12KeyTouchKeypad(HbInputMethod*, QGraphicsItem*)
     is deprecated.
Constructs the object.
*/
Hb12KeyTouchKeypad::Hb12KeyTouchKeypad(HbInputMethod* aOwner,
                                       QGraphicsItem* aParent)
                                       : HbInputVkbWidget(*new Hb12KeyTouchKeypadPrivate, aParent)
{
    if (0 == aOwner) {
        return;
    }
    Q_D(Hb12KeyTouchKeypad);
    d->q_ptr = this;
    d->mOwner = aOwner;
}

/*!
\deprecated Hb12KeyTouchKeypad::keyboardType() const
    is deprecated.
Returns keyboard type.
*/
HbKeyboardType Hb12KeyTouchKeypad::keyboardType() const
{
    return HbKeyboardVirtual12Key;
}

/*!
\deprecated Hb12KeyTouchKeypad::~Hb12KeyTouchKeypad()
    is deprecated.
Destructs the object.
*/
Hb12KeyTouchKeypad::~Hb12KeyTouchKeypad()
{
}

/*!
\deprecated Hb12KeyTouchKeypad::mappedKeyPress(int)
    is deprecated.
Handles virtual key press
*/
void Hb12KeyTouchKeypad::mappedKeyPress(int buttonid)
{
    Q_D(Hb12KeyTouchKeypad);
	if(buttonid >= 0 && d->mButtons[buttonid] && !d->mButtons[buttonid]->isFaded()) {
        HbInputVkbWidget::mappedKeyPress(buttonid);
    }
}

/*!
\deprecated Hb12KeyTouchKeypad::mappedKeyRelease(int)
    is deprecated.
Handles virtual key release
*/
void Hb12KeyTouchKeypad::mappedKeyRelease(int buttonid)
{
    Q_D(Hb12KeyTouchKeypad);
    if(buttonid >= 0 && d->mButtons[buttonid] && !d->mButtons[buttonid]->isFaded()) {
        HbInputVkbWidget::mappedKeyRelease(buttonid);
    }
}
/*!
\deprecated Hb12KeyTouchKeypad::setMode(HbKeypadMode, QFlags<HbModifier>)
    is deprecated.
Sets the keypad to given mode. Possible values are EModeAbc, EModeNumeric and EModeSct.
*/
void Hb12KeyTouchKeypad::setMode(HbKeypadMode mode, HbModifiers modifiers)
{
    Q_D(Hb12KeyTouchKeypad);
    d->mModifiers = modifiers;
    d->mMode = mode;

    if (!d->mKeypadCreated) {
        d->createKeypad();
        setupToolCluster();
        d->createLayout();
        d->applyEditorConstraints();
        return;
    }
    setupToolCluster();
    if (mode == EModeNumeric) {
        d->setKeyMappingTitleNumeric(0, d->mButtons[0], 0);
        d->setKeyMappingTitleNumeric(1, d->mButtons[1], 0);
        d->setKeyMappingTitleNumeric(2, d->mButtons[2], 0);
        d->setKeyMappingTitleNumeric(3, d->mButtons[3], 0);
        d->setKeyMappingTitleNumeric(4, d->mButtons[4], 0);
        d->setKeyMappingTitleNumeric(5, d->mButtons[5], 0);
        d->setKeyMappingTitleNumeric(6, d->mButtons[6], 0);
        d->setKeyMappingTitleNumeric(7, d->mButtons[7], 0);
        d->setKeyMappingTitleNumeric(8, d->mButtons[8], 0);
        d->setKeyMappingTitleNumeric(9, d->mButtons[10], 0);
    } else {
        if (d->mKeymap) {
            d->setKeyMappingTitle(0, d->mButtons[0], d->mModifiers);
            d->setKeyMappingTitle(1, d->mButtons[1], d->mModifiers);
            d->setKeyMappingTitle(2, d->mButtons[2], d->mModifiers);
            d->setKeyMappingTitle(3, d->mButtons[3], d->mModifiers);
            d->setKeyMappingTitle(4, d->mButtons[4], d->mModifiers);
            d->setKeyMappingTitle(5, d->mButtons[5], d->mModifiers);
            d->setKeyMappingTitle(6, d->mButtons[6], d->mModifiers);
            d->setKeyMappingTitle(7, d->mButtons[7], d->mModifiers);
            d->setKeyMappingTitle(8, d->mButtons[8], d->mModifiers);
            d->setKeyMappingTitle(9, d->mButtons[10], d->mModifiers);
        } else {
            // Default fallback.
            d->mButtons[0]->setText(QString(".,!"));
            d->mButtons[1]->setText(QString("abc"));
            d->mButtons[2]->setText(QString("def"));
            d->mButtons[3]->setText(QString("ghi"));
            d->mButtons[4]->setText(QString("jkl"));
            d->mButtons[5]->setText(QString("mno"));
            d->mButtons[6]->setText(QString("pqrs"));
            d->mButtons[7]->setText(QString("tuv"));
            d->mButtons[8]->setText(QString("wxyz"));
            d->mButtons[10]->setText(QString("0_"));
        }
    }

    d->applyEditorConstraints();
}

/*!
\reimp
\deprecated Hb12KeyTouchKeypad::setKeymap(const HbKeymap*)
    is deprecated.
*/
void Hb12KeyTouchKeypad::setKeymap(const HbKeymap* keymap)
{
    Q_D(Hb12KeyTouchKeypad);
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
\deprecated Hb12KeyTouchKeypad::aboutToOpen(HbVkbHost*)
    is deprecated.
*/
void Hb12KeyTouchKeypad::aboutToOpen(HbVkbHost *host)
{
    Q_D(Hb12KeyTouchKeypad);

    HbInputVkbWidget::aboutToOpen(host);

    QSizeF keypadSize = keypadButtonAreaSize();

    keypadSize.setWidth(keypadSize.width() / (qreal)HbVirtual12KeyNumberOfColumn);
    keypadSize.setHeight(keypadSize.height() / (qreal)HbVirtual12KeyNumberOfRows);

    for (int i=0; i < 4 ;i++) {
        d->mButtonLayout->setColumnFixedWidth(i, keypadSize.width());
        d->mButtonLayout->setRowFixedHeight(i, keypadSize.height());
    }

    for (int i = 0; i < HbNum12KeypadBaseButtons; ++i) {
        d->mButtons[i]->setInitialSize(keypadSize);
    }
    if (d->mSettingsButton) {
        d->mSettingsButton->setInitialSize(keypadSize);
    }
    if (d->mApplicationButton) {
        d->mApplicationButton->setInitialSize(keypadSize);
    }

}

/*!
\deprecated Hb12KeyTouchKeypad::initSctModeList()
    is deprecated. Sct mode list is not supported anymore.
*/
void Hb12KeyTouchKeypad::initSctModeList()
{
}

/*!
\deprecated Hb12KeyTouchKeypad::sctModeListClosed()
    is deprecated. Sct mode list is not supported anymore.
*/
void Hb12KeyTouchKeypad::sctModeListClosed()
{
}

// End of file
