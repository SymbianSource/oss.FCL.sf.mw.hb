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

#include "hbabstractitemview_p.h"
#include "hbabstractitemview.h"
#include "hbabstractviewitem.h"
#include "hbabstractitemcontainer_p.h"
#include "hbmodeliterator.h"

#include <hbinstance.h>
#include <hbscrollbar.h>
#include <hbapplication.h>
#include <hbeffect.h>
#include <hbpangesture.h>
#include <hbwidgetfeedback.h>

#include <QGraphicsSceneMouseEvent>
#include <QEvent>
#include <QItemSelectionModel>
#include <QGraphicsScene>
#include <QGraphicsLayout>
#include <QTimer>
#include <QGestureEvent>
#include <QDebug>

static const qreal CONTIGUOUS_SELECTION_SCROLL_SPEED = 0.2;
static const qreal CONTIGUOUS_SELECTION_AREA_THRESHOLD = 0.2;

HbAbstractItemViewPrivate::HbAbstractItemViewPrivate() :
    mSelectionMode(HbAbstractItemView::NoSelection),
    mOptions(NoOptions),
    mSelectionSettings(None),
    mContainer(0),
    mSelectionModel(0),
    mContSelectionAction(QItemSelectionModel::NoUpdate),
    mClearingSelection(false),
    mAnimateItems(false),
    mPostponedScrollHint(HbAbstractItemView::PositionAtTop),
    mPreviousSelectedCommand(QItemSelectionModel::NoUpdate),
    mAnimationTimer(0),
	mModelIterator(0),
    mEnabledAnimations(HbAbstractItemView::All),
    mLongPressEnabled(true),
	mDoingContiguousSelection(false)
{
}

HbAbstractItemViewPrivate::~HbAbstractItemViewPrivate()
{
    if (mModelIterator) {
        delete mModelIterator;
        mModelIterator = 0;
    }
}

/*!

*/
void HbAbstractItemViewPrivate::init(HbAbstractItemContainer *container, HbModelIterator *modelIterator)
{
    Q_Q(HbAbstractItemView);

    q->setLongPressEnabled(true);
    q->setFlag(QGraphicsItem::ItemIsFocusable, true);
    q->setFocusPolicy(Qt::StrongFocus);
    
    q->setContentWidget(container);

    q->grabGesture(Qt::PanGesture);

    //mAlignment = 0; // no alignment - there is no sense with recycling

    mContainer = container;
    mContainer->setItemView(q);

    mModelIterator = modelIterator;

    q->connect(mContainer, SIGNAL(itemCreated(HbAbstractViewItem *)),
            q, SLOT(itemCreated(HbAbstractViewItem *)));

    HbMainWindow *window = q->mainWindow();
    if (window
        && q->scene()) { // added to scene
        q->connect(window, SIGNAL(aboutToChangeOrientation()),
                   q, SLOT(orientationAboutToBeChanged()));

        q->connect(window, SIGNAL(orientationChanged(Qt::Orientation)),
                   q, SLOT(orientationChanged(Qt::Orientation)));

        if (q->verticalScrollBar()) {
            q->verticalScrollBar()->installSceneEventFilter(q);
        }
        if (q->horizontalScrollBar()) {
            q->horizontalScrollBar()->installSceneEventFilter(q);
        }
    }
}

/*!
    Replaces current model with the given one. This deletes the existing
    view items and calls reset() to update the view to correspond to 
    current model. 
*/
void HbAbstractItemViewPrivate::setModel(QAbstractItemModel *model)
{
    Q_Q(HbAbstractItemView);

    if (model != mModelIterator->model()) {
        mAnimateItems = false;
        clearCurrentModel();
        mModelIterator->setModel(model);
        initializeNewModel();

        q->reset();

        if (!mAnimationTimer) {
            mAnimationTimer = new QTimer(q);
            mAnimationTimer->setObjectName(QString("animationTimer"));
            mAnimationTimer->setSingleShot(true);

            QObject::connect(mAnimationTimer, SIGNAL(timeout()), q, SLOT(_q_animationEnabled()));
        }

        mAnimationTimer->start(3000);
    }
}

