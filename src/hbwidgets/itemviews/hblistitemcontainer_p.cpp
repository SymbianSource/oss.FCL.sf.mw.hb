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

#include "hblistitemcontainer_p.h"
#include "hblistitemcontainer_p_p.h"
#include "hblistlayout_p.h"

#include "hbabstractitemcontainer.h"
#include "hbabstractitemview.h"
#include "hblistviewitem.h"
#include "hblistview.h"
#include "hbmodeliterator.h"

#include <qmath.h>
#include <QDebug>

const int Hb_Recycle_Buffer_Shrink_Threshold = 2; // Rather arbitrary

HbListItemContainerPrivate::HbListItemContainerPrivate() :
    HbAbstractItemContainerPrivate(),
    mLayout(0)
{ 
}

HbListItemContainerPrivate::~HbListItemContainerPrivate()
{
}

void HbListItemContainerPrivate::init()
{   
    Q_Q(HbListItemContainer);
 
    mLayout = new HbListLayout();
    mLayout->setContentsMargins(0, 0, 0, 0);
    //mLayout->setPreferredWidth(q->size().width());

    q->setLayout(mLayout);
}

/*!
    \private

    Changes first abstract view item from item buffer to be the last one.
*/
HbAbstractViewItem *HbListItemContainerPrivate::shiftDownItem(QPointF& delta)
{
    Q_Q(HbListItemContainer);

    HbAbstractViewItem *item = 0;
    HbAbstractViewItem *lastItem = mItems.last();

    QModelIndex nextIndex = mItemView->modelIterator()->nextIndex(lastItem->modelIndex());
    if (nextIndex.isValid()) {
        item = mItems.takeFirst();

        q->itemRemoved(item);

        delta.setY(delta.y() - item->size().height());

        mItems.append(item);

        q->setItemModelIndex(item, nextIndex);

        q->itemAdded(mItems.count() - 1, item);
    }

    return item;
}

/*!
    \private

    Changes last view item from item buffer to be the first one.
*/
HbAbstractViewItem *HbListItemContainerPrivate::shiftUpItem(QPointF& delta)
{
    Q_Q(HbListItemContainer);

    HbAbstractViewItem *item = 0;
    HbAbstractViewItem *firstItem = mItems.first();

    QModelIndex previousIndex = mItemView->modelIterator()->previousIndex(firstItem->modelIndex());
    if (previousIndex.isValid()) {
        item = mItems.takeLast();

        q->itemRemoved(item);

        mItems.insert(0, item);

        q->setItemModelIndex(item, previousIndex);
        
        qreal itemHeight=0;
        if (q->uniformItemSizes()) {
            itemHeight = mItems.last()->preferredHeight();
        } else {
            //This is time consuming and causes backwards srolling to be slower than forwards.
            //The sizehint of the item is dirty.
            itemHeight = item->preferredHeight();
        }
        
        delta.setY(delta.y() + itemHeight);

        q->itemAdded(0, item);
    }
    return item;
}

bool HbListItemContainerPrivate::intoContainerBuffer(const QModelIndex &index) const
{   
    if (mItems.first()->modelIndex().row() <= index.row()   
        && mItems.last()->modelIndex().row() >= index.row()){
        return true;
    } else {
        return false;
    }
}

int HbListItemContainerPrivate::containerBufferIndexForModelIndex(const QModelIndex &index) const 
{   
    return qMax(0, index.row() - mItems.first()->modelIndex().row());
}

qreal HbListItemContainerPrivate::getSmallestItemHeight() const
{
    Q_Q(const HbListItemContainer);

    qreal minHeight = 0;
    if (mItems.count() > 0) {
        minHeight = mLayout->smallestItemHeight();
    }

    if (minHeight == 0) {
        QModelIndex index;
        while (mItems.isEmpty()) {
            // in practise following conditions must apply: itemview is empty and scrollTo() has been called.
            // Starts populating items from given mFirstItemIndex
            if ( mFirstItemIndex.isValid()) {
                index = mFirstItemIndex;
                const_cast<QPersistentModelIndex&>(mFirstItemIndex) = QModelIndex();
            } else {
                index = mItemView->modelIterator()->nextIndex(index);
            }
            if (!index.isValid()) {
                break;
            }
            const_cast<HbListItemContainer *>(q)->insertItem(0, index);
        }

        if (!mItems.isEmpty()) {
            minHeight = mItems.first()->preferredHeight();
        }
    }
    return minHeight;
}

HbListItemContainer::HbListItemContainer(QGraphicsItem *parent) :
    HbAbstractItemContainer(*new HbListItemContainerPrivate, parent)
{
    Q_D(HbListItemContainer);

    d->q_ptr = this;
    d->init();
}

HbListItemContainer::HbListItemContainer(HbListItemContainerPrivate &dd, QGraphicsItem *parent) :
    HbAbstractItemContainer(dd, parent)
{
    Q_D(HbListItemContainer);

    d->q_ptr = this;
    d->init();
}

