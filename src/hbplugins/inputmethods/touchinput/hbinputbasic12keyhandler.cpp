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

#include "virtual12key.h"
#include <QTimer>
#include <hbinputmethod.h>
#include <hbinputkeymapfactory.h>
#include <hbinputpredictionengine.h>
#include <hbinputsettingproxy.h>
#include <hbinputpredictionfactory.h>
#include <hbinputbutton.h>
#include "hbinputabstractbase.h"
#include "hbinputbasic12keyhandler.h"
#include "hbinputbasic12keyhandler_p.h"

HbInputBasic12KeyHandlerPrivate::HbInputBasic12KeyHandlerPrivate()
:    mLastKey(0),
    mCurrentChar(0),
    mNumChr(0),
    mDownKey(0),
    mCurrentlyFocused(0),
    mLongPressHappened(false),
    mShiftKeyDoubleTapped(false)
{
}

HbInputBasic12KeyHandlerPrivate::~HbInputBasic12KeyHandlerPrivate()
{
}

// handles the key press events.
bool HbInputBasic12KeyHandlerPrivate::handleAlphaEvent(int buttonId, HbKeyboardType type)
{
    Q_Q(HbInputBasic12KeyHandler);
    HbInputFocusObject *focusObject = mInputMethod->focusObject();
    if (!focusObject) {
        return false;
    }

    int index = mNumChr;
    do {
	    //This condition is to avoid get the characters mapped to Asterisk
	    //Especially for Thai language we have mapped character to Asterisk
        if (buttonId != HbInputButton::ButtonKeyCodeAsterisk ||
            mInputMethod->currentKeyboardType() == HbKeyboardSctPortrait) {
            mCurrentChar = q->getNthCharacterInKey(mNumChr, buttonId, type);
	    }

        if (mCurrentChar != 0) {
            if (focusObject->characterAllowedInEditor(mCurrentChar)) {
                QString str;
                str += mCurrentChar;
                
                //If the keypad is SCT, we can commit the character immidiately
                if (mInputMethod->currentKeyboardType() == HbKeyboardSctPortrait) {
                    focusObject->filterAndCommitCharacter(mCurrentChar);
                    mCurrentChar = 0;
                } else {
                    QList<QInputMethodEvent::Attribute> list;
                    QInputMethodEvent event(str, list);
                    focusObject->sendEvent(event);
                }
                return true;
            }
        } else {
            break;
        }
    } while (index != mNumChr);
    return false;
}

bool HbInputBasic12KeyHandlerPrivate::buttonPressed(const QKeyEvent *keyEvent)
{
    Q_Q(HbInputBasic12KeyHandler);
    mLongPressHappened = false;
    HbInputFocusObject *focusObject = mInputMethod->focusObject();
    if (!focusObject) {
        return false;
    }
    int buttonId = keyEvent->key();

    if (keyEvent->isAutoRepeat() && mDownKey == buttonId) {
        mTimer->stop();
        if (mDownKey == HbInputButton::ButtonKeyCodeShift) {
            mLongPressHappened = true;
            mInputMethod->switchMode(HbInputButton::ButtonKeyCodeShift);                
        } else if (mDownKey == HbInputButton::ButtonKeyCodeSymbol ||
            (mDownKey == HbInputButton::ButtonKeyCodeAsterisk &&
            mInputMethod->currentKeyboardType() != HbKeyboardSctPortrait)) {
            mLongPressHappened = true;
            mInputMethod->selectSpecialCharacterTableMode();
        } else if (mDownKey != HbInputButton::ButtonKeyCodeDelete &&
                   mInputMethod->currentKeyboardType() != HbKeyboardSctPortrait) {
            mLongPressHappened = true;
            q->commitFirstMappedNumber(mDownKey, mInputMethod->currentKeyboardType());
        }
        if (mLongPressHappened) {
            mDownKey = 0;
            return true;
        }
    }

    // mark a shift key double tap. This would be handled when the release event is received.
    if ((buttonId == HbInputButton::ButtonKeyCodeShift) && (mLastKey == buttonId) && mTimer->isActive()) {
        mShiftKeyDoubleTapped = true;        
    }

    if (mInputMethod) {
        if (mLastKey != buttonId || mInputMethod->currentKeyboardType() == HbKeyboardSctPortrait)
        {
            if (mCurrentChar != 0) {
                if (!focusObject->characterAllowedInEditor(mCurrentChar)) {
                    focusObject->sendCommitString(QString());            
                } else {
                    if (isEnterCharacter(mCurrentChar)) {
                        focusObject->sendPreEditString(QString("")); // Make sure the enter character is cleared.
                        mCurrentChar = QChar('\n');                  // Convert enter character to line feed.
                    }
                    QChar commitChar(mCurrentChar);
                    mCurrentChar = 0;
                    mNumChr = 0;
                    focusObject->filterAndCommitCharacter(commitChar);
                }            
            }            
        }

        mDownKey = buttonId;
        mTimer->stop();
        if (mInputMethod->currentKeyboardType() == HbKeyboardVirtual12Key) {
            mTimer->start(HbMultiTapTimerTimeout);
        }
    }
    return false;
}

