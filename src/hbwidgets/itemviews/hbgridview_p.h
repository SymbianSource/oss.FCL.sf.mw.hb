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
#ifndef HBGRIDVIEW_P_H
#define HBGRIDVIEW_P_H

#include <hbabstractitemview_p.h>
#include <hbgriditemcontainer_p.h>
#include <hbgridview.h>
#include <hbgridviewitem_p.h>

class HbGridViewParams;
class HbGridViewPrivate: public HbAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(HbGridView)

public:
    HbGridViewPrivate();
    virtual ~HbGridViewPrivate();
    void init();

    QModelIndex itemIndexUnder(const QPointF& point);

    bool visible(HbAbstractViewItem* item, bool fullyVisible = true) const;

    inline HbGridItemContainer *itemContainer() const;

    qreal calculateScrollBarPos() const;
    qreal calculateScrollBarThumbSize() const;
    void updateScrollBar(Qt::Orientation orientation);
    void setScrollBarMetrics(Qt::Orientation orientation);

    void setContentPosition(qreal value, Qt::Orientation orientation, bool animate);

    inline int getScrollDirectionColumnCount() const;
    inline int getScrollDirectionRowCount() const;
    inline HbScrollBar *getScrollDirectionScrollBar() const;
    inline qreal getScrollDirectionContainerPos() const;
    inline qreal getScrollDirectionItemSize() const;

    inline QModelIndex indexInTheCenter() const;
    QModelIndex indexInTheCenter(Qt::Orientations scrollDirection) const;

public:
    bool mIconVisible;
    bool mTextVisible;
    bool mSwapDimensionsOnOrientationChange;

    void relayout();

    void setIconVisible(bool iconVisible);
    void setTextVisible(bool textVisible);

private:
    static HbGridViewPrivate *d_ptr(HbGridView *view)
    {
        Q_ASSERT(view);
        return view->d_func();
    }

    friend class HbGridViewItem;
};

inline HbGridItemContainer *HbGridViewPrivate::itemContainer() const
{
    return qobject_cast<HbGridItemContainer *> (mContainer);
}

inline int HbGridViewPrivate::getScrollDirectionColumnCount() const
{
    return (mScrollDirections == Qt::Vertical) ?
        itemContainer()->columnCount() : itemContainer()->rowCount();
}

inline int HbGridViewPrivate::getScrollDirectionRowCount() const
{
    return (mScrollDirections == Qt::Vertical) ?
        itemContainer()->rowCount() : itemContainer()->columnCount();
}

inline HbScrollBar *HbGridViewPrivate::getScrollDirectionScrollBar() const
{
    return (mScrollDirections == Qt::Vertical) ?
        mVerticalScrollBar : mHorizontalScrollBar;
}

inline qreal HbGridViewPrivate::getScrollDirectionContainerPos() const
{
    return (mScrollDirections == Qt::Vertical) ?
        mContainer->pos().y() : mContainer->pos().x();
}

inline qreal HbGridViewPrivate::getScrollDirectionItemSize() const
{
    // caller responsibility to check if container has any item
    return (mScrollDirections == Qt::Vertical) ?
        mContainer->items().first()->size().height()
        : mContainer->items().first()->size().width();
}

QModelIndex HbGridViewPrivate::indexInTheCenter() const
{
    return indexInTheCenter(mScrollDirections);
}

#endif /*HBGRIDVIEW_P_H*/
