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

#include <hbinputpredictionengine.h>
#include <hbinputmethod.h>
#include <hbinputkeymap.h>
#include <hbinputkeymapfactory.h>
#include <hbinputbutton.h>

#include "hbinputabstractbase.h"
#include "hbinputbasicqwertyhandler.h"
#include "hbinputbasichandler_p.h"

class HbInputBasicQwertyHandlerPrivate: public HbInputBasicHandlerPrivate
{
    Q_DECLARE_PUBLIC(HbInputBasicQwertyHandler)
public:
    HbInputBasicQwertyHandlerPrivate();
    ~HbInputBasicQwertyHandlerPrivate();

    void init();

    // button related operations.
    bool buttonPressed(const QKeyEvent *event);
    bool buttonReleased(const QKeyEvent *event);

public:
    HbFnState mFnState;
    int mButton;
    bool mLongPressHappened;
    QChar mPrevDeadKey;
    HbInputFocusObject *mCurrentlyFocused;
};

HbInputBasicQwertyHandlerPrivate::HbInputBasicQwertyHandlerPrivate()
:mFnState(HbFnOff),
mButton(0),
mLongPressHappened(0),
mPrevDeadKey(0),
mCurrentlyFocused(0)
{
}

HbInputBasicQwertyHandlerPrivate::~HbInputBasicQwertyHandlerPrivate()
{
}

void HbInputBasicQwertyHandlerPrivate::init()
{
}

bool HbInputBasicQwertyHandlerPrivate::buttonPressed(const QKeyEvent * event)
{
    if (!mKeymap->isDeadKey(event->key())) {
        if (event->isAutoRepeat() && mButton == event->key()) {
            if (mButton == HbInputButton::ButtonKeyCodeSymbol) {
                mInputMethod->selectSpecialCharacterTableMode();
                mLongPressHappened = true;
            }
            if (mLongPressHappened) {
                mButton = 0;
                return true;
            }
        }
        mButton = event->key();
    } 
    return false;
}

