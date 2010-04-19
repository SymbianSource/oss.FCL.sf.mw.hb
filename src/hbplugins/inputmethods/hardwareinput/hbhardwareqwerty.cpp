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

#include <QApplication>
#include <QLocale>
#include <hbinputkeymapfactory.h>
#include <hbinputcommondialogs.h>


#include "hbhardwareqwerty.h"
#include "hbinputkeymapdata.h"
#include "hbinpututils.h"
#include "hbinputsettingproxy.h"
#include "hbinputfocusobject.h"
#include "hbinputexactwordpopup.h"
#include "hbinputcandidatelist.h"
#include "hbinputcharpreviewpane.h"
#include "hbhardwareinputbasicqwertyhandler.h"
#include "hbhardwareinputpredictionqwertyhandler.h"
#include "hbhardwareinputnumericqwertyhandler.h"

// ---------------------------------------------------------------------------
// HbHardwareQwerty::HbHardwareQwerty
//
// ---------------------------------------------------------------------------
//
HbHardwareQwerty::HbHardwareQwerty() :  mKeyData(0),
                                        mExactWordPopup(0),
                                        mCandidatePopup(0),
                                        mPreviewPane(0),
                                        mMode(HbInputModeDefault),
                                        mKeypad(0),
                                        mVkbHost(0)

{
    // Assume that this will be the language we're going to use and cache it.
    mKeyData = HbKeyMapFactory::instance()->keymapDataForLanguage(HbInputSettingProxy::instance()->globalInputLanguage().language());

    if (!mKeyData) {
        qDebug("HbHardwareQwerty: ERROR: Initialization FAILED!");     
    } 
    initializeModeHandlers();
}

void HbHardwareQwerty::initializeModeHandlers()
{
    // lets construct all the three supported mode handlers.
    mBasicModeHandler = new HbHardwareInputBasicQwertyHandler(this);
    mPredictionModeHandler = new HbHardwareInputPredictionQwertyHandler(this);
    mNumericModeHandler = new HbHardwareInputNumericQwertyHandler(this);
    // bydefault latin basic mode handler is activated.
    mActiveModeHandler = mBasicModeHandler;

    // init will initialize the individual mode handlers.
    mBasicModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionInit);
    mPredictionModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionInit);
    mNumericModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionInit);

    // exact word popup connections.
    connect(this , SIGNAL(flipStatusChange()), HbInputSettingProxy::instance(), SLOT(flipToggle()));    
}

// ---------------------------------------------------------------------------
// HbHardwareQwerty::~HbHardwareQwerty
//
// ---------------------------------------------------------------------------
//
HbHardwareQwerty::~HbHardwareQwerty()
{
    delete mCandidatePopup;
    mCandidatePopup = 0;
    
    delete mExactWordPopup;
    mExactWordPopup = 0;

    // free mode handlers
    delete mBasicModeHandler;
    mBasicModeHandler = 0;
    delete mPredictionModeHandler;
    mPredictionModeHandler = 0;
    delete mNumericModeHandler;
    mNumericModeHandler = 0;
    delete mPreviewPane;
    mPreviewPane = 0;
    delete mKeypad;
    mKeypad = 0;
}


// ---------------------------------------------------------------------------
// HbHardwareQwerty::identifierName
//
// ---------------------------------------------------------------------------
//
QString HbHardwareQwerty::identifierName()
{
    return QString("HbHardwareQwerty");
}


// ---------------------------------------------------------------------------
// HbHardwareQwerty::isComposing
//
// ---------------------------------------------------------------------------
//
bool HbHardwareQwerty::isComposing() const
{
    return mActiveModeHandler->isComposing();
}


// ---------------------------------------------------------------------------
// HbHardwareQwerty::language
//
// ---------------------------------------------------------------------------
//
QString HbHardwareQwerty::language()
{
    return QString();
}


// ---------------------------------------------------------------------------
// HbHardwareQwerty::reset
//
// ---------------------------------------------------------------------------
//
void HbHardwareQwerty::reset()
{
    mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionReset);
}


// ---------------------------------------------------------------------------
// HbHardwareQwerty::filterEvent
//
// ---------------------------------------------------------------------------
//
bool HbHardwareQwerty::filterEvent(const QEvent *event)
{
	const QKeyEvent *keyEvent = 0;
    keyEvent = static_cast<const QKeyEvent *>(event);
	if(keyEvent->key() == Qt::Key_F8){
		emit flipStatusChange();
		return false;
	}
    if(handleEvent()){
        return mActiveModeHandler->filterEvent(event);
    } 
    return false;
}

// ---------------------------------------------------------------------------
// HbHardwareQwerty::listInputModes
//
// ---------------------------------------------------------------------------
//
void HbHardwareQwerty::listInputModes(QVector<HbInputModeProperties>& results)
{       
        // list out each mode handlers.
    mBasicModeHandler->listInputModes(results);
    mPredictionModeHandler->listInputModes(results);
    mNumericModeHandler->listInputModes(results);
}