/*!
    Resets current model,selection Model,mRootIndex and mCurrentIndex to null. 
*/
void HbAbstractItemViewPrivate::clearCurrentModel()
{
    Q_Q(HbAbstractItemView);
    if (mModelIterator->model()) {
        QAbstractItemModel *model = mModelIterator->model();
        q->disconnect(model, SIGNAL(destroyed()),
                      q, SLOT(_q_modelDestroyed()));
        q->disconnect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                      q, SLOT( dataChanged(QModelIndex,QModelIndex)));
        q->disconnect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                      q, SLOT(rowsInserted(QModelIndex,int,int)));
        q->disconnect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                      q, SLOT(rowsRemoved(QModelIndex,int,int)));
        q->disconnect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                      q, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));
        q->disconnect(model, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
                      q, SLOT(rowsAboutToBeInserted(QModelIndex,int,int)));
        q->disconnect(model, SIGNAL(columnsInserted(QModelIndex,int,int)),
                        q, SLOT(columnsInserted(QModelIndex,int,int)));
        q->disconnect(model, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
                        q, SLOT(columnsAboutToBeInserted(QModelIndex,int,int)));
        q->disconnect(model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
                        q, SLOT(columnsRemoved(QModelIndex,int,int)));
        q->disconnect(model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                        q, SLOT(columnsAboutToBeRemoved(QModelIndex,int,int)));
        q->disconnect(model, SIGNAL(modelReset()), q, SLOT(reset()));
        q->disconnect(model, SIGNAL(layoutChanged()), q, SLOT(modelLayoutChanged()));

        mModelIterator->setModel(0);
    }

    setSelectionModel(0);

    mCurrentIndex = QModelIndex();
    mModelIterator->setRootIndex(QPersistentModelIndex());
}

/*!
    Updates current selectionModel to selectionModel.If selectionModel is invalid, current
    selectionModel is not updated.
*/
void HbAbstractItemViewPrivate::setSelectionModel(QItemSelectionModel *selectionModel)
{
    Q_Q( HbAbstractItemView );
    if (selectionModel
        && selectionModel->model() != mModelIterator->model()) {
        qWarning("QAbstractItemView::setSelectionModel() failed: "
                 "Trying to set a selection model, which works on "
                 "a different model than the view.");
        return;
    }

    if (mSelectionModel) {
        q->disconnect(mSelectionModel, SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
                       q, SLOT(currentSelectionChanged(QItemSelection, QItemSelection)));

        q->disconnect(mSelectionModel, SIGNAL(currentChanged(QModelIndex, QModelIndex)),
                       q, SLOT(currentIndexChanged(QModelIndex, QModelIndex)));

        delete mSelectionModel;
        mSelectionModel = 0;
    }

    mSelectionModel = selectionModel;

    if (mSelectionModel) {
        q->connect(mSelectionModel, SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
                   q, SLOT(currentSelectionChanged(QItemSelection, QItemSelection)));
        q->connect(mSelectionModel, SIGNAL(currentChanged(QModelIndex, QModelIndex)),
                    q, SLOT(currentIndexChanged(QModelIndex, QModelIndex)));
    }
}

/*!
    Initializes newModel
*/
void HbAbstractItemViewPrivate::initializeNewModel()
{
    Q_Q(HbAbstractItemView);

    if (mModelIterator->model()) {
        QAbstractItemModel *model = mModelIterator->model();
        // These asserts do basic sanity checking of the model
        Q_ASSERT_X(model->index(0,0) == model->index(0,0),
                   "HbAbstractItemView::setModel",
                   "A model should return the exact same index "
                   "(including its internal id/pointer) when asked for it twice in a row.");
        Q_ASSERT_X(model->index(0,0).parent() == QModelIndex(),
                   "HbAbstractItemView::setModel",
                   "The parent of a top level index should be invalid");

        q->connect(model, SIGNAL(destroyed()),
                   q, SLOT(_q_modelDestroyed()));
        q->connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                   q, SLOT( dataChanged(QModelIndex,QModelIndex)));
        q->connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                   q, SLOT(rowsInserted(QModelIndex,int,int)));
        q->connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                   q, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));
        q->connect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                   q, SLOT(rowsRemoved(QModelIndex,int,int)));
        q->connect(model, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
                   q, SLOT(rowsAboutToBeInserted(QModelIndex,int,int)));
        q->connect(model, SIGNAL(columnsInserted(QModelIndex,int,int)),
                      q, SLOT(columnsInserted(QModelIndex,int,int)));
        q->connect(model, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
                   q, SLOT(columnsAboutToBeInserted(QModelIndex,int,int)));
        q->connect(model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
                      q, SLOT(columnsRemoved(QModelIndex,int,int)));
        q->connect(model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                   q, SLOT(columnsAboutToBeRemoved(QModelIndex,int,int)));
        q->connect(model, SIGNAL(modelReset()), q, SLOT(reset()));
        q->connect(model, SIGNAL(layoutChanged()), q, SLOT(modelLayoutChanged()));

        setSelectionModel(new QItemSelectionModel(model, q));
    }   
}

