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
#include "virtualqwerty.h"
#include <hbapplication.h>
#include <hbaction.h>
#include <hbview.h>
#include <hbmainwindow.h>
#include <QLocale>

#include <hbinputexactwordpopup.h>
#include <hbinputkeymapfactory.h>
#include <hbinputkeymap.h>
#include <hbinputcandidatelist.h>
#include <hbinputsettingproxy.h>
#include <hbinpututils.h>
#include <hbinputsctkeyboard.h>
#include <hbinputeditorinterface.h>
#include <hbinputdef.h>
#include <hbinputvkbhost.h>
#include <hbinputcommondialogs.h>
#include <hbinputpredictionfactory.h>

#include "hbinputbasicqwertyhandler.h"
#include "hbinputpredictionqwertyhandler.h"
#include "hbinputnumericqwertyhandler.h"
#include "hbinputqwerty10x4touchkeyboard.h"
#include "hbinputqwerty11x4touchkeyboard.h"
#include "hbinputqwertynumerictouchkeyboard.h"

const int HbVirtualQwerty4x10MaxKeysCount = 32;
const qreal HbExactPopupHeightFactor = 0.15;
const qreal HbDeltaWidth = 12.0;
const qreal HbEditorExtraMargin = 5.0;

HbVirtualQwerty::HbVirtualQwerty() : mCurrentKeypad(0),
                                     mQwertyAlphaKeypad(0),
                                     mQwerty10x4Keypad(0),
                                     mQwerty11x4Keypad(0),
                                     mQwertyNumericKeypad(0),
                                     mSctKeypad(0),
                                     mKeymap(0),
                                     mExactWordPopup(0),
                                     mCandidatePopup(0),
                                     mShiftKeyState(0),
                                     mVkbHost(0)
{
    initializeModeHandlers();
}

void HbVirtualQwerty::initializeModeHandlers()
{
    // lets construct all the three supported mode handlers.
    mBasicModeHandler = new HbInputBasicQwertyHandler(this);
    mPredictionModeHandler = new HbInputPredictionQwertyHandler(this);
    mNumericModeHandler = new HbInputNumericQwertyHandler(this);
    // bydefault latin basic mode handler is activated.
    mActiveModeHandler = mBasicModeHandler;

    // init will initialize the individual mode handlers.
    mBasicModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionInit);
    mPredictionModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionInit);
    mNumericModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionInit);

    // autocompleter connection
    connect(this, SIGNAL(autoCompletionPopupClosed(QString, int)), mBasicModeHandler, SLOT(autoCompletionPopupClosed(QString, int)));

    connect(HbInputSettingProxy::instance(), SIGNAL(predictiveInputStateChanged(HbKeyboardSettingFlags,bool)), this, SLOT(predictiveInputStateChanged(HbKeyboardSettingFlags,bool)));
    connect(HbInputSettingProxy::instance(), SIGNAL(primaryCandidateModeChanged(HbPrimaryCandidateMode)), this, SLOT(primaryCandidateModeChanged(HbPrimaryCandidateMode)));
    connect(HbInputSettingProxy::instance(), SIGNAL(autocompletionStateChanged(HbKeyboardSettingFlags,bool)), this, SLOT(autocompletionStateChanged(HbKeyboardSettingFlags,bool)));
    connect(HbInputSettingProxy::instance(), SIGNAL(typingCorrectionLevelChanged(HbTypingCorrectionLevel)), this, SLOT(typingCorrectionLevelChanged(HbTypingCorrectionLevel)));
    mPredictionModeHandler->setPrimaryCandidateMode(HbInputSettingProxy::instance()->primaryCandidateMode());
    mPredictionModeHandler->setAutocompletionStatus(HbInputSettingProxy::instance()->isAutocompletionEnabled(HbKeyboardSettingQwerty));
  
}

