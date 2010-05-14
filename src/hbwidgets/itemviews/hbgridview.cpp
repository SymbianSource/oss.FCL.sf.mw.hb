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
#include <hbgridview.h>
#include <hbgridview_p.h>
#include <hbgridlayout_p.h>
#include <hbabstractviewitem.h>
#include <hbeffect.h>
#include <hbmodeliterator.h>

#include <QGraphicsSceneMouseEvent>

/*!
 @beta
 @hbwidgets
 \class HbGridView
 \brief HbGridView implements a grid view. Data is represented by HbGridViewItem.
 HbGridView is an HbAbstractItemView implementation using the model/view design pattern. 

 Since HbGridView is derived from HbAbstractItemView it can use QAbstractItemModel-derived implementations
 (for example QStandardItemModel) as a model.

 See HbAbstractItemView for emitted signals (touch events & item activation).
 
 <b> Scrolling Directions </b>
 
 Scrolling can be set to either a \link Qt::Horizontal \endlink or \link Qt::Vertical \endlink direction.
 Items are arranged according to the scroll direction i.e Qt::Horizontal lays out items 
 horizontally and Qt::Vertical lays out items vertically.

 By default the HbScrollArea::ClampingStyle used is BounceBackClamping, the HbScrollArea::ScrollingStyle is PanOrFlick,
 scrollbars are invisible and recycling and inertia are disabled.

 The following code snippet demonstrates the use of HbGridView
 \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,21}
 */

/*!
 GridView recycling is in proto state.
 */

/*!    
  See also  HbAbstractItemView,HbAbstractViewitem,HbGridViewItem,HbScrollArea
 */

/*!
 \fn void HbGridView::setUniformItemSizes(bool enabled) is public from HbAbstractItemView but for HbGridView
 Calling this method make no sense - in grid case it is reimplemented and do not change
 anything (items are always same size).
*/

/*!
 Constructs a new HbGridView with \a parent.
 */
HbGridView::HbGridView(QGraphicsItem *parent) :
    HbAbstractItemView(*new HbGridViewPrivate(), new HbGridItemContainer(), new HbModelIterator(), parent)
{
    Q_D( HbGridView );
    d->q_ptr = this;
    d->init();
}

/*!
    Constructs a grid view with a private class object \a dd, 
    \a container and \a parent.
 */
HbGridView::HbGridView(HbGridViewPrivate &dd, HbAbstractItemContainer *container, QGraphicsItem *parent) :
    HbAbstractItemView(dd, container, new HbModelIterator(), parent)
{
    Q_D( HbGridView );
    d->q_ptr = this;
    d->init();
}

/*!
    \reimp
*/
HbGridView::~HbGridView()
{
}

/*!
    Returns the total number of rows in the view.

    \sa setRowCount()
 */
int HbGridView::rowCount() const
{
    Q_D(const HbGridView);
    return d->itemContainer()->rowCount();
}

/*!
    Sets the total number of rows to \a rowCount.

    \sa HbGridView::rowCount()
 */
void HbGridView::setRowCount(int rowCount)
{
    Q_D(HbGridView);

    d->mVisibleIndex = d->indexInTheCenter();

    d->itemContainer()->setRowCount(rowCount);

    scrollTo(d->mVisibleIndex, HbAbstractItemView::PositionAtCenter);
}

/*!
    Returns the total number of columns in the view.

    \sa setColumnCount()
 */
int HbGridView::columnCount() const
{
    Q_D(const HbGridView);
    return d->itemContainer()->columnCount();
}

/*!
    Sets the total number of columns to \a columnCount.

    \sa columnCount()
 */
void HbGridView::setColumnCount(int columnCount)
{
    Q_D(HbGridView);

    d->mVisibleIndex = d->indexInTheCenter();

    d->itemContainer()->setColumnCount(columnCount);

    scrollTo(d->mVisibleIndex, HbAbstractItemView::PositionAtCenter);
}


/*!
    Returns true if icons are currently displayed in GridView.
 
    \sa setIconVisible()
 */
bool HbGridView::iconVisible() const
{
    Q_D(const HbGridView);
    return d->mIconVisible;
}

/*!
    Set visibility of icons in grid view to \a visible. All view items are updated.

    By default icons are visible.

    \sa iconVisible()
 */
void HbGridView::setIconVisible(bool visible)
{
    Q_D(HbGridView);
    d->setIconVisible(visible);
}

/*!
    Returns visibility of text in grid view.

    \sa setTextVisible()
 */
bool HbGridView::textVisible() const
{
    Q_D(const HbGridView);
    return d->mTextVisible;
}

/*!
    Sets visibility of text in grid view to \a visible. All view items are updated.

    By default text is visible.

    \sa HbGridView::textVisible()
 */
void HbGridView::setTextVisible(bool visible)
{
    Q_D(HbGridView);
    d->setTextVisible(visible);
}

/*!
    Returns item at \a row and \a column.
 */
