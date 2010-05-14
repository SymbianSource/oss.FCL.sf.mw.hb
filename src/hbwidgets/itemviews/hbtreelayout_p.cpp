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

#include "hbtreelayout_p.h"
#include "hbtreelayout_p_p.h"
#include "hblayoututils_p.h"

#include "hbabstractitemcontainer_p.h"
#include <hbapplication.h>
#include <QDebug>

/*!
    Constructor.
    \param parent parent layout item.
 */
HbTreeLayout::HbTreeLayout(QGraphicsLayoutItem *parent)
    : QGraphicsLayout(parent), d(new HbTreeLayoutPrivate())
{
    d->q_ptr = this;
}

/*!
    Destructor.
    Does not clear the parentLayoutItem of it's sub items.
    If you want to reuse the sub items first call removeAt()
    for each of them.
 */
HbTreeLayout::~HbTreeLayout()
{
    delete d;
}

/*!
    \brief Appends a layout item to the list layout.

    This is convenience function which will internally call the \c insertItem method.

    \param item layout item to be added to list.
 */
void HbTreeLayout::addItem(QGraphicsLayoutItem *item, int level, bool animate)
{
    d->insertItem(-1, item, level, animate);
}

/*!
    \brief  Inserts a layout item to the list layout.

    Inserts the given \a item to given position \a index.
    Does not take ownership.

    If index is out of range, the item is appended to the end of the list.

    \param  index position where to insert the layout.
    \param  item layout item to be inserted to stack.
 */
void HbTreeLayout::insertItem(int index, QGraphicsLayoutItem *item, int level, bool animate)
{
    d->insertItem(index, item, level, animate);
}

/*!
    \brief Returns position of layout item in the list layout.
    \param item item to look for.
    \return position of layout item, or -1 if not found.
 */
int HbTreeLayout::indexOf(QGraphicsLayoutItem *item) const
{
    for (int i = 0; i < count(); ++i) {
        if (itemAt( i ) == item) {
            return i;
        }
    }
    return -1;
}

/*!
    \brief Removes layout item from the list layout.

    Equivalent of calling removeAt(indexOf(item)).

    \param item item to be removed.
    \sa removeAt
 */
void HbTreeLayout::removeItem(QGraphicsLayoutItem *item, bool animate)
{
    if (animate) {
        invalidate();
    } else {
        removeAt(indexOf(item));
    }
}

/*!
    From QGraphicsLayout.
 */
int HbTreeLayout::count() const
{
    return d->mItems.count();
}

/*!
    From QGraphicsLayout.
 */
QGraphicsLayoutItem *HbTreeLayout::itemAt(int i) const
{
    return d->mItems.value(i).mItem;
}

/*!
    From QGraphicsLayout.
    Clears the parentLayoutItem of removed item.
 */
