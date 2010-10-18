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
#include "hbtapgesture.h"

#include "hblistviewitem_p.h"
#include "hbnamespace_p.h"
#include <hbstyleoption_p.h>

#include <QGestureEvent>
#include <QCoreApplication>

/*!
    @proto
    \class HbTumbleViewitem
    \brief The HbTumbleViewItem class represents a single row in HbTumbleView.

    HbTumbleViewItem class provides an item that is used by the HbTumbleView class to
    visualize content within single model index. HbTumbleViewItem supports 
    QStringList that is stored into Qt::DisplayRole role within the index. 

    This class acts as a default item prototype inside HbTumbleView.
*/

/*!
    Constructs a TumbleViewItem with \a parent.
*/
HbTumbleViewItem::HbTumbleViewItem(QGraphicsItem *parent) :
    HbListViewItem(parent)
{
    HB_SDD(HbAbstractViewItem);
    sd->mItemType = QString("tumbleviewitem");
    setFocusPolicy(Qt::NoFocus);
}

/*!
    Destructor
*/
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
    connect(item,SIGNAL(released(QPointF)),item->itemView(),SLOT(_q_itemSelected(QPointF)));
    return item;
}

/*!
    \reimp
*/
void HbTumbleViewItem::updateChildItems()
{
    Q_D(HbListViewItem);
    if (d->mIndex.data(Qt::DisplayRole).isNull())
         return;
    HbListViewItem::updateChildItems();
    if(d->mSelectionItem){
        d->mSelectionItem->hide();
    }
}

void HbTumbleViewItem::gestureEvent(QGestureEvent *event)
{
    HbTapGesture *gesture = static_cast<HbTapGesture *>(event->gesture(Qt::TapGesture));
    if (gesture) {
        Q_D(HbAbstractViewItem);
        if(itemView()->isScrolling()){
            event->ignore();
            event->ignore(Qt::TapGesture);
            return;
        }
        if(gesture->state() == Qt::GestureCanceled){
            d->setPressed(false, false);
            event->accept();
            scene()->setProperty(HbPrivate::OverridingGesture.latin1(),QVariant());
            return;
        }
        d->tapTriggered(event);
    } else {
        HbWidget::gestureEvent(event);
   
    }
}

QSizeF HbTumbleViewItem::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    HB_SDD(const HbAbstractViewItem);
    static qreal height=0;

    QSizeF sh=HbListViewItem::sizeHint(which,constraint);
    if(which == Qt::PreferredSize && sh.height()<=0) {
        //TODO:remove this check once refresh issue in DTP is solved on removeRows/insertRows
        if(height<=0) {
            //Let's create a temporary item and take the height for the size hint
            HbAbstractViewItem *tempitem = sd->mPrototype->createItem();
            if(sd->mItemView) {
                QAbstractItemModel *model = sd->mItemView->model();
                if(model) {
                    int rowCount=model->rowCount();
                    QModelIndex ind=model->index(rowCount,0);
                    if(ind.isValid()) {
                        tempitem->setModelIndex(ind);
                    }
                }
            }


            //call polish            
            QEvent polishEvent(QEvent::Polish);
            QCoreApplication::sendEvent(const_cast<HbAbstractViewItem *>(tempitem), &polishEvent);
            height=tempitem->effectiveSizeHint(which,constraint).height();
            delete tempitem;
        }
        return QSizeF(sh.width(),height);
    }
    return sh;
}

#include "moc_hbtumbleviewitem.cpp"

