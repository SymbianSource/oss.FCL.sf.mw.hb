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
#include <hbabstractitemview_p.h>
#include <hbgriditemcontainer_p_p.h>
#include <hbmodeliterator.h>

#include <QMetaObject>

/*!
    \class HbGridItemContainer
    \brief HbGridItemContainer implements HbAbstractItemContainer.
    @internal

    Creates HbGridLayout, manages grid view items.
     
    SeeAlso HbAbstractItemContainer,HbGridView,HbGridViewParameters,HbGridLayout

    IMPORTANT!!
    It is possible that last row will not be full - int this case it contain
    invisible items with have BAD_INDEX. Visibile property of item is also
    used to determine if removed item was really removed or it was previously
    removed from model and currently some item above or bellow should be removed
    (for better understanding look to:
    void HbGridItemContainerPrivate::removeItem(const QModelIndex &index))
*/

/*!
    Constructor.
 */
HbGridItemContainer::HbGridItemContainer(QGraphicsItem *parent)
                    :HbAbstractItemContainer( *new HbGridItemContainerPrivate(), parent)
{
    Q_D(HbGridItemContainer);
    d->q_ptr = this;
    d->init();
}
/*!
    Constructor.
*/
HbGridItemContainer::HbGridItemContainer( HbGridItemContainerPrivate &dd, QGraphicsItem *parent )
                      :HbAbstractItemContainer(dd, parent)
{
    Q_D(HbGridItemContainer);
    d->q_ptr = this;
    d->init();
}

/*!
    Destructor.
 */
HbGridItemContainer::~HbGridItemContainer()
{
}

/*!
    \reimp Adds item for model \a index to container.
*/
void HbGridItemContainer::addItem(const QModelIndex &index, bool animate)
{
    Q_D(HbGridItemContainer);

    if (!index.isValid())
        return;

    int bufferIndex = 0;
    QModelIndex firstInBuffer;
    int firstInBufferPosition = -1;
    int indexPosition = d->mItemView->modelIterator()->indexPosition(index);
    if (!d->mItems.isEmpty()) {
        firstInBuffer = d->mItems.first()->modelIndex();
        firstInBufferPosition = d->mItemView->modelIterator()->indexPosition(firstInBuffer);
        bufferIndex = qMax(0, indexPosition - firstInBufferPosition);
    }
    // inserting new item because of buffer size
    if (d->mItems.isEmpty()
        || d->mItems.count() < maxItemCount()) {
        insertItem(bufferIndex, index, animate);
        viewLayout()->invalidate();
    }
    // special case - only for grid, if added item is above the 
    // visible region we need to shift all visible items by one!
    else if (!d->mItems.isEmpty()
        && firstInBufferPosition > indexPosition) {
        d->shiftUpItem(animate); // shift up in this case always return something
        viewLayout()->invalidate();
    }
    // new item is in visible range
    else if (bufferIndex < d->mItems.count()) {
        // new added item comes to buffer - it will be
        // recycled from last item in buffer (it is also ok
        // when that last item was invalid)
        HbAbstractViewItem *last = d->mItems.last();
        if (animate) {
            last->setOpacity(0.0);
        }
        setItemModelIndex(last, index);
        viewLayout()->moveItem(last, d->mapToLayoutIndex(bufferIndex), animate);
        d->mItems.move(d->mItems.count() - 1, bufferIndex);
        viewLayout()->invalidate();
    }

    if (d->mItems.count() % d->mItemsPerRow == 1) { // scrollable area size has been changed
        resize(viewLayout()->sizeHint(Qt::PreferredSize));
    }
}

/*!
    \reimp Removes item representing \a index from container.
*/
void HbGridItemContainer::removeItem(const QModelIndex &index, bool animate)
{
    Q_D(HbGridItemContainer);

    d->removeItem(index, animate);
}
    
/*!
    \reimp
*/
void HbGridItemContainer::reset()
{
    Q_D(HbGridItemContainer);
    HbAbstractItemContainer::reset();
    d->resetBuffer();
}

