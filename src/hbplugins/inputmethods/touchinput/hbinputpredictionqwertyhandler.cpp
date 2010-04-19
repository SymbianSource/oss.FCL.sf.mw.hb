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
#include <QTimer>
#include <hbinputmethod.h>
#include <hbinputkeymapfactory.h>
#include <hbinputpredictionengine.h>

#include "hbinputpredictionqwertyhandler.h"
#include "hbinputpredictionhandler_p.h"
#include "hbinputabstractbase.h"

class HbInputPredictionQwertyHandlerPrivate: public HbInputPredictionHandlerPrivate
{
    Q_DECLARE_PUBLIC(HbInputPredictionQwertyHandler)
public:
    HbInputPredictionQwertyHandlerPrivate();
    ~HbInputPredictionQwertyHandlerPrivate();
    void deleteOneCharacter();

public:
    bool buttonPressed(const QKeyEvent *event);
    bool buttonReleased(const QKeyEvent *event);
    void init();
    void _q_timeout();

public:
    int mButton;    
    HbFnState mFnState;
    bool mExactPopupLaunched;
    bool mPreviewAvailable;
};

HbInputPredictionQwertyHandlerPrivate::HbInputPredictionQwertyHandlerPrivate()
:mButton(0),    
    mFnState(HbFnOff),
    mExactPopupLaunched(false),
    mPreviewAvailable(false)
{
}

HbInputPredictionQwertyHandlerPrivate::~HbInputPredictionQwertyHandlerPrivate()
{
}

void HbInputPredictionQwertyHandlerPrivate::init()
{
}

void HbInputPredictionQwertyHandlerPrivate::_q_timeout()
{
    qDebug("HbInputPredictionQwertyHandler::timeout called");
    mTimer->stop();
    QStringList spellList;
    
	//If long key press of shift key is received, just return
    if (mButton == Qt::Key_Shift) {
        return;
    }
    else if (mButton == Qt::Key_Control) {
        mInputMethod->selectSpecialCharacterTableMode();
    }

    //If long key press of shift key, space key and enter key is received, don't
    if (mButton) {
        mInputMethod->launchCharacterPreviewPane(mButton);
    }
}


bool HbInputPredictionQwertyHandlerPrivate::buttonReleased(const QKeyEvent *event)
{
    Q_Q(HbInputPredictionQwertyHandler);
    HbInputFocusObject *focusObject = 0;
    focusObject = mInputMethod->focusObject();
    if (!focusObject) {
        qDebug("HbVirtualQwerty::virtualButtonClicked : no focused editor widget!");
        return false;
    }

    int key = event->key();
    
    // If the timer is not active and it is alpha mode, it is a long press
    // and handled in another function. So just return.
    if (mTimer->isActive()) {
        mTimer->stop();
    } else if (key == Qt::Key_Control) {
        return true;  
    } else if (!(key & 0xffff0000) && mPreviewAvailable) {
        return false;
    }

    bool ret = true;
    switch(key) {    
    case Qt::Key_Alt:  //Fn
        if (mFnState == HbFnOff) {
                mFnState = HbFnNext;
            } else if (mFnState == HbFnNext) {
                mFnState = HbFnOn;
            } else {
                mFnState = HbFnOff;
            }
        break;
    case Qt::Key_Shift: {
			HbTextCase currentTextCase = (HbTextCase)focusObject->editorInterface().textCase();
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
        }
        break;
    case Qt::Key_Control: { // Ctrl/Chr
            mInputMethod->switchSpecialCharacterTable();
        }
        break;
    default: {
            HbTextCase currentTextCase = focusObject->editorInterface().textCase();
            Qt::KeyboardModifiers modifiers = Qt::NoModifier;
            if (mFnState == HbFnNext) {
                modifiers |= Qt::AltModifier;
                mFnState = HbFnOff;
            } else if (mFnState == HbFnOn) {
                modifiers |= Qt::AltModifier;
            }
            // If shift is pressed, the shifted characters have to be input.
            if ( HbTextCaseUpper == currentTextCase || HbTextCaseAutomatic == currentTextCase ) {
                modifiers |= Qt::ShiftModifier;
            }

            // let's pass it to the base class.
            ret = q->HbInputPredictionHandler::filterEvent(event);

            mInputMethod->updateState();
        }
        break;
    };
    return ret;
}

