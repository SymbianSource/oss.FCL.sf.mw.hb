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

#include "hbtreemodeliterator_p_p.h"
#include "hbabstractitemcontainer_p.h"
#include <QAbstractItemModel>

HbTreeModelIteratorPrivate::HbTreeModelIteratorPrivate()
    : HbModelIteratorPrivate(),
    mCachedCount(CachedIndexCount()), mCachedPosition(CachedIndexCount()),
    mItemContainer(0), mExpansionKey(0), mUseCache(true)
{
}

HbTreeModelIteratorPrivate::~HbTreeModelIteratorPrivate()
{
}

/*
    Return first index belonging to specified parent
    If parent is colapsed QModelIndex is returned
*/
QModelIndex HbTreeModelIteratorPrivate::first(const QModelIndex &parent) const
{
    if (mModel) {
        if (!parent.isValid()
            || isExpanded(parent)) {
            return mModel->index(0, 0, parent);
        }
    }
    return QModelIndex();
}

/*
    Return last index belonging to specified parent - last child in whole parent branch
*/
QModelIndex HbTreeModelIteratorPrivate::last(const QModelIndex &parent) const
{
    // last(mRootIndex) when mRootIndex is not expanded return invalid index
    if (mModel) {
        if (!parent.isValid()
            || isExpanded(parent)) {
            // parent can be invalid, mModel->index instead of index.child
            QModelIndex index = mModel->index(mModel->rowCount(parent)-1, 0, parent);
            if (index.isValid()) {
                while (isExpanded(index)) {
                    QModelIndex lastChild = mModel->index(mModel->rowCount(index)-1, 0, index);
                    if (!lastChild.isValid()) break;
                    index = lastChild;
                }
            }
            return index;
        }
    }
    return QModelIndex();
}

int HbTreeModelIteratorPrivate::childIndexCount(const QModelIndex &index) const
{
    int itemCount = 0;
    int rowCount = 0;
    if (isExpanded(index)) {
        rowCount = mModel->rowCount(index);
        itemCount += rowCount;
        for (int row = 0; row < rowCount; ++row) {
            itemCount += childIndexCount(mModel->index(row, 0, index));
        }
    }

    return itemCount;
}

void HbTreeModelIteratorPrivate::resetCache()
{
    mCachedCount.count = -1;
    mCachedCount.index = QModelIndex();
    mCachedPosition.count = -1;
    mCachedPosition.index = QModelIndex();
}

int HbTreeModelIteratorPrivate::searchForward(const QModelIndex &index) const
{
    Q_Q(const HbTreeModelIterator);
    int result = mCachedPosition.count;
    QModelIndex startIndex = mCachedPosition.index;
    while (startIndex != index) {
        startIndex = q->nextIndex(startIndex);
        result++;
        if (!startIndex.isValid()) { //end reached, not found
            return -1;
        }
    }
    mCachedPosition.count = result;
    mCachedPosition.index = index;
    return result;
}

int HbTreeModelIteratorPrivate::searchBackward(const QModelIndex &index) const
{
    Q_Q(const HbTreeModelIterator);
    int result = mCachedPosition.count;
    QModelIndex startIndex = mCachedPosition.index;
    while (startIndex != index) {
        startIndex = q->previousIndex(startIndex);
        result--;
        if (startIndex == mRootIndex
            || !startIndex.isValid()) { //start reached, not found
            return -1;
        }
    }
    mCachedPosition.count = result;
    mCachedPosition.index = index;
    return result;
}

QModelIndexList HbTreeModelIteratorPrivate::createParentChainList(const QModelIndex &index) const
{
    QModelIndex loopIndex = index;
    QModelIndexList indexList;
    indexList.append(loopIndex);
    while (loopIndex != mRootIndex
           && loopIndex.isValid()) {
        loopIndex = loopIndex.parent();
        indexList.prepend(loopIndex);
    }
    return indexList;
}

bool HbTreeModelIteratorPrivate::isExpanded(const QModelIndex &index) const
{
    Q_ASSERT(mItemContainer);
    //if (index == mRootIndex) return true; // must be expanded, otherwise mean empty view
    QVariant flags = mItemContainer->itemState(index).value(mExpansionKey);
    if (flags.isValid() && flags.toBool() == true) {
        return true;
    } else {
        return false;
    }
}

bool HbTreeModelIteratorPrivate::isExpandedBranch(const QModelIndex &index) const
{
    if (!index.isValid()) {
        // root index is expanded!
        return (index == mRootIndex);
    }

    QModelIndexList parents = createParentChainList(index);
    if (!parents.isEmpty()
        && !parents.first().isValid()) {
        // returned list contain root, it root is invalid then
        // isExpanded return false for it, so first invalid need to be removed
        parents.removeFirst();
    }
    while (!parents.isEmpty()) {
        if (!isExpanded(parents.first())) {
            return false;
        }
        parents.removeFirst();
    }
    return true;
}