/*!
    \reimp
    Set model indexes starting from \a startIndex. If \a startIndex is not
    a index of first item in a row, then it is calculated automatically 
    (prevent) from different content looking (grid items arrangement should be
    always the same). If there is not enough items function fetch data that
    are before \a startIndex. After calling this function item, with specified 
    index is in container but it position depends on model size.
    If \a startIndex is invlaid then container is filled starting from first 
    item in model.
*/
void HbGridItemContainer::setModelIndexes(const QModelIndex &startIndex)
{
    Q_D(HbGridItemContainer);

    if (!d->mItemView || !d->mItemView->model()) {
        return;
    }

    HbModelIterator *modelIterator = d->mItemView->modelIterator();
    QModelIndex index = startIndex;
    if (!index.isValid()) {
        index = modelIterator->nextIndex(QModelIndex());
        if (!index.isValid()) {
            // this mean model is empty
            return;
        }
    }

    int indexPosition = modelIterator->indexPosition(index);
    indexPosition = d->alignIndexToClosestFirstInRow(indexPosition);
    index = modelIterator->index(indexPosition);

    int modelItemsCount = modelIterator->indexCount();
    int itemsCount = d->mItems.count();
    int diff = indexPosition + itemsCount - modelItemsCount;
    if (diff >= d->mItemsPerRow) {
        // starting from index do not fill the buffer
        // so new starting index need to be calculated
        // to fill the buffer with items
        int newStartIndex = modelItemsCount - itemsCount;
        int remainder = newStartIndex % d->mItemsPerRow;;
        if (remainder) {
            // move newStartIndex forward to contain
            // last row and empty items
            newStartIndex += d->mItemsPerRow - remainder;
        }
        index = modelIterator->index(newStartIndex);
        if (!index.isValid()) {
            // if invalid get first item from model
            index = modelIterator->nextIndex(QModelIndex());
        }
        indexPosition = modelIterator->indexPosition(index);
    }

    if (d->mItems.first()->modelIndex() == index) {
        // container already contain right items
        return;
    }

    int i = 0;
    for (; i < itemsCount && index.isValid(); ++i) {
        setItemModelIndex(d->mItems.at(i), index);
        index = modelIterator->nextIndex(index);
    }

    if (i < itemsCount) {
        for (; i % d->mItemsPerRow != 0; i++) {
            setItemModelIndex(d->mItems.at(i), index);
        }
        if (i < itemsCount) {
            // somehow model size was change
            // this is almost impossible do get there -
            // means that items were removed but view 
            // was not noticed about that - or setModelIndexes
            // was call before model has noticed view
            while (i > d->mItems.count()) {
                d->mItems.removeLast();
            }
        }
    }
}

/*!
    Returns HbGridLayout used by gridview.
*/
HbGridLayout *HbGridItemContainer::viewLayout() const
{
    Q_D(const HbGridItemContainer);
    return d->mLayout;
}

/*!
    \reimp
*/
void HbGridItemContainer::viewResized(const QSizeF &)
{
    Q_D(HbGridItemContainer);
    d->resetBuffer();
}

/*!
    \reimp
*/
void HbGridItemContainer::setItemModelIndex(HbAbstractViewItem *item, 
                                            const QModelIndex &index)
{
    Q_D(HbGridItemContainer);

    if (item->modelIndex() != index) {
        d->mLayout->recycleItems(itemByIndex(index), item);
    }

    HbAbstractItemContainer::setItemModelIndex(item, index);
    if (!index.isValid()/* && item->isVisible()*/) {
        // this is needed because this is only way to determine 
        // if there were no item, or it was already removed from model
        item->setVisible(false);
    }
    else if (index.isValid() && !item->isVisible()) {
        item->setVisible(true);
    }
}

