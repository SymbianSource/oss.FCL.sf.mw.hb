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
#include "hbabstractitemcontainer_p.h"
#include "hbabstractitemcontainer_p_p.h"

#include "hbabstractviewitem.h"
#include "hbabstractitemview.h"
#include "hbmodeliterator.h"
#include <hbapplication.h>

#include <QGraphicsLayout>
#include <QGraphicsSceneResizeEvent>
#include <QEvent>
#include <QDebug>


/*!   
    HbAbstractItemContainer is used in HbAbstractItemView to hold the layout and view items.
    Container should have a layout, otherwise its size is zero always.
    HbAbstractItemContainer can have any kind of layout as it child.
*/

/*!
    Function is called after new \a item was added into \a index position into
    container.
*/

/*!
    Function is called after the \a item was removed from the container. 
*/

/*!
    Function is called when container needs to be resized.
*/

/*!
    Returns the default prototype.

    Subclasses of this class must implement this function to introduce their own default prototype. 
    Default prototype is used to create view items unless the class user excplicitly sets prototype with setItemPrototype method. 
    Implementation of this method must construct and return a new view item widget. 
*/

const int HB_DEFAULT_BUFFERSIZE = 4;
const int UpdateItemBufferEvent = QEvent::registerEventType();

HbAbstractItemContainerPrivate::HbAbstractItemContainerPrivate() : 
    HbWidgetPrivate(),
    mItemView(0),
    mBufferSize(HB_DEFAULT_BUFFERSIZE),
    mItemRecycling(false),
    mUniformItemSizes(false)
{
}

HbAbstractItemContainerPrivate::~HbAbstractItemContainerPrivate()
{
}


void HbAbstractItemContainerPrivate::init()
{
    Q_Q(HbAbstractItemContainer);
    q->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
}

/*!
   Returns given item's bounding rectangle in scene coordinates.
 */
QRectF HbAbstractItemContainerPrivate::itemBoundingRect(const QGraphicsItem *item) const
{
    Q_Q(const HbAbstractItemContainer);
    if (q->layout()) {
        q->layout()->activate();
    }
    return item->mapToItem(mItemView, item->boundingRect()).boundingRect();
}

void HbAbstractItemContainerPrivate::firstAndLastVisibleBufferIndex(
        int& firstVisibleBufferIndex,
        int& lastVisibleBufferIndex,
        const QRectF &viewRect,
        bool fullyVisible) const
{
    Q_Q(const HbAbstractItemContainer);

    if (q->layout() && !q->layout()->isActivated()) {
        q->layout()->activate();
    }

    firstVisibleBufferIndex = -1;
    lastVisibleBufferIndex = -1;

    int count = mItems.count();
    for (int i = 0; i < count; ++i) {
        if (visible(mItems.at(i), viewRect, fullyVisible)) {
            if (firstVisibleBufferIndex == -1) {
                firstVisibleBufferIndex = i;
            }
            lastVisibleBufferIndex = i;
        } else if ( lastVisibleBufferIndex != -1 ) {
            // no need to check the remaining ones.
            break;
        }
    }
}

/*!
    \private

    Returns true if given item is located within viewport (i.e.  view), otherwise
    returns false. If fullyVisible parameter is true method will return true only
    for item that is shown fully. In this case for partially visible items false is returned.
*/
bool HbAbstractItemContainerPrivate::visible(HbAbstractViewItem* item, const QRectF &viewRect, bool fullyVisible) const
{
    if (item) {
        QRectF itemRect(itemBoundingRect(item));
        if (fullyVisible) {
            if (viewRect.contains(itemRect)) {
                return true;
            }
        } else {
            if (viewRect.intersects(itemRect)) {
                return true;
            }
        }
    }

    return false;
}

/*!
  Clears the prototype list and deletes the prototypes.
*/
void HbAbstractItemContainerPrivate::deletePrototypes()
{
    qDeleteAll(mPrototypes);
    mPrototypes.clear();
}

int HbAbstractItemContainerPrivate::findStateItem(const QModelIndex &index) const
{
    for (int current = 0; current < mItemStateList.count(); ++current) {
        if (mItemStateList.at(current).index == index) {
            return current;
        }
    }
    return -1;
}