/*!
    \private

    Slot is called whenever the model is destroyed.
    deletes all children and disconnects all signal- slot connections
    with model and selectionmodel
*/
void HbAbstractItemViewPrivate::_q_modelDestroyed()
{
    Q_Q(HbAbstractItemView);

    mModelIterator->setModel(0);
    setSelectionModel(0);
    q->reset();
}

void HbAbstractItemViewPrivate::_q_animationEnabled()
{
    mAnimateItems = true;
}

void HbAbstractItemViewPrivate::_q_animationFinished(const HbEffect::EffectStatus &status)
{
    Q_UNUSED(status);
    if (status.effectEvent == "appear") {
        if (mPostponedScrollIndex.isValid()) { 
            int count = mAppearAnimationIndexes.count();
            for (int i=0; i<count; i++) {
                if (mPostponedScrollIndex == mAppearAnimationIndexes.at(i)) {
                    scrollTo(mPostponedScrollIndex, mPostponedScrollHint);
                    break;
                }
            }
        } 

        status.item->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
        mAppearAnimationIndexes.clear();
    }
}

/*!
    \reimp
*/
void HbAbstractItemViewPrivate::setContentPosition(qreal value, Qt::Orientation orientation, bool animate)
{
    Q_Q(HbAbstractItemView);

    if (handleScrollBar(orientation)) {
        if (mContainer->layout() && !mContainer->layout()->isActivated()) {
            mContainer->layout()->activate();
        }

        HbAbstractViewItem *firstItem = mContainer->items().first();
        qreal itemHeight;
        if (mContainer->uniformItemSizes()) {
            itemHeight = firstItem->size().height();
        } else {
            // avarrage height based on container content
            itemHeight = mContainer->size().height() / mContainer->items().size();
        }
        qreal visiblePos = -mContainer->pos().y()
                           + mModelIterator->indexPosition(firstItem->modelIndex()) * itemHeight;
        qreal viewHeight = q->boundingRect().height();
        qreal modelHeight = itemHeight * mModelIterator->indexCount() - viewHeight;
        qreal thumbPos = visiblePos / modelHeight;

        qreal diff = (value - thumbPos) * modelHeight;

        q->scrollByAmount(QPointF(0, diff));
    } else {
        HbScrollAreaPrivate::setContentPosition(value, orientation, animate);
    }

    if (animate) {
        updateScrollBar(orientation);
    }
}

bool HbAbstractItemViewPrivate::panTriggered(QGestureEvent *event)
{
    Q_Q(HbAbstractItemView);

    HbPanGesture *gesture = static_cast<HbPanGesture *>(event->gesture(Qt::PanGesture));

    switch (gesture->state()) {
        case Qt::GestureStarted:
            mOptions |= PanningActive;
            // Fallthrough
        case Qt::GestureUpdated: {
            QPointF scenePos = event->mapToGraphicsScene(gesture->hotSpot());
            if (mDoingContiguousSelection) {
                int retVal = false;

                // loop through the items in the scene
                qreal scenePosY = scenePos.y();
                QPointF lastScenePos = scenePos + gesture->lastOffset() - gesture->offset();
                qreal lastScenePosY = lastScenePos.y();
                QPolygonF polygon;
                polygon << lastScenePos << scenePos;
                QList<QGraphicsItem *> items = q->scene()->items(polygon);
                int itemCount = items.count();
                for (int current = 0; current < itemCount ; ++current) {
                    HbAbstractViewItem *item = viewItem(items.at(current));
                    if (item && item->itemView() == q) {
                        QModelIndex itemIndex(item->modelIndex());
                        QGraphicsSceneMouseEvent mouseMoveEvent(QEvent::GraphicsSceneMouseMove);
                        QPointF scenePosInItemCoordinates = item->mapFromScene(scenePos);
                        QPointF position(qBound((qreal)0.0, scenePosInItemCoordinates.x(), item->size().width()), 
                                         qBound((qreal)0.0, scenePosInItemCoordinates.y(), item->size().height()));
                        mouseMoveEvent.setPos(position);
                        QItemSelectionModel::SelectionFlags command = q->selectionCommand(item, &mouseMoveEvent);

						// in contiguousselectionarea there shall be no panning from HbScrollArea, thus return true
                        if (command != QItemSelectionModel::NoUpdate) {
                            retVal = true;
                        }

                        if ( itemIndex != mPreviousSelectedIndex
                          || command != mPreviousSelectedCommand) {
                            mPreviousSelectedIndex = itemIndex;
                            mPreviousSelectedCommand = command;
                            mSelectionModel->select(itemIndex, command);
                        }

                        // check if we need to start or keep on scrolling
                        int scrollDirection = 0;
                        QPointF pos = q->mapFromScene(scenePos);
                        if (pos.y() < (q->size().height() * CONTIGUOUS_SELECTION_AREA_THRESHOLD)) {
                             if (q->isScrolling()
                                 || (!q->isScrolling()
                                     && lastScenePosY >= scenePosY)) {                                
                                 scrollDirection = 1;
                             }
                        } else if (pos.y() > (q->size().height() * (1 - CONTIGUOUS_SELECTION_AREA_THRESHOLD))) {
                             if (q->isScrolling()
                                 || (!q->isScrolling()
                                     && lastScenePosY <= scenePosY)) {
                                 scrollDirection = -1;
                             }                        
                        }

                        // Start scrolling if needed. 
                        if (scrollDirection != 0) {
                            if (!mIsAnimating) {
                                mPositionInContiguousSelection = scenePos;
                                QObject::connect(q, SIGNAL(scrollPositionChanged(QPointF)), q, SLOT(_q_scrolling(QPointF)));    
                                QObject::connect(q, SIGNAL(scrollingEnded()), q, SLOT(_q_scrollingEnded()));    
                                QObject::connect(q, SIGNAL(scrollingStarted()), q, SLOT(_q_scrollingStarted()));    
                                animateScroll(QPointF (0.0f, scrollDirection * CONTIGUOUS_SELECTION_SCROLL_SPEED));
                                retVal = true;
                            }
                        } else if (q->isScrolling()) {
                            stopAnimating();
                            retVal = true;
                        }
                        break;
                    }
                }
                return retVal;
            }
            else if (!mDoingContiguousSelection){
                HbWidgetFeedback::continuousTriggered(q, Hb::ContinuousScrolled);
            }
            break;
        }
        case Qt::GestureFinished: 
        case Qt::GestureCanceled: {
            mOptions &= ~PanningActive;
            if (mDoingContiguousSelection) {
                stopAnimating();
                mDoingContiguousSelection = false;
                return true;
            }
            else {
                HbWidgetFeedback::continuousStopped(q, Hb::ContinuousScrolled);
            }
            break;
        }
        default:
            break;
    }

    return false;
}