bool HbInputBasicQwertyHandlerPrivate::buttonReleased(const QKeyEvent *event)
{
    Q_Q(HbInputBasicQwertyHandler);
    mButton = 0;

    int buttonId = event->key();
    if (!event->text().isEmpty()) {
        buttonId = event->text().at(0).unicode();
    }

    QChar firstChar = 0;
    QChar secondChar = 0;

    if (mLongPressHappened) {
        mLongPressHappened = false;
        return false;
    }

    HbInputFocusObject *focusObject = 0;
    focusObject = mInputMethod->focusObject();
    if (!focusObject) {
        qDebug("HbInputBasicQwertyHandler::virtualButtonClicked : no focused editor widget!");
        return false;
    }
    int currentTextCase = focusObject->editorInterface().textCase();
    const HbMappedKey *mappedKey = mKeymap->keyForKeycode(mInputMethod->currentKeyboardType(), buttonId);
    QChar newChar;
    if (mFnState == HbFnNext) {
        newChar = (mappedKey->characters(HbModifierFnPressed)).at(0);
        mFnState = HbFnOff;
    } else {
        if (currentTextCase == HbTextCaseLower) {
            if (mappedKey && mappedKey->characters(HbModifierNone).size() > 0) {
                newChar = mappedKey->characters(HbModifierNone).at(0);
            } else {
                newChar = buttonId;
            }
        } else {
            if (mappedKey && mappedKey->characters(HbModifierShiftPressed).size() > 0) {
                newChar = mappedKey->characters(HbModifierShiftPressed).at(0);
            } else {
                newChar = buttonId;
            }
        }
    }

    if (mInputMethod->currentKeyboardType() != HbKeyboardSctLandscape) {
        if (!mPrevDeadKey.isNull()) {
            if (buttonId != HbInputButton::ButtonKeyCodeShift && buttonId != Qt::Key_Alt) {
                mKeymap->combineCharacter(mPrevDeadKey, newChar, firstChar, secondChar );
                mPrevDeadKey = 0;
                if (firstChar.isNull() && secondChar.isNull()) {
                    return true;
                }
            }
        } else {
            if (mKeymap->isDeadKey(newChar.unicode())) {
                mPrevDeadKey = newChar.unicode();
                return true;
            } else {
                firstChar = newChar;
            }
        }
    } else {
        firstChar = newChar;
    }

    bool ret = false;
    switch(buttonId) {
    case Qt::Key_Alt:
        if (mFnState == HbFnOff) {
            mFnState = HbFnNext;
        } else if (mFnState == HbFnNext) {
            mFnState = HbFnOn;
        } else {
            mFnState = HbFnOff;
        }
        ret = true;
        break;
    case HbInputButton::ButtonKeyCodeShift: {
            HbTextCase currentTextCase = focusObject->editorInterface().textCase();
            HbInputLanguage language = mInputMethod->inputState().language();
            
            // Update the Case Information in HbInputState, it internally updates in HbEditorInterface as well
            switch(currentTextCase) {
            case HbTextCaseLower:
                // For Case-insensitive languages, Shift Key is used to switch between character sets (i.e lower case characters and shifted characters)
                if(!language.isCaseSensitiveLanguage()){
                    currentTextCase = HbTextCaseUpper; 
                }
                else {
                    currentTextCase = HbTextCaseAutomatic;
                }                                 
                break;
            case HbTextCaseUpper:
                currentTextCase = HbTextCaseLower;                
                break;
            case HbTextCaseAutomatic:
                currentTextCase = HbTextCaseUpper;               
                break;
            default:
                break;
            } 
            HbInputState state = mInputMethod->inputState();
            state.setTextCase(currentTextCase);            
            mInputMethod->activateState(state);
            ret = true;
        }
        break;
    case HbInputButton::ButtonKeyCodeSymbol:   // Ctrl/Chr
    case HbInputButton::ButtonKeyCodeAlphabet:
        mInputMethod->switchSpecialCharacterTable();
        ret = true;
        break;
    default:

        // let's pass non deadkey event to the base class.
        if (!mKeymap->isDeadKey(firstChar.unicode()) && secondChar.isNull() && q->HbInputBasicHandler::filterEvent(event)) {
            return true;
        }

        if (event->key() == HbInputButton::ButtonKeyCodeDelete || event->key() == Qt::Key_Backspace) {
            return false;
        }

        QList<QInputMethodEvent::Attribute> list;
        QString newText;
        // If function key is pressed, get the functionized 
        // character and commit.

        if (mFnState == HbFnNext) {
            if (!mappedKey->characters(HbModifierFnPressed).isEmpty() && focusObject && focusObject->characterAllowedInEditor(mappedKey->characters(HbModifierFnPressed).at(0))) {
                QInputMethodEvent event(QString(), list);
                event.setCommitString(QString(mappedKey->characters(HbModifierFnPressed).at(0)));
                focusObject->sendEvent(event);
            }
            mFnState = HbFnOff;
            return true;
        }
        
        if (!firstChar.isNull()){
            q->commitAndUpdate(firstChar); 
            if (!secondChar.isNull() ) {
                if (!q->HbInputBasicHandler::filterEvent(event)) {
                    q->commitAndUpdate(secondChar);
                }
            }

            // Need to refresh the autocompletion list
            refreshAutoCompleter();
            ret = true;
        }

        break;
    }
    return ret;
}

HbInputBasicQwertyHandler::HbInputBasicQwertyHandler(HbInputAbstractMethod* inputMethod)
:HbInputBasicHandler(* new HbInputBasicQwertyHandlerPrivate, inputMethod)
{
    Q_D(HbInputBasicQwertyHandler);
    d->q_ptr = this;
    d->init();
}


HbInputBasicQwertyHandler::~HbInputBasicQwertyHandler()
{
}

/*!
filterEvent function key handling.
*/
bool HbInputBasicQwertyHandler::filterEvent(const QKeyEvent* event)
{
    Q_D(HbInputBasicQwertyHandler);

    if (event->type() == QEvent::KeyRelease) {
        return d->buttonReleased(event);
    } else {
        return d->buttonPressed(event);
    }
}

/*!
returns true if in inline edit.
*/
bool HbInputBasicQwertyHandler::isComposing() const
{   
    return false;
}

/*!
Action handler
*/
bool HbInputBasicQwertyHandler::actionHandler(HbInputModeAction action)
{
    Q_D(HbInputBasicQwertyHandler);
    HbInputFocusObject *focusObject = 0;
    focusObject = d->mInputMethod->focusObject();
    bool ret = true;
    switch (action) {
    case HbInputModeActionCancelButtonPress:
    case HbInputModeActionReset:
        d->mButton = 0;
        break;
    case HbInputModeActionFocusRecieved:
        d->mButton = 0;
        // set up auto completer
        setUpAutoCompleter();
    break;
    case HbInputModeActionFocusLost:
        d->mButton = 0;
        if (d->mCurrentlyFocused != focusObject) {
            d->mCurrentlyFocused = focusObject;
            addWordInAutoCompleter();
        } 
    break;
    default: {
        ret = HbInputBasicHandler::actionHandler(action);
        }
    }
    return ret;
}

void HbInputBasicQwertyHandler::sctCharacterSelected(QString character)
{
    HbInputModeHandler::sctCharacterSelected(character);
}

