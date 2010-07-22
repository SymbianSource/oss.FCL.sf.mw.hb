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

#ifndef HBTREELAYOUT_P_P_H
#define HBTREELAYOUT_P_P_H

#include <QList>

QT_BEGIN_NAMESPACE
class QGraphicsLayoutItem;
QT_END_NAMESPACE

class HbTreeLayout;

/*
    \private
    \class HbTreeLayout
    \brief HbTreeLayout manages geometries of hierarchical tree view contents.
*/

class HbTreeLayoutPrivate
{
public:
    struct TreeItem
    {
        QGraphicsLayoutItem* mItem;
        int mLevel;
    };

    HbTreeLayoutPrivate();
    ~HbTreeLayoutPrivate();

    bool uniformSizedItems() const;
    qreal calculateSmallestItemHeight() const;

    void insertItem(int index, QGraphicsLayoutItem *item, int level, bool animate);

public:
    QList<TreeItem> mItems;
    HbTreeLayout *q_ptr;
    qreal mIndentation;
    mutable qreal mSmallestItemHeight;
    static const qreal INVALID_ITEM_HEIGHT;
};

#endif // HBTREELAYOUT_P_P_H