bool HbInputPredictionQwertyHandlerPrivate::buttonPressed(const QKeyEvent *event)
{
    mButton = event->key();
    mTimer->start(HbLongPressTimerTimeout);
    mPreviewAvailable = false;
    return false;
}


HbInputPredictionQwertyHandler::HbInputPredictionQwertyHandler(HbInputAbstractMethod *inputMethod)
    :HbInputPredictionHandler(* new HbInputPredictionQwertyHandlerPrivate, inputMethod)
{
    Q_D(HbInputPredictionQwertyHandler);
    d->q_ptr = this;
    d->init();
}

HbInputPredictionQwertyHandler::~HbInputPredictionQwertyHandler()
{
}

/*!
lists different input mode bindings..
*/
void HbInputPredictionQwertyHandler::listInputModes(QVector<HbInputModeProperties>& modes) const
{
    Q_UNUSED(modes); 
}

/*!
Action Handler.
*/
bool HbInputPredictionQwertyHandler::actionHandler(HbInputModeAction action)
{
    Q_D(HbInputPredictionQwertyHandler);
    bool ret = true;
    switch (action) {
        case HbInputModeActionCancelButtonPress:
        case HbInputModeActionReset:
            if (d->mTimer->isActive()) {
                d->mTimer->stop();
            }
            break;
        case HbInputModeActionFocusRecieved:
            HbInputPredictionHandler::actionHandler(HbInputModeActionSetCandidateList);
            HbInputPredictionHandler::actionHandler(HbInputModeActionSetKeypad);
            d->mTimer->stop();
            break;
        case HbInputModeActionFocusLost:
            HbInputPredictionHandler::actionHandler(HbInputModeActionFocusLost);

            //TODO
            /*
            if (d->mExactPopupLaunched) {
                sendCommitString(d->mCandidates->at(1));
            } else {
                sendCommitString(d->mCandidates->at(0));
            }
            */
            // close exactword popup.
            d->mInputMethod->closeExactWordPopup();
            break;
        case HbInputModeActionCommit: {   
			d->commit();        
		}
        default: ret = HbInputPredictionHandler::actionHandler(action);
    }
    return ret;
}

/*!
filterEvent for key event.
*/
bool HbInputPredictionQwertyHandler::filterEvent(const QKeyEvent * event)
{
    Q_D(HbInputPredictionQwertyHandler);

    if (event->type() == QEvent::KeyRelease) {
        return d->buttonReleased(event);
    } else {
        return d->buttonPressed(event);
    }
}

/*!
Commits the word and closes the exact popup if it is visible.
*/
void HbInputPredictionQwertyHandler::commitAndUpdate(const QString& string, int replaceFrom, int replaceLength)
{
    Q_D(HbInputPredictionQwertyHandler);
    HbInputModeHandler::commitAndUpdate(string, replaceFrom, replaceLength);
    d->mInputMethod->closeExactWordPopup();
    d->mExactPopupLaunched = false;  
	d->mTailShowing = false;
}

/*!
    this function deletes one character and updates the engine and editor.
*/
void HbInputPredictionQwertyHandler::deleteOneCharacter()
{
    Q_D(HbInputPredictionHandler);
    d->deleteOneCharacter();
}

/*!
this SLOT is called when a character on character previe pane is selected.
*/
void HbInputPredictionQwertyHandler::charFromPreviewSelected(QString character)
{
	Q_D(HbInputPredictionQwertyHandler);
    if(character.size() > 0) {
        appendUnicodeCharacter(character[0]);
		d->mInputMethod->updateState();
    }
}

