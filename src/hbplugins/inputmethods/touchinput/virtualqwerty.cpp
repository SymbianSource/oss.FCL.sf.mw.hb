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
#include <QLocale>

#include <hbinputexactwordpopup.h>
#include <hbinputkeymapfactory.h>
#include <hbinputkeymap.h>
#include <hbinputcandidatelist.h>
#include <hbinputsettingproxy.h>
#include <hbinpututils.h>
#include <hbinputvirtualrocker.h>
#include <hbinputsctlandscape.h>
#include <hbinputqwertytouchkeyboard.h>
#include <hbinputeditorinterface.h>
#include <hbinputdef.h>
#include <hbinputvkbhost.h>
#include <hbinputcommondialogs.h>

#include <hbmainwindow.h>

#include "hbinputbasicqwertyhandler.h"
#include "hbinputpredictionqwertyhandler.h"
#include "hbinputnumericqwertyhandler.h"
#include <hbaction.h>
#include <hbview.h>
#include <hbinputpredictionfactory.h>

HbVirtualQwerty::HbVirtualQwerty() : mCurrentKeypad(0),
                                     mQwertyAlphaKeypad(0),
                                     mQwertyNumericKeypad(0),
                                     mSctKeypad(0),
                                     mKeymap(0),
                                     mExactWordPopup(0),
                                     mCandidatePopup(0),
                                     mOrientationAboutToChange(false),
                                     mSctMode(HbInputVkbWidget::HbSctViewSpecialCharacter),
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
}

void HbVirtualQwerty::focusReceived()
{
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
        state.inputMode() = HbInputModeDefault;
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

    HbInputVkbWidget * keypadToOpen = 0;
    if (currentInputType == EModeAbc) {
        if(!mQwertyAlphaKeypad) {
            mQwertyAlphaKeypad = constructKeypad(EModeAbc);
            connect(mQwertyAlphaKeypad, SIGNAL(smileySelected(QString)), this, SLOT(smileySelected(QString)));
            //FLICKDISABLED connect(mQwertyAlphaKeypad, SIGNAL(flickEvent(HbInputVkbWidget::FlickDirection)), this, SLOT(flickEvent(HbInputVkbWidget::FlickDirection)));
        }
        keypadToOpen = mQwertyAlphaKeypad;
    } else if(currentInputType == EModeNumeric) {
        if(!mQwertyNumericKeypad) {
            mQwertyNumericKeypad = constructKeypad(EModeNumeric);
            mQwertyNumericKeypad->setBackgroundDrawing(true);
        }
        keypadToOpen = mQwertyNumericKeypad;
    }

    if(!keypadToOpen) {
        return; // just return if keypad can not be created
    }

    // inform active mode handler about the focusrecieve event.
    mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionFocusRecieved);

    // We need to check if this focusRecieved call is due to a orientation
    // switch. If yes we should get the keypad status prior to the orientation
    // switch and open the keypad in that state only.
    // For example we have minimized the keypad in Qwerty mode and change the
    // orientation to portrait then in Itu-T mode also keypad should be in minimized state.
    HbVkbHost *host = focusObject()->editorInterface().vkbHost();
    if (orientationContextSwitchInProgress()) {
        if (host) {
            // We can get the keypad status prior to the orientation switch from vkbHost it self.
            HbVkbHost::HbVkbStatus vkbStatus = host->keypadStatusBeforeOrientationChange();
            if (vkbStatus != HbVkbHost::HbVkbStatusClosed) {
                openKeypad(keypadToOpen,vkbStatus == HbVkbHost::HbVkbStatusMinimized);
            }
        }
    } else {
        openKeypad(keypadToOpen);
    }

    if (mVkbHost) {
        connect(&(focusObject()->editorInterface()), SIGNAL(cursorPositionChanged(int, int)), mVkbHost, SLOT(ensureCursorVisibility()));
    }

    if (focusObject()) {
        disconnect(&(focusObject()->editorInterface()), SIGNAL(cursorPositionChanged(int, int)), mActiveModeHandler, SLOT(cursorPositionChanged(int, int)));
        connect(&(focusObject()->editorInterface()), SIGNAL(cursorPositionChanged(int, int)), mActiveModeHandler, SLOT(cursorPositionChanged(int, int)));
    }
}

