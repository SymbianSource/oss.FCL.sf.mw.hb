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
#include <QTextCharFormat>
#include <QTimer>
#include <hbinputsettingproxy.h>
#include <hbinputkeymapfactory.h>
#include <hbinputkeymap.h>
#include <hbinpututils.h>

#include "hbinputmodehandler.h"
#include "hbinputmodehandler_p.h"
#include "hbinputabstractbase.h"

HbInputModeHandlerPrivate::HbInputModeHandlerPrivate()
:mKeymap(0),
mInputMethod(0),
mTimer(0)
{
}

HbInputModeHandlerPrivate::~HbInputModeHandlerPrivate()
{
}

void HbInputModeHandlerPrivate::init()
{
    Q_Q(HbInputModeHandler);
    mTimer = new QTimer(q);
    q->connect(mTimer, SIGNAL(timeout()), q, SLOT(_q_timeout()));
}

// A virtual timeout function mode handlers should implement this slot.
void HbInputModeHandlerPrivate::_q_timeout()
{
}

QString HbInputModeHandlerPrivate::surroundingText()
{
    if (mInputMethod->focusObject()) {
        return mInputMethod->focusObject()->editorSurroundingText();
    }
    return QString();
}

int HbInputModeHandlerPrivate::cursorPosition()
{
    if (mInputMethod->focusObject()) {
       return mInputMethod->focusObject()->editorCursorPosition();
    }

    return -1;
}

void HbInputModeHandlerPrivate::getAndFilterCharactersBoundToKey(QStringList &spellList, Qt::Key key)
{
    HbInputFocusObject *focusObject = mInputMethod->focusObject();

    spellList.clear();
    // Get the functionized character
    const HbMappedKey* mappedKey = mKeymap->keyForKeycode(mInputMethod->inputState().keyboard(), key);
    if (!mappedKey) {
        return;
    }

    if (!mappedKey->characters(HbModifierFnPressed).isNull() && focusObject && focusObject->characterAllowedInEditor(mappedKey->characters(HbModifierFnPressed).at(0))) {
        spellList.append(mappedKey->characters(HbModifierFnPressed).at(0));
    }

    // Get the characters mapped to the key.
    HbInputState inputState = mInputMethod->inputState();
    HbTextCase textCase = inputState.textCase();
    HbModifiers modifiers = HbModifierNone;
	
    if (textCase == HbTextCaseUpper || textCase == HbTextCaseAutomatic) {
        modifiers |= HbModifierShiftPressed;
    }
    for (int i=0; i < mappedKey->characters(modifiers).length(); i++) {
        if (focusObject && focusObject->characterAllowedInEditor(mappedKey->characters(modifiers).at(i))) {
            spellList.append(mappedKey->characters(modifiers).at(i));
        }
    }
}


void HbInputModeHandlerPrivate::updateTextCase()
{
    HbInputState nextState = mInputMethod->inputState();
    HbTextCase nextCase = HbTextCaseNone;

    switch(nextState.textCase()) {
        case HbTextCaseLower:
            // In multiline editor, if enter char is input and then shift is pressed twice,
            // the case should be reverted to automatic.
            if (mInputMethod->automaticTextCaseNeeded()) {
                nextCase = HbTextCaseAutomatic;
            } else {
                nextCase = HbTextCaseUpper;
            }
            break;
        case HbTextCaseUpper:
        case HbTextCaseAutomatic:
            nextCase = HbTextCaseLower;
            break;
        default:
            break;
        }
        nextState.setTextCase(nextCase);
        mInputMethod->activateState(nextState);
}

bool HbInputModeHandlerPrivate::isEnterCharacter(QChar character)
{
    if (character == 0x21B2 || character == 0x21b3) {
        return true;
    }
    return false;
}
HbInputModeHandler::HbInputModeHandler(HbInputModeHandlerPrivate &dd, HbInputAbstractMethod* inputMethod)
    :d_ptr(&dd)
{
    Q_D(HbInputModeHandler);
    d->q_ptr = this;
    d->init();
    d->mInputMethod = inputMethod;
}

HbInputModeHandler::~HbInputModeHandler()
{
    delete d_ptr;
}

/*!
 Gateway for all the mode handlers.
*/
bool HbInputModeHandler::filterEvent(const QEvent * event)
{
    if (event) {
        if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
            const QKeyEvent *keyEvent = static_cast<const QKeyEvent *>(event);
            return filterEvent(keyEvent);
		}
	}
    return false;
}

/*!
 This function handles the most common key events.
*/
bool HbInputModeHandler::filterEvent(const QKeyEvent * event)
{
    Q_UNUSED(event);
    return false;
}

