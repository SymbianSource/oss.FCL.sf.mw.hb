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
#include <QGraphicsScene>

#include <hbinputmethod.h>
#include <hbinputkeymapfactory.h>
#include <hbinputpredictionengine.h>
#include <hbinputsettingproxy.h>
#include <hbinputvkbhost.h>
#include <hbinputdialog.h>
#include <hbaction.h>
#include "virtual12key.h"

#include "hbinputprediction12keyhandler.h"
#include "hbinputpredictionhandler_p.h"
#include "hbinputabstractbase.h"

#define HbDeltaHeight 3.0
#define MAXUDBWORDSIZE 64

class HbInputPrediction12KeyHandlerPrivate: public HbInputPredictionHandlerPrivate
{
    Q_DECLARE_PUBLIC(HbInputPrediction12KeyHandler)

public:
    HbInputPrediction12KeyHandlerPrivate();
    ~HbInputPrediction12KeyHandlerPrivate();

    bool buttonReleased(const QKeyEvent *keyEvent);
    bool buttonPressed(const QKeyEvent *keyEvent);
    void _q_timeout();
    void launchSpellDialog(QString customWord);
    void getSpellDialogPositionAndSize(QPointF & pos,QSizeF & size,QRectF & geom);
    void cancelButtonPress();
public:
    int mLastKey;
    bool mButtonDown;
    QChar mCurrentChar;
    bool mLongPressHappened;
    bool mShiftKeyDoubleTap;
};

HbInputPrediction12KeyHandlerPrivate::HbInputPrediction12KeyHandlerPrivate()
:mLastKey(0),
mButtonDown(false),
mCurrentChar(0),
mLongPressHappened(false),
mShiftKeyDoubleTap(false)
{
}

HbInputPrediction12KeyHandlerPrivate::~HbInputPrediction12KeyHandlerPrivate()
{
}

void HbInputPrediction12KeyHandlerPrivate::_q_timeout()
{
    qDebug("HbInputPrediction12KeyHandlerPrivate::_q_timeout()");
    Q_Q(HbInputPrediction12KeyHandler);

    // let's stop the timer first.
    mTimer->stop();

    //Long key press number key is applicable to all keys
    if (mButtonDown) {	
        if (mLastKey == Qt::Key_Asterisk) {
			//Remove the "?" mark if present
			if(!mCanContinuePrediction && (*mCandidates)[mBestGuessLocation].endsWith('?')) {	
                (*mCandidates)[mBestGuessLocation].chop(1);
                updateEditor();
                mCanContinuePrediction = true;
			}
            mInputMethod->switchMode(mLastKey);
        } else if (mLastKey == Qt::Key_Shift) {
            mInputMethod->switchMode(Qt::Key_Shift);
            mLongPressHappened = true;
        } else if (mLastKey == Qt::Key_Control) {
            mInputMethod->selectSpecialCharacterTableMode();
        } else {
            //With a long key press of a key, numbers are supposed to be entered.
            //When the existing input (along with the short key press input of the
            //existing key) results in tail (i.e. autocompletion), we need to accept
            //the short key press as well as the tail. In case of ? delete the questionmark and add the number value.          
            //mTailShowing = false;            
            // Delete "?" entered
            if (!mCanContinuePrediction) {
                deleteOneCharacter();
            }
			if (mLastKey != Qt::Key_Delete) {
				q->commitFirstMappedNumber(mLastKey);
			}
            mLongPressHappened = true;
        }
    }
}

bool HbInputPrediction12KeyHandlerPrivate::buttonPressed(const QKeyEvent *keyEvent)
{
    mLongPressHappened = false;
    HbInputFocusObject *focusObject = 0;
    focusObject = mInputMethod->focusObject();
    if (!focusObject) {
        return false;
    }

    int buttonId = keyEvent->key();

    
    if (buttonId == Qt::Key_Control) {
        mLastKey = buttonId;
        mButtonDown = true;
        mTimer->start(HbLongPressTimerTimeout);
        return true;
    } else if (buttonId == Qt::Key_Shift) {		
    // if we get a second consequtive shift key press, 
    // we want to handle it in buttonRelease
        if (mTimer->isActive() && (mLastKey == buttonId)){
            mShiftKeyDoubleTap = true;
        }
        if (!mTimer->isActive()) {            		
            mTimer->start(HbLongPressTimerTimeout);
        }
        mLastKey = buttonId;
        mButtonDown = true;
        return true;
    }

    mLastKey = buttonId;
    mButtonDown = true;
    
    // custom button should not start timer.
    if ((buttonId & CUSTOM_INPUT_MASK) != CUSTOM_INPUT_MASK) {
        mTimer->start(HbLongPressTimerTimeout);
    }
    return false;
}