/*!
Handles the key release events from the VKB. Launches the SCT with key release event of
asterisk.
*/
bool HbInputBasic12KeyHandlerPrivate::buttonReleased(const QKeyEvent *keyEvent)
{
    if (mInputMethod) {
        Q_Q(HbInputBasic12KeyHandler);
        int buttonId = keyEvent->key();
        HbInputFocusObject *focusObject = mInputMethod->focusObject();
        if (!focusObject || !mDownKey) {
            return false;
        }
        mDownKey = 0;
		if ( mLongPressHappened ){
            mLongPressHappened = false;
            return false;
        }

		if(mTimer->isActive() && mLastKey != buttonId) {
            mNumChr = 0;

            // For QLineEdit it works fine. For HbLineEdit, need to set the state 
            // to lower by calling activateState().
            // This is needed for the scenario - When automatic text case is true
            // click a button and before the multitap timer expires click on
            // another button.
            // Need to check for shift key : In empty editor, click on editor
            // press shift, multitap on a button. The char is entered in upper case.
            // It should be entered in lower case.
            if (mLastKey && (HbInputButton::ButtonKeyCodeShift != mLastKey)) {
                mInputMethod->updateState();
            }
            refreshAutoCompleter();
        }

        if (buttonId == HbInputButton::ButtonKeyCodeEnter) {
            mInputMethod->closeKeypad();
            mLastKey = buttonId;
            return true;
        } else if (buttonId == HbInputButton::ButtonKeyCodeShift) {
            // single tap of shift key toggles prediction status in case insensitive languages
            // The Editor should not be Web or URL which allows only Latin Alphabet
            if (!HbInputSettingProxy::instance()->globalInputLanguage().isCaseSensitiveLanguage() &&
                                ((HbEditorConstraintLatinAlphabetOnly | HbEditorConstraintAutoCompletingField)!=focusObject->editorInterface().inputConstraints()) &&
                // when the language does not support prediction in that case we should not update the state and prediction
                HbPredictionFactory::instance()->predictionEngineForLanguage(mInputMethod->inputState().language())) {
                HbInputSettingProxy::instance()->togglePrediction();
            } else {
                // For single key press, change the text input case. If the second shift key press is 
                // received within long key press time out interval, then activate the next state
                if (mShiftKeyDoubleTapped){
                    mShiftKeyDoubleTapped = false;
                    mTimer->stop();
                    if( HbInputSettingProxy::instance()->globalInputLanguage() == mInputMethod->inputState().language() ||
                        HbInputSettingProxy::instance()->globalSecondaryInputLanguage() == mInputMethod->inputState().language() ||
                                                ((HbEditorConstraintLatinAlphabetOnly | HbEditorConstraintAutoCompletingField)==focusObject->editorInterface().inputConstraints())){
                        // in latin variants , double tap of shift key toggles the prediction status	
                        // revert back to the old case as this is a double tap 
                        // (the case was changed on the single tap)
                        updateTextCase();
                        // when the language does not support prediction in that case we should not update the state and prediction
                        if(HbPredictionFactory::instance()->predictionEngineForLanguage(mInputMethod->inputState().language()) &&
                            mInputMethod->focusObject()->editorInterface().isPredictionAllowed()) {
                            q->togglePrediction();
                        }
                    } else {
                        // if the global language is different from the input mode language, we should 
                        // go back to the root state
                        // e.g. double tap of hash/shift key is used to change 
                        // to chinese input mode from latin input mode
                        HbInputState rootState;
                        mInputMethod->editorRootState(rootState);
                        mInputMethod->activateState(rootState);         
                    }
                } else {
                    updateTextCase();
                    mTimer->start(HbMultiTapTimerTimeout);
                }
            }    
            mLastKey = buttonId;
            mCurrentChar = 0;
            return true;
        }

        // Let's see if we can get the handler for this button in the base class.
        if (q->HbInputBasicHandler::filterEvent(keyEvent)) {
            mCurrentChar = 0;
            return true;
        }
        mLastKey = buttonId;
        if (handleAlphaEvent(buttonId, mInputMethod->currentKeyboardType())) {
            return true;
        }

        // it was a long press on sct swith button. so just return form here.
        if (!mTimer->isActive() && buttonId == HbInputButton::ButtonKeyCodeSymbol) {
            return true;
        }
        if (buttonId == HbInputButton::ButtonKeyCodeAsterisk || buttonId == HbInputButton::ButtonKeyCodeSymbol ||
            buttonId == HbInputButton::ButtonKeyCodeAlphabet) {
            //Same asterisk key is used for launching candidate list (long key press)
            //and also for SCT. So, do not launch SCT if candidate list is already launched.            
            mInputMethod->switchMode(buttonId);
            return true;
        }
    }
    return false;
}

