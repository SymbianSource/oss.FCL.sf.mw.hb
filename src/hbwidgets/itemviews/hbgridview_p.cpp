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
#include <hbgridview_p.h>
#include <hbgridlayout_p.h>
#include <hbgriditemcontainer_p.h>
#include <hbgridviewitem_p.h>

#include <hbgesturefilter.h>
#include <hbgridviewitem.h>
#include <hbscrollbar.h>

#include "hbmodeliterator.h"

#include <QDebug>
#include <QGraphicsView>


const QString KDefaultLayoutOption = "default";

HbGridViewPrivate::HbGridViewPrivate() : 
    mIconVisible(true), 
    mTextVisible(true)
{
}

HbGridViewPrivate::~HbGridViewPrivate()
{
}

void HbGridViewPrivate::init()
{
    Q_Q(HbGridView);
    q->setClampingStyle(q->BounceBackClamping);
    q->setScrollingStyle(q->PanOrFlick);
    q->setVerticalScrollBarPolicy(HbScrollArea::ScrollBarAlwaysOff);
    q->setHorizontalScrollBarPolicy(HbScrollArea::ScrollBarAlwaysOff);
    q->setFrictionEnabled(false);
    q->setFlag(QGraphicsItem::ItemClipsToShape);
    QObject::connect(q, SIGNAL(scrollDirectionsChanged(Qt::Orientations)),
                      q, SLOT(scrollDirectionChanged(Qt::Orientations)));
    mLayoutOptionName = KDefaultLayoutOption;
}

/*
 Utility function to find a item under certain scene pixel
 */
QModelIndex HbGridViewPrivate::itemIndexUnder(const QPointF& point)
{
    Q_Q ( HbGridView );
    QModelIndex ret = QModelIndex();
    HbAbstractViewItem* hitItem = 0;
    QList<QGraphicsItem *> items = q->scene()->items(point);
    if (!items.isEmpty()) {
        int count(items.count());
        for (int current(0); current < count; ++current) {
            QGraphicsItem *item = items.at(current);
            hitItem = viewItem(item);
            if (hitItem && hitItem->modelIndex().isValid()) {
                ret = hitItem->modelIndex();
                break;
            }
        }
    }
    return ret;
}
/*!
    \reimp
*/
bool HbGridViewPrivate::visible(HbAbstractViewItem* item, bool fullyVisible) const
{
    if (item && item->isVisible()
        && item->modelIndex().isValid()) {
        return HbAbstractItemViewPrivate::visible(item, fullyVisible);
    }
    return false;
}


void HbGridViewPrivate::setIconVisible(bool iconVisible)
{
    if (iconVisible != mIconVisible) {
        mIconVisible = iconVisible;
        relayout();
    }
}

void HbGridViewPrivate::setTextVisible(bool textVisible)
{
    if (textVisible != mTextVisible) {
        mTextVisible = textVisible;
        relayout();
    }
}

void HbGridViewPrivate::relayout()
{
    QList<HbAbstractViewItem *> items = mContainer->items();
    foreach (HbAbstractViewItem *item, items) {
        HbGridViewItem *gridItem = qobject_cast<HbGridViewItem *>(item);
        if (gridItem) {
            gridItem->updateChildItems();
        }
    }
}

/*!
    Overwrites the default scroll area scrollbar updating algorithm when
    recycling is used. On recycling the scrollbar position & size is calculated
    using rows and their pixel size is not used.
*/
void HbGridViewPrivate::updateScrollBar(Qt::Orientation orientation)
{
    if (!mContainer->itemRecycling()
        || mContainer->itemPrototypes().count() != 1 
        || mContainer->items().count() == 0) {
        HbScrollAreaPrivate::updateScrollBar(orientation);
    } else {
        if (mContainer->layout() && !mContainer->layout()->isActivated()) {
            mContainer->layout()->activate();
        }
        if (orientation == Qt::Vertical) {
            updateVerticalScrollBar();
        } else {
            updateHorizontalScrollBar();
        }
    }
} 