void HbTreeLayout::removeAt(int index)
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
void HbTreeLayout::setGeometry( const QRectF &rect )
{
    QGraphicsLayout::setGeometry(rect);
    QRectF effectiveRect = geometry();
    // TODO: Apply margins?
    //qreal left, top, right, bottom;
    //getContentsMargins(&left, &top, &right, &bottom);
    //effectiveRect.adjust(+left, +top, -right, -bottom);

    bool mirrorLayout = false;
    if (HbApplication::layoutDirection() == Qt::RightToLeft) {
        mirrorLayout = true;
    }

    qreal y = effectiveRect.y();
    int itemCount = count();
    for (int i = 0; i < itemCount; ++i) {
        HbTreeLayoutPrivate::TreeItem listItem = d->mItems.at(i);
        QGraphicsLayoutItem *item = listItem.mItem;

        qreal itemHeight = item->preferredHeight();
        qreal itemWidth = item->preferredWidth();

        qreal viewWidth = minimumWidth();
        if (viewWidth > 0.0) {
            if (d->mIndentation * listItem.mLevel + itemWidth < viewWidth) {
                itemWidth = viewWidth - (d->mIndentation * listItem.mLevel);
            } else if (itemWidth > viewWidth) {
                itemWidth = viewWidth;
            }
        }

        qreal x;
        if (mirrorLayout) {
            x = effectiveRect.right() - d->mIndentation * listItem.mLevel - itemWidth;
        } else {
            x = d->mIndentation * listItem.mLevel + effectiveRect.left();
        }

        if (item->graphicsItem()->transform().isScaling()) {
            itemHeight *= item->graphicsItem()->transform().m22();
        }

        item->setGeometry( QRectF(x, y, itemWidth, itemHeight ) );
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
QSizeF HbTreeLayout::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    QSizeF sizeHint(0, 0);

    if (which == Qt::MaximumSize) {
        // Sub items do not affect the maximum size hint.
        sizeHint = QSizeF(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    } else if (which == Qt::PreferredSize) {
        QGraphicsWidget *view = 0;
        if (    parentLayoutItem()
            &&  !parentLayoutItem()->isLayout()
            &&  parentLayoutItem()->graphicsItem()) {
            view = parentLayoutItem()->graphicsItem()->parentWidget();
        }
        qreal viewWidth = view ? view->size().width() : 0.0;

        int itemCount = count();
        for (int i = 0; i < itemCount; ++i) {
            QSizeF itemSize(0, 0);
            HbTreeLayoutPrivate::TreeItem listItem = d->mItems.at(i);

            itemSize = listItem.mItem->effectiveSizeHint(which);
            if (viewWidth > 0.0) {
                if (d->mIndentation * listItem.mLevel + itemSize.width() < viewWidth) {
                    // if indentation + item width fits into view, expand item so that
                    // indentation + item width == view width
                    itemSize.setWidth(viewWidth - (d->mIndentation * listItem.mLevel));
                } else if (itemSize.width() > viewWidth) {
                    itemSize.setWidth(viewWidth);
                }
            }

            if (itemSize.isValid()) {
                sizeHint.rwidth() = qMax((d->mIndentation * listItem.mLevel + itemSize.width()), sizeHint.width());
                sizeHint.rheight() = sizeHint.height() + itemSize.height();
            }
        }
    } else if (which == Qt::MinimumSize) {
        if(itemAt(0)) {
            sizeHint = itemAt(0)->effectiveSizeHint(Qt::PreferredSize);
        }
        if (d->uniformSizedItems()) {
            return sizeHint; //No need to goto loop below to check the sizes of every item
        }
        
        int itemCount = count();
        for( int i = 1; i < itemCount; ++i ) {
            QSizeF itemSize(0, 0);
            HbTreeLayoutPrivate::TreeItem listItem = d->mItems.at(i);

            itemSize = listItem.mItem->effectiveSizeHint(Qt::PreferredSize);

            if ( itemSize.isValid() ) {
                sizeHint.rwidth() = qMin( itemSize.width(), sizeHint.width() );
                sizeHint.rheight() = qMin( sizeHint.height(), itemSize.height() );
            }
        }
    } else {
        Q_ASSERT_X(false, "HbTreeLayout::sizeHint()", "Unhandled Qt::SizeHint type");
        return sizeHint;
    }

    if (constraint.height() >= 0) {
        sizeHint.setHeight(qMin(sizeHint.height(), constraint.height()));
    }

    if (constraint.width() >= 0) {
        sizeHint.setWidth(qMin(sizeHint.width(), constraint.width()));
    }

    return sizeHint;

}

void HbTreeLayout::setItemLevel(QGraphicsLayoutItem *item, int level)
{
    for (int current = 0; current < d->mItems.count(); ++current) {
        if (d->mItems.at(current).mItem == item) {
            d->mItems[current].mLevel = level;
            invalidate();
            break;
        }
    }
}

qreal HbTreeLayout::indentation() const
{
    return d->mIndentation;
}

void HbTreeLayout::setIndentation(qreal indentation)
{
    if (indentation != d->mIndentation) {
        d->mIndentation = indentation;
        invalidate();
    }
}

/*!
    Returns the smallest item height.
*/
qreal HbTreeLayout::smallestItemHeight() const
{
    if (d->mSmallestItemHeight == d->INVALID_ITEM_HEIGHT) {
        d->mSmallestItemHeight = d->calculateSmallestItemHeight();
    }
    return d->mSmallestItemHeight;
}

/*!
	\reimp
	
	Invalidates smallest item height cache.
*/
void HbTreeLayout::invalidate()
{
    QGraphicsLayout::invalidate();

    d->mSmallestItemHeight = d->INVALID_ITEM_HEIGHT;
}


