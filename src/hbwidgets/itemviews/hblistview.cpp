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

#include "hblistview.h"
#include "hblistview_p.h"

#include "hblistlayout_p.h"
#include "hblistviewitem.h"
#include "hblistitemcontainer_p.h"
#include "hblistitemcontainer_p.h"
#include "hbgesturefilter.h"
#include "hbscrollbar.h"
#include <hbwidgetfeedback.h>
#include "hbmodeliterator.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QAbstractItemModel>

#include <qdebug.h>
/*!
    @beta
    @hbwidgets
    \class HbListView
    \brief HbListView represents a list

    The list view is derived from QGraphicsWidget and can thus be embedded to
    other QGraphicsLayout based layouts or other widgets.

    HbListView uses the model view design pattern. The model can be any QAbstractItemModel
    based model.

    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,9}

    Each list view item is represented by an instance of HbListViewItem. HbListView
    uses HbListViewItem prototype to instantiate the list view items. HbListViewItem
    can be subclassed for customization purposes.
    Setting own prototype can be made like this:

    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,10}

    By default, HbListView uses list view item recycling. This means that only the
    visible list view items plus a small buffer of items above and below the visible
    list is instantiated at a time. When the list is scrolled the list view items are
    recycled so that the buffer size above and below the list is kept constant.

    <b> View item layout </b>

    View item layout configuration happens from HbListViewItem prototype that can be accesses with HbListViewItem::listItemPrototype function.
    
    More information about this can be found from HbListViewItem documentation.
*/

static const qreal DRAGGED_ITEM_SCROLL_SPEED = 0.2;
static const int FLICKMINDISTANCE = 50;
static const qreal FLICK_TIMEOUT = 200;
static const qreal SCROLLSPEED_FACTOR = 0.0004;

/*!
    Constructs a list view with \a parent.
 */
HbListView::HbListView(QGraphicsItem *parent)
    : HbAbstractItemView( *new HbListViewPrivate, new HbListItemContainer, new HbModelIterator(), parent)
{
    Q_D( HbListView );
    d->q_ptr = this;

    d->init();
}

/*!
    Constructs a list view with a private class object \a dd, 
    \a container and \a parent.
 */
HbListView::HbListView( HbListViewPrivate &dd, HbAbstractItemContainer *container, QGraphicsItem * parent )
    : HbAbstractItemView(dd, container, new HbModelIterator(), parent)
{
    Q_D( HbListView );
    d->q_ptr = this;

    d->init();
}


/*!
    Destructs the list view.
 */
HbListView::~HbListView()
{
}

/*
    Returns the first item prototype from the list of item prototypes 
    after trying to convert it as HbListViewItem.

    This function is provided for convenience.
*/
HbListViewItem *HbListView::listItemPrototype() const
{
    Q_D(const HbListView);
    return qobject_cast<HbListViewItem *>(d->mContainer->itemPrototypes().first());
}


/*
    Scrolls the view if necessary to ensure that the item at \a index is visible.
*/
void HbListView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    Q_D(HbListView); 

    if (!d->mModelIterator->model()
        ||  index.model() != d->mModelIterator->model()) {
        return;
    }

    //If item is in the buffer, just reveal it.
    //This is always the case if recycling is off 
    //and sometimes the case when recycling is on
    if (itemRecycling()) {
        if (!   (    d->mContainer->itemByIndex(index)
                &&  (   hint == PositionAtTop
                    ||  hint == EnsureVisible))) {
            //Now the item is not in the buffer.
            //We must first set the item to be in the buffer
            //If the item is above let's put it first and if it is below put it last
            int newIndex = -1;
            switch (hint) {
                case PositionAtTop: {
                    newIndex = index.row();
                    break;
                }
                case PositionAtBottom: {
                    int containerCount = d->mContainer->items().count();
                    newIndex = qMax(0,index.row() - containerCount + 1);
                    break;
                }
                case PositionAtCenter: {
                    int containerCount = d->mContainer->items().count();
                    newIndex = qMax(0,index.row() - containerCount / 2 );  
                    break;
                }
                case EnsureVisible:
                default: {
                    int containerCount = d->mContainer->items().count();
                    if (containerCount
                        && index < d->mContainer->items().first()->modelIndex()){
                        newIndex = index.row();
                    }
                    else {
                        newIndex = qMax(0,index.row() - containerCount + 1);  
                    }
                    break;
                }
            }
            d->mContainer->setModelIndexes(d->mModelIterator->index(newIndex));
        }
    }
    HbAbstractItemView::scrollTo(index, hint);
}

