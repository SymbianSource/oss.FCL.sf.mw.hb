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

#include <qglobal.h>
#include <hbinputvkbwidget.h>
#include <hbinputcandidatelist.h>
#include <hbmainwindow.h>
#include <hbaction.h>
#include <hbinputsettingproxy.h>

#include "hbinputabstractbase.h"

const qreal HbDeltaHeight = 2.0;

// ---------------------------------------------------------------------------
// HbInputAbstractMethod::HbInputAbstractMethod
//
// ---------------------------------------------------------------------------
HbInputAbstractMethod::HbInputAbstractMethod()
{
    mVanillQwertySwitch = new HbAction(QString("QTY"));
}

// ---------------------------------------------------------------------------
// HbInputAbstractMethod::~HbInputAbstractMethod
//
// ---------------------------------------------------------------------------
HbInputAbstractMethod::~HbInputAbstractMethod()
{
    delete mVanillQwertySwitch;
}

// ---------------------------------------------------------------------------
// HbInputAbstractMethod::launchCandidatePopup
//
// ---------------------------------------------------------------------------
void HbInputAbstractMethod::launchCandidatePopup(const QStringList &candidates)
{
    Q_UNUSED(candidates);
}

// ---------------------------------------------------------------------------
// HbInputAbstractMethod::closeKeypad
//
// ---------------------------------------------------------------------------
void HbInputAbstractMethod::closeKeypad()
{
}

// ---------------------------------------------------------------------------
// HbInputAbstractMethod::switchSpecialCharacterTable
//
// ---------------------------------------------------------------------------
void HbInputAbstractMethod::switchSpecialCharacterTable()
{
}

// ---------------------------------------------------------------------------
// HbInputAbstractMethod::selectSpecialCharacterTableMode
//
// ---------------------------------------------------------------------------
void HbInputAbstractMethod::selectSpecialCharacterTableMode()
{
}

// ---------------------------------------------------------------------------
// HbInputAbstractMethod::closeExactWordPopup
//
// ---------------------------------------------------------------------------
void HbInputAbstractMethod::closeExactWordPopup()
{
}

// ---------------------------------------------------------------------------
// HbInputAbstractMethod::launchExactWordPopup
//
// ---------------------------------------------------------------------------
void HbInputAbstractMethod::launchExactWordPopup(QString exactWord)
{
    Q_UNUSED(exactWord);
}

// ---------------------------------------------------------------------------
// HbInputAbstractMethod::launchAutoCompletionPopup
//
// ---------------------------------------------------------------------------
void HbInputAbstractMethod::launchAutoCompletionPopup(const QStringList& candidates)
{
    Q_UNUSED(candidates);
}

// ---------------------------------------------------------------------------
// HbInputAbstractMethod::closeAutoCompletionPopup
//
// ---------------------------------------------------------------------------
void HbInputAbstractMethod::closeAutoCompletionPopup()
{
}

// ---------------------------------------------------------------------------
// HbInputAbstractMethod::switchMode
//
// ---------------------------------------------------------------------------
void HbInputAbstractMethod::switchMode(int keyCode)
{
    Q_UNUSED(keyCode);
} 

// ---------------------------------------------------------------------------
// HbInputAbstractMethod::starKeySelected
//
// ---------------------------------------------------------------------------
void HbInputAbstractMethod::starKeySelected()
{
}

void HbInputAbstractMethod::getCandidatePositionAndSize(HbCandidateList *candidatePopup,
                    HbInputVkbWidget *currentKeypad, QPointF & pos,QSizeF & size)
{
    QRectF cursorRect = focusObject()->microFocus(); // from the top of the screen

    QSize vpSize = currentKeypad->mainWindow()->size();
    QRectF viewport = QRectF(currentKeypad->mainWindow()->mapToScene(QPoint(0, 0)),
                      currentKeypad->mainWindow()->mapToScene(QPoint(vpSize.width(), vpSize.height())));

    pos = QPointF(cursorRect.bottomLeft().x(), cursorRect.bottomLeft().y() + HbDeltaHeight);
    size = candidatePopup->size();
    size.setHeight(abs((int)viewport.height()) - pos.y());

    QRectF geom = candidatePopup->geometry();
    if (size.height() > geom.height()) {
        size.setHeight(geom.height());
    }

    if ((pos.x() + size.width()) > abs((int)viewport.width())) {
        // can not fit candidate popup to the right side of inline edit text.
        pos.setX(abs((int)viewport.width()) - size.width());
    }
}


bool HbInputAbstractMethod::isSctModeActive() const
{
	return false;
}


HbKeyboardType HbInputAbstractMethod::currentKeyboardType() const
{
    return HbKeyboardNone;
}

void HbInputAbstractMethod::focusReceived()
{
    bool isVannilaApp = false;
    QInputContext* context = qApp->inputContext();
    if (context && context->focusWidget()) {
        QWidget *focusedWidget = context->focusWidget();
        if (!focusedWidget->inherits("HbMainWindow")) {
            isVannilaApp = true;
        }
    }
    
    if(isVannilaApp && focusObject() ) {
        QList<HbAction*> customActions= focusObject()->editorInterface().actions();
        if(!customActions.contains(mVanillQwertySwitch)) {
            disconnect(mVanillQwertySwitch, SIGNAL(triggered(bool)));
            connect(mVanillQwertySwitch, SIGNAL(triggered(bool)), this, SLOT(switchKeypad(bool)));
            focusObject()->editorInterface().addAction(mVanillQwertySwitch);
        }
    }
}

void HbInputAbstractMethod::switchKeypad(bool isActive)
{
    Q_UNUSED(isActive);
    HbKeyboardType keyboard = HbInputSettingProxy::instance()->activeKeyboard();
    if (keyboard == HbKeyboardVirtual12Key) {
        HbInputSettingProxy::instance()->setActiveKeyboard(HbKeyboardVirtualQwerty);
    } else if (keyboard == HbKeyboardVirtualQwerty) {
        HbInputSettingProxy::instance()->setActiveKeyboard(HbKeyboardVirtual12Key);
    }
}

// EOF



   
QChar HbInputAbstractMethod ::previousChar()
{
	return QChar();
}


// EOF