void HbInputBasic12KeyHandlerPrivate::_q_timeout()
{
    mTimer->stop();
    mNumChr = 0;

    HbInputFocusObject *focusedObject = mInputMethod->focusObject();
    if (!focusedObject) {
        qDebug("HbInputBasic12KeyHandler::timeout focusObject == 0");
        return;
    }

    if (isEnterCharacter(mCurrentChar)) {
        focusedObject->sendPreEditString(QString("")); // Make sure the enter character is cleared.
        mCurrentChar = QChar('\n');                    // Convert enter character to line feed.
    }

    if (!mDownKey && mLastKey != HbInputButton::ButtonKeyCodeShift) {
        if (!focusedObject->characterAllowedInEditor(mCurrentChar)) {
            focusedObject->sendCommitString(QString());
        } else {
            if ( mCurrentChar != 0){
                focusedObject->filterAndCommitCharacter(mCurrentChar);
            }
            mInputMethod->updateState();
            // pass this character to autocompleter.
            refreshAutoCompleter();
        }
    }

    mCurrentChar = 0;
    return;    
}

bool HbInputBasic12KeyHandlerPrivate::actionHandler(HbInputModeHandler::HbInputModeAction action)
{
    HbInputFocusObject *focusObject = 0;
    focusObject = mInputMethod->focusObject();
    if (!focusObject) {
        return false;
    }

    bool ret = true;
    switch (action) {
    case HbInputModeHandler::HbInputModeActionReset:
        mTimer->stop();
        if (mCurrentChar != 0) {
            focusObject->filterAndCommitCharacter(mCurrentChar);
            mCurrentChar = 0;
        }
        mLastKey = 0;
        mNumChr = 0;
        mDownKey = 0;
        mShiftKeyDoubleTapped = false;
    break;        
    case HbInputModeHandler::HbInputModeActionCancelButtonPress:
        mLastKey = 0;
        mNumChr = 0;
        mDownKey = 0;
        mShiftKeyDoubleTapped = false;
        if (mTimer->isActive()) {
            mTimer->stop();
        }
    break;
    case HbInputModeHandler::HbInputModeActionDeleteAndCommit:{
        mTimer->stop();
        mNumChr = 0;
        QString empty;
        if(mInputMethod) {
            if (mCurrentChar != 0) {
                mCurrentChar = 0;
            } else if (focusObject->editorCursorPosition()){
                QList<QInputMethodEvent::Attribute> list;
                QInputMethodEvent event(QString(), list);
                event.setCommitString(empty, -1, 1);
                focusObject->sendEvent(event);
                mInputMethod->updateState();            
            }
        } else {
            // Close event was originated from a button press, remove the uncommitted character.
            focusObject->sendCommitString(empty);
        }
    }
    break;
    case HbInputModeHandler::HbInputModeActionFocusRecieved:
        mTimer->stop();
        mNumChr = 0;        
        // set up auto completer
        setUpAutoCompleter();
    break;
    case HbInputModeHandler::HbInputModeActionCommit:
    case HbInputModeHandler::HbInputModeActionFocusLost:
        if (mTimer->isActive()) {
            mTimer->stop();
            if (mCurrentChar != 0) {
                if (isEnterCharacter(mCurrentChar)) {
                    focusObject->sendPreEditString(QString("")); // Make sure the enter character is cleared.
                    mCurrentChar = QChar('\n');                    // Convert enter character to line feed.
                }
                focusObject->filterAndCommitCharacter(mCurrentChar);
                mCurrentChar = 0;
            }
            mLastKey = 0;
            mNumChr = 0;
            mInputMethod->updateState();
        }
        // We should add the commit autocompleting text when focus lost happens
        if(HbInputModeHandler::HbInputModeActionFocusLost == action){
            if (mCurrentlyFocused != focusObject) {
                mCurrentlyFocused = focusObject;
                if (mAutoCompleter) {
                    mAutoCompleter->commit();
                }
            }
        }
        break;
    default: {
        ret = false;
    }
    break;
    };

    return ret;
}