// ---------------------------------------------------------------------------
// HbVirtualQwerty::~HbVirtualQwerty
//
// ---------------------------------------------------------------------------
//
HbVirtualQwerty::~HbVirtualQwerty()
{
    delete mCandidatePopup;
    mCandidatePopup = 0;

    delete mQwertyAlphaKeypad;
    mQwertyAlphaKeypad = 0;

    delete mQwerty10x4Keypad;
    delete mQwerty11x4Keypad;

    delete mQwertyNumericKeypad;
    mQwertyNumericKeypad = 0;

    delete mSctKeypad;
    mSctKeypad = 0;

    delete mExactWordPopup;
    mExactWordPopup = 0;

    // free mode handlers
    delete mBasicModeHandler;
    mBasicModeHandler = 0;
    delete mPredictionModeHandler;
    mPredictionModeHandler = 0;
    delete mNumericModeHandler;
    mNumericModeHandler = 0;
}

QString HbVirtualQwerty::identifierName()
{
    return QString("HbVirtualQwerty");
}

bool HbVirtualQwerty::isComposing() const
{
    return mActiveModeHandler->isComposing();
}

QString HbVirtualQwerty::language()
{
    return QString();
}

void HbVirtualQwerty::reset()
{
    mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionReset);
    // close candidate popup if visible
    closeCandidatePopup();
}

void HbVirtualQwerty::focusReceived()
{
    /* Update the text case */
    updateState();
    // set input mode to default ABC
    HbInputLanguage language = inputState().language();
    if ((!focusObject()->editorInterface().isNumericEditor() && inputState().inputMode() == HbInputModeNumeric) || !language.isCaseSensitiveLanguage()) {
        HbInputState state = inputState();
        // For Case insensitive languages, the default case should be lowercase.
        if (!language.isCaseSensitiveLanguage())
        {
            state.setTextCase(HbTextCaseLower);
        }
        else if (automaticTextCaseNeeded()) {
            state.setTextCase(HbTextCaseAutomatic);
        }
        state.setInputMode(HbInputModeDefault);
        activateState(state);
        inputStateToEditor(state);
    }

    // load the new keymappings to all keypads and all mode handlers
    loadKeymap(inputState().language());
    // After loadKeyMapData call, mKeyData should have keymappings data of the current language
    if(!mKeymap) {
        return; // could not get keymappings. Just return.
    }

    //Get vkbhost
    mVkbHost = focusObject()->editorInterface().vkbHost();

    HbKeypadMode currentInputType = EModeAbc;
    if (focusObject()->editorInterface().isNumericEditor()) {
        currentInputType = EModeNumeric;
    }

    HbInputVkbWidget* keypadToOpen = 0;
    if (currentInputType == EModeAbc) {
        // See if the editor prefers numeric mode when focused. 
        Qt::InputMethodHints hints = focusObject()->inputMethodHints();
        if(hints & Qt::ImhPreferNumbers) {
            createSctKeypad();
            keypadToOpen = mSctKeypad;            
        }
        else {
            mQwertyAlphaKeypad = constructKeyboard(EModeAbc);
            keypadToOpen = mQwertyAlphaKeypad;            
        }
    } else if(currentInputType == EModeNumeric) {
        if(!mQwertyNumericKeypad) {
            mQwertyNumericKeypad = static_cast<HbQwertyNumericKeyboard*>(constructKeyboard(EModeNumeric));
            mQwertyNumericKeypad->setBackgroundDrawing(true);
        }
        keypadToOpen = mQwertyNumericKeypad;
    }

    if(!keypadToOpen) {
        return; // just return if keypad can not be created
    }

    // inform active mode handler about the focusrecieve event.
    mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionFocusRecieved);

    openKeypad(keypadToOpen);

    if (mVkbHost) {
        connect(&(focusObject()->editorInterface()), SIGNAL(cursorPositionChanged(int, int)), mVkbHost, SLOT(ensureCursorVisibility()));
    }

    if (focusObject()) {
        disconnect(&(focusObject()->editorInterface()), SIGNAL(cursorPositionChanged(int, int)), mActiveModeHandler, SLOT(cursorPositionChanged(int, int)));
        connect(&(focusObject()->editorInterface()), SIGNAL(cursorPositionChanged(int, int)), mActiveModeHandler, SLOT(cursorPositionChanged(int, int)));
        
        disconnect(focusObject(), SIGNAL(aboutToChangeOrientation()), this, SLOT(orientationAboutToChange()));
        connect(focusObject(), SIGNAL(aboutToChangeOrientation()), this, SLOT(orientationAboutToChange()));
    }
    HbInputAbstractMethod::focusReceived();
}

