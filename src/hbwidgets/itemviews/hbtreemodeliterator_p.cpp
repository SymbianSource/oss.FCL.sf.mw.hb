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

#include "hbtreemodeliterator_p.h"
#include "hbtreemodeliterator_p_p.h"

HbTreeModelIterator::HbTreeModelIterator(QAbstractItemModel *model,
                                         QModelIndex rootIndex, bool useCache)
    : HbModelIterator(*new HbTreeModelIteratorPrivate())
{
    Q_D(HbTreeModelIterator);
    d->mUseCache = useCache;
    setModel(model, rootIndex);
}

HbTreeModelIterator::~HbTreeModelIterator()
{
}

/*!
    \reimp
    Returns count of visible indexes under \a parent.
    Children of collapsed parents are not taken into account.
*/
int HbTreeModelIterator::indexCount(const QModelIndex &parent) const
{
    Q_D(const HbTreeModelIterator);
    int itemsCount = 0;
    if (d->mModel) {
        QModelIndex parentIndex = parent;
        if (!parentIndex.isValid()) {
            parentIndex = d->mRootIndex;
        }
        if (d->isInCountCache(parentIndex)) {
            return d->mCachedCount.count;
        }
        if (parentIndex != d->mRootIndex) {
            itemsCount = d->childIndexCount(parentIndex);
        } else {
            int rowsCount = d->mModel->rowCount(parentIndex);
            itemsCount += rowsCount;
            for (int i = 0; i < rowsCount; i++) {
                itemsCount += d->childIndexCount(d->mModel->index(i, 0, parentIndex));
            }
        }
        d->mCachedCount.count = itemsCount;
        d->mCachedCount.index = parentIndex;
    }
    return itemsCount;
}

/*!
    \reimp
    Returns ordinal of index starting from root. Ordinal for first index under root is 0.
    Indexes in collapsed parents are taken into account.
*/
int HbTreeModelIterator::indexPosition(const QModelIndex &index) const
{
    Q_D(const HbTreeModelIterator);
    int result = -1;
    if (d->mModel) {
        if (d->isInPositionCache(index)) {
            return d->mCachedPosition.count;
        } else if (d->mCachedPosition.index == d->mRootIndex
                   || lessThan(d->mCachedPosition.index, index)) {  //Cache has been reset, start from the beginning
            result = d->searchForward(index);
        } else {
            result = d->searchBackward(index);
        }
    }
    return result;
}

/*!
    \reimp
    Returnes index of item, which is visible at pos ordinal under parent. 
    Indexes in collapsed parents are not taken into account.
    Very slow - need to interate through whole model in worst case!
*/
QModelIndex HbTreeModelIterator::index(int pos, const QModelIndex &parent) const
{
    Q_D(const HbTreeModelIterator);
    QModelIndex index;
    QModelIndex parentIndex = parent;
    if (!parentIndex.isValid()) {
        parentIndex = d->mRootIndex;
    }
    if (pos < 0) {
        int count = indexCount(parentIndex);
        pos = count - pos;
        if (pos > count) {
            return index;
        }
    }
    if (d->isInCountCache(parentIndex)
        && d->mCachedCount.count >= 0
        && (d->mCachedCount.count/2) < pos) {
        // is this reasonable?
        // last go through whole branch to find last
        // faster than nextIndex but...
        int i = d->mCachedCount.count - pos;
        if (i > 0) {
            index = d->last(parentIndex);
            --i;
        }
        while (i > 0) {
            index = previousIndex(index);
            if (!index.isValid()) break;
            --i;
        }
    } else {
        int i = 0;
        index = d->first(parentIndex);
        while (i < pos) {
            index = nextIndex(index);
            if (index.parent() != parentIndex) {
                // next index must be in the same branch as parent!
                QModelIndex temp = index.parent();
                while (temp.isValid()
                    && temp.parent() != parentIndex) {
                    temp = temp.parent();
                }
                if (!temp.isValid()) {
                    index = temp;
                    break;
                }
            }
            if (!index.isValid()) {
                break;
            }
            ++i;
        }
    }
    return index;
}

QModelIndex HbTreeModelIterator::child(int pos, const QModelIndex &parent) const
{
    Q_D(const HbTreeModelIterator);
    if (d->mModel) {
        return d->mModel->index(pos, 0, parent);
    }
    return QModelIndex();
}

int HbTreeModelIterator::childCount(const QModelIndex &parent) const
{
    Q_D(const HbTreeModelIterator);
    if (d->mModel) {
        return d->mModel->rowCount(parent);
    }
    return 0;
}

/*!
    \reimp

    Next visible index for valid index is determined in following way:
    - If index is in collapsed branch QModelIndex is returned. 
    - If index has children and it is expanded then first child is returned
    - Otherwise if index has next sibling then that is returned
    - Otherwise next valid sibling for parent is returned
    - Otherwise QModelIndex is returned
    If index was invalid then first valid index is returned.

    \a index must belong to mRootIndex branch, otherwise result is not determined.

    To get first index use nextIndex(QModelIndex()).
*/
QModelIndex HbTreeModelIterator::nextIndex(const QModelIndex &index) const
{
    Q_D(const HbTreeModelIterator);
    if (d->mModel) {
        QModelIndex result;
        if (!index.isValid()) {
            result = d->first(d->mRootIndex);
        } else {
            // nextIndex(d->mRootIndex) == first(d->mRootIndex)
            if (!d->isExpandedBranch(index.parent())) {
                return QModelIndex();
            }
            QModelIndex tmpIndex = index;

            do {
                if (d->isExpanded(tmpIndex)) {
                    // if expanded, take first child - going deeper
                    result = tmpIndex.child(0, 0);
                }
                if (!result.isValid()) {
                    // if not expanded, or expanded but without child
                    // take next on the same level
                    result = tmpIndex.sibling(tmpIndex.row()+1, 0);
                }
                while (!result.isValid()) {
                    // if not possible to take next on the same level, go level up
                    tmpIndex = tmpIndex.parent();
                    if (tmpIndex == d->mRootIndex
                        || !tmpIndex.isValid()) {
                        tmpIndex = QModelIndex();
                        break;
                    }
                    result = tmpIndex.sibling(tmpIndex.row()+1, 0);
                }
            }
            while (!result.isValid() && tmpIndex.isValid());
        }
        return result;
    } // if (mModel)
    return QModelIndex();
}

