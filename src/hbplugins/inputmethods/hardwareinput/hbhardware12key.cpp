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

#include "hbhardware12key.h"

//Required Qt Headers
#include <QLocale>

//Required Hb Headers
#include <hbinputkeymapfactory.h>
#include <hbinputkeymapdata.h>
#include <hbinpututils.h>
#include <hbinputcommondialogs.h>
#include <hbinputsctportrait.h>
#include <hbinputeditorinterface.h>
#include <hbinputsettingproxy.h>
#include <hbinputpredictionfactory.h>
#include <hbinputpredictionengine.h>
#include <hbinputcandidatelist.h>
#include <hbinstance.h>
#include <hbmainwindow.h>
#include <hbaction.h>
#include <hbview.h>

//User Includes
#include "hbhardwareinputbasic12keyhandler.h"
#include "hbhardwareinputprediction12keyhandler.h"
#include "hbhardwareinputnumeric12keyhandler.h"

//const Declaration
const qreal HbAutoComplPopupLeftMargin = 15.0;
const qreal HbDeltaWidth = 12.0;

// ---------------------------------------------------------------------------
// HbHardware12key::HbHardware12key
//
// ---------------------------------------------------------------------------
//
HbHardware12key::HbHardware12key() :
mKeyData(0),
mInputMode(HbInputModeNone),
mSctKeypad(0),
mCandidatePopup(0),
mCurrentlyFocused(0),
mPrevInputState()
{

    // Assume that this will be the language we're going to use and cache it.
    mKeyData = HbKeyMapFactory::instance()->keymapDataForLanguage(HbInputSettingProxy::instance()->globalInputLanguage().language());

    if (!mKeyData) {
        qDebug("HbHardware12key: ERROR: Initialization FAILED!");     
    } 
    initializeModeHandlers();
}


void HbHardware12key::initializeModeHandlers()
{
    mBasicModeHandler = new HbHardwareInputBasic12KeyHandler(this);
    mPredictionModeHandler = new HbHardwareInputPrediction12KeyHandler(this);
    mNumericModeHandler = new HbHardwareInputNumeric12KeyHandler(this);
    mActiveModeHandler = mBasicModeHandler;

    mBasicModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionInit);
    mPredictionModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionInit);
    mNumericModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionInit);

    // let's connect prediction mode handler with latin basic mode handler. It is required incase we Qt::key_0 is pressed in prediction mode
    // key
    connect(mPredictionModeHandler, SIGNAL(passFilterEvent(const QKeyEvent *)), mBasicModeHandler, SLOT(filterEvent(const QKeyEvent *)));
    connect(mPredictionModeHandler, SIGNAL(passActionHandler(HbInputModeAction )), mBasicModeHandler, SLOT(actionHandler(HbInputModeAction )));

}


// ---------------------------------------------------------------------------
// HbHardware12key::~HbHardware12key
//
// ---------------------------------------------------------------------------
//
HbHardware12key::~HbHardware12key()
{
    delete mCandidatePopup;
    mCandidatePopup = 0;

    // Free mode handlers
    delete mBasicModeHandler;
    mBasicModeHandler = 0;
    delete mPredictionModeHandler;
    mPredictionModeHandler = 0;
    delete mNumericModeHandler;
    mNumericModeHandler = 0;
}


// ---------------------------------------------------------------------------
// HbHardware12key::identifierName
//
// ---------------------------------------------------------------------------
//
QString HbHardware12key::identifierName()
{
    return QString("HbHardware12key");
}


// ---------------------------------------------------------------------------
// HbHardware12key::isComposing
//
// ---------------------------------------------------------------------------
//
bool HbHardware12key::isComposing() const
{
    qDebug("HbHardware12key::isComposing");     
    return mActiveModeHandler->isComposing();
}


// ---------------------------------------------------------------------------
// HbHardware12key::language
//
// ---------------------------------------------------------------------------
//
QString HbHardware12key::language()
{
    QLocale::Language activeLanguage;
    activeLanguage = (QLocale::Language)HbInputSettingProxy::instance()->globalInputLanguage().language();
    QLocale locale(activeLanguage);
    return locale.name();
}