void HbVirtualQwerty::focusLost(bool focusSwitch)
{
    mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionFocusLost);

    if (focusObject()) {
        disconnect(&(focusObject()->editorInterface()), SIGNAL(cursorPositionChanged(int, int)), mActiveModeHandler, SLOT(cursorPositionChanged(int, int)));
    }

    closeExactWordPopup();
    if (!focusSwitch) {
        if (mVkbHost && mVkbHost->keypadStatus() != HbVkbHost::HbVkbStatusClosed) {
            // Context switch has happened but the keypad is still open.
            // Close it.
            closeKeypad();
        }
    }
    HbInputAbstractMethod::focusLost();
}

void HbVirtualQwerty::closeKeypad()
{
    if (mVkbHost && mVkbHost->keypadStatus() != HbVkbHost::HbVkbStatusClosed) {
        mVkbHost->closeKeypad(!stateChangeInProgress());
        // set mCurrentKeypad to null.
        mCurrentKeypad = 0;
        closeCandidatePopup();
    }
}

void HbVirtualQwerty::openKeypad(HbInputVkbWidget *keypadToOpen)
{
    // if null is sent, just return.
    if(!keypadToOpen || !focusObject()) {
        return;
    }
    bool disableAnimation = false;
    // see if we are trying to open a different keypad than what is already opened.
    if (mCurrentKeypad != keypadToOpen) {
        // close currently open keypad. We always close keypad without animation
        // keypad should be closed with animation only when we loses focus and this is handled
        // in focusLost function call.
        if (mVkbHost && mVkbHost->keypadStatus() != HbVkbHost::HbVkbStatusClosed) {
            mVkbHost->closeKeypad(false);
            // when their is a keypad that needs to be closed before opening the new keypad, we don't
            // want to animate the opening of new keypad.
            disableAnimation = true;
        }
    }

    // Close candidate popup if open
    closeCandidatePopup();

    QObject::disconnect(mCurrentKeypad,SIGNAL(aboutToActivateCustomAction(HbAction*)),
        this,SLOT(aboutToActivateCustomAction(HbAction*)));
    QObject::connect(keypadToOpen,SIGNAL(aboutToActivateCustomAction(HbAction*)),
        this,SLOT(aboutToActivateCustomAction(HbAction*)));

    // assign new keypad to be opened to varable mCurrentKeypad
    mCurrentKeypad =  keypadToOpen; 
    if (mVkbHost && mVkbHost->keypadStatus() != HbVkbHost::HbVkbStatusOpened) {
        connect(mVkbHost, SIGNAL(keypadClosed()), this, SLOT(keypadClosed()));

        mVkbHost->openKeypad(mCurrentKeypad, this, !disableAnimation);

        if (focusObject()) {
            connect(&(focusObject()->editorInterface()), SIGNAL(cursorPositionChanged(int, int)),
                    mVkbHost, SLOT(ensureCursorVisibility()));
        }
    }

    if(mActiveModeHandler) {
        mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionSetKeypad);
    }

    if(mCurrentKeypad && mCurrentKeypad == mQwertyAlphaKeypad) {
        mPredictionModeHandler->setTypingCorrectionLevel(HbInputSettingProxy::instance()->typingCorrectionLevel());
    }
    else {
        mPredictionModeHandler->setTypingCorrectionLevel(HbTypingCorrectionLevelLow);
    }
}

