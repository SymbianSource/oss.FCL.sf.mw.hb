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

#include "hbtreeitemcontainer_p.h"
#include "hbtreeitemcontainer_p_p.h"
#include "hbtreelayout_p.h"
#include "hbabstractitemcontainer_p.h"
#include "hbabstractitemview.h"
#include "hbtreeviewitem.h"
#include "hbapplication.h"
#include "hbmodeliterator.h"

#include <qmath.h>

const int Hb_Recycle_Buffer_Shrink_Threshold = 2;

HbTreeItemContainerPrivate::HbTreeItemContainerPrivate() :
    HbAbstractItemContainerPrivate(),
    mLayout(0),
    mUserIndentation(-1.0),
    mStyleIndentation(15.0)
{
}

HbTreeItemContainerPrivate::~HbTreeItemContainerPrivate()
{
}

void HbTreeItemContainerPrivate::init()
{   
    Q_Q(HbTreeItemContainer);

    mLayout = new HbTreeLayout();
    mLayout->setContentsMargins(0, 0, 0, 0);
    
    q->setLayout(mLayout);
}

int HbTreeItemContainerPrivate::levelForItem(HbAbstractViewItem *item) const
{
    int level = 0;
    if (mItemView) {
        QModelIndex parentIndex = item->modelIndex().parent();
        QModelIndex rootIndex = mItemView->rootIndex();

        while (parentIndex != rootIndex && parentIndex.isValid()) {
            level++;
            parentIndex = parentIndex.parent();
        }
    }
    return level;
}

/*!
    \private

    Changes first abstract view item from item buffer to be the last one.
*/
HbAbstractViewItem *HbTreeItemContainerPrivate::shiftDownItem(QPointF& delta)
{
    Q_Q(HbTreeItemContainer);

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
HbAbstractViewItem *HbTreeItemContainerPrivate::shiftUpItem(QPointF& delta)
{
    Q_Q(HbTreeItemContainer);

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

void HbTreeItemContainerPrivate::resolveIndentation()
{
    Q_Q(const HbTreeItemContainer);
    qreal indentation = mUserIndentation;
    if (indentation < 0.0) {
        q->style()->parameter("hb-param-margin-gene-left", indentation);
        if (indentation >= 0.0) {
            mStyleIndentation = indentation;
        } else {
            indentation = mStyleIndentation;
        }
    }
    mLayout->setIndentation(indentation);
}

qreal HbTreeItemContainerPrivate::getSmallestItemHeight() const
{
    Q_Q(const HbTreeItemContainer);

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
            const_cast<HbTreeItemContainer *>(q)->insertItem(0, index);
        }

        if (!mItems.isEmpty()) {
            minHeight = mItems.first()->preferredHeight();
        }
    }
    return minHeight;
}


HbTreeItemContainer::HbTreeItemContainer(QGraphicsItem *parent) :
    HbAbstractItemContainer(*new HbTreeItemContainerPrivate, parent)
{
    Q_D(HbTreeItemContainer);

    d->q_ptr = this;
    d->init();
}

HbTreeItemContainer::HbTreeItemContainer( HbTreeItemContainerPrivate &dd, QGraphicsItem *parent) :
    HbAbstractItemContainer(dd, parent)
{
    Q_D(HbTreeItemContainer);

    d->q_ptr = this;
    d->init();
}

/*!
    Destructor.
 */
HbTreeItemContainer::~HbTreeItemContainer()
{
}

/*!
    \reimp
*/
void HbTreeItemContainer::itemRemoved(HbAbstractViewItem *item, bool animate)
{
    Q_UNUSED(animate);
    Q_D(HbTreeItemContainer);

    d->mLayout->removeItem(item);
}

/*!
    \reimp
*/
void HbTreeItemContainer::itemAdded(int index, HbAbstractViewItem *item, bool animate)
{
    Q_UNUSED(animate);
    Q_D(HbTreeItemContainer);

    d->mLayout->insertItem(index, item, d->levelForItem(item));
}

/*!
    \reimp
*/
void HbTreeItemContainer::viewResized(const QSizeF &size)
{
    Q_D(HbTreeItemContainer);
    d->mLayout->setMinimumWidth(size.width());
}

void HbTreeItemContainer::setItemModelIndex(HbAbstractViewItem *item, const QModelIndex &index)
{
    if (item && item->modelIndex() != index) {
        HbAbstractItemContainer::setItemModelIndex(item, index);

        Q_D(HbTreeItemContainer);
        d->mLayout->setItemLevel(item, d->levelForItem(item));
    }
}


qreal HbTreeItemContainer::indentation() const
{
    Q_D(const HbTreeItemContainer);
    return d->mUserIndentation;
}

void HbTreeItemContainer::setIndentation(qreal indentation)
{
    Q_D(HbTreeItemContainer);
    if (d->mUserIndentation != indentation) {
        d->mUserIndentation = indentation;
        d->resolveIndentation();
    }
}

/*!
    \reimp
*/
QPointF HbTreeItemContainer::recycleItems(const QPointF &delta)
{
    Q_D(HbTreeItemContainer);

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

    // during scrolling the x-coordinate of the container is moved to match the 
    // indentation level of the visible items. 
    if (!itemView()->isDragging() ) {
        int minIndent = d->levelForItem(d->mItems[hiddenAbove]);
        for (int i = hiddenAbove + 1; i < d->mItems.count() - hiddenBelow - 1; i++) {
            minIndent = qMin(minIndent,d->levelForItem(d->mItems[i]));
        }
        // if the indentation level is bigger than the current position, container is moved to right.
        // pixel amount of one indentation is added to the current position, in deep indentation 
        // levels this will make the container to show some empty on the left side to indicate that 
        // the items are not on the root level. This is just a visual trick
        if (HbApplication::layoutDirection() == Qt::LeftToRight) {
            if ( minIndent * d->mLayout->indentation() > -pos().x() + d->mLayout->indentation() + 1) {
                newDelta.setX( newDelta.x() + 1 );
            } else if ( minIndent * d->mLayout->indentation() < -pos().x() + d->mLayout->indentation()){
                newDelta.setX( newDelta.x() - 1 );
            }
        } 
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
int HbTreeItemContainer::maxItemCount() const
{
    Q_D(const HbTreeItemContainer);

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

bool  HbTreeItemContainer::event(QEvent *e)
{
    Q_D(HbTreeItemContainer);
    if (e->type() == QEvent::LayoutRequest) {
        // User indentation overrides style indentation.
        // Resolving is here not to do it too often or too seldom
        d->resolveIndentation();
    }
    return HbAbstractItemContainer::event(e);
}

HbAbstractViewItem *HbTreeItemContainer::createDefaultPrototype() const
{
    return new HbTreeViewItem();
}

bool HbTreeItemContainer::isExpanded(const QModelIndex &index) const
{
    QVariant flags = itemState(index).value(HbTreeViewItem::ExpansionKey);
    if (flags.isValid() && flags.toBool() == true) {
        return true;
    } else {
        return false;
    }
}

void HbTreeItemContainer::setExpanded(const QModelIndex &index, bool expanded) 
{
    Q_D(HbTreeItemContainer);

    HbTreeViewItem *item = qobject_cast<HbTreeViewItem *>(itemByIndex(index));
    if (item) {
        item->setExpanded(expanded);
    } 

    setItemStateValue(index, HbTreeViewItem::ExpansionKey, expanded);
    setModelIndexes();
    d->updateItemBuffer(); // Expanding/Collapsing might resize the buffer.
}

#include "moc_hbtreeitemcontainer_p.cpp"

