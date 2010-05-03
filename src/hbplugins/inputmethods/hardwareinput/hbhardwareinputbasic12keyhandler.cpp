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
#include <hbinputkeymapfactory.h>
#include <hbinputpredictionengine.h>
#include <HbInputSettingProxy.h>

#include "hbhardwareinputbasic12keyhandler.h"
#include "hbinputbasichandler_p.h"
#include "hbinputabstractbase.h"
#include "hbhardware12key.h"

class HbHardwareInputBasic12KeyHandlerPrivate: public HbInputBasicHandlerPrivate
{
	Q_DECLARE_PUBLIC(HbHardwareInputBasic12KeyHandler)

public:
	HbHardwareInputBasic12KeyHandlerPrivate();
	~HbHardwareInputBasic12KeyHandlerPrivate();

	void handleAlphaEvent(int buttonId);
	bool keyPressed(const QKeyEvent *keyEvent);
	bool keyReleased(const QKeyEvent *keyEvent);
	bool actionHandler(HbInputModeHandler::HbInputModeAction action);

	void _q_timeout();
public:
	int mLastKey;
	QChar mCurrentChar;
	int mNumChr;
	bool mButtonDown;
	HbInputFocusObject *mCurrentlyFocused;
};

HbHardwareInputBasic12KeyHandlerPrivate::HbHardwareInputBasic12KeyHandlerPrivate()
:    mLastKey(0),
mCurrentChar(0),
mNumChr(0),
mButtonDown(false),
mCurrentlyFocused(0)
{
}

HbHardwareInputBasic12KeyHandlerPrivate::~HbHardwareInputBasic12KeyHandlerPrivate()
{
}

// handles the key press events. It starts a multitap timer as well.
void HbHardwareInputBasic12KeyHandlerPrivate::handleAlphaEvent(int buttonId)
{
	Q_Q(HbHardwareInputBasic12KeyHandler);
	HbInputFocusObject *focusObject = 0;
	focusObject = mInputMethod->focusObject();
	if (!focusObject) {
		return;
	}

	mCurrentChar = q->getNthCharacterInKey(mNumChr, buttonId);

	if (mCurrentChar != 0) {
		QString str;
		str += mCurrentChar;

		QList<QInputMethodEvent::Attribute> list;
		QInputMethodEvent event(str, list);
		focusObject->sendEvent(event);
		mTimer->start(HbMultiTapTimerTimeout);
	}
}

bool HbHardwareInputBasic12KeyHandlerPrivate::keyPressed(const QKeyEvent *keyEvent)
{
	Q_Q(HbHardwareInputBasic12KeyHandler);
	HbInputFocusObject *focusObject = 0;
	focusObject = mInputMethod->focusObject();
	if (!focusObject) {
		return false;
	}

	int buttonId = keyEvent->key();
	

	if( mLastKey != buttonId) {
		mLastKey = buttonId;
		if(mTimer->isActive()) {
			mTimer->stop();
			mNumChr = 0;
			if (isEnterCharacter(mCurrentChar)) {
				focusObject->sendPreEditString(QString("")); // Make sure the enter character is cleared.
				mCurrentChar = QChar('\n');                  // Convert enter character to line feed.
			}

			if(mCurrentChar != 0) {
				focusObject->filterAndCommitCharacter(mCurrentChar);
			}			
			// For QLineEdit it works fine. For HbLineEdit, need to set the state 
			// to lower by calling activateState().
			// This is needed for the scenario - When automatic text case is true
			// click a button and before the multitap timer expires click on
			// another button.
			mInputMethod->updateState();

        refreshAutoCompleter();
		}
	}

	if (buttonId == Qt::Key_Return) {
		mLastKey = buttonId;
		return true;
	} else if (buttonId == Qt::Key_Shift) {
		// For single key press, change the text input case. If the second shift key press is 
		// received within long key press time out interval, then activate the next state
		if (mTimer->isActive() && (mLastKey == buttonId)){
			mTimer->stop();
             HbInputState rootState;
             mInputMethod->editorRootState(rootState);
             mInputMethod->activateState(rootState);            
		} else {
			updateTextCase();
			mTimer->start(HbLongPressTimerTimeout);
		}
		mCurrentChar = 0;
		mButtonDown = true;
		mCurrentChar = 0;
		return true;
	}
	// Let's see if we can get the handler for this button in the base class.
	if (q->HbInputBasicHandler::filterEvent(keyEvent)) {
		return true;
	}

	mLastKey = buttonId;
	mButtonDown = true;

	handleAlphaEvent(buttonId);
	return true;
}

