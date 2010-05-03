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
#include "hbinputbasic12keythaihandler.h"
#include "hbinputbasic12keyhandler_p.h"

class HbInputBasic12KeyThaiHandlerPrivate: public HbInputBasic12KeyHandlerPrivate
{
    Q_DECLARE_PUBLIC(HbInputBasic12KeyThaiHandler)

public:
    HbInputBasic12KeyThaiHandlerPrivate();
    ~HbInputBasic12KeyThaiHandlerPrivate();

	void showThaiSpecialCharacters();
    bool buttonReleased(const QKeyEvent *keyEvent);
    void _q_timeout();

};

HbInputBasic12KeyThaiHandlerPrivate::HbInputBasic12KeyThaiHandlerPrivate()
{

}

HbInputBasic12KeyThaiHandlerPrivate::~HbInputBasic12KeyThaiHandlerPrivate()
{

}


/*!
Handles the key release events from the VKB. Launches Thai special popup with key release event of
asterisk and shift key.
*/
bool HbInputBasic12KeyThaiHandlerPrivate::buttonReleased(const QKeyEvent *keyEvent)
{
	Q_UNUSED(keyEvent);
    HbInputVkbWidget::HbFlickDirection flickDir = static_cast<HbVirtual12Key*>(mInputMethod)->flickDirection();
	if (mInputMethod && flickDir!=HbInputVkbWidget::HbFlickDirectionDown) {
		int buttonId = keyEvent->key();
		HbInputFocusObject *focusObject = 0;
		focusObject = mInputMethod->focusObject();
		if (!focusObject || !mDownKey) {
			return false;
		}
		if ( mLongPressHappened ){
			return false;
		}
		//Handle if Shift and Asterisk key release happen or else let's pass it to base class to handle
		if (buttonId == Qt::Key_Shift) {
			//For Thai Language Launch Special Characters popup 
			mInputMethod->showThaiSpecialCharacters(buttonId);
			mLastKey = buttonId;
			mCurrentChar = 0;
			mDownKey = 0;
			return true;				
		} else if (buttonId == Qt::Key_Asterisk && !mInputMethod->isSctModeActive()) {
			//For Thai Language Launch Special Characters popup 
			mInputMethod->showThaiSpecialCharacters(buttonId);
			mLastKey = buttonId;
			mCurrentChar = 0;
			mDownKey = 0;
			return true;			
		} else {
			HbInputBasic12KeyHandlerPrivate::buttonReleased(keyEvent);
		}
	}
	return false;
}
/*!
Launches Thai special popup with long key press event of asterisk 
*/
void HbInputBasic12KeyThaiHandlerPrivate::_q_timeout()
{
    mTimer->stop();
    mNumChr = 0;

    HbInputFocusObject *focusedObject = 0;
    focusedObject = mInputMethod->focusObject();
    if (!focusedObject) {
        return;
    }

    //Long key press number key is applicable to all keys except Asterisk
    if (mDownKey && mDownKey == Qt::Key_Asterisk) {
		//For Thai Language Launch Special Characters popup 
		mInputMethod->showThaiSpecialCharacters(mDownKey); 
	} else {
		HbInputBasic12KeyHandlerPrivate::_q_timeout();
	}
 	mDownKey = 0;        
    mCurrentChar = 0;
    return;
}

HbInputBasic12KeyThaiHandler::HbInputBasic12KeyThaiHandler(HbInputAbstractMethod* inputMethod)
:HbInputBasic12KeyHandler(*new HbInputBasic12KeyThaiHandlerPrivate, inputMethod)
{
    Q_D(HbInputBasic12KeyThaiHandler);
    d->q_ptr = this;
}

HbInputBasic12KeyThaiHandler::~HbInputBasic12KeyThaiHandler()
{
}

/*!
 filterEvent function for handling different keyevents.
*/
bool HbInputBasic12KeyThaiHandler::filterEvent(const QKeyEvent * event)
{
    Q_D(HbInputBasic12KeyThaiHandler);

    if (event->type() == QEvent::KeyRelease) {
        return d->buttonReleased(event);
    } else {
        return d->buttonPressed(event);
    }
}

//End of file

