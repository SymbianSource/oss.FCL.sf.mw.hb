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

#ifndef HBTREEVIEW_PRIVATE_H
#define HBTREEVIEW_PRIVATE_H

#include "hbtreeview.h"
#include "hbabstractitemview_p.h"
#include "hbtreemodeliterator_p.h"

QT_BEGIN_NAMESPACE
class QModelIndex;
class QAbstractItemModel;
class QItemSelectionModel;
QT_END_NAMESPACE

class HbTreeViewPrivate : public HbAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(HbTreeView)

public:

    HbTreeViewPrivate();
    virtual ~HbTreeViewPrivate();

    void init();

    bool isParentValid(const QModelIndex &parent) const;

    bool isChild(const QModelIndex &index, const QModelIndex &parent) const;

    QModelIndex searchIndexUp(const QModelIndex &index,
                                int  maxStepCount) const;

    bool handleScrollBar(Qt::Orientation orientation);

    inline HbTreeModelIterator *treeModelIterator();

    virtual bool animationEnabled(bool insertOperation);

    bool mSelectionStarted;
    QList<int> mItemsToBeDeleted;

    bool mInSetExpanded;
};

HbTreeModelIterator *HbTreeViewPrivate::treeModelIterator()
{
    return static_cast<HbTreeModelIterator *>(mModelIterator);
}

#endif // HBTREEVIEW_PRIVATE_H