/*!
vkb widget is about to call a custom action that is mapped to one of the keypad buttons.
*/
void HbVirtualQwerty::aboutToActivateCustomAction(HbAction *custAction)
{
    Q_UNUSED(custAction);
    mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionCommit);
}

HbInputVkbWidget* HbVirtualQwerty::constructKeyboard(HbKeypadMode currentInputType)
{
    HbInputVkbWidget *keyboard = 0;
    if (currentInputType == EModeAbc) {
        const HbKeyboardMap *keyboardMap = mKeymap->keyboard(HbKeyboardVirtualQwerty);
        if (keyboardMap && keyboardMap->keys.count() > HbVirtualQwerty4x10MaxKeysCount) {
            if (mQwerty11x4Keypad) {
                return mQwerty11x4Keypad;
            }
            mQwerty11x4Keypad = new HbQwerty11x4Keyboard(this, mKeymap);
            keyboard = mQwerty11x4Keypad;
        } else {
            if (mQwerty10x4Keypad) {
                return mQwerty10x4Keypad;
            }
            mQwerty10x4Keypad = new HbQwerty10x4Keyboard(this, mKeymap);
            keyboard = mQwerty10x4Keypad;
        }
        connect(keyboard, SIGNAL(smileySelected(QString)), this, SLOT(smileySelected(QString)));
        connect(keyboard, SIGNAL(settingsListClosed()), this, SLOT(restorePreviousState()));
        //FLICKDISABLED connect(keyboard, SIGNAL(flickEvent(HbInputVkbWidget::FlickDirection)), this, SLOT(flickEvent(HbInputVkbWidget::FlickDirection)));
    } else {
        keyboard = new HbQwertyNumericKeyboard(this, mKeymap);
    }    
    connect(keyboard, SIGNAL(keypadCloseEventDetected(HbInputVkbWidget::HbVkbCloseMethod)),
        this, SLOT(keypadCloseEventDetected(HbInputVkbWidget::HbVkbCloseMethod)));
    connect(keyboard, SIGNAL(rockerDirection(int, HbInputVirtualRocker::RockerSelectionMode)),
        this, SLOT(rockerDirection(int, HbInputVirtualRocker::RockerSelectionMode)));
    connect(keyboard, SIGNAL(mouseMovedOutOfButton()), this, SLOT(mouseMovedOutOfButton()));

    return keyboard;
}

void HbVirtualQwerty::mouseHandler(int x, QMouseEvent* event)
{
    mActiveModeHandler->mouseHandler(x, event);
}

void HbVirtualQwerty::keypadClosed()
{
}

void HbVirtualQwerty::keypadCloseEventDetected(HbInputVkbWidget::HbVkbCloseMethod vkbCloseMethod)
{
    Q_UNUSED(vkbCloseMethod);
    if (isActiveMethod()) {
        if (mVkbHost) {
            mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionCommit);
            closeCandidatePopup();

            // Close input.
            QInputContext* ic = qApp->inputContext();
            if (ic) {
                QEvent *closeEvent = new QEvent(QEvent::CloseSoftwareInputPanel);
                ic->filterEvent(closeEvent);
                delete closeEvent;
            }
        }
    }
}

void HbVirtualQwerty::inputLanguageChanged(const HbInputLanguage &aNewLanguage)
{
    mPredictionModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionCommit);
    if (mExactWordPopup && mExactWordPopup->isVisible())
        closeExactWordPopup();
    // move keypad off screen
    if (mCurrentKeypad){
        mCurrentKeypad->keypadLanguageChangeAnimationUpdate(0);
    }
    // load the new key keymappings for newLanguage to all keypads and all mode handlers
    loadKeymap(aNewLanguage);

    if (focusObject() && mCurrentKeypad) {
        if (mCurrentKeypad != mQwertyAlphaKeypad && mCurrentKeypad != mQwertyNumericKeypad) {
            mCurrentKeypad->animKeyboardChange();
            openKeypad(mQwertyAlphaKeypad);
        }
        mCurrentKeypad->animKeyboardChange();
    }
    mPredictionModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionPrimaryLanguageChanged);
}

