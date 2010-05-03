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

#include "hbhardwareinputprediction12keyhandler.h"
#include "hbinputpredictionhandler_p.h"
#include "hbinputabstractbase.h"

class HbHardwareInputPrediction12KeyHandlerPrivate: public HbInputPredictionHandlerPrivate
{
    Q_DECLARE_PUBLIC(HbHardwareInputPrediction12KeyHandler)

public:
    HbHardwareInputPrediction12KeyHandlerPrivate();
    ~HbHardwareInputPrediction12KeyHandlerPrivate();

    bool keyReleased(const QKeyEvent *keyEvent);
    bool keyPressed(const QKeyEvent *keyEvent);
    void _q_timeout();

public:
    int mLastKey;
    bool mButtonDown;
    QChar mCurrentChar;
};

HbHardwareInputPrediction12KeyHandlerPrivate::HbHardwareInputPrediction12KeyHandlerPrivate()
:mLastKey(0),
mButtonDown(false),
mCurrentChar(0)
{
}

HbHardwareInputPrediction12KeyHandlerPrivate::~HbHardwareInputPrediction12KeyHandlerPrivate()
{
}

void HbHardwareInputPrediction12KeyHandlerPrivate::_q_timeout()
{
    qDebug("HbHardwareInputPrediction12KeyHandlerPrivate::_q_timeout()");
    Q_Q(HbHardwareInputPrediction12KeyHandler);

    // let's stop the timer first.
    mTimer->stop();

    //Long key press number key is applicable to all keys
    if (mButtonDown && mCanContinuePrediction) {
        if (mLastKey == Qt::Key_Asterisk) {
            q->actionHandler(HbInputModeHandler::HbInputModeActionLaunchCandidatePopup);
        } else if (mLastKey == Qt::Key_Shift) {            
            updateTextCase();            
            mInputMethod->switchMode(Qt::Key_Shift);
        } else {
            q->actionHandler(HbInputModeHandler::HbInputModeActionDeleteAndCommit);
            q->commitFirstMappedNumber(mLastKey);            
        }
    }
}

bool HbHardwareInputPrediction12KeyHandlerPrivate::keyPressed(const QKeyEvent *keyEvent)
{
    Q_Q(HbHardwareInputPrediction12KeyHandler);
    HbInputFocusObject *focusObject = 0;
    focusObject = mInputMethod->focusObject();
    if (!focusObject) {
        return false;
    }

    int buttonId = keyEvent->key();
    // A new key has been pressed, so the timer for the previous key is not valid.
    if (((mLastKey != buttonId)) && mTimer->isActive()) {
        mTimer->stop();
        mInputMethod->updateState();
    }

    if (buttonId == Qt::Key_Return) {

        return true;
    } else if (buttonId == Qt::Key_Shift) {
        if (mTimer->isActive() && (mLastKey == buttonId)){
            mTimer->stop();
            //Since the mode change should not commit the auto-completed part
            //we need to disable the tail.
            mShowTail = false;
            //This should switch to the numeric mode and with a long key press 
            //should be back to the prediction mode again.
            //mInputMethod->activateNextState();
            mInputMethod->switchMode(Qt::Key_Shift);
        } else {
            updateTextCase();
            mTimer->start(HbLongPressTimerTimeout);
        }
        mLastKey = buttonId;
        mButtonDown = true;
        return true;
    } else if(buttonId == Qt::Key_Down) {
        q->actionHandler(HbInputModeHandler::HbInputModeActionLaunchCandidatePopup);
        return true;
    }

    mLastKey = buttonId;
    mButtonDown = true;

    // Asterisk key / sym key is handled in this class it self, so not passing it to 
    // the base mode handlers.
    if (buttonId != Qt::Key_Asterisk) {
        q->HbInputPredictionHandler::filterEvent(keyEvent);        
    }

    // custom button should not start timer.
    if ((buttonId & CUSTOM_INPUT_MASK) != CUSTOM_INPUT_MASK) {
        mTimer->start(HbLongPressTimerTimeout);
    }

    return true;
}

