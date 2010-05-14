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
#ifndef HBGRIDVIEW_H
#define HBGRIDVIEW_H

#include <hbabstractitemview.h>

class HbGridViewPrivate;

class HB_WIDGETS_EXPORT HbGridView: public HbAbstractItemView
{
    Q_OBJECT

    Q_PROPERTY(int columnCount READ columnCount WRITE setColumnCount)
    Q_PROPERTY(int rowCount READ rowCount WRITE setRowCount)
    Q_PROPERTY(bool iconVisible READ iconVisible WRITE setIconVisible)
    Q_PROPERTY(bool textVisible READ textVisible WRITE setTextVisible)

public:
    explicit HbGridView(QGraphicsItem *parent = 0);
    virtual ~HbGridView();
    HbAbstractViewItem *itemAt(int row, int column) const;
    void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible);

    void setRowCount(int rowCount);
    int rowCount() const;

    void setColumnCount(int coloumnCount);
    int columnCount() const;

    bool iconVisible() const;
    void setIconVisible(bool visible);
    bool textVisible() const;
    void setTextVisible(bool visible);

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void orientationAboutToBeChanged();
    void orientationChanged(Qt::Orientation newOrientation);

    void rowsInserted(const QModelIndex &parent, int start, int end);
    void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    void rowsRemoved(const QModelIndex &parent, int start, int end);
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

protected slots:
    void scrollDirectionChanged(Qt::Orientations scrollDirection);

protected:
    HbGridView(HbGridViewPrivate &dd, HbAbstractItemContainer *container, QGraphicsItem *parent = 0);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbGridView)
    Q_DISABLE_COPY(HbGridView)
};

#endif /*HbGridView_H*/