// ---------------------------------------------------------------------------
// HbHardwareQwerty::focusReceived
//
// ---------------------------------------------------------------------------
//
void HbHardwareQwerty::focusReceived()
{    
        // inform active mode handler about the focusrecieve event.
    mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionFocusRecieved);

    HbInputLanguage lang = activeLanguage();
    if (!mKeyData ||  mKeyData->languageCode() != lang.language()) {
        mKeyData = HbKeyMapFactory::instance()->keymapDataForLanguage(lang.language());
    }
    if (focusObject()) {    
        focusObject()->syncEditorInterface();
    }
    
    if(!mKeypad) {
        mKeypad = constructKeypad();
    }
    // We need to check if this focusRecieved call is due to a orientation 
    // switch. If yes we should get the keypad status prior to the orientation
    // switch and open the keypad in that state only.
    // For example we have minimized the keypad in Qwerty mode and change the 
    // orientation to portrait then in Itu-T mode also keypad should be in minimized state.
    if (orientationContextSwitchInProgress()) {
        HbVkbHost *host = focusObject()->findVkbHost();
        if (host) {
            // We can get the keypad status prior to the orientation switch from vkbHost itself.
            HbVkbHost::HbVkbStatus vkbStatus = host->keypadStatusBeforeOrientationChange();
            if (vkbStatus != HbVkbHost::HbVkbStatusClosed) {
                openKeypad(vkbStatus == HbVkbHost::HbVkbStatusMinimized);
            }
        }
    } else {
        openKeypad();
    }
    
    //This may not be required with the generic solution for all focusing problems
    //But, let it be there for the time being.
    if (focusObject()) {
        connect(&(focusObject()->editorInterface()), SIGNAL(cursorPositionChanged(int, int)), mVkbHost, SLOT(ensureCursorVisibility()));
    }
    qDebug("HbHardwareQwerty::focusReceived");  
}   


// ---------------------------------------------------------------------------
// HbHardwareQwerty::focusLost
//
// ---------------------------------------------------------------------------
//
void HbHardwareQwerty::focusLost(bool focusSwitch)
{
    Q_UNUSED(focusSwitch);
    mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionFocusLost);
    if (!focusSwitch) {
        if (mVkbHost && mVkbHost->keypadStatus() != HbVkbHost::HbVkbStatusClosed) {
            // Context switch has happened but the keypad is still open.
            // Close it.
            closeKeypad();
        }
    }
}   

// ---------------------------------------------------------------------------
// HbHardwareQwerty::mouseHandler
//
// ---------------------------------------------------------------------------
//
void HbHardwareQwerty::mouseHandler(int x, QMouseEvent* event)
{
    mActiveModeHandler->mouseHandler(x, event);
}



// ---------------------------------------------------------------------------
// HbHardwareQwerty::sctCharacterSelected
//
// ---------------------------------------------------------------------------
//
void HbHardwareQwerty::sctCharacterSelected(QChar character)
{
    // Needs to be handled based on how SCT is implemented (on keypad or on dialog)
    Q_UNUSED(character)
    qDebug("HbVirtual12Key::sctCharacterSelected");
}


// ---------------------------------------------------------------------------
// HbHardwareQwerty::InputLanguageChanged
//
// ---------------------------------------------------------------------------
//       
void HbHardwareQwerty::InputLanguageChanged(int newLanguage)
{
    qDebug("HbHardwareQwerty::InputLanguageChanged");   
    mKeyData = HbKeyMapFactory::instance()->keymapDataForLanguage(newLanguage);
    if (!mKeyData) {
        qDebug("HbHardwareQwerty: ERROR: Language switch FAILED for language %d", newLanguage);
    }

    // inform mode handlers about the language change.
    mBasicModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionPrimaryLanguageChanged);
    mPredictionModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionPrimaryLanguageChanged);
}  


