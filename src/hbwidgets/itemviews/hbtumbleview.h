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

#ifndef HBTUMBLEVIEW_H
#define HBTUMBLEVIEW_H

#include <hblistview.h>

class HbTumbleViewPrivate;

QT_FORWARD_DECLARE_CLASS(QGraphicsSceneMouseEvent)

class HB_WIDGETS_EXPORT HbTumbleView : public HbListView
{
    Q_OBJECT
    Q_PROPERTY(QStringList items READ items WRITE setItems)
    Q_PROPERTY(int selected READ selected WRITE setSelected)

public:
    explicit HbTumbleView(QGraphicsItem *parent=0);
    explicit HbTumbleView(const QStringList &list, QGraphicsItem *parent=0);
    ~HbTumbleView();     

    void setItems(const QStringList &list);
    QStringList items() const;

    void setSelected(int index);
    int selected() const;

    void setGeometry(const QRectF &rect);

    void setLoopingEnabled(bool enabled);
    bool isLoopingEnabled() const;

    enum { Type = Hb::ItemType_TumbleView };
    int type() const { return Type; }

    QGraphicsItem *primitive(HbStyle::Primitive) const;

public slots:

    void updatePrimitives();

signals:
    void itemSelected(int index);
protected slots:
    void currentIndexChanged(const QModelIndex &current, const QModelIndex &previous);
    void rowsAboutToBeRemoved(const QModelIndex &index, int start, int end);
    void rowsRemoved(const QModelIndex &parent, int start, int end);
    void rowsAboutToBeInserted(const QModelIndex &index, int start, int end);
    void rowsInserted(const QModelIndex &parent, int start, int end);
protected:
    HbTumbleView(HbTumbleViewPrivate &dd, QGraphicsItem *parent = 0);
    void scrollTo(const QModelIndex &index, ScrollHint);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);        
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const;
    QVariant itemChange(GraphicsItemChange change,const QVariant &value);

    bool event(QEvent *e);

private:
    Q_DECLARE_PRIVATE_D(d_ptr,HbTumbleView)
    Q_DISABLE_COPY(HbTumbleView)
    Q_PRIVATE_SLOT(d_func(), void _q_scrollingStarted())
    Q_PRIVATE_SLOT(d_func(), void _q_scrollingEnded())
};

#endif

