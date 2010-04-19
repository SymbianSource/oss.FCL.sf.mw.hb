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

#include <hbinputkeymap.h>
#include <hbinputpredictionfactory.h>

#include "hbinputbasichandler_p.h"
#include "hbinputpredictionengine.h"
#include "hbinputabstractbase.h"

HbInputBasicHandlerPrivate::HbInputBasicHandlerPrivate()
:mAutoCompleter(0)
{
}

HbInputBasicHandlerPrivate::~HbInputBasicHandlerPrivate()
{
}

void HbInputBasicHandlerPrivate::init()
{
}

void HbInputBasicHandlerPrivate::setUpAutoCompleter()
{
    HbInputFocusObject *focusObject = 0;
    focusObject = mInputMethod->focusObject();
    if (!focusObject) {
        return;
    }
    // Check if this is auto completion field and set it up if it is.
    if (focusObject->editorInterface().constraints() & HbEditorConstraintAutoCompletingField) {
        if (!mAutoCompleter) {
            mAutoCompleter = HbPredictionFactory::instance()->createEngine(HbAutoCompleteVendorIdString);
        }

        mInputMethod->closeAutoCompletionPopup();

        if (mAutoCompleter) {
            mAutoCompleter->setExtraUserDictionary(mInputMethod->focusObject()->editorInterface().extraDictionaryId());
        }
    }
}

void HbInputBasicHandlerPrivate::refreshAutoCompleter()
{
    HbInputFocusObject *focusObject = 0;
    focusObject = mInputMethod->focusObject();

    if (!focusObject) {
        return;
    }

    if (focusObject->editorInterface().constraints() & HbEditorConstraintAutoCompletingField &&
        mAutoCompleter) {
        mAutoCompleter->setWord(focusObject->editorSurroundingText());
        mInputMethod->launchAutoCompletionPopup(mAutoCompleter->candidateList());
    }
}

void HbInputBasicHandlerPrivate::deleteCharacterInAutoCompleter()
{
     refreshAutoCompleter();
}

void HbInputBasicHandlerPrivate::addWordInAutoCompleter()
{
    if (mAutoCompleter) {
        mAutoCompleter->commit();
    }
}

void HbInputBasicHandlerPrivate::autoCompletionPopupClosed(QString currentCandidate, int closingKey)
{
    Q_UNUSED(closingKey);
    Q_Q(HbInputBasicHandler);

    HbInputFocusObject *focusObject = 0;
    focusObject = mInputMethod->focusObject();
    if (!focusObject) {
        return;
    }

    if (focusObject->editorInterface().constraints() & HbEditorConstraintAutoCompletingField) {
        if (mAutoCompleter) {
            int inputLength = mAutoCompleter->inputLength();

            mAutoCompleter->clear();
            QString current = currentCandidate;
            q->commitAndUpdate(current, -inputLength, inputLength);
            mAutoCompleter->addUsedWord(current);
        }
    }
}


HbInputBasicHandler::HbInputBasicHandler(HbInputAbstractMethod* inputMethod)
:HbInputModeHandler(*new HbInputBasicHandlerPrivate(), inputMethod)
{
    Q_D(HbInputBasicHandler);
    d->q_ptr = this;
    d->init();
}

HbInputBasicHandler::HbInputBasicHandler(HbInputBasicHandlerPrivate &dd, HbInputAbstractMethod* inputMethod)
:HbInputModeHandler(dd, inputMethod)
{
    Q_D(HbInputBasicHandler);
    d->q_ptr = this;
    d->init();
}


HbInputBasicHandler::~HbInputBasicHandler()
{
}

/*!
    This function handles different keyevents.
*/
bool HbInputBasicHandler::filterEvent(const QKeyEvent *event)
{
    Q_D(HbInputBasicHandler);
    HbInputFocusObject *focusObject = 0;
    focusObject = d->mInputMethod->focusObject();
    if (!focusObject) {
        qDebug("HbInputBasicHandler::filterEvent no focusObject ... failed!!");
        return false;
    }

    bool ret = true;
    switch (event->key()) {
    case Qt::Key_Backspace:
    case Qt::Key_Delete: {
        QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
        sendAndUpdate(keyEvent);
        // pass event to auto completer.
        deleteCharacterInAutoCompleter();
        // return false since the event is sent forward
        ret = false;
        break;
    }
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Space: {
        QChar qc(event->key());
        if (qc == Qt::Key_Enter || qc == Qt::Key_Return) {
            qc = QChar('\n');  // Editor expects normal line feed.
        }
        commitAndUpdate(qc);
        }
        break;
    case Qt::Key_Period:
    case Qt::Key_Comma: {
        QString qc(event->key());
        HbModifier modifier = HbModifierNone;
        int currentTextCase = focusObject->editorInterface().textCase();
        if ( HbTextCaseUpper == currentTextCase || HbTextCaseAutomatic == currentTextCase ) {
            modifier = HbModifierShiftPressed;
        }
        // If shift is pressed, the shifted characters should
        // be input.
        const HbMappedKey* mappedKey = d->mKeymap->keyForKeycode(d->mInputMethod->inputState().keyboard(), event->key());
        if (mappedKey) {
            qc = mappedKey->characters(modifier).left(1);
        }
        commitAndUpdate(qc);
        break;
    }
    default:
        ret = HbInputModeHandler::filterEvent(event);
        break;
    }
    return ret;
}

/*!
 Action Handler for latin basic.
*/
bool HbInputBasicHandler::actionHandler(HbInputModeAction action)
{
    return HbInputModeHandler::actionHandler(action);
}

/*!
Commits the candidate upon closing of the candidate list. Now can the closing key be anything other than
just selection. Need to evaluate this. When the candidate list is visible, the current implementation of the
candidate list does not allow the virtual keypad to be clicked.
*/
void HbInputBasicHandler::autoCompletionPopupClosed(QString currentCandidate, int closingKey)
{
    Q_D(HbInputBasicHandler);
    d->autoCompletionPopupClosed(currentCandidate, closingKey);
}

/*!
 Sets up autocompleter
*/
void HbInputBasicHandler::setUpAutoCompleter()
{
    Q_D(HbInputBasicHandler);
    d->setUpAutoCompleter();
}

/*!
 Appends a character in to the autocompleter engine and shows the popup with the updated list.
*/
void HbInputBasicHandler::refreshAutoCompleter()
{
    Q_D(HbInputBasicHandler);
    d->refreshAutoCompleter();
}

/*!
 Deletes a character and shows the popup with the new list.
*/
void HbInputBasicHandler::deleteCharacterInAutoCompleter()
{
    Q_D(HbInputBasicHandler);
    d->deleteCharacterInAutoCompleter();
}

/*!
 Adds a word to the autocompleter engine.
*/
void HbInputBasicHandler::addWordInAutoCompleter()
{
    Q_D(HbInputBasicHandler);
    d->addWordInAutoCompleter();
}

/*!
Call-back implementation to indicate that a character was selected from the SCT. With this, the character is committed to the
editor and editor is again made to focus.
*/
void HbInputBasicHandler::sctCharacterSelected(QString character)
{       
    HbInputModeHandler::sctCharacterSelected(character);
    refreshAutoCompleter();
}

void HbInputBasicHandler::smileySelected(QString character)
{       
    HbInputModeHandler::smileySelected(character);
    refreshAutoCompleter();
}
//EOF