// ---------------------------------------------------------------------------
// HbHardware12key::reset
//
// ---------------------------------------------------------------------------
//
void HbHardware12key::reset()
{
    mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionReset);
}

/*!
The mouse event handler for the input method. Launches the candidate list if the mouse is clicked on the
pre-editing text.
*/
void HbHardware12key::mouseHandler(int cursorPosition, QMouseEvent* mouseEvent)
{
    qDebug("HbHardware12key::mouseHandler");
    mActiveModeHandler->mouseHandler(cursorPosition, mouseEvent);
}

void HbHardware12key::predictiveInputStatusChanged(int newStatus)
{   
    Q_UNUSED(newStatus);
    //HbInputFocusObject *focusedObject = 0;
    //focusedObject = focusObject();
    //if (!focusedObject) {
    //    qDebug("HbKeyboard12Key::predictiveInputStatusChanged focusObject() failed!!");
    //    return;
    //}

    //HbInputState inputState;
    //if (mInputMode != HbInputModeLatinPredictive) {
    //    mInputMode = HbInputModeLatinPredictive;
    //    inputState = HbInputState(HbInputModeLatinPredictive,
    //    HbTextCase(focusedObject->editorInterface().textCase()),
    //        HbKeyboard12Key, HbInputSettingProxy::instance()->globalInputLanguage());
    //    
    //} else {
    //    mInputMode = HbInputModeLatinBasic;
    //    inputState = HbInputState(HbInputModeLatinBasic,
    //    HbTextCase(focusedObject->editorInterface().textCase()),
    //        HbKeyboard12Key, HbInputSettingProxy::instance()->globalInputLanguage());

    //}
    //activateState(inputState);
    //focusedObject->editorInterface().setInputMode(mInputMode);
    //focusedObject->syncEditorInterface();
	HbInputFocusObject *focusedObject = focusObject();
    if (focusedObject) {
        // Just refresh the situation.
        inputStateActivated(inputState());   
        return;
    }     
}



/*!
Calculates the candidate list position.
*/
QPoint HbHardware12key::getCandidatePosition()
{
    qreal candListXPos = 0;
    qreal candListYPos = 0;
    HbMainWindow *mainWin = hbInstance->allMainWindows().at(0);
    if (mainWin) {
        HbView *currView = mainWin->currentView();

        QRectF mainRect = mainWin->sceneRect();
        QRectF editorRect = focusObject()->editorGeometry();
        QSizeF candListSize = mCandidatePopup->size();

        QRectF winRect = currView->windowFrameGeometry();
        QRectF cursorRect = focusObject()->inputMethodQuery(Qt::ImMicroFocus).toRectF();

        candListXPos = mainRect.x() + cursorRect.bottomRight().x() + HbDeltaWidth;
        candListYPos = winRect.y() + editorRect.y() + cursorRect.y()+ HbDeltaWidth;

        if((candListXPos + candListSize.width()) > mainRect.width()){
            // Doesn't fit in editor, so launch towards the left
            candListXPos = candListXPos - candListSize.width();
            if(candListSize.height() > winRect.height()/3) {
                // Position vertically at the bottom of the word
                candListYPos = candListYPos + candListSize.height()/2 + HbDeltaWidth;
            }
        }

        if(candListYPos  > (winRect.height()/2)){
            // Position vertically on the top of the word
            candListYPos = candListYPos - (candListSize.height() + HbDeltaWidth);
        }
    }
    return QPoint((int)candListXPos, (int)candListYPos);
}

/*!
Launches the candidate list.
*/
void HbHardware12key::launchCandidatePopup(QStringList * candidates)
{
    if (!mCandidatePopup) {
        mCandidatePopup = new HbCandidateList(this);
    }

    mCandidatePopup->populateList(*candidates);
    QPoint pos = getCandidatePosition();
    mCandidatePopup->setPos(pos);
    mCandidatePopup->show();
}

