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

#ifndef HBABSTRACTITEMCONTAINER_P_H
#define HBABSTRACTITEMCONTAINER_P_H

#include <hbwidget.h>

class HbAbstractItemContainerPrivate;
class QModelIndex;
class HbAbstractViewItem;
class HbAbstractItemView;

class HB_AUTOTEST_EXPORT HbAbstractItemContainer : public HbWidget
{
    Q_OBJECT

public:
    explicit HbAbstractItemContainer(QGraphicsItem *parent = 0);
    virtual ~HbAbstractItemContainer();

    virtual void addItem(const QModelIndex &index, bool animate = false);
    virtual void removeItem(const QModelIndex &index, bool animate = false);
    virtual void removeItem(int pos, bool animate = false);
    void removeItems();

    HbAbstractViewItem* itemByIndex(const QModelIndex &index) const;
    QList<HbAbstractViewItem *> items() const;

    virtual void reset();

    HbAbstractItemView *itemView() const;
    void setItemView(HbAbstractItemView *view);

    virtual void setModelIndexes(const QModelIndex &startIndex = QModelIndex());

    void firstAndLastVisibleModelIndex(
        QModelIndex& firstVisibleModelIndex,
        QModelIndex& lastVisibleModelIndex,
        bool fullyVisible = true) const;

    QList<HbAbstractViewItem *> itemPrototypes() const;
    bool setItemPrototype(HbAbstractViewItem *prototype);
    bool setItemPrototypes(const QList<HbAbstractViewItem *> &prototypes);

    void setItemTransientState(const QModelIndex &index, QHash<QString,QVariant> state);
    void setItemTransientStateValue(const QModelIndex &index, const QString &key, const QVariant &value);

    QMap<int,QVariant> itemState(const QModelIndex &index) const;
    QHash<QString, QVariant> itemTransientState(const QModelIndex &index) const;

    void removeItemTransientStates();

    void setItemRecycling(bool enabled);
    bool itemRecycling() const;

    virtual void setUniformItemSizes(bool enable);
    bool uniformItemSizes() const;

    virtual bool eventFilter(QObject *obj, QEvent *event);

    virtual void resizeContainer();

signals:

    void itemCreated(HbAbstractViewItem *item);
    void itemAboutToBeDeleted(HbAbstractViewItem *item);

protected:

    HbAbstractItemContainer(HbAbstractItemContainerPrivate &dd, QGraphicsItem *parent);

    virtual void itemAdded(int index, HbAbstractViewItem *item, bool animate) = 0;
    virtual void itemRemoved(HbAbstractViewItem *item, bool animate) = 0;

    virtual void viewResized(const QSizeF &size) = 0;

    virtual void setItemModelIndex(HbAbstractViewItem *item, const QModelIndex &index);

    QVariant itemChange(GraphicsItemChange change, const QVariant & value) ;

    virtual QPointF recycleItems(const QPointF &delta);

    void insertItem(int pos, const QModelIndex &index, bool animate = false);

    virtual int maxItemCount() const;

    virtual bool event(QEvent *e);

    virtual HbAbstractViewItem *createDefaultPrototype() const = 0;

private:
    
    Q_DECLARE_PRIVATE_D(d_ptr, HbAbstractItemContainer)
    Q_DISABLE_COPY(HbAbstractItemContainer)

    friend class HbAbstractItemView;
};

#endif 
