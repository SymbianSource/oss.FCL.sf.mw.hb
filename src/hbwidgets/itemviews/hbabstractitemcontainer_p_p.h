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

#ifndef HBABSTRACTITEMCONTAINER_P_P_H
#define HBABSTRACTITEMCONTAINER_P_P_H

#include "hbwidget_p.h"
#include "hbabstractitemcontainer_p.h"
#include "hbabstractviewitem.h"
#include "hbabstractitemview.h"
#include "hbmodeliterator.h"
#include "hbabstractitemview_p.h"

#include <QPersistentModelIndex>

class HbAbstractViewItem;
class HbAbstractItemView;

#include <QHash>

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

    void init();

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

    virtual HbAbstractViewItem* item(const QModelIndex &index) const;

    void doRemoveItem(HbAbstractViewItem *item, const QModelIndex &index, bool animate = false);

    void deleteItem(HbAbstractViewItem *item, bool animate = false);

    virtual bool intoContainerBuffer(const QModelIndex &index) const; 
    virtual int containerBufferIndexForModelIndex(const QModelIndex &index) const;

    virtual qreal getDiffWithoutScrollareaCompensation(const QPointF &delta) const;

    void restoreItemPosition(HbAbstractViewItem *item, const QPointF &position);

    void insertItem(HbAbstractViewItem *item, int pos, const QModelIndex &index, bool animate);
	
    inline HbModelIterator *modelIterator() const;

    inline void adjustContent() const;

    mutable QList<HbAbstractViewItem*>  mPrototypes;
    QList<StateItem> mItemStateList;
    QHash<QPersistentModelIndex, QHash<QString, QVariant> > mItemStates;

    QList<HbAbstractViewItem*>  mItems;
    HbAbstractItemView *mItemView;

    int mBufferSize;
    bool mItemRecycling;

    bool mUniformItemSizes;
    QPersistentModelIndex mFirstItemIndex;
    static const int UpdateItemBufferEvent;
};

HbModelIterator *HbAbstractItemContainerPrivate::modelIterator() const
{
    if (mItemView) {
        return mItemView->modelIterator();
    } else {
        return 0;
    }
}

void HbAbstractItemContainerPrivate::adjustContent() const
{
    if (mItemView) {
        // this will force the HbScrollArea to adjust the content correctly. Adjustment
        // is not done in the setPos generated event handling by default to speed up scrolling.
        HbAbstractItemViewPrivate::d_ptr(mItemView)->adjustContent();
    }
}

Q_DECLARE_METATYPE(HbAbstractItemContainerPrivate::StateItem)

#endif /* HBABSTRACTITEMCONTAINERPRIVATE_H */