/*!
    This slot is called when the view is scrolling doing countinuousselection. It does the item 
    selection/deselection 
*/
void HbAbstractItemViewPrivate::_q_scrolling(QPointF newPosition)
{
    Q_UNUSED(newPosition);

    HbAbstractViewItem* hitItem = itemAt(mPositionInContiguousSelection);
    if (hitItem) {
        QModelIndex itemIndex(hitItem->modelIndex());
        if ( itemIndex != mPreviousSelectedIndex) {
            mPreviousSelectedIndex = itemIndex;
            mSelectionModel->select(itemIndex, mPreviousSelectedCommand);
        }
    }
}

/*!
    This slot is called when scrolling during continuousselction ends. It restores the original scrolling parameters 
*/
void HbAbstractItemViewPrivate::_q_scrollingEnded()
{
    Q_Q(HbAbstractItemView);

    mFrictionEnabled = mOrigFriction;

    QObject::disconnect(q, SIGNAL(scrollPositionChanged(QPointF)), q, SLOT(_q_scrollingI(QPointF)));    
    QObject::disconnect(q, SIGNAL(scrollingEnded()), q, SLOT(_q_scrollingEnded()));    
    QObject::disconnect(q, SIGNAL(scrollingStarted()), q, SLOT(_q_scrollingStarted()));    
}

/*!
    This slot is called when scrolling during continuousselction starts. It saves the original scrolling parameters 
*/
void HbAbstractItemViewPrivate::_q_scrollingStarted()
{
    mOrigFriction = mFrictionEnabled;
    mFrictionEnabled = false;
}

/*!
    \private

    When orientation switch occurs, 1) or 2) is applied to view after layout switch:
          1) if last item is wholly visible, it will be visible
          2) if last item is not fully visible, the first fully visible item before layout switch is made the 
		     first fully visible item
 */
void HbAbstractItemViewPrivate::saveIndexMadeVisibleAfterMetricsChange()
{
    QModelIndex firstVisibleModelIndex;
    QModelIndex lastVisibleModelIndex;
    mContainer->firstAndLastVisibleModelIndex(firstVisibleModelIndex, lastVisibleModelIndex);

    if (mModelIterator->model()) {
        if (lastVisibleModelIndex == mModelIterator->index(mModelIterator->indexCount() - 1)) {
            mVisibleIndex = lastVisibleModelIndex;
        } else {
            mVisibleIndex = firstVisibleModelIndex;
        }
    }
}