/*!
    Returns the view item representing \a row. This can be NULL if
    recycling is active and none of the loaded view items is representing
    given row.
*/
HbAbstractViewItem *HbListView::viewItem(int row) const
{
    Q_D(const HbListView);

    if (!d->mModelIterator->model()) {
        return 0;
    }

    return d->mContainer->itemByIndex(d->mModelIterator->index(row));
}

/*!
    \reimp
*/
int HbListView::type() const
{
    return Type;
}

/*!
    Returns the current arrange mode
 */
bool HbListView::arrangeMode() const
{
    Q_D(const HbListView);
    return d->mArrangeMode;
}

/*!
 * Returns the view item being dragged. This is NULL if no item is being dragged.
 */
HbAbstractViewItem *HbListView::draggedItem() const
{
    Q_D( const HbListView );

    return d->mDraggedItem;
}

/*!
    Tries to change the arrageMode. ArrangeMode is the mode where drag and drop 
    for arranging the items inside the list is supported instead of the normal 
    mouse event handling. 
    Setting the arrange mode to true will fail if lists selection mode is set. Selection 
    mode and arrange do not work together.
    Setting the mode to true also fails in case there is no model set or the underlying 
    model does not support Qt::MoveAction. 
    Returns true if the mode setting was succesfull. 
*/
bool HbListView::setArrangeMode(const bool arrangeMode)
{
    Q_D(HbListView);
    if (arrangeMode != d->mArrangeMode) {
        if (arrangeMode == true) {
            if (d->mSelectionMode != HbAbstractItemView::NoSelection
                || !d->mModelIterator->model()
                || !(d->mModelIterator->model()->supportedDropActions().testFlag(Qt::MoveAction))) {
                return false;
            }
            if (d->mGestureFilter) {
                removeSceneEventFilter(d->mGestureFilter);
                d->mFilterRemoved = true;
            }
            verticalScrollBar()->setInteractive(true);

        } else {
            if (d->mFilterRemoved) {
                installSceneEventFilter(d->mGestureFilter);
                d->mFilterRemoved = false;
            }
            verticalScrollBar()->setInteractive(false);
        }
        d->mArrangeMode = arrangeMode;
        d->mAnimateItems = !d->mArrangeMode;
    }
    return true;
}

/*!
    \reimp
*/
void HbListView::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(HbListView);
    if (d->mArrangeMode 
        && d->mSelectionMode == HbAbstractItemView::NoSelection
        && !d->mDraggedItem) {

        if (d->mFilterRemoved == false && d->mGestureFilter) {
            removeSceneEventFilter(d->mGestureFilter);
            d->mFilterRemoved = true;
        }

        d->mDraggedItem = d->itemAt(event->scenePos());
        if(d->mDraggedItem) {
            d->mDraggedItemIndex = d->mDraggedItem->modelIndex();

            if (d->mDraggedItemIndex.isValid()) {
                setCurrentIndex(d->mDraggedItemIndex);
                d->mMousePressTimer.restart();
                d->mMousePressPos = event->scenePos();
                d->mOriginalTransform = d->mDraggedItem->transform();
                d->mDraggedItem->setZValue(d->mDraggedItem->zValue() + 1);
                d->mDraggedItem->setPressed(true);

                connect(this, SIGNAL(scrollPositionChanged(QPointF)), this, SLOT(scrolling(QPointF)));    
                Hb::InteractionModifiers modifiers = 0;
                if (d->mWasScrolling) {
                    modifiers |= Hb::ModifierScrolling;
                }
                HbWidgetFeedback::triggered(d->mDraggedItem,Hb::InstantPressed,modifiers);
            } else {
                d->mDraggedItem = 0;
            }
        }
    } else {
        if (!d->mDraggedItem) {
            HbAbstractItemView::mousePressEvent(event);
        }
    }
}

