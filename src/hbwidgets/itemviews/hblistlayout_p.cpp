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

#include "hblistlayout_p.h"
#include "hblistlayout_p_p.h"
#include "hblayoututils_p.h"

#include "hbwidgetbase.h"
#include "hbabstractitemcontainer.h"

#include <QWidget> // for QWIDGETSIZE_MAX
#include <QDebug>

/*
    \class HbListLayout
    \brief HbListLayout manages geometries of list view contents.
*/
namespace
{
static const qreal INVALID_ITEM_HEIGHT = -1.0;
}

HbListLayoutPrivate::HbListLayoutPrivate(HbListLayout *q_ptr) :
    q(q_ptr),
    mSmallestItemHeight(INVALID_ITEM_HEIGHT)
{
}

/*!
    Calculates the smallest item height by taking the minimum of all items preferred heights.
*/
qreal HbListLayoutPrivate::calculateSmallestItemHeight() const
{
    qreal smallestHeight(0);
    if (uniformSizedItems()) {
        QGraphicsLayoutItem *firstItem = mItems.value(0);
        if (firstItem) {
            smallestHeight = firstItem->preferredHeight();  
        } 
    } else {  
        int itemCount = mItems.count();
        if (itemCount > 0) {
            smallestHeight = mItems.first()->preferredHeight();
        }
        for (int i = 1; i < itemCount; ++i) {       
            smallestHeight = qMin(smallestHeight, mItems.at(i)->preferredHeight());
        }
    }
    return smallestHeight;
}

/*!
    Returns the preferred sizeHint.
*/
QSizeF HbListLayoutPrivate::preferredSizeHint()
{
    QSizeF sizeHint(0, 0);
    if (uniformSizedItems()) {
        QGraphicsLayoutItem *firstItem = mItems.value(0);
        if (firstItem) {
            sizeHint = firstItem->effectiveSizeHint(Qt::PreferredSize);  
            sizeHint.rheight() = sizeHint.rheight() * mItems.count();
        } 
    } else {  
        int itemCount = mItems.count();
        for (int i = 0; i < itemCount; ++i) {       
            QSizeF itemSize(0, 0);  
            itemSize = mItems.at(i)->effectiveSizeHint(Qt::PreferredSize);

            if (itemSize.isValid()) {
                sizeHint.rwidth() = qMax(itemSize.width(), sizeHint.width());
                sizeHint.rheight() = sizeHint.height() + itemSize.height();
            }
        }
    }
    return sizeHint;
}

/*!
    Returns the minimum sizeHint.
*/
QSizeF HbListLayoutPrivate::minimumSizeHint()
{
    QSizeF sizeHint(0, 0);
    if (uniformSizedItems()) {
        QGraphicsLayoutItem *firstItem = mItems.value(0);
        if (firstItem) {
            sizeHint = firstItem->effectiveSizeHint(Qt::MinimumSize);
        }
    } else {
        int itemCount = mItems.count();
        for (int i = 0; i < itemCount; ++i) {       
            QSizeF itemSize(0, 0);  
            itemSize = mItems.at(i)->effectiveSizeHint(Qt::MinimumSize);

            if (itemSize.isValid()) {
                sizeHint.rwidth() = qMax(itemSize.width(), sizeHint.width());
                sizeHint.rheight() = qMax(sizeHint.height(), itemSize.height());
            }
        }
    }
    return sizeHint;
}

/*!
    Returns true if uniform items flag is enabled; otherwise return false.
*/
bool HbListLayoutPrivate::uniformSizedItems() const
{
    HbWidgetBase *parentLayoutItem = static_cast<HbWidgetBase*>(q->parentLayoutItem());
    HbAbstractItemContainer *itemContainer = qobject_cast<HbAbstractItemContainer *>(parentLayoutItem);
    if (itemContainer && itemContainer->uniformItemSizes()) {
        return true;
    } else {
        return false;
    }      
}

/*!
    Constructor.
    \param parent parent layout item.
 */
HbListLayout::HbListLayout( QGraphicsLayoutItem *parent )
    : QGraphicsLayout( parent ), d(new HbListLayoutPrivate(this))
{
}

/*!
    Destructor.
    Does not clear the parentLayoutItem of it's sub items.
    If you want to reuse the sub items first call removeAt()
    for each of them.
 */
HbListLayout::~HbListLayout()
{
    if (d) {
         for (int i = count() - 1; i >= 0; --i) {
            QGraphicsLayoutItem *item = itemAt(i);
            // The following lines can be removed, but this removes the item
            // from the layout more efficiently than the implementation of 
            // ~QGraphicsLayoutItem.
            removeAt(i);
            if (item) {
                item->setParentLayoutItem(0);
                if (item->ownedByLayout()) {
                    delete item;
                }
            }
        }
    }
    
    delete d;
}

/*!
    \brief Appends a layout item to the list layout.

    This is convenience function which will internally call the \c insertItem method.

    \param item layout item to be added to list.
 */
