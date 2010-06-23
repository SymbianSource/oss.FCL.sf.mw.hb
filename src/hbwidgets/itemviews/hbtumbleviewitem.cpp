/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbWidgets module of the UI Extensions for Mobile.
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
#include "hbtumbleviewitem.h"

#include "hblistviewitem_p.h"
#include <hbstyleoptionlistviewitem_p.h>

#include <QGestureEvent>

HbTumbleViewItem::HbTumbleViewItem(QGraphicsItem *parent) :
    HbListViewItem(parent)
{
    HB_SDD(HbAbstractViewItem);
    sd->mItemType = QString("tumbleviewitem");
    setFocusPolicy(Qt::NoFocus);
}

HbTumbleViewItem::~HbTumbleViewItem()
{
}

/*!
    \reimp
*/
int HbTumbleViewItem::type() const
{
    return Type;
}


/*!
    \reimp
*/
HbAbstractViewItem *HbTumbleViewItem::createItem() 
{
    HbTumbleViewItem* item = new HbTumbleViewItem(*this);
    item->setFocusPolicy(Qt::NoFocus);

    return item;
}

/*!
    \reimp
*/
void HbTumbleViewItem::updateChildItems()
{
    Q_D(HbListViewItem);
    HbListViewItem::updateChildItems();
    if(d->mSelectionItem){
        d->mSelectionItem->hide();
    }
}

void HbTumbleViewItem::gestureEvent(QGestureEvent *event)
{
    if (event->gesture(Qt::TapGesture)) {
        Q_D(HbAbstractViewItem);
        if(itemView()->isScrolling()){
            event->ignore();
            event->ignore(Qt::TapGesture);
            return;
        }
        d->tapTriggered(event);

    } else {
        HbWidget::gestureEvent(event);
    }
}


#include "moc_hbtumbleviewitem.cpp"

