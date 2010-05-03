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

//Required Qt headers
#include <QLocale>
//Required Hb headers
#include <hbinputkeymapfactory.h>
#include <hbinputkeymap.h>
#include <hbinput12keytouchkeypad.h>
#include <hbinputsettingproxy.h>
#include <hbinpututils.h>
#include <hbinputsctportrait.h>
#include <hbinputeditorinterface.h>
#include <hbinputcandidatelist.h>
#include <hbinputpredictionfactory.h>
#include <hbinputpredictionengine.h>
#include <hbmainwindow.h>
#include <hbaction.h>
#include <hbview.h>
#include <hbinputvkbhost.h>
#include <hbinputcommondialogs.h>
#include <hbdeviceprofile.h>

//User includes
#include "hbinputthaispecialpopup.h"
#include "hbinputbasic12keyhandler.h"
#include "hbinputprediction12keyhandler.h"
#include "hbinputnumeric12keyhandler.h"
#include "hbinputbasic12keythaihandler.h"
#include "hbinputprediction12keythaihandler.h"

const int HbSmileyNumberOfRows = 5;
const int HbSmileyNumberOfColumns = 5;

/*!
\class HbVirtual12Key
\brief Input method implementations for virtual ITU-T mode in HbInputs framework.

It implements the HbInputMethod interface from HbInput framework. This particular
input method supports input of predictive and multi-tap input.

\sa HbInputMethod
*/

HbVirtual12Key::HbVirtual12Key()
              : mCurrentKeypad(0),
                mItutKeypad(0),
                mSctKeypad(0),
				mThaiSpecialChar(0),
                mKeymap(0),
                mSctMode(HbInputVkbWidget::HbSctViewSpecialCharacter),
                mOrientationAboutToChange(false),
                mCandidatePopup(0),
                mCurrentlyFocused(0),
                mVkbHost(0),
                mKeyboardChangeAlreadyInprogress(false)
{
    initializeModeHandlers();
}

void HbVirtual12Key::initializeModeHandlers()
{
    mBasicModeHandler = new HbInputBasic12KeyHandler(this);
    mPredictionModeHandler = new HbInputPrediction12KeyHandler(this);
    mNumericModeHandler = new HbInputNumeric12KeyHandler(this);
	mBasic12keyThaiHandler = new HbInputBasic12KeyThaiHandler(this);
	mPrediction12keyThaiHandler = new HbInputPrediction12KeyThaiHandler(this);
    mActiveModeHandler = mBasicModeHandler;

    mBasicModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionInit);
    mPredictionModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionInit);
    mNumericModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionInit);
	mPrediction12keyThaiHandler->actionHandler(HbInputModeHandler::HbInputModeActionInit);

    // let's connect prediction mode handler with latin basic mode handler. It is required incase we Qt::key_0 is pressed in prediction mode
    // key
    connect(mPredictionModeHandler, SIGNAL(passFilterEvent(const QKeyEvent *)), mBasicModeHandler, SLOT(filterEvent(const QKeyEvent *)));
    connect(mPredictionModeHandler, SIGNAL(passActionHandler(HbInputModeAction )), mBasicModeHandler, SLOT(actionHandler(HbInputModeAction )));
	connect(mPrediction12keyThaiHandler, SIGNAL(passFilterEvent(const QKeyEvent *)), mBasicModeHandler, SLOT(filterEvent(const QKeyEvent *)));
    connect(mPrediction12keyThaiHandler, SIGNAL(passActionHandler(HbInputModeAction )), mBasicModeHandler, SLOT(actionHandler(HbInputModeAction )));

}

bool HbVirtual12Key::isSctModeActive() const
{
    return (mCurrentKeypad == mSctKeypad);
}