// ---------------------------------------------------------------------------
// HbHardwareQwerty::inputStateActivated
//
// ---------------------------------------------------------------------------
//     
void HbHardwareQwerty::inputStateActivated(const HbInputState& newState)
{    
    // TODO: Switch editing indicator here....
    //     SetEditingIndicator(HbEditingIndicatorLatinMultitap);
    qDebug("inputStateActivated");
    HbInputLanguage lang = activeLanguage();
    if (!mKeyData || mKeyData->languageCode() != lang.language()) {
        mKeyData = HbKeyMapFactory::instance()->keymapDataForLanguage(lang.language());
    }
        HbInputModeHandler *previousModeHandler = mActiveModeHandler;
    if (newState.inputMode() == HbInputModeNumeric) {
        mActiveModeHandler = mNumericModeHandler;
    } else if (newState.inputMode() == HbInputModeDefault && usePrediction()) {
        mActiveModeHandler = mPredictionModeHandler;
    } else if (newState.inputMode() == HbInputModeDefault) {        
        mActiveModeHandler = mBasicModeHandler;
    } 
    //handle character preview pane logic here
    //Handle connection to character preview popup selected signal  
    
    // if there is a change in the modehandler we need send a commit in previous mode handler.
    if (previousModeHandler != mActiveModeHandler) {
        if (previousModeHandler == mPredictionModeHandler) {
            previousModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionHideTail);
        }
        previousModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionCommit);
        // lets set candidate list and keypad type to the engine.
        mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionSetCandidateList);
        mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionSetKeypad);
    }
    if (focusObject()) {    
        focusObject()->syncEditorInterface();
    }
}

// ---------------------------------------------------------------------------
// HbHardwareQwerty::switchSpecialCharacterTable
//
// ---------------------------------------------------------------------------
//
void HbHardwareQwerty::switchSpecialCharacterTable()
{
    displaySpecialCharacterTable(this);
}

// ---------------------------------------------------------------------------
// HbHardwareQwerty::displaySpecialCharacterTable
//
// ---------------------------------------------------------------------------
//
int HbHardwareQwerty::displaySpecialCharacterTable(QObject* receiver)
{
    Q_UNUSED(receiver)
    return 0;
}


/*!
This function is called during a long key press
for a long time.
*/
void HbHardwareQwerty::launchCharacterPreviewPane(int key)
{
    if(!mPreviewPane){
    mPreviewPane = new HbCharPreviewPane();
    }
    // get the characters bound to the key.
    QStringList spellList;
    mActiveModeHandler->getAndFilterCharactersBoundToKey(spellList, static_cast<Qt::Key> (key));
    
    qDebug("previrew request..!");
    if (spellList.size()) {
        // preview pane should show the correct case.
        int currentTextCase = focusObject()->editorInterface().textCase();
        for(int i = 0; i < spellList.size(); i++) {
            if (currentTextCase == HbTextCaseLower) {
                spellList[i] = spellList.at(i).toLower();
            } else {
                spellList[i] = spellList.at(i).toUpper();
            }
        }
//        mPreviewPane->showCharacters(spellList, getCursorCoordinatePosition());
    }
}

/*!
Launches the candidate list with the passed candidates.
*/
void HbHardwareQwerty::launchCandidatePopup(QStringList *candidates)
{
    //before launching candidate popup, close exact word popup if visible.
    closeExactWordPopup();
    if (!mCandidatePopup) {
        mCandidatePopup = new HbCandidateList(this);
    }
    mCandidatePopup->populateList(*candidates);
    mCandidatePopup->show();
}

/*!
Commits the candidate upon closing of the candidate list.
*/
void HbHardwareQwerty::candidatePopupClosed(int closingKey)
{
    if (mCandidatePopup) {
        QString currentCandidate = mCandidatePopup->currentCandidate();
        if (currentCandidate.size() > 0) {
            mPredictionModeHandler->candidatePopupClosed(currentCandidate, closingKey);
        }
    }
}


void HbHardwareQwerty::predictiveInputStatusChanged(int newStatus)
{
    //Q_UNUSED(newStatus);
    //HbInputFocusObject *focusedObject = 0;
    //focusedObject = focusObject();
    //if (!focusedObject) {
    //    qDebug("HbHardwareQwerty::predictiveInputStatusChanged focusObject() failed!!");
    //    return;
    //}

    //HbInputState inputState;
    //if (mMode == HbInputModeDefault) {
    //    mMode = HbInputModeLatinPredictive;
    //    inputState = HbInputState(HbInputModeLatinPredictive,
    //        HbTextCase(focusedObject->editorInterface().textCase()),
    //        HbInputSettingProxy::instance()->activeHwKeyboard(), HbInputSettingProxy::instance()->globalInputLanguage());
    //} else {
    //    inputState = HbInputState(HbInputModeLatinBasic,
    //        HbTextCase(focusedObject->editorInterface().textCase()),
    //        HbInputSettingProxy::instance()->activeHwKeyboard(), HbInputSettingProxy::instance()->globalInputLanguage());
    //    mMode = HbInputModeLatinBasic;
    //}
    //activateState(inputState);
    //focusedObject->editorInterface().setInputMode(mMode);
    //focusedObject->syncEditorInterface();
	Q_UNUSED(newStatus);

//    HbInputFocusObject *focusedObject = focusObject();
    /*if (focusedObject)*/ {
        // Just refresh the situation.
        inputStateActivated(inputState());   
    }     

}