/*!
Handles the key release events from the VKB. Launches the SCT with key release event of
asterisk.
*/
bool HbHardwareInputBasic12KeyHandlerPrivate::keyReleased(const QKeyEvent *keyEvent)
{
	Q_Q(HbHardwareInputBasic12KeyHandler);
	mButtonDown = false;
	int buttonId = keyEvent->key();

	if (buttonId == Qt::Key_Asterisk) {
		//Same asterisk key is used for launching candidate list (long key press)
		//and also for SCT. So, do not launch SCT if candidate list is already launched.            
		mInputMethod->switchMode(buttonId);
		return true;
	} else if (buttonId == Qt::Key_Delete) {
		QKeyEvent keyEvent(QEvent::KeyRelease, Qt::Key_Backspace, Qt::NoModifier);
		q->sendAndUpdate(keyEvent);
		return true;
	}

	return false;
}


void HbHardwareInputBasic12KeyHandlerPrivate::_q_timeout()
{
	Q_Q(HbHardwareInputBasic12KeyHandler);
	mTimer->stop();
	mNumChr = 0;

	HbInputFocusObject *focusedObject = 0;
	focusedObject = mInputMethod->focusObject();
	if (!focusedObject) {
		qDebug("HbHardwareInputBasic12KeyHandler::timeout focusObject == 0");
		return;
	}

	if (isEnterCharacter(mCurrentChar)) {
		focusedObject->sendPreEditString(QString("")); // Make sure the enter character is cleared.
		mCurrentChar = QChar('\n');                    // Convert enter character to line feed.
	}

	//Long key press number key is applicable to all keys
	if (mButtonDown) {
		//switch to numeric mode for long key press of Hash key	
		if (mLastKey == Qt::Key_Shift){
			updateTextCase();
			mInputMethod->switchMode(Qt::Key_Shift);				
		}else {		
			q->commitFirstMappedNumber(mLastKey);
		}		  
	} else if(mLastKey != Qt::Key_Shift){
		if (!focusedObject->characterAllowedInEditor(mCurrentChar)) {
			focusedObject->sendCommitString(QString());
		} else {
			if ( mCurrentChar != 0){
				QString curString;
				curString.append(mCurrentChar);
				focusedObject->sendCommitString(curString);
			}		
			mInputMethod->updateState();
			// pass this character to autocompleter.
            refreshAutoCompleter();
		}
	}
	return;	
}

bool HbHardwareInputBasic12KeyHandlerPrivate::actionHandler(HbInputModeHandler::HbInputModeAction action)
{
	HbInputFocusObject *focusObject = 0;
	focusObject = mInputMethod->focusObject();
	if (!focusObject) {
		return false;
	}

	bool ret = true;
	switch (action) {
case HbInputModeHandler::HbInputModeActionReset:
	mLastKey = 0;
	mNumChr = 0;
	if (mTimer->isActive()) {
		mTimer->stop();
	}
	break;
case HbInputModeHandler::HbInputModeActionDeleteAndCommit:{
	mTimer->stop();
	QString empty;
	if(mInputMethod) {
		//In case of the sct the character is already committed
		//We need to remove the committed character.
		QList<QInputMethodEvent::Attribute> list;
		QInputMethodEvent event(QString(), list);
		event.setCommitString(empty, -1, 1);
		focusObject->sendEvent(event);
	} else {
		// Close event was originated from a button press, remove the uncommitted character.
		focusObject->sendCommitString(empty);
	}
														  }
														  break;
case HbInputModeHandler::HbInputModeActionFocusRecieved:
	mTimer->stop();
	mNumChr = 0;
	if (mCurrentlyFocused != focusObject) {
		mCurrentlyFocused = focusObject;
		if (mAutoCompleter) {
			mAutoCompleter->commit();
		}
	}
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
	break;
default: {
	ret = false;
		 }
		 break;
	};

	return ret;
}


HbHardwareInputBasic12KeyHandler::HbHardwareInputBasic12KeyHandler(HbInputAbstractMethod* inputMethod)
:HbInputBasicHandler( *new HbHardwareInputBasic12KeyHandlerPrivate, inputMethod)
{
	Q_D(HbHardwareInputBasic12KeyHandler);
	d->q_ptr = this;
}

HbHardwareInputBasic12KeyHandler::~HbHardwareInputBasic12KeyHandler()
{
}

/*!
filterEvent function for handling different keyevents.
*/
bool HbHardwareInputBasic12KeyHandler::filterEvent(const QKeyEvent * event)
{
	Q_D(HbHardwareInputBasic12KeyHandler);

	if (event->isAutoRepeat()) {
		// Ignore all repeat events after first repeat event
		return true;	
	} else if (event->type() == QEvent::KeyRelease) {
		return d->keyReleased(event);
	} else {
		return d->keyPressed(event);
	}
}

/*!
returns true if in inline edit.
*/
bool HbHardwareInputBasic12KeyHandler::isComposing() const
{
	Q_D(const HbHardwareInputBasic12KeyHandler);
	return d->mTimer->isActive();
}

/*!
Action Handler
*/
bool HbHardwareInputBasic12KeyHandler::actionHandler(HbInputModeAction action)
{
	Q_D(HbHardwareInputBasic12KeyHandler);
	if (!d->actionHandler(action)) {
		// no action taken so let's pass it to the base class.
		return HbInputBasicHandler::actionHandler(action);
	}
	return false;
}
// EOF