/*!
Destructor for HbVirtual12Key. Responsible for clean up of the class.

\sa HbInputMethod::~HbInputMethod
*/
HbVirtual12Key::~HbVirtual12Key()
{
    delete mItutKeypad;
    mItutKeypad = 0;
    delete mSctKeypad;
    mSctKeypad = 0;
    delete mCandidatePopup;
    mCandidatePopup = 0;
	delete mThaiSpecialChar;
	mThaiSpecialChar = 0;

    // free mode handlers
    delete mBasicModeHandler;
    mBasicModeHandler = 0;
    delete mPredictionModeHandler;
    mPredictionModeHandler = 0;
    delete mNumericModeHandler;
    mNumericModeHandler = 0;
	delete mBasic12keyThaiHandler;
	mBasic12keyThaiHandler = 0;
	delete mPrediction12keyThaiHandler;
	mPrediction12keyThaiHandler = 0;
}

/*!
Implementation of the pure virtual QInputContext::identifierName()
Returns a string which specifies an identifier for the input method.
*/
QString HbVirtual12Key::identifierName()
{
    return QString("HbVirtual12Key");
}

/*!
Implementation of the pure virtual QInputContext::isComposing()
*/
bool HbVirtual12Key::isComposing() const
{
    return mActiveModeHandler->isComposing();
}

/*!
Implementation of the pure virtual QInputContext::language()
Returns the currently active language for the input method. The language
name returned is in the format language-code_country-code e.g. zh_CN.
*/
QString HbVirtual12Key::language()
{
    QLocale::Language activeLanguage;
    activeLanguage = (QLocale::Language)HbInputSettingProxy::instance()->globalInputLanguage().language();
    QLocale locale(activeLanguage);
    return locale.name();
}

/*!
Implementation of the pure virtual QInputContext::reset()
Resets the input method. This could be called by the widgets for reseting the
state of the input method.
*/
void HbVirtual12Key::reset()
{
    mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionReset);
    mOrientationAboutToChange = false;
}

/*!
Construct 12key touchkeypad and make the necessary connections.
*/
Hb12KeyTouchKeypad * HbVirtual12Key::construct12Keypad()
{
    Hb12KeyTouchKeypad * tempKeypad = 0;
    tempKeypad = new Hb12KeyTouchKeypad(this, 0);
    connect(tempKeypad, SIGNAL(keypadCloseEventDetected(HbInputVkbWidget::HbVkbCloseMethod)), this, SLOT(keypadCloseEventDetected(HbInputVkbWidget::HbVkbCloseMethod)));
    connect(tempKeypad, SIGNAL(rockerDirection(int, HbInputVirtualRocker::RockerSelectionMode)),
        this, SLOT(rockerDirection(int, HbInputVirtualRocker::RockerSelectionMode)));
    //FLICKDISABLED connect(tempKeypad, SIGNAL(flickEvent(HbInputVkbWidget::FlickDirection)), this, SLOT(flickEvent(HbInputVkbWidget::FlickDirection)));
    connect(tempKeypad, SIGNAL(smileySelected(QString)), this, SLOT(smileySelected(QString)));
    connect(tempKeypad, SIGNAL(mouseMovedOutOfButton()), this, SLOT(mouseMovedOutOfButton()));
	connect(tempKeypad, SIGNAL(chrSelected(QString)), this, SLOT(thaiSctCharacterSelected(QString)));
    tempKeypad->setRockerVisible(true);
    return tempKeypad;
}