/*!
    \reimp
*/
void HbListView::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(HbListView);

    if (d->mArrangeMode
        && d->mSelectionMode == HbAbstractItemView::NoSelection
        && d->mDraggedItem) {

        if (!isScrolling()) {
            // move the item with the cursor to indicate the move
            d->mDraggedItem->translate(0, event->scenePos().y() - event->lastScenePos().y());

            if (d->mMousePressTimer.elapsed() >= FLICK_TIMEOUT) {
                d->moveDraggedItemTo(event->scenePos());
            }
        }

        // in case we are "dragging" an item and at the top/bottom of
        // the view the view is scrolled to reveal more items in
        // that direction
        QModelIndex firstVisible;
        QModelIndex lastVisible;        
        d->mContainer->firstAndLastVisibleModelIndex(firstVisible, lastVisible);
        if (firstVisible.isValid() && lastVisible.isValid()) {
            // above indexes are valid so container contain at least one item - so it is
            // safe to call first and last
            QModelIndex firstItemIndex = d->mContainer->items().first()->modelIndex();
            QModelIndex lastItemIndex = d->mContainer->items().last()->modelIndex();
            // If the item is dragged up in the list (and there are more items to show), scroll up
            if (!isScrolling()
                && !isVisible(firstItemIndex)
                && event->scenePos().y() < d->mMousePressPos.y()
                && event->pos().y() < itemByIndex(firstVisible)->size().height()) {
                d->mScrollStartMousePos = event->scenePos();
                d->mLastScrollPos = QPointF(0,0);
                d->animateScroll(QPointF(0.0f , DRAGGED_ITEM_SCROLL_SPEED));
            }
            // If the item is dragged down in the list (and there are more items to show), scroll down
            else if (!isScrolling()
                       && !isVisible(lastItemIndex)
                       && event->scenePos().y() > d->mMousePressPos.y()
                       && event->pos().y() > (size().height() - itemByIndex(lastVisible)->size().height())) {
                d->mScrollStartMousePos = event->scenePos();
                d->mLastScrollPos = QPointF(0,0);
                d->animateScroll(QPointF(0.0f , (-1 * DRAGGED_ITEM_SCROLL_SPEED)));
            }
            // If the view is scrolling and the drag event is inside the view, we need to stop the scrolling
            else if (event->pos().y() < (size().height() - itemByIndex(lastVisible)->size().height())
                       && event->pos().y() > itemByIndex(firstVisible)->size().height()
                       && isScrolling()) {
                d->stopAnimating();
            }
        }
    } else {
        HbAbstractItemView::mouseMoveEvent(event);
    }
}


/*!
    This slot is called when the arrangeMode is true, user is dragging an item 
    and the underlying scrollarea is moving. 
*/void HbListView::scrolling(QPointF newPosition)
{
    Q_UNUSED(newPosition);

    Q_D(HbListView);
    if (d->mArrangeMode
        && d->mSelectionMode == HbAbstractItemView::NoSelection
        && d->mDraggedItem) {

        QPointF itemPos = d->itemBoundingRect(d->mDraggedItem).topLeft();
        if (d->mLastScrollPos.isNull()) {
            d->mLastScrollPos = itemPos;
        }        

        QPointF delta = d->mLastScrollPos - itemPos;
        d->mLastScrollPos = itemPos + delta;

        d->moveDraggedItemTo(d->mScrollStartMousePos);

        if (d->mDraggedItem) {
            d->mDraggedItem->translate(delta.x(), delta.y());
        }
    }
}



/*!
    \reimp
*/
void HbListView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(HbListView);

    if (d->mArrangeMode 
        && d->mSelectionMode == HbAbstractItemView::NoSelection
        && d->mDraggedItem) {

        disconnect(this, SIGNAL(scrollPositionChanged(QPointF)), this, SLOT(scrolling(QPointF)));

        if (isScrolling()) {
            d->stopAnimating();
        }

        // remove item's drag indications
        d->mDraggedItem->setOpacity(1.0);
        d->mDraggedItem->setTransform(d->mOriginalTransform);
        d->mDraggedItem->setZValue(d->mDraggedItem->zValue() - 1);
        d->mDraggedItem->setPressed(false);

        if (d->itemAt(event->scenePos())) {
            int downTime = d->mMousePressTimer.elapsed();
            // this seems to be a flick rather than item move, so start 
            // scrolling
            qreal distance = event->scenePos().y() - d->mMousePressPos.y();
            if (downTime > 0 && downTime < FLICK_TIMEOUT 
                && qAbs(distance) > FLICKMINDISTANCE ) {
                d->animateScroll(QPointF (0.0f, (distance * 1000 / downTime) * SCROLLSPEED_FACTOR));
            }
        }

        Hb::InteractionModifiers modifiers = 0;
        if (d->mWasScrolling) {
            modifiers |= Hb::ModifierScrolling;
        }
        HbWidgetFeedback::triggered(d->mDraggedItem,Hb::InstantReleased,modifiers);
        d->mDraggedItem = 0;

    } else {
        HbAbstractItemView::mouseReleaseEvent(event);
    }
}

