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
#include <hbinputkeymap.h>
#include <hbinputkeymapfactory.h>
#include <hbinputsettingproxy.h>

#include "hbinputabstractbase.h"
#include "hbinputbasichandler_p.h"
#include "hbinputmodehandler_p.h"
#include "hbhardwareinputbasicqwertyhandler.h"

class HbHardwareInputBasicQwertyHandlerPrivate: public HbInputBasicHandlerPrivate
{
    Q_DECLARE_PUBLIC(HbHardwareInputBasicQwertyHandler)
public:
    HbHardwareInputBasicQwertyHandlerPrivate();
    ~HbHardwareInputBasicQwertyHandlerPrivate();

    // button related operations.
    bool buttonPressed(const QKeyEvent *event);
    bool buttonReleased(const QKeyEvent *event);
    void _q_timeout();

public:
    HbFnState mFnState;
    int mButton;
    HbInputShiftKeyState mShiftKeyState;
};

HbHardwareInputBasicQwertyHandlerPrivate::HbHardwareInputBasicQwertyHandlerPrivate()
:mFnState(HbFnOff),
mButton(0),
mShiftKeyState(EKeyPressedNone)
{
}

HbHardwareInputBasicQwertyHandlerPrivate::~HbHardwareInputBasicQwertyHandlerPrivate()
{
}

bool HbHardwareInputBasicQwertyHandlerPrivate::buttonPressed(const QKeyEvent * event)
{
	mButton = event->key();
	if (!mTimer->isActive()){
		mTimer->start(HbLongPressTimerTimeout);
	}

	if (event->key() != Qt::Key_Delete && event->key() != Qt::Key_Backspace) {
		return true;
	} else {
		return false;
	}
}

bool HbHardwareInputBasicQwertyHandlerPrivate::buttonReleased(const QKeyEvent * event)
{
// Kept for handling button released specific logic
	
	Q_Q(HbHardwareInputBasicQwertyHandler);
	HbInputFocusObject *focusObject = 0;
    focusObject = mInputMethod->focusObject();
    if (!focusObject) {
        qDebug("HbHardwareInputBasicQwertyHandler::virtualButtonClicked : no focused editor widget!");
        return false;
    }
	
	// If the timer is not active and it is alpha mode, it is a long press
    // and handled in another function. So just return.
    if (mTimer->isActive()) {
        mTimer->stop();
    }
	int eventKey = event->key();

	//Handle long key press here
   
    switch(eventKey) {
    case Qt::Key_Alt:
		//handle function key (fn key) states here using fn & shift handler helper class
		break;
    case Qt::Key_Shift: {
        //If the shift key state is EShiftKeyPressed and a second shift key press is received before any other
        //key events, then current text case should not be changed.
        if (mShiftKeyState){
            mShiftKeyState = EKeyPressedNone;
        } else {
            mShiftKeyState = EShiftKeyPressed;
            int currentTextCase = focusObject->editorInterface().textCase();
            switch(currentTextCase) {
            case HbTextCaseLower:
                focusObject->editorInterface().setTextCase(HbTextCaseUpper);
                focusObject->syncEditorInterface();
                break;
            case HbTextCaseUpper:
            case HbTextCaseAutomatic:
                focusObject->editorInterface().setTextCase(HbTextCaseLower);
                focusObject->syncEditorInterface();
                break;
            default:
                break;
            }
        }
    }
        break;
    case Qt::Key_Control:{
		return true;
		}
		
		break;
	case Qt::Key_Backspace:
	case Qt::Key_Delete: {
		// let's pass the backspace event to the focussed editor.
		//return q->HbInputBasicHandler::filterEvent(event);
		break;
	}
	case Qt::Key_Return:
	case Qt::Key_Enter:
	case Qt::Key_Space: {
		QChar qc(eventKey);
		if(Qt::Key_Space == eventKey && (event->modifiers() & Qt::ControlModifier)){
			if (HbInputSettingProxy::instance()->predictiveInputStatus()) {
					HbInputSettingProxy::instance()->setPredictiveInputStatus(0);
				} else {
					HbInputSettingProxy::instance()->setPredictiveInputStatus(1);
				}
			break;
		}

		if (qc == Qt::Key_Enter || qc == Qt::Key_Return) {
			qc = QChar('\n');  // Editor expects normal line feed.
		}
		if(focusObject){
			q->commitAndUpdate(qc);	
		}
		break;
		}
    default: {
		if (q->HbInputBasicHandler::filterEvent(event)) {
				return true;
			}   
		QList<QInputMethodEvent::Attribute> list;
        QString newText;
        int currentTextCase = focusObject->editorInterface().textCase();
        // If function key is pressed, get the functionized 
        
        QString chars;

        const HbMappedKey* mappedKey = mKeymap->keyForKeycode(mInputMethod->inputState().keyboard(), eventKey);
        
        if (currentTextCase == HbTextCaseLower)
           chars = mappedKey->characters(HbModifierNone);
        else
           chars = mappedKey->characters(HbModifierShiftPressed);

        if(chars.length()){
            q->commitAndUpdate(chars);
        }

        // Reset the shift state.
        if (EShiftKeyPressed == mShiftKeyState) {
            mShiftKeyState = EKeyPressedNone;
            int textCase = HbTextCaseLower;
            HbModifier modifier = HbModifierNone;

            if (HbTextCaseLower == currentTextCase) {
                textCase = HbTextCaseUpper;
                modifier = HbModifierShiftPressed;
            }
            focusObject->editorInterface().setTextCase(textCase);
            focusObject->syncEditorInterface();
        }
        mInputMethod->updateState();
        }
        break;
    }	
	return true;
}

