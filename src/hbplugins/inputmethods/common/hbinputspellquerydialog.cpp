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
#include <hbinputvirtualkeyboard.h>
#include <hbinputvkbhost.h>
#include <hbaction.h>
#include <hbmainwindow.h>
#include <hbinstance.h>
#include <hbeffect.h>
#include <hbinputregioncollector_p.h>

#include "hbinputspellquerydialog.h"
#include "hbinputmodehandler.h"

#define MAXUDBWORDSIZE 64

HbInputSpellQuery::HbInputSpellQuery(HbInputMethod *inputMethod, HbInputPredictionHandler *predictionHandler)
 : mOwner(inputMethod), mPredictionHandler(predictionHandler), mPrimaryAction(0) 
{
    setInputMode(HbInputDialog::TextInput);
    setPromptText(tr("Word:"));
    setActive(true);
    HbInputRegionCollector::instance()->attach(this);
}

HbInputSpellQuery::~HbInputSpellQuery()
{
}

void HbInputSpellQuery::launch(QString editorText)
{
    HbInputFocusObject *focusObject = 0;
    if (!mOwner || !(focusObject = mOwner->focusObject())) {
        return;
    }
    mSavedState = mOwner->inputState();
    // close the keypad before showing the spell dialog
    HbVkbHost *vkbHost = focusObject->editorInterface().vkbHost();
    if (vkbHost && vkbHost->keypadStatus() != HbVkbHost::HbVkbStatusClosed) {
        vkbHost->closeKeypad();
    }

    setValue(QVariant(editorText));

    // set the spell dialog position
    QPointF newPos((qreal)HbDeviceProfile::current().logicalSize().width() * 0.5,
                   (qreal)HbDeviceProfile::current().logicalSize().height() * 0.5);
    if (vkbHost) {
        newPos.setY(((qreal)HbDeviceProfile::current().logicalSize().height() -
                    vkbHost->activeKeypad()->preferredKeyboardSize().height()) * 0.5);
    }
    setPreferredPos(newPos, HbPopup::Center);

    // change the focus to spell dialog editor
    HbLineEdit *spellEdit = lineEdit();
    if (spellEdit) {
        spellEdit->setMaxLength(MAXUDBWORDSIZE);
        spellEdit->setSmileysEnabled(false);
        HbEditorInterface eInt(spellEdit);
        // we don't want prediction and automatic textcase in spell query dialog
        spellEdit->setInputMethodHints(spellEdit->inputMethodHints() | Qt::ImhNoPredictiveText | Qt::ImhNoAutoUppercase);
        eInt.setLastFocusedState(mSavedState);
        spellEdit->setFocus();
    }
    // execute the spell dialog
    mSavedFocusObject = focusObject->object();
    mSavedEditorText = editorText;
    mDidHandleFinish = false;
    mainWindow()->setProperty("SpellQueryLaunched", true);
    open(this,SLOT(dialogClosed(HbAction*)));
    mPrimaryAction = qobject_cast<HbAction*>(actions().first()); 
}

void HbInputSpellQuery::dialogClosed(HbAction* action)
{
    mainWindow()->setProperty("SpellQueryLaunched", false);
    //There are multiple dialog closed event received. This will make sure we handle finish
    //only once
    if(mDidHandleFinish) {
        return;
    } else {
        mDidHandleFinish = true;
    }

    bool isOk = false;
    bool isCancel = false;
    bool isExternalClose = false;
    // action is null when input query is closed externally , for example by calling
    // HbDialog::close() function.
    if (action) {
        isOk = mPrimaryAction == action ? true : false; 
        isCancel = mPrimaryAction != action ? true : false; 
    } else {
        isExternalClose = true;
    }

    //Need to disable effects as asynchronous hide will commit the word otherwise.
    HbEffect::disable(this);
    hide();
    HbEffect::enable(this);  

    QString string = mSavedEditorText;
    bool status = false;
    if (isOk) {
        string = value().toString();
        status = true;
    } else if (isCancel) {
        status = false;
    } else if (isExternalClose) {
        status = true;
    }
    mPredictionHandler->spellQueryDialogClosed(mSavedFocusObject,status,string);
    mSavedFocusObject = 0;
    mSavedEditorText.clear();
    mPrimaryAction = 0;
}

// End of file
