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
#include "hbabstractitemcontainer.h"
#include "hbmodeliterator.h"

#include <hbgesturefilter.h>
#include <hbinstance.h>
#include <hbscrollbar.h>
#include <hbapplication.h>
#include <hbeffect.h>

#include <QGraphicsSceneMouseEvent>
#include <QEvent>
#include <QItemSelectionModel>
#include <QGraphicsScene>
#include <QGraphicsLayout>
#include <QTimer>
#include <QDebug>

HbAbstractItemViewPrivate::HbAbstractItemViewPrivate() :
    mSelectionMode(HbAbstractItemView::NoSelection),
    mOptions(NoOptions),
    mSelectionSettings(None),
    mHitItem(0),
    mContainer(0),
    mSelectionModel(0),
    mContSelectionAction(QItemSelectionModel::NoUpdate),
    mWasScrolling(false),
    mFilterRemoved(false),
    mClearingSelection(false),
    mAnimateItems(false),
    mPostponedScrollHint(HbAbstractItemView::PositionAtTop),
    mPreviousSelectedCommand(QItemSelectionModel::NoUpdate),
    mInstantClickedModifiers(0),
    mAnimationTimer(0),
	mModelIterator(0),
    mEnabledAnimations(HbAbstractItemView::All)
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

    mContainer = container;
    mContainer->setItemView(q);

    mModelIterator = modelIterator;

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
        q->disconnect(model, SIGNAL(layoutChanged()), q, SLOT(_q_layoutChanged()));

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
        q->connect(model, SIGNAL(layoutChanged()), q, SLOT(_q_layoutChanged()));

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

/*!
    \private

    Slot is called whenever the model layout changes. This resets the container.
*/
void HbAbstractItemViewPrivate::_q_layoutChanged()
{
    mContainer->setModelIndexes(mModelIterator->nextIndex(QModelIndex()));
}

void HbAbstractItemViewPrivate::_q_animationEnabled()
{
    mAnimateItems = true;
}

void HbAbstractItemViewPrivate::_q_animationFinished(const HbEffect::EffectStatus &status)
{
    Q_UNUSED(status);
    if ( status.effectEvent == "appear") {
        if (mPostponedScrollIndex.isValid()) { 
            int count = mAppearAnimationIndexes.count();
            for (int i=0; i<count; i++) {
                if (mPostponedScrollIndex == mAppearAnimationIndexes.at(i)) {
                    scrollTo(mPostponedScrollIndex, mPostponedScrollHint);
                    break;
                }
            }
        } 
        mAppearAnimationIndexes.clear();
    }
}

/*!
    \private

    When orientation switch occurs, 1) or 2) is applied to view after layout switch:
          1) if current item is wholly visible, it will be visible
          2) if current item is not wholly visible, first visible item before layout switch is made visible
          In either case the visible item is at top of the view or as near as possible
 */