HbInputBasic12KeyHandler::HbInputBasic12KeyHandler(HbInputAbstractMethod* inputMethod)
:HbInputBasicHandler( *new HbInputBasic12KeyHandlerPrivate, inputMethod)
{
    Q_D(HbInputBasic12KeyHandler);
    d->q_ptr = this;
}

HbInputBasic12KeyHandler::HbInputBasic12KeyHandler(HbInputBasic12KeyHandlerPrivate &dd, HbInputAbstractMethod* inputMethod)
 : HbInputBasicHandler(dd, inputMethod)
{
    Q_D(HbInputBasic12KeyHandler);
    d->q_ptr = this;
    d->init();
}

HbInputBasic12KeyHandler::~HbInputBasic12KeyHandler()
{
}

/*!
 filterEvent function for handling different keyevents.
*/
bool HbInputBasic12KeyHandler::filterEvent(const QKeyEvent * event)
{
    Q_D(HbInputBasic12KeyHandler);

    if (event->type() == QEvent::KeyRelease) {
        return d->buttonReleased(event);
    } else {
        return d->buttonPressed(event);
    }
}

/*!
 returns true if in inline edit.
*/
bool HbInputBasic12KeyHandler::isComposing() const
{
    Q_D(const HbInputBasic12KeyHandler);
    return d->mTimer->isActive();
}

/*!
Action Handler
*/
bool HbInputBasic12KeyHandler::actionHandler(HbInputModeAction action)
{
    Q_D(HbInputBasic12KeyHandler);
    if (!d->actionHandler(action)) {
        // no action taken so let's pass it to the base class.
        return HbInputBasicHandler::actionHandler(action);
    }
    return false;
}

void HbInputBasic12KeyHandler::cursorPositionChanged(int oldPos, int newPos)
{
    Q_D(HbInputBasic12KeyHandler);

    if (d->mTimer->isActive() && d->mCurrentChar != 0) {
        QString commitString(d->mCurrentChar);
        d->mCurrentChar = 0;
        commitAndUpdate(commitString, oldPos - newPos, 0);
        d->actionHandler(HbInputModeActionReset);
    }
    HbInputModeHandler::cursorPositionChanged(oldPos,newPos);
}

// EOF