/*!
    \private

    Minimum amount of pixels to scroll to make \a item visible.
    Negative value scrolls view up, and positive value vice versa.
    Returns 0, if no scrolling is needed.

    Horizontally ensures that item is visible. 
 */
QPointF HbAbstractItemViewPrivate::pixelsToScroll(const HbAbstractViewItem *item,
                                                  HbAbstractItemView::ScrollHint hint)
{
    Q_Q(HbAbstractItemView);

    QPointF result(0,0);

    if (item) {
        mContainer->resizeContainer();

        QRectF itemRect = itemBoundingRect(item);
        QRectF viewRect = q->boundingRect();

        if (!viewRect.isValid()) {
            return result;
        }

        QSizeF sizeOffset;
        sizeOffset.setHeight(qMin(itemRect.height(), viewRect.height()));
        sizeOffset.setWidth(qMin(itemRect.width(), viewRect.width()));

        if (mScrollDirections & Qt::Vertical) {
            switch (hint) {
                case HbAbstractItemView::PositionAtTop: {
                    result.setY(itemRect.bottom() - viewRect.top() - sizeOffset.height());
                    break;
                }
                case HbAbstractItemView::PositionAtBottom: {
                    result.setY(itemRect.top() + sizeOffset.height() - viewRect.bottom());
                    break;
                }
                case HbAbstractItemView::PositionAtCenter: {
                    qreal yCentre = viewRect.top() + (viewRect.height()) / 2 ;
                    result.setY(itemRect.top() - yCentre + sizeOffset.height()/2);
                    break;
                }
                case HbAbstractItemView::EnsureVisible:
                default: {
                    if (itemRect.top() < viewRect.top()) { 
                        result.setY(itemRect.bottom() - viewRect.top() - sizeOffset.height());
                    } else if (itemRect.bottom() > viewRect.bottom()) {
                        result.setY(itemRect.top() + sizeOffset.height() - viewRect.bottom());
                    }
                    break;
                }
            }

            if (itemRect.width() < viewRect.width()) {
                if (itemRect.left() < viewRect.left()) {
                    result.setX(itemRect.left() - viewRect.left());
                } else if (itemRect.right() > viewRect.right()) {
                    result.setX(itemRect.right() - viewRect.right());
                }
            } else {
                // item does not fit in the screen, always align according to 
                // mirroring 
                if (HbApplication::layoutDirection() == Qt::LeftToRight) {
                    result.setX(itemRect.left() - viewRect.left());
                } else {                
                    result.setX(itemRect.right() - viewRect.right());
                }
            }
        } else if (mScrollDirections & Qt::Horizontal) {
            switch (hint) {
                case HbAbstractItemView::PositionAtTop: {    // left
                    result.setX(itemRect.right() - viewRect.left() - sizeOffset.width());
                    break;
                }
                case HbAbstractItemView::PositionAtBottom: { // right
                    result.setX(itemRect.left() + sizeOffset.width() - viewRect.right());
                    break;
                }
                case HbAbstractItemView::PositionAtCenter: {
                    qreal xCentre = viewRect.left() + (viewRect.width()) / 2 ;
                    result.setX(itemRect.left() - xCentre + sizeOffset.width()/2);
                    break;
                }
                case HbAbstractItemView::EnsureVisible:
                default: {
                    if (itemRect.left() < viewRect.left()) { 
                        result.setX(itemRect.right() - viewRect.left() - sizeOffset.width());
                    } else if (itemRect.right() > viewRect.right()) {
                        result.setX(itemRect.left() + sizeOffset.width() - viewRect.right());
                    }
                    break;
                }
            }

            if (itemRect.top() < viewRect.top()) {
                result.setY(itemRect.top() - viewRect.top());
            } else if (itemRect.bottom() > viewRect.bottom()) {
                result.setY(itemRect.bottom() - viewRect.bottom());
            }
        }
    }

    return result;
}

QItemSelectionModel::SelectionFlags HbAbstractItemViewPrivate::singleSelectionCommand(
        const HbAbstractViewItem *item,
        const QEvent *event)
{
    if (item) {
        switch (event->type())  {
        case QEvent::GraphicsSceneMousePress: 
        case QEvent::GraphicsSceneMouseDoubleClick:
            if (item->selectionAreaContains(static_cast<const QGraphicsSceneMouseEvent *>(event)->pos(), HbAbstractViewItem::SingleSelection)) {
                mSelectionSettings |= Selection;
            }
            break;
        case QEvent::GraphicsSceneMouseRelease:
            if (mSelectionSettings.testFlag(Selection)) {
                mSelectionSettings &= ~Selection;
                return QItemSelectionModel::ClearAndSelect;
            }
            break;
        default:
            break;
        }
    }

    return QItemSelectionModel::NoUpdate;
}


