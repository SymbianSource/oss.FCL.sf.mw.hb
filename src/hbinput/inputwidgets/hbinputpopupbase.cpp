/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbInput module of the UI Extensions for Mobile.
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
#include "hbinputpopupbase.h"
#include "hbinputpopupbase_p.h"

#include <QGraphicsScene>
#include <hbinputmethod.h>

/*!
@stable
@hbinput
\class HbInputPopupBase
\brief Base class for input popups

HbInputPopupBase creates a layer on top of HbDialog and it is
used as a base class for all popups that are used with virtual
keyboards. Inherited popup is set as an inactive panel and
popup priority is set based on the focused editor.

\sa HbDialog
*/
HbInputPopupBase::HbInputPopupBase(QGraphicsItem *parent)
    : HbDialog(*new HbInputPopupBasePrivate(), parent)
{
}

/*!
Constructs input popup
*/
HbInputPopupBase::HbInputPopupBase(HbInputPopupBasePrivate &dd, QGraphicsItem *parent)
    : HbDialog(dd, parent)
{
}

/*!
Destructs the object.
*/
HbInputPopupBase::~HbInputPopupBase()
{
}

/*!
/reimp
*/
void HbInputPopupBase::showEvent(QShowEvent *event)
{
    Q_D(HbInputPopupBase);

    // Make sure the input popup never steals focus.
    setFlag(QGraphicsItem::ItemIsPanel, true);
    d->mActivePopup = false;
    QGraphicsScene *activeScene = scene();
    //QGraphicsItem::setActive() doe not check the following
    if(activeScene && activeScene->activePanel() == this) {
        setActive(false);
    }

    HbInputMethod *inputMethod = HbInputMethod::activeInputMethod();
    if (inputMethod && inputMethod->focusObject()) {
        d->setPriority(inputMethod->focusObject()->editorPriority());
    }

    HbDialog::showEvent(event);
}

// End of file
