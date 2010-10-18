
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
#include <hbinputbutton.h>
#include <QTextCharFormat>
#include <HbColorScheme>
#include "hbinputpredictionqwertyhandler.h"
#include "hbinputpredictionhandler_p.h"
#include "hbinputabstractbase.h"

class HbInputPredictionQwertyHandlerPrivate: public HbInputPredictionHandlerPrivate
{
    Q_DECLARE_PUBLIC(HbInputPredictionQwertyHandler)
public:
    HbInputPredictionQwertyHandlerPrivate();
    ~HbInputPredictionQwertyHandlerPrivate();
    void deleteOneCharacter();

public:
    bool buttonPressed(const QKeyEvent *event);
    bool buttonReleased(const QKeyEvent *event);
    void init();
    void updateEditor();
    void commitSecondaryWord();
    QString getCommitString();
    void showExactWordPopupIfNeeded();

public:
    int mButton;    
    HbFnState mFnState;
    bool mExactPopupLaunched;    
    bool mShowExactWordInTooltip;
    int mPrimaryCandidateIndex;
    int mSecondaryCandidateIndex;
    int mTypingCorrectionLevel;
    HbTypingCorrectionLevel mCorrectionLevel;
};

HbInputPredictionQwertyHandlerPrivate::HbInputPredictionQwertyHandlerPrivate()
:mButton(0),    
    mFnState(HbFnOff),
    mExactPopupLaunched(false)
{
}

HbInputPredictionQwertyHandlerPrivate::~HbInputPredictionQwertyHandlerPrivate()
{
}

void HbInputPredictionQwertyHandlerPrivate::init()
{
}

bool HbInputPredictionQwertyHandlerPrivate::buttonReleased(const QKeyEvent *event)
{
    Q_Q(HbInputPredictionQwertyHandler);
    HbInputFocusObject *focusObject = 0;
    focusObject = mInputMethod->focusObject();
    if (!focusObject) {
        qDebug("HbVirtualQwerty::virtualButtonClicked : no focused editor widget!");
        return false;
    }


    int key = event->key();
    
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
    case HbInputButton::ButtonKeyCodeShift: {
            HbTextCase currentTextCase = (HbTextCase)focusObject->editorInterface().textCase();
            HbInputLanguage language = mInputMethod->inputState().language();

            // Update the Case Information in HbInputState, it internally updates in HbEditorInterface as well
            switch(currentTextCase) {
            case HbTextCaseLower:
                // For Case-insensitive languages, Shift Key is used to switch between character sets (i.e lower case characters and shifted characters)
                if(!language.isCaseSensitiveLanguage()){
                    currentTextCase = HbTextCaseUpper; 
                }
                else {
                    currentTextCase = HbTextCaseAutomatic;
                }
                break;              
            case HbTextCaseUpper:   
                currentTextCase = HbTextCaseLower;              
                break;
            case HbTextCaseAutomatic:
                currentTextCase = HbTextCaseUpper;                              
                break;
            default:
                break;
            }
            HbInputState state = mInputMethod->inputState();
            state.setTextCase(currentTextCase);         
            mInputMethod->activateState(state);
        }
        break;
    case HbInputButton::ButtonKeyCodeSymbol: { // Ctrl/Chr
    case HbInputButton::ButtonKeyCodeAlphabet:
        mInputMethod->switchSpecialCharacterTable();
        }       
        break;
    case HbInputButton::ButtonKeyCodeSettings:
        mInputMethod->closeExactWordPopup();
        break;
    default: {
            HbTextCase currentTextCase = focusObject->editorInterface().textCase();
            Qt::KeyboardModifiers modifiers = Qt::NoModifier;
            if (mFnState == HbFnNext) {
                modifiers |= Qt::AltModifier;
                mFnState = HbFnOff;
            } else if (mFnState == HbFnOn) {
                modifiers |= Qt::AltModifier;
            }
            // If shift is pressed, the shifted characters have to be input.
            if ( HbTextCaseUpper == currentTextCase || HbTextCaseAutomatic == currentTextCase ) {
                modifiers |= Qt::ShiftModifier;
            }


            // let's pass it to the base class.
            ret = q->HbInputPredictionHandler::filterEvent(event);

            mInputMethod->updateState();
        }
        break;
    };
    return ret;
}