/*!
Handles the key release events from the VKB. Launches the SCT with key release event of
asterisk.
*/
bool HbInputPrediction12KeyHandlerPrivate::buttonReleased(const QKeyEvent *keyEvent)
{
    Q_Q(HbInputPrediction12KeyHandler);
    
    if(!mButtonDown || mLongPressHappened){
        return false;
    }

    // since button is released we can set buttonDown back to false.
    mButtonDown = false;
    int buttonId = keyEvent->key(); 
    // it was a long press on sct swith button. so just return form here.
    if (!mTimer->isActive() && buttonId == Qt::Key_Control) {
        return true;
    }

    // Sym key is handled in this class it self, so not passing it to 
    // the base mode handlers.	
    if ( buttonId == Qt::Key_Control) {
        //Same SYM key is used for launching candidate list (long key press)
        //and also for SCT. So, do not launch SCT if candidate list is already launched.
        mInputMethod->switchMode(buttonId);
        return true;
    } 
    /* Behavior of Short Press of Asterisk Key when in inline editing state 
		- Should launch Candidate List if we can continue with prediction i.e. "?" is not displayed
		- Should launch Spell Query Dialog if we cannot continue with prediction 
	- Behavior of Short Press of Asterisk Key when not in inline editing state 
		- Should launch SCT
	*/
    else if (buttonId == Qt::Key_Asterisk ) {
		if(!mCanContinuePrediction && (*mCandidates)[mBestGuessLocation].endsWith('?')) {			
            //Remove the "?" mark
            (*mCandidates)[mBestGuessLocation].chop(1);
            updateEditor();
            q->processCustomWord((*mCandidates)[mBestGuessLocation]);
            mCanContinuePrediction = true;
		}
		else
			mInputMethod->starKeySelected();
        return true;
    }	
    else if (buttonId == Qt::Key_Return) {
        mInputMethod->closeKeypad();
        return true;
    }
    if (buttonId == Qt::Key_Shift) {
        // single tap of shift key toggles prediction status in case insensitive languages
        if (!HbInputSettingProxy::instance()->globalInputLanguage().isCaseSensitiveLanguage()) {
            HbInputSettingProxy::instance()->togglePrediction();
        } else {
            if (mShiftKeyDoubleTap) {
                mTimer->stop(); 
                mShiftKeyDoubleTap = false;	
                //mShowTail = false;      
                if (HbInputSettingProxy::instance()->globalInputLanguage()== mInputMethod->inputState().language()) {
                    // in latin variants , double tap of shift key toggles the prediction status	
                    // revert back to the old case as this is a double tap 
                    // (the case was changed on the single tap)
                    updateTextCase();				 
                    HbInputSettingProxy::instance()->togglePrediction();
                } else {
                    // if the global language is different from the input mode language, we should 
                    // go back to the root state
                    // e.g. double tap of hash/shift key is used to change 
                    // to chinese input mode from latin input mode
                    HbInputState rootState;
                    mInputMethod->editorRootState(rootState);
                    mInputMethod->activateState(rootState); 		
                }
            } else {
                updateTextCase();
                if( !mTimer->isActive()){
                    mTimer->start();
                }
            }
        }
        return true;
    }

    // text input happens on button release		
    if (q->HbInputPredictionHandler::filterEvent(keyEvent)) {
        return true;
    }	
    return false;
}