void HbVirtualQwerty::focusLost(bool focusSwitch)
{
    mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionFocusLost);

    if (focusObject()) {
        disconnect(&(focusObject()->editorInterface()), SIGNAL(cursorPositionChanged(int, int)), mActiveModeHandler, SLOT(cursorPositionChanged(int, int)));
    }

    if (!focusSwitch) {
        if (mVkbHost && mVkbHost->keypadStatus() != HbVkbHost::HbVkbStatusClosed) {
            // Context switch has happened but the keypad is still open.
            // Close it.
            closeKeypad();
        }
    }
}

void HbVirtualQwerty::closeKeypad()
{
    if (mVkbHost && mVkbHost->keypadStatus() != HbVkbHost::HbVkbStatusClosed) {
        mVkbHost->closeKeypad();
        // set mCurrentKeypad to null.
        mCurrentKeypad = 0;
        if (mCandidatePopup && mCandidatePopup->isVisible()) {
            mCandidatePopup->hide();
        }
    }
}

void HbVirtualQwerty::openKeypad(HbInputVkbWidget * keypadToOpen,bool inMinimizedMode )
{
    // if null is sent, just return.
    if(!keypadToOpen) {
        return;
    }
    // see if we are trying to open a different keypad than what is already opened.
    if (mCurrentKeypad != keypadToOpen) {
        // close currently open keypad. We always close keypad without animation
        // keypad should be closed with animation only when we loses focus and this is handled
        // in focusLost function call.
        if (mVkbHost && mVkbHost->keypadStatus() != HbVkbHost::HbVkbStatusClosed) {
            mVkbHost->closeKeypad(false);
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
        if (inMinimizedMode) {
            mVkbHost->openMinimizedKeypad(mCurrentKeypad, this);
        } else {
            mVkbHost->openKeypad(mCurrentKeypad, this);
        }

        // If previous focused editor was numeric, prediction is disabled.
        // Enable prediction if prediction was set in alpha editor prior
        // to focusing numeric editor.
/*        if (mPrevKeypadMode == EModeAbc && HbInputModeLatinPredictive == mPreviousInputMode) {
            mMode = HbInputModeLatinPredictive;
        } else if (mPrevKeypadMode == EModeNumeric && HbInputModeLatinPredictive == mMode) {
            // If the previous focused editor was alpha and if prediction is
            // on, disable prediction. Store the previous state because if
            // any alpha editor is focused next, the previous prediction state
            // should be enabled.
            mMode = HbInputModeDefault;
            mPreviousInputMode = HbInputModeLatinPredictive;
        }  */
        connect(&(focusObject()->editorInterface()), SIGNAL(cursorPositionChanged(int, int)), mVkbHost, SLOT(ensureCursorVisibility()));
    }
}

HbQwertyKeyboard* HbVirtualQwerty::constructKeypad(HbKeypadMode currentInputType)
{
    HbQwertyKeyboard* keypad =  new HbQwertyKeyboard(this, mKeymap, 0, currentInputType);
    connect(keypad, SIGNAL(keypadCloseEventDetected(HbInputVkbWidget::HbVkbCloseMethod)),
        this, SLOT(keypadCloseEventDetected(HbInputVkbWidget::HbVkbCloseMethod)));
    connect(keypad, SIGNAL(rockerDirection(int, HbInputVirtualRocker::RockerSelectionMode)),
        this, SLOT(rockerDirection(int, HbInputVirtualRocker::RockerSelectionMode)));
    connect(keypad, SIGNAL(mouseMovedOutOfButton()), this, SLOT(mouseMovedOutOfButton()));
    keypad->setRockerVisible(true);

    return keypad;
}

void HbVirtualQwerty::mouseHandler(int x, QMouseEvent* event)
{
    mActiveModeHandler->mouseHandler(x, event);
}

void HbVirtualQwerty::keypadClosed()
{
    // by calling focuslost we will be committing the inline text.
    mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionFocusLost);

    if (mOrientationAboutToChange) {
        mOrientationAboutToChange = false;
    }
}

