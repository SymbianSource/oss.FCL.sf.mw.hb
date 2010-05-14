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
#include "hbabstractitemcontainer_p_p.h"
#include "hbabstractitemview.h"
#include "hbtreeviewitem.h"
#include "hbapplication.h"
#include "hbmodeliterator.h"

#include <qmath.h>

#include <QDebug>

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

        delta.setY(delta.y() - item->size().height());

        q->itemRemoved(item);

        mItems.append(item);

        q->itemAdded(mItems.count() - 1, item);

        q->setItemModelIndex(item, nextIndex);
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

        q->itemAdded(0, item);

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

/*
    Calculates avarage item height
*/
qreal HbTreeItemContainerPrivate::itemHeight() const
{
    qreal minHeight = 0.0;
    if (mItems.count() > 0) {
        minHeight = mLayout->effectiveSizeHint(Qt::PreferredSize).height() / mItems.count();
    }

    if (minHeight == 0.0) {
        minHeight = getSmallestItemHeight();
    }

    return minHeight;
}

qreal HbTreeItemContainerPrivate::getSmallestItemHeight() const
{
    Q_Q(const HbTreeItemContainer);

    qreal minHeight = 0.0;
    if (mItems.count() > 0) {
        minHeight = mLayout->smallestItemHeight();
    }

    if (minHeight == 0.0) {
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

int HbTreeItemContainerPrivate::mapToLayoutIndex(int index) const
{
    int layoutIndex = index;

    int itemCount = mAnimatedItems.count();
    for (int i = 0; i < itemCount; ++i) {
        QPair<HbAbstractViewItem *, int> animatedItem = mAnimatedItems.at(i);
        if (animatedItem.second <= index) {
            layoutIndex++;
        }
    }

    return layoutIndex;
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

void HbTreeItemContainer::addItem(const QModelIndex &index, bool animate)
{
    Q_D(HbTreeItemContainer);
   
    // Cancel the collapse & disappear effect if such is ongoing for the given index.
    int itemCount = d->mAnimatedItems.count();
    for (int i = 0; i < itemCount; ++i) {
        QPair<HbAbstractViewItem *, int> animatedItem = d->mAnimatedItems.at(i);
        if (animatedItem.first->modelIndex() == index) {
            HbEffect::cancel(animatedItem.first, "collapse");
            HbEffect::cancel(animatedItem.first, "disappear");
            break;
        }
    }

    HbAbstractItemContainer::addItem(index, animate);
}

/*!
    \reimp
*/
void HbTreeItemContainer::removeItem(int pos, bool animate)
{
    if (animate) {
        Q_D(HbTreeItemContainer);

        QPair<HbAbstractViewItem *, int> pair(d->mItems.at(pos), pos);
        d->mAnimatedItems.append(pair);
    }

    HbAbstractItemContainer::removeItem(pos, animate);
}

/*!
    \reimp
*/
void HbTreeItemContainer::itemRemoved(HbAbstractViewItem *item, bool animate)
{
    Q_D(HbTreeItemContainer);
    d->mLayout->removeItem(item, animate);
}

/*!
    \reimp
*/
void HbTreeItemContainer::itemAdded(int index, HbAbstractViewItem *item, bool animate)
{
    Q_D(HbTreeItemContainer);

    d->mLayout->insertItem(d->mapToLayoutIndex(index), item, d->levelForItem(item), animate);
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

QPointF HbTreeItemContainer::recycleItems(const QPointF &delta)
{
    Q_D(HbTreeItemContainer);

    if (d->mPrototypes.count() != 1) {
        return delta;
    }

    // current invisible area can be scrolled by base class
    // calculation for that need to be done
    const qreal diff = d->getDiffWithoutScrollareaCompensation(delta);

    if (diff != 0.0) {
        HbModelIterator *modelIterator = d->mItemView->modelIterator();
        qreal result = 0.0;
        bool doFarJump = false;
        if (qAbs(diff) > size().height()) {
            // if huge diff - current buffer does not containt any item that should
            // be there after jump - because of that use setModelIndexes instead of
            // recycling items - faster
            // but it is possible that even if far jump was requested (huge delta)
            // it can't be done because of model size and current position (at the end)
            if (diff > 0) {
                // scrolling down
                int indexPos = modelIterator->indexPosition(d->mItems.last()->modelIndex())
                                + d->mItems.count();
                doFarJump = (indexPos < modelIterator->indexCount());
            } else {
                // scrolling up
                int indexPos = modelIterator->indexPosition(d->mItems.first()->modelIndex())
                                - d->mItems.count();
                doFarJump = (indexPos >= 0);
            }
        }
        if (doFarJump) {
            // start calculations for far jump
            // take back into account real delta (do jump as far as possible
            // without leaving it for scroll area) - use delta.y() instead
            // of calculated diff
            qreal itemHeight = d->itemHeight();
            int rowDiff = (int)(delta.y() / itemHeight);
            QPointF deltaAfterJump(delta.x(), delta.y() - (qreal)rowDiff * itemHeight);
            // after setModelIndexes will be used it will still be some delta - deltaAfterJump
            // bottom lines check if those delta can be consumed by scrollArea, if not then
            // corrections to new index need to be done (otherwise it is possible that scrollArea
            // will do the rest of scrolling but leave some empty space)
            qreal diffAfterJump = d->getDiffWithoutScrollareaCompensation(deltaAfterJump);
            if (diffAfterJump != 0.0) {
                // this mean that rest of delta can not be handled by scroll area
                // so jump one item more
                if (rowDiff < 0.0) {
                    rowDiff--;
                } else {
                    rowDiff++;
                }
            }
            int firstIndexPos = modelIterator->indexPosition(d->mItems.first()->modelIndex());
            int jumpIndexPos = firstIndexPos + rowDiff;
            QModelIndex jumpIndex = modelIterator->index(jumpIndexPos);
            if (!jumpIndex.isValid()) {
                // get first or last valid index depending on scroll directions
                if (rowDiff < 0) {// first index
                    jumpIndex = modelIterator->nextIndex(jumpIndex);
                } else {// last index
                    jumpIndex = modelIterator->previousIndex(jumpIndex);
                }
            }
            setModelIndexes(jumpIndex);

            result = -(qreal)rowDiff * itemHeight;
        }
        else {
            QPointF newDelta(0.0, 0.0);
            HbAbstractViewItem *item = 0;
            if (diff < 0.0) {
                while (-newDelta.y() > diff) {
                    item = d->shiftUpItem(newDelta);
                    if (!item) {
                        break;
                    }
                }
            }
            else {
                while (-newDelta.y() < diff) {
                    item = d->shiftDownItem(newDelta);
                    if (!item) {
                        break;
                    }
                }
            }
            qreal layoutPreferredHeight = layout()->preferredHeight();
            if (layoutPreferredHeight < size().height()) {
                // in non uniform item list container can change size
                // while recycling, to catch that layoutPreferredHeight
                // need to be checked - important case is only when new
                // containerHeight is smaller than old containerHeight
                // because only then container can go out of bounds
                qreal viewSize = itemView()->boundingRect().size().height();
                if (layoutPreferredHeight + pos().y() < viewSize) {
                    // position is allways negative
                    // view out of bounds
                    if (diff > 0.0) {
                        QPointF posDiff(pos().x(), 0.0);
                        while (item
                               && layoutPreferredHeight - d->mItems.at(0)->size().height() > viewSize
                               && layoutPreferredHeight + pos().y() - posDiff.y() < viewSize) {
                            // try to shiftDownMoreItems
                            item = d->shiftDownItem(posDiff);
                            layoutPreferredHeight = layout()->preferredHeight();
                        }
                        setPos(pos() - posDiff);
                    }
                }
            }

            result = newDelta.y();
        }
        QPointF newDelta(delta.x(), delta.y() + result);

        return newDelta;
    }

    return delta;
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

            if (!d->mAnimatedItems.isEmpty()) {
                targetCount = currentCount;
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

void HbTreeItemContainer::animationFinished(const HbEffect::EffectStatus &status)
{
    Q_D(HbTreeItemContainer);

    HbAbstractViewItem *item = static_cast<HbAbstractViewItem *>(status.item);
    item->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
   
    // Remove item from mAnimatedItems list.
    int itemCount = d->mAnimatedItems.count();
    for (int i = 0; i < itemCount; ++i) {
        QPair<HbAbstractViewItem *, int> animatedItem = d->mAnimatedItems.at(i);
        if (animatedItem.first == item) {
            d->mAnimatedItems.removeAt(i);
            break;
        }
    }
    
    d->mItems.removeOne(item);
    d->mLayout->removeAt(d->mLayout->indexOf(item));

    item->resetTransform();
    item->setOpacity(1.0);

    QModelIndex index;
    int bufferIndex;
    if (d->mItems.isEmpty()) {
        index = d->mItemView->modelIterator()->nextIndex(QModelIndex());
        bufferIndex = 0;
    } else {
        index = d->mItemView->modelIterator()->nextIndex(d->mItems.last()->modelIndex());
        bufferIndex = d->mItems.count();
        if (!index.isValid()) {
            index = d->mItemView->modelIterator()->previousIndex(d->mItems.first()->modelIndex());
            bufferIndex = 0;
        } 
    }

    if (index.isValid()) {
        d->insertItem(item, bufferIndex, index, false);

        if (bufferIndex == 0) {
            QPointF newPos = pos();
            newPos.setY(newPos.y() - item->preferredHeight());
            setPos(newPos);
        }
    } else {
        item->deleteLater();
    }
}

/*!
    \reimp

    All other sizehints are taken from list layout except preferred sizehint. List container preferred sizeHint 
    width is maximum width and height is average item height times index count.
*/
QSizeF HbTreeItemContainer::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_D(const HbTreeItemContainer);

    if (which == Qt::PreferredSize) {
        HbModelIterator *modelIterator = d->modelIterator();
        if (modelIterator) {
            return QSizeF(QWIDGETSIZE_MAX, d->itemHeight() * modelIterator->indexCount());
        }
    }

    return HbAbstractItemContainer::sizeHint(which, constraint);
}

/*!
    \reimp

    Resizes the container to use layout preferred size.
*/
void HbTreeItemContainer::resizeContainer()
{      
    resize(layout()->preferredSize());
}

#include "moc_hbtreeitemcontainer_p.cpp"

