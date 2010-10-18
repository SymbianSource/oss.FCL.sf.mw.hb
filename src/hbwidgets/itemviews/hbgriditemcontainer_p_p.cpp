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

#include <hbgriditemcontainer_p.h>
#include <hbgridviewitem.h>
#include <hbgridlayout_p.h>
#include <hbabstractitemview.h>
#include <hbgriditemcontainer_p_p.h>
#include <hbapplication.h>
#include <hbmodeliterator.h>

HbGridItemContainerPrivate::HbGridItemContainerPrivate()
                             : mLayout(0),
                             mMinCount(0),
                             mRowCount(4),
                             mColumnCount(3),
                             mItemsPerRow(3),
                             mScrollDirection(Qt::Vertical),
                             mOldItemsPerRow(0)
{
}

HbGridItemContainerPrivate::~HbGridItemContainerPrivate()
{
    //See the base class destructor comments
}

void HbGridItemContainerPrivate::init()
{   
    Q_Q(HbGridItemContainer);
    mLayout = new HbGridLayout();
    q->connect(mLayout, SIGNAL(animationFinished(QGraphicsLayoutItem *, HbGridLayout::AnimationType)), 
        q, SLOT(layoutAnimationFinished(QGraphicsLayoutItem *, HbGridLayout::AnimationType)));

    mItemRecycling = false;
    mUniformItemSizes = true;
    q->setLayout(mLayout);
    mLayout->setRowCount(mRowCount);
    mLayout->setColumnCount(mColumnCount);
}

qreal HbGridItemContainerPrivate::getDiffWithoutScrollareaCompensation(const QPointF &delta) const
{
    // subtract invisible space from delta - part of scrolling that can
    // be done by scroll area
    Q_Q(const HbGridItemContainer);
    const QSizeF containerSize(q->size());
    const QPointF containerPos(q->pos());
    qreal diff = 0.0;
    qreal invisibleArea = 0.0;
    QSizeF viewSize = mItemView->size();
    if (Qt::Vertical == mScrollDirection) {
        if (delta.y() > 0) {
            // space at the bottom
            invisibleArea = containerSize.height() - viewSize.height() + containerPos.y();
            if (invisibleArea < delta.y()) {
                diff = delta.y() - invisibleArea;
            }
        } else {
            // space at the top
            invisibleArea = -containerPos.y();
            if (containerPos.y() > delta.y()) {
                diff = delta.y() + invisibleArea;
            }
        }
    }
    else {
        if (delta.x() > 0) {
            // space at the right
            invisibleArea = containerSize.width() - viewSize.width() + containerPos.x();
            if (invisibleArea < delta.x()) {
                diff = delta.x() - invisibleArea;
            }
        } else {
            // space at the left
            invisibleArea = -containerPos.x();
            if (containerPos.x() > delta.x()) {
                diff = delta.x() + invisibleArea;
            }
        }
    }

    return diff;
}

qreal HbGridItemContainerPrivate::recycling(qreal diff)
{
    qreal result(0.0);
    bool resetLayout(false);
    qreal itemSize = getScrollDirectionItemSize();
    if (diff < 0.0) {
        while (result > diff) {
            HbAbstractViewItem *item = shiftUp(false);
            if (!item) {
                break;
            }
            result -= itemSize;
            resetLayout = true;
        }
    }
    else {
        while (result < diff) {
            HbAbstractViewItem *item = shiftDown(false);
            if (!item) {
                break;
            }
            result += itemSize;
            resetLayout = true;
        }
    }
    if (resetLayout) {
        mLayout->invalidate();
    }

    return result;
}

