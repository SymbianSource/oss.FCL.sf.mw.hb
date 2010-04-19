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

#include "hblayoututils_p.h"
#include <hbapplication.h>

/*
    \class HbLayoutUtils
    \brief This class constains common utility methods for all layouts.
    \alpha
 */

static bool removeLayoutItemFromLayout(QGraphicsLayout *lay, QGraphicsLayoutItem *layoutItem);
static QGraphicsItem *parentItem(QGraphicsLayoutItem* childItem);

/*!
    Adds a layout item \a child to given layout \a layout.

    \param layout layout to which child is being added.
    \param childItem new child item.
*/
void HbLayoutUtils::addChildItem(QGraphicsLayout *layout, QGraphicsLayoutItem *childItem)
{
    if (QGraphicsLayoutItem *maybeLayout = childItem->parentLayoutItem()) {
        if (maybeLayout->isLayout()) {
            removeLayoutItemFromLayout(static_cast<QGraphicsLayout*>(maybeLayout), childItem);
        }
    }
    childItem->setParentLayoutItem(layout);
    if (childItem->isLayout()) {
        if (QGraphicsItem *parItem = parentItem(layout)) {
            reparentChildWidgets(static_cast<QGraphicsLayout*>(childItem), static_cast<QGraphicsWidget*>(parItem));
        }
    } else {
        if (QGraphicsItem *item = childItem->graphicsItem()) {
            QGraphicsItem *newParent = parentItem(layout);
            QGraphicsItem *oldParent = item->parentItem();
            if (oldParent == newParent || !newParent) {
                return;
            }

            item->setParentItem(newParent);
        }
    }
}

/*!
    Returns the visual direction of a layout. Inherits the direction from parent widget,
    or if no parent widget exists, from the application.

    \param layout
    \return layout direction
 */
Qt::LayoutDirection HbLayoutUtils::visualDirection(QGraphicsLayout *layout)
{
    QGraphicsWidget *widget = parentWidget(layout);
    if (widget) {
        return widget->layoutDirection();
    }
    return HbApplication::layoutDirection();
}

/*!
    Converts given rect to visual rect.
    Mirrors the rect if the given layout direction is right-to-left.
    Does nothing if the given layout direction is left-to-right.

    \param dir
    \param rect
    \param boundingRect
 */
void HbLayoutUtils::visualRect( Qt::LayoutDirection dir, QRectF &rect, const QRectF &boundingRect )
{
    if (dir == Qt::RightToLeft) {
        rect.moveLeft( 2 * boundingRect.x() + boundingRect.width() - rect.x() - rect.width() );
    }
}


/*
    \internal

    Returns ancestor parent item that's not layout.
*/
static QGraphicsItem *parentItem(QGraphicsLayoutItem* childItem)
{
    const QGraphicsLayoutItem *parent = childItem;
    while (parent && parent->isLayout()) {
        parent = parent->parentLayoutItem();
    }
    return parent ? parent->graphicsItem() : 0;
}

/*
    \internal

    Removes layout item from specified layout.
*/
static bool removeLayoutItemFromLayout(QGraphicsLayout *lay, QGraphicsLayoutItem *layoutItem)
{
    if (!lay)
        return false;

    const int itemCount = lay->count();
    for (int i = 0; i < itemCount; ++i) {
        QGraphicsLayoutItem *child = lay->itemAt(i);
        if (child) {
            if (child->isLayout()) {
                if (removeLayoutItemFromLayout(static_cast<QGraphicsLayout*>(child), layoutItem)) {
                    return true;
                }
            } else if (child == layoutItem) {
                lay->removeAt(i);
                return true;
            }
        }
    }
    return false;
}

/*!
    \internal

    Sets \a widget as the parent item of all the child widgets of \a layout
    and and all the child widgets of its sub-layouts, recursively.
*/
void HbLayoutUtils::reparentChildWidgets(QGraphicsLayout *layout, QGraphicsWidget *widget)
{
    int n = layout->count();
    for (int i = 0; i < n; ++i) {
        QGraphicsLayoutItem *item = layout->itemAt(i);
        if (item->isLayout()) {
            QGraphicsLayout *l = static_cast<QGraphicsLayout*>(item);
            reparentChildWidgets(l, widget);
        } else {
            QGraphicsWidget *w = static_cast<QGraphicsWidget*>(item);
            QGraphicsWidget *pw = w->parentWidget();
            if (pw != widget) {
                w->setParentItem(widget);
            }
        }
    }
}

/*!
    \internal

    Returns the parent widget of given layout, or 0 if the layout is
    not installed on any widget.

    If the layout is a sub-layout, this function returns the parent
    widget of its parent layout.

    \param layout layout of which parent widget is requested.
    \return parent widget or 0 if there is not such assigned.
*/
QGraphicsWidget *HbLayoutUtils::parentWidget(QGraphicsLayout *layout)
{
    const QGraphicsLayoutItem *parent = layout;
    while (parent && parent->isLayout()) {
        parent = parent->parentLayoutItem();
    }
    return static_cast<QGraphicsWidget *>(const_cast<QGraphicsLayoutItem *>(parent));
}

