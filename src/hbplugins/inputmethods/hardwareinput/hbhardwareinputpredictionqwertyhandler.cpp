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
#include <hbinputsettingproxy.h>

#include "hbinputpredictionhandler_p.h"
#include "HbHardwareInputPredictionQwertyHandler.h"
#include "hbinputabstractbase.h"

class HbHardwareInputPredictionQwertyHandlerPrivate: public HbInputPredictionHandlerPrivate
{
    Q_DECLARE_PUBLIC(HbHardwareInputPredictionQwertyHandler)
public:
    HbHardwareInputPredictionQwertyHandlerPrivate();
    ~HbHardwareInputPredictionQwertyHandlerPrivate();

public:
    bool buttonPressed(const QKeyEvent *event);
    bool buttonReleased(const QKeyEvent *event);
    void init();
    void _q_timeout();

public:
    bool mExactPopupLaunched;
    HbInputShiftKeyState mShiftKeyState;
    HbFnState           mFnState;
    int                 mButton;
};

HbHardwareInputPredictionQwertyHandlerPrivate::HbHardwareInputPredictionQwertyHandlerPrivate()
:   mExactPopupLaunched(false),
    mShiftKeyState(EKeyPressedNone),
    mFnState(HbFnOff),
    mButton(0)
{
}

HbHardwareInputPredictionQwertyHandlerPrivate::~HbHardwareInputPredictionQwertyHandlerPrivate()
{
}

void HbHardwareInputPredictionQwertyHandlerPrivate::init()
{
}

void HbHardwareInputPredictionQwertyHandlerPrivate::_q_timeout()
{
    qDebug("HbHardwareInputPredictionQwertyHandler::timeout called");
    mTimer->stop();
    if (mButton == Qt::Key_Shift ){
        mShiftKeyState = EShiftKeyPressed;
    } else {        
        mFnState = HbFnNext;
    }
}


bool HbHardwareInputPredictionQwertyHandlerPrivate::buttonReleased(const QKeyEvent *event)
{
    Q_Q(HbHardwareInputPredictionQwertyHandler);
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
        qDebug("Timer stoped");
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
        mShiftKeyState = EShiftKeyPressed;
            int currentTextCase = focusObject->editorInterface().textCase();
            switch(currentTextCase) {
            case HbTextCaseLower:
                focusObject->editorInterface().setTextCase(HbTextCaseUpper);
                focusObject->syncEditorInterface();
                emit q->setModifiers(HbModifierShiftPressed);
                break;
            case HbTextCaseUpper:
            case HbTextCaseAutomatic:
                focusObject->editorInterface().setTextCase(HbTextCaseLower);
                focusObject->syncEditorInterface();
                emit q->setModifiers(HbModifierNone);
                break;
            default:
                break;
            }
        }
        break;
    case Qt::Key_Control: { 
        }
        break;
    case Qt::Key_Left:{
        if(mCandidates->count()) {
            if (mExactPopupLaunched) {
                q->commitAndUpdate(mCandidates->at(1));
            } else {
                q->commitAndUpdate(mCandidates->at(0));
            }
        }       
        HbInputPredictionHandlerPrivate::reset();
        // close exactword popup.
        mInputMethod->closeExactWordPopup();
        break;
        }
    case Qt::Key_Right:{
        q->actionHandler(HbInputModeHandler::HbInputModeActionCommit);
        break;
        }
    case Qt::Key_Up:{
         if (mExactPopupLaunched) {
                q->commitAndUpdate(mCandidates->at(0));
            } else {
                q->actionHandler(HbInputModeHandler::HbInputModeActionLaunchCandidatePopup);
         }
        HbInputPredictionHandlerPrivate::reset();   
        // close exactword popup.
        mInputMethod->closeExactWordPopup();
        break;
        }
    case Qt::Key_Down:{
        q->actionHandler(HbInputModeHandler::HbInputModeActionLaunchCandidatePopup);
        break;
        }
    case Qt::Key_Space:{
        if (event->modifiers() & Qt::ControlModifier){
            if (HbInputSettingProxy::instance()->predictiveInputStatus()) {
                    HbInputSettingProxy::instance()->setPredictiveInputStatus(0);
                } else {
                    HbInputSettingProxy::instance()->setPredictiveInputStatus(1);
                }
            break;
            }
        }
    default: {
            int currentTextCase = focusObject->editorInterface().textCase();
            mModifiers = Qt::NoModifier;
            if (mFnState == HbFnNext) {
                mModifiers |= Qt::AltModifier;
                mFnState = HbFnOff;
            } else if (mFnState == HbFnOn) {
                mModifiers |= Qt::AltModifier;
            }
            // If shift is pressed, the shifted characters have to be input.
            if ( HbTextCaseUpper == currentTextCase || HbTextCaseAutomatic == currentTextCase ) {
                mModifiers |= Qt::ShiftModifier;
            }
            // let's pass it to the base class.
            ret = q->HbInputPredictionHandler::filterEvent(event);

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
                emit q->setModifiers(modifier);
            }
            mInputMethod->updateState();
        }
        break;
    };
    return ret;
}

