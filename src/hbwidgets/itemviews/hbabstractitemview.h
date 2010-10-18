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

#ifndef HBABSTRACTITEMVIEW_H
#define HBABSTRACTITEMVIEW_H

#include <hbscrollarea.h>
#include <hbglobal.h>
#include <hbeffect.h>

#include <QItemSelectionModel>

class HbAbstractItemViewPrivate;
class HbAbstractViewItem;
class HbAbstractViewItemPrivate;
class HbAbstractItemContainer;
class HbModelIterator;

class HB_WIDGETS_EXPORT HbAbstractItemView : public HbScrollArea
{
    Q_OBJECT
    
    Q_ENUMS(SelectionMode ScrollHint IconLoadPolicy)  
    Q_FLAGS(ItemAnimations)

    Q_PROPERTY(bool itemRecycling READ itemRecycling WRITE setItemRecycling)
    Q_PROPERTY(SelectionMode selectionMode READ selectionMode WRITE setSelectionMode)
    Q_PROPERTY(QString layoutName READ layoutName WRITE setLayoutName)
    Q_PROPERTY(bool uniformItemSizes READ uniformItemSizes WRITE setUniformItemSizes)
    Q_PROPERTY(ItemAnimations enabledAnimations READ enabledAnimations WRITE setEnabledAnimations)
    Q_PROPERTY(bool longPressEnabled  READ longPressEnabled  WRITE setLongPressEnabled)
    Q_PROPERTY(IconLoadPolicy iconLoadPolicy READ iconLoadPolicy WRITE setIconLoadPolicy)
    Q_PROPERTY(QString emptyText READ emptyText WRITE setEmptyText)

public:

    enum IconLoadPolicy
    {
        LoadSynchronously,
        LoadAsynchronouslyWhenScrolling,
        LoadAsynchronouslyAlways
    };

    enum SelectionMode
    {
        NoSelection,
        SingleSelection,
        MultiSelection
    };

    enum ScrollHint
    {
        EnsureVisible,
        PositionAtTop,
        PositionAtBottom,
        PositionAtCenter
    };

    enum ItemAnimation
    {
        None               = 0x00000,
        Appear             = 0x00001,
        Disappear          = 0x00002,
        TouchDown          = 0x00004,
        Expand             = 0x00008,
        Collapse           = 0x00010,
        All                = 0xFFFFF
    };

    Q_DECLARE_FLAGS(ItemAnimations, ItemAnimation)

    
public:
    virtual ~HbAbstractItemView();

    virtual void setModel(QAbstractItemModel *model, HbAbstractViewItem *prototype = 0);
    QAbstractItemModel *model() const;
    
    void setItemPrototype(HbAbstractViewItem *prototype);

    void setItemPrototypes(const QList<HbAbstractViewItem *> &prototypes);
    QList<HbAbstractViewItem *> itemPrototypes() const;

    void setSelectionModel(QItemSelectionModel *selectionModel);
    QItemSelectionModel *selectionModel() const;
    
    QModelIndex currentIndex() const;
    
    QModelIndex rootIndex() const;
    
    void setSelectionMode(SelectionMode newMode);
    SelectionMode selectionMode() const;

    void setItemRecycling(bool enabled);
    bool itemRecycling() const;

    using HbScrollArea::isVisible;
    bool isVisible(const QModelIndex &index) const;

    HbAbstractViewItem *currentViewItem() const;

    HbAbstractViewItem *itemByIndex(const QModelIndex &index) const;

    virtual void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible);
    QList<HbAbstractViewItem *> visibleItems() const;

    QString layoutName() const;
    void setLayoutName(const QString &layoutName);

    void setUniformItemSizes(bool enable);
    bool uniformItemSizes() const;

    HbModelIterator *modelIterator() const;

    void setEnabledAnimations(ItemAnimations flags);
    ItemAnimations enabledAnimations() const;

    void setLongPressEnabled(bool enabled);
    bool longPressEnabled() const;

    virtual void setItemPixmapCacheEnabled(bool enabled);
    bool itemPixmapCacheEnabled() const;

    void setIconLoadPolicy(IconLoadPolicy policy);
    IconLoadPolicy iconLoadPolicy() const;

    QString emptyText() const;
    void setEmptyText(const QString &emptyText);