/*!
this slot is called when the candidate popup is closed
*/
void HbHardware12key::candidatePopupClosed(int closingKey)
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


// ---------------------------------------------------------------------------
// HbHardware12key::filterEvent
//
// ---------------------------------------------------------------------------
//
bool HbHardware12key::filterEvent(const QEvent *event)
{
    qDebug("HbHardware12key:filterEvent called, event type %d", event->type());
    return mActiveModeHandler->filterEvent(event);
}



// ---------------------------------------------------------------------------
// HbHardware12key::listInputModes
//
// ---------------------------------------------------------------------------
//
void HbHardware12key::listInputModes(QVector<HbInputModeProperties>& results)
{   
    mBasicModeHandler->listInputModes(results);
    mPredictionModeHandler->listInputModes(results);
    mNumericModeHandler->listInputModes(results);
}


// ---------------------------------------------------------------------------
// HbHardware12key::focusReceived
//
// ---------------------------------------------------------------------------
//
void HbHardware12key::focusReceived()
{
    qDebug("HbHardware12key::focusReceived");   
    // Inform active mode handler about the focusrecieve event.
    mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionFocusRecieved);
}   


// ---------------------------------------------------------------------------
// HbHardware12key::focusLost
//
// ---------------------------------------------------------------------------
//
void HbHardware12key::focusLost(bool focusSwitch)
{
    Q_UNUSED(focusSwitch);

    qDebug("HbHardware12key::focusLost");     
    // inform the active mode handler about the focus lost event.
    mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionFocusLost);
}   


// ---------------------------------------------------------------------------
// HbHardware12key::sctCharacterSelected
//
// ---------------------------------------------------------------------------
//
void HbHardware12key::sctCharacterSelected(QChar character)
{
    qDebug("HbHardware12key::sctCharacterSelected");
    mActiveModeHandler->sctCharacterSelected(character);
}


// ---------------------------------------------------------------------------
// HbHardware12key::InputLanguageChanged
//
// ---------------------------------------------------------------------------
//      
void HbHardware12key::inputLanguageChanged(int newLanguage)
{
    qDebug("HbHardware12key::InputLanguageChanged");    

    mKeyData = HbKeyMapFactory::instance()->keymapDataForLanguage(newLanguage);
    if (!mKeyData) {
        qDebug("HbHardware12key: ERROR: Language switch FAILED for language %d", newLanguage);
    }    
    // let's inform all the mode handlers about the language change.
    mBasicModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionPrimaryLanguageChanged);
    mPredictionModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionPrimaryLanguageChanged);

}  


// ---------------------------------------------------------------------------
// HbHardware12key::inputStateActivated
//
// ---------------------------------------------------------------------------
//     
void HbHardware12key::inputStateActivated(const HbInputState& newState)
{    
    if (!isActiveMethod()) {
        return;  // Just to be sure...
    }
    // TODO: Switch editing indicator here....
    //     SetEditingIndicator(HbEditingIndicatorLatinMultitap);

    HbInputLanguage lang = activeLanguage();
    if (!mKeyData || mKeyData->languageCode() != lang.language()) {
        mKeyData = HbKeyMapFactory::instance()->keymapDataForLanguage(lang.language());
    }

    if (focusObject()) {    
        focusObject()->syncEditorInterface();
    }
    HbInputModeHandler *previousModeHandler = mActiveModeHandler;
    if (newState.inputMode() == HbInputModeDefault && usePrediction()) {
        mActiveModeHandler = mPredictionModeHandler;
        // by passing HbInputModeActionFocusRecieved we will be setting the candidate list and keypad
        mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionFocusRecieved);
    } else if (newState.inputMode() == HbInputModeDefault) {
        mActiveModeHandler = mBasicModeHandler;
    } else if (newState.inputMode() == HbInputModeNumeric) {
        mActiveModeHandler = mNumericModeHandler;
    }

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
}

