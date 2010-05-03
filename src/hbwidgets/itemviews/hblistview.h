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

#ifndef HBLISTVIEW_H
#define HBLISTVIEW_H

#include <hbabstractitemview.h>
#include <hbglobal.h>

class HbListViewPrivate;
class HbListViewItem;

class HB_WIDGETS_EXPORT HbListView : public HbAbstractItemView
{
    Q_OBJECT

    Q_PROPERTY(bool arrangeMode READ arrangeMode WRITE setArrangeMode)
public:

    explicit HbListView(QGraphicsItem *parent = 0);
    virtual ~HbListView();    

    enum { Type = Hb::ItemType_ListView };
    int type() const;

    HbListViewItem *listItemPrototype() const;

    bool arrangeMode() const;
    virtual bool setArrangeMode(bool arrangeMode);

    HbAbstractViewItem *draggedItem() const;

    HbAbstractViewItem *viewItem(int row) const;

    virtual void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible);

protected:
    HbListView(HbListViewPrivate &dd, HbAbstractItemContainer *container, QGraphicsItem * parent);

    void rowsInserted(const QModelIndex &parent, int start, int end);
    void rowsAboutToBeRemoved(const QModelIndex &parent,int start,int end);
    void rowsRemoved(const QModelIndex &parent,int start,int end);
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void orientationChanged(Qt::Orientation newOrientation);

    virtual void move(const QModelIndex &from, const QModelIndex &to);

protected slots:
    void scrolling(QPointF newPosition);
private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbListView)
    Q_DISABLE_COPY(HbListView)
};

#endif // HBLISTVIEW_H
