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

#ifndef HBABSTRACTITEMCONTAINERPRIVATE_H
#define HBABSTRACTITEMCONTAINERPRIVATE_H

#include "hbwidget_p.h"
#include "hbabstractitemcontainer.h"

#include <QPersistentModelIndex>

class HbAbstractViewItem;
class HbAbstractItemView;

class HbAbstractItemContainerPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC(HbAbstractItemContainer)

public:

    struct StateItem
    {
        QPersistentModelIndex index;
        QMap<int,QVariant> state;
    };

    HbAbstractItemContainerPrivate();
    virtual ~HbAbstractItemContainerPrivate();

    QRectF itemBoundingRect(const QGraphicsItem *item) const;

    void firstAndLastVisibleBufferIndex(int& firstVisibleBufferIndex,
        int& lastVisibleBufferIndex,
        const QRectF &viewRect,
        bool fullyVisible = true) const;

    bool visible(HbAbstractViewItem* item, const QRectF &viewRect, bool fullyVisible = true) const;

    void deletePrototypes();

    int findStateItem(const QModelIndex &index) const;

    void initPrototype(HbAbstractViewItem *prototype) const;

    HbAbstractViewItem *createItem(const QModelIndex &index);
    HbAbstractViewItem *itemPrototype(const QModelIndex& index) const;

    virtual void updateItemBuffer();
    void increaseBufferSize(int amount);
    void decreaseBufferSize(int amount);

    HbAbstractViewItem* item(const QModelIndex &index) const;

    void doRemoveItem(HbAbstractViewItem *item, const QModelIndex &index, bool animate = false);

    void deleteItem(HbAbstractViewItem *item, bool animate = false);

    virtual bool intoContainerBuffer(const QModelIndex &index) const; 
    virtual int containerBufferIndexForModelIndex(const QModelIndex &index) const;

    mutable QList<HbAbstractViewItem*>  mPrototypes;
    QList<StateItem> mItemStateList;

    QList<HbAbstractViewItem*>  mItems;
    HbAbstractItemView *mItemView;

    int mBufferSize;
    bool mItemRecycling;

    bool mUniformItemSizes;
    QPersistentModelIndex mFirstItemIndex;
    static const int UpdateItemBufferEvent;
};

Q_DECLARE_METATYPE(HbAbstractItemContainerPrivate::StateItem)

#endif /* HBABSTRACTITEMCONTAINERPRIVATE_H */
