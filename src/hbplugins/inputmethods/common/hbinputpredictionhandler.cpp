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
#include <QTextCharFormat>
#include <hbinputpredictionengine.h>
#include <hbinputsettingproxy.h>
#include <hbinputpredictionfactory.h>
#include <hbinputkeymap.h>
#include <hbinputkeymapfactory.h>
#include <hbinputdialog.h>
#include <hbaction.h>
#include <hbinputvkbhost.h>
#include <hbcolorscheme.h>
#include <hbinpututils.h>
#include <hbinputbutton.h>
#include "hbinputspellquerydialog.h"
#include "../touchinput/virtualqwerty.h"

#include "hbinputpredictionhandler_p.h"
#include "hbinputabstractbase.h"

#define HbDeltaHeight 3.0

HbInputPredictionHandlerPrivate::HbInputPredictionHandlerPrivate()
    :mEngine(0),
    mCandidates(0),
    mBestGuessLocation(0),
    mShowTail(true),
    mTailShowing(false),
    mAutoAddedSpace(true),
    mCanContinuePrediction(true),
    mShowTooltip(true),
    mSpellQueryDialog(0)
{
}

HbInputPredictionHandlerPrivate::~HbInputPredictionHandlerPrivate()
{
    if (mCandidates) {
        delete mCandidates;
        mCandidates = 0;
    }
}

void HbInputPredictionHandlerPrivate::deleteOneCharacter()
{
    mShowTail = true;
    mShowTooltip = true;
    // A backspace in predictive means updating the engine for the delete key press
    // and get the new candidate list from the engine.
    if ( mEngine->inputLength() >= 1 ) {
        //Only autocomplition part should be deleted when autocompliton part is enable and user pressed a delete key
        if(false == mTailShowing) {
            mEngine->deleteKeyPress( this );
        }
        //To prevent showing autocompletion part while deleting the characters using backspace key
        mShowTail = false;
        mShowTooltip = false;
		bool unused = false;
        mEngine->updateCandidates(mBestGuessLocation, unused);
		//If Input length greater or equal to one then Append the current word to candidate 
		if (!mCandidates->count() && mEngine->inputLength() >= 1) {
			mCandidates->append(mEngine->currentWord());
        }
		mCanContinuePrediction = true;
		// update the editor with the new preedit text.
        updateEditor();
        return;
    } else {
        // we come here if their is no data in engine.
        // once the word is committed, we can not bring it back to inline edit.
        // so if the engine does not have any data, we just send backspace event to the editor.
        Q_Q(HbInputPredictionHandler);
        QKeyEvent event = QKeyEvent(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);		
        q->sendAndUpdate(event);
        event = QKeyEvent(QEvent::KeyRelease, Qt::Key_Backspace, Qt::NoModifier);
        q->sendAndUpdate(event);
        return;
    }
}

void HbInputPredictionHandlerPrivate::commitAndAppendCharacter(QChar character)
{
    if (!mEngine) {
        return;
    }

    HbInputFocusObject* focusedObject = 0;
    focusedObject = mInputMethod->focusObject();
    if (!focusedObject) {
        return;
    }

    QString commitString;
    if (mEngine->inputLength() > 0 && mCandidates->count() > 0) {
        if(mCandidates->count() <= mBestGuessLocation) {
            commitString = mCandidates->at(0);
            mEngine->addUsedWord(mCandidates->at(0));
        } else if (mShowTail == false) {
            commitString = mCandidates->at(mBestGuessLocation).left(mEngine->inputLength());
            mEngine->addUsedWord(mCandidates->at(mBestGuessLocation).left(mEngine->inputLength()));
        } else {
            commitString = mCandidates->at(mBestGuessLocation);
            mEngine->addUsedWord(mCandidates->at(mBestGuessLocation));
        }
        if (character == QChar(' ') || character == QChar('\n')) {
            mAutoAddedSpace = true;
        }
        commit(commitString);
    } else {
        mAutoAddedSpace = false;
    }
    commitString = character;
    commit(commitString);
}

