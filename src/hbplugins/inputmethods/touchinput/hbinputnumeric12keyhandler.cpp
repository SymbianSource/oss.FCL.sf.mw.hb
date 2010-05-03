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

#include "hbinputnumeric12keyhandler.h"
#include "hbinputnumerichandler_p.h"
#include "virtual12key.h"
#include "hbinputabstractbase.h"

class HbInputNumeric12KeyHandlerPrivate: public HbInputNumericHandlerPrivate
{
    Q_DECLARE_PUBLIC(HbInputNumeric12KeyHandler)

public:
    HbInputNumeric12KeyHandlerPrivate();
    ~HbInputNumeric12KeyHandlerPrivate();

	void handleMultitapStarKey();
    bool buttonPressed(const QKeyEvent *keyEvent);
    bool buttonReleased(const QKeyEvent *keyEvent);
    void _q_timeout();
public:
    int mLastKey;
    int mButtonDown;
	int mMultiTapNum;
	QChar mCurrentMultitapChar;
};

HbInputNumeric12KeyHandlerPrivate::HbInputNumeric12KeyHandlerPrivate():
    mLastKey(0),
    mButtonDown(0),
	mMultiTapNum(0),
	mCurrentMultitapChar(0)
{
}

HbInputNumeric12KeyHandlerPrivate::~HbInputNumeric12KeyHandlerPrivate()
{
}

void HbInputNumeric12KeyHandlerPrivate::handleMultitapStarKey()
{
	HbInputFocusObject *focusObject = 0;
	focusObject = mInputMethod->focusObject();
	if (!focusObject) {
		return;
	}
	QChar MultitapStarKeyArray[] = {'+','*','p','w','\0'};
	mCurrentMultitapChar = MultitapStarKeyArray[mMultiTapNum];
	
	mMultiTapNum = (++mMultiTapNum)%4;
	if (mCurrentMultitapChar != 0) {
		QString str;
		str += mCurrentMultitapChar;

		QList<QInputMethodEvent::Attribute> list;
		QInputMethodEvent event(str,list);
		focusObject->sendEvent(event);
	}
}
bool HbInputNumeric12KeyHandlerPrivate::buttonPressed(const QKeyEvent *keyEvent)
{
	Q_Q(HbInputNumeric12KeyHandler);
	HbInputFocusObject *focusObject = 0;
    focusObject = mInputMethod->focusObject();
    if (!focusObject) {
        return false;
    }
	int buttonId = keyEvent->key();
	mButtonDown = buttonId;
    
	if (buttonId == Qt::Key_Shift) {
        mTimer->start(HbLongPressTimerTimeout);
		mLastKey = buttonId;
		return true;
	}		   
	if (mInputMethod) {
		if (mLastKey != buttonId) {
			if (mCurrentMultitapChar !=0) {
				if (!focusObject->characterAllowedInEditor(mCurrentMultitapChar))
					focusObject->sendCommitString(QString());
				else {
					QChar commitChar(mCurrentMultitapChar);
					mCurrentMultitapChar = 0;
					q->commitAndUpdate(commitChar);
				}
			}
		}
		if (buttonId == Qt::Key_Asterisk) {
			mTimer->stop();
			mTimer->start(HbMultiTapTimerTimeout);
		}
		return false;
	}           
    return false;
}

