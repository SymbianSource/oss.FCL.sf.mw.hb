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

#ifndef HBGRIDLAYOUT_P_P_H
#define HBGRIDLAYOUT_P_P_H

#include <hbgridlayout_p.h>
#include <QTransform>

class HbGridLayout;

class HbGridLayoutPrivate
{
    Q_DECLARE_PUBLIC(HbGridLayout)
public:

    struct AnimationData {
        QGraphicsItem *item;
        
        QPointF startPos;
        QPointF startScenePos;
        qreal startDelay;
        
        qreal originalM31;
        qreal originalM32;
        int originalZValue;
        
        HbGridLayout::AnimationType type;
        QGraphicsLayoutItem *startItem;
    };

    HbGridLayoutPrivate();
    ~ HbGridLayoutPrivate();

    void init();

    void setSize(const QSizeF &size,int &targetCount);
    void setGeometry(const QRectF &effectiveRect);
    void setVerticalGeometry(const QRectF &effectiveRect);
    void setHorizontalGeometry(const QRectF &effectiveRect);
    QSizeF horSizeHint() const;
    QSizeF verSizeHint() const;
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF( )) const;
    void setMargins(qreal topMargin, qreal bottomMargin, qreal leftMargin, qreal rightMargin);
    void reset();

    void insertItem(QGraphicsLayoutItem *item, int index, bool animate, bool reset);
    void removeItem(QGraphicsLayoutItem *item, int index, bool animate);

    void updateAnimationList(QGraphicsLayoutItem *item, int index, HbGridLayout::AnimationType type);

    void stopAnimation(AnimationData *data);

    AnimationData *animationData(QGraphicsLayoutItem *item) const;

    void _q_timerUpdate(bool updateProgress = true);

public:

    int mMaxVisRows;
    int mMaxVisColumns;
    Qt::Orientations mScrollDirection;
    QSizeF mItemSize;
    QSizeF mPreferedSizeHint;
    QSizeF mMaxSizeHint;
    qreal mSpacing;
    qreal mLeftMargin;
    qreal mRightMargin;
    qreal mTopMargin;
    qreal mBottomMargin;
    bool mClearCacheHint;
    bool mClearGeometry;
    bool mRecycling;
    QRectF mGeometry;
    QList<QGraphicsLayoutItem*> mItems;
    HbGridLayout *q_ptr;

    QList<AnimationData *> mAnimatedItems;
    QTimer *mTimer;

    bool mEffectLoaded;
};
#endif // HBGRIDLAYOUT_P_P_H