qreal HbGridItemContainerPrivate::farRecycling(const QPointF &delta)
{
    const qreal jumpDiff = (Qt::Vertical == mScrollDirection)
                           ? delta.y() : delta.x();
    qreal itemSize = getScrollDirectionItemSize();
    int rowDiff = (int)(jumpDiff / itemSize);
    QPointF deltaAfterJump = (Qt::Vertical == mScrollDirection)
                             ? QPointF(delta.x(), delta.y() - (qreal)rowDiff * itemSize)
                                 : QPointF(delta.x() - (qreal)rowDiff * itemSize, delta.y());
    // after setModelIndexes will be used it will still be some delta - deltaAfterJump
    // bottom lines check if those delta can be consumed by scrollArea, if not then
    // corrections to new index need to be done (otherwise it is possible that scrollArea
    // will do the rest of scrolling but leave some empty space)
    qreal diffAfterJump = getDiffWithoutScrollareaCompensation(deltaAfterJump);
    if (diffAfterJump != 0.0) {
        // this mean that rest of delta can not be handled by scroll area
        // so jump one row more
        if (rowDiff < 0.0) {
            rowDiff--;
        } else {
            rowDiff++;
        }
    }
    QModelIndex currentIndex = mItems.first()->modelIndex();
    HbModelIterator *modelIterator = mItemView->modelIterator();
    int jumpIndexPos = modelIterator->indexPosition(currentIndex)
                       + rowDiff * mItemsPerRow;
    QModelIndex jumpIndex = modelIterator->index(jumpIndexPos);
    if (!jumpIndex.isValid()) {
        // get first or last valid index depending on scroll directions
        if (rowDiff < 0) { // first index
            jumpIndex = modelIterator->nextIndex(jumpIndex);
        } else { // last index
            jumpIndex = modelIterator->previousIndex(jumpIndex);
        }
    }
    scrollToPositionAtTop(jumpIndex);

    return (qreal)rowDiff * itemSize;
}

/*!
    First item is moved to the end of the buffer and it gets 
    next item index after last item in buffer.
    If doEventBadIndex is true then this operation is performed 
    even if buffer already contain last item from model - in this 
    case item is moved to mHiddenItems buffer without index change.
*/
HbAbstractViewItem *HbGridItemContainerPrivate::shiftDownItem(bool doEvenBadIndex, bool animate)
{
    Q_Q(HbGridItemContainer);

    HbAbstractViewItem *item = 0;
    QModelIndex nextIndex;
    HbAbstractViewItem *lastItem = mItems.last();
    if (lastItem->modelIndex().isValid()) {
        nextIndex = mItemView->modelIterator()->nextIndex(lastItem->modelIndex());
    }
    if (nextIndex.isValid()) {
        item = mItems.takeFirst();
        q->setItemModelIndex(item, nextIndex);
        mLayout->moveItem(item, -1, animate);
        mItems.append(item);
    }
    else if (doEvenBadIndex) {
        item = mItems.takeFirst();
        q->setItemModelIndex(item, nextIndex);
        mLayout->moveItem(item, -1, animate);
        mItems.append(item);
        item = 0;
    }
    return item;
}

/*!
    Overloaded function used in case when only some part of buffer 
    need to be shifted - in case when one item was removed, and if
    it was in the buffer then only items below this one need to be
    shifted.
    Return true if shitf was possible, false if not, or if after 
    shifting extra shift up was done.
*/
bool HbGridItemContainerPrivate::shiftDownItem(int pos, bool animate)
{
    Q_Q(HbGridItemContainer);

    if (pos >= 0
        && pos < mItems.count()) {
        HbAbstractViewItem *lastItem = mItems.last();
        HbAbstractViewItem *item = mItems.at(pos);
        QModelIndex nextIndex;
        if (lastItem->modelIndex().isValid())
            nextIndex = mItemView->modelIterator()->nextIndex(lastItem->modelIndex());
        q->setItemModelIndex(item, nextIndex);
        mLayout->moveItem(item, -1, animate);
        mItems.move(pos, mItems.count()-1);

        if (mItemView->model()->rowCount() % mItemsPerRow == 0
            && !nextIndex.isValid()) {
            // there is mItemsPerRow items at the end and they are empty
            if (!shiftUp(animate)) { // not possible to shift up
                // all items from end that are not visible should be removed
                while (mItems.count() > 0) {
                    if (mItems.last()->isVisible()) break;
                    deleteItem(mItems.last());
                }
            }
            return false;
        }
        return true;
    }
    return false;
}