void HbAbstractItemContainerPrivate::initPrototype(HbAbstractViewItem *prototype) const
{
    prototype->setParentItem(mItemView);
    prototype->setItemView(mItemView);
    prototype->resize(QSizeF(0, 0));
    prototype->hide();
}

HbAbstractViewItem* HbAbstractItemContainerPrivate::createItem(const QModelIndex& index)
{
    Q_Q(HbAbstractItemContainer);

    HbAbstractViewItem *result = 0;
    HbAbstractViewItem *prototype = itemPrototype(index);
    if (prototype) {
        result = q_check_ptr(prototype->createItem());
        Q_ASSERT_X(result->prototype() == prototype, "HbAbstractItemContainerPrivate::createItem", "Copy constructor must be used for creating concrete view items in createItem(). Create your custom view item with 'new YourCustomViewItem(*this)' instead of 'new YourCustomViewItem(this)'");
        result->setParentItem(q);

        emit q->itemCreated(result);
    }
    return result;
}

HbAbstractViewItem* HbAbstractItemContainerPrivate::itemPrototype(const QModelIndex& index) const
{
    Q_Q(const HbAbstractItemContainer);

    if (mPrototypes.isEmpty()) {
        HbAbstractViewItem *defaultPrototype = q->createDefaultPrototype();
        if (defaultPrototype) {
            initPrototype(defaultPrototype);

            mPrototypes.append(defaultPrototype);
        }
    }

    HbAbstractViewItem *result = 0;
    int count = mPrototypes.count() - 1;
    for (int i = count; i >= 0; i--) {
        if (mPrototypes[i]->canSetModelIndex(index)) {
            result = mPrototypes[i];
            break;
        }
    }
    return result;
}


/*!
    \private
    
    Updates item buffer to contain correct amount of items for the current situation.
*/
void HbAbstractItemContainerPrivate::updateItemBuffer()
{
    Q_Q(HbAbstractItemContainer);

    if (!mItemView) {
        return;
    }

    int targetCount = q->maxItemCount();
    int itemCount = mItems.count();

    if (itemCount == targetCount) {
        return;
    }
    
    // Store the first item position related to view.
    QPointer<HbAbstractViewItem> firstItem = mItems.value(qMax(0, itemCount - targetCount));
    QPointF firstItemPos;
    if (firstItem) {
        firstItemPos = itemBoundingRect(firstItem).topLeft();
    }

    // Perform the incresing/decreasing
    if (itemCount < targetCount) {
        increaseBufferSize(targetCount - itemCount);
    } else {
        decreaseBufferSize(itemCount - targetCount);
    }

    restoreItemPosition(firstItem, firstItemPos);
}

/*!
    Increases the item buffer size with given \a amount.

    First tries to add as many items after the last item with valid
    index as it can. If there is not enough valid indexes then inserts items
    before the first item with valid index.
*/
void HbAbstractItemContainerPrivate::increaseBufferSize(int amount)
{
    Q_Q(HbAbstractItemContainer);

    // Append new items.
    QModelIndex index;
    for (int i = mItems.count() - 1; i >= 0; --i) {
        index = mItems.at(i)->modelIndex();
        if (index.isValid()) {
            break;
        }
    }

    int itemsAdded = 0;
    // in practize following conditions must apply: itemview is empty and scrollTo() has been called.
    // Starts populating items from given mFirstItemIndex
    if (    !index.isValid()
        &&  mFirstItemIndex.isValid()) {
        index = mFirstItemIndex;
        q->insertItem(mItems.count(), index);
        itemsAdded++;

        mFirstItemIndex = QModelIndex();
    }

    while (itemsAdded < amount) {
        index = mItemView->modelIterator()->nextIndex(index);
        if (!index.isValid()) {
            break;
        }
        
        q->insertItem(mItems.count(), index);
        itemsAdded++;
    }

    if (itemsAdded == amount) {
        return;
    } 

    // Prepend new items.
    for (int i = 0; i < mItems.count(); ++i) {
        index = mItems.at(i)->modelIndex();
        if (index.isValid()) {
            break;
        }
    }

    while (itemsAdded < amount) {
        index = mItemView->modelIterator()->previousIndex(index);
        if (!index.isValid()) {
            break;
        }

        q->insertItem(0, index);
        itemsAdded++;
    }
}