QItemSelectionModel::SelectionFlags HbAbstractItemViewPrivate::multiSelectionCommand(
        const HbAbstractViewItem *item,
        const QEvent *event)
{
    Q_Q(HbAbstractItemView);
    if (item) {
        switch (event->type()) {
        case QEvent::GraphicsSceneMousePress: 
        case QEvent::GraphicsSceneMouseDoubleClick: {

            // check if the mouse click is in the multiselectionarea
		    if (item->selectionAreaContains(static_cast<const QGraphicsSceneMouseEvent *>(event)->pos(), HbAbstractViewItem::MultiSelection)) {
		        mSelectionSettings |= Selection;
                if (mSelectionModel && mSelectionModel->isSelected(item->modelIndex())) {
                    mContSelectionAction = QItemSelectionModel::Deselect;
                } else {
                    mContSelectionAction = QItemSelectionModel::Select;
                }
            }

            // check if the mouse click is in the "contiguousselectionarea"
            if (item->selectionAreaContains(static_cast<const QGraphicsSceneMouseEvent *>(event)->pos(), 
                                            HbAbstractViewItem::ContiguousSelection)) {
                // it is assumed that the "contiguousselectionarea" is in the multiselectionarea
                q->setLongPressEnabled(false);
                mDoingContiguousSelection = true;
            } else {
                mDoingContiguousSelection = false;
            }

            break;
            }
        case QEvent::GraphicsSceneMouseRelease: {
            QItemSelectionModel::SelectionFlag flags =  QItemSelectionModel::NoUpdate;
            if (mSelectionSettings.testFlag(Selection)){
                flags = mContSelectionAction;
                mSelectionSettings &= ~Selection;
                mContSelectionAction = QItemSelectionModel::NoUpdate;
            } 
            
            q->setLongPressEnabled(true);
            return flags;
            }
        case QEvent::GraphicsSceneMouseMove:
            if (mDoingContiguousSelection) {
                return mContSelectionAction;
            } else {
                return QItemSelectionModel::NoUpdate;
            }
        default:
            break;
        }
    }
    return QItemSelectionModel::NoUpdate;
}


/*!
    Returns the abstract view item from given scene position, if there is any.
*/
HbAbstractViewItem *HbAbstractItemViewPrivate::itemAt(const QPointF& position) const
{
    Q_Q(const HbAbstractItemView);

    HbAbstractViewItem *hitItem = 0;
    QList<QGraphicsItem *> items = q->scene()->items(position);
    
    int count = items.count();
    for (int current = 0; current < count; ++current) {
        QGraphicsItem *item = items.at(current);
        hitItem = viewItem(item);
        // second condition needed, because in form there can be radio button list 
        // and list of the form itself on top of each other
        if (hitItem && mContainer->items().indexOf(hitItem) != -1)
            return hitItem;
    }
    return hitItem;
}


QRectF HbAbstractItemViewPrivate::itemBoundingRect(const QGraphicsItem *item) const
{
    Q_Q(const HbAbstractItemView);

    if (mContainer) {
        QGraphicsLayout *containerLayout = mContainer->layout();
        if (containerLayout) {
            containerLayout->activate();
        }
    }

    return item->mapToItem(q, item->boundingRect()).boundingRect();
}

/*!
    Returns true if given item is located within viewport (i.e.  view), otherwise
    returns false. If fullyVisible parameter is true method will return true only
    for item that is shown fully. In this case for partially visible items false is returned.
*/
bool HbAbstractItemViewPrivate::visible(HbAbstractViewItem* item, bool fullyVisible) const
{
    Q_Q(const HbAbstractItemView);
    bool visible = false;
    if (item) {
        QRectF itemRect(itemBoundingRect(item));
        QRectF abstractViewRect(itemBoundingRect(q));
        if (fullyVisible) {
            if (abstractViewRect.contains(itemRect)) {
                visible = true;
            }
        } else {
            if (abstractViewRect.intersects(itemRect)) {
                visible = true;
            }
        }
    }
    return visible;
}

/*!
    Returns current Item.
*/
HbAbstractViewItem* HbAbstractItemViewPrivate::currentItem() const
{
    return mContainer->itemByIndex(mCurrentIndex);
}