/*!
    Moves the item in row \a from to row \a to. 
*/
 void HbListView::move(const QModelIndex &from, const QModelIndex &to)
{   
    Q_D(HbListView);
    
    int fromRow = from.row();
    int toRow = to.row();

    if (from == to
        || fromRow < 0
        || toRow < 0
        || fromRow >= model()->rowCount()
        || toRow >= model()->rowCount()) {
        return;
    }

    QModelIndexList indexList;
    indexList.append(from);
    QMimeData* dragAndDropData = model()->mimeData(indexList); 

    model()->removeRow(fromRow, d->mModelIterator->rootIndex());
    model()->dropMimeData(dragAndDropData, 
                                  Qt::MoveAction, 
                                  toRow, 
                                  0, 
                                  d->mModelIterator->rootIndex());
}

/*!
    \reimp
*/
void HbListView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_D(HbListView);

    if (parent == d->mModelIterator->rootIndex()) {
        HbAbstractItemView::rowsInserted(parent, start, end);
        bool animate = d->mEnabledAnimations & HbAbstractItemView::Appear ? d->mAnimateItems : false;
        if (!d->mArrangeMode && animate) {
            d->startAppearEffect(parent, start, end);
        }
    }
}

/*!
    \reimp
*/
void HbListView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_D(HbListView);

    if (parent == d->mModelIterator->rootIndex()) {
        for (int i = start; i <= end; i++) {
            HbAbstractViewItem* item = viewItem(i);
            if (item) {
                if (d->mDraggedItem == item) {
                   d->mDraggedItem = 0;
                }
                bool animate = d->mEnabledAnimations & HbAbstractItemView::Disappear ? d->mAnimateItems : false;
                if (!d->mArrangeMode && animate) {
                    d->mItemsAboutToBeDeleted.append(item);
                }
            }
        }
    }
}

void HbListView::rowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_D(HbListView);
    if (parent == d->mModelIterator->rootIndex()) {
        bool animate = d->mEnabledAnimations & HbAbstractItemView::Disappear ? d->mAnimateItems: false;
        if (animate) {
            for (int i = 0; i < d->mItemsAboutToBeDeleted.count(); i++) {
                HbEffect::start(d->mItemsAboutToBeDeleted.at(i), 
                                "viewitem", 
                                "disappear",
                                d->mContainer,
                                "animationFinished");    
            }
            d->mItemsAboutToBeDeleted.clear();
        }
        HbAbstractItemView::rowsRemoved(parent, start, end);
    }
}

/*!
    \reimp
*/
void HbListView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_D(const HbListView);

    QList<HbAbstractViewItem *> items = d->mContainer->items();

    if (!items.isEmpty() &&
        topLeft.parent() == d->mModelIterator->rootIndex()
        && items.first()->modelIndex().row() <= bottomRight.row()
        && topLeft.row() <= items.last()->modelIndex().row()) {
        HbAbstractItemView::dataChanged(topLeft, bottomRight);
    }
}

/*!
    \reimp
    Handles change concerning multiline support of secondary texts at orientation switch
*/
void HbListView::orientationChanged(Qt::Orientation newOrientation)
{
    Q_D(HbListView);
    Q_UNUSED(newOrientation);
    HbAbstractItemContainer *container = d->mContainer;
    if (container) {
        QList<HbAbstractViewItem *> prototypes = container->itemPrototypes();
        int countPrototypes = prototypes.count();
        for (int i=0; i< countPrototypes; i++) {
            HbListViewItem *prototypeItem = qobject_cast<HbListViewItem*>(prototypes.at(i));
            if (    prototypeItem
                &&  prototypeItem->stretchingStyle() == HbListViewItem::StretchLandscape
                &&  prototypeItem->graphicsSize() != HbListViewItem::Thumbnail) {
                int minimum(0);
                int maximum(0);
                prototypeItem->getSecondaryTextRowCount(minimum, maximum);
                if (    minimum != 1
                    ||  maximum != 1) {
                    QList<HbAbstractViewItem *> items = container->items();
                    int countItems = items.count();
                    for (int j=0; j< countItems; j++) {
                        HbAbstractViewItem *viewItem = items.at(j);
                        if (    viewItem
                            &&  viewItem->prototype() == prototypeItem) {
                            viewItem->updatePrimitives();
                        }
                    }
                }
            }
        }
    }
    HbAbstractItemView::orientationChanged(newOrientation);
}


#include "moc_hblistview.cpp"