/*!
 this function is called by HbPredictionHandler when HbPredictionHandler encounters a exact word.
*/
void HbInputPredictionQwertyHandler::processExactWord(QString exactWord)
{
    Q_D(HbInputPredictionQwertyHandler);
    if (exactWord.size()) {
        d->mInputMethod->launchExactWordPopup(exactWord);
        d->mExactPopupLaunched = true;
    } else {
        d->mInputMethod->closeExactWordPopup();
        d->mExactPopupLaunched = false;
    }
}

/*!
 this slot should be called when exact word popup is closed.
*/
void HbInputPredictionQwertyHandler::exactWordPopupClosed()
{
    commitExactWord();
}

void HbInputPredictionQwertyHandler::sctCharacterSelected(QString character)
{	
    HbInputPredictionHandler::sctCharacterSelected(character);
}

void HbInputPredictionQwertyHandler::smileySelected(QString smiley)
{
    HbInputPredictionHandler::smileySelected(smiley);
}
void HbInputPredictionQwertyHandler::characterPreviewAvailable(bool available)
{
    Q_D(HbInputPredictionQwertyHandler);
    d->mPreviewAvailable = available;
}

/*!
Returns true if preidciton engine is available and initialized.
*/
bool HbInputPredictionQwertyHandler::isActive() const
{ 
    Q_D(const HbInputPredictionQwertyHandler);
    return d->mEngine != 0;
}

void HbInputPredictionQwertyHandlerPrivate::deleteOneCharacter()
{
    mShowTail = true;
    mShowTooltip = true;
    // A backspace in predictive means updating the engine for the delete key press
    // and get the new candidate list from the engine.
    if ((mEngine->inputLength() >= 1) || selectWord()) {
        //Only autocomplition part should be deleted when autocompliton part is enable and user pressed a delete key
        //To prevent showing autocompletion part while deleting the characters using backspace key
        mShowTail = false;
        mShowTooltip = false;
        //The assumption here is that with deletion of a character we always
        //can go on with prediction. This is because when we delete a key press
        //we actually reduce ambiguity in the engine and hence we should have
        //some word getting predicted as a result to that.
        mCanContinuePrediction = true;
        if (true == mExactPopupLaunched) {			
			QString exactWord = mCandidates->at(0);
			mEngine->setWord(exactWord);	
			mCandidates->clear();
			mCandidates->append(exactWord);
			mBestGuessLocation = 0 ;
		} 
		if(false == mTailShowing && true == mExactPopupLaunched) {
				mEngine->deleteKeyPress();				
		}
        //When there is a deletion of key press, no need to update the candidate list
        //This is because deletion should not cause reprediction.
		if(mCandidates->count() && (mCandidates->count()>mBestGuessLocation) && false == mTailShowing && false == mExactPopupLaunched) {
		    QString currentWord = mCandidates->at(mBestGuessLocation);
			if(currentWord.length() > mEngine->inputLength()) {
				//chop off the autocompletion part
				currentWord = currentWord.left(mEngine->inputLength());
			}
		    if(currentWord.length()) {
                currentWord.chop(1);
		        mEngine->deleteKeyPress();
				//We are not supposed to re-construct the candidate list as deletion
				//does not cause reprediction. Also, candidate list construction is the
				//heaviest operation out of all engine operations.
				(*mCandidates)[mBestGuessLocation] = currentWord;
		    } else {
		        commit(QString(""),false);
		    }
		        
		} else if(!mCandidates->count()) {
            mCandidates->append(mEngine->currentWord());
        }
        // update the editor with the new preedit text.
        updateEditor();
        return;
    }

    HbInputFocusObject* focusedObject = 0;
    focusedObject = mInputMethod->focusObject();
    if (!focusedObject) {
        return;
    }

    if ((focusedObject->inputMethodQuery(Qt::ImCursorPosition).toInt() >= 0) || focusedObject->preEditString().length()) {
        QList<QInputMethodEvent::Attribute> list;
        QInputMethodEvent event(QString(), list);
        event.setCommitString(QString(), -1, 1);
        commit(event);
    }
}

//EOF