/*!
Virtual 12-key specific implementation of HbInputMethod::focusReceived().
Up on reception of focus, the framework switches the input context and calls this
method indicating the gain of focus to the input method.
With focus received, the virtual 12-key input method, refreshes the special character
table as it needs to be based on the focused editor. It also opens the virtual keypad
and stops the timer which is used for multi tapping and detecting long key press events.
*/
void HbVirtual12Key::focusReceived()
{
    // load the new keymappings to all keypads and all mode handlers
    loadKeymap(inputState().language());
    // After loadKeyMapData call, mKeyData should have keymappings data of the current language
    if(!mKeymap) {
        return; // could not get keymappings. Does not make sense to continue
    }

    //Get vkbhost
    mVkbHost = focusObject()->editorInterface().vkbHost();

    if(!mItutKeypad){
        mItutKeypad = construct12Keypad();
        mItutKeypad->setKeymap(mKeymap);
    }

    if (!mItutKeypad) {
        // Something has caused keypad QPointers to zero themselves, no point to go on...
        return;
    }

    // inform active mode handler about the focusrecieve event.
    mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionFocusRecieved);

    if (mCurrentlyFocused != focusObject()) {
        mCurrentlyFocused = focusObject();
        // Focus changed from one editor to another.
        if (mCandidatePopup) {
            mCandidatePopup->hide();
        }
    }

    // We need to check if this focusRecieved call is due to a orientation
    // switch. If yes we should get the keypad status prior to the orientation
    // switch and open the keypad in that state only.
    // For example we have minimized the keypad in Qwerty mode and change the
    // orientation to portrait then in Itu-T mode also keypad should be in minimized state.
    if (orientationContextSwitchInProgress()) {
        HbVkbHost *host = focusObject()->editorInterface().vkbHost();
        if (host) {
            // We can get the keypad status prior to the orientation switch from vkbHost it self.
            HbVkbHost::HbVkbStatus vkbStatus = host->keypadStatusBeforeOrientationChange();
            if (vkbStatus != HbVkbHost::HbVkbStatusClosed) {
                openKeypad(mItutKeypad,vkbStatus == HbVkbHost::HbVkbStatusMinimized);
            }
        }
    } else {
        openKeypad(mItutKeypad);
    }

    if (focusObject() && mVkbHost) {
        connect(&(focusObject()->editorInterface()), SIGNAL(cursorPositionChanged(int, int)), mVkbHost, SLOT(ensureCursorVisibility()));
    }

    if (focusObject()) {
        disconnect(&(focusObject()->editorInterface()), SIGNAL(cursorPositionChanged(int, int)), mActiveModeHandler, SLOT(cursorPositionChanged(int, int)));
        connect(&(focusObject()->editorInterface()), SIGNAL(cursorPositionChanged(int, int)), mActiveModeHandler, SLOT(cursorPositionChanged(int, int)));
    }
}

/*!
\reimp
*/
void HbVirtual12Key::focusLost(bool focusSwitch)
{
    // inform the active mode handler about the focus lost event.
    mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionFocusLost);

    if (focusObject()) {
        disconnect(&(focusObject()->editorInterface()), SIGNAL(cursorPositionChanged(int, int)), mActiveModeHandler, SLOT(cursorPositionChanged(int, int)));
    }

    if (!focusSwitch && mVkbHost && mVkbHost->keypadStatus() != HbVkbHost::HbVkbStatusClosed) {
        // Context switch has happened but the keypad is still open.
        // Close it. First notify handler by sending an event and give it changes to
        // do any closing related things.
        // Then make sure that keypad closes.
        closeKeypad();
        mVkbHost = 0;
    }
}

/*!
Closes the keypad if it is open.
*/
void HbVirtual12Key::closeKeypad()
{
    if (mVkbHost && mVkbHost->keypadStatus() != HbVkbHost::HbVkbStatusClosed) {
        mVkbHost->closeKeypad(!stateChangeInProgress());
        // set mCurrentKeypad to null.
        mCurrentKeypad = 0;
		if(mThaiSpecialChar) {
			mThaiSpecialChar->hide();
		}
        if (mCandidatePopup) {
            mCandidatePopup->hide();
        }
    }
}