/*!
Commits the candidate upon closing of the candidate list. Now can the closing key be anything other than
just selection. Need to evaluate this. When the candidate list is visible, the current implementation of the
candidate list does not allow the virtual keypad to be clicked.
*/
void HbInputPredictionHandlerPrivate::candidatePopupClosed(int closingKey, const QString& activatedText)
{
    if (!mEngine) {
        return;
    }

    HbInputFocusObject* focusedObject = 0;
    focusedObject = mInputMethod->focusObject();
    if (!focusedObject) {
        return;
    }

    QString commitString  = activatedText;
    if (closingKey == Qt::Key_0 || closingKey == HbInputButton::ButtonKeyCodeSpace) {
        commitString = activatedText+' ';
    } else if (closingKey == HbInputButton::ButtonKeyCodeEnter) {
        commitString = activatedText;
        commit(commitString);
        commitString = '\n';
    }

    commit(commitString,true);
}

/*!
This function shows the exact popup if needed.
*/
void HbInputPredictionHandlerPrivate::showExactWordPopupIfNeeded()
{
    Q_Q(HbInputPredictionHandler);
    if (mShowTooltip && mBestGuessLocation > 0 &&  mCandidates->count() > 0
    	&& mCandidates->at(0).mid(0, mEngine->inputLength()) \
        != mCandidates->at(mBestGuessLocation).mid(0, mEngine->inputLength())) {                
        q->processExactWord(mCandidates->at(0));
    } else {
        QString empty;
        q->processExactWord(empty);
    }
}

QList<HbKeyPressProbability> HbInputPredictionHandlerPrivate::probableKeypresses()
{
    if(HbInputUtils::isQwertyKeyboard(mInputMethod->inputState().keyboard())) {
        //Useof the concrete input method implementations make the modehandlersin flexible.
        //Later an intermediate abstract class needsto be introduced.
        HbVirtualQwerty * qwertyInputMethod = qobject_cast<HbVirtualQwerty*>(mInputMethod);
        if(qwertyInputMethod) {
            return qwertyInputMethod->probableKeypresses();
        }
    }
    return QList<HbKeyPressProbability>();
}

