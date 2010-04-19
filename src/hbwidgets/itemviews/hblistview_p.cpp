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
    
#include "hblistview_p.h"
#include "hblistview.h"

#include "hbinstance.h"
#include "hblistlayout_p.h"
#include "hblistviewitem.h"
#include "hbapplication.h"
#include "hbscrollbar.h"
#include "hbgesturefilter.h"
#include "hbgesture.h"
#include "hbabstractitemcontainer.h"
#include <hbwidgetfeedback.h>
#include "hbmodeliterator.h"

#include <QTimeLine>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLinearLayout>
#include <QDebug>
#include <QItemSelectionModel>
#include <QGraphicsScene>

static const qreal DRAGGED_ITEM_OPACITY = 0.75;
const QString KDefaultLayoutOption = "default";

HbListViewPrivate::HbListViewPrivate() :
    HbAbstractItemViewPrivate(),
    mArrangeMode(false),
    mDraggedItemIndex(),
    mDraggedItem(0),
    mMousePressTimer(),
    mMousePressPos(),
    mScrollStartMousePos(),
    mLastScrollPos(),
    mOriginalTransform()
{
}

HbListViewPrivate::~HbListViewPrivate()
{
}

void HbListViewPrivate::init()
{
    Q_Q(HbListView);

    q->setClampingStyle(HbScrollArea::StrictClamping);
    q->setFrictionEnabled(0);
    q->setItemRecycling(true);
    mLayoutOptionName = KDefaultLayoutOption;
}

void HbListViewPrivate::moveDraggedItemTo(const QPointF &mousePosition)
{
    Q_Q(HbListView);

    if(!mDraggedItem) {
        return;
    }

    mDraggedItem->setOpacity(DRAGGED_ITEM_OPACITY);

    // find the list item under the cursor. The basic itemAt is not working
    // here as we have the draggeditem always first. So the graphicsitems are
    // iterated here to find a HbAbstractViewItem that is not the draggeditem
    HbAbstractViewItem *itemUnder = 0;
    QList<QGraphicsItem *> items = q->scene()->items(mousePosition);
    int count = items.count();
    for (int current = count - 1; current >= 0; --current) {
        QGraphicsItem *i = items.at(current);
        itemUnder = viewItem(i);
        if (itemUnder && itemUnder != mDraggedItem && itemUnder->itemView() == q) {
            break;
        }
    }

    if (itemUnder) {
        QModelIndex targetIndex;
        if (mDraggedItemIndex.row() > itemUnder->modelIndex().row() + 1 
            || mDraggedItemIndex.row() < itemUnder->modelIndex().row() - 1 ) {
                // item is far away from the original place, must move
                targetIndex = itemUnder->modelIndex();
        } else if (mDraggedItemIndex.row() < itemUnder->modelIndex().row()) {
            // moving down if drag position over the next item
            if (mDraggedItem->scenePos().y() > itemUnder->scenePos().y()) {
                targetIndex = itemUnder->modelIndex();
            }
        } else {
            // moving up if drag position over the next item
            if (mDraggedItem->scenePos().y() < itemUnder->scenePos().y()) {
                targetIndex = itemUnder->modelIndex();
            } 
        }

        if (targetIndex.isValid()) {
            // now drop
            HbWidgetFeedback::triggered(mDraggedItem,Hb::InstantDraggedOver);

            QPointF itemPos = itemBoundingRect(mDraggedItem).topLeft();

            mDraggedItem->setOpacity(1.0);
            mDraggedItem->setTransform(mOriginalTransform);
            mDraggedItem->setZValue(mDraggedItem->zValue() - 1);
            mDraggedItem->setPressed(false, false);

            int targetRow = targetIndex.row();
            q->move(mDraggedItemIndex, targetIndex);

            // in the swap the dragged item may have been deleted and recreated. So take it again here
            mDraggedItemIndex = mModelIterator->index(targetRow);
            if (mDraggedItemIndex.isValid()) {
                mDraggedItem = q->itemByIndex(mDraggedItemIndex);
                if (mDraggedItem) {
                    mDraggedItem->setTransform(mOriginalTransform);

                    // Keep the same visual position.
                    QPointF delta = itemPos - itemBoundingRect(mDraggedItem).topLeft();
                    mDraggedItem->translate(delta.x(), delta.y());

                    mDraggedItem->setOpacity(DRAGGED_ITEM_OPACITY);
                    mDraggedItem->setZValue(mDraggedItem->zValue() + 1);

                    mDraggedItem->setPressed(true, false);

                    q->setCurrentIndex(mDraggedItemIndex);
                }
            }
        }
    }
}

void HbListViewPrivate::setContentPosition( qreal value, Qt::Orientation orientation, bool animate )
{
    Q_Q( HbListView );

    if (mContainer->itemRecycling()
        && mModelIterator->model()) {

        if (mContainer->layout() && !mContainer->layout()->isActivated()) {
            mContainer->layout()->activate();
        }

        qreal filteredValue = (int)(value * 1000) / 1000.0;        

        qreal itemHeight = mContainer->items().first()->size().height();
        if (!mContainer->uniformItemSizes()) {
            // add here the wisdom to get a proper item size in case they are all
            // not of the same size
        }

        int indexCount = mModelIterator->indexCount();
        qreal containerVirtualHeight = itemHeight * indexCount;
        qreal target = (containerVirtualHeight * filteredValue) / itemHeight;

        qreal posToBeInView = q->size().height() * filteredValue;

        int newRow = (int)(indexCount * filteredValue);
        newRow = qMin(newRow, indexCount - 1);
        QModelIndex newIndex = mModelIterator->index(newRow);

        if (!mContainer->itemByIndex(newIndex)) {

            int itemsInBuffer = mContainer->items().count();
            int newBufferStartRow = newRow - qMin(itemsInBuffer - 1, (int)(itemsInBuffer * filteredValue));
            mContainer->setModelIndexes(mModelIterator->index(newBufferStartRow));

            qreal posToBeInBuffer = (target - newBufferStartRow) * itemHeight;

            qreal topToBe = posToBeInView - posToBeInBuffer;

            HbScrollAreaPrivate::setContentPosition(QPointF(0, topToBe)); 

        } else {
            qreal topInBeginning = itemBoundingRect(mContainer->items().first()).top();

            int firstRow = mContainer->items().first()->modelIndex().row();
            qreal posToBeInBuffer = (target - firstRow) * itemHeight;
            
            qreal topToBe = posToBeInView - posToBeInBuffer;


            q->scrollByAmount(QPointF(0, topInBeginning - topToBe));
        }
    } else {
        HbScrollAreaPrivate::setContentPosition(value, orientation, animate);
    }
    
    if (animate) {
        updateScrollBar(orientation);
    }
}