/*!
    Decreases the item buffer size with given \a amount.

    Tries to avoid deleting visible items and keep the buffer balanced.
*/
void HbAbstractItemContainerPrivate::decreaseBufferSize(int amount)
{
    Q_UNUSED(amount)

    int firstVisible = 0;
    int lastVisible = 0;
    firstAndLastVisibleBufferIndex(firstVisible, lastVisible, mItemView->geometry(), false);

    int deletableItemsOnTop = firstVisible - 1;
    int deletableItemsOnBottom = mItems.count() - lastVisible - 1;
    int itemsDeleted = 0;
    HbAbstractViewItem* item = 0;

    // in decreasing the buffer we try to keep it balanced
    while (itemsDeleted < amount ) {
        if (deletableItemsOnTop > deletableItemsOnBottom) {
            item = mItems.takeFirst();
            deletableItemsOnTop--;
        } else {
            item = mItems.takeLast();
            deletableItemsOnBottom--;
        }
        deleteItem(item);
        ++itemsDeleted;
     }

}

/*!
    \private
*/
HbAbstractViewItem* HbAbstractItemContainerPrivate::item(const QModelIndex &index) const
{
    int itemCount = mItems.count();
    if (index.isValid()) {
        if (itemCount > 0) { 
            HbModelIterator *iterator = mItemView->modelIterator();
            int positionFirstIndex = iterator->indexPosition(mItems.first()->modelIndex());

            // when new items are inserted, they will appear in the end of container before a model index is assigned to them
            if (positionFirstIndex >= 0) {
                int positionIndex = iterator->indexPosition(index);
                HbAbstractViewItem *item = mItems.value(positionIndex - positionFirstIndex);
                if (    item
                    &&  item->modelIndex() == index) { 
                    return item;
                }
            }
        }
    } else {
        // searching items e.g. removed from model 
        for (int i = 0; i < itemCount; ++i) {
            if (!mItems.at(i)->modelIndex().isValid()) {
                return mItems.at(i);
            }
        }
    }
    return 0;
}

/*!
    \private

    Removes \a item with model index \a index.
*/
void HbAbstractItemContainerPrivate::doRemoveItem(HbAbstractViewItem *item, const QModelIndex &index, bool animate)
{
    if (item) {
        deleteItem(item, animate);
        if (!index.isValid()) {
            mItemStates.remove(index);
        }
    }
}


/*!
    \private

    Deletes \a item.
*/
void HbAbstractItemContainerPrivate::deleteItem(HbAbstractViewItem *item, bool animate)
{
    Q_Q(HbAbstractItemContainer);

    q->setItemTransientState(item->modelIndex(), item->transientState());
    mItems.removeOne(item);
    q->itemRemoved(item, animate);

#ifndef HB_EFFECTS
    delete item;
#else
    if (!HbEffect::effectRunning(item, "disappear") 
        && !HbEffect::effectRunning(item, "collapse")) {
        delete item;
    }
#endif
}

/*
    \private
    The previous or the next index must be in the buffer. We cannot assume in this base class 
    that the container is a list we just have to loop through the items and find if the previous
    or next exist in the buffer. Next index should not be the first one in the buffer and the
    previous index should not be the last one in the buffer in order this index to the buffer.
*/
bool HbAbstractItemContainerPrivate::intoContainerBuffer(const QModelIndex &index) const
{    
    QModelIndex nextIndex = mItemView->modelIterator()->nextIndex(index);
    QModelIndex previousIndex = mItemView->modelIterator()->previousIndex(index);

    int itemCount = mItems.count();
    for (int i = 0; i < itemCount; ++i) {
        QModelIndex currentIndex = mItems.at(i)->modelIndex();
        if (currentIndex == nextIndex && i != 0){
            return true;
        } else if (currentIndex == previousIndex && i != (itemCount - 1)) {
            return true;
        }
    }
    return false;
} 

