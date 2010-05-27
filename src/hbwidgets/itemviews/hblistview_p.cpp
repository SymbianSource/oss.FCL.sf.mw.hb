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

#include "hblistviewitem.h"
#include "hbabstractitemcontainer_p.h"
#include "hblistitemcontainer_p.h"
#include <hbwidgetfeedback.h>
#include "hbmodeliterator.h"
#include <hbpangesture.h>

#include <QGraphicsScene>

namespace {
    static const int FLICKMINDISTANCE = 50;
    static const qreal SCROLLSPEED_FACTOR = 0.0004;
    static const qreal FLICK_TIMEOUT = 200;
    static const qreal DRAGGED_ITEM_SCROLL_SPEED = 0.2;
    static const qreal DRAGGED_ITEM_OPACITY = 0.75;
    const QString KDefaultLayoutOption = "default";
}

HbListViewPrivate::HbListViewPrivate() :
    HbAbstractItemViewPrivate(),
    mArrangeMode(false),
    mDraggedItemIndex(),
    mDraggedItem(0),
    mMousePressTimer(),
    mMousePressPos(),
    mScrollStartMousePos(),
    mLastScrollPos(),
    mOriginalTransform(),
    mMoveOngoing(false)
{
}

HbListViewPrivate::~HbListViewPrivate()
{
}

void HbListViewPrivate::init()
{
    Q_Q(HbListView);
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
            // reached a droppable spot
            HbWidgetFeedback::triggered(mDraggedItem,Hb::InstantDraggedOver);

            QPointF itemPos = itemBoundingRect(mDraggedItem).topLeft();

            mDraggedItem->setOpacity(1.0);
            mDraggedItem->setTransform(mOriginalTransform);
            mDraggedItem->setZValue(mDraggedItem->zValue() - 1);
            //mDraggedItem->setPressed(false, false);

            int targetRow = targetIndex.row();
            int startRow = qMin(targetRow, mDraggedItemIndex.row());
            int endRow = qMax(targetRow, mDraggedItemIndex.row());
            int containerStartIndex = mContainer->items().at(0)->modelIndex().row();
            mMoveOngoing = true;
            q->move(mDraggedItemIndex, targetIndex);
            static_cast <HbListItemContainer*>(mContainer)->setItemModelIndexes(startRow - containerStartIndex,
                                                                                startRow,
                                                                                endRow - startRow);
            mMoveOngoing = false;
           
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

                    //mDraggedItem->setPressed(true, false);

                    q->setCurrentIndex(mDraggedItemIndex);
                }
            }
        }
    }
}

