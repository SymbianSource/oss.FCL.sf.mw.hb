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

#include <hbgridlayout_p.h>
#include <hbgridlayout_p_p.h>

#include <QGraphicsWidget>

/*!
    \class HbGridLayout
    \brief HbGridLayout manages geometries of grid view contents.
     @internal
*/

/*!
    Constructor.
    \param parent parent layout item.
*/
HbGridLayout::HbGridLayout(QGraphicsLayoutItem *parent)
    : QGraphicsLayout(parent), d_ptr(new HbGridLayoutPrivate())
{
    Q_D(HbGridLayout);
    d->q_ptr = this;
    d->init();
}

HbGridLayout::HbGridLayout(HbGridLayoutPrivate &dd, QGraphicsLayoutItem *parent)
                               : QGraphicsLayout(parent),d_ptr(&dd)
{
    Q_D(HbGridLayout);
    d->q_ptr = this;
    d->init();
}

/*!
    Destructor.
*/
HbGridLayout::~HbGridLayout()
{
    if (d_ptr) {
        for (int i = count() - 1; i >= 0; --i) {
            QGraphicsLayoutItem *item = itemAt(i);
            // The following lines can be removed, but this removes the item
            // from the layout more efficiently than the implementation of 
            // ~QGraphicsLayoutItem.
            d_ptr->removeItem(item, i, false);
            if (item) {
                item->setParentLayoutItem(0);
                if (item->ownedByLayout()) {
                    delete item;
                }
            }
        }
    }

    delete d_ptr;
}

/*!
    adds \a item to gridlayout.
    NOTE::there is no support for adding items at a specific row and column yet.
*/
void HbGridLayout::addItem(QGraphicsLayoutItem *item)
{
    insertItem(-1, item, false);
}
/*!
   Sets size of gridlayout to \a size.
   \a targetCount will be updated with the maximum no of items it can show
   at a time. 
*/
void HbGridLayout::setSize(const QSizeF &size,int &targetCount)
{
    Q_D(HbGridLayout);
    d->setSize(size,targetCount);
}

/*!
    Returns position of layout \a item in the grid layout or -1 if not found.
*/
int HbGridLayout::indexOf(QGraphicsLayoutItem *item) const
{
    const int totalItems = count();
    for(int i = 0; i < totalItems; ++i) {
        if (itemAt(i) == item) {
            return i;
        }
    }
    return -1;
}

/*!
    Removes \a item from the grid layout.
    Equivalent of calling removeAt(indexOf(\aitem)).
*/
void HbGridLayout::removeItem(QGraphicsLayoutItem *item, bool animate)
{
    Q_D(HbGridLayout);
    d->removeItem(item, indexOf(item), animate);
}

/*!
    Sets spacing to \a spacing.
    This sets uniform horizontal and vertical spacing between each item in the layout.
*/
void HbGridLayout::setSpacing(qreal spacing)
{
    Q_D(HbGridLayout);
    if( spacing != d->mSpacing){
         //TODO RECALCULATE ITEM SIZE
        d->mSpacing = spacing;
        if(count()>0){
            d->reset();
        }
    }
}

void HbGridLayout::setMargins(qreal topMargin, qreal bottomMargin, qreal leftMargin, qreal rightMargin)
{
    Q_D(HbGridLayout);
    //TODO CALCULATE ITEM SIZE
    d->setMargins(topMargin, bottomMargin, leftMargin, rightMargin);
    d->reset();

}

/*!
    Returns spacing between items.This is uniform spacing between items
    in both horizontally and vertically directions
*/
qreal HbGridLayout::spacing() const
{
    Q_D(const HbGridLayout);
    return d->mSpacing;
}
/*!
    Inserts a layout at \a item at given position \a index to the grid layout.
    If index is out of range, the item is appended to the end of the list.
*/
void HbGridLayout::insertItem(int index, QGraphicsLayoutItem *item, bool animate)
{
    if (item) {
        Q_D(HbGridLayout);
        d->insertItem(item, index, animate, true);
    }
}

/*!
    \reimp
*/
void HbGridLayout::setRowCount(int rowCount)
{
    Q_D(HbGridLayout);
    if (d->mMaxVisRows!= rowCount){
        //TODO RECALCULATE ITEM SIZE
        d->mMaxVisRows = rowCount;
        d->reset();
    }
}