bool HbInputPredictionQwertyHandlerPrivate::buttonPressed(const QKeyEvent *event)
{
    if (event->isAutoRepeat() && mButton == event->key()) {
        if (mButton == HbInputButton::ButtonKeyCodeSymbol) {
            mInputMethod->selectSpecialCharacterTableMode();
            mButton = 0;        
            return true;
        }
    }
        
    mButton = event->key();
    return false;
}


HbInputPredictionQwertyHandler::HbInputPredictionQwertyHandler(HbInputAbstractMethod *inputMethod)
    :HbInputPredictionHandler(* new HbInputPredictionQwertyHandlerPrivate, inputMethod)
{
    Q_D(HbInputPredictionQwertyHandler);
    d->q_ptr = this;
    d->init();
}

HbInputPredictionQwertyHandler::~HbInputPredictionQwertyHandler()
{
}

/*!
Action Handler.
*/
bool HbInputPredictionQwertyHandler::actionHandler(HbInputModeAction action)
{
    Q_D(HbInputPredictionQwertyHandler);
    bool ret = true;
    switch (action) {
        case HbInputModeActionCancelButtonPress:
            break;
        case HbInputModeActionFocusRecieved:
            HbInputPredictionHandler::actionHandler(HbInputModeActionSetCandidateList);
            HbInputPredictionHandler::actionHandler(HbInputModeActionSetKeypad);
            break;
        case HbInputModeActionFocusLost:
            HbInputPredictionHandler::actionHandler(HbInputModeActionFocusLost);

            //TODO
            /*
            if (d->mExactPopupLaunched) {
                sendCommitString(d->mCandidates->at(1));
            } else {
                sendCommitString(d->mCandidates->at(0));
            }
            */
            // close exactword popup.
            d->mInputMethod->closeExactWordPopup();
            break;
        default: 
            ret = HbInputPredictionHandler::actionHandler(action);
    }
    return ret;
}

/*!
filterEvent for key event.
*/
bool HbInputPredictionQwertyHandler::filterEvent(const QKeyEvent * event)
{
    Q_D(HbInputPredictionQwertyHandler);

    if (event->type() == QEvent::KeyRelease) {
        return d->buttonReleased(event);
    } else {
        return d->buttonPressed(event);
    }
}

/*!
Commits the word and closes the exact popup if it is visible.
*/
void HbInputPredictionQwertyHandler::commitAndUpdate(const QString& string, int replaceFrom, int replaceLength)
{
    Q_D(HbInputPredictionQwertyHandler);
    HbInputModeHandler::commitAndUpdate(string, replaceFrom, replaceLength);
    d->mInputMethod->closeExactWordPopup();
    d->mExactPopupLaunched = false;  
}

/*!
    this function deletes one character and updates the engine and editor.
*/
void HbInputPredictionQwertyHandler::deleteOneCharacter()
{
    Q_D(HbInputPredictionHandler);
    d->deleteOneCharacter();
}

