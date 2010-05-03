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
#include "hbabstractitemcontainer.h"

#include <hblayoututils_p.h>
#include <hbabstractviewitem.h>
#include <hbtreeview.h>
#include <hbmodeliterator.h>

#include <QWidget> // for QWIDGETSIZE_MAX
#include <QDebug>

const qreal HbTreeLayoutPrivate::INVALID_ITEM_HEIGHT = -1.0;

HbTreeLayoutPrivate::HbTreeLayoutPrivate()
    : q_ptr(0),
    mIndentation(15.0),
    mSmallestItemHeight(INVALID_ITEM_HEIGHT)
{
}

HbTreeLayoutPrivate::~HbTreeLayoutPrivate()
{
}

bool HbTreeLayoutPrivate::uniformSizedItems() const
{
    if (q_ptr->parentLayoutItem() && (static_cast<HbAbstractItemContainer *>(q_ptr->parentLayoutItem()))->uniformItemSizes() ) {
        return true;
    } else {
        return false;
    }
}

/*!
    Calculates the smallest item height from all items.
    Calculations don't consider if there is ongoing animation for items
*/
qreal HbTreeLayoutPrivate::calculateSmallestItemHeight() const
{
    qreal smallestHeight(0);
    if (uniformSizedItems()) {
        QGraphicsLayoutItem *firstItem = mItems.value(0).mItem;
        if (firstItem) {
            smallestHeight = firstItem->preferredHeight();
        }
    } else {
        int itemCount = mItems.count();
        if (itemCount > 0) {
            smallestHeight = mItems.first().mItem->preferredHeight();
        }
        for (int i = 1; i < itemCount; ++i) {
            smallestHeight = qMin(smallestHeight, mItems.at(i).mItem->preferredHeight());
        }
    }
    return smallestHeight;
}

void HbTreeLayoutPrivate::insertItem(int index, QGraphicsLayoutItem *item, int level, bool animate)
{
    Q_UNUSED(animate);

    index = qMin(index, mItems.count());
    if (index < 0) {
        index = mItems.count();
    }
    HbLayoutUtils::addChildItem(q_ptr, item);

    HbTreeLayoutPrivate::TreeItem listItem;
    listItem.mItem = item;
    listItem.mLevel = level;

    mItems.insert(index, listItem);
    q_ptr->invalidate();
}

/*
// For debugging purposes
void HbTreeLayoutPrivate::checkItemListIntegrity()
{
    Q_Q(HbTreeLayout);

    if (!mItems.isEmpty()) {
        HbModelIterator *iterator = qobject_cast<HbTreeView *>(static_cast<QGraphicsWidget *>(q->parentLayoutItem()->graphicsItem())->parentWidget())->modelIterator();

        int previousItemPosition = -1;
        for (int i = 0; i < mItems.count(); ++i) {
            HbAbstractViewItem * item = qobject_cast<HbAbstractViewItem *>(static_cast<QGraphicsWidget *>(mItems.at(i).mItem->graphicsItem()));
            QModelIndex index = item->modelIndex();
            if (index.isValid()) {
                int currentItemPosition = iterator->indexPosition(index);
                if (previousItemPosition != -1 && currentItemPosition != -1) {
                    Q_ASSERT((currentItemPosition == (previousItemPosition + 1)));
                    qDebug() << i << " item model position: " << currentItemPosition;
                }

                previousItemPosition = currentItemPosition;
            }
        }
    }
}
*/