int HbAbstractItemContainerPrivate::containerBufferIndexForModelIndex(const QModelIndex &index) const
{   
    int bufferIndex = 0;
    QModelIndex nextIndex = mItemView->modelIterator()->nextIndex(index);
    QModelIndex previousIndex = mItemView->modelIterator()->previousIndex(index);

    while (bufferIndex < mItems.count()) {
        QModelIndex currentIndex = mItems.at(bufferIndex)->modelIndex();
        if (currentIndex == nextIndex) {
            break;
        }

        ++bufferIndex;

        if (currentIndex == previousIndex) {
            break;
        }
    }
    return bufferIndex;
}

void HbAbstractItemContainerPrivate::restoreItemPosition(HbAbstractViewItem *item, const QPointF &position)
{
    Q_Q(HbAbstractItemContainer);

    if (item) {
        QPointF delta = itemBoundingRect(item).topLeft() - position;
        if (!delta.isNull()) {
            q->setPos(q->pos() - delta);

            adjustContent();
        }
    }
}

void HbAbstractItemContainerPrivate::insertItem(HbAbstractViewItem *item, int pos, const QModelIndex &index, bool animate)
{
    Q_Q(HbAbstractItemContainer);

    if (item) {
        mItems.insert(pos, item);
        q->itemAdded(pos, item, animate);

        q->setItemModelIndex(item, index);
    }
}