/*!
 this function is called by HbPredictionHandler when HbPredictionHandler encounters a exact word.
*/
void HbInputPredictionQwertyHandler::processExactWord(QString exactWord)
{
    Q_D(HbInputPredictionQwertyHandler);
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
void HbInputPredictionQwertyHandler::exactWordPopupClosed()
{
    commitExactWord();
}

void HbInputPredictionQwertyHandler::exactWordSelected()
{
    Q_D(HbInputPredictionQwertyHandler);    
    d->commitSecondaryWord();
}

void HbInputPredictionQwertyHandler::sctCharacterSelected(QString character)
{   
    HbInputPredictionHandler::sctCharacterSelected(character);
}

void HbInputPredictionQwertyHandler::smileySelected(QString smiley)
{
    HbInputPredictionHandler::smileySelected(smiley);
}

/*!
Returns true if preidciton engine is available and initialized.
*/
bool HbInputPredictionQwertyHandler::isActive() const
{ 
    Q_D(const HbInputPredictionQwertyHandler);
    return d->mEngine != 0;
}

void HbInputPredictionQwertyHandlerPrivate::deleteOneCharacter()
{
    if (!mEngine && !mInputMethod->focusObject()) {
        return;
    }
    mShowTail = true;
    mShowTooltip = true;
    // A backspace in predictive means updating the engine for the delete key press
    // and get the new candidate list from the engine.
    if ( mEngine->inputLength() >= 1 ) {
        //Only autocomplition part should be deleted when autocompliton part is enable and user pressed a delete key
        //To prevent showing autocompletion part while deleting the characters using backspace key
        mShowTail = false;
        mShowTooltip = false;
        //The assumption here is that with deletion of a character we always
        //can go on with prediction. This is because when we delete a key press
        //we actually reduce ambiguity in the engine and hence we should have
        //some word getting predicted as a result to that.
        mCanContinuePrediction = true;

        int tailLength =  mInputMethod->focusObject()->preEditString().length() - mEngine->inputLength();
        if( true == mExactPopupLaunched ) {
            mEngine->deleteKeyPress();
            mEngine->updateCandidates(mBestGuessLocation);
        }

        mBestGuessLocation = 0;
        if(mCandidates->count() && (mCandidates->count()>mBestGuessLocation) && tailLength <= 0 && false == mExactPopupLaunched) {
            QString currentWord = mCandidates->at(mBestGuessLocation);
            if(currentWord.length()) {
                currentWord.chop(1);
                mEngine->deleteKeyPress();
                //We are not supposed to re-construct the candidate list as deletion
                //does not cause reprediction. Also, candidate list construction is the
                //heaviest operation out of all engine operations.
                (*mCandidates)[mBestGuessLocation] = currentWord;
            } else {
                commit(QString(""),false);
            }

        } else if(!mCandidates->count() && mEngine->inputLength() >= 1) {
            //If Input length greater or equal to one then Append the current word to candidate 
            mCandidates->append(mEngine->currentWord());
        }
        // update the editor with the new preedit text.
        updateEditor();
        return;
    } else {
        // we come here if their is no data in engine.
        // once the word is committed, we can not bring it back to inline edit.
        // so if the engine does not have any data, we just send backspace event to the editor.
        Q_Q(HbInputPredictionQwertyHandler);
        QKeyEvent event = QKeyEvent(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);       
        q->sendAndUpdate(event);
        event = QKeyEvent(QEvent::KeyRelease, Qt::Key_Backspace, Qt::NoModifier);
        q->sendAndUpdate(event);
        return;
    }
}

void HbInputPredictionQwertyHandler::setPrimaryCandidateMode(HbPrimaryCandidateMode mode)
{
    Q_D(HbInputPredictionQwertyHandler);
    if(mode == HbPrimaryCandidateModeExactTyping) {
        d->mShowExactWordInTooltip = false;
    } else {
        d->mShowExactWordInTooltip = true;
    }
    // we need to close the exact word popup when this happens.
    d->mInputMethod->closeExactWordPopup();
    d->mExactPopupLaunched = false;
}

void HbInputPredictionQwertyHandlerPrivate::updateEditor()
{
    Q_Q(HbInputPredictionQwertyHandler);
    if (!mEngine) {
        return;
    }
    HbInputFocusObject *focusedObject = 0;
    QList<QInputMethodEvent::Attribute> list;
    focusedObject = mInputMethod->focusObject();
    QTextCharFormat underlined;    
    underlined.setFontUnderline(true);

    Q_ASSERT(focusedObject);
    if (mEngine->inputLength() == 0) {
        QInputMethodEvent event(QString(), list);
        q->sendAndUpdate(event);
    } else {
        if (mCandidates->count() > 0) {
            // index of the candidate that is shown on the typing line
            mPrimaryCandidateIndex = mBestGuessLocation;
            // index of the candidate that is shown in the tooltip (if any)
            mSecondaryCandidateIndex = 0;    
            QString bestGuessWord = mCandidates->at(mBestGuessLocation);
            QString exactTypedWord = mCandidates->at(0);
            bool hasAutocompletionPart = (bestGuessWord.length() > exactTypedWord.length()) && (bestGuessWord.left(exactTypedWord.length()) == exactTypedWord);            
            // when we are showing the autocompletion part we generally do not show the tooltip,
            // both primary and secondary candidates are the same.
            if(hasAutocompletionPart && mAutocompletionEnabled && mShowTail) {
                mPrimaryCandidateIndex = mSecondaryCandidateIndex = mBestGuessLocation;                
            }
            // in case of exact tying, exact typed word in the primary candidate
            // and the best predicted word is the secondary candidate
            if(!mShowExactWordInTooltip && !(hasAutocompletionPart && mAutocompletionEnabled)) {
                mPrimaryCandidateIndex = 0;
                mSecondaryCandidateIndex = mBestGuessLocation;
            }
            if (hasAutocompletionPart && mShowTail && mAutocompletionEnabled) {
                // showing the word with auto completion
                int taillength = bestGuessWord.length() - exactTypedWord.length();
                // TODO: Color from skin should be used
                QColor col = HbColorScheme::color("qtc_input_hint_normal");
                QBrush brush(col);
                // underline formatting is applied to only the input length part of the word
                QInputMethodEvent::Attribute textstyle(QInputMethodEvent::TextFormat, 0, mEngine->inputLength(), underlined);
                list.append(textstyle);

                QTextCharFormat gray;
                gray.setForeground(brush);
                if((focusedObject->object())->inherits("QGraphicsWebView") || (focusedObject->object())->inherits("QWebView")) {
                    //QGraphicsWebView does not handle partial input length formatting well. Causes crash, a temporary fix provided,
                    //This makes the whole text field grey insted of just the auto-completion part. Anyways, it does not cause crash.
                    //This should be treated as a work around till QGraphicsWebView is fixed.
                    list.append(QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, 0, QInputMethodEvent::TextFormat, gray));
                } else {
                    list.append(QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, mEngine->inputLength(), taillength, gray));
                }               
                list.append(QInputMethodEvent::Attribute(QInputMethodEvent::Cursor, mEngine->inputLength(), 0, 0));
                // the best guess word is sent to the editor
                QInputMethodEvent event(bestGuessWord, list);
                focusedObject->sendEvent(event);
            } else {
                QInputMethodEvent::Attribute textstyle(QInputMethodEvent::TextFormat, 0, mCandidates->at(mPrimaryCandidateIndex).length(), underlined);
                list.append(textstyle);
                list.append(QInputMethodEvent::Attribute(QInputMethodEvent::Cursor, mCandidates->at(mPrimaryCandidateIndex).length(), 0, 0));
                QInputMethodEvent event(mCandidates->at(mPrimaryCandidateIndex), list);
                focusedObject->sendEvent(event);
            }

            if (mShowTooltip && mPrimaryCandidateIndex != mSecondaryCandidateIndex) {                
                q->processExactWord(mCandidates->at(mSecondaryCandidateIndex));
            } else {
                QString empty;
                q->processExactWord(empty);
            }
        } else {
            QInputMethodEvent event(QString(""), list);
            focusedObject->sendEvent(event);
        }
    }
}