/*!
 Action hanlder. Currently it handles only keymapping loading.
*/
bool HbInputModeHandler::actionHandler(HbInputModeAction action)
{
    Q_D(HbInputModeHandler);
    switch (action) {
    case HbInputModeActionInit:
        if (d->mKeymap) {
            break;
        }
    case HbInputModeActionPrimaryLanguageChanged: {
        HbInputLanguage primaryLanguage = HbInputSettingProxy::instance()->globalInputLanguage();
        d->mKeymap = HbKeymapFactory::instance()->keymap(primaryLanguage);
        if (!d->mKeymap) {
            qDebug(" HbInputModeHandler::actionHandler --> Failed to get keymapData!!!");
            return false;
        }
        }
        break;
    case HbInputModeActionDeleteAndCommit: {
        HbInputFocusObject *focusObject = 0;
        focusObject = d->mInputMethod->focusObject();

        if (focusObject && focusObject->editorCursorPosition()) {
            QString empty;
            QList<QInputMethodEvent::Attribute> list;
            QInputMethodEvent event(QString(), list);
            event.setCommitString(empty, -1, 1);
            sendAndUpdate(event);
        }
        break;
        }
    default :
        return false;
    };

    return true;
}

/*!
Call-back implementation to indicate that a character was selected from the SCT. With this, the character is committed to the
editor and editor is again made to focus.
*/
void HbInputModeHandler::sctCharacterSelected(QString aChar)
{
    commitAndUpdate(aChar);
}

void HbInputModeHandler::smileySelected(QString smiley)
{
    Q_D(HbInputModeHandler);
    HbInputFocusObject *focusObject = 0;
    focusObject = d->mInputMethod->focusObject();
    if (!focusObject) {
        qDebug("HbInputModeHandler::smileySelected no focusObject ... failed!!");
        return ;
    }
    QStringList patterns = focusObject->editorInterface().smileyTheme().patterns(smiley);
    foreach( const QString string, patterns) {
        QString filtered;
        focusObject->filterStringWithEditorFilter(string, filtered);
        if (filtered == string) {
            focusObject->commitSmiley(smiley);
            break;
        }
    }
    d->mInputMethod->updateState();
}

void HbInputModeHandler::cursorPositionChanged(int oldPos, int newPos)
{
    Q_D(HbInputModeHandler);
    d->mInputMethod->updateState();
    Q_UNUSED(oldPos);
    Q_UNUSED(newPos);
}

/*!
 this function commits the first number found in the key.
*/
void HbInputModeHandler::commitFirstMappedNumber(int key, HbKeyboardType type)
{
    Q_D(HbInputModeHandler);

    HbInputLanguage language = d->mInputMethod->inputState().language();
    // This is long key press number shortcut functionality.
    if (!d->mKeymap) {
        d->mKeymap = HbKeymapFactory::instance()->keymap(language);
    }
	bool isNumericEditor = d->mInputMethod->focusObject()->editorInterface().isNumericEditor();
	HbInputDigitType digitType = HbInputUtils::inputDigitType(language);
	if (isNumericEditor) {
        QLocale::Language systemLanguage = QLocale::system().language();		 
		if (language.language() != systemLanguage) {
            digitType = HbDigitTypeLatin;
	 	}
	}	
    QChar numChr = HbInputUtils::findFirstNumberCharacterBoundToKey(
        d->mKeymap->keyForKeycode(type, key), language, digitType);
	// when a number is to be entered, it should commit 
    // the previous string and then append the number to the string
    if (numChr != 0) {
        commitAndAppendString(numChr);
    }
}

/*!
Gets the character at index in a key and incriments the index by 1. Returns the Zeroth character if
this function finds the index to be out of range and incriments index to 1.
*/
QChar HbInputModeHandler::getNthCharacterInKey(int &index, int key, HbKeyboardType type)
{
    Q_D(HbInputModeHandler);
    HbModifiers modifiers = 0;
    int textCase = d->mInputMethod->inputState().textCase();
    if (type != HbKeyboardSctPortrait && (textCase == HbTextCaseUpper || textCase == HbTextCaseAutomatic)) {
        modifiers |= HbModifierShiftPressed;
    }
    HbInputLanguage language = d->mInputMethod->inputState().language();
	
    if (!d->mKeymap) {
        d->mKeymap = HbKeymapFactory::instance()->keymap(language);
    }
    const HbMappedKey* mappedKey = d->mKeymap->keyForKeycode(type, key);
    if (!mappedKey) {
        return 0;
    }
    QString chars = mappedKey->characters(modifiers);
	// check whether current input language supports native digits. if yes, replace latin digits with native digits    
    for (int i = 0; i < chars.length(); i++) {
        if (chars.at(i) >= '0' && chars.at(i) <= '9') {
            chars = chars.replace(chars.at(i), HbInputUtils::findFirstNumberCharacterBoundToKey(mappedKey,
				language, HbInputUtils::inputDigitType(language)));
        }		
    }		
    // We need to see which of the characters in keyData are allowed to the editor.
    // this looks like expensive operation, need to find out a better way/place to do it.
    QString allowedChars = chars;
    HbInputFocusObject *focusedObject = d->mInputMethod->focusObject();
    if(focusedObject) {
        focusedObject->filterStringWithEditorFilter(chars,allowedChars);
    } 
    QChar character = 0;
    if (!allowedChars.isNull()) {
        if (index >= allowedChars.length() || index < 0) {
            index = 0;
        }
        character = allowedChars.at(index);
        index++;
    }
    return character;
}