HbAbstractViewItem *HbGridView::itemAt(int row, int column) const
{
    Q_D(const HbGridView);
   if (     row >= 0
       &&   column >= 0
       &&   column < d->itemContainer()->columnCount()
       &&   d->mModelIterator->model()) {
        int index  = row*d->itemContainer()->columnCount() + column;
        return d->itemContainer()->itemByIndex(d->mModelIterator->index(index));
   }
   return 0;
}

/*!
 \reimp
 */
void HbGridView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    Q_D(HbGridView);
    // always use container, event if recycling is off and all items are 
    // in container, but still aditional action is needed -
    // container::scrollTo is responsible for procesing all
    // posponed events (DelayedLayoutRequest)
    if (    d->mModelIterator->model()
        &&  index.model() == d->mModelIterator->model()) {
        d->itemContainer()->scrollTo(index, hint);
        HbAbstractItemView::scrollTo(index, hint);
    }
}

/*!
    \reimp
*/
void HbGridView::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // overriding HbAbstractitemView's selection behaviour
    HbScrollArea::mouseMoveEvent( event );
    event->accept();
}

/*!
    \reimp
 */
void HbGridView::orientationAboutToBeChanged()
{
    Q_D(HbGridView);
    d->mVisibleIndex = d->indexInTheCenter();
}

/*!
    \reimp
 */
void HbGridView::orientationChanged(Qt::Orientation newOrientation)
{
    Q_D(HbGridView);
    d->mContainer->setPos(0,0);
    d->itemContainer()->orientationChanged(newOrientation);

    // abstract part is enought - container update buffer
    HbAbstractItemView::scrollTo(d->mVisibleIndex, HbAbstractItemView::PositionAtCenter);

    d->mVisibleIndex = QModelIndex();
}

/*!
    \reimp
*/
void HbGridView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_D(HbGridView);

    if (parent == d->mModelIterator->rootIndex()) {
        HbAbstractItemView::rowsInserted(parent, start, end);

        bool animate = d->mEnabledAnimations & HbAbstractItemView::Appear ? d->mAnimateItems : false;
        if (animate) {
            if (d->mAppearAnimationIndexes.count()) {
                d->mAppearAnimationIndexes.clear();
            }

            for (int i = start; i <= end; i++) {
                QPersistentModelIndex index = d->mModelIterator->index(i, parent);
                HbAbstractViewItem *item = itemByIndex(index);
                if (item) {
                    d->mAppearAnimationIndexes.append(index);
                }
            }
        }
    }
}

/*!
    \reimp
*/
void HbGridView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_D(HbGridView);

    if (parent == d->mModelIterator->rootIndex() && d->mModelIterator->model()) {
        for (int i = start; i <= end; i++) {
            HbAbstractViewItem* item = d->mContainer->itemByIndex(d->mModelIterator->index(i));
            if (item) {
                bool animate = d->mEnabledAnimations & HbAbstractItemView::Disappear ? d->mAnimateItems : false;
                if (animate) {
                    d->mItemsAboutToBeDeleted.append(item);
                }
            }
        }
    }
}

/*!
    \reimp
*/
void HbGridView::rowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_D(HbGridView);

    if (parent == d->mModelIterator->rootIndex()) {
        bool animate = d->mEnabledAnimations & HbAbstractItemView::Disappear ? d->mAnimateItems: false;
        if (animate) {
            for (int i = 0; i < d->mItemsAboutToBeDeleted.count(); i++) {
                QGraphicsItem *item = d->mItemsAboutToBeDeleted.at(i);
                HbEffect::cancel(item, "appear");
                HbEffect::start(item,
                                "gridviewitem", 
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
void HbGridView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_D(const HbGridView);

    QList<HbAbstractViewItem *> items = d->mContainer->items();

    if (!items.isEmpty()
        && topLeft.parent() == d->mModelIterator->rootIndex()
        && items.first()->modelIndex().row() <= bottomRight.row()
        && topLeft.row() <= d->itemContainer()->lastValidItemIndex().row()) {
        HbAbstractItemView::dataChanged(topLeft, bottomRight);
    }
}

/*!
    This slot is connected to scrollDirectionsChanged signal emitted from HbScrollArea.
 */
void HbGridView::scrollDirectionChanged(Qt::Orientations scrollDirection)
{
    Q_D(HbGridView);
    // scroll direction changed, calculations need to be done on old value
    d->mVisibleIndex = d->indexInTheCenter((d->mScrollDirections == Qt::Vertical) 
        ? Qt::Horizontal : Qt::Vertical);
    d->mContainer->setPos(0,0);
    d->itemContainer()->scrollDirectionChanged(scrollDirection);

    // abstract part is enought - container update buffer
    HbAbstractItemView::scrollTo(d->mVisibleIndex, HbAbstractItemView::PositionAtCenter);

    d->mVisibleIndex = QModelIndex();
}

#include "moc_hbgridview.cpp"
