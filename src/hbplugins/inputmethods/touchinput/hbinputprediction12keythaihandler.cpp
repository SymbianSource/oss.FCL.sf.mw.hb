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

#include <hbinputsettingproxy.h>
#include <hbinputkeymapfactory.h>
#include <hbinputpredictionengine.h>

#include "virtual12key.h"

#include "hbinputprediction12keyhandler_p.h"
#include "hbinputprediction12keythaihandler.h"

class HbInputPrediction12KeyThaiHandlerPrivate: public HbInputPrediction12KeyHandlerPrivate
{
    Q_DECLARE_PUBLIC(HbInputPrediction12KeyThaiHandler)

public:
    HbInputPrediction12KeyThaiHandlerPrivate();
    ~HbInputPrediction12KeyThaiHandlerPrivate();

    bool buttonReleased(const QKeyEvent *keyEvent);
    bool buttonPressed(const QKeyEvent *keyEvent);
    void _q_timeout();
};

HbInputPrediction12KeyThaiHandlerPrivate::HbInputPrediction12KeyThaiHandlerPrivate()
{
}

HbInputPrediction12KeyThaiHandlerPrivate::~HbInputPrediction12KeyThaiHandlerPrivate()
{
   
}

void HbInputPrediction12KeyThaiHandlerPrivate::_q_timeout()
{
	Q_Q(HbInputPrediction12KeyHandler);
    // let's stop the timer first.
    mTimer->stop();

    if (mButtonDown) {
		if(mLastKey == Qt::Key_0){
			q->actionHandler(HbInputModeHandler::HbInputModeActionCommit);
			q->commitFirstMappedNumber(mLastKey);		
		} else if(mLastKey != Qt::Key_Asterisk) {
			//Long key press number key is applicable to all keys so pass it to Base class
			HbInputPrediction12KeyHandlerPrivate::_q_timeout();            
        }
    }
}

bool HbInputPrediction12KeyThaiHandlerPrivate::buttonPressed(const QKeyEvent *keyEvent)
{
    mLongPressHappened = false;
    HbInputFocusObject *focusObject = 0;
    focusObject = mInputMethod->focusObject();
    if (!focusObject) {
        return false;
    }
	
    int buttonId = keyEvent->key();

    //Pass the event to base class except Shift key
	if (buttonId == Qt::Key_Shift ) {		
	  mLastKey = buttonId;
	  mButtonDown = true;
	} else {
		HbInputPrediction12KeyHandlerPrivate::buttonPressed(keyEvent);
	}
	// custom button should not start timer.
    if ((buttonId & CUSTOM_INPUT_MASK) != CUSTOM_INPUT_MASK) {
        mTimer->start(HbLongPressTimerTimeout);
    }
    return false;
}

/*!
Handles the key release events from the VKB. Launches the SCT with key release event of
asterisk.
*/
bool HbInputPrediction12KeyThaiHandlerPrivate::buttonReleased(const QKeyEvent *keyEvent)
{
    Q_Q(HbInputPrediction12KeyHandler);
    
    if(!mButtonDown || mLongPressHappened){
        return false;
    }

    int buttonId = keyEvent->key(); 
    // it was a long press on sct swith button. so just return form here.
    if (!mTimer->isActive() && buttonId == Qt::Key_Control) {
        return true;
    }

	if (buttonId == Qt::Key_Asterisk && !mInputMethod->isSctModeActive()) {
		//Handle if key Asterisk pressed and SCT is not launched or else pass it to base handlers
		if (q->HbInputPredictionHandler::filterEvent(keyEvent)) {
			mButtonDown = false;
			return true;
		}
    } else if ( buttonId == Qt::Key_Shift ) {
		//As we can't map charatcers to Shift key in keymapping, making use of "#" key i.e. Qt::Key_NumberSign
		//in keymapping and manipulating event to Qt::Key_NumberSign when shift key is pressed
		const QKeyEvent *event = new QKeyEvent(QEvent::KeyPress, Qt::Key_NumberSign, Qt::NoModifier);
		if (q->HbInputPredictionHandler::filterEvent(event)) {
			mButtonDown = false;
			return true;
		}		
	} else {
		HbInputPrediction12KeyHandlerPrivate::buttonReleased(keyEvent);
	}
    return false;
}


HbInputPrediction12KeyThaiHandler::HbInputPrediction12KeyThaiHandler(HbInputAbstractMethod *inputMethod)
    :HbInputPrediction12KeyHandler(* new HbInputPrediction12KeyThaiHandlerPrivate, inputMethod)
{
    Q_D(HbInputPrediction12KeyThaiHandler);
    d->q_ptr = this;
}

HbInputPrediction12KeyThaiHandler::~HbInputPrediction12KeyThaiHandler()
{
}
/*!
    filterEvent to handler keypress/release events.
*/

bool HbInputPrediction12KeyThaiHandler::filterEvent(const QKeyEvent * event)
{
    Q_D(HbInputPrediction12KeyThaiHandler);
    HbInputFocusObject *focusObject = 0;
    focusObject = d->mInputMethod->focusObject();

    //If there was a handling for empty candidate-list, i.e. the engine did not predict
    //any meaningful word for the input sequence. 
   
	if(!d->mCanContinuePrediction) {
		int eventKey = event->key();
		//let's us return If engine did not predict any meaningful word for the input sequence 
		//for Shift,Asterisk and Control
		if(eventKey == Qt::Key_Control || eventKey == Qt::Key_0) {
			if(d->mCandidates->size() && focusObject ) {
				//Remove the "?" mark
				(*d->mCandidates)[d->mBestGuessLocation].chop(1);
				d->updateEditor();
				d->mCanContinuePrediction = true;
			}
		} else if (eventKey != Qt::Key_Shift && eventKey != Qt::Key_Asterisk){
			// For Shift key and Asterisk key Will handle it in button release Since we have character mapped to Shift and Asterisk
			// or else pass it to Prediction12KeyHandler handler
			HbInputPrediction12KeyHandler::filterEvent(event);
		}
    }	

	// If the word is in inline edit First tap of Qt::Key_0 should commit the word in the editor
	// For successive tap prediction mode can't handle Qt::Key_0, so we will emit a passFilterEvent
    // this signal must be connected to by the plugin to a modehandler.
    // which can handle it.

	if (event->key() == Qt::Key_0 && d->mEngine->inputLength() >= 1 ) {
		if(event->type() == QEvent::KeyPress) {
			d->mButtonDown = true;
			// start Long Press timer as zero key should be allowed to enter
			d->mTimer->start(HbLongPressTimerTimeout);
		} else if(event->type() == QEvent::KeyRelease) {
			d->mTimer->stop();
			d->mButtonDown = false;
			actionHandler(HbInputModeHandler::HbInputModeActionCommit);
		}
		d->mLastKey = Qt::Key_0;
		return true;
	} else if (event->key() == Qt::Key_0) {
		emit passFilterEvent(event);
		d->mLastKey = Qt::Key_0;
		return true;
	} else {
		if (d->mLastKey == Qt::Key_0) {
			emit passActionHandler(HbInputModeActionCommit);
		}
		if (event->type() == QEvent::KeyRelease) {
			return d->buttonReleased(event);
		} else {
			return d->buttonPressed(event);
		}
	}
}


//EOF