void HbVirtualQwerty::keypadCloseEventDetected(HbInputVkbWidget::HbVkbCloseMethod vkbCloseMethod)
{
    Q_UNUSED(vkbCloseMethod);
    if (isActiveMethod()) {
        if (mVkbHost && mVkbHost->keypadStatus() != HbVkbHost::HbVkbStatusMinimized) {
            mVkbHost->minimizeKeypad(!stateChangeInProgress());
            if (mCandidatePopup) {
                mCandidatePopup->hide();
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
    mPredictionModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionPrimaryLanguageChanged);
    if (mCurrentKeypad){
        mCurrentKeypad->animKeyboardChange();
    }

}

void HbVirtualQwerty::inputStateActivated(const HbInputState& newState)
{

    if (!isActiveMethod()) {
        return;  // Just to be sure...
    }

    if (newState.inputMode() == HbInputModeNumeric && mQwertyNumericKeypad) {
        mQwertyNumericKeypad->setMode(EModeNumeric, HbModifierNone);
    } else if(mQwertyAlphaKeypad) {
        if (newState.textCase() == HbTextCaseUpper || newState.textCase() == HbTextCaseAutomatic) {
            mQwertyAlphaKeypad->setMode(EModeAbc, HbModifierShiftPressed);
        } else {
            mQwertyAlphaKeypad->setMode(EModeAbc, HbModifierNone);
        }
    }

    HbInputModeHandler *previousModeHandler = mActiveModeHandler;
    if (newState.inputMode() == HbInputModeNumeric) {
        mActiveModeHandler = mNumericModeHandler;
    }  else if (newState.inputMode() == HbInputModeDefault && usePrediction()) {
        mActiveModeHandler = mPredictionModeHandler;
        if (mQwertyAlphaKeypad) {
            mQwertyAlphaKeypad->disconnect(SIGNAL(charFromPreviewSelected(QString)));
            connect(mQwertyAlphaKeypad, SIGNAL(charFromPreviewSelected(QString)), mActiveModeHandler, SLOT(charFromPreviewSelected(QString)));
        }
    } else if (newState.inputMode() == HbInputModeDefault) {
        mActiveModeHandler = mBasicModeHandler;
        // Auto completer setup needs following line.
        mActiveModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionFocusRecieved);
        if (mQwertyAlphaKeypad) {
            mQwertyAlphaKeypad->disconnect(SIGNAL(charFromPreviewSelected(QString)));
            connect(mQwertyAlphaKeypad, SIGNAL(charFromPreviewSelected(QString)), mActiveModeHandler, SLOT(charFromPreviewSelected(QString)));
        }
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
    //dont try to get the keymappings if we ( mKeyData) already have keymappings for newLanguage
    if (!mKeymap || mKeymap->language() != newLanguage) {

        const HbKeymap* keymap = HbKeymapFactory::instance()->keymap(newLanguage);
        if (keymap) {
            mKeymap =  keymap;
            if (mQwertyNumericKeypad) {
                mQwertyNumericKeypad->setKeymap(mKeymap);
            }

            if (mQwertyAlphaKeypad) {
                mQwertyAlphaKeypad->setKeymap(mKeymap);
            }

            if (mSctKeypad) {
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
        }
    }
}

void HbVirtualQwerty::switchSpecialCharacterTable()
{
    if (mCurrentKeypad != mSctKeypad) {
        mSctMode = HbInputVkbWidget::HbSctViewSpecialCharacter;
        displaySpecialCharacterTable(this);
    } else {
        // we always go back to alpha qwerty mode after coming back from sct
        openKeypad(mQwertyAlphaKeypad);
    }
}

void HbVirtualQwerty::secondaryInputLanguageChanged(const HbInputLanguage &newLanguage)
{
    Q_UNUSED(newLanguage);
    mPredictionModeHandler->actionHandler(HbInputModeHandler::HbInputModeActionSecondaryLanguageChanged);
}

int HbVirtualQwerty::displaySpecialCharacterTable(QObject* aReceiver)
{
    Q_UNUSED(aReceiver);

    if (!mSctKeypad) {
        mSctKeypad = new HbInputSctLandscape(this, mKeymap);
        connect(mSctKeypad, SIGNAL(sctCharacterSelected(QString)), this, SLOT(sctCharacterSelected(QString)));
        connect(mSctKeypad, SIGNAL(smileySelected(QString)), this, SLOT(smileySelected(QString)));
        connect(mSctKeypad, SIGNAL(keypadCloseEventDetected(HbInputVkbWidget::HbVkbCloseMethod)), this, SLOT(keypadCloseEventDetected(HbInputVkbWidget::HbVkbCloseMethod)));
        connect(mSctKeypad, SIGNAL(rockerDirection(int, HbInputVirtualRocker::RockerSelectionMode)),
            this, SLOT(rockerDirection(int, HbInputVirtualRocker::RockerSelectionMode)));
        mSctKeypad->setRockerVisible(true);
    }

    // set up sct!
    mSctKeypad->setSct(mSctMode);
    //open the keypad
    openKeypad(mSctKeypad);

    return 0;
}

/*!
Call-back implementation to indicate that a character was selected from the SCT. With this, the character is committed to the
editor and editor is again made to focus.
*/
void HbVirtualQwerty::sctCharacterSelected(QString character)
{
    mActiveModeHandler->sctCharacterSelected(character);
    /* Update the text case */
    updateState();
}
void HbVirtualQwerty::smileySelected(QString smiley)
{
     mActiveModeHandler->smileySelected(smiley);
}

void HbVirtualQwerty::selectSpecialCharacterTableMode()
{
    if (mQwertyAlphaKeypad) {
        mQwertyAlphaKeypad->showSmileyPicker(4, 10);
    }
}

/*!
Slot used by mode handlers to close the autocompletion popup.
*/
void HbVirtualQwerty::closeAutoCompletionPopup()
{
    if (mCandidatePopup && mCandidatePopup->isVisible()) {
        mCandidatePopup->hide();
    }
}

/*!
Launches auto-completion popup if there are candidates available.
*/
void HbVirtualQwerty::launchAutoCompletionPopup(const QStringList& candidates)
{
    if (!mCandidatePopup) {
        mCandidatePopup = new HbCandidateList(this);
        connect(mCandidatePopup, SIGNAL(candidatePopupCancelled()), this, SLOT(candidatePopupCancelled()));
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
        connect(mCandidatePopup, SIGNAL(candidatePopupCancelled()), this, SLOT(candidatePopupCancelled()));
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
void HbVirtualQwerty::candidatePopupClosed(int closingKey)
{
    if (mCandidatePopup) {
        QString currentCandidate = mCandidatePopup->currentCandidate();
        if (currentCandidate.size() > 0) {
            if ((focusObject()->editorInterface().constraints() & HbEditorConstraintAutoCompletingField)) {
                emit autoCompletionPopupClosed(currentCandidate, closingKey);
            } else {
            mPredictionModeHandler->candidatePopupClosed(currentCandidate, closingKey);
            }
        }
    }
}

/*!
The call back from framework to indicate that the orientation is about to change. This closes the keypad
if it is already open.
*/
void HbVirtualQwerty::orientationAboutToChange()
{
    HbInputMethod::orientationAboutToChange();

    if (isActiveMethod()) {
        mOrientationAboutToChange = true;
    }
    closeKeypad();
}

/*!
This function is called during a long key press
for a long time.
*/
void HbVirtualQwerty::launchCharacterPreviewPane(int key)
{
    // In alpha keyboard, when the keypad is closed by
    // dragging it, long key press event is generated.
    // Character preview must not be shown in this case.
    if (mVkbHost && mVkbHost->keypadStatus() != HbVkbHost::HbVkbStatusOpened) {
        return;
    }

    // get the characters bound to the key.
    QStringList spellList;
    mActiveModeHandler->getAndFilterCharactersBoundToKey(spellList, static_cast<Qt::Key>(key));

    bool previewAvailable = false;
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
        previewAvailable = mQwertyAlphaKeypad->previewCharacters(spellList);
    }
    mActiveModeHandler->characterPreviewAvailable(previewAvailable);
}

/*!
Slot used by virtual rocker to move cursor.
*/
void HbVirtualQwerty::rockerDirection(int aDirection, HbInputVirtualRocker::RockerSelectionMode aSelectionMode)
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

void HbVirtualQwerty::predictiveInputStatusChanged(int newStatus)
{
    Q_UNUSED(newStatus);

    HbInputFocusObject *focusedObject = focusObject();
    if (focusedObject) {
        // Just refresh the situation.
        inputStateActivated(inputState());
        return;
    }
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
        connect(mExactWordPopup, SIGNAL(exactWordSelected()), mPredictionModeHandler, SLOT(exactWordPopupClosed()));
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
    QRectF microRect = focusObject()->microFocus();
    return microRect.topLeft();
}

/*!
Returns true if prediction is on, prediction engine is available and predictions is allowed in current editor.
*/
bool HbVirtualQwerty::usePrediction() const
{
    HbInputFocusObject *fo = focusObject();
    if (HbInputSettingProxy::instance()->predictiveInputStatus() &&
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

void HbVirtualQwerty::candidatePopupCancelled()
{
    if(mPredictionModeHandler) {
        mPredictionModeHandler->showExactWordPopupIfNeeded();
    }
}
// End of file