/*!
Opens the virtual keypad for input.
*/
void HbVirtual12Key::openKeypad(HbInputVkbWidget * keypadToOpen,bool inMinimizedMode)
{
    mKeyboardChangeAlreadyInprogress = true;
    HbInputSettingProxy::instance()->setActiveKeyboard(HbKeyboardVirtual12Key);
    mKeyboardChangeAlreadyInprogress = false;

    // if null is sent, just return.
    if(!keypadToOpen) {
        return;
    }
    bool wasKeypadOpen = false;
    // see if we are trying to open a different keypad than what is already opened.
    if (mCurrentKeypad != keypadToOpen) {
        // close currently open keypad. We always close keypad without animation
        // keypad should be closed with animation only when we loses focus and this is handled
        // in focusLost function call.
        if (mVkbHost && mVkbHost->keypadStatus() != HbVkbHost::HbVkbStatusClosed) {
            mVkbHost->closeKeypad(false);
            // when their is a keypad that needs to be closed before opening the new keypad, we don't
            // want to animate the opening of new keypad.
            wasKeypadOpen = true;
        }
    }
    // Close candidate popup if open
    if (mCandidatePopup) {
        mCandidatePopup->hide();
    }
    // assign new keypad to be opened to varable mCurrentKeypad
    mCurrentKeypad =  keypadToOpen;

    if (mVkbHost && mVkbHost->keypadStatus() != HbVkbHost::HbVkbStatusOpened) {
        connect(mVkbHost, SIGNAL(keypadClosed()), this, SLOT(keypadClosed()));
        connect(mVkbHost, SIGNAL(keypadOpened()), this, SLOT(keypadOpened()));

        if (inMinimizedMode) {
            mVkbHost->openMinimizedKeypad(mCurrentKeypad, this);
        } else {
            mVkbHost->openKeypad(mCurrentKeypad, this, (!stateChangeInProgress() && !wasKeypadOpen));
        }
        connect(&(focusObject()->editorInterface()), SIGNAL(cursorPositionChanged(int, int)), mVkbHost, SLOT(ensureCursorVisibility()));
    }
}

/*!
The mouse event handler for the input method. Launches the candidate list if the mouse is clicked on the
pre-editing text.
*/
void HbVirtual12Key::mouseHandler(int cursorPosition, QMouseEvent* mouseEvent)
{
    mActiveModeHandler->mouseHandler(cursorPosition, mouseEvent);
}

/*!
\deprecated HbVirtual12Key::predictiveInputStatusChanged(int newStatus)
    is deprecated. Use predictiveInputStateChanged instead.
*/
void HbVirtual12Key::predictiveInputStatusChanged(int newStatus)
{
    predictiveInputStateChanged(HbKeyboardSetting12key, newStatus);
}

/*!
The framework calls this method when the predictive input status changes.
*/
void HbVirtual12Key::predictiveInputStateChanged(HbKeyboardSettingFlags keyboardType, bool newStatus)
{
    Q_UNUSED(newStatus);

    if (keyboardType & HbKeyboardSetting12key) {
        HbInputFocusObject *focusedObject = focusObject();
        if (focusedObject) {
            // Just refresh the situation.
            inputStateActivated(inputState());
        }
    }
}

/*!
Call back indicating that the keypad is closed.
*/
void HbVirtual12Key::keypadClosed()
{
    if (mOrientationAboutToChange) {
        mOrientationAboutToChange = false;
    }
}

/*!
Call back indicating that the keypad is opened.
*/
void HbVirtual12Key::keypadOpened()
{
}

/*!
The call back from the VKB keypad widget before it closes the keypad.
*/
void HbVirtual12Key::keypadCloseEventDetected(HbInputVkbWidget::HbVkbCloseMethod vkbCloseMethod)
{
    Q_UNUSED(vkbCloseMethod);
    if (isActiveMethod()) {
        if (mVkbHost) {
            if (mVkbHost->keypadStatus() == HbVkbHost::HbVkbStatusOpened) {
                // We need to commit the inline word when we minimize the keypad
                mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionCommit);
                if (mCandidatePopup) {
                    mCandidatePopup->hide();
                }
                mVkbHost->minimizeKeypad(!stateChangeInProgress());
            }
        }
    }
}

/*!
The rocker widget emits the signal rockerDirection when there is any rocker movement. This method catches the emitted
signal and makes the cursor movements according to the rcker movement.
*/
void HbVirtual12Key::rockerDirection(int aDirection, HbInputVirtualRocker::RockerSelectionMode aSelectionMode)
{
    Qt::KeyboardModifiers modifiers = 0;
    if (aSelectionMode == HbInputVirtualRocker::RockerSelectionModeOn) {
        modifiers = Qt::ShiftModifier;
    }
    // commit any character/word which is in inline edit.
    mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionCommit);

    HbInputLanguage inputlang = HbInputSettingProxy::instance()->globalInputLanguage();

    switch (aDirection) {
    case HbInputVirtualRocker::HbRockerDirectionLeft:
        if(inputlang.isRightToLeftLanguage()) {
            focusObject()->cursorRight(modifiers);
        } else {
            focusObject()->cursorLeft(modifiers);
        }
        break;
    case HbInputVirtualRocker::HbRockerDirectionRight:
        if(inputlang.isRightToLeftLanguage()) {
            focusObject()->cursorLeft(modifiers);
        } else {
            focusObject()->cursorRight(modifiers);
        }
        break;
    case HbInputVirtualRocker::HbRockerDirectionUp: {
        QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Up, modifiers);
        focusObject()->sendEvent(keyEvent);
        }
        break;
    case HbInputVirtualRocker::HbRockerDirectionDown: {
        QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Down, modifiers);
        focusObject()->sendEvent(keyEvent);
        }
        break;
    default:
        break;
    }
}

