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

#ifndef HBTREEVIEWITEMPRIVATE_H
#define HBTREEVIEWITEMPRIVATE_H

#include "hblistviewitem_p.h"
#include "hbtreeviewitem.h"

class QGraphicsItem;

class HbTreeViewItemShared : public HbListViewItemShared
{
    public:

        HbTreeViewItemShared() :
            HbListViewItemShared(),
            mUserExpandable(true)
        {
        }

        bool mUserExpandable;
};


class HbTreeViewItemPrivate : public HbListViewItemPrivate
{
    Q_DECLARE_PUBLIC(HbTreeViewItem)

public:

    HbTreeViewItemPrivate(HbTreeViewItem *prototype);
    
    HbTreeViewItemPrivate(const HbTreeViewItemPrivate &source);
    
    HbTreeViewItemPrivate &operator=(const HbTreeViewItemPrivate &source);

    virtual ~HbTreeViewItemPrivate();

    void init();

    virtual int modelItemType() const;

    void updateExpandItem();

    virtual void tapTriggered(QGestureEvent *event);
    QGraphicsItem *mExpandItem;

public:

    bool mExpanded;
};

#endif // HBTREEVIEWITEMPRIVATE_H
