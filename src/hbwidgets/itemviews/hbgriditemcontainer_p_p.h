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
#ifndef HBGRIDITEMCONTAINER_P_P_H
#define HBGRIDITEMCONTAINER_P_P_H

#include <hbabstractitemcontainer_p.h>
#include <hbgriditemcontainer_p.h>
#include <hbapplication.h>

#include <QPair>

class HbGridLayout;
class HbGridItemContainer;

class HbGridItemContainerPrivate: public HbAbstractItemContainerPrivate
{
    Q_DECLARE_PUBLIC(HbGridItemContainer)
public:
    HbGridItemContainerPrivate();
    virtual ~HbGridItemContainerPrivate();
    void init();
    HbAbstractViewItem *shiftDownItem(bool doEvenBadIndex, bool animate);
    bool shiftDownItem(int pos, bool animate);
    HbAbstractViewItem *shiftDown(bool animate);
    HbAbstractViewItem *shiftUpItem(bool animate);
    HbAbstractViewItem *shiftUp(bool animate);
    qreal recycling(qreal diff);
    void resetBuffer();
    void removeItem(const QModelIndex &index, bool animate);
    void scrollToEnsureVisible(const QModelIndex &index);
    void scrollToPositionAtTop(const QModelIndex &index);
    void scrollToPositionAtBottom(const QModelIndex &index);
    void scrollToPositionAtCenter(const QModelIndex &index);
    QModelIndex lastValidItemIndex() const;
    void updateItemBuffer();
    void updateItemBufferContent(int targetCount, const QModelIndex &indexInTheCenter);
    void decreaseBufferSize(int targetCount, int startingIndex);
    void increaseBufferSize(int targetCount, int startingIndex);
    void alignBufferContent(int startingIndex);
    int alignIndexToClosestFirstInRow(int index);

    int mapToLayoutIndex(int index) const;

    HbGridLayout *mLayout;
    QSizeF mViewSize;
    mutable int mMinCount;
    mutable int mRowCount;
    mutable int mColumnCount;
    mutable int mItemsPerRow;
    qreal mCachedItemHeight;
    Qt::Orientations mScrollDirection;
    // used by updateItemBuffer to obtain what change and what kind of update is needed
    int mOldItemsPerRow;

    QList< QPair<HbAbstractViewItem *, int> > mAnimatedItems;
};

#endif /*HBGRIDITEMCONTAINER_P_P_H*/
