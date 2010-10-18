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
#include <hbinputbutton.h>

#include "hbinputnumerichandler_p.h"
#include "hbinputabstractbase.h"

HbInputNumericHandlerPrivate::HbInputNumericHandlerPrivate()
{
}

HbInputNumericHandlerPrivate::~HbInputNumericHandlerPrivate()
{
}

HbInputNumericHandler::HbInputNumericHandler(HbInputNumericHandlerPrivate &dd, HbInputAbstractMethod* inputMethod)
:HbInputModeHandler(dd, inputMethod)
{
    Q_D(HbInputNumericHandler);
    d->q_ptr = this;
}


HbInputNumericHandler::~HbInputNumericHandler()
{
}

/*!
    This function handles different keyevents.
*/
bool HbInputNumericHandler::filterEvent(const QKeyEvent *event)
{
    bool ret = true;
    switch (event->key()) {
    case Qt::Key_Backspace:
    case HbInputButton::ButtonKeyCodeDelete: {
        QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
        sendAndUpdate(keyEvent);
        // return false since the event is sent forward
        ret = false;
        break;
    }
    case HbInputButton::ButtonKeyCodeEnter:
    case HbInputButton::ButtonKeyCodeSpace:
    case Qt::Key_Period:
    case Qt::Key_Comma: {
        QChar qc(event->key());
        if (qc == Qt::Key_Enter || qc == Qt::Key_Return) {
            qc = QChar('\n');  // Editor expects normal line feed.
        }
        commitAndUpdate(qc);
        break;
    }
    case HbInputButton::ButtonKeyCodeSettings:
        break;
    default:
        ret = HbInputModeHandler::filterEvent(event);
        break;
    }
    return ret;
}

/*!
 Action handler
*/
bool HbInputNumericHandler::actionHandler(HbInputModeAction action)
{
    return HbInputModeHandler::actionHandler(action);
}

//EOF