/*!
    Shift down by one row.
    First row is moved to the end and item indexes are 
    corrected to contain right items.
    Pointer to first item in row is returned
*/
HbAbstractViewItem *HbGridItemContainerPrivate::shiftDown(bool animate)
{
    HbAbstractViewItem *item = shiftDownItem(false, animate);
    if (item) {
        for (int currentColumn = 1; currentColumn < mItemsPerRow; currentColumn++) {
            shiftDownItem(true, animate);
        }
    }
    return item;
}

/*!
    Shift buffer content by one item up.
    This mean that last item from buffer is taken and its
    model index is changed to "before first".
*/
HbAbstractViewItem *HbGridItemContainerPrivate::shiftUpItem(bool animate)
{
    Q_Q(HbGridItemContainer);

    HbAbstractViewItem *firstItem = mItems.first();
    HbAbstractViewItem *item = 0;
    QModelIndex previousIndex = mItemView->modelIterator()->previousIndex(firstItem->modelIndex());
    if (previousIndex.isValid()) {
        item = mItems.takeLast();
        q->setItemModelIndex(item, previousIndex);
        mLayout->moveItem(item, 0, animate);
        mItems.insert(0, item);
    }
    return item;
}

/*!
    Shift up by one row.
    Last row is moved to beginning and item indexes are
    changed to be in right order.
    Pointer to first item in row is returned
*/
HbAbstractViewItem *HbGridItemContainerPrivate::shiftUp(bool animate)
{
    HbAbstractViewItem *item = shiftUpItem(animate);
    if (item) {
        for (int currentColumn = 1; currentColumn < mItemsPerRow; currentColumn++) {
            item = shiftUpItem(animate);
        }
    }
    return item;
}

/*!
    Updates the buffer we maintain. It also informs layout to recalculate item size as something has changed
    (rows, columns or viewSize). Calling this results in updating geometry and size hint of layout.
*/
void HbGridItemContainerPrivate::resetBuffer()
{
    if(!mItemView || !mItemView->model()) {
        return;
    }
    mMinCount = 0;
    Q_ASSERT(mLayout);
    QRectF viewRect = itemBoundingRect(mItemView);
    mLayout->setSize(viewRect.size(), mMinCount);
    mLayout->invalidate();
    QSizeF itemSize = mLayout->effectiveSizeHint(Qt::MinimumSize);
    mItemsPerRow = (Qt::Vertical == mScrollDirection)
        ? mLayout->columnCount() : mLayout->rowCount();
    mMinCount += mBufferSize * mItemsPerRow;
    updateItemBuffer();
}