/*!
Notification from the generic input method framework indicating a change of input language.
The VKB keypad needs to refreshed to show the renewed key mapping.
*/
void HbVirtual12Key::inputLanguageChanged(const HbInputLanguage &newLanguage)
{
    // inform all the mode handler about the language change.
    mPredictionModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionCommit);
    // move keypad off screen
    if (mCurrentKeypad){
        mCurrentKeypad->keypadLanguageChangeAnimationUpdate(0);
    }
    loadKeymap(newLanguage);
    // inform all the mode handler about the language change.
    mPredictionModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionPrimaryLanguageChanged);
    if (mCurrentKeypad){
        mCurrentKeypad->animKeyboardChange();
    }
}

/*!
Notification from the generic input method framework that there has been a flick event
*/
void HbVirtual12Key::flickEvent(HbInputVkbWidget::HbFlickDirection direction)
{
    Q_UNUSED(direction);
}


/*!
Notification from the generic input method framework that the mouse moved out of the button pressed.
*/
void HbVirtual12Key::mouseMovedOutOfButton()
{
    qDebug("HbVirtual12Key::mouseMovedOutOfButton()");
    mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionCancelButtonPress);
}

/*!
Notification from the generic input method framework to indicate the activation of a state.
*/
void HbVirtual12Key::inputStateActivated(const HbInputState& newState)
{
    if (!isActiveMethod()
        || mKeyboardChangeAlreadyInprogress) {
        return;  // Just to be sure...
    }

    if (mItutKeypad) {
        if (newState.inputMode() == HbInputModeNumeric) {
            // Numeric mode is activated, set numeric mode to itu-t keypad
                mItutKeypad->setMode(EModeNumeric, HbModifierNone);
        } else if (newState.inputMode() != HbInputModeNumeric) {
            if (newState.textCase() == HbTextCaseUpper || newState.textCase() == HbTextCaseAutomatic) {
                mItutKeypad->setMode(EModeAbc, HbModifierShiftPressed);
            } else {
                mItutKeypad->setMode(EModeAbc, HbModifierNone);
            }
        }
    }

    HbInputModeHandler *previousModeHandler = mActiveModeHandler;
	if(HbInputSettingProxy::instance()->globalInputLanguage() == QLocale::Thai && usePrediction() && newState.inputMode() != HbInputModeNumeric ) {
		mActiveModeHandler = mPrediction12keyThaiHandler;
		// by passing HbInputModeActionFocusRecieved we will be setting the candidate list and keypad
        mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionFocusRecieved);
	} else if (HbInputSettingProxy::instance()->globalInputLanguage() == QLocale::Thai && !usePrediction() && newState.inputMode() != HbInputModeNumeric 
		&& ((HbEditorConstraintLatinAlphabetOnly | HbEditorConstraintAutoCompletingField)!=focusObject()->editorInterface().constraints())) {
		mActiveModeHandler = mBasic12keyThaiHandler;
		// by passing HbInputModeActionFocusRecieved we will be setting the candidate list and keypad
        mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionFocusRecieved);
	} else if (newState.inputMode() == HbInputModeDefault && usePrediction()) {
        mActiveModeHandler = mPredictionModeHandler;
        // by passing HbInputModeActionFocusRecieved we will be setting the candidate list and keypad
        mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionFocusRecieved);
    } else if (newState.inputMode() == HbInputModeDefault) {
        mActiveModeHandler = mBasicModeHandler;
        // Auto completer setup needs following line.
        mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionFocusRecieved);
    } else if (newState.inputMode() == HbInputModeNumeric) {
        mActiveModeHandler = mNumericModeHandler;
    }

    if (focusObject()) {
        disconnect(&(focusObject()->editorInterface()), SIGNAL(cursorPositionChanged(int, int)), previousModeHandler, SLOT(cursorPositionChanged(int, int)));
        connect(&(focusObject()->editorInterface()), SIGNAL(cursorPositionChanged(int, int)), mActiveModeHandler, SLOT(cursorPositionChanged(int, int)));
    }

    // load the new keymappings to all keypads and all mode handlers
    loadKeymap(newState.language());

    // if there is a change in the modehandler we need send a commit in previous mode handler.
    if (previousModeHandler != mActiveModeHandler) {
        // Auto Completion part needs to be committed as well on mode change.

        previousModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionCommit);
        if (mActiveModeHandler == mPredictionModeHandler || mActiveModeHandler == mPrediction12keyThaiHandler) {
            // lets set candidate list and keypad type to the engine.
            mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionSetCandidateList);
            mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionSetKeypad);
        }
    }
}