void HbListLayout::addItem(QGraphicsLayoutItem *item, bool animate)
{

    insertItem( -1, item, animate );
}

/*!
    \brief  Inserts a layout item to the list layout.

    Inserts the given \a item to given position \a index.
    Does not take ownership.

    If index is out of range, the item is appended to the end of the list.

    \param  index position where to insert the layout.
    \param  item layout item to be inserted to stack.
    \param  animate true if layout should smoothly expand to give room for the new item
 */
void HbListLayout::insertItem(int index, QGraphicsLayoutItem *item, bool animate)
{
    Q_UNUSED(animate);

    index = qMin(index, d->mItems.count());
    if (index < 0) {
        index = d->mItems.count();
    }
    HbLayoutUtils::addChildItem(this, item);

    d->mItems.insert( index, item );
    invalidate();
}

/*!
    \brief Returns position of layout item in the list layout.
    \param item item to look for.
    \return position of layout item, or -1 if not found.
 */
int HbListLayout::indexOf( QGraphicsLayoutItem *item ) const
{
    for ( int i = 0; i < count(); ++i ) {
        if ( itemAt( i ) == item ) {
            return i;
        }
    }
    return -1;
}

/*!
    \brief Removes layout item from the list layout.

    Equivalent of calling removeAt(indexOf(item)).

    \param item item to be removed.
    \param  animate true if layout should smoothly take the room of the item
    \sa removeAt
 */
void HbListLayout::removeItem( QGraphicsLayoutItem *item, bool animate )
{
    if (animate) {
        invalidate();
    } else {
        removeAt(indexOf(item));
    }
}

/*!
    Returns the smallest item height.
*/
qreal HbListLayout::smallestItemHeight() const
{
    if (d->mSmallestItemHeight == INVALID_ITEM_HEIGHT) {
        d->mSmallestItemHeight = d->calculateSmallestItemHeight();
    }
    return d->mSmallestItemHeight;
}

/*!
	\reimp
	
	Invalidates the smallest item cache.
*/
void HbListLayout::invalidate()
{
    QGraphicsLayout::invalidate();

    d->mSmallestItemHeight = INVALID_ITEM_HEIGHT;
}

/*!
    From QGraphicsLayout.
 */
int HbListLayout::count() const
{
    return d->mItems.count();
}

/*!
    From QGraphicsLayout.
 */
QGraphicsLayoutItem *HbListLayout::itemAt(int i) const
{
    return d->mItems.value(i);
}

/*!
    From QGraphicsLayout.
    Clears the parentLayoutItem of removed item.
 */
void HbListLayout::removeAt(int index)
{
    QGraphicsLayoutItem *item = itemAt(index);
    if ( item ) {
        d->mItems.removeAt(index);
        item->setParentLayoutItem(0);
        invalidate();
    }
}

/*!
    From QGraphicsLayout.
 */
void HbListLayout::setGeometry( const QRectF &rect )
{
    QGraphicsLayout::setGeometry(rect);
    QRectF effectiveRect = geometry();

    // TODO: Apply margins?
    //qreal left, top, right, bottom;
    //getContentsMargins(&left, &top, &right, &bottom);
    //effectiveRect.adjust(+left, +top, -right, -bottom);

    qreal y = effectiveRect.y();
    int itemCount = count();
    for ( int i = 0; i < itemCount; i++ ) {
        QGraphicsLayoutItem *item = itemAt( i );
        qreal itemHeight = item->preferredHeight();
        if (item->graphicsItem()->transform().isScaling()) {
            itemHeight *= item->graphicsItem()->transform().m22();
        }

        QRectF itemRect(effectiveRect.x(), y, effectiveRect.width(), itemHeight);
        item->setGeometry(itemRect);
        y += itemHeight;
    }
}

/*!
    From QGraphicsLayout.

    The minimum size hint's width and height are the smallest ones of the
    child layout items.

    The preferred size hint is the sum of the preferred size hints of the
    child layout items..

    The maximum size hint "unlimited", i.e. the child layout items do not
    affect the maximum size hint.
 */
QSizeF HbListLayout::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    QSizeF sizeHint(0, 0);

    if (which == Qt::MaximumSize) {
        // Sub items do not affect the maximum size hint.
        sizeHint = QSizeF(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    } else if (which == Qt::PreferredSize) {        
        sizeHint = d->preferredSizeHint();
    } else if (which == Qt::MinimumSize) {
        sizeHint = d->minimumSizeHint();   
    } else {
        Q_ASSERT_X(false, "HbListLayout::sizeHint()", "Unhandled Qt::SizeHint type");
        return sizeHint;
    }

    // TODO: Apply margins?
    /*qreal left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    sizeHint.rheight() += top + bottom;
    sizeHint.rwidth() += left + right;*/
    
    if (constraint.height() >= 0) {
        sizeHint.setHeight(qMin(sizeHint.height(), constraint.height()));
    }

    if (constraint.width() >= 0) {
        sizeHint.setWidth(qMin(sizeHint.width(), constraint.width()));
    }

    return sizeHint;

}