void HbVirtualQwerty::inputStateActivated(const HbInputState& newState)
{

    if (!isActiveMethod()) {
        return;  // Just to be sure...
    }

    if (newState.inputMode() == HbInputModeNumeric && mQwertyNumericKeypad) {
        mQwertyNumericKeypad->setMode(EModeNumeric, HbModifierNone);
    } else if (mQwertyAlphaKeypad) {
        if (newState.textCase() == HbTextCaseUpper || newState.textCase() == HbTextCaseAutomatic) {
            mQwertyAlphaKeypad->setMode(EModeAbc, HbModifierShiftPressed);
        } else {
            mQwertyAlphaKeypad->setMode(EModeAbc, HbModifierNone);
        }
    }
    // need to make sure that the prediction mode handler has the input state's language set 
    // it is the global input language by default
    if(mPredictionModeHandler && newState.language().language() != HbInputSettingProxy::instance()->globalInputLanguage().language()) {
        HbInputLanguage inputStateLang = newState.language();
        mPredictionModeHandler->setLanguage(inputStateLang);
    }
    HbInputModeHandler *previousModeHandler = mActiveModeHandler;
    if (newState.inputMode() == HbInputModeNumeric) {
        mActiveModeHandler = mNumericModeHandler;
    }  else if (newState.inputMode() == HbInputModeDefault && usePrediction()) {
        mActiveModeHandler = mPredictionModeHandler;
        // by passing HbInputModeActionFocusRecieved we will be setting the candidate list and keypad
        mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionFocusRecieved);
        /* The below line should be added in orientationChanged Slot, however currently it is not working correctly
         hence this fix is temporarily added here */
        mPredictionModeHandler->setAutocompletionStatus(HbInputSettingProxy::instance()->isAutocompletionEnabled(HbKeyboardSettingQwerty));
    } else if (newState.inputMode() == HbInputModeDefault) {
        mActiveModeHandler = mBasicModeHandler;
        // Auto completer setup needs following line.
        mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionFocusRecieved);
    }

    if (focusObject()) {
        disconnect(&(focusObject()->editorInterface()), SIGNAL(cursorPositionChanged(int, int)), previousModeHandler, SLOT(cursorPositionChanged(int, int)));
        connect(&(focusObject()->editorInterface()), SIGNAL(cursorPositionChanged(int, int)), mActiveModeHandler, SLOT(cursorPositionChanged(int, int)));
    }

    // load the new keymappings to all keypads and all mode handlers
    loadKeymap(newState.language());

    // if there is a change in the modehandler we need send a commit in previous mode handler.
    if (previousModeHandler != mActiveModeHandler) {
        //Auto Completion part also to be committed on mode change and hide exact word popup.
        previousModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionCommit);
        if (previousModeHandler == mPredictionModeHandler) {
            closeExactWordPopup();
        }
        // lets set candidate list and keypad type to the engine.
        mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionSetCandidateList);
        mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionSetKeypad);
    }
}