void HbInputPrediction12KeyHandlerPrivate::launchSpellDialog(QString editorText)
{
    HbInputFocusObject *focusObject = mInputMethod->focusObject();
    if (!focusObject) {
        return;
    }
        QPointer<QObject> focusedQObject = focusObject->object();
	// store the current focused editor 
			 
    HbTextCase currentTextCase = focusObject->editorInterface().textCase();
    mEngine->clear();
    mCanContinuePrediction = true;
    // close the keypad before showing the spell dialog
	HbVkbHost *vkbHost = focusObject->editorInterface().vkbHost();
    if (vkbHost && vkbHost->keypadStatus() != HbVkbHost::HbVkbStatusClosed) {
        vkbHost->closeKeypad(true);
    }
    // create the spell dialog
    HbInputDialog *spellDialog = new HbInputDialog();
    spellDialog->setInputMode(HbInputDialog::TextInput);
    spellDialog->setPromptText("");
    spellDialog->setValue(QVariant(editorText));
    QSizeF  dialogSize = spellDialog->size();
    QPointF dialogPos = spellDialog->pos();
    QRectF geom = spellDialog->geometry();
    
    //set the spell dialog position
    getSpellDialogPositionAndSize(dialogPos,dialogSize, geom);
    geom.setHeight(dialogSize.height());
    geom.setWidth(dialogSize.width());
    spellDialog->setGeometry(geom);
    spellDialog->setPos(dialogPos);

    // change the focus to spell dialog editor
    HbLineEdit *spellEdit = spellDialog->lineEdit();

    if (spellEdit) {
        spellEdit->setFocus();
        spellEdit->clearFocus();
        spellEdit->setFocus();
        spellEdit->setMaxLength(MAXUDBWORDSIZE);
        spellEdit->setSmileysEnabled(false);
        HbEditorInterface eInt(spellEdit);
		spellEdit->setInputMethodHints(spellEdit->inputMethodHints() | Qt::ImhNoPredictiveText);
        eInt.setTextCase(currentTextCase);
    }
    // execute the spell dialog
    HbAction *act = spellDialog->exec();
 
        //create new focus object and set the focus back to main editor
        HbInputFocusObject *newFocusObject = new HbInputFocusObject(focusedQObject);
	   
		
        newFocusObject->releaseFocus();
        newFocusObject->setFocus();

	    HbAbstractEdit *abstractEdit = qobject_cast<HbAbstractEdit*>(focusedQObject);
        if(abstractEdit) {
            abstractEdit->setCursorPosition(abstractEdit->cursorPosition());
        }

        mInputMethod->setFocusObject(newFocusObject);
        mInputMethod->focusObject()->editorInterface().setTextCase(currentTextCase);

    if (act->text() == spellDialog->primaryAction()->text()) {
        commit(spellDialog->value().toString() , true, true);
    } else if (act->text() == spellDialog->secondaryAction()->text()) {
    //update the editor with pre-edit text
        mEngine->setWord(editorText);
        bool used = false;	 
        mEngine->updateCandidates(mBestGuessLocation, used);
		mShowTail = false;
		updateEditor();
	}  
    delete spellDialog;
}

void HbInputPrediction12KeyHandlerPrivate::getSpellDialogPositionAndSize(QPointF & pos,QSizeF & size, QRectF &geom)
{
    QRectF cursorRect = mInputMethod->focusObject()->microFocus(); // from the top of the screen
    pos = QPointF(cursorRect.bottomLeft().x(),cursorRect.bottomLeft().y());
    qreal heightOfTitlebar = 80.0; // Using magic number for now...
    qreal screenHeight = (qreal)HbDeviceProfile::current().logicalSize().height();
    if( ((screenHeight - cursorRect.bottomLeft().y()) > (cursorRect.y() - heightOfTitlebar))
        || ((screenHeight - cursorRect.bottomLeft().y() + HbDeltaHeight ) > geom.height()) ) {
        // this means there is amore space below inline text than at the top or we can fit spell Dialog
        // below inline text
        pos.setY(cursorRect.bottomLeft().y() + HbDeltaHeight);
        size.setHeight(screenHeight - pos.y());
    } else {
        // this means there is amore space above inline text than below it
        pos.setY(cursorRect.y() - geom.height() - HbDeltaHeight);
        if (pos.y() < heightOfTitlebar) {
            // this means that spell dialog can not be fit in from top of inline text, we need to trim it
            pos.setY(heightOfTitlebar);
        }
        size.setHeight(cursorRect.y() - heightOfTitlebar - HbDeltaHeight);
    }
    if ( size.height() > geom.height()) {
        size.setHeight(geom.height());
    }
    if ((pos.x() + size.width()) > (qreal)HbDeviceProfile::current().logicalSize().width()) {
        // can not fit spell dialog to the right side of inline edit text.
        pos.setX((qreal)HbDeviceProfile::current().logicalSize().width()- size.width());
    }
}