/*!
    \reimp
*/
void HbGridLayout::setColumnCount(int columnCount)
{
    Q_D(HbGridLayout);
    if (d->mMaxVisColumns != columnCount){
         //TODO RECALCULATE ITEM SIZE
        d->mMaxVisColumns = columnCount;
        d->reset();
    }
}

/*!
    \reimp
*/
int HbGridLayout::count() const
{
  Q_D(const HbGridLayout);
  return d->mItems.count();
}

/*!
   \reimp
*/
QGraphicsLayoutItem *HbGridLayout::itemAt(int i) const
{
    Q_D(const HbGridLayout);
    return d->mItems.value(i);
}

/*!
  \reimp
*/
int HbGridLayout::rowCount() const
{
   Q_D(const HbGridLayout);
   return d->mMaxVisRows;
}

/*!
  \reimp
*/
int HbGridLayout::columnCount() const
{
  Q_D(const HbGridLayout);
  return d->mMaxVisColumns;
}

/*!
    \reimp
*/
void HbGridLayout::removeAt(int index)
{
    Q_D(HbGridLayout);

    QGraphicsLayoutItem *item = itemAt(index);
    if (item){
        d->removeItem(item, index, false);
    }
}

/*!
   \reimp
*/
void HbGridLayout::setGeometry(const QRectF &rect)
{
    Q_D( HbGridLayout );
    if(!d->mClearGeometry) {
           return;
        }
    d->setGeometry(rect);
    QRectF newGeom = rect;
    newGeom.setSize(rect.size().expandedTo(d->mItemSize)
                               .boundedTo(d->mMaxSizeHint));
    if (newGeom == d->mGeometry)
        return;
     d->mGeometry = rect;
    QGraphicsLayout::setGeometry(d->mGeometry);
}


/*!
   \reimp
*/
QSizeF HbGridLayout::sizeHint( Qt::SizeHint which, const QSizeF &constraint ) const
{
    Q_D(const HbGridLayout);
    return d->sizeHint(which,constraint);
}

/*!
   sets the scrollDirection of the layout to \a scrollDirection.
   This determines how items are drawni.e horizontally or vertically.
*/
void HbGridLayout::setScrollDirection(Qt::Orientations scrollDirection)
{
    Q_D(HbGridLayout);
    if(d->mScrollDirection != scrollDirection){
        d->mScrollDirection = scrollDirection;
        d->reset();
    }
}

/*!
   Returns current scrollDirection.
*/
Qt::Orientations HbGridLayout::scrollDirection() const
{
    Q_D(const HbGridLayout);
    return d->mScrollDirection;
}

/*!
   moves \a item to \a targetIndex and sets clearGeometry Flag to true.
   Need to call updateGeometry for updating layout.
*/
void HbGridLayout::moveItem(QGraphicsLayoutItem *item, int targetIndex, bool animate)
{
    if (item) {
        Q_D(HbGridLayout);

        d->removeItem(item, indexOf(item), animate);
        d->insertItem(item, targetIndex, animate, false);
    }
}

/*!
   \reimp
*/
void HbGridLayout::invalidate()
{
    Q_D(HbGridLayout);
    if(!d->mRecycling)
       d->mClearCacheHint = true;
    d->mClearGeometry = true;
    QGraphicsLayout::invalidate();
}

void HbGridLayout::setRecycling(bool enableRecycling)
{
    Q_D(HbGridLayout);
    if(enableRecycling != d->mRecycling){
    invalidate();
    d->mRecycling = enableRecycling;
   }
}

bool HbGridLayout::isRecyclingEnabled()
{
  Q_D(HbGridLayout);
  return  d->mRecycling;
}

void HbGridLayout::recycleItems(QGraphicsLayoutItem *sourceItem, QGraphicsLayoutItem *targetItem)
{
    Q_D(HbGridLayout);

    HbGridLayoutPrivate::AnimationData *sourceData = d->animationData(sourceItem);
    HbGridLayoutPrivate::AnimationData *targetData = d->animationData(targetItem);

    d->stopAnimation(sourceData);

    d->stopAnimation(targetData);
}

bool HbGridLayout::isAnimated(QGraphicsLayoutItem *item) const
{
    Q_D(const HbGridLayout);
    if (d->animationData(item) != 0) {
        return true;
    } else {
        return false;
    }
}

void HbGridLayout::effectFinished(const HbEffect::EffectStatus &status)
{
    Q_D(HbGridLayout);

    QGraphicsLayoutItem *item = static_cast<QGraphicsWidget *>(status.item);
    d->stopAnimation(d->animationData(item));
}

#include "moc_hbgridlayout_p.cpp"

