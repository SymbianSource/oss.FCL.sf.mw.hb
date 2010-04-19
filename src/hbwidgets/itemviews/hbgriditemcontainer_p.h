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
#ifndef HBGRIDITEMCONTAINER_P_H
#define HBGRIDITEMCONTAINER_P_H

#include <QObject>
#include <hbabstractitemcontainer.h>
#include <hbabstractitemview.h>
#include <hbgridlayout_p.h>

class HbGridItemContainerPrivate;

class HB_AUTOTEST_EXPORT HbGridItemContainer : public HbAbstractItemContainer
{
    Q_OBJECT
public:
    HbGridItemContainer(QGraphicsItem *parent = 0);
    ~HbGridItemContainer();

public:    
    
    virtual void addItem(const QModelIndex &index, bool animate = false);
    virtual void removeItem(const QModelIndex &index, bool animate = false);
    virtual void reset();

    virtual void setModelIndexes(const QModelIndex &startIndex = QModelIndex());

    void setRowCount(int rowCount);
    int rowCount() const;

    void setColumnCount(int columnCount);
    int columnCount() const;

    void orientationChanged(Qt::Orientation newOrientation);
    void scrollDirectionChanged(Qt::Orientations);
    HbGridLayout *viewLayout() const;
    virtual int maxItemCount() const;
    void scrollTo(const QModelIndex &index, HbAbstractItemView::ScrollHint hint = HbAbstractItemView::EnsureVisible);
    QModelIndex lastValidItemIndex() const;

protected:
    HbGridItemContainer( HbGridItemContainerPrivate &dd, QGraphicsItem *parent = 0 );
    virtual QPointF recycleItems(const QPointF &delta);
    virtual void viewResized(const QSizeF &size);
    virtual void setItemModelIndex(HbAbstractViewItem *item, const QModelIndex &index);
    virtual void itemAdded(int index, HbAbstractViewItem *item, bool animate = false);
    virtual void itemRemoved(HbAbstractViewItem *item, bool animate = false);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);

    virtual HbAbstractViewItem *createDefaultPrototype() const;

protected slots:

    void animationFinished(const HbEffect::EffectStatus &status);
    void layoutAnimationFinished(QGraphicsLayoutItem *startItem, HbGridLayout::AnimationType animationType);

private:
    QModelIndex getViewIndexInTheCenter() const;

    Q_DECLARE_PRIVATE_D(d_ptr, HbGridItemContainer)
    Q_DISABLE_COPY(HbGridItemContainer)
};

#endif /*HBGRIDITEMCONTAINER_P_H*/