/*!
This method updates the editor contents based on the candidates available in the candidate list.
*/
void HbInputPredictionHandlerPrivate::updateEditor()
{
    Q_Q(HbInputPredictionHandler);
    if (!mEngine) {
        return;
    }

    QList<QInputMethodEvent::Attribute> list;
    QTextCharFormat underlined;
    HbInputFocusObject *focusedObject = 0;
    underlined.setFontUnderline(true);
    QInputMethodEvent::Attribute textstyle(QInputMethodEvent::TextFormat, 0, mEngine->inputLength(), underlined);
    list.append(textstyle);

    focusedObject = mInputMethod->focusObject();
    Q_ASSERT(focusedObject);

    if (mEngine->inputLength() == 0) {
        QInputMethodEvent event(QString(), list);
        q->sendAndUpdate(event);
    } else {
        if (mCandidates->count() > mBestGuessLocation) {
            int taillength = mCandidates->at(mBestGuessLocation).length() - mEngine->inputLength();
            if (taillength > 0 && mShowTail) {
                // TODO: Color from skin should be used
				QColor col = HbColorScheme::color("qtc_input_hint_normal");
                QBrush brush(col);
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
                QInputMethodEvent event(mCandidates->at(mBestGuessLocation), list);
                focusedObject->sendEvent(event);
                mTailShowing = true;
            } else {
				list.append(QInputMethodEvent::Attribute(QInputMethodEvent::Cursor, mCandidates->at(mBestGuessLocation).length(), 0, 0));
                QInputMethodEvent event(mCandidates->at(mBestGuessLocation).left(mEngine->inputLength()), list);
                focusedObject->sendEvent(event);
                mTailShowing = false;
            }
            if (mShowTooltip && mBestGuessLocation > 0 && mCandidates->at(0).mid(0, mEngine->inputLength()) \
                != mCandidates->at(mBestGuessLocation).mid(0, mEngine->inputLength())) {                
                    q->processExactWord(mCandidates->at(0));
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

bool HbInputPredictionHandlerPrivate::filterEvent(const QKeyEvent * event)
{
    HbInputFocusObject* focusObject = 0;
    focusObject = mInputMethod->focusObject();
    //If the focused object is NULL or the key event is improper, can not continue
    if(!focusObject || event->key() < 0 ||
        event->key() == HbInputButton::ButtonKeyCodeCustom) {
        return false;
    }

    Q_Q(HbInputPredictionHandler);
    if (!mEngine) {
        return false;
    }

    bool ret = false;
    mModifiers = Qt::NoModifier;   
    mCanContinuePrediction = true;
    switch (event->key()) {
    case Qt::Key_Backspace:
    case HbInputButton::ButtonKeyCodeDelete:
        {
            QString currentSelection = focusObject->inputMethodQuery(Qt::ImCurrentSelection).toString();
            if(currentSelection.length()) {
                QKeyEvent event = QKeyEvent(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);		
                q->sendAndUpdate(event);
                event = QKeyEvent(QEvent::KeyRelease, Qt::Key_Backspace, Qt::NoModifier);
                q->sendAndUpdate(event);
            } else {
                deleteOneCharacter();
            }
		}
        ret = true;
        break;
    case Qt::Key_Period: // TODO: better handling for punctuation
    case Qt::Key_Comma: { // Need to take fn, shift etc. in account
            HbModifier modifier = HbModifierNone;
            int currentTextCase = focusObject->editorInterface().textCase();
            if ( HbTextCaseUpper == currentTextCase || HbTextCaseAutomatic == currentTextCase ) {
                modifier = HbModifierShiftPressed;
            }
            QString qc;
            const HbMappedKey* mappedKey = mKeymap->keyForKeycode(mInputMethod->inputState().keyboard(), event->key());

            if (mappedKey) {
                if (modifier == HbModifierNone) {
                    qc = mappedKey->characters(HbModifierNone).left(1);
                } else if (modifier == HbModifierShiftPressed) {
                    qc = mappedKey->characters(HbModifierShiftPressed).left(1);
                }
            }

            if (mEngine->inputLength() == 0) {
                QList<QInputMethodEvent::Attribute> list;
                QInputMethodEvent event(QString(), list);
                if (mAutoAddedSpace) {
                    int cursorPos = mInputMethod->focusObject()->inputMethodQuery(Qt::ImCursorPosition).toInt();
                    QString text = mInputMethod->focusObject()->inputMethodQuery(Qt::ImSurroundingText).toString();
                    if (cursorPos > 0 && text.at(cursorPos-1).isSpace()) {
                        event.setCommitString(qc, -1, 1);
                    } else {
                        event.setCommitString(qc);
                    }
                } else {
                    event.setCommitString(qc);
                }
                mAutoAddedSpace = false;
                q->sendAndUpdate(event);
            } else {
                // Fix for input stopping after ,. keys in qwerty predictive
                commitAndAppendCharacter(qc.at(0));
                QString empty;
                q->processExactWord(empty);
            }
            ret = true;
        }
        break;
    case HbInputButton::ButtonKeyCodeEnter:
    case HbInputButton::ButtonKeyCodeSpace:
    case Qt::Key_0: {//Space
            // A space means we have to commit the candidates when we are in predictive mode.
            QChar qc(event->key());
            if (qc == Qt::Key_Enter) {
                qc = QChar('\n');  // Editor expects normal line feed.
            } else if (qc == Qt::Key_0) {
                qc = QChar(' ');
            }
            commitAndAppendCharacter(qc);
            // if exact word popup functionality is on then we should inform exact word popup
            // about the space.//++TODO
            QString empty;
            q->processExactWord(empty);
            ret = true;
        }
        break;
    default: {
            mShowTail = true;
            mShowTooltip = true;
            mAutoAddedSpace = false;
            if (q->HbInputModeHandler::filterEvent(event)) {
                return true;
            }
            if (mEngine) {
                int currentTextCase = focusObject->editorInterface().textCase();
                if ( HbTextCaseUpper == currentTextCase || HbTextCaseAutomatic == currentTextCase ) {
                    mModifiers |= Qt::ShiftModifier;
                }
                if (event->text().isEmpty()) {
                    mEngine->appendKeyPress(event->key(), mModifiers, mInputMethod->inputState().textCase(), this);
                } else {
                    mEngine->appendCharacter(event->text().at(0), mInputMethod->inputState().textCase(), this);
                }
                bool isCustomWord = false;
                mEngine->updateCandidates(mBestGuessLocation, isCustomWord);
                //The engine can not predict the word, it is a custom word. Now engine returns a
                //single candidate entry which is actually the previous properly predicted character.
                //TODO: In such a scenario, the prediction should stop and there needs to be some way for
                //the user to tell the correct word which he wants (Some query editor). But, such a thing is
                //not in place and hence the current implementation just appends a "?" and displays the
                //previous word itself.
                if (isCustomWord && mCandidates->count()) {
                    QString replacementText = mCandidates->at(0);//+"?";
                    mCandidates->clear();
                    mCandidates->append(replacementText);
                    //Since there is only one candidate, that's the best prediction.
                    mBestGuessLocation = 0;
                }
                //We need to check and carry out necessary steps if the engine fails to predict any candidate,
                //for the give input sequence.
                handleEmptyCandidateList();
                updateEditor();
                mInputMethod->updateState();
                ret = true;
            }
        }
    }

    return ret;
}

void HbInputPredictionHandlerPrivate::keyHandlerUnicode(const QChar& character)
{
    if (!mEngine) {
        return;
    }
    mEngine->appendCharacter(character, mInputMethod->inputState().textCase(), this);
    bool unused = false;
    mEngine->updateCandidates(mBestGuessLocation, unused);
    //Check if the candidate list is empty and take necessary steps if so.
    handleEmptyCandidateList();
    updateEditor();
}



void HbInputPredictionHandlerPrivate::mouseHandler(int cursorPosition, QMouseEvent* mouseEvent)
{
    Q_Q(HbInputPredictionHandler);
    // we are missing something we must get out.
    if (!mEngine && !mInputMethod->focusObject()) {
        return;
    }

    // mouse move / hover events needs to be ignored.
    if (mouseEvent->type() != QEvent::MouseButtonPress) {
        return;
    }

    //The mouse has been clicked outside of the pre-editing word and hence need to commit the word.
    if ( cursorPosition < 0 || (mCandidates->size()>0 && cursorPosition >= mCandidates->at(mBestGuessLocation).length())) {
        if (mEngine->inputLength() > 0 && mCandidates->count() > 0 && mBestGuessLocation < mCandidates->count()) {
            commit(mCandidates->at(mBestGuessLocation),true);
        }
    } else if (mCandidates->size() > 0) {
        if(!mCanContinuePrediction && (*mCandidates)[mBestGuessLocation].endsWith('?')) {
			// mouse has been clicked on the pre-editing string ends with "?"
            //Remove the "?" mark
            (*mCandidates)[mBestGuessLocation].chop(1);
            updateEditor();
            q->launchSpellQueryDialog();
            mCanContinuePrediction = true;
        } else {

        //The mouse has been clicked on the pre-editing word, launch candidate list
        mInputMethod->launchCandidatePopup(*mCandidates);
        }	
    }
}

void HbInputPredictionHandlerPrivate::init()
{
    mEngine = NULL;
    HbInputLanguage language = HbInputSettingProxy::instance()->globalInputLanguage();
    mEngine = HbPredictionFactory::instance()->predictionEngineForLanguage(language.language());
    if (mEngine && !mCandidates) {
        mCandidates = new QStringList();
    }
}

void HbInputPredictionHandlerPrivate::reset()
{
    if (mEngine) {
        mEngine->clear();
    }
    if (mCandidates) {
        mCandidates->clear();
    }

    mTailShowing = false;
}

void HbInputPredictionHandlerPrivate::commit()
{
    if (mEngine && mEngine->inputLength() > 0 && mCandidates->count() > 0) {
        if(!mCanContinuePrediction) {
            //Remove the "?" mark
            (*mCandidates)[mBestGuessLocation].chop(1);
        }
		
		// Close exact word pop up in qwerty when the word is committed
		if(HbInputUtils::isQwertyKeyboard(mInputMethod->inputState().keyboard())) {
			mInputMethod->closeExactWordPopup();
		}
		
        QString commitString;
        if(mCandidates->count() <= mBestGuessLocation) {
            commitString = mCandidates->at(0);
        } else if (mShowTail == false) {
            commitString = mCandidates->at(mBestGuessLocation).left(mEngine->inputLength());
        } else {
            commitString = mCandidates->at(mBestGuessLocation);
        }
        // need to update the freq information
        mEngine->commit(commitString);
        commit(commitString,false);
    }
}

/*!
This function accepts a Qstring and commits the string to editor. This also clears all the key presses and
candidates from prediction engine
*/
void HbInputPredictionHandlerPrivate::commit(const QString& string, bool addToUsedWordDict, bool isAsync)
{
    Q_Q(HbInputPredictionHandler);
    if(!mCanContinuePrediction) {
        //Remove the "?" mark
        (*mCandidates)[mBestGuessLocation].chop(1);
    }

	// Close exact word pop up in qwerty when the word is committed
	if(HbInputUtils::isQwertyKeyboard(mInputMethod->inputState().keyboard())) {
		mInputMethod->closeExactWordPopup();
	}

    q->commitAndUpdate(string, 0, 0, isAsync);

    if(mEngine) {
        if(addToUsedWordDict && !string.isEmpty()) {
            QString separator = " ";
            QStringList stringList = string.split(separator, QString::SkipEmptyParts);
            foreach (QString str, stringList) {
                mEngine->addUsedWord(str);
            }
        }
        mEngine->clear();
    }

    //Enable the flag after commit
    mCanContinuePrediction = true;
	mTailShowing = false;
}

/*!
This function accepts a QInputMethodEvent and commits the event to editor. This also clears all the key presses and
candidates from prediction engine
Warning: Use this only when you want to commit some string to editor. If you want to send some formating information 
to editor, use sendAndUpdate(QInputMethodEvent & event); The reason is that, this commit function cleans prediction
engine state also.
*/
void HbInputPredictionHandlerPrivate::commit(QInputMethodEvent & event,bool addToUsedWordDict)
{
    Q_Q(HbInputPredictionHandler);

	// Close exact word pop up in qwerty when the word is committed
	if(HbInputUtils::isQwertyKeyboard(mInputMethod->inputState().keyboard())) {
		mInputMethod->closeExactWordPopup();
	}

    q->sendAndUpdate(event);

    if(mEngine) {
        if(addToUsedWordDict && !event.commitString().isEmpty())
            mEngine->addUsedWord(event.commitString());
        mEngine->clear();
    }

    //Enable the flag after commit
    mCanContinuePrediction = true;
	mTailShowing = false;

}

void HbInputPredictionHandlerPrivate::commitExactWord()
{
    if (mEngine && mEngine->inputLength() /*> 0 && mCandidates->count() > 0*/) {
        commit(mCandidates->at(0), true);
    }
}

/*!
This function checks if the constructed candidate list is empty and if so, then 
appends a "?" mark to the best guessed candidate with put the current key input.
*/
void HbInputPredictionHandlerPrivate::handleEmptyCandidateList()
{
    if (!mCandidates->count()) {
        QString existingWord(mEngine->currentWord());
        int lastKeyCode = 0;
        int lastCharacterCode = 0;
        bool isCustomWord = false;
        if(existingWord.length()) {
            lastCharacterCode = existingWord[existingWord.length()-1].unicode();
        }
        const HbKeymap* keymap = HbKeymapFactory::instance()->keymap(HbInputSettingProxy::instance()->globalInputLanguage());
        if(keymap) {
            const HbMappedKey* lastKey = keymap->keyForKeycode(mInputMethod->inputState().keyboard(), lastCharacterCode);
            if (lastKey) {
                lastKeyCode = lastKey->keycode.unicode();
            }
        }
        //Temporarily delete the key press
        mEngine->deleteKeyPress();
        mEngine->updateCandidates(mBestGuessLocation, isCustomWord);
        if (mCandidates->count()){
			(*mCandidates)[mBestGuessLocation] = (*mCandidates)[mBestGuessLocation].left(mEngine->inputLength());
            (*mCandidates)[mBestGuessLocation].append("?");
        } else {
            //Should the mBestGuessLocation not be zero. 
            mBestGuessLocation = 0;
            mCandidates->append(mEngine->currentWord());
            (*mCandidates)[mBestGuessLocation].append("?");
        }
        //Now again append the same key press
        if (lastKeyCode) {
            mEngine->appendKeyPress(lastKeyCode, mModifiers, mInputMethod->inputState().textCase(), this);
        } else {
            mEngine->appendKeyPress(lastCharacterCode, mModifiers, mInputMethod->inputState().textCase(), this);
        }
        mCanContinuePrediction = false;
    } else {
        mCanContinuePrediction = true;
    }
}

void HbInputPredictionHandlerPrivate::setPreEditTextToEditor(QString string)
{
    //update the editor with pre-edit text
    mEngine->setWord(string);
    bool used = false;	 
    mEngine->updateCandidates(mBestGuessLocation, used);
    mTailShowing = true;
    updateEditor();

}

HbInputPredictionHandler::HbInputPredictionHandler(HbInputPredictionHandlerPrivate &dd, HbInputAbstractMethod* inputMethod)
:HbInputModeHandler(dd, inputMethod)
{
    Q_D(HbInputPredictionHandler);
    d->q_ptr = this;
    d->init();
}


HbInputPredictionHandler::~HbInputPredictionHandler()
{
}

/*!
    Mouse handler
*/
void HbInputPredictionHandler::mouseHandler(int cursorPosition, QMouseEvent* mouseEvent)
{
    Q_D(HbInputPredictionHandler);
    d->mouseHandler(cursorPosition, mouseEvent);
}

/*!
    Action Handler
*/
bool HbInputPredictionHandler::actionHandler(HbInputModeAction action)
{
    Q_D(HbInputPredictionHandler);
    bool ret = true;
    switch (action) {
        case HbInputModeActionReset:
        case HbInputModeActionCommit:
        case HbInputModeActionFocusLost: {
            //At the moment we are commiting the text with the autocompletion part as it needs to be committed on clicking outside the editor. 
            //TO DO : When We back to the application by pressing Application key the inline word should not commit and remain in the inline editing
            //d->mShowTail = false;
            d->commit();
            d->reset();
        }
        break;
        case HbInputModeActionDeleteAndCommit: {
            deleteOneCharacter();
            d->commit();
        }
        break;
        case HbInputModeActionSetCandidateList: {
            if (d->mEngine) {
                d->mEngine->setCandidateList(d->mCandidates);
            } else {
                ret = false;
            }
        }
        break;
        case HbInputModeActionSetKeypad: {
            if (d->mEngine) {
                d->mEngine->setKeyboard(d->mInputMethod->inputState().keyboard());
            } else {
                ret = false;;
            }
        }
        break;
        case HbInputModeActionLaunchCandidatePopup:
            if (d->mEngine && d->mCandidates->size()) {
                d->mInputMethod->launchCandidatePopup(*d->mCandidates);
            } else {
                ret = false;
            }
            break;
        case HbInputModeActionSecondaryLanguageChanged:
            if (d->mEngine) {
                d->mEngine->setSecondaryLanguage(HbInputSettingProxy::instance()->globalSecondaryInputLanguage());
            }
            break;
        case HbInputModeActionPrimaryLanguageChanged:
            if(!d->mEngine) {
            d->init();
            }
            if (d->mEngine) {
                d->mEngine->setLanguage(HbInputSettingProxy::instance()->globalInputLanguage());
            }
            break;
        case HbInputModeActionHideTail:
            d->mShowTail = false;
            break;
        case HbInputModeActionCloseSpellQuery:
            closeSpellQueryDialog();
            break;
        default:
            ret = HbInputModeHandler::actionHandler(action);
            break;
    }
    
    return ret;
}

/*!
    returs the true if we have something in in-line editing.
*/
bool HbInputPredictionHandler::isComposing()
{
    Q_D(HbInputPredictionHandler);
    if (d->mEngine && d->mEngine->inputLength()>0) {
        return true;
    }
    return false;
}

/*!
    SLOT to receive input candidate popup close events.
*/
void HbInputPredictionHandler::candidatePopupClosed(QString activatedText, int closingKey)
{
    Q_D(HbInputPredictionHandler);
    d->candidatePopupClosed(closingKey, activatedText);
}

/*!
    SLOT to receive input qwery dialog box events.
*/
void HbInputPredictionHandler::inputQueryPopupClosed(QString activatedWord, int closingKey)
{
    Q_UNUSED(activatedWord);
    Q_UNUSED(closingKey);
}

/*!
    Appends a unicode character and updates the candidates and editor.
*/
void HbInputPredictionHandler::appendUnicodeCharacter(QChar character)
{
    Q_D(HbInputPredictionHandler);
    d->keyHandlerUnicode(character);
}

/*!
    returns supported languages in current engine.
*/
QList<HbInputLanguage> HbInputPredictionHandler::supportedLanguages() const
{
    Q_D(const HbInputPredictionHandler);
    if (d->mEngine) {
       return d->mEngine->languages();
    }

    return QList<HbInputLanguage>();
}

/*!
    filterEvent function to handler key events.
*/
bool HbInputPredictionHandler::filterEvent(const QKeyEvent * event)
{
    Q_D(HbInputPredictionHandler);
    return d->filterEvent(event);
}

/*!
    this SLOT is called when a character in sct is selected.
*/
void HbInputPredictionHandler::sctCharacterSelected(QString character)
{
    Q_D(HbInputPredictionHandler);
	//d->mShowTail = false;
    d->commit();
    HbInputModeHandler::sctCharacterSelected(character);
}

void HbInputPredictionHandler::smileySelected(QString smiley)
{
    Q_D(HbInputPredictionHandler);
    d->commit();
    HbInputModeHandler::smileySelected(smiley);
}
/*!
    this function commits current pre-edit and adds a character at the end.
*/
void HbInputPredictionHandler::commitAndAppendCharacter(QChar character)
{
    Q_D(HbInputPredictionHandler);
    d->commitAndAppendCharacter(character);
}

/*!
    this function deletes one character and updates the engine and editor.
*/
void HbInputPredictionHandler::deleteOneCharacter()
{
    Q_D(HbInputPredictionHandler);
    d->deleteOneCharacter();
}

void HbInputPredictionHandler::commitExactWord()
{
    Q_D(HbInputPredictionHandler);
    d->commitExactWord();
}

/*!
 * A virtual function called by the HbInputPredictionHandler when there is a exact word found
 * and needs to to handled. Called with an empty string when there is no exact word found.
 */
void HbInputPredictionHandler::processExactWord(QString exactWord)
{
    Q_UNUSED(exactWord);
}

void HbInputPredictionHandler::showExactWordPopupIfNeeded()
{
    Q_D(HbInputPredictionHandler);
    d->showExactWordPopupIfNeeded();
}

// Launch spell query dialog in responce to launchSpellQueryDialog signal
void HbInputPredictionHandler::launchSpellQueryDialog()
{
    Q_D(HbInputPredictionHandler);
    HbInputFocusObject *focusedObject = d->mInputMethod->focusObject();
    if(!focusedObject) {
        return;
    }

    // As of now we need to delete and create mSpellQueryDialog every time 
    // we launch it. If we launch the same dialog, keypad does not open sometimes. 
    // Will take sometime to find out the root cause of this, and will fix this. 
    if(d->mSpellQueryDialog) {
        delete d->mSpellQueryDialog;
        d->mSpellQueryDialog =0;
    }
    if(!d->mSpellQueryDialog) {
        d->mSpellQueryDialog = new HbInputSpellQuery(d->mInputMethod,this); 
        d->mSpellQueryDialog->setParent(this);
    }

    QString string;
    if(d->mCandidates && (*(d->mCandidates)).size() >= d->mBestGuessLocation + 1) {
        string = (*(d->mCandidates))[d->mBestGuessLocation].left(d->mEngine->inputLength());
    }
    d->reset();
    d->mSpellQueryDialog->launch(string);

}

// To force the spell query dialog to close.
void HbInputPredictionHandler::closeSpellQueryDialog()
{    
    Q_D(HbInputPredictionHandler);
    if (d->mSpellQueryDialog && d->mSpellQueryDialog->isVisible()) {
        d->mSpellQueryDialog->close();
    }
}

//
void HbInputPredictionHandler::spellQueryDialogClosed(QObject *savedFocusObject,bool isOk,QString string)
{
    Q_D(HbInputPredictionHandler);

    // set the focus back to the editor which caused the launch of spell dialog.
    if(savedFocusObject) {
        HbInputFocusObject *newFocusObject = new HbInputFocusObject(savedFocusObject);
        newFocusObject->releaseFocus();
        newFocusObject->setFocus();
        HbAbstractEdit *abstractEdit = qobject_cast<HbAbstractEdit*>(savedFocusObject);
        if(abstractEdit) {
            abstractEdit->setCursorPosition(abstractEdit->cursorPosition());
        }
        d->mInputMethod->setFocusObject(newFocusObject);
    }

    if (isOk) {
        d->commit(string,true);
    } else {
        //update the editor with pre-edit text
        d->setPreEditTextToEditor(string);
        // This update is need for below usecase
        // Editor is empty => enter some data till their is no match => click on word
        // to lauch spell query => now press cancel => testcase of keypad is uppercase,
        // but it should be lower case
        d->mInputMethod->updateState();
    }
}
// EOF