/*!
This function loads the keymappings data of the given language to all the keypads and to all mode handlers.
It first checks if we already have keymappings data of given language in mKeyData, if not then gets the
keymappings,loads them to all avaialble keyboards and to all mode handlers
*/
void HbVirtualQwerty::loadKeymap(const HbInputLanguage &newLanguage)
{
    // inform all the mode handler about the language change.
    //don't try to get the keymappings if we ( mKeyData) already have keymappings for newLanguage
    if (!mKeymap || mKeymap->language() != newLanguage) {

        const HbKeymap* keymap = HbKeymapFactory::instance()->keymap(newLanguage);
        if (keymap) {
            mKeymap =  keymap;
            if (mQwertyNumericKeypad) {
                mQwertyNumericKeypad->setKeymap(mKeymap);
            }

            mQwertyAlphaKeypad = constructKeyboard(EModeAbc);
            mQwertyAlphaKeypad->setKeymap(mKeymap);

            if (mSctKeypad) {
                mSctKeypad->setKeymap(mKeymap);
            }

            // inform mode handlers about the language change.
            if (mBasicModeHandler) {
                mBasicModeHandler->setKeymap(mKeymap);
            }
            if (mPredictionModeHandler) {
                mPredictionModeHandler->setKeymap(mKeymap);
            }
            if (mNumericModeHandler) {
                mNumericModeHandler->setKeymap(mKeymap);
            }
        }
    }
}

void HbVirtualQwerty::switchSpecialCharacterTable()
{
    if (mCurrentKeypad != mSctKeypad) {
        displaySpecialCharacterTable(this);
    } else {
        // we always go back to alpha qwerty mode after coming back from sct
        openKeypad(constructKeyboard(EModeAbc));
    }
}

void HbVirtualQwerty::secondaryInputLanguageChanged(const HbInputLanguage &newLanguage)
{
    Q_UNUSED(newLanguage);
    // inform mode handler about the language change.
    mPredictionModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionCommit);
    mPredictionModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionSecondaryLanguageChanged);
}

int HbVirtualQwerty::displaySpecialCharacterTable(QObject* aReceiver)
{
    Q_UNUSED(aReceiver);

    createSctKeypad();
    //open the keypad
    openKeypad(mSctKeypad);

    return 0;
}

void HbVirtualQwerty::smileySelected(QString smiley)
{
     mActiveModeHandler->smileySelected(smiley);
}

void HbVirtualQwerty::selectSpecialCharacterTableMode()
{
    mQwertyAlphaKeypad = constructKeyboard(EModeAbc);
    mQwertyAlphaKeypad->showSmileyPicker();
}

/*!
Closes the candidate popup if visible.
*/
void HbVirtualQwerty::closeCandidatePopup()
{
    if (mCandidatePopup && mCandidatePopup->isVisible()) {
        mCandidatePopup->hide();
    }
}

/*!
Slot used by mode handlers to close the autocompletion popup.
*/
void HbVirtualQwerty::closeAutoCompletionPopup()
{
    closeCandidatePopup();
}

/*!
Launches auto-completion popup if there are candidates available.
*/
void HbVirtualQwerty::launchAutoCompletionPopup(const QStringList& candidates)
{
    if (!mCandidatePopup) {
        mCandidatePopup = new HbCandidateList(this);
        connect(mCandidatePopup, SIGNAL(candidatePopupCancelled()), this, SLOT(restorePreviousState()));
        connect(mCandidatePopup, SIGNAL(candidateSelected(int,const QString&)), this, SLOT(candidatePopupClosed(int, const QString&)));
    }

    if (candidates.count() > 0) {
        mCandidatePopup->populateList(candidates);
        mCandidatePopup->setModal(false);

        if (mCandidatePopup->setSizeAndPositionForAutoCompletion(mVkbHost)) {
            mCandidatePopup->setDismissPolicy(HbPopup::TapInside);
            mCandidatePopup->setBackgroundFaded(false);
            mCandidatePopup->show();
        }
    } else {
        closeCandidatePopup();
    }
}

HbKeyboardType HbVirtualQwerty::currentKeyboardType() const
{
    HbKeyboardType type = HbKeyboardNone;
    if (mCurrentKeypad) {
        type = mCurrentKeypad->keyboardType();
    }
    return type;
}