void HbHardware12key::switchMode(int keyCode)
{
    if (keyCode == Qt::Key_Asterisk){
        //TO-DO SCT part
        //....
    } else if ( keyCode == Qt::Key_Shift) {
        HbInputState inputState = this->inputState();
        if (inputState.inputMode() != HbInputModeNumeric ){
            mPrevInputState = inputState;
            inputState.inputMode() = HbInputModeNumeric;
            this->activateState(inputState);
        } else {
            this->activateState( mPrevInputState );
        }
        mInputMode = this->inputState().inputMode();      
   }
}

// ---------------------------------------------------------------------------
// HbHardware12key::switchSpecialCharacterTable
//
// ---------------------------------------------------------------------------
//
int HbHardware12key::switchSpecialCharacterTable()
{
    switchToSctMode();
    //Q_UNUSED(receiver);
    return 0;
}

/*!
Switches the virtual keypad to the SCT input mode. It refreshes the keypad with special characters.
*/
void HbHardware12key::switchToSctMode()
{
    //TO-DO SCT part
    //....
}

/*!
Sets the special character table with the required characters.
*/
void HbHardware12key::setSpecialCharacters()
{
    QString charSet;
#if 0 /* code commented as numeric SCT set is added to the default list */
    if (focusObject()->editorInterface().filter() & HbNumberModeFilterMask) {
        charSet = mKeyData->numericModeSpecialCharacterData(HbKeyboardVirtual12Key);
    } else {
        charSet = mKeyData->specialCharacterData(HbKeyboardVirtual12Key);
    }

    charSet = mKeyData->specialCharacterData(HbKeyboard12Key);

    QString filtered;
    if (focusObject()) {
        focusObject()->filterStringWithEditorFilter(charSet, filtered);
    } else {
        filtered = charSet;
    }

    if (filtered.size() > 0) {
        mSctKeypad->setCharSet(filtered, focusObject()->editorInterface().filter());
    }
#endif
}


/*!
Slot used by mode handlers to close the candidate popup.
*/
void HbHardware12key::closeCandidatePopup()
{
    if (mCandidatePopup && mCandidatePopup->isVisible()) {
        mCandidatePopup->hide();
    }
}

/*!
Slot used by mode handlers to close the autocompletion popup.
*/
void HbHardware12key::closeAutoCompletionPopup()
{
    closeCandidatePopup();
}

void HbHardware12key::launchAutoCompletionPopup(QStringList* candidates)
{
    if (!mCandidatePopup) {
        mCandidatePopup = new HbCandidateList(this);
    }

    if (candidates) {
        if (candidates->count() > 0) {
            mCandidatePopup->populateList(*candidates);
            mCandidatePopup->setModal(false);

            if (focusObject()) {
                QRectF editorGeom = focusObject()->editorGeometry();
                mCandidatePopup->setPos(QPointF(HbAutoComplPopupLeftMargin, editorGeom.top() - mCandidatePopup->size().height()));
            }

            // Set width. Currently done like this, later it can be changed to take the string lengths
            // into account too.
            HbMainWindow *mainWin = hbInstance->allMainWindows().at(0);
            if (mainWin) {
                QRectF mainRect = mainWin->sceneRect();
                QSizeF candListSize = mCandidatePopup->size();
                candListSize.setWidth(mainRect.width() - (HbAutoComplPopupLeftMargin * 2.0));
                mCandidatePopup->resize(candListSize);
            }

			mCandidatePopup->setBackgroundFaded(false);
            mCandidatePopup->show();
        } else if (mCandidatePopup->isVisible()) {
            mCandidatePopup->hide();
        }
    }
}

/*!
Returns true if prediction is on, prediction engine is available and predictions is allowed in current editor.
*/
bool HbHardware12key::usePrediction() const
{
    HbInputFocusObject *fo = focusObject();
    if (HbInputSettingProxy::instance()->predictiveInputStatus() &&
        fo &&
        fo->editorInterface().isPredictionAllowed() &&
        mPredictionModeHandler->isActive()) {
         return true;           
    }

    return false;
}

// End of file

