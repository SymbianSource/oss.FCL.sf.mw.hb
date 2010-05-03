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

#include "HbHardware12key.h"
#include "Hbhardwareinputnumeric12keyhandler.h"
#include "hbinputnumerichandler_p.h"
#include "hbinputabstractbase.h"

class HbHardwareInputNumeric12KeyHandlerPrivate: public HbInputNumericHandlerPrivate
{
	Q_DECLARE_PUBLIC(HbHardwareInputNumeric12KeyHandler)

public:
	HbHardwareInputNumeric12KeyHandlerPrivate();
	~HbHardwareInputNumeric12KeyHandlerPrivate();

	bool keyPressed(const QKeyEvent *keyEvent);
	void _q_timeout();	
public:
	int mLastKey;
	bool mButtonDown;
};

HbHardwareInputNumeric12KeyHandlerPrivate::HbHardwareInputNumeric12KeyHandlerPrivate():
mLastKey(0),
mButtonDown(false)
{
}

HbHardwareInputNumeric12KeyHandlerPrivate::~HbHardwareInputNumeric12KeyHandlerPrivate()
{
}

bool HbHardwareInputNumeric12KeyHandlerPrivate::keyPressed(const QKeyEvent *keyEvent)
{
	Q_Q(HbHardwareInputNumeric12KeyHandler);
	HbInputFocusObject *focusObject = 0;
	focusObject = mInputMethod->focusObject();
	if (!focusObject) {
		qDebug("HbInputModeHandler::buttonClicked no focusObject ... failed!!");
		return false;
	}

	int buttonId = keyEvent->key();

	if (buttonId == Qt::Key_Return) {

		return true;
	} else if (buttonId == Qt::Key_Shift) {

		if (mTimer->isActive() && (mLastKey == buttonId)){
			mTimer->stop();
			HbHardware12key *hostInputMethod = qobject_cast<HbHardware12key*>(mInputMethod);    
			if (hostInputMethod) {              
				HbInputState newState;
				mInputMethod->editorRootState(newState);           
				mInputMethod->activateState(newState);                   
			}
		} else {
			mTimer->start(HbLongPressTimerTimeout);
		}
		mLastKey = buttonId;
		mButtonDown = true;
		return true;
	} else if (buttonId == Qt::Key_Asterisk) {
		mInputMethod->switchMode(Qt::Key_Asterisk);
		return true;
	}

	// Let's see if we can get the handler for this button in the base class.
	if (q->HbInputNumericHandler::filterEvent(keyEvent)) {
		return true;
	}

	q->commitFirstMappedNumber(buttonId);
	return true;
}

void HbHardwareInputNumeric12KeyHandlerPrivate::_q_timeout()
{
	mTimer->stop();

	HbInputFocusObject *focusedObject = 0;
	focusedObject = mInputMethod->focusObject();
	if (!focusedObject) {
		qDebug("HbHardwareInputNumeric12KeyHandler::timeout focusObject == 0");
		return;
	}
	//switch to Alpha mode when Long key press of Shift key is received
	if (mButtonDown && (mLastKey == Qt::Key_Shift ))
	{
		mButtonDown = false;
		// If the editor is not a number only editor, then activate the alphanumeric keypad
		if( !focusedObject->editorInterface().isNumericEditor() ){
			mInputMethod->switchMode(mLastKey);
		}           
	}	   
	return;
}

HbHardwareInputNumeric12KeyHandler::HbHardwareInputNumeric12KeyHandler(HbInputAbstractMethod* inputMethod)
:HbInputNumericHandler( *new HbHardwareInputNumeric12KeyHandlerPrivate, inputMethod)
{
	Q_D(HbHardwareInputNumeric12KeyHandler);
	d->q_ptr = this;
}

HbHardwareInputNumeric12KeyHandler::~HbHardwareInputNumeric12KeyHandler()
{
}

/*!
filterEvent function for handling different keyevents.
*/
bool HbHardwareInputNumeric12KeyHandler::filterEvent(const QKeyEvent * event)
{
	Q_D(HbHardwareInputNumeric12KeyHandler);

	// in numeric mode in itut keypad only KeyPress events are handled.
	if ((event->type() == QEvent::KeyPress )
		|| (event->type() == QEvent::KeyRelease && event->key() == Qt::Key_Asterisk)) {
			return d->keyPressed(event);
	}
	return false;
}

/*!
Action handler
*/
bool HbHardwareInputNumeric12KeyHandler::actionHandler(HbInputModeAction action)
{
	bool ret = false;
	switch (action) {
		//In case of the numeric editor the character is already committed. 
		//Need to remove the committed character.
case HbInputModeHandler::HbInputModeActionDeleteAndCommit:{
	HbInputFocusObject *focusObject = 0;
	Q_D(HbHardwareInputNumeric12KeyHandler);
	focusObject = d->mInputMethod->focusObject();
	if (!focusObject) {
		return false;
	}
	d->mTimer->stop();
	QString empty;
	QList<QInputMethodEvent::Attribute> list;
	QInputMethodEvent event(QString(), list);
	event.setCommitString(empty, -1, 1);
	focusObject->sendEvent(event);
	ret = true;
														  }
default: {
	ret = false;
		 }
	}
	if(!ret) {
		ret = HbInputNumericHandler::actionHandler(action);
	}
	return ret;
}

// EOF
