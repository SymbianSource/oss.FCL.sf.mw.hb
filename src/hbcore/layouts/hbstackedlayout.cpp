/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbCore module of the UI Extensions for Mobile.
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

#include "hbstackedlayout.h"
#include "hbapplication.h"
#include "hbinstance.h"
#include "hblayoututils_p.h"

#include <QList>
#include <QGraphicsLayoutItem>
#include <QGraphicsWidget>
#include <QWidget> // for QWIDGETSIZE_MAX

/*!
    @stable
    @hbcore
    \class HbStackedLayout
    \brief HbStackedLayout manages geometries of stacked layout contents.

    HbStackedLayout is a simple stack layout manager. It will only set position
    and size for the contained child widget.

    Example code:
    \snippet{stackedlayoutsample.cpp,1}

    \sa HbStackedWidget
 */

class HbStackedLayoutPrivate
{
public:
    HbStackedLayoutPrivate();
    QList<QGraphicsLayoutItem *> mList;
};

/*!
    \internal
 */
HbStackedLayoutPrivate::HbStackedLayoutPrivate()
{
}

/*!
    Constructor.
    \param parent parent layout item.
 */
HbStackedLayout::HbStackedLayout(QGraphicsLayoutItem *parent)
    : QGraphicsLayout(parent), d(new HbStackedLayoutPrivate())
{
}

/*!
    Destructor.
    Does not clear the parentLayoutItem of it's sub items.
    If you want to reuse the sub items first call removeAt()
    for each of them.
 */
HbStackedLayout::~HbStackedLayout()
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
    \brief Appends a layout item to the stacked layout.

    This is convenience function which will internally call the \c insertItem method.

    \param item layout item to be added to stack.
    \return index to which layout item was added.
 */
int HbStackedLayout::addItem(QGraphicsLayoutItem *item)
{
    return insertItem(-1, item);
}

/*!
    \brief  Inserts a layout item to the stacked layout.

    Inserts the given \a item to given position \a index.

    Also reparents \a item to the parent item of this layout, if there
    is one.

    If index is out of range, the item is appended (in which case it
    is the actual index of the item that is returned).

    \param  index position where to insert the layout.
    \param  item layout item to be inserted to stack.
    \return index to which layout item was added.
 */
int HbStackedLayout::insertItem(int index, QGraphicsLayoutItem *item)
{
    if (!item) {
        qWarning("HbStackedLayout::insertItem: item is NULL");
        return -1;
    }

    if (item->parentLayoutItem()) {
        qWarning("HbStackedLayout::insertItem: layout already has a parent");
        return -1;
    }

    HbLayoutUtils::addChildItem(this, item);

    index = qMin(index, d->mList.count());
    if (index < 0) {
        index = d->mList.count();
    }
    d->mList.insert(index, item);

    invalidate();
    return index;
}

/*!
    \brief Removes layout item from the stacked layout.

    This method will internally call \c removeAt.

    \param item item to be removed.
    \sa removeAt
 */
void HbStackedLayout::removeItem(QGraphicsLayoutItem *item)
{
    removeAt(indexOf(item));
}

/*!
    \brief Returns position of layout item in the stacked layout.
    \return position of layout item, or -1 if not found.
 */
int HbStackedLayout::indexOf(QGraphicsLayoutItem *item) const
{
    int c = d->mList.count();
    for (int i = 0; i < c; i++) {
        if (d->mList.at(i) == item) {
            return i;
        }
    }
    return -1;
}

/*!
    From QGraphicsLayout.
 */
int HbStackedLayout::count() const
{
    return d->mList.size();
}

/*!
    From QGraphicsLayout.
 */
QGraphicsLayoutItem *HbStackedLayout::itemAt(int index) const
{
    return d->mList.value(index);
}

/*!
    From QGraphicsLayout.
    Clears the parentLayoutItem of removed item.
 */
void HbStackedLayout::removeAt(int index)
{
    if (index < 0 || index >= d->mList.size()) {
        return;
    }
    itemAt(index)->setParentLayoutItem(0);
    d->mList.removeAt(index);
    invalidate();
}

/*!
    From QGraphicsLayout.
 */
void HbStackedLayout::invalidate()
{
    QGraphicsLayout::invalidate();
}

/*!
    From QGraphicsLayout.

    The minimum size hint is the maximum of the minimum size hints of the
    child layout items plus content margins.

    The preferred size hint is the maximum of the preferred size hints of the
    child layout items plus content margins.

    The maximum size hint "unlimited", i.e. the child layout items do not
    affect the maximum size hint.
 */
QSizeF HbStackedLayout::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    if (which == Qt::MaximumSize) {
        // Sub items do not affect the maximum size hint.
        return QSizeF(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    }
    QSizeF sizeHint(0, 0);
    int c = d->mList.count();
    for (int i = 0; i < c; ++i) {
        QGraphicsLayoutItem *item = d->mList.at(i);
        QSizeF itemSize(item->effectiveSizeHint(which, constraint));
        if (item->sizePolicy().horizontalPolicy() == QSizePolicy::Ignored) {
            itemSize.setWidth(0);
        }
        if (item->sizePolicy().verticalPolicy() == QSizePolicy::Ignored) {
            itemSize.setHeight(0);
        }
        if (itemSize.isValid()) {
            sizeHint = sizeHint.expandedTo(itemSize);
        }
    }
    // Apply margins
    qreal left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    sizeHint.rheight() += top + bottom;
    sizeHint.rwidth() += left + right;
    return sizeHint;
}

/*!
    From QGraphicsLayout.
 */
void HbStackedLayout::setGeometry(const QRectF &rect)
{
    QGraphicsLayout::setGeometry(rect);

    int c = d->mList.count();
    for (int i = 0; i < c; ++i) {
        QGraphicsLayoutItem *item = d->mList.at(i);
        QSizeF itemMax = item->effectiveSizeHint(Qt::MaximumSize);
        QSizeF itemMin = item->effectiveSizeHint(Qt::MinimumSize);
        QRectF itemRect(rect);
        // Have to check min and max "manually" in order
        // to make mirroring work.
        if (itemMax.width() < itemRect.width()) {
            itemRect.setWidth(itemMax.width());
        }
        if (itemMax.height() < itemRect.height()) {
            itemRect.setHeight(itemMax.height());
        }
        if (itemMin.width() > itemRect.width()) {
            itemRect.setWidth(itemMin.width());
        }
        if (itemMin.height() > itemRect.height()) {
            itemRect.setHeight(itemMin.height());
        }
        alignRect(itemRect, rect);
        item->setGeometry(itemRect);
    }
}

/*!
    \internal
 */
void HbStackedLayout::alignRect(QRectF &rect, const QRectF &boundingRect)
{
    if (HbApplication::layoutDirection() == Qt::RightToLeft) {
        rect.moveLeft(2 * boundingRect.x() + boundingRect.width() - rect.x() - rect.width());
    }
}