/*!
    This function gets all the characters bound to a key and filters those character based on the editor.
*/
void HbInputModeHandler::getAndFilterCharactersBoundToKey(QStringList &spellList, Qt::Key key)
{
    Q_D(HbInputModeHandler);
    d->getAndFilterCharactersBoundToKey(spellList, key);
}

/*!
    A virtual function, this should be called from plugin when ever there is mouse event.
*/
void HbInputModeHandler::mouseHandler(int x, QMouseEvent* mouseEvent)
{
    Q_UNUSED(mouseEvent);
    Q_D(HbInputModeHandler);
    if(d->cursorPosition() != x) {
        actionHandler(HbInputModeActionCommit);
    }
}

/*!
    A virtual function, default implementation first commits any inline text by calling actionHandler with
    HbInputModeActionCommit. Then commits the string.
*/
void HbInputModeHandler::commitAndAppendString(const QString& string)
{
    // first commit the pre-edit string if any.
    actionHandler(HbInputModeActionCommit);
    commitAndUpdate(string);
}

/*!
Ignores currently active pre-edit text if any and directly commits the passed string in to the editor. This
function also updates the inputmethod state.
*/
void HbInputModeHandler::commitAndUpdate(const QString& string, int replaceFrom, int replaceLength, bool isAsync)
{
    Q_D(HbInputModeHandler);
    HbInputFocusObject *focusObject = 0;
    focusObject = d->mInputMethod->focusObject();
    if (!focusObject) {
        qDebug("HbInputModeHandler::commitAndUpdate no focusObject ... failed!!");
        return ;
    }

    QString filtered;
    focusObject->filterStringWithEditorFilter(string, filtered);

    // now commit the string which is passed in.
    QList<QInputMethodEvent::Attribute> list;

    if(isAsync) {
        QInputMethodEvent *event = new QInputMethodEvent(QString(), list);
        event->setCommitString(filtered, replaceFrom, replaceLength);
        focusObject->postEvent(*event);
    } else {
        QInputMethodEvent event;
        event.setCommitString(filtered, replaceFrom, replaceLength);
        focusObject->sendEvent(event);
    }
    d->mInputMethod->updateState();
}

/*!
Sends the passed event to the focused object and upates the inputmethod state.
*/
void HbInputModeHandler::sendAndUpdate(QEvent& event)
{
    Q_D(HbInputModeHandler);
    HbInputFocusObject *focusObject = 0;
    focusObject = d->mInputMethod->focusObject();
    if (!focusObject) {
        qDebug("HbInputModeHandler::sendAndUpdate no focusObject ... failed!!");
        return;
    }

    focusObject->sendEvent(event);
    d->mInputMethod->updateState();
}

void HbInputModeHandler::setKeymap(const HbKeymap* keymap)
{
    Q_D(HbInputModeHandler);
    d->mKeymap = keymap;
}

/*!
Toggles prediction after doing a check if the editor allows it.
*/
void HbInputModeHandler::togglePrediction()
{
    Q_D(HbInputModeHandler);
    bool currentStatus = HbInputSettingProxy::instance()->predictiveInputStatusForActiveKeyboard();
    HbInputFocusObject* focusedObject = 0;
    focusedObject = d->mInputMethod->focusObject();
    bool isPredictionAllowed = focusedObject->editorInterface().isPredictionAllowed();
    if (currentStatus) {
        HbInputSettingProxy::instance()->setPredictiveInputStatusForActiveKeyboard(false);
    } else if (isPredictionAllowed) {
        HbInputSettingProxy::instance()->setPredictiveInputStatusForActiveKeyboard(true);
    }
}

#include "moc_hbinputmodehandler.cpp"
// EOF
