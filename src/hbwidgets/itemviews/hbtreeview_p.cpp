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
    
#include "hbtreeview_p.h"
#include "hbtreeview.h"

#include "hbabstractitemcontainer_p.h"

const QString KDefaultLayoutOption = "default";

HbTreeViewPrivate::HbTreeViewPrivate() :
    HbAbstractItemViewPrivate(),
    mSelectionStarted(false),
    mInSetExpanded(false)
{
}

HbTreeViewPrivate::~HbTreeViewPrivate()
{
}

void HbTreeViewPrivate::init()
{
    Q_Q(HbTreeView);
    q->setItemRecycling(true);
    q->setScrollDirections(Qt::Horizontal | Qt::Vertical);
    mLayoutOptionName = KDefaultLayoutOption;
    treeModelIterator()->setItemContainer(mContainer);
}

bool HbTreeViewPrivate::isParentValid(const QModelIndex &parent) const
{
    bool valid = false;
    QModelIndex currentParent = parent;
    while (!valid) {
        if (currentParent == mModelIterator->rootIndex()) {
            valid = true;
        } else if (currentParent.isValid()) {
            currentParent = currentParent.parent();
        } else {
            break;
        }
    }
    return valid;
}

bool HbTreeViewPrivate::isChild(const QModelIndex &index, const QModelIndex &parent) const
{
    bool isChild = false;
    QModelIndex currentIndex = index;
    while (!isChild) {
        if (currentIndex.parent() == parent) {
            isChild = true;
        } else if (currentIndex.isValid()) {
            currentIndex = currentIndex.parent();
        } else {
            break;
        }
    }
    return isChild;
}

QModelIndex HbTreeViewPrivate::searchIndexUp( 
                            const   QModelIndex &index,
                            int     maxStepCount) const
{
    Q_Q(const HbTreeView);

    QModelIndex root = q->rootIndex();
    int counter = 1;

    // previousIndex() returns invalid, if root found
    QModelIndex previousIndex = index;
    QModelIndex previousIndexTemp = mModelIterator->previousIndex(previousIndex);
    while (previousIndex.isValid() && counter < maxStepCount) {
        counter++;
        previousIndexTemp = mModelIterator->previousIndex(previousIndex);
        if (previousIndexTemp.isValid()) {
            previousIndex = previousIndexTemp;
        }
    }
    return previousIndex;
}

/*!
    \reimp
*/
bool HbTreeViewPrivate::handleScrollBar(Qt::Orientation orientation)
{
    if (!mContainer->itemRecycling()
        || orientation == Qt::Horizontal
        || mContainer->itemPrototypes().count() != 1
        || mContainer->items().isEmpty()
        || !mVerticalScrollBar) {
        return false;
    } else {
        return true;
    }
}

bool HbTreeViewPrivate::animationEnabled(bool insertOperation)
{
    if (mInSetExpanded) {
        if (insertOperation) {
            if (mEnabledAnimations & HbAbstractItemView::Expand) {
                return true;
            }
        } else {
            if (mEnabledAnimations & HbAbstractItemView::Collapse) {
                return true;
            }
        }
        return false;
    } else {
        return HbAbstractItemViewPrivate::animationEnabled(insertOperation);

    }
}

