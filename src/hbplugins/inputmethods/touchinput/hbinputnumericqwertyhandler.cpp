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
#include <hbinputmethod.h>
#include <hbinputkeymapfactory.h>

#include "hbinputnumericqwertyhandler.h"
#include "hbinputnumerichandler_p.h"
#include "hbinputabstractbase.h"

class HbInputNumericQwertyHandlerPrivate: public HbInputNumericHandlerPrivate
{
    Q_DECLARE_PUBLIC(HbInputNumericQwertyHandler)
public:
    HbInputNumericQwertyHandlerPrivate();
    ~HbInputNumericQwertyHandlerPrivate();

    void init();

    // button related operations.
    bool buttonPressed(const QKeyEvent *event);
    bool buttonReleased(const QKeyEvent *event);
};

HbInputNumericQwertyHandlerPrivate::HbInputNumericQwertyHandlerPrivate()
{
}

HbInputNumericQwertyHandlerPrivate::~HbInputNumericQwertyHandlerPrivate()
{
}

bool HbInputNumericQwertyHandlerPrivate::buttonPressed(const QKeyEvent * event)
{
    Q_UNUSED(event);
    return false;
}

bool HbInputNumericQwertyHandlerPrivate::buttonReleased(const QKeyEvent *event)
{
    Q_Q(HbInputNumericQwertyHandler);

    // let's pass event to the base class.
    if (q->HbInputNumericHandler::filterEvent(event)) {
        return true;
    }

    int buttonId = event->key();
    // currently we shift and control key are not clear as concept. So just ignoring
    // these buttons.
    if (buttonId == Qt::Key_Shift || buttonId == Qt::Key_Control) {
        return false;
    }

    // Don't handle this
    if (buttonId == Qt::Key_Backspace || buttonId == Qt::Key_Delete) {
        return false;
    }

    //TODO
    HbInputFocusObject *focusObject = 0;
    focusObject = mInputMethod->focusObject();
    if (!focusObject) {
        qDebug("HbInputNumericQwertyHandler::buttonReleased : no focused editor widget!");
        return false;
    }
    int currentTextCase = focusObject->editorInterface().textCase();
    if (buttonId >= 0) {
        QChar newChar = QChar(buttonId);
        if (currentTextCase == HbTextCaseLower) {
            newChar = newChar.toLower();
        } else {
            newChar = newChar.toUpper();
        }
        q->commitAndUpdate(newChar);
        return true;
    }
    return false;
}

HbInputNumericQwertyHandler::HbInputNumericQwertyHandler(HbInputAbstractMethod* inputMethod)
:HbInputNumericHandler(* new HbInputNumericQwertyHandlerPrivate, inputMethod)
{
    Q_D(HbInputNumericQwertyHandler);
    d->q_ptr = this;
}


/*!
 This function lists different input modes.
*/
void HbInputNumericQwertyHandler::listInputModes(QVector<HbInputModeProperties>& modes) const
{
    HbInputModeProperties binding;
    binding.iMode = HbInputModeNumeric;
    binding.iKeyboard = HbKeyboardVirtualQwerty;

    QList<HbInputLanguage> languages = HbKeymapFactory::availableLanguages();
    foreach (HbInputLanguage language, languages) {
        binding.iLanguage = language;
        modes.push_back(binding);
    }
}

HbInputNumericQwertyHandler::~HbInputNumericQwertyHandler()
{
}

/*!
 filterEvent function key handling.
*/
bool HbInputNumericQwertyHandler::filterEvent(const QKeyEvent* event)
{
    Q_D(HbInputNumericQwertyHandler);

    if (event->type() == QEvent::KeyRelease) {
        return d->buttonReleased(event);
    } else {
        return d->buttonPressed(event);
    }
}

/*!
 Action handler
*/
bool HbInputNumericQwertyHandler::actionHandler(HbInputModeAction action)
{
    return HbInputNumericHandler::actionHandler(action);
}

//EOF