/*!
        Tries to convert given graphics item to HbLIstViewItem.
        qgraphicsitem_cast cannot be used here as it does not support subclassing.
        Also qobject_cast cannot be used directly as QGraphicsItem is not derived from 
        QObject. But you can ask qgraphicsitem whether it is a widget or not
        and cast the item to widget based on this information. After the item is
        casted to widget then qobject_cast can be used.
*/
HbAbstractViewItem* HbAbstractItemViewPrivate::viewItem(QGraphicsItem *item) const
    {
        HbAbstractViewItem *result = 0;
        if (item && item->isWidget()) {
            result = qobject_cast<HbAbstractViewItem *>(static_cast<QGraphicsWidget *>(item));
        }
        return result;
    }

void HbAbstractItemViewPrivate::updateItems()
{
    QList<HbAbstractViewItem *> items = mContainer->items();
    foreach (HbAbstractViewItem *item, items) {
        item->updateChildItems();
    }
}

void HbAbstractItemViewPrivate::scrollTo(const QModelIndex &index, HbAbstractItemView::ScrollHint hint)
{
    Q_Q(HbAbstractItemView);
    // when called from HbAbstractItemView::scrollTo(), mPostponedScrollIndex is invalid
    HbAbstractViewItem *viewItem = q->itemByIndex(index);
    if (viewItem) {
        HbScrollArea::ClampingStyle clampingStyle = mClampingStyle;
        if (clampingStyle == HbScrollArea::BounceBackClamping) {
            mClampingStyle = HbScrollArea::StrictClamping;
        }
        revealItem(viewItem, hint);
        mClampingStyle = clampingStyle;
    } else if (     index.isValid()
                &&  index == mPostponedScrollIndex) {
        q->scrollTo(index, hint);
    }
}

void HbAbstractItemViewPrivate::revealItem(const HbAbstractViewItem *item, 
                                            HbAbstractItemView::ScrollHint hint )
{
    Q_Q(HbAbstractItemView);
    QPointF delta = pixelsToScroll(item, hint);
    if (delta != QPointF()) {
        QPointF newPos = -mContainer->pos() + delta;
        checkBoundaries(newPos);

        // scroll area logic is opposite to real position.
        q->scrollContentsTo(newPos);
    }
}
void HbAbstractItemViewPrivate::checkBoundaries(QPointF &newPos)
{
    Q_Q(HbAbstractItemView);

    if (mClampingStyle != HbScrollArea::NoClamping) {
        QRectF viewRect = q->boundingRect();
        QSizeF containerSize = mContainer->layout()->preferredSize();
        
        if (newPos.y() < topBoundary() ) {
            newPos.setY(topBoundary());
        }

        // it is possible that above checking set newPos.y > 0
        if (newPos.y() > bottomBoundary()) {
            newPos.setY(bottomBoundary()); 
        }
            
        if (newPos.x() < leftBoundary() ) {
            newPos.setX(leftBoundary());
        }

        // it is possible that above checking set newPos.x > 0
        if (newPos.x() > rightBoundary()) {
            newPos.setX(rightBoundary()); 
        }
    }
}

void HbAbstractItemViewPrivate::setScrollBarMetrics(Qt::Orientation orientation)
{
    if (!handleScrollBar(orientation) ) {
        HbScrollAreaPrivate::setScrollBarMetrics(orientation);
    } else {
        Q_Q(HbAbstractItemView);

        if (mContainer->layout() && !mContainer->layout()->isActivated()) {
            mContainer->layout()->activate();
        }

        qreal itemHeight;
        if (mContainer->uniformItemSizes()) {
            itemHeight = mContainer->items().first()->size().height();
        } else {
            // avarrage height based on container content
            itemHeight = mContainer->size().height() / mContainer->items().size();
        }
        qreal rowCount = q->boundingRect().height() / itemHeight;
        qreal modelRowCount = mModelIterator->indexCount();
        qreal thumbSize = rowCount / modelRowCount;
        mVerticalScrollBar->setPageSize(thumbSize);
    }
}

/*!
    This function combines the conditions to solve whether the scroll bar calcultion should be handled in
    this class or is the base class calculation sufficient
*/
bool HbAbstractItemViewPrivate::handleScrollBar(Qt::Orientation orientation)
{
    if (!mContainer->itemRecycling()
        || !(orientation & mScrollDirections)
        || mContainer->itemPrototypes().count() != 1
        || mContainer->items().isEmpty()
        || (!mVerticalScrollBar && orientation == Qt::Vertical)
        || (!mHorizontalScrollBar && orientation == Qt::Horizontal)) {
        return false;
    } else {
        return true;
    }
}