/*!
    Removes item from buffer shifting other items if necessary
*/
void HbGridItemContainerPrivate::removeItem(const QModelIndex &index, bool animate)
{
    Q_Q(HbGridItemContainer);
    HbAbstractViewItem *viewItem = item(index);

    bool scrollingNeeded = false;
    bool layoutUpdateNeeded = false;

    // some tricky solution because if buffer has the last items from model
    // it is possible that it contain items with bad indexes, then it is not possible
    // to determine if this item was just removed or it is some empty item (in case)
    // when model_count % items_per_row != 0
    if (viewItem) {
        if (!viewItem->isVisible()) {
            viewItem = 0;
        }
    }
    if (viewItem) {
        if (!animate) {
            if (!shiftDownItem(mItems.indexOf(viewItem), animate)) {
                scrollingNeeded = true;
            }
            layoutUpdateNeeded = true;
        } else {
            QPair<HbAbstractViewItem *, int> pair(viewItem, mItems.indexOf(viewItem));
            mAnimatedItems.append(pair);
            mItems.removeAt(pair.second);
            updateItemBuffer();
        }
    }
    else {
        // in case when item before visible region was removed
        if (mItems.count() > 0) {
            if (mItems.first()->modelIndex().row() % mItemsPerRow) {
                if (!shiftDownItem(0, animate)) {
                    scrollingNeeded = true;
                }
                layoutUpdateNeeded = true;
            }
        }
    }

    if (scrollingNeeded) {
        // when shiftDownItem return false then shift up was done
        // need to scroll visible area to be consist in grid view behaviour
        // checking boundaries after scroll is also done
        QRectF viewRect(itemBoundingRect(mItemView));
        QSizeF itemsCanvas(q->layout()->preferredSize());
        QPointF pos = q->pos();
        qreal itemSize = getScrollDirectionItemSize();
        if (Qt::Vertical == mScrollDirection) {
            pos.setY(pos.y() - itemSize);
            if (pos.y() < viewRect.height() - itemsCanvas.height()) {
                pos.setY(viewRect.height() - itemsCanvas.height()); 
            }
            if (pos.y() > 0) {
                pos.setY(0);
            }
        } else {
            pos.setX(pos.x() - itemSize);
            if (pos.x() < viewRect.width() - itemsCanvas.width()) {
                pos.setX(viewRect.width() - itemsCanvas.width()); 
            }
            if (pos.x() > 0) {
                pos.setX(0);
            }
        }
        q->setPos(pos);
    }
    if (layoutUpdateNeeded) {
        mLayout->invalidate();
    }
    emit q->itemAboutToBeDeleted(viewItem);
}


void HbGridItemContainerPrivate::scrollToEnsureVisible(const QModelIndex &index)
{
    Q_Q(HbGridItemContainer);

    if (index.row() < mItems[0]->modelIndex().row()) {
        q->setModelIndexes(index);
    }
    else {
        QModelIndex lastIndex = lastValidItemIndex();
        if (lastIndex.isValid()
            && index.row() > lastIndex.row()) {
            q->setModelIndexes(mItemView->model()->index(
                index.row() - mItems.count() + mItemsPerRow, 0));
        }
    }
}

void HbGridItemContainerPrivate::scrollToPositionAtTop(const QModelIndex &index)
{
    Q_Q(HbGridItemContainer);

    q->setModelIndexes(index);
}

void HbGridItemContainerPrivate::scrollToPositionAtBottom(const QModelIndex &index)
{
    Q_Q(HbGridItemContainer);
    HbModelIterator *modelIterator = mItemView->modelIterator();
    q->setModelIndexes(modelIterator->index(
            modelIterator->indexPosition(index) - mItems.count() + mItemsPerRow));
}

void HbGridItemContainerPrivate::scrollToPositionAtCenter(const QModelIndex &index)
{
    Q_Q(HbGridItemContainer);
    HbModelIterator *modelIterator = mItemView->modelIterator();
    q->setModelIndexes(modelIterator->index(
            modelIterator->indexPosition(index)- mItems.count()/2));
}

QModelIndex HbGridItemContainerPrivate::lastValidItemIndex() const
{
    int lastIndex = mItems.count() - 1;
    // it always find some valid item - buffer should contain
    // some items
    while(!mItems[lastIndex]->modelIndex().isValid()) {
        --lastIndex;
        if (lastIndex < 0) {
            return QModelIndex();
        }
    }
    return mItems[lastIndex]->modelIndex();
}

void HbGridItemContainerPrivate::updateItemBuffer()
{
    Q_Q(HbGridItemContainer);

    if (!mItemView) {
        mOldItemsPerRow = 0;
        return;
    }

    int targetCount = q->maxItemCount();

    if (targetCount == 0 || mItemsPerRow == 0) {
        while (mItems.size() > 0) {
            deleteItem(mItems.first());
        }
        mOldItemsPerRow = 0;
    } else {
        if (targetCount != mItems.size()
            || mOldItemsPerRow != mItemsPerRow) {
            QModelIndex indexInTheCenter = q->getViewIndexInTheCenter();
            updateItemBufferContent(targetCount, indexInTheCenter);
        }
        mOldItemsPerRow = mItemsPerRow;
    }
}