/*!
This function loads the keymappings data of the given language to all the keypads and to all mode handlers.
It first checks if we already have keymappings data of given language in mKeyData, if not then gets the
keymappings,loads them to all avaialble keyboards and to all mode handlers
*/
void HbVirtual12Key::loadKeymap(const HbInputLanguage &newLanguage)
{
    //dont try to get the keymappings if we ( mKeyData) already have keymappings for newLanguage
    if(!mKeymap || mKeymap->language().language() != newLanguage.language()){
        const HbKeymap* keymap = HbKeymapFactory::instance()->keymap(newLanguage);
        if(keymap){
            mKeymap = keymap;
            if(mItutKeypad) {
                mItutKeypad->setKeymap(mKeymap);
            }
            if(mSctKeypad) {
                mSctKeypad->setKeymap(mKeymap);
            }

            // inform mode handlers about the language change.
            if(mBasicModeHandler) {
                mBasicModeHandler->setKeymap(mKeymap);
            }
            if(mPredictionModeHandler) {
                mPredictionModeHandler->setKeymap(mKeymap);
            }
            if(mNumericModeHandler) {
                mNumericModeHandler->setKeymap(mKeymap);
            } 
			if(mBasic12keyThaiHandler) {
				mBasic12keyThaiHandler->setKeymap(mKeymap);
			}
			if (mPrediction12keyThaiHandler) {
				mPrediction12keyThaiHandler->setKeymap(mKeymap);
			}
        }
    }
}

/*!
this slot is called by the mode handlers when there is Short Press of Asterisk key.
*/
void HbVirtual12Key::starKeySelected(){
    // if sct is active keypad, then launch itu-t keypad
    if (isSctModeActive()){
        switchToAlphaMode();
    } else {
        // launch Candidate List
        bool ret = mPredictionModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionLaunchCandidatePopup);
        // In case when candidate list is not launched i.e. when the word is not in inlline editing state, launch SCT
        if (!ret){
            mSctMode = HbInputVkbWidget::HbSctViewSpecialCharacter;
            displaySpecialCharacterTable(this);
        }
    }
}