/*!
    \reimp
*/
QPointF HbGridItemContainer::recycleItems(const QPointF &delta)
{
    Q_D(HbGridItemContainer);

    if (d->mPrototypes.count() != 1) {
        return delta;
    }

    // current invisible space can be scrolled by base class
    // recycling need only do the rest
    const qreal diff = d->getDiffWithoutScrollareaCompensation(delta);

    if (diff != 0.0) {
        HbModelIterator *modelIterator = d->mItemView->modelIterator();
        qreal result = 0.0;
        qreal containerSize = (d->mScrollDirection == Qt::Vertical)
            ? size().height() : size().width();
        bool doFarJump = false;
        if (qAbs(diff) > containerSize) {
            // if huge diff - current buffer does not containt any item that should
            // be there after jump - because of that use setModelIndexes instead of
            // recycling items - faster
            // but it is possible that even if far jump was requested (huge delta) 
            // it can't be done because of model size and current position (at the end)
            if (diff > 0) {
                // scrolling down
                int indexPos = modelIterator->indexPosition(d->lastValidItemIndex())
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
            // without leaving it for scroll area)
            result = d->farRecycling(delta);
        }
        else {
            result = d->recycling(diff);
        }

        QPointF newDelta(Qt::Vertical == d->mScrollDirection
                ? QPointF(0.0, delta.y() - result) 
                : QPointF(delta.x() - result, 0.0));

        return newDelta;
    }

    return delta;
}

/*!
    \reimp
*/
void HbGridItemContainer::itemRemoved( HbAbstractViewItem *item, bool animate )
{
    if (item) {
        Q_D(HbGridItemContainer);

        d->mLayout->removeItem(item, animate);
        d->mLayout->invalidate();
    }
}

/*!
    \reimp
*/
void HbGridItemContainer::itemAdded(int index, HbAbstractViewItem *item, bool animate)
{
    if (item) {
        Q_D(HbGridItemContainer);
        if (animate) {
            item->setOpacity(0.0);
        }
        d->mLayout->insertItem(d->mapToLayoutIndex(index), item, animate);
        d->mLayout->invalidate();
     }
}

/*!
    Updates rows and columns count according to changed \a newOrientation.
*/
void HbGridItemContainer::orientationChanged(Qt::Orientation)
{
    Q_D(HbGridItemContainer);
    if(d->mItemView && d->mLayout) {
        qSwap(d->mRowCount, d->mColumnCount);
        d->mLayout->setRowCount(d->mRowCount);
        d->mLayout->setColumnCount(d->mColumnCount);
        d->resetBuffer();
    }
}

/*!
    Items are arranged in gridLayout according to \a scrollDirection
    Qt::Horizontal layout in horizontal direction and Qt::Vertical layout 
    in vertical direction.
*/
void HbGridItemContainer::scrollDirectionChanged(Qt::Orientations scrollDirection)
{
    Q_D(HbGridItemContainer);
    if(d->mScrollDirection != scrollDirection){
        viewLayout()->setScrollDirection(scrollDirection);
        d->mScrollDirection = scrollDirection;
        d->resetBuffer();
    }
}

/*!
   Sets total no of rows to \a rowCount.
*/
void HbGridItemContainer::setRowCount(int rowCount)
{
    Q_D(HbGridItemContainer);

    Q_ASSERT_X(rowCount > 0, "HbGridItemContainer::setRowCount", "row count can not be <= 0");

    if ((rowCount > 0) && (rowCount != d->mRowCount)) {
        d->mRowCount = rowCount;
        viewLayout()->setRowCount(d->mRowCount);
        d->resetBuffer();
    }
}

/*!
   Returns rowCount.
*/
int HbGridItemContainer::rowCount() const
{
    return viewLayout()->rowCount();
}

/*!
    Sets total no of columns to \a columnCount.
*/
void HbGridItemContainer::setColumnCount(int columnCount)
{
    Q_D(HbGridItemContainer);

    Q_ASSERT_X(columnCount > 0, "HbGridItemContainer::setColumnCount", "column count can not be <= 0");

    if ((columnCount > 0) && (columnCount != d->mColumnCount)) {
        d->mColumnCount = columnCount;
        d->mLayout->setColumnCount(d->mColumnCount);
        d->resetBuffer();
    }
}

/*!
    Returns columnCount.
*/
int HbGridItemContainer::columnCount() const
{
    return viewLayout()->columnCount();
}

/*!
    \reimp
*/
int HbGridItemContainer::maxItemCount() const
{
    Q_D(const HbGridItemContainer);

    int count = HbAbstractItemContainer::maxItemCount();
    // inform the grid layout if recycling is on or off
    viewLayout()->setRecycling(d->mItemView->itemRecycling());
    // check if it has been specified, how many items can be show at a time.
    if (d->mItemView->itemRecycling()) {
        count = qMin(d->mMinCount, count);
    }
    return count;
}

/*!
    \reimp
*/
QVariant HbGridItemContainer::itemChange(GraphicsItemChange change, const QVariant & value)
{
    return HbAbstractItemContainer::itemChange(change, value);
}

HbAbstractViewItem *HbGridItemContainer::createDefaultPrototype() const
{
    return new HbGridViewItem();
}

/*!
    Scrolls container to show \a index.
*/
void HbGridItemContainer::scrollTo(const QModelIndex &index, HbAbstractItemView::ScrollHint hint)
{
    Q_D(HbGridItemContainer);

    if (!index.isValid() || d->mItems.isEmpty())
        return;
    
    switch (hint) {
        case HbAbstractItemView::EnsureVisible: d->scrollToEnsureVisible(index); break;
        case HbAbstractItemView::PositionAtTop: d->scrollToPositionAtTop(index); break;
        case HbAbstractItemView::PositionAtBottom: d->scrollToPositionAtBottom(index); break;
        case HbAbstractItemView::PositionAtCenter: d->scrollToPositionAtCenter(index); break;
    };
}

QModelIndex HbGridItemContainer::lastValidItemIndex() const
{
    Q_D(const HbGridItemContainer);
    return d->lastValidItemIndex();
}

QModelIndex HbGridItemContainer::getViewIndexInTheCenter() const
{
    Q_D(const HbGridItemContainer);
    return HbAbstractItemViewPrivate::d_ptr(d->mItemView)->mVisibleIndex;
}

void HbGridItemContainer::animationFinished(const HbEffect::EffectStatus &status)
{
    Q_D(HbGridItemContainer);

    HbAbstractViewItem *item = static_cast<HbAbstractViewItem *>(status.item);
    
    int itemCount = d->mAnimatedItems.count();
    for (int i = 0; i < itemCount; ++i) {
        QPair<HbAbstractViewItem *, int> animatedItem = d->mAnimatedItems.at(i);
        if (animatedItem.first == item) {
            d->mAnimatedItems.removeAt(i);
            break;
        }
    }

    d->mLayout->removeItem(item, true);  
    item->hide();
    QMetaObject::invokeMethod(item, "deleteLater", Qt::QueuedConnection);
}

void HbGridItemContainer::layoutAnimationFinished(QGraphicsLayoutItem *item, HbGridLayout::AnimationType animationType)
{
    Q_D(HbGridItemContainer);

    if (animationType == HbGridLayout::InsertAnimation) {
        item->graphicsItem()->setOpacity(1.0);
        HbEffect::start(item->graphicsItem(), "gridviewitem", "appear", d->mItemView, "_q_animationFinished");
    }
}

/*!
    \reimp
    In grid case items are always same size
*/
void HbGridItemContainer::setUniformItemSizes(bool enable)
{
    Q_UNUSED(enable);
    // d->mUniformItemSizes - always true
}

/*!
    \reimp

    All other sizehints are taken from grid layout except preferred sizehint. Preferred sizeHint for grid container
    is maximum height & width.
*/
QSizeF HbGridItemContainer::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    if (which == Qt::PreferredSize) {
        return QSizeF(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);       
    } else {
        return HbAbstractItemContainer::sizeHint(which, constraint);
    }
}

/*!
    \reimp

    Resizes the container. 
    
    If vertical scrolling is used view width and layout preferred height are used. 
    
    If horizontal scrolling is used view height and layout preferred width are used.
*/
void HbGridItemContainer::resizeContainer()
{
    Q_D(HbGridItemContainer);
    
    qreal width = 0;
    qreal height = 0;

    if (d->mItemView) {
        if (d->mScrollDirection == Qt::Vertical) {
            width = d->mItemView->size().width();
            height = layout()->preferredHeight();
        } else {
            width = layout()->preferredWidth();
            height = d->mItemView->size().height();
        }
    }

    resize(width, height);
}

#include "moc_hbgriditemcontainer_p.cpp"