bool HbListViewPrivate::panTriggered(QGestureEvent *event)
{
    Q_Q(HbListView);

    bool arrangeModeEnabled = false;
    if (mArrangeMode && mSelectionMode == HbAbstractItemView::NoSelection) {
        arrangeModeEnabled = true;
    }

    HbPanGesture *gesture = static_cast<HbPanGesture *>(event->gesture(Qt::PanGesture));
    switch (gesture->state()) {
        case Qt::GestureStarted: {
            if (arrangeModeEnabled && !mDraggedItem) {
                QPointF scenePos = event->mapToGraphicsScene(gesture->startPos());
                mDraggedItem = itemAt(scenePos);
                if (mDraggedItem) {
                    mDraggedItemIndex = mDraggedItem->modelIndex();

                    if (mDraggedItemIndex.isValid()) {
                        HbEffect::cancel(mDraggedItem, QString(), true);
                        mMousePressTimer.restart();
                        mMousePressPos = scenePos;
                        mOriginalTransform = mDraggedItem->transform();
                        mDraggedItem->setZValue(mDraggedItem->zValue() + 1);
                        mDraggedItem->translate(0, event->mapToGraphicsScene(gesture->offset()).y() -
                                               event->mapToGraphicsScene(gesture->lastOffset()).y());

                        QObject::connect(q, SIGNAL(scrollPositionChanged(QPointF)), q, SLOT(scrolling(QPointF)));    
                    } else {
                        mDraggedItem = 0;
                    }
                }
                return true;
            }
        }
        case Qt::GestureUpdated: {
            if (arrangeModeEnabled && mDraggedItem) {
                QPointF scenePos = event->mapToGraphicsScene(gesture->hotSpot());
                QPointF pos = q->mapFromScene(scenePos);

                if (!q->isScrolling()) {
                    // move the item with the cursor to indicate the move
                    mDraggedItem->translate(0, event->mapToGraphicsScene(gesture->offset()).y() -
                                               event->mapToGraphicsScene(gesture->lastOffset()).y());

                    if (mMousePressTimer.elapsed() >= FLICK_TIMEOUT) {
                        moveDraggedItemTo(scenePos);
                    }
                }

                // in case we are "dragging" an item and at the top/bottom of
                // the view the view is scrolled to reveal more items in
                // that direction
                QModelIndex firstVisible;
                QModelIndex lastVisible;
                mContainer->firstAndLastVisibleModelIndex(firstVisible, lastVisible);
                if (firstVisible.isValid() && lastVisible.isValid()) {
                    // above indexes are valid so container contain at least one item - so it is
                    // safe to call first and last
                    QModelIndex firstItemIndex = mContainer->items().first()->modelIndex();
                    QModelIndex lastItemIndex = mContainer->items().last()->modelIndex();
                    // If the item is dragged up in the list (and there are more items to show), scroll up

                    if (!mIsAnimating
                        && !(visible(mContainer->itemByIndex(firstItemIndex), true) 
                             && (firstItemIndex.row() == 0))
                        && scenePos.y() < mMousePressPos.y()
                        && pos.y() < q->itemByIndex(firstVisible)->size().height()) {
                        mScrollStartMousePos = scenePos;
                        mLastScrollPos = QPointF(0,0);
                        animateScroll(QPointF(0.0f , DRAGGED_ITEM_SCROLL_SPEED));
                    }
                    // If the item is dragged down in the list (and there are more items to show), scroll down
                    else if (!mIsAnimating
                               && !(visible(mContainer->itemByIndex(lastItemIndex), true) 
                                    && (lastItemIndex.row() == mModelIterator->indexCount() - 1))
                               && scenePos.y() > mMousePressPos.y()
                               && pos.y() > (q->size().height() - q->itemByIndex(lastVisible)->size().height())) {
                        mScrollStartMousePos = scenePos;
                        mLastScrollPos = QPointF(0,0);
                        animateScroll(QPointF(0.0f , (-1 * DRAGGED_ITEM_SCROLL_SPEED)));
                    }
                    // If the view is scrolling and the drag event is inside the view, we need to stop the scrolling
                    else if (pos.y() < (q->size().height() - q->itemByIndex(lastVisible)->size().height())
                               && pos.y() > q->itemByIndex(firstVisible)->size().height()
                               && q->isScrolling()) {
                        stopAnimating();
                    }
                }
                return true;
            }
        }
        case Qt::GestureFinished: 
		case Qt::GestureCanceled: {
            if (arrangeModeEnabled && mDraggedItem) {
                // drop dragged item
                HbWidgetFeedback::triggered(mDraggedItem,Hb::InstantReleased);
                QObject::disconnect(q, SIGNAL(scrollPositionChanged(QPointF)), q, SLOT(scrolling(QPointF)));

                if (q->isScrolling()) {
                    stopAnimating();
                }

                // remove item's drag indications
                mDraggedItem->setOpacity(1.0);
                mDraggedItem->setTransform(mOriginalTransform);
                mDraggedItem->setZValue(mDraggedItem->zValue() - 1);
                //mDraggedItem->setPressed(false);

                QPointF scenePos = event->mapToGraphicsScene(gesture->hotSpot());
                if (itemAt(scenePos)) {
                    int downTime = mMousePressTimer.elapsed();
                    // this seems to be a flick rather than item move, so start 
                    // scrolling
                    qreal distance = scenePos.y() - mMousePressPos.y();
                    if (downTime > 0 && downTime < FLICK_TIMEOUT 
                        && qAbs(distance) > FLICKMINDISTANCE ) {
                        animateScroll(QPointF (0.0f, (distance * 1000 / downTime) * SCROLLSPEED_FACTOR));
                    }
                }

                mDraggedItem = 0;
                return true;
            }
        }
        default:
            break;
    }

    return HbAbstractItemViewPrivate::panTriggered(event);
}