/*!
    Destructor.
 */
HbListItemContainer::~HbListItemContainer()
{
}

/*!
    \reimp
*/
void HbListItemContainer::itemRemoved( HbAbstractViewItem *item, bool animate )
{
    Q_D(HbListItemContainer);

    if (static_cast<HbListView *>(d->mItemView)->arrangeMode()) {
        d->mLayout->removeItem(item, false);
    } else {
        d->mLayout->removeItem(item, animate);
    }
}

/*!
    \reimp
*/
void HbListItemContainer::itemAdded(int index, HbAbstractViewItem *item, bool animate)
{
    Q_D(HbListItemContainer);

    if (static_cast<HbListView *>(d->mItemView)->arrangeMode()) {
        d->mLayout->insertItem(index,item, false);
    } else {
        d->mLayout->insertItem(index,item, animate);
    }

}

/*!
    \reimp
*/
void HbListItemContainer::viewResized(const QSizeF &viewSize)
{
//    Q_D(HbListItemContainer);
    //d->mLayout->setPreferredWidth(size.width());
    QSizeF newSize = size();
    newSize.setWidth( viewSize.width() );
    resize( newSize );
}

/*!
    \reimp
*/
QPointF HbListItemContainer::recycleItems(const QPointF &delta)
{
    Q_D(HbListItemContainer);

    if (d->mPrototypes.count() != 1) {
        return delta;
    }

    QRectF viewRect(d->itemBoundingRect(d->mItemView));
    viewRect.moveTopLeft(viewRect.topLeft() + delta);

    int firstVisibleBufferIndex = -1;
    int lastVisibleBufferIndex = -1;
    d->firstAndLastVisibleBufferIndex(firstVisibleBufferIndex, lastVisibleBufferIndex, viewRect, false);

    int hiddenAbove = firstVisibleBufferIndex;
    int hiddenBelow = d->mItems.count() - lastVisibleBufferIndex - 1;

    if (d->mItems.count()
        && (firstVisibleBufferIndex == -1 || lastVisibleBufferIndex == -1)) {
        // All items out of sight.
        if (d->itemBoundingRect(d->mItems.first()).top() < 0) {
            // All items above view.
            hiddenAbove = d->mItems.count();
            hiddenBelow = 0;
        } else {
            // All items below view.
            hiddenAbove = 0;
            hiddenBelow = d->mItems.count();
        }
    }

    QPointF newDelta(delta);

    while (hiddenAbove > hiddenBelow + 1) {
        HbAbstractViewItem *item = d->shiftDownItem(newDelta);
        if (!item){
            break;
        }

        if (!d->visible(item, viewRect)) {
            hiddenBelow++;
        }
        hiddenAbove--;
    }

    while (hiddenBelow > hiddenAbove + 1) {
        HbAbstractViewItem *item = d->shiftUpItem(newDelta);
        if (!item) {
            break;
        }

        if (!d->visible( item, viewRect)) {
            hiddenAbove++;
        }
        hiddenBelow--;
    }

    return newDelta;
}


/*!
    Calculates the optimal view item buffer size. When recycling is turned off
    returned buffer same is same as the amount of rows within the model. When 
    recycling is enabled the amount is calculated by dividing the view area with
    average item height. Average item height is taken from the average of existing
    view item's heights or by creating item using prototype for the first row.
*/
int HbListItemContainer::maxItemCount() const
{
    Q_D(const HbListItemContainer);

    int targetCount = HbAbstractItemContainer::maxItemCount();
    if (targetCount > 0
        && d->mItemRecycling
        && d->mPrototypes.count() <= 1) {
    
        qreal minHeight = d->getSmallestItemHeight();

        if (minHeight > 0) {
            int countEstimate = qCeil(d->mItemView->boundingRect().height() / minHeight)
                + d->mBufferSize;

            int currentCount = d->mItems.count();
            if (countEstimate < currentCount) {
                // Shrink the recycle buffer only if change is more than
                // buffer shrink threshold. 
                if (countEstimate > (currentCount - Hb_Recycle_Buffer_Shrink_Threshold)) {
                    countEstimate = currentCount;
                }
            }

            // This limits the targetCount not to be larger
            // than row count inside model.
            targetCount = qMin(targetCount, countEstimate);
        } else {
            targetCount = 0;
        }
    } 
    return targetCount;
}

HbAbstractViewItem *HbListItemContainer::createDefaultPrototype() const
{
    return new HbListViewItem();
}

void HbListItemContainer::animationFinished(const HbEffect::EffectStatus &status)
{
    Q_D(HbListItemContainer);

    HbAbstractViewItem *item = static_cast<HbAbstractViewItem *>(status.item);
    d->mLayout->removeAt(d->mLayout->indexOf(item));
    item->deleteLater();
}

#include "moc_hblistitemcontainer_p.cpp"