/*!
Slot used by mode handlers to set the keypad modifiers
*/
void HbHardwareQwerty::launchExactWordPopup(QString exactWord)
{
    if (!mExactWordPopup) {
        mExactWordPopup = HbExactWordPopup::instance();
        connect(mExactWordPopup, SIGNAL(exactWordSelected()), mPredictionModeHandler, SLOT(exactWordPopupClosed()));
    }
    mExactWordPopup->setText(exactWord);
    mExactWordPopup->showText(getCursorCoordinatePosition());
}

/*!
Slot for hiding the exact word popup.
*/
void HbHardwareQwerty::closeExactWordPopup()
{
    if (mExactWordPopup && mExactWordPopup->isVisible()) {
        mExactWordPopup->hide();
    }
}


QPointF HbHardwareQwerty::getCursorCoordinatePosition()
{
    QRectF microRect = focusObject()->microFocus();
    return microRect.topLeft();
}

/*!
Call back indicating that the keypad is closed.
*/
void HbHardwareQwerty::keypadClosed()
{
}

/*!
Call back indicating that the keypad is opened.
*/
void HbHardwareQwerty::keypadOpened()
{
}

/*!
The call back from the VKB keypad widget before it closes the keypad.
*/
void HbHardwareQwerty::keypadCloseEventDetected(HbInputVkbWidget::HbVkbCloseMethod vkbCloseMethod)
{
    if (isActiveMethod()) {
        // A drag on the button meaning we need to delete character and commit.
        if (mKeypad && vkbCloseMethod == HbInputVkbWidget::HbVkbCloseMethodButtonDrag) {
            mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionDeleteAndCommit); 
        }

        if (mVkbHost && mVkbHost->keypadStatus() != HbVkbHost::HbVkbStatusMinimized) {
            mVkbHost->minimizeKeypad(false);
            if (mCandidatePopup) {
                mCandidatePopup->hide();
            }
        }
    }
}

/*!
Closes the keypad if it is open.
*/
void HbHardwareQwerty::closeKeypad()
{
    if (mVkbHost && mVkbHost->keypadStatus() != HbVkbHost::HbVkbStatusClosed) {
        mVkbHost->closeKeypad(!stateChangeInProgress());
        if (mCandidatePopup) {
            mCandidatePopup->hide();
        }
    }
}

/*!
The call back from framework to indicate that the orientation is about to change. This closes the keypad
if it is already open.
*/
void HbHardwareQwerty::orientationAboutToChange()
{
    HbInputMethod::orientationAboutToChange();
    closeKeypad();
}

void HbHardwareQwerty::openKeypad(bool inMinimizedMode)
{
    mKeypad->createLayout();
    
    mVkbHost = focusObject()->findVkbHost();

    if (mVkbHost && mVkbHost->keypadStatus() != HbVkbHost::HbVkbStatusOpened) {
        connect(mVkbHost, SIGNAL(keypadClosed()), this, SLOT(keypadClosed()));
        connect(mVkbHost, SIGNAL(keypadOpened()), this, SLOT(keypadOpened()));
        
        if (inMinimizedMode) {
            mVkbHost->openMinimizedKeypad(mKeypad, this);
        } else {
            mVkbHost->openKeypad(mKeypad, this, false);
        }
        connect(&(focusObject()->editorInterface()), SIGNAL(cursorPositionChanged(int, int)), mVkbHost, SLOT(ensureCursorVisibility()));
    }
}

HbHwToolCluster* HbHardwareQwerty::constructKeypad()
{

    HbHwToolCluster* keypad = new HbHwToolCluster(this, 0);
    connect(keypad, SIGNAL(keypadCloseEventDetected(HbInputVkbWidget::HbVkbCloseMethod)),
        this, SLOT(keypadCloseEventDetected(HbInputVkbWidget::HbVkbCloseMethod)));
    return keypad;
}

/*!
Returns true if prediction is on, prediction engine is available and predictions is allowed in current editor.
*/
bool HbHardwareQwerty::usePrediction() const
{
	qDebug("in usePrediction()");
    HbInputFocusObject *fo = focusObject();
	if(!fo){
		qDebug("NO Focus object");
	} else {
		qDebug("Yes Focus object");
	}
	
	if (!HbInputSettingProxy::instance()->predictiveInputStatus()){
		qDebug("No HbInputSettingProxy::instance()->predictiveInputStatus()");
	} else {
		qDebug("Yes HbInputSettingProxy::instance()->predictiveInputStatus()");	
	}
	
	if(!mPredictionModeHandler->isActive()){
		qDebug("No mPredictionModeHandler->isActive()");

	} else {
		qDebug("Yes mPredictionModeHandler->isActive()");
	}


    if (HbInputSettingProxy::instance()->predictiveInputStatus() &&
        fo &&
        fo->editorInterface().isPredictionAllowed() &&
        mPredictionModeHandler->isActive()) {
         return true;           
    }

    return false;
}
// End of file