/*!
Launches the candidate list.
*/
void HbVirtualQwerty::launchCandidatePopup()
{

}
/*!
Launches the candidate list with the passed candidates.
*/
void HbVirtualQwerty::launchCandidatePopup(const QStringList &candidates)
{
    //before launching candidate popup, close exact word popup if visible.
    closeExactWordPopup();
    if (!mCandidatePopup) {
        mCandidatePopup = new HbCandidateList(this);
        connect(mCandidatePopup, SIGNAL(candidatePopupCancelled()), this, SLOT(restorePreviousState()));
        connect(mCandidatePopup, SIGNAL(candidateSelected(int,const QString&)), this, SLOT(candidatePopupClosed(int, const QString&)));
    }
    mCandidatePopup->populateList(candidates);
    mCandidatePopup->setModal(true);

    QSizeF candListSize = mCandidatePopup->size();
    QPointF candListPos = mCandidatePopup->pos();
    getCandidatePositionAndSize(mCandidatePopup, mCurrentKeypad,candListPos,candListSize);

    QRectF geom = mCandidatePopup->geometry();
    geom.setHeight(candListSize.height());
    geom.setWidth(candListSize.width());
    mCandidatePopup->setGeometry(geom);

    mCandidatePopup->setPos(candListPos);

    mCandidatePopup->show();
}

/*!
Commits the candidate upon closing of the candidate list.
*/
void HbVirtualQwerty::candidatePopupClosed(int closingKey, const QString& candidate)
{       
    if (candidate.size() > 0) {
        if ((focusObject()->editorInterface().inputConstraints() & HbEditorConstraintAutoCompletingField)) {
            emit autoCompletionPopupClosed(candidate, closingKey);
        } else {
            mPredictionModeHandler->candidatePopupClosed(candidate, closingKey);
        }
    }
}

/*!
The call back from framework to indicate that the orientation is about to change. This closes the keypad
if it is already open.
*/
void HbVirtualQwerty::orientationAboutToChange()
{
}

/*!
The framework calls this method when the predictive input status changes.
*/
void HbVirtualQwerty::predictiveInputStateChanged(HbKeyboardSettingFlags keyboardType, bool newStatus)
{
    Q_UNUSED(newStatus);

    if (keyboardType & HbKeyboardSettingQwerty) {
        HbInputFocusObject *focusedObject = focusObject();
        if (focusedObject) {
            // Just refresh the situation.
            inputStateActivated(inputState());
        }
    }
}

void HbVirtualQwerty::primaryCandidateModeChanged(HbPrimaryCandidateMode mode)
{
    mPredictionModeHandler->setPrimaryCandidateMode(mode);
}

void HbVirtualQwerty::autocompletionStateChanged(HbKeyboardSettingFlags keyboardType, bool newState)
{
    if ( isActiveMethod() && (keyboardType & HbKeyboardSettingQwerty) ){
        mPredictionModeHandler->setAutocompletionStatus(newState);
    }
}

void HbVirtualQwerty::typingCorrectionLevelChanged(HbTypingCorrectionLevel correctionLevel)
{
    mPredictionModeHandler->setTypingCorrectionLevel(correctionLevel);
}
/*!
this function is called whenever there is a hardware keypress Or virtual keypad button is pressed.
*/
bool HbVirtualQwerty::filterEvent(const QEvent* event)
{
    return mActiveModeHandler->filterEvent(event);
}

void HbVirtualQwerty::flickEvent(HbInputVkbWidget::HbFlickDirection direction)
{
    Q_UNUSED(direction);
}

/*!
Slot used by mode handlers to set the keypad modifiers
*/
void HbVirtualQwerty::launchExactWordPopup(QString exactWord)
{
    if (!mExactWordPopup) {
        mExactWordPopup = HbExactWordPopup::instance();
        mExactWordPopup->setPreferredHeight(HbDeviceProfile::profile(
            mCurrentKeypad->mainWindow()).logicalSize().height() * HbExactPopupHeightFactor);
        connect(mExactWordPopup, SIGNAL(exactWordSelected()), mPredictionModeHandler, SLOT(exactWordSelected()));
    }
    mExactWordPopup->setText(exactWord);
    mExactWordPopup->showText(getCursorCoordinatePosition());
}

