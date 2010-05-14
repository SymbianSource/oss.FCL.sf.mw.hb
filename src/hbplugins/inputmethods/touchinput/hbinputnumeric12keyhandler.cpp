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
#include <hbinputbutton.h>

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
    bool handleAlphaEvent(int buttonId, HbKeyboardType type);
    bool buttonPressed(const QKeyEvent *keyEvent);
    bool buttonReleased(const QKeyEvent *keyEvent);
    void _q_timeout();
public:
    int mLastKey;
    bool mLongPressHappened;
    int mButtonDown;
	int mMultiTapNum;
	QChar mCurrentMultitapChar;
};

HbInputNumeric12KeyHandlerPrivate::HbInputNumeric12KeyHandlerPrivate():
    mLastKey(0), mLongPressHappened(false),
    mButtonDown(false),
	mMultiTapNum(0),
	mCurrentMultitapChar(0)
{
}

HbInputNumeric12KeyHandlerPrivate::~HbInputNumeric12KeyHandlerPrivate()
{
}

void HbInputNumeric12KeyHandlerPrivate::handleMultitapStarKey()
{
	HbInputFocusObject *focusObject = mInputMethod->focusObject();
	if (!focusObject) {
		return;
	}
	QChar MultitapStarKeyArray[] = {'*','+','p','w','\0'};
	
    int index = mMultiTapNum;
    do {
        mCurrentMultitapChar = MultitapStarKeyArray[mMultiTapNum];
        mMultiTapNum = (++mMultiTapNum)%4;
	    if (mCurrentMultitapChar != 0) {
            if (focusObject->characterAllowedInEditor(mCurrentMultitapChar)) {
		        QString str;
		        str += mCurrentMultitapChar;

		        QList<QInputMethodEvent::Attribute> list;
		        QInputMethodEvent event(str,list);
		        focusObject->sendEvent(event);
                return;
            }
	    }
    } while (index != mMultiTapNum);
}

bool HbInputNumeric12KeyHandlerPrivate::handleAlphaEvent(int buttonId, HbKeyboardType type)
{
    Q_Q(HbInputNumeric12KeyHandler);

    HbInputFocusObject *focusObject = 0;
    focusObject = mInputMethod->focusObject();
    if (!focusObject) {
        return false;
    }

    QChar character = 0;
	//This condition is to avoid get the characters mapped to Asterisk
	//Especially for Thai language we have mapped character to Asterisk
    if (buttonId != HbInputButton::ButtonKeyCodeAsterisk ||
        mInputMethod->currentKeyboardType() == HbKeyboardSctPortrait) {
        int index = 0;
        character = q->getNthCharacterInKey(index, buttonId, type);
	}

    if (character != 0) {
        q->commitAndUpdate(character);
        return true;
    }
    return false;
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
    if (keyEvent->isAutoRepeat() && mLastKey == buttonId) {
        if (buttonId == HbInputButton::ButtonKeyCodeShift) {
            // If the editor is not a number only editor, then activate the alphanumeric keypad
            if (!focusObject->editorInterface().isNumericEditor()) {
                mInputMethod->switchMode(buttonId);
                mLongPressHappened = true;
            }
        } else if (buttonId == HbInputButton::ButtonKeyCodeSymbol) {
            mInputMethod->selectSpecialCharacterTableMode();
            mLongPressHappened = true;
        } else if (buttonId == HbInputButton::ButtonKeyCodeAsterisk &&
                   mInputMethod->currentKeyboardType() != HbKeyboardSctPortrait) {
            mLongPressHappened = true;
            mCurrentMultitapChar = QChar(HbInputButton::ButtonKeyCodeAsterisk);
            HbInputFocusObject *focusedObject = mInputMethod->focusObject();
            if (focusedObject) {
			    focusedObject->filterAndCommitCharacter(mCurrentMultitapChar);
            }
        	mCurrentMultitapChar = 0;
        }
        if (mLongPressHappened) {
            mLastKey = 0;
            return true;
        }
	}		   
   
	if (mInputMethod) {
		if (mLastKey != buttonId) {
			if (mCurrentMultitapChar !=0) {
                if (!focusObject->characterAllowedInEditor(mCurrentMultitapChar)) {
					focusObject->sendCommitString(QString());
                } else {
					QChar commitChar(mCurrentMultitapChar);
					mCurrentMultitapChar = 0;
					q->commitAndUpdate(commitChar);
				}
			}
		}
		if (buttonId == HbInputButton::ButtonKeyCodeAsterisk) {
			mTimer->stop();
			mTimer->start(HbMultiTapTimerTimeout);
		}
	} 

    mLastKey = buttonId;
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
    
    if (mLongPressHappened) {
        mLongPressHappened = false;
        return false;
    }

    if (mTimer->isActive() && buttonId == Qt::Key_Shift) {
        mTimer->stop();
    }
    if (mLastKey != buttonId) {
		mMultiTapNum = 0;
    }

	mButtonDown = 0;

    if (buttonId == HbInputButton::ButtonKeyCodeAsterisk &&
        mInputMethod->currentKeyboardType() != HbKeyboardSctPortrait) {
        //Asterisk Key will multitap bettween *,+,p,w
        //mInputMethod->switchMode(buttonId);
		mLastKey = buttonId;
		handleMultitapStarKey();
        return true;
    } else if (buttonId == HbInputButton::ButtonKeyCodeSymbol) {
        mInputMethod->switchMode(buttonId);
    } else if (buttonId == HbInputButton::ButtonKeyCodeAlphabet) {
		mInputMethod->switchMode(buttonId);
		mLastKey = buttonId;
		return true;
	} else if (buttonId == Qt::Key_Return) {
        mInputMethod->closeKeypad();
        return true;
	} else if (buttonId == HbInputButton::ButtonKeyCodeShift) {
        //Let's commit character "#" on single tap and double tap of shift Key
		mLastKey = buttonId;
        QChar qc = QChar('#');
        q->commitAndUpdate(qc);
        return true;            
    } else if (buttonId >= 0) {
        // Let's see if we can get the handler for this button in the base class.
        if (q->HbInputNumericHandler::filterEvent(keyEvent)) {
            return true;
        }
        mLastKey = buttonId;
        if (mInputMethod->currentKeyboardType() == HbKeyboardSctPortrait &&
            handleAlphaEvent(buttonId, mInputMethod->currentKeyboardType())) {
            return true;
        }
        q->commitFirstMappedNumber(buttonId, mInputMethod->currentKeyboardType());
        return true;
    }
    return false;
}

void HbInputNumeric12KeyHandlerPrivate::_q_timeout()
{
    mTimer->stop();
	mMultiTapNum = 0;

    HbInputFocusObject *focusedObject = mInputMethod->focusObject();
    if (!focusedObject) {
        qDebug("HbInputNumeric12KeyHandler::timeout focusObject == 0");
        return;
    }

    if (!mButtonDown) {
        if (mCurrentMultitapChar != 0) {
			focusedObject->filterAndCommitCharacter(mCurrentMultitapChar);
        }
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
        	d->mButtonDown = false;
            break;
        //In case of the numeric editor the character is already committed.
        //Need to remove the committed character.
        case HbInputModeHandler::HbInputModeActionDeleteAndCommit: {
            HbInputFocusObject *focusObject = 0;
            
            focusObject = d->mInputMethod->focusObject();
            if (!focusObject) {
                return false;
            }
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