public slots:
    void setCurrentIndex(const QModelIndex &index,
                         QItemSelectionModel::SelectionFlags selectionFlag
                         = QItemSelectionModel::NoUpdate);
    virtual void setRootIndex(const QModelIndex &index);
    virtual void reset();
    virtual void selectAll();
    virtual void clearSelection();

signals:
    void pressed(const QModelIndex &index);
    void released(const QModelIndex &index);
    void activated(const QModelIndex &index);
    void longPressed(HbAbstractViewItem *item, const QPointF &coords);

protected:
    HbAbstractItemView(HbAbstractItemViewPrivate &dd,
                       HbAbstractItemContainer *container,
                       HbModelIterator *modelIterator,
                       QGraphicsItem *parent = 0);

    HbAbstractItemView(HbAbstractItemContainer *container,
                       HbModelIterator *modelIterator,
                       QGraphicsItem *parent = 0);

    virtual QItemSelectionModel::SelectionFlags selectionCommand( 
                                                    const HbAbstractViewItem *item, 
                                                    const QEvent *event);
    bool event(QEvent *e);
    void gestureEvent(QGestureEvent *event);

    virtual void emitActivated(const QModelIndex &modelIndex);
    virtual void emitPressed(const QModelIndex &modelIndex);
    virtual void emitReleased(const QModelIndex &modelIndex);

    virtual bool scrollByAmount(const QPointF &delta);

    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);

protected slots:

    virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

    virtual void orientationChanged(Qt::Orientation newOrientation);
    virtual void orientationAboutToBeChanged();

    virtual void currentIndexChanged(const QModelIndex &current, const QModelIndex &previous);
    virtual void currentSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    virtual void rowsAboutToBeInserted(const QModelIndex &index, int start, int end);
    virtual void rowsInserted(const QModelIndex &parent, int start, int end);

    virtual void rowsAboutToBeRemoved(const QModelIndex &index, int start, int end);
    virtual void rowsRemoved(const QModelIndex &parent, int start, int end);

    virtual void columnsAboutToBeInserted(const QModelIndex &index, int start, int end);
    virtual void columnsInserted(const QModelIndex &parent, int start, int end);
    
    virtual void columnsAboutToBeRemoved(const QModelIndex &index, int start, int end);
    virtual void columnsRemoved(const QModelIndex &parent, int start, int end);

    virtual void itemPressed(const QPointF &pos);
    virtual void itemReleased(const QPointF &pos);
    virtual void itemActivated(const QPointF &pos);
    virtual void itemLongPressed(const QPointF &pos);

    virtual void itemCreated(HbAbstractViewItem *item);

    virtual void modelLayoutChanged();

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbAbstractItemView)
    Q_DISABLE_COPY(HbAbstractItemView)
    Q_PRIVATE_SLOT(d_func(), void _q_modelDestroyed())
    Q_PRIVATE_SLOT(d_func(), void _q_animationEnabled())
    Q_PRIVATE_SLOT(d_func(), void _q_animationFinished(const HbEffect::EffectStatus &status))
    Q_PRIVATE_SLOT(d_func(), void _q_scrolling(QPointF newPosition))
    Q_PRIVATE_SLOT(d_func(), void _q_scrollingEnded())
    Q_PRIVATE_SLOT(d_func(), void _q_scrollingStarted())
    Q_PRIVATE_SLOT(d_func(), void _q_itemAboutToBeDeleted(HbAbstractViewItem *item))


    friend class HbAbstractItemContainer;
    friend class HbAbstractViewItemPrivate;
};

Q_DECLARE_METATYPE(HbAbstractItemView::SelectionMode)

Q_DECLARE_OPERATORS_FOR_FLAGS(HbAbstractItemView::ItemAnimations)

#endif // HBABSTRACTITEMVIEW_H
