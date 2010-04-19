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

#ifndef HBTREEMODELITERATOR_P_P_H
#define HBTREEMODELITERATOR_P_P_H

#include <hbglobal.h>
#include "hbtreemodeliterator_p.h"
#include "hbmodeliterator_p.h"

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
class QModelIndex;
QT_END_NAMESPACE

class HbAbstractItemContainer;
class HbTreeModelIterator;

class HbTreeModelIteratorPrivate : public HbModelIteratorPrivate
{
    Q_DECLARE_PUBLIC(HbTreeModelIterator)

public:
    HbTreeModelIteratorPrivate();
    ~HbTreeModelIteratorPrivate();

    QModelIndex first(const QModelIndex &parent) const;
    QModelIndex last(const QModelIndex &parent) const;
    int childIndexCount(const QModelIndex &index) const;
    void resetCache();
    int searchForward(const QModelIndex &index) const;
    int searchBackward(const QModelIndex &index) const;
    QModelIndexList createParentChainList(const QModelIndex &index) const;
    bool isExpanded(const QModelIndex &index) const;
    bool isExpandedBranch(const QModelIndex &index) const;
    inline bool isInCountCache(const QModelIndex &index) const;
    inline bool isInPositionCache(const QModelIndex &index) const;

    struct CachedIndexCount {
        int count;
        QModelIndex index;
        CachedIndexCount(int c = 0, QModelIndex i = QModelIndex()) {count = c; index = i;};
    };

    mutable struct CachedIndexCount mCachedCount;
    mutable struct CachedIndexCount mCachedPosition;

    HbAbstractItemContainer *mItemContainer;
    int mExpansionKey;
    bool mUseCache;
};

bool HbTreeModelIteratorPrivate::isInCountCache(const QModelIndex &index) const
{
    return (mUseCache
            && index == mCachedCount.index)
            && (mCachedCount.count >= 0);
}

inline bool HbTreeModelIteratorPrivate::isInPositionCache(const QModelIndex &index) const
{
    return (mUseCache
            && index == mCachedPosition.index)
            && (mCachedPosition.count >= 0);
}

#endif // HBMODELITERATOR_P_P_H