void HbGridViewPrivate::updateHorizontalScrollBar()
{    
    Q_Q(const HbGridView);
    
    HbAbstractViewItem *firstItem = mContainer->items().first();
    qreal uniformItemWidth = firstItem->size().width();

    int rowCount = q->rowCount();
    int indexCount = mModelIterator->indexCount();

    int virtualColumnCount = indexCount / rowCount;
    int remainder = indexCount % rowCount;
    if (remainder != 0) {
        virtualColumnCount++;
    }

    int firstItemModelRowNumber = mModelIterator->indexPosition(firstItem->modelIndex());
    int firstBufferItemRowNumber = firstItemModelRowNumber / rowCount; 

    QRectF itemRect = itemBoundingRect(firstItem);
    qreal realLeftBoundary = itemRect.left();   
    qreal virtualLeftBoundary = realLeftBoundary - (firstBufferItemRowNumber*uniformItemWidth); 

    qreal containerVirtualWidth = uniformItemWidth *  virtualColumnCount;
    qreal thumbPosition(0);

    // The scrollbar "thumb" position is the current position of the contents widget divided
    // by the difference between the width of the contents widget and the width of the scroll area.
    // This formula assumes that the "thumb" of the the scroll bar is sized proportionately to
    // the width of the contents widget.
    qreal hiddenVirtualWidth = containerVirtualWidth - q->boundingRect().width();
    if (hiddenVirtualWidth != 0) {
        thumbPosition = (-virtualLeftBoundary)  / hiddenVirtualWidth;
    }

    if (thumbPosition < 0.0)
        thumbPosition = 0.0;
    else if (thumbPosition > 1.0)
        thumbPosition = 1.0;

    if (mHorizontalScrollBar) {
        if (containerVirtualWidth!=0) {
            mHorizontalScrollBar->setPageSize(qBound ( (qreal)0.0,
                                     q->boundingRect().width() / containerVirtualWidth,
                                      (qreal)1.0));
        }
        mHorizontalScrollBar->setValue(thumbPosition); 
    }
}

void HbGridViewPrivate::updateVerticalScrollBar()
{
    Q_Q(const HbGridView);

    HbAbstractViewItem *firstItem = mContainer->items().first();
    qreal uniformItemHeight = firstItem->size().height();

    int columnCount = q->columnCount();
    int indexCount = mModelIterator->indexCount();

    int virtualRowCount = indexCount / columnCount;
    int remainder = indexCount % columnCount;
    if (remainder != 0) {  //even one item requires the whole row
        virtualRowCount++;
    }

    int firstItemModelRowNumber = mModelIterator->indexPosition(firstItem->modelIndex());
    int firstBufferItemRowNumber = firstItemModelRowNumber / columnCount; 

    QRectF itemRect = itemBoundingRect(firstItem);
    qreal realTopBoundary = itemRect.top();
    qreal virtualTopBoundary = realTopBoundary - (firstBufferItemRowNumber*uniformItemHeight); 

    qreal containerVirtualHeight = uniformItemHeight *  virtualRowCount;
    qreal thumbPosition = 0;

    // The scrollbar "thumb" position is the current position of the contents widget divided
    // by the difference between the height of the contents widget and the height of the scroll area.
    // This formula assumes that the "thumb" of the the scroll bar is sized proportionately to
    // the height of the contents widget.
    qreal hiddenVirtualHeight = containerVirtualHeight - q->boundingRect().height();
    if (hiddenVirtualHeight != 0) {
        thumbPosition = (-virtualTopBoundary) / hiddenVirtualHeight;
    } 

    if (thumbPosition < 0.0)
        thumbPosition = 0.0;
    else if (thumbPosition > 1.0)
        thumbPosition = 1.0;

    if (mVerticalScrollBar) {
        if (containerVirtualHeight!=0) {
            mVerticalScrollBar->setPageSize(qBound ( (qreal)0.0,
                                     q->boundingRect().height() / containerVirtualHeight,
                                      (qreal)1.0));
        }
        mVerticalScrollBar->setValue(thumbPosition); 
    }    
}

void HbGridViewPrivate::setScrollBarMetrics(Qt::Orientation orientation)
{   
    if (!mContainer->itemRecycling()
        || mContainer->itemPrototypes().count() != 1 
        || mContainer->items().count() == 0)  {
        HbScrollAreaPrivate::setScrollBarMetrics(orientation);
    } else {
        //We just make sure that the base clas is not called
        //It set the page size wrongly
        updateScrollBar(orientation); 
    }
} 