/*!
this slot is called by the mode handlers when there is sym key or long  press of Asterisk key.
*/
void HbVirtual12Key::switchMode(int keyCode)
{
    if (keyCode == Qt::Key_Asterisk || keyCode == Qt::Key_Control){
        if (mCandidatePopup && mCandidatePopup->isVisible()) {
            return;
        }

        if (mCurrentKeypad == mSctKeypad){
            // if sct is active keypad, then launch itu-t keypad
            switchToAlphaMode();
        } else {
            // launch special character keypad
            mSctMode = HbInputVkbWidget::HbSctViewSpecialCharacter;
            displaySpecialCharacterTable(this);
        }
    } else if ( keyCode == Qt::Key_Shift) {
        HbInputState nextState = inputState();
        if (nextState.inputMode() != HbInputModeNumeric ) {
            nextState.setInputMode( HbInputModeNumeric );
        } else {
            editorRootState(nextState);
        }
        activateState(nextState);
        switchToAlphaMode();
    }
}
/*!
Returns previous character from editor
*/
uint HbVirtual12Key::previousChar()
{
	if(focusObject()) {
		int cursorPosition = focusObject()->editorCursorPosition();
		if(cursorPosition) {
			QString editorText = focusObject()->editorSurroundingText();
			return((editorText.at(cursorPosition-1)).unicode());
		}
	}
	return 0;
}
/*!
Shows the Thai specific special characters 
*/
void HbVirtual12Key::showThaiSpecialCharacters(uint buttonId)
{
	uint prevChar = previousChar();
	int screenWidth = 0;
	int screenHeight = 0;
	//Get the sceen size from device profile
	if(mCurrentKeypad) {
		screenWidth = HbDeviceProfile::profile(mCurrentKeypad).logicalSize().width();
		screenHeight = HbDeviceProfile::profile(mCurrentKeypad).logicalSize().height();
	}
	//Create Thai special popup if not created 
	if(!mThaiSpecialChar) {
		mThaiSpecialChar = new HbInputThaiSpecialPopup(buttonId,prevChar,0);
		connect(mThaiSpecialChar, SIGNAL(chrSelected(QString)), this, SLOT(thaiSctCharacterSelected(QString)));
	}
	//Set Geometry and Layout for popup 
	if (mThaiSpecialChar) {
		mThaiSpecialChar->setGeometry(QRectF(0,screenHeight/2,screenWidth,screenHeight/2));
		mThaiSpecialChar->setPopupLayout(mKeymap,prevChar,buttonId,screenWidth,screenHeight/2);
		mThaiSpecialChar->show();
	}
}

/*!
Shows the special character table. Re-implements the same method from HbInputMethod.
*/
int HbVirtual12Key::displaySpecialCharacterTable(QObject* receiver)
{
    Q_UNUSED(receiver);

    if(!mSctKeypad) {
        mSctKeypad = new HbInputSctPortrait(this,mKeymap,0);
        connect(mSctKeypad, SIGNAL(keypadCloseEventDetected(HbInputVkbWidget::HbVkbCloseMethod)), this, SLOT(keypadCloseEventDetected(HbInputVkbWidget::HbVkbCloseMethod)));
        connect(mSctKeypad, SIGNAL(sctCharacterSelected(QString)), this, SLOT(sctCharacterSelected(QString)));
        connect(mSctKeypad, SIGNAL(smileySelected(QString)), this, SLOT(smileySelected(QString)));
        mSctKeypad->setRockerVisible(false);
    }

    mSctKeypad->setSct(mSctMode);
    //Open the keypad
    openKeypad(mSctKeypad);

    return 0;
}

/*!
Switches the keypad to the alphabetic / normal input mode.
*/
void HbVirtual12Key::switchToAlphaMode( bool openAlphaInMinimizedMode)
{
    //Open the keypad
    openKeypad(mItutKeypad,openAlphaInMinimizedMode);
}

/*!
The call back from framework to indicate that the orientation is about to change. This closes the keypad
if it is already open.
*/
void HbVirtual12Key::orientationAboutToChange()
{
    HbInputMethod::orientationAboutToChange();
    if (isActiveMethod()) {
        mOrientationAboutToChange = true;
        mPredictionModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionCloseSpellQuery);
        // We need to commit the inline word before orientation change.
        mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionCommit);
        closeKeypad();
    }
}

/*!
Launches the candidate list.
*/
void HbVirtual12Key::launchCandidatePopup(const QStringList& candidates)
{
    if (!mCandidatePopup) {
        mCandidatePopup = new HbCandidateList(this);
    }
    mCandidatePopup->populateList(candidates);
    mCandidatePopup->setModal(true);

    QSizeF candListSize = mCandidatePopup->size();
    QPointF candListPos = mCandidatePopup->pos();
    getCandidatePositionAndSize(mCandidatePopup, mCurrentKeypad, candListPos,candListSize);

    QRectF geom = mCandidatePopup->geometry();
    geom.setHeight(candListSize.height());
    geom.setWidth(candListSize.width());
    mCandidatePopup->setGeometry(geom);

    mCandidatePopup->setPos(candListPos);

    mCandidatePopup->show();
}