void HbInputPrediction12KeyHandlerPrivate::cancelButtonPress()
{
    mTimer->stop();
    mButtonDown = false;
    mLastKey = 0;
    mShiftKeyDoubleTap = false;
}

HbInputPrediction12KeyHandler::HbInputPrediction12KeyHandler(HbInputAbstractMethod *inputMethod)
    :HbInputPredictionHandler(* new HbInputPrediction12KeyHandlerPrivate, inputMethod)
{
    Q_D(HbInputPrediction12KeyHandler);
    d->q_ptr = this;
}

HbInputPrediction12KeyHandler::~HbInputPrediction12KeyHandler()
{
}

/*!
    this function lists different modes.
*/
void HbInputPrediction12KeyHandler::listInputModes(QVector<HbInputModeProperties>& modes) const
{
    Q_UNUSED(modes); 
}

/*!
    filterEvent to handler keypress/release events.
*/
bool HbInputPrediction12KeyHandler::filterEvent(const QKeyEvent * event)
{
    Q_D(HbInputPrediction12KeyHandler);
    HbInputFocusObject *focusObject = 0;
    focusObject = d->mInputMethod->focusObject();

    //If there was a handling for empty candidate-list, i.e. the engine did not predict
    //any meaningful word for the input sequence.
    if(!d->mCanContinuePrediction) {
        int eventKey = event->key();
        switch(eventKey) {
        case Qt::Key_0:        
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
            if(event->type() == QEvent::KeyRelease && d->mShiftKeyDoubleTap) {
                //Remove the "?" mark
                deleteOneCharacter();
                }
            }
        //For the following set of keys, it does not matter.
        case Qt::Key_Backspace:
        case Qt::Key_Delete:
        case Qt::Key_Return:
        case Qt::Key_Enter:
		case Qt::Key_Asterisk:
            break;
        /* Behavior for other keys i.e. from key1 to key9 - 
        To start the long press timer as we need to handle long press functionality i.e Enter corresponding number mapped to a key */
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9: {
            if (event->type() == QEvent::KeyRelease) {
                d->mButtonDown = false;
            } else {
                d->mButtonDown = true;			
                d->mLastKey = event->key();		
                // start Long Press timer as corresponding number mapped to a key should be allowed to enter
                d->mTimer->start(HbLongPressTimerTimeout);					
            }
            return true;
        }
        //The default behavior for any other key press is just to consume the key event and
        //not to do anything.
        default: {
            return true;
            }
        }
    }

    // prediction mode can't handle Qt::Key_0, so we will emit a passFilterEvent
    // this signal must be connected to by the plugin to a modehandler.
    // which can handle it.
    if (event->key() == Qt::Key_0) {
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
        if (event->type() == QEvent::KeyRelease) {
            return d->buttonReleased(event);
        } else {
            return d->buttonPressed(event);
        }
    }
}

/*!
Action handler 
*/
bool HbInputPrediction12KeyHandler::actionHandler(HbInputModeAction action)
{
    Q_D(HbInputPrediction12KeyHandler);
    bool ret = true;
    switch (action) {
        case HbInputModeActionReset:
            HbInputPredictionHandler::actionHandler(HbInputModeActionReset);
            d->mTimer->stop();
            break;
        case HbInputModeActionCancelButtonPress:
            d->cancelButtonPress();
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

void HbInputPrediction12KeyHandler::mouseHandler(int cursorPosition, QMouseEvent* mouseEvent)
{
    Q_D(HbInputPrediction12KeyHandler);
    if(d->mLastKey == Qt::Key_0) {
        // zero key handling is done in the basic handler
        emit passActionHandler(HbInputModeActionCommit);
    } else {
        HbInputPredictionHandler::mouseHandler(cursorPosition,mouseEvent);
    }
}


/*!
Returns true if preidciton engine is available and initialized.
*/
bool HbInputPrediction12KeyHandler::isActive() const
{ 
    Q_D(const HbInputPrediction12KeyHandler);
    return d->mEngine != 0;
}

void HbInputPrediction12KeyHandler::processCustomWord(QString customWord)
{
    Q_D(HbInputPrediction12KeyHandler);
    if (customWord.size()) {
        d->launchSpellDialog(customWord);
    }
    return;	  
}
//EOF