/*!
    Overwrites the default scroll area scrollbar updating algorithm when
    recycling is used. While recycling is on scrollbar position & size 
    are calculated using rows and their pixel sizes are not used (in fact only 
    container position is interesting to determine partially visible items
    - when container contain small amount of items it is important).
*/
void HbAbstractItemViewPrivate::updateScrollBar(Qt::Orientation orientation)
{
    if (!handleScrollBar(orientation)) {
        HbScrollAreaPrivate::updateScrollBar(orientation);
    } else {
        if (mContainer->layout() && !mContainer->layout()->isActivated()) {
            mContainer->layout()->activate();
        }

        Q_Q(const HbAbstractItemView);

        qreal containerPos = mContainer->pos().y();
        qreal itemHeight;
        if (mContainer->uniformItemSizes()) {
            itemHeight = mContainer->items().first()->size().height();
        } else {
            // avarrage height based on container content
            itemHeight = mContainer->size().height() / mContainer->items().size();
        }
        qreal rowCount = q->boundingRect().height() / itemHeight;
        qreal modelRowCount = mModelIterator->indexCount() - rowCount;
        qreal firstVisibleRow = mModelIterator->indexPosition(mContainer->items().first()->modelIndex());
        firstVisibleRow += -containerPos / itemHeight;
        qreal thumbPos = firstVisibleRow / (qreal)modelRowCount;
        mVerticalScrollBar->setValue(thumbPos);
    }
}

void HbAbstractItemViewPrivate::rowsRemoved(const QModelIndex &parent, int start, int end)
{
    if (mModelIterator->model()->columnCount(parent) == 0) {
        return;
    }

    if (start <= mCurrentIndex.row() && mCurrentIndex.row() <= end) {
        // new current: 1) next after last deleted (note that
        // start and end index in model prior to deleting)
        // 2) just before first deleted
        QModelIndex newCurrentIndex = mModelIterator->model()->index(start, 0, parent);
        if (!newCurrentIndex.isValid()) {
            newCurrentIndex = mModelIterator->model()->index(qMax(0,start-1), 0, parent);
        }

        if (mSelectionModel) {
            mSelectionModel->setCurrentIndex(newCurrentIndex, QItemSelectionModel::NoUpdate);
        } 
    }

    for (int current = end; current >= start; --current) {
        //The items are already removed from the model. That's why their indexes are already invalid.
        //Here we loop the items in container and call removeItem() with QModelIndex().
        mContainer->removeItem(QModelIndex(), animationEnabled(false));
    }
}

QItemSelectionModel::SelectionFlags HbAbstractItemViewPrivate::selectionFlags( 
                                                    const HbAbstractViewItem *item, 
                                                    const QEvent *event)
{
    if (!item || !item->modelIndex().isValid() || !(item->flags() & QGraphicsItem::ItemIsSelectable))
        return QItemSelectionModel::NoUpdate;

    QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::NoUpdate;
    if (item && event){
        switch (mSelectionMode) {
        case HbAbstractItemView::SingleSelection: 
            flags =  singleSelectionCommand(item, event);
            break;
        case HbAbstractItemView::MultiSelection: {
            flags = multiSelectionCommand(item, event);
            break;
        }
        case HbAbstractItemView::NoSelection: // Never update selection model
            break;
        }
    }

    return flags;
}

void HbAbstractItemViewPrivate::resetContainer()
{
    mPostponedScrollIndex = QPersistentModelIndex();
    mContainer->reset();
}

void HbAbstractItemViewPrivate::startAppearEffect(const QString &itemType, const QString &effectEvent, const QModelIndex &parent, int start, int end)
{
    Q_Q(HbAbstractItemView);
    if( mAppearAnimationIndexes.count()) {
        mAppearAnimationIndexes.clear();
    }
    QList< QGraphicsItem * >items;
    for (int i = start; i <= end; i++) {
        QPersistentModelIndex index = mModelIterator->index(i, parent);
        HbAbstractViewItem *item = q->itemByIndex(index);
        if (item) {
            items.append(item);
            item->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

            mAppearAnimationIndexes.append(index);
        }
    }


    if (mContainer->layout()) {
        mContainer->layout()->activate();
    }

    HbEffect::start(items, itemType, effectEvent, q, "_q_animationFinished");
}

void HbAbstractItemViewPrivate::ensureVisible(QPointF position, qreal xMargin, qreal yMargin)
{
    mPostponedScrollIndex = QPersistentModelIndex();
    HbScrollAreaPrivate::ensureVisible(position, xMargin, yMargin);
}

bool HbAbstractItemViewPrivate::animationEnabled(bool insertOperation)
{
    if (insertOperation) {
        return mEnabledAnimations & HbAbstractItemView::Appear ? mAnimateItems : false;
    } else {
        return mEnabledAnimations & HbAbstractItemView::Disappear ? mAnimateItems : false;
    }
}
