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

#ifndef HBTREEVIEW_H
#define HBTREEVIEW_H

#include <hbabstractitemview.h>
#include <hbglobal.h>

class HbTreeViewPrivate;
class HbAbstractItemContainer;
class HbAbstractViewItem;
class HbTreeModelIterator;

QT_BEGIN_NAMESPACE
class QModelIndex;
class QGraphicsSceneMouseEvent;
class QAbstractItemModel;
class QItemSelection;
QT_END_NAMESPACE

class HB_WIDGETS_EXPORT HbTreeView : public HbAbstractItemView
{
    Q_OBJECT
    Q_PROPERTY(qreal indentation READ indentation WRITE setIndentation)

public:

    explicit HbTreeView(QGraphicsItem *parent = 0);
    virtual ~HbTreeView();    

    virtual void setModel(QAbstractItemModel *model, HbAbstractViewItem *prototype = 0);

    virtual void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible);

    void setExpanded(const QModelIndex &index, bool expanded);
    bool isExpanded(const QModelIndex &index) const;

    void setIndentation(qreal indentation);
    qreal indentation() const;

public slots:
    void reset();
    void setRootIndex(const QModelIndex &index);

protected:
    HbTreeView(HbTreeViewPrivate &dd, HbAbstractItemContainer *container, QGraphicsItem *parent);

    virtual void currentSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    virtual void rowsInserted(const QModelIndex &parent, int start, int end);
    
    virtual void rowsAboutToBeRemoved(const QModelIndex &index, int start, int end);
    virtual void rowsRemoved(const QModelIndex &parent, int start, int end);
    
    virtual void columnsInserted(const QModelIndex &parent, int start, int end);
    virtual void columnsRemoved(const QModelIndex &parent, int start, int end);

    virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private:
    Q_DISABLE_COPY(HbTreeView)
    Q_DECLARE_PRIVATE_D(d_ptr, HbTreeView)
};

#endif // HBTREEVIEW_H