void HbAbstractItemViewPrivate::saveIndexMadeVisibleAfterMetricsChange()
{
    QModelIndex firstVisibleModelIndex;
    QModelIndex lastVisibleModelIndex;
    mContainer->firstAndLastVisibleModelIndex(firstVisibleModelIndex, lastVisibleModelIndex);

    int firstVisibleRow = firstVisibleModelIndex.isValid() ? firstVisibleModelIndex.row() : 0;
    int lastVisibleRow = lastVisibleModelIndex.isValid() ? lastVisibleModelIndex.row() : 0;

    // save current, if it is visible
    firstVisibleRow = qMax(0, firstVisibleRow);
    lastVisibleRow = qMax(0, lastVisibleRow);

    if (mCurrentIndex.row() >= firstVisibleRow 
        && mCurrentIndex.row() <= lastVisibleRow) {
        mVisibleIndex = mCurrentIndex;
    } else if (mModelIterator->model()) {
        mVisibleIndex = mModelIterator->index(firstVisibleRow);
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
        refreshContainerGeometry();

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
        }
        else if (mScrollDirections & Qt::Horizontal) {
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
            if (item->selectionAreaContains(static_cast<const QGraphicsSceneMouseEvent *>(event)->scenePos())) {
                mSelectionSettings |= Selection;
            }
            break;
        case QEvent::GraphicsSceneMouseRelease:
            if (    mHitItem
                &&  item->modelIndex() == mHitItem->modelIndex()
                &&  mSelectionSettings.testFlag(Selection)) {
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
    if (item) {
        switch (event->type())  {
        case QEvent::GraphicsSceneMousePress: 
        case QEvent::GraphicsSceneMouseDoubleClick:
            if (item->selectionAreaContains(static_cast<const QGraphicsSceneMouseEvent *>(event)->scenePos())) {
                mSelectionSettings |= Selection;
            }
            break;
        case QEvent::GraphicsSceneMouseRelease:
            if (mHitItem
                && item->modelIndex() == mHitItem->modelIndex() 
                && mSelectionSettings.testFlag(Selection)) {
                mSelectionSettings &= ~Selection;
                return QItemSelectionModel::Toggle;
            }
            break;
        default:
            break;
        }
    }
    return QItemSelectionModel::NoUpdate;
}

QItemSelectionModel::SelectionFlags HbAbstractItemViewPrivate::contiguousSelectionCommand(
        const HbAbstractViewItem *item,
        const QEvent *event )
{
    Q_Q(HbAbstractItemView);
    if (item) {
        switch (event->type()) {
        case QEvent::GraphicsSceneMousePress: 
        case QEvent::GraphicsSceneMouseDoubleClick: {
            if (item->selectionAreaContains(static_cast<const QGraphicsSceneMouseEvent *>(event)->scenePos())) {
                mSelectionSettings |= Selection;

                if (mSelectionModel && mSelectionModel->isSelected(item->modelIndex())) {
                    mContSelectionAction = QItemSelectionModel::Deselect;
                } else {
                    mContSelectionAction = QItemSelectionModel::Select;
                }

                // TODO: This should be changed to changing the gesture area of effect when that is possible...
                // Gesture filter does not reset all of its internals: workaround is to delete and create the filter
                q->setLongPressEnabled(false);
                q->removeSceneEventFilter(mGestureFilter);
                mFilterRemoved = true;
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
            
            if (mFilterRemoved) {
                // setLongPressEnabled installs filter
                q->setLongPressEnabled(true);
                mFilterRemoved = false;
            }
            return flags;
            }
        case QEvent::GraphicsSceneMouseMove:
            return mContSelectionAction;
        default:
            break;
        }
    }
    return QItemSelectionModel::NoUpdate;
}

/*!
    Overwrites the default scroll area scrollbar updating algorithm when
    recycling is used. On recycling the scrollbar position & size is calculated
    using rows and their pixel size is not used.
*/
void HbAbstractItemViewPrivate::updateScrollBar(Qt::Orientation orientation)
{
    if (!handleScrollBar(orientation)) {
        HbScrollAreaPrivate::updateScrollBar(orientation);
    } else {
        if (mContainer->layout() && !mContainer->layout()->isActivated()) {
            mContainer->layout()->activate();
        }

        if (mContainer->uniformItemSizes()) {
            updateScrollBarForUniformSizedItems();
        } else {
            updateScrollBarForVariableSizedItems();
        }
    } 
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

/*!
    
*/
void HbAbstractItemViewPrivate::refreshContainerGeometry()
{
    Q_Q(const HbAbstractItemView);

    if (mContainer->layout())  {
        if (!mContainer->layout()->isActivated()) {
            // Make sure that the layout process has stopped.
            mContainer->layout()->activate();
        }
    } 

    QSizeF newSize = mContainer->effectiveSizeHint(Qt::PreferredSize);

    if (!mScrollDirections.testFlag(Qt::Vertical)) {
        newSize.setHeight(q->size().height());
    } 
    
    if (!mScrollDirections.testFlag(Qt::Horizontal)) {
        newSize.setWidth(q->size().width());
    }
       
    mContainer->resize( newSize );    
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
        // scroll area logic is oposite to real position
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

void HbAbstractItemViewPrivate::updateScrollBarForUniformSizedItems()
{
    Q_Q(const HbAbstractItemView);
    
    HbAbstractViewItem *firstItem = mContainer->items().first();
    qreal uniformItemHeight = firstItem->size().height();
    qreal containerVirtualHeight = uniformItemHeight *  (mModelIterator->indexCount());
    qreal thumbPosition(0);
    int firstBufferItemRowNumber = mModelIterator->indexPosition(firstItem->modelIndex());
     
    QRectF itemRect = itemBoundingRect(firstItem);
    qreal realTopBoundary = itemRect.top();   
    qreal virtualTopBoundary = realTopBoundary - (firstBufferItemRowNumber*uniformItemHeight); 
   
    if ((containerVirtualHeight - q->boundingRect().height()) != 0) {
        thumbPosition = 
            (-virtualTopBoundary) / (containerVirtualHeight - q->boundingRect().height());
    }  
 
    thumbPosition = qBound((qreal)0.0, thumbPosition, (qreal)1.0);
 
    if (mVerticalScrollBar) {
        if (containerVirtualHeight!=0) {
            mVerticalScrollBar->setPageSize(qBound ( (qreal)0.0,
                                     q->boundingRect().height() / containerVirtualHeight,
                                      (qreal)1.0));
        }
        mVerticalScrollBar->setValue(thumbPosition); 
    }    
}

void HbAbstractItemViewPrivate::setScrollBarMetrics(Qt::Orientation orientation)
{   
    if (!handleScrollBar(orientation) ) {
        HbScrollAreaPrivate::setScrollBarMetrics(orientation);
    } else {
        //We just make sure that the base clas is not called
        //It set the page size wrongly
        updateScrollBar(orientation); 
    }
}

/*!
    This function combines the conditions to solve whether the scroll bar calcultion should be handled in
    this class or is the base class calculation sufficient
*/
bool  HbAbstractItemViewPrivate::handleScrollBar(Qt::Orientation orientation)
{
    if (!mContainer->itemRecycling()
        || mContainer->itemPrototypes().count() != 1 
        || orientation == Qt::Horizontal
        || mContainer->items().count() == 0) {
            return false;
    } else {
        return true;
    }
}

void HbAbstractItemViewPrivate::updateScrollBarForVariableSizedItems()
{
    Q_Q(const HbAbstractItemView);
    HbAbstractViewItem *firstItem = mContainer->items().first();
 
    // View position is the amount of hidden (fully or partially)
    // rows above the view area.
    int position = mModelIterator->indexPosition(firstItem->modelIndex());
    if (position == -1) {
        return; 
    }
    qreal viewY = (qreal)(position);

    // View area height is the amount of rows within the view area.
    qreal viewH = 0;

    //Index count calculation is time consuming with tree
    int indexCount = mModelIterator->indexCount();

    // Total height is the amount of rows in the model.
    qreal totalH = indexCount;

    qreal itemTop = firstItem->mapToItem(q, firstItem->pos()).y();
    qreal viewHeight = q->size().height();
    int itemCount = mContainer->items().count();
    
    for (int i=0; i < itemCount; ++i) {
        qreal itemHeight = mContainer->items().at(i)->size().height();
        qreal itemBottom = itemTop + itemHeight;
        if (itemTop < 0) {
            // Some part of the item is above the view area.
            if (itemBottom < 0) {
                // Fully above the view area
                viewY += 1;
            } else {
                // Partially at the view area and partially above the view area.
                viewY += (1.0 - itemBottom / itemHeight);
                viewH += itemBottom / itemHeight;
            }
        } else if (itemTop < viewHeight) {
            // So part of the item is at the view area.
            if (itemBottom < viewHeight) {
                // Fully at the view area
                viewH += 1;
            } else {
                // Partially at the view area and partially below the view area.
                viewH += (viewHeight - itemTop) / itemHeight;
            }
        } else {
            break;
        }

        itemTop += itemHeight;
    }

    // Shifting the values to scrollbar range that is from 0.0-1.0. 
    qreal pos = viewY / (totalH - viewH);
    pos = qBound((qreal)0.0, pos, (qreal)1.0);

    if (mVerticalScrollBar) {
        if (indexCount!=0) {
            mVerticalScrollBar->setPageSize(viewH / (qreal)(indexCount));
        }
        mVerticalScrollBar->setValue(pos);
    }    
}

void HbAbstractItemViewPrivate::rowsRemoved(const QModelIndex &parent,int start,int end)
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
        bool animate = mEnabledAnimations & HbAbstractItemView::Disappear ? mAnimateItems : false;
        mContainer->removeItem(QModelIndex(), animate);
    }
}

QItemSelectionModel::SelectionFlags HbAbstractItemViewPrivate::selectionFlags( 
                                                    const HbAbstractViewItem *item, 
                                                    const QEvent *event)
{
    if (!item || !item->modelIndex().isValid() || !(item->flags() & QGraphicsItem::ItemIsSelectable))
        return QItemSelectionModel::NoUpdate;

    QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::NoUpdate;
    if (item && mHitItem && event){
        switch (mSelectionMode) {
        case HbAbstractItemView::SingleSelection: 
            flags =  singleSelectionCommand(item, event);
            break;
        case HbAbstractItemView::MultiSelection:
            flags =  multiSelectionCommand(item, event);
            break;
        case HbAbstractItemView::ContiguousSelection: {
            flags = contiguousSelectionCommand(item, event);
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

void HbAbstractItemViewPrivate::startAppearEffect(const QModelIndex &parent, int start, int end)
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
            mAppearAnimationIndexes.append(index);
        }
    }

    refreshContainerGeometry();

    HbEffect::start(items, "viewitem", "appear", q, "_q_animationFinished");
}

void HbAbstractItemViewPrivate::ensureVisible(QPointF position, qreal xMargin, qreal yMargin)
{
    mPostponedScrollIndex = QPersistentModelIndex();
    HbScrollAreaPrivate::ensureVisible(position, xMargin, yMargin);
}