/*!
    \reimp

    Previous visible index for valid index is determined in following way:
    - If index is in collapsed branch QModelIndex is returned
    - If index has previous sibling last child from it is returned
    - Otherwise previous sibling is returned
    - Otherwise parent index is returned
    - Otherwise QModelIndex is returned
    If index was invalid then last valid is returned.

    \a index must belong to mRootIndex branch, otherwise result is not determined.

    To get last index use previousIndex(QModelIndex()).
*/
QModelIndex HbTreeModelIterator::previousIndex(const QModelIndex &index) const
{
    Q_D(const HbTreeModelIterator);
    if (d->mModel) {
        QModelIndex result;
        if (!index.isValid()) {
            result = d->last(d->mRootIndex);
        } else if (index != d->mRootIndex) { // can't go outside mRootIndex
            if (!d->isExpandedBranch(index.parent())) {
                return QModelIndex();
            }
            QModelIndex tmpIndex = index;

            // try to take previous on the same level
            result = tmpIndex.sibling(tmpIndex.row()-1, 0);
            if (result.isValid()) {
                // take last
                tmpIndex = d->last(result);
                if (tmpIndex.isValid()) {
                    result = tmpIndex;
                }
            } else {
                result = tmpIndex.parent();
                if (result == d->mRootIndex) {
                    result = QModelIndex();
                }
            }
        }
        return result;
    }
    return QModelIndex();
}

void HbTreeModelIterator::setModel(QAbstractItemModel *model,
                                   QModelIndex rootIndex)
{
    Q_D(HbTreeModelIterator);
    if (model != d->mModel) {
        d->resetCache();
        d->setModel(model, rootIndex);
        if (d->mModel) {
            connect(d->mModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                    this, SLOT(rowsInserted(QModelIndex,int,int)));
            connect(d->mModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                    this, SLOT(rowsRemoved(QModelIndex,int,int)));
            connect(d->mModel, SIGNAL(columnsInserted(QModelIndex,int,int)),
                    this, SLOT(columnsInserted(QModelIndex,int,int)));
            connect(d->mModel, SIGNAL(columnsRemoved(QModelIndex,int,int)),
                    this, SLOT(columnsRemoved(QModelIndex,int,int)));
            connect(d->mModel, SIGNAL(layoutChanged()),
                    this, SLOT(modelLayoutChanged()));
        }
    } else {
        setRootIndex(rootIndex);
    }
}

void HbTreeModelIterator::setRootIndex(QModelIndex rootIndex)
{
    if (d->mRootIndex != rootIndex) {
        Q_D(HbTreeModelIterator);
        d->setRootIndex(rootIndex);
        d->resetCache();
    }
}

void HbTreeModelIterator::setItemContainer(HbAbstractItemContainer *itemContainer)
{
    Q_D(HbTreeModelIterator);
    d->mItemContainer = itemContainer;
}

bool HbTreeModelIterator::lessThan(const QModelIndex &index1,
                                    const QModelIndex &index2) const
{
    Q_D(const HbTreeModelIterator);
    if (index1 == index2) {
        return false;
    }

    //- Create lists from item to root so that the root will be the first item in the list (prepend...)
    QModelIndexList indexList1 = d->createParentChainList(index1);
    QModelIndexList indexList2 = d->createParentChainList(index2);

    int list1Count = indexList1.count();
    int list2Count = indexList2.count();
    int minCount = qMin(list1Count, list2Count);

    //- Loop the lists starting from the root until different item is found or if other list ends.
    //- If difference is found the comparison can be made compring the rows
    //- If no difference has been found and other list ends step out of the loop
    for (int i = 0; i < minCount; i++) {
        if (indexList1.at(i)!= indexList2.at(i)) {
            if (indexList1.at(i).row() < indexList2.at(i).row()) {
                return true;
            } else {
                return false;
            }
        }
    }

    //- Now we can be sure that the items to be compared are from the same branch.
    //- The longer list indicates lower position
    return (list1Count < list2Count);
}

void HbTreeModelIterator::itemExpansionChanged(const QModelIndex &index)
{
    Q_UNUSED(index);
    Q_D(HbTreeModelIterator);
    d->resetCache();
}

void HbTreeModelIterator::rowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_D(HbTreeModelIterator);
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    d->resetCache();
}

void HbTreeModelIterator::rowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_D(HbTreeModelIterator);
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    d->resetCache();
}

void HbTreeModelIterator::columnsInserted(const QModelIndex &parent, int start, int end)
{
    Q_D(HbTreeModelIterator);
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    d->resetCache();
}

void HbTreeModelIterator::columnsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_D(HbTreeModelIterator);
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    d->resetCache();
}

void HbTreeModelIterator::modelLayoutChanged()
{
    Q_D(HbTreeModelIterator);
    d->resetCache();
}

#include "moc_hbtreemodeliterator_p.cpp"

