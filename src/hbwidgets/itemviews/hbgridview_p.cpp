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

#include <hbgridviewitem.h>
#include <hbscrollbar.h>

#include "hbmodeliterator.h"

#include <QDebug>
#include <QGraphicsView>

const QString KDefaultLayoutOption = "default";

HbGridViewPrivate::HbGridViewPrivate() : 
    mIconVisible(true), 
    mTextVisible(true),
    mSwapDimensionsOnOrientationChange(true)
{
}

HbGridViewPrivate::~HbGridViewPrivate()
{
}

void HbGridViewPrivate::init()
{
    Q_Q(HbGridView);
    q->connect(q, SIGNAL(scrollDirectionsChanged(Qt::Orientations)),
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

qreal HbGridViewPrivate::calculateScrollBarPos() const
{
    // calculate thumb size and position
    Q_Q(const HbGridView);

    qreal thumbPos = 0.0;
    int columnCount = getScrollDirectionColumnCount();
    int rowCount = getScrollDirectionRowCount();
    qreal containerPos = getScrollDirectionContainerPos();
    qreal itemSize = getScrollDirectionItemSize();

    // add coulmnCount-1 to indexCount to get remainder included in result (rounding up)
    int modelRowCount = (mModelIterator->indexCount() + columnCount - 1) / columnCount;
    modelRowCount -= rowCount;
    QModelIndex firstItem = mContainer->items().first()->modelIndex();
    qreal firstVisibleRow = (qreal)(q->modelIterator()->indexPosition(firstItem)) / columnCount;
    firstVisibleRow += -containerPos / itemSize; // applying container pos to get trully visible row 
    thumbPos = firstVisibleRow / (qreal)modelRowCount;
    thumbPos = qBound((qreal)0.0, thumbPos, (qreal)1.0);

    return thumbPos;
}

qreal HbGridViewPrivate::calculateScrollBarThumbSize() const
{
    // calculate thumb size and position
    qreal thumbSize = 0.0;
    int columnCount = getScrollDirectionColumnCount();
    int rowCount = getScrollDirectionRowCount();

    // add coulmnCount-1 to indexCount to get remainder included in result (rounding up)
    int modelRowCount = (mModelIterator->indexCount() + columnCount - 1) / columnCount;
    thumbSize = (qreal)rowCount / (qreal)modelRowCount;
    thumbSize = qBound((qreal)0.0, thumbSize, (qreal)1.0);

    return thumbSize;
}

/*!
    Overwrites the default scroll area scrollbar updating algorithm when
    recycling is used. On recycling the scrollbar position & size is calculated
    using rows and their pixel size is not used.
*/
void HbGridViewPrivate::updateScrollBar(Qt::Orientation orientation)
{
    if (!handleScrollBar(orientation)) {
        HbScrollAreaPrivate::updateScrollBar(orientation);
    } else {
        if (mContainer->layout() && !mContainer->layout()->isActivated()) {
            mContainer->layout()->activate();
        }

        qreal thumbPos = calculateScrollBarPos();
        HbScrollBar *scrollBar = getScrollDirectionScrollBar();
        scrollBar->setValue(thumbPos);
    }
}

/*!
    \reimp
*/
void HbGridViewPrivate::setScrollBarMetrics(Qt::Orientation orientation)
{
    if (!handleScrollBar(orientation)) {
        HbScrollAreaPrivate::setScrollBarMetrics(orientation);
    } else {
        if (mContainer->layout() && !mContainer->layout()->isActivated()) {
            mContainer->layout()->activate();
        }

        qreal thumbSize = calculateScrollBarThumbSize();
        HbScrollBar *scrollBar = getScrollDirectionScrollBar();
        scrollBar->setPageSize(thumbSize);
    }
}

/*!
    \reimp
*/
void HbGridViewPrivate::setContentPosition(qreal value, Qt::Orientation orientation, bool animate)
{
    Q_Q( HbGridView );

    if (mContainer->itemRecycling() && mModelIterator->model()) {
        if (mContainer->layout() && !mContainer->layout()->isActivated()) {
            mContainer->layout()->activate();
        }

        int columnCount = getScrollDirectionColumnCount();
        int rowCount = getScrollDirectionRowCount();
        int modelRowCount = (mModelIterator->indexCount() + columnCount - 1) / columnCount;
        modelRowCount -= rowCount;
        qreal thumbPos = calculateScrollBarPos();
        qreal itemSize = getScrollDirectionItemSize();
        qreal diff = (value - thumbPos) * itemSize * modelRowCount;

        if (orientation == Qt::Vertical) {
            q->scrollByAmount(QPointF(0, diff));
        } else {
            q->scrollByAmount(QPointF(diff, 0));
        }
    } else {
        HbScrollAreaPrivate::setContentPosition(value, orientation, animate);
    }
}

QModelIndex HbGridViewPrivate::firstFullyVisibleIndex() const
{
    QModelIndex first, last;
    mContainer->firstAndLastVisibleModelIndex(first, last, true);
    return first;
}