qreal HbAbstractItemContainerPrivate::getDiffWithoutScrollareaCompensation(const QPointF &delta) const
{
    Q_Q( const HbAbstractItemContainer);
    const QSizeF containerSize(q->size());
    const QPointF containerPos(q->pos());
    qreal diff = 0.0;
    qreal invisibleArea = 0.0;
    if (delta.y() > 0) {
        // space at the bottom
        QSizeF viewSize = mItemView->size();
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

    return diff;
}

/*!  
    Constructs a new HbAbstractItemContainer with \a parent.
*/
HbAbstractItemContainer::HbAbstractItemContainer(QGraphicsItem *parent) : 
    HbWidget(*new HbAbstractItemContainerPrivate(), parent) 
{
    Q_D(HbAbstractItemContainer);

	d->q_ptr = this;
    d->init();
}

/*!
    Constructs an item with private class object \a dd and \a parent. 
*/
HbAbstractItemContainer::HbAbstractItemContainer(HbAbstractItemContainerPrivate &dd, QGraphicsItem *parent) :
    HbWidget(dd, parent)
{
    Q_D(HbAbstractItemContainer);

    d->q_ptr = this;
    d->init();
}

/*!
    Destroys the container.
*/
HbAbstractItemContainer::~HbAbstractItemContainer()
{
}

/*!
    \reimp
*/
bool HbAbstractItemContainer::event(QEvent *e)
{
    if (e->type() == QEvent::LayoutRequest) {
        QGraphicsWidget *parentWid = parentWidget();
        if (!parentLayoutItem() 
            && parentWid) {
            HbApplication::postEvent(parentWid, new QEvent(QEvent::LayoutRequest));
        }
        Q_D(HbAbstractItemContainer);
        d->updateItemBuffer();
    } else if (e->type() == UpdateItemBufferEvent) {
        Q_D(HbAbstractItemContainer);
        d->updateItemBuffer();
    }

    return HbWidget::event(e);
}

/*!
    Returns view item object corresponding given model \a index. This might be 0 pointer if
    there is no view item representing given index or given \a index is invalid.
*/
HbAbstractViewItem* HbAbstractItemContainer::itemByIndex(const QModelIndex &index) const
{
    Q_D(const HbAbstractItemContainer);
    
    if (!index.isValid()) {
        return 0;
    }
    return d->item(index);
}

/*!
    This function is provided for convenience. 
    
    It removes all the items from the container and clears the internal state model.
*/
void HbAbstractItemContainer::removeItems()
{
    Q_D(HbAbstractItemContainer);

    qDeleteAll(d->mItems);
    d->mItems.clear();
    d->mItemStateList.clear();
    d->mItemStates.clear();
}

/*!
    Returns the item view that container is connected.
*/
HbAbstractItemView *HbAbstractItemContainer::itemView() const
{
    Q_D(const HbAbstractItemContainer);
    return d->mItemView;
}

/*!
    Sets item \a view that container is connected.
*/
void HbAbstractItemContainer::setItemView(HbAbstractItemView *view)
{
    Q_D(HbAbstractItemContainer);

    if (view != d->mItemView) {
        if (d->mItemView) {
            d->mItemView->removeEventFilter(this);
        }

        d->mItemView = view;

        foreach (HbAbstractViewItem *prototype, d->mPrototypes) {
            d->initPrototype(prototype);
        }

        if (d->mItemView) {
            setParentItem(view);
            d->mItemView->installEventFilter(this);
        }
    }
}

/*!
    Assigns new model \a index to the given \a item. Item's current state is saved
    and state for \a index is restored to item.
*/
void HbAbstractItemContainer::setItemModelIndex(HbAbstractViewItem *item, const QModelIndex &index)
{

    if (item && item->modelIndex() != index) { 

        setItemTransientState(item->modelIndex(), item->transientState());

        // Transfer the state from item currently representing index to new item, if such exists.
        HbAbstractViewItem *oldItem = itemByIndex(index);

        if (oldItem) {
            item->setTransientState(oldItem->transientState());
        } else {
            item->setTransientState(itemTransientState(index));
        }


        item->setModelIndex(index);
    }
}

/*!
    Sets item's model indexes starting from given \a startIndex. If \a startIndex is
    QModelIndex() then startIndex is taken from the first item. 

    \note If there are not enough model indexes from \a startIndex to last model index 
    for all the items then QModelIndex() is assigned to rest of the view items.
*/
void HbAbstractItemContainer::setModelIndexes(const QModelIndex &startIndex)
{
    Q_D(HbAbstractItemContainer);

    if (!d->mItemView || !d->mItemView->model()) {
        return;
    }

    QModelIndex index = startIndex;
    if (!index.isValid()) {
        if (!d->mItems.isEmpty()) {
            index = d->mItems.first()->modelIndex();
        } 
        
        if (!index.isValid()) {
            index = d->mItemView->modelIterator()->nextIndex(index);
        }
    }

    QModelIndexList indexList;

    int itemCount(d->mItems.count());

    if (itemCount != 0 && index.isValid()) {
        indexList.append(index);
    }

    for (int i = indexList.count(); i < itemCount; ++i) {
        index = d->mItemView->modelIterator()->nextIndex(indexList.last());

        if (index.isValid()) {
            indexList.append(index);
        } else {
            break;
        }
    }

    for (int i = indexList.count(); i < itemCount; ++i) {
        index = d->mItemView->modelIterator()->previousIndex(indexList.first());

        if (index.isValid()) {
            indexList.prepend(index);
        } else {
            break;
        }
    }

    // if items have been added/removed in the middle of the buffer, there might be items 
	// that can be reused at the end of the buffer. The following block will scan for 
	// those items and move them in correct position 
    int lastUsedItem = -1;
    for (int indexCounter = 0; indexCounter < indexList.count(); ++indexCounter) {
        HbAbstractViewItem *item = d->mItems.at(indexCounter);
        if (item && item->modelIndex() == indexList.at(indexCounter)) { 
            lastUsedItem = indexCounter;
        } else {
            for (int itemCounter = lastUsedItem + 1; itemCounter < d->mItems.count(); itemCounter++) {
                HbAbstractViewItem *item2 = d->mItems.at(itemCounter);
                if (item2->modelIndex() == indexList.at(indexCounter)) { 
                    d->mItems.swap(indexCounter, itemCounter);
                    itemRemoved(d->mItems.at(indexCounter), false);
                    itemRemoved(d->mItems.at(itemCounter), false);
                    itemAdded(qMin(indexCounter, itemCounter), d->mItems.at(qMin(indexCounter, itemCounter)), false);
                    itemAdded(qMax(indexCounter, itemCounter), d->mItems.at(qMax(indexCounter, itemCounter)), false);
                    lastUsedItem = itemCounter;
                    break;
                }
            }
        }
    }

    int indexCount(indexList.count());
    for (int i = 0; i < itemCount; ++i) {
        HbAbstractViewItem *item = d->mItems.at(i);
        HbAbstractViewItem *prototype = 0;
        // setItemModelIndex() is considered recycling. It is called only, if recycling is permitted
        if (i < indexCount) {
            prototype = d->itemPrototype(indexList.at(i));
        }
        if (prototype) {
            if (    prototype == item->prototype()
                &&  d->mItemRecycling) {
                setItemModelIndex(item, indexList.at(i));
            } else if (     d->mItemRecycling
                       ||   (   !d->mItemRecycling
                            &&  item->modelIndex() != indexList.at(i))) {
                d->deleteItem(item);
                insertItem(i, indexList.at(i));
            } // else: !d->mItemRecycling && item->modelIndex().isValid() == indexList.at(i))
        } else {
            d->deleteItem(item);
            itemCount--;
            i--;
        }
    }
}

/*!
    Returns list of items inside item buffer.
*/
QList<HbAbstractViewItem *> HbAbstractItemContainer::items() const
{
    Q_D(const HbAbstractItemContainer);
    return d->mItems;
}

/*!
    Adds item for model \a index to container.
*/
void HbAbstractItemContainer::addItem(const QModelIndex &index, bool animate)
{
    Q_D(HbAbstractItemContainer);

    // Add new item if maximum item count allows it or item falls within the range of
    // item buffer items. 
    if (d->intoContainerBuffer(index)) {
        int bufferIndex = d->containerBufferIndexForModelIndex(index); 

        if (bufferIndex >= d->mItems.count()
            || d->mItems.at(bufferIndex)->modelIndex() != index) {
            // Store the second item position related to view.
            HbAbstractViewItem *referenceItem = d->mItems.value(1);
            QPointF referenceItemPos;
            if (referenceItem) {
                referenceItemPos = d->itemBoundingRect(referenceItem).topLeft();
            }

            HbAbstractViewItem *recycledItem = 0;
            QRectF viewRect = d->itemBoundingRect(d->mItemView);

            if (d->mItemRecycling && !viewRect.isEmpty()) {
                // Recycling allowed. Try recycling the items from buffer.
                int firstVisible = 0;
                int lastVisible = 0;
                d->firstAndLastVisibleBufferIndex(firstVisible, lastVisible, viewRect, false);

                int itemsOnTop = firstVisible - 1;
                int itemsOnBottom = d->mItems.count() - lastVisible - 1;

                if (itemsOnBottom > 0) {
                    recycledItem = d->mItems.takeLast();
                } else if (itemsOnTop > 0) {
                    recycledItem = d->mItems.takeFirst();
                    bufferIndex--;
                    bufferIndex = qMax(0, bufferIndex);
                }

                if (recycledItem) {
                    itemRemoved(recycledItem, false);
                    d->insertItem(recycledItem, bufferIndex, index, animate);
                }
            }

            if (!recycledItem) {
                // No recycling has happened. Insert completely new item.
                insertItem(bufferIndex, index, animate);
            }

            // Restore second item position.
            d->restoreItemPosition(referenceItem, referenceItemPos);
            
            if (!recycledItem && d->mItemRecycling) {
                // Resize the buffer.
                d->updateItemBuffer();
            }
        }
    } else if (d->mItems.count() < maxItemCount()) {
        d->updateItemBuffer();
    }
}

/*!
    Removes item representing \a index from container.
*/
void HbAbstractItemContainer::removeItem(const QModelIndex &index, bool animate)
{
    Q_D(HbAbstractItemContainer);

    HbAbstractViewItem *item = d->item(index);
    d->doRemoveItem(item, index, animate);
}

/*!
    Removes item from \a pos.
*/
void HbAbstractItemContainer::removeItem(int pos, bool animate)
{
    Q_D(HbAbstractItemContainer);

    Q_ASSERT(pos >= 0 && pos < d->mItems.count());        

    HbAbstractViewItem *item = d->mItems.at(pos);
    QModelIndex index = item->modelIndex();
    d->doRemoveItem(item, index, animate);
}

/*!
    Derived class should implement this function to perform item recycling based on container \a delta.
    Given \a delta is the distance between container's current position and desired new position. Recycling
    should be done based on the new position and function should return the actual delta. Actual delta could
    be modified value from \a delta. Delta modification can be useful e.g. to compensate the item changes
    that are caused by relayouting of items after changing the item position within layout.

    Default implementation does not recycle items and returns the \a delta unchanged.
*/
QPointF HbAbstractItemContainer::recycleItems(const QPointF &delta)
{
    return delta;
}

/*!
    \reimp
*/
bool HbAbstractItemContainer::eventFilter(QObject *obj, QEvent *event)
{
    Q_D(HbAbstractItemContainer);

    if (obj == d->mItemView) {
        switch (event->type()){
            case QEvent::GraphicsSceneResize: {
                viewResized(d->mItemView->size());
                d->updateItemBuffer();
                break;
            }
            default:
                break;
        }
    }

    return HbWidget::eventFilter(obj, event);
}

/*!
    Returns maximum amount of items that item buffer can hold. 

    Default implementation returns the total number of indexes that can
    be accessed on current view.

    Derived class that supports item recycling should define their own implementation
    for this function.

    \note Return value should not be more than HbAbstractItemView::indexCount()
*/
int HbAbstractItemContainer::maxItemCount() const
{    
    Q_D(const HbAbstractItemContainer);

    if (d->mItemView) {
        return d->mItemView->modelIterator()->indexCount();
    } else {
        return 0;
    }
}

/*!
    Deletes other prototypes and sets \a prototype as only prototype.

    Returns true if the prototype list was changed; otherwise returns false.
*/
bool HbAbstractItemContainer::setItemPrototype(HbAbstractViewItem *prototype)
{
    QList<HbAbstractViewItem *> prototypeList;
    prototypeList.append(prototype);
    return setItemPrototypes(prototypeList);
}

/*!
    Returns the list of item prototypes.
*/
QList<HbAbstractViewItem *> HbAbstractItemContainer::itemPrototypes() const
{
    Q_D(const HbAbstractItemContainer);
    
    if (d->mPrototypes.isEmpty()) {
        HbAbstractViewItem *defaultPrototype = createDefaultPrototype();
        if (defaultPrototype) {
            d->initPrototype(defaultPrototype);

            d->mPrototypes.append(defaultPrototype);
        }
    }

    return d->mPrototypes;
}

/*!
    Sets the list of prototypes. 
    
    Returns true if the prototype list was changed; otherwise returns false.
*/
bool HbAbstractItemContainer::setItemPrototypes(const QList<HbAbstractViewItem *> &prototypes)
{
    Q_D(HbAbstractItemContainer);
    
    bool changed = false;

    if (prototypes.count() > 0) {
        if (d->mPrototypes != prototypes) {
            foreach (HbAbstractViewItem *prototype, d->mPrototypes) {
                if (!prototypes.contains(prototype)) {
                    delete prototype;
                }
            }

            foreach (HbAbstractViewItem *prototype, prototypes) {
                if (!d->mPrototypes.contains(prototype)) {
                    d->initPrototype(prototype);
                }
            }
            changed = true;
        }

        d->mPrototypes = prototypes;
    }

    return changed;
    
}

/*!
    Returns transient state of view item with \a index.
*/
QHash<QString, QVariant> HbAbstractItemContainer::itemTransientState(const QModelIndex &index) const
{
    Q_D(const HbAbstractItemContainer);
    return d->mItemStates.value(index);
}

/*!
    This is an overloaded member function, provided for convenience.

    Stores \a key with \a value of a view item with \a index into state model.
    \a key is usually name of a Qt property. If \a value is invalid, state item with the \a key is removed.

    Default values of properties should not be added.
*/
void HbAbstractItemContainer::setItemTransientStateValue(const QModelIndex &index, const QString &key, const QVariant &value)
{
    Q_D(HbAbstractItemContainer);
    if (index.isValid()) {
        QHash<QString, QVariant> stateItem = d->mItemStates.value(index);
        if (!value.isValid()) {
            stateItem.remove(key);
        } else {
            stateItem.insert(key, value);
        }
        if (stateItem.count()) {
            d->mItemStates.insert(index, stateItem);
        } else {
            d->mItemStates.remove(index);
        }
    } else {
        d->mItemStates.remove(index);
    }
}


/*!
    Stores state of a view item with \a index into item state model. State of the view item is usually 
    retrieved by calling HbAbstractViewItem::transientState().
    
    Existing state is replaced. If \a state is empty, existing state is removed. 
    Default values of state items should not be added into \a state.

    \sa HbAbstractViewItem::transientState()
*/
void HbAbstractItemContainer::setItemTransientState(const QModelIndex &index, QHash<QString,QVariant> state)
{
    Q_D(HbAbstractItemContainer);
    if (index.isValid() && state.count()) {
        d->mItemStates.insert(index, state);
    } else {
        d->mItemStates.remove(index);
    }
}

/*!
    \reimp
*/
QVariant HbAbstractItemContainer::itemChange(GraphicsItemChange change, const QVariant & value)
{
    return HbWidget::itemChange(change, value);
}

/*!
    Returns the model indexes of items that are located on top left and bottom right corners
    of visible area.
*/
void HbAbstractItemContainer::firstAndLastVisibleModelIndex(
        QModelIndex& firstVisibleModelIndex,
        QModelIndex& lastVisibleModelIndex,
        bool fullyVisible) const
{
    Q_D(const HbAbstractItemContainer);

    QRectF viewRect(d->itemBoundingRect(d->mItemView));

    int firstVisibleBufferIndex( -1 );
    int lastVisibleBufferIndex( -1 );
    d->firstAndLastVisibleBufferIndex( firstVisibleBufferIndex, lastVisibleBufferIndex, viewRect, fullyVisible );
    if (firstVisibleBufferIndex != -1) {
        firstVisibleModelIndex = d->mItems.at(firstVisibleBufferIndex)->modelIndex();
    }
    if (lastVisibleBufferIndex != -1) {
        lastVisibleModelIndex = d->mItems.at(lastVisibleBufferIndex)->modelIndex();
    }
}

/*!
    Clears the state model.
*/
void HbAbstractItemContainer::removeItemTransientStates()
{
    Q_D(HbAbstractItemContainer);
    d->mItemStateList.clear();
    d->mItemStates.clear();
}

/*!
    Sets item recycling to \a enabled.
    By default recycling is off.
 */
void HbAbstractItemContainer::setItemRecycling(bool enabled)
{
    Q_D(HbAbstractItemContainer);
    if (d->mItemRecycling != enabled) {
        d->mItemRecycling = enabled;
        if (!enabled) {
            removeItemTransientStates();
        }

        d->updateItemBuffer();
    }
}

/*!
    Returns whether item recycling feature is in use.
 */
bool HbAbstractItemContainer::itemRecycling() const
{
    Q_D(const HbAbstractItemContainer);
    return d->mItemRecycling;
}

/*!
    Sets the feature informing whether all items in the item view have the same size.
    In case all the items have the same size, the item view can do some 
    optimizations for performance purposes.
*/
void HbAbstractItemContainer::setUniformItemSizes(bool enable)
{
    Q_D(HbAbstractItemContainer);
    d->mUniformItemSizes = enable;
}

/*!
    Returns whether the uniform item sizes feature is in use.
 */
bool HbAbstractItemContainer::uniformItemSizes() const
{
    Q_D(const HbAbstractItemContainer);
    return d->mUniformItemSizes;
}

/*!
    Inserts item for \a index to \a pos.
*/
void HbAbstractItemContainer::insertItem(int pos, const QModelIndex &index, bool animate)
{
    Q_D(HbAbstractItemContainer);
    HbAbstractViewItem *item = d->createItem(index);
    d->insertItem(item, pos, index, animate);
}

/*!
    Reset the internal state of the container.
*/
void HbAbstractItemContainer::reset()
{
    // position need to be reseted while changing model
    setPos(0.0, 0.0);
    removeItems(); 
    QCoreApplication::postEvent(this, new QEvent((QEvent::Type)UpdateItemBufferEvent));
}

/*!
*/
void HbAbstractItemContainer::resizeContainer()
{
    Q_D(HbAbstractItemContainer);

    QSizeF newSize = effectiveSizeHint(Qt::PreferredSize);

    if (d->mItemView) {
        Qt::Orientations scrollingDirections = d->mItemView->scrollDirections();

        if (!scrollingDirections.testFlag(Qt::Vertical)) {
            newSize.setHeight(d->mItemView->size().height());
        } 
        
        if (!scrollingDirections.testFlag(Qt::Horizontal)) {
            newSize.setWidth(d->mItemView->size().width());
        }
    }

    resize(newSize);
}

#include "moc_hbabstractitemcontainer_p.cpp"