bool HbHardwareInputPredictionQwertyHandlerPrivate::buttonPressed(const QKeyEvent* event)
{
    Q_UNUSED(event);
    if (!mTimer->isActive()){
        mTimer->start(HbLongPressTimerTimeout);
        qDebug("Timer started");
    }
    return true;
}


HbHardwareInputPredictionQwertyHandler::HbHardwareInputPredictionQwertyHandler(HbInputAbstractMethod *inputMethod)
    :HbInputPredictionHandler(* new HbHardwareInputPredictionQwertyHandlerPrivate, inputMethod)
{
    Q_D(HbHardwareInputPredictionQwertyHandler);
    d->q_ptr = this;
    d->init();
}

HbHardwareInputPredictionQwertyHandler::~HbHardwareInputPredictionQwertyHandler()
{
}

/*!
Action Handler.
*/
bool HbHardwareInputPredictionQwertyHandler::actionHandler(HbInputModeAction action)
{
    Q_D(HbHardwareInputPredictionQwertyHandler);
    bool ret = true;
    switch (action) {
        case HbInputModeActionFocusRecieved:
            HbInputPredictionHandler::actionHandler(HbInputModeActionSetCandidateList);
            HbInputPredictionHandler::actionHandler(HbInputModeActionSetKeypad);
            d->mTimer->stop();
            qDebug("Timer stoped");
            break;
        case HbInputModeActionFocusLost:
            HbInputPredictionHandler::actionHandler(HbInputModeActionFocusLost);
            if (d->mCandidates->count()) {
                if (d->mExactPopupLaunched) {
                    commitAndUpdate(d->mCandidates->at(1));
                } else {
                    commitAndUpdate(d->mCandidates->at(0));
                }
            }        
            // close exactword popup.
            d->mInputMethod->closeExactWordPopup();
            break;
        default: ret = HbInputPredictionHandler::actionHandler(action);
    }

    return ret;
}

/*!
filterEvent for key event.
*/
bool HbHardwareInputPredictionQwertyHandler::filterEvent(const QKeyEvent * event)
{
    Q_D(HbHardwareInputPredictionQwertyHandler);

    if (!event->isAutoRepeat()) {
        if ((event->type() == QEvent::KeyRelease) ) {
            return d->buttonReleased(event);
        } else {
            return d->buttonPressed(event);
        }
    }

    return true;
}

/*!
this SLOT is called when a character on character previe pane is selected.
*/
void HbHardwareInputPredictionQwertyHandler::charFromPreviewSelected(QString character)
{
    if(character.size() > 0) {
        appendUnicodeCharacter(character[0]);
    }
}

/*!
 this function is called by HbPredictionHandler when HbPredictionHandler encounters a exact word.
*/
void HbHardwareInputPredictionQwertyHandler::processExactWord(QString exactWord)
{
    Q_D(HbHardwareInputPredictionQwertyHandler);
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
void HbHardwareInputPredictionQwertyHandler::exactWordPopupClosed()
{
    commitExactWord();
}
void HbHardwareInputPredictionQwertyHandler::sctCharacterSelected(QChar character)
{
    HbInputPredictionHandler::sctCharacterSelected(character);
}

/*!
Returns true if preidciton engine is available and initialized.
*/
bool HbHardwareInputPredictionQwertyHandler::isActive() const
{ 
    Q_D(const HbHardwareInputPredictionQwertyHandler);
    return d->mEngine != 0;
}

//EOF