/*!
Handles the key release events from the VKB. Launches the SCT with key release event of
asterisk.
*/
bool HbInputNumeric12KeyHandlerPrivate::buttonReleased(const QKeyEvent *keyEvent)
{
    Q_Q(HbInputNumeric12KeyHandler);
    HbInputFocusObject *focusObject = 0;
    focusObject = mInputMethod->focusObject();
    if (!focusObject || !mButtonDown) {
        qDebug("HbInputModeHandler::buttonReleased no focusObject ... failed!!");
        return false;
    }
	int buttonId = keyEvent->key();
	if(mTimer->isActive() && buttonId == Qt::Key_Shift) {
		mTimer->stop();
	}
    
	if (mLastKey != buttonId)
		mMultiTapNum = 0;

	mButtonDown = 0;

    if (buttonId == Qt::Key_Asterisk) {
        //Asterisk Key will multitap bettween *,+,p,w
        //mInputMethod->switchMode(buttonId);
		mLastKey = buttonId;
		handleMultitapStarKey();
        return true;
	} else if (buttonId == Qt::Key_Control){
		mInputMethod->switchMode(buttonId);
		mLastKey = buttonId;
		return true;
	}
	else if (buttonId == Qt::Key_Return) {
        mInputMethod->closeKeypad();
        return true;
	} else if ( buttonId == Qt::Key_Shift ) {
		//Let's commit character "#" on single tap and double tap of shift Key
		mLastKey = buttonId;
		QChar qc(keyEvent->key());
		qc = QChar('#');
		q->commitAndUpdate(qc);
		return true;			
    } else if (buttonId >= 0) {
        // Let's see if we can get the handler for this button in the base class.
        if (q->HbInputNumericHandler::filterEvent(keyEvent)) {
            return true;
        }
        mLastKey = buttonId;
        q->commitFirstMappedNumber(buttonId);
        return true;
    }
    return false;
}

void HbInputNumeric12KeyHandlerPrivate::_q_timeout()
{
	Q_Q(HbInputNumeric12KeyHandler);
    mTimer->stop();
	mMultiTapNum = 0;

    HbInputFocusObject *focusedObject = 0;
    focusedObject = mInputMethod->focusObject();
    if (!focusedObject) {
        qDebug("HbInputNumeric12KeyHandler::timeout focusObject == 0");
        return;
    }
    //switch to Alpha mode when Long key press of Shift key is received
    if (mButtonDown)
    {
		if (mButtonDown == Qt::Key_Shift) {
			// If the editor is not a number only editor, then activate the alphanumeric keypad
            if( !focusedObject->editorInterface().isNumericEditor() ) {
				mInputMethod->switchMode(mLastKey);
				mLastKey = 0;
			}
			else
			{
				q->commitAndUpdate(QChar('#'));
			}
        }
		else if (mButtonDown == Qt::Key_Asterisk)
		{
			q->commitAndUpdate(QChar('*'));
		}
		mButtonDown = 0;
    }
	else {
		if (mCurrentMultitapChar != 0)
			focusedObject->filterAndCommitCharacter(mCurrentMultitapChar);
	}
	mCurrentMultitapChar = 0;
}

HbInputNumeric12KeyHandler::HbInputNumeric12KeyHandler(HbInputAbstractMethod* inputMethod)
:HbInputNumericHandler( *new HbInputNumeric12KeyHandlerPrivate, inputMethod)
{
    Q_D(HbInputNumeric12KeyHandler);
    d->q_ptr = this;
}

HbInputNumeric12KeyHandler::~HbInputNumeric12KeyHandler()
{
}

/*!
 filterEvent function for handling different keyevents.
*/
bool HbInputNumeric12KeyHandler::filterEvent(const QKeyEvent * event)
{
    Q_D(HbInputNumeric12KeyHandler);

    if (event->type() == QEvent::KeyPress) {
        return d->buttonPressed(event);
    } else if (event->type() == QEvent::KeyRelease) {
        return d->buttonReleased(event);
    }

    return false;
}

/*!
 Action handler
*/
bool HbInputNumeric12KeyHandler::actionHandler(HbInputModeAction action)
{
	Q_D(HbInputNumeric12KeyHandler);
    bool ret = false;
    switch (action) {
		case HbInputModeHandler::HbInputModeActionCancelButtonPress:
        case HbInputModeHandler::HbInputModeActionReset:
			d->mLastKey = 0;
        	d->mButtonDown = 0;
			d->mTimer->stop();
			break;
        //In case of the numeric editor the character is already committed.
        //Need to remove the committed character.
        case HbInputModeHandler::HbInputModeActionDeleteAndCommit: {
            HbInputFocusObject *focusObject = 0;
            
            focusObject = d->mInputMethod->focusObject();
            if (!focusObject) {
                return false;
            }
            d->mTimer->stop();
            if (focusObject->editorCursorPosition()) {
                QString empty;
                QList<QInputMethodEvent::Attribute> list;
                QInputMethodEvent event(QString(), list);
                event.setCommitString(empty, -1, 1);
                focusObject->sendEvent(event);
                ret = true;
            }
            break;
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