/*!
Handles the key release events from the VKB. Launches the SCT with key release event of
asterisk.
*/
bool HbHardwareInputPrediction12KeyHandlerPrivate::keyReleased(const QKeyEvent *keyEvent)
{
    Q_Q(HbHardwareInputPrediction12KeyHandler);
    // since button is released we can set buttonDown back to false.
    mButtonDown = false;

    if (keyEvent->key() == Qt::Key_Asterisk) {
        //Same asterisk key is used for launching candidate list (long key press)
        //and also for SCT. So, do not launch SCT if candidate list is already launched.
        mInputMethod->switchMode(Qt::Key_Asterisk);
        return true;
    } else if (keyEvent->key() == Qt::Key_Delete) {
        QKeyEvent keyEvent(QEvent::KeyRelease, Qt::Key_Backspace, Qt::NoModifier);
        q->sendAndUpdate(keyEvent);
        return true;
    }

    return false;
}


HbHardwareInputPrediction12KeyHandler::HbHardwareInputPrediction12KeyHandler(HbInputAbstractMethod *inputMethod)
:HbInputPredictionHandler(* new HbHardwareInputPrediction12KeyHandlerPrivate, inputMethod)
{
    Q_D(HbHardwareInputPrediction12KeyHandler);
    d->q_ptr = this;
}

HbHardwareInputPrediction12KeyHandler::~HbHardwareInputPrediction12KeyHandler()
{
}

/*!
filterEvent to handler keypress/release events.
*/
bool HbHardwareInputPrediction12KeyHandler::filterEvent(const QKeyEvent * event)
{
    Q_D(HbHardwareInputPrediction12KeyHandler);
    
        HbInputFocusObject *focusObject = 0;
    focusObject = d->mInputMethod->focusObject();
    
    //If there was a handling for empty candidate-list, i.e. the engine did not predict
    //any meaningful word for the input sequence.
    if(!d->mCanContinuePrediction) {
        int eventKey = event->key();
        switch(eventKey) {
        case Qt::Key_0:
        case Qt::Key_Asterisk:
        case Qt::Key_Space: {
            if(d->mCandidates->size() && focusObject) {
                //Remove the "?" mark
                (*d->mCandidates)[d->mBestGuessLocation].chop(1);
                d->updateEditor();
                d->mCanContinuePrediction = true;
                }
            }
        break;
        case Qt::Key_Shift: {
            if(event->type() == QEvent::KeyRelease) {
                //Remove the "?" mark
                deleteOneCharacter();
                }
            }
        //For the following set of keys, it does not matter.
        case Qt::Key_Backspace:
        case Qt::Key_Delete:
        case Qt::Key_Return:
        case Qt::Key_Enter: {            
            }
        break;
        //The default behavior for any other key press is just to consume the key event and
        //not to do anything.
        default: {
            return true;
            }
        break;
        }
    }

    // prediction mode can't handle Qt::Key_0, so we will emit a passFilterEvent
    // this signal must be connected to by the plugin to a modehandler.
    // which can handle it.
    if (event->key() == Qt::Key_0) {
        // if this previous key is not Qt::Key_0, we must commit it.
        if (d->mLastKey != Qt::Key_0) {
            actionHandler(HbInputModeHandler::HbInputModeActionCommit);
        }
        emit passFilterEvent(event);
        d->mLastKey = Qt::Key_0;
        return true;
    } else {
        if (d->mLastKey == Qt::Key_0) {
            emit passActionHandler(HbInputModeActionCommit);
        }
        if (event->isAutoRepeat()) {
            // Ignore all repeat events after first repeat event
            return true;
        } else if (event->type() == QEvent::KeyRelease) {
            return d->keyReleased(event);
        } else {
            return d->keyPressed(event);
        }
    }
}

/*!
Action handler 
*/
bool HbHardwareInputPrediction12KeyHandler::actionHandler(HbInputModeAction action)
{
    Q_D(HbHardwareInputPrediction12KeyHandler);
    bool ret = true;
    switch (action) {
case HbInputModeActionReset:
    HbInputPredictionHandler::actionHandler(HbInputModeActionReset);
    d->mTimer->stop();
    break;
case HbInputModeActionFocusRecieved:
    HbInputPredictionHandler::actionHandler(HbInputModeActionSetCandidateList);
    HbInputPredictionHandler::actionHandler(HbInputModeActionSetKeypad);
    d->mTimer->stop();
    break;
default:
    ret = HbInputPredictionHandler::actionHandler(action);
    break;
    }

    return ret;
}

/*!
Returns true if preidciton engine is available and initialized.
*/
bool HbHardwareInputPrediction12KeyHandler::isActive() const
{ 
    Q_D(const HbHardwareInputPrediction12KeyHandler);
    return d->mEngine != 0;
}

//EOF