void HbGridViewPrivate::setContentPosition( qreal value, Qt::Orientation orientation, bool animate )
{
    Q_Q( HbGridView );

    if (mContainer->itemRecycling() && mModelIterator->model()) {
        if (mContainer->layout() && !mContainer->layout()->isActivated()) {
            mContainer->layout()->activate();
        }

        qreal filteredValue = (int)(value * 1000) / 1000.0;        

        HbAbstractViewItem *firstItem = mContainer->items().first();

        qreal uniformItemDimension = 0; // width or height of item
        qreal dimension = 0; // width or height of view
        int dimensionCount = 0; // rowcount or columncount
        qreal posInBeginning = 0; // top or left position of first item in buffer

        if (orientation == Qt::Vertical) {
            posInBeginning = itemBoundingRect(firstItem).top();
            uniformItemDimension = firstItem->size().height();
            dimension = q->boundingRect().height();
            dimensionCount = q->columnCount();
        } else {
            posInBeginning = itemBoundingRect(firstItem).left();
            uniformItemDimension = firstItem->size().width();
            dimension = q->boundingRect().width();
            dimensionCount = q->rowCount();
        }

        int indexCount = mModelIterator->indexCount();
        int virtualCount = indexCount / dimensionCount; // amount of rows/columns in "complete" grid
        int remainder = indexCount % dimensionCount;
        if (remainder != 0) {  //even one item requires the whole row
            virtualCount++;
        }

        qreal target = virtualCount * filteredValue;  // target position in "complete" grid (in rows/columns)
        int virtualItemCount = virtualCount * dimensionCount; // item count when all the "empty" items are also counted in
        qreal posToBeInView = dimension * filteredValue; 

        QModelIndex newIndex = mModelIterator->index(qMin((int)(virtualItemCount * filteredValue), indexCount - 1));

        if (!mContainer->itemByIndex(newIndex)) {
            //jump
            int itemsInBuffer = mContainer->items().count();

            int newBufferStartItem = (int)(virtualItemCount * filteredValue) - qMin(itemsInBuffer - 1, (int)(itemsInBuffer * filteredValue));
            mContainer->setModelIndexes(mModelIterator->index(newBufferStartItem));
            int newBufferStartRow = newBufferStartItem / dimensionCount;

            qreal posToBeInBuffer = ((target - newBufferStartRow) * uniformItemDimension);

            qreal posToBe = posToBeInView - posToBeInBuffer;

            if (orientation == Qt::Vertical) {
                HbScrollAreaPrivate::setContentPosition(QPointF(0, posToBe)); 
            } else {
                HbScrollAreaPrivate::setContentPosition(QPointF(posToBe, 0)); 
            }
        } else {
            // scroll
            int firstItemRow = mContainer->items().first()->modelIndex().row() / dimensionCount;

            qreal posToBeInBuffer = (target - firstItemRow) * uniformItemDimension;
            
            qreal posToBe = posToBeInView - posToBeInBuffer;

            if (orientation == Qt::Vertical) {
                q->scrollByAmount(QPointF(0, posInBeginning - posToBe));
            } else {
                q->scrollByAmount(QPointF(posInBeginning - posToBe, 0));
            }
        }
    } else {
        HbScrollAreaPrivate::setContentPosition(value, orientation, animate);
    }
       
    if (animate) {
        updateScrollBar(orientation);
    }
}

QModelIndex HbGridViewPrivate::indexInTheCenter(Qt::Orientations scrollDirection) const
{
    Q_Q(const HbGridView);
    QModelIndex centerViewModelIndex; // nearest to center of the screen
    if (!mContainer->items().isEmpty()) {
        qreal centerPos;
        HbGridItemContainer *container = itemContainer();
        if (scrollDirection == Qt::Vertical) {
            // calculating row:
            centerPos = -container->pos().y() + q->boundingRect().height() / 2
                        // take rather item that is just above of view center instead of bottom one
                        - container->viewLayout()->effectiveSizeHint(Qt::MinimumSize).height() / 2;
            // calculate item row
            centerPos /= container->viewLayout()->effectiveSizeHint(Qt::MinimumSize).height();
            //calculate item index
            centerPos *= itemContainer()->columnCount();
            // go to center column
            centerPos += itemContainer()->columnCount() / 2;
        } else {
            // calculating row:
            centerPos = -container->pos().x() + q->boundingRect().width() / 2
                        // take rather item that is just on the left of view center instead of right one
                        - container->viewLayout()->effectiveSizeHint(Qt::MinimumSize).width() / 2;
            // calculate buffer row
            centerPos /= container->viewLayout()->effectiveSizeHint(Qt::MinimumSize).width();
            //calculate item index
            centerPos *= itemContainer()->rowCount();
            // go to center row
            centerPos += itemContainer()->rowCount() / 2;
        }
        int centerIndex = qRound(centerPos);
        if (centerIndex >= container->items().size()) {
            centerIndex = container->items().size() / 2;
        }
        centerViewModelIndex = container->items().at(centerIndex)->modelIndex();
    }
    return centerViewModelIndex;
}