void HbGridItemContainerPrivate::updateItemBufferContent(int targetCount, 
                                    const QModelIndex &indexInTheCenter)
{
    int startingIndex = 0;

    // this function will calculate new first index in buffer
    // as input it take an index that should be in the center of the new view
    // and try to make it also center item in buffer - it can't be possible
    // in some cases because of model size
    if (indexInTheCenter.row() >= 0) {
        startingIndex = indexInTheCenter.row() - targetCount / 2;
        startingIndex = qMax(0, startingIndex);
    } else if (!mItems.isEmpty()) {
        startingIndex = mItems.first()->modelIndex().row();
    }

    if (targetCount > 0) {
        startingIndex = alignIndexToClosestFirstInRow(startingIndex);
        int modelSize = mItemView->model()->rowCount();
        int emptyItems = (mItemsPerRow - modelSize % mItemsPerRow) % mItemsPerRow;
        // emptyItems - empty items are allowed only at end of the buffer -
        // in case when modelSize divide with remainder (modelSize/itemsPerRow != 0)
        // there is no need for checking if modelsSize/itemsPerRow == 0
        // because extra modulo operation at the end of calculations cover
        // that case
        if (targetCount > modelSize + emptyItems) {
            // make sure that buffer will not be bigger than modelSize
            targetCount = modelSize + emptyItems;
        }

        if (startingIndex + targetCount > modelSize + emptyItems) {
            startingIndex = modelSize + emptyItems - targetCount;
            startingIndex = qMax(0, startingIndex);
        }
    }

    if (mItems.size() > targetCount) {
        decreaseBufferSize(targetCount, startingIndex);
    } else if (mItems.size() < targetCount) {
        increaseBufferSize(targetCount, startingIndex);
    }

    alignBufferContent(startingIndex);
}

void HbGridItemContainerPrivate::decreaseBufferSize(int targetCount,
                                                    int startingIndex)
{
    int removeCounter = mItems.size() - targetCount;
    while (removeCounter > 0) {
        if (!mItems.size()) break;
        if (startingIndex > mItems[0]->modelIndex().row()) {
            deleteItem(mItems.first());
        }
        else {
            deleteItem(mItems.last());
        }
        --removeCounter;
    }
}

void HbGridItemContainerPrivate::increaseBufferSize(int targetCount,
                                                    int startingIndex)
{
    Q_Q(HbGridItemContainer);
    QModelIndex index;
    if (mItems.size() == 0) {
        // in case when buffer was empty and to be sure 
        // it always has at least one item - to make rest
        // code simpler
        index = mItemView->model()->index(startingIndex, 0);
        if (index.isValid()) {
            q->insertItem(0, index);
        }
        else { // increase item buffer performed when model is empty
            return;
        }
    }
    int addCounter = targetCount - mItems.size();
    int position = 0;
    while (addCounter > 0) {
        if (startingIndex >= mItems[0]->modelIndex().row()) {
            index = mItemView->modelIterator()->nextIndex(lastValidItemIndex());
            position = mItems.size();
        }
        else {
            index = mItemView->modelIterator()->previousIndex(mItems[0]->modelIndex());
            if (index.isValid()) {
                position = 0;
            }
            else {
                position = mItems.size();
            }
        }
        q->insertItem(position, index);
        --addCounter;
    }
}

void HbGridItemContainerPrivate::alignBufferContent(int startingIndex)
{
    if (mItems.size()) {
        // align buffer
        int shiftCount = startingIndex - mItems[0]->modelIndex().row();
        if (shiftCount < 0) {
            for (int i = 0; i > shiftCount; i--) {
                shiftUpItem(false);
            }
        }
        else {
            for (int i = 0; i < shiftCount; i++) {
                shiftDownItem(true, false);
            }
        }
    }
}

int HbGridItemContainerPrivate::alignIndexToClosestFirstInRow(int index)
{
    index -= index % mItemsPerRow;
    return index;
}

int HbGridItemContainerPrivate::mapToLayoutIndex(int index) const
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