void HbHardwareInputBasicQwertyHandlerPrivate::_q_timeout()
{
    mTimer->stop();
	qDebug("Timer stoped");
	if (mButton == Qt::Key_Shift ){
        mShiftKeyState = EShiftKeyPressed;
	} else {		
		mFnState = HbFnNext;
	}
	return;
}

HbHardwareInputBasicQwertyHandler::HbHardwareInputBasicQwertyHandler(HbInputAbstractMethod* inputMethod)
:HbInputBasicHandler(* new HbHardwareInputBasicQwertyHandlerPrivate, inputMethod)
{
    Q_D(HbHardwareInputBasicQwertyHandler);
    d->q_ptr = this;
}

HbHardwareInputBasicQwertyHandler::~HbHardwareInputBasicQwertyHandler()
{
}

/*!
filterEvent function key handling.
*/
bool HbHardwareInputBasicQwertyHandler::filterEvent(const QKeyEvent* event)
{
    Q_D(HbHardwareInputBasicQwertyHandler);

	if (!event->isAutoRepeat()) {
		if ((event->type() == QEvent::KeyRelease) ) {
			return d->buttonReleased(event);
		} else {
			return d->buttonPressed(event);
		}
	}
    return false;
}

/*!
returns true if in inline edit.
*/
bool HbHardwareInputBasicQwertyHandler::isComposing() const
{   
    Q_D(const HbHardwareInputBasicQwertyHandler);
    return d->mTimer->isActive();
}

/*!
Action handler
*/
bool HbHardwareInputBasicQwertyHandler::actionHandler(HbInputModeAction action)
{
    Q_D(HbHardwareInputBasicQwertyHandler);
    bool ret = true;
    switch (action) {
    case HbInputModeActionReset:
        if (d->mTimer->isActive()) {
            d->mTimer->stop();
			qDebug("Timer stoped");
        }
        break;
    default: {
        ret = HbInputBasicHandler::actionHandler(action);
        }
    }
    return ret;
}

/*!
this SLOT is called by input plugin when there is a character selected from character preview pane.
*/
void HbHardwareInputBasicQwertyHandler::charFromPreviewSelected(QString characters)
{
    Q_D(HbHardwareInputBasicQwertyHandler);
    if(characters.size() > 0) {
        sctCharacterSelected(characters[0]);
        d->mInputMethod->updateState();
    }
}

void HbHardwareInputBasicQwertyHandler::sctCharacterSelected(QChar character)
{
    HbInputModeHandler::sctCharacterSelected(character);
}