/*!
 this slot is called when the candidate popup is closed
*/
void HbVirtual12Key::candidatePopupClosed(int closingKey)
{
    if (mCandidatePopup) {
        QString currentCandidate = mCandidatePopup->currentCandidate();
        if (currentCandidate.size() > 0) {
            if ((focusObject()->editorInterface().constraints() & HbEditorConstraintAutoCompletingField)) {
                mBasicModeHandler->autoCompletionPopupClosed(currentCandidate, closingKey);
            } else {
                mPredictionModeHandler->candidatePopupClosed(currentCandidate, closingKey);
            }
        }
    }
}

/*!
 this function is called whenever there is a hardware keypress Or virtual keypad button is pressed.
*/
bool HbVirtual12Key::filterEvent(const QEvent* event)
{
    return mActiveModeHandler->filterEvent(event);
}

void HbVirtual12Key::secondaryInputLanguageChanged(const HbInputLanguage &newLanguage)
{
    Q_UNUSED(newLanguage);
    mPredictionModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionSecondaryLanguageChanged);
}

/*!
 this slot is called by sct when a character is selected from sct.
*/
void HbVirtual12Key::sctCharacterSelected(QString character)
{
    mActiveModeHandler->sctCharacterSelected(character);
    /* Update the text case */
    updateState();
}

void HbVirtual12Key::smileySelected(QString smiley)
{
     mActiveModeHandler->smileySelected(smiley);
}

void HbVirtual12Key::thaiSctCharacterSelected(QString sctChar)
{
     mActiveModeHandler->sctCharacterSelected(sctChar);
}


void HbVirtual12Key::selectSpecialCharacterTableMode()
{
    if (mItutKeypad) {
        mItutKeypad->showSmileyPicker(HbSmileyNumberOfRows, HbSmileyNumberOfColumns);
    }
}

/*!
Slot used by mode handlers to close the candidate popup.
*/
void HbVirtual12Key::closeCandidatePopup()
{
    if (mCandidatePopup && mCandidatePopup->isVisible()) {
        mCandidatePopup->hide();
    }
}

/*!
Slot used by mode handlers to close the autocompletion popup.
*/
void HbVirtual12Key::closeAutoCompletionPopup()
{
    closeCandidatePopup();
}

/*!
Launches auto-completion popup if there are candidates available.
*/
void HbVirtual12Key::launchAutoCompletionPopup(const QStringList& candidates)
{
    if (!mCandidatePopup) {
        mCandidatePopup = new HbCandidateList(this);
    }

    if (candidates.count() > 0) {
        mCandidatePopup->populateList(candidates);
        mCandidatePopup->setModal(false);

        if (mCandidatePopup->setSizeAndPositionForAutoCompletion(mVkbHost)) {
            mCandidatePopup->setDismissPolicy(HbPopup::TapInside);
            mCandidatePopup->setBackgroundFaded(false);
            mCandidatePopup->show();
        }
    } else if (mCandidatePopup->isVisible()) {
        mCandidatePopup->hide();
    }
}

/*!
Returns true if prediction is on, prediction engine is available and predictions is allowed in current editor.
*/
bool HbVirtual12Key::usePrediction() const
{
    HbInputFocusObject *fo = focusObject();
    if (HbInputSettingProxy::instance()->predictiveInputStatus(HbKeyboardSetting12key) &&
        fo &&
        fo->editorInterface().isPredictionAllowed() &&
        mPredictionModeHandler->isActive() &&
        HbPredictionFactory::instance()->predictionEngineForLanguage(inputState().language())) {
         return true;           
    }

    return false;
}

/*!
This function returns true if the latest mouse release was part of a horizontal flick event
*/
HbInputVkbWidget::HbFlickDirection HbVirtual12Key::flickDirection() const
{
    if ( mCurrentKeypad ) {
        return mCurrentKeypad->flickDirection();
    }else{
        return HbInputVkbWidget::HbFlickDirectionNone;
    }
}
// End of file