void HbInputPredictionQwertyHandlerPrivate::commitSecondaryWord()
{
    if(mCandidates->count()>mSecondaryCandidateIndex) {
        QString commitString = mCandidates->at(mSecondaryCandidateIndex);
        commit(commitString, true, false);
    }
}

void HbInputPredictionQwertyHandlerPrivate::showExactWordPopupIfNeeded()
{
    Q_Q(HbInputPredictionHandler);
    if (mShowTooltip && mCandidates && mCandidates->count() > mSecondaryCandidateIndex && mPrimaryCandidateIndex != mSecondaryCandidateIndex) {                
        q->processExactWord(mCandidates->at(mSecondaryCandidateIndex));
    } else {
        QString empty;
        q->processExactWord(empty);
    }
}

QString HbInputPredictionQwertyHandlerPrivate::getCommitString()
{
    QString commitString;
    if(mCandidates->count() <= mPrimaryCandidateIndex) {
        commitString = mCandidates->at(0);
    } else {
        commitString = mCandidates->at(mPrimaryCandidateIndex);
    }
    return commitString;
}

void HbInputPredictionQwertyHandler::setTypingCorrectionLevel(HbTypingCorrectionLevel correctionLevel)
{
    Q_D(HbInputPredictionQwertyHandler);
    HbPredictionEngine::HbErrorCorrectionLevel errorCorrectionLevel = HbPredictionEngine::HbErrorCorrectionLevelMedium;
    if(d->mCorrectionLevel == correctionLevel) {
        return;
    }
    d->mCorrectionLevel = correctionLevel;
    switch(correctionLevel) {
        case HbTypingCorrectionLevelLow:
            errorCorrectionLevel = HbPredictionEngine::HbErrorCorrectionLevelLow;
            break;
        case HbTypingCorrectionLevelMedium:
            errorCorrectionLevel = HbPredictionEngine::HbErrorCorrectionLevelMedium;
            break;
        case HbTypingCorrectionLevelHigh:
            errorCorrectionLevel = HbPredictionEngine::HbErrorCorrectionLevelHigh;
            break;
        default:
            break;
    }
    if(d->mEngine) {
        d->mEngine->setErrorCorrectionLevel(errorCorrectionLevel);
    }
}

//EOF