/*!
Slot for hiding the exact word popup.
*/
void HbVirtualQwerty::closeExactWordPopup()
{
    if (mExactWordPopup && mExactWordPopup->isVisible()) {
        mExactWordPopup->hide();
    }
}

QPointF HbVirtualQwerty::getCursorCoordinatePosition()
{
    QPointF cursorPos(0.0, 0.0);
    QRectF microRect = focusObject()->microFocus();
    microRect.adjust(0.0, -HbEditorExtraMargin, 0.0, HbEditorExtraMargin);

    if (mVkbHost) {
        QRectF activeViewRect= mVkbHost->applicationArea();
        qreal exactPopupHeight = HbDeviceProfile::profile(
            mCurrentKeypad->mainWindow()).logicalSize().height() * HbExactPopupHeightFactor;
        qreal topArea = microRect.top() - activeViewRect.top();
        qreal bottomArea = activeViewRect.bottom() - microRect.bottom();
        qreal xPos = microRect.left();
        cursorPos.setX(xPos);
        QSizeF exactPopupSize = mExactWordPopup->size();

        // show the exact word popup either above or below the currently focused editor if both fits in the visible area.
        // If enough space is not available, then show the exact word popup inside the focused editor        
        if (topArea > exactPopupHeight || bottomArea > exactPopupHeight) {
            // show the popup above the editor if more space available
            if (topArea > exactPopupHeight) {
                cursorPos.setY(microRect.topLeft().y() - exactPopupHeight);
            } else {
                cursorPos.setY(microRect.bottomLeft().y());
            }
            // if there is no space to show the exact word popup in the right side of the cursor
            // recatangle, then show it on the left side of the cursor            
            if (xPos + exactPopupSize.width() > activeViewRect.width()) {
                cursorPos.setX(xPos - exactPopupSize.width());
            }
        } else {
            // show the exact word popup inside the editor
            cursorPos.setY(microRect.top() + (microRect.height() / 2) - (exactPopupHeight / 2));
            xPos = xPos + HbDeltaWidth;
            if (xPos + exactPopupSize.width() > activeViewRect.width()) {
                qreal startPos = microRect.left() - (3 * exactPopupSize.width()) - HbDeltaWidth;
                cursorPos.setX((startPos > activeViewRect.left()) ? startPos : activeViewRect.left());
            }
        }
    }
    return cursorPos;
}


/*!
Returns true if prediction is on, prediction engine is available and predictions is allowed in current editor.
*/
bool HbVirtualQwerty::usePrediction() const
{
    HbInputFocusObject *fo = focusObject();
    if (HbInputSettingProxy::instance()->predictiveInputStatus(HbKeyboardSettingQwerty) &&
        fo &&
        fo->editorInterface().isPredictionAllowed() &&
        mPredictionModeHandler->isActive() &&
        HbPredictionFactory::instance()->predictionEngineForLanguage(inputState().language())) {
        return true;           
    }

    return false;
}

void HbVirtualQwerty::mouseMovedOutOfButton()
{
    mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionCancelButtonPress);
}

QList<HbKeyPressProbability> HbVirtualQwerty::probableKeypresses()
{
    return mCurrentKeypad->probableKeypresses();
}

void HbVirtualQwerty::restorePreviousState()
{
    mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionRestorePreviousState);
}

void HbVirtualQwerty::createSctKeypad()
{
    if (!mSctKeypad) {
        mSctKeypad = new HbSctKeyboard(this, mKeymap);
        connect(mSctKeypad, SIGNAL(smileySelected(QString)), this, SLOT(smileySelected(QString)));
        connect(mSctKeypad, SIGNAL(keypadCloseEventDetected(HbInputVkbWidget::HbVkbCloseMethod)), this, SLOT(keypadCloseEventDetected(HbInputVkbWidget::HbVkbCloseMethod)));      
    }
    mSctKeypad->setMode(EModeAbc, HbModifierNone);
}

// End of file



