/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbCore module of the UI Extensions for Mobile.
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

#ifndef HBSCROLLAREA_P_H
#define HBSCROLLAREA_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "hbscrollarea.h"
#include <hbwidget_p.h>

#include <QTimer>
#include <QTime>

class HbScrollBar;
class QEasingCurve;
class QPanGesture;

class HB_CORE_PRIVATE_EXPORT HbScrollAreaPrivate: public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC( HbScrollArea )

public:
    HbScrollAreaPrivate();
    virtual ~HbScrollAreaPrivate();

    void init();
    void doLazyInit();

    bool mScrollFeedbackOngoing;
    bool mBoundaryReached;
    virtual void updateScrollBar(Qt::Orientation orientation);
    void displayScrollBar(Qt::Orientation orientation);

    void setScrollBarPolicy(Qt::Orientation orientation, HbScrollArea::ScrollBarPolicy policy);
    void replaceScrollBar(Qt::Orientation orientation, HbScrollBar *scrollBar);

    void changeLayoutDirection(Qt::LayoutDirection aNewDirection);

    // returns true if it was able to scroll in either direction
    bool scrollByAmount(const QPointF& delta);

    void startAnimating();
    void stopAnimating();
    virtual void stopScrolling();

    void animateScroll(QPointF speed); // speed in pixels per millisecond

    qreal calculateVelocity ( qreal timeDifference, qreal curVelocity, qreal pos, qreal min, qreal max );
    bool clampToBoundaries ( qreal& deltaRef, qreal pos, qreal min, qreal max );

    void addPositionToQueue(const QPointF& position, qreal time,
                            bool isRelative);
    virtual void setScrollBarMetrics(Qt::Orientation orientation);
    // private slot functions
    void _q_animateScrollTimeout();
    void _q_hideScrollBars();
    void _q_thumbPositionChanged(qreal value, Qt::Orientation orientation);
    void _q_groovePressed(qreal value, Qt::Orientation orientation);
    void _q_thumbPressed();
    void _q_thumbReleased();

    //void bounceBackValueChanged(qreal value);
    virtual void updateScrollMetrics();
    void prepareScrollBars();

    void setContentPosition( const QPointF &newPosition );

    bool sceneEvent(QEvent *event);

    bool positionOutOfBounds ();

    bool pan(QPanGesture *gesture);

    void adjustContent();

    virtual void ensureVisible(QPointF position, qreal xMargin, qreal yMargin);

    void startTargetAnimation (QPointF delta, const int time);

    void createPrimitives();
    void updatePrimitives();
    void updateIndicators(QPointF newPosition);

    void hideChildComponents();

    virtual void setContentPosition( qreal value, Qt::Orientation orientation, bool animate );

    virtual qreal topBoundary ()
    {
        return mContents->boundingRect().top();
    }
    virtual qreal bottomBoundary ()
    {
        Q_Q ( HbScrollArea );
        return mContents->boundingRect().bottom() - q->boundingRect().height();
    }
    virtual qreal leftBoundary ()
    {
        return mContents->boundingRect().left();
    }
    virtual qreal rightBoundary ()
    {
        Q_Q ( HbScrollArea );
        return mContents->boundingRect().right() - q->boundingRect().width();
    }

        // HbScrollAreaPrivate data
    QGraphicsWidget* mContents;
    // We need our own copy of this in order to pass it to the animation state machines
    // valid during animation
    QPointF mScrollPosition;

    Qt::Orientations mScrollDirections;    

    HbScrollBar* mHorizontalScrollBar;
    HbScrollBar* mVerticalScrollBar;

    HbScrollArea::ScrollBarPolicy mVerticalScrollBarPolicy;
    HbScrollArea::ScrollBarPolicy mHorizontalScrollBarPolicy;

    bool mAbleToScrollX;
    bool mAbleToScrollY;

    bool mIsScrolling;
    bool mIsAnimating;

    QPointF mScrollSpeed; // in pixels per ms
    QTimer mScrollTimer;
    QTime mScrollElapsedTime;
    qreal mLastElapsedTime;

    bool mTargetAnimationInProgress;
    qreal mElapsedTimeInAnimation;
    qreal mOverallTimeToTarget;
    QPointF mTargetDelta;
    QPointF mAnimationInitialPosition;
    QEasingCurve* mAnimationShape;

    QTimer mScrollBarHideTimer;

    bool mFrictionEnabled;
    bool mResetAlignment;

    HbScrollArea::ClampingStyle mClampingStyle;
    HbScrollArea::ScrollingStyle mScrollingStyle;    

    // TODO/Q: All circular array related members could go
    // to circular array class if so wanted.

    // circular array to keep track of last N event positions
    // this is used to compute the velocity at the end
    // of a panning action when the scrollingStyle is PanFollowOn
    QTime mDragElapsedTime;
    QPointF mEventPositionQueue[10];
    qreal mEventTimeQueue[10];
    uint mEventPositionQueueSize;
    uint mEventPositionQueueLastIndex;
    bool mEventPositionQueueIsFull;
    Qt::LayoutDirection mLayoutDirection;

    Qt::Alignment mAlignment;
    bool mContinuationIndicators;
    QGraphicsItem *continuationIndicatorTopItem;
    QGraphicsItem *continuationIndicatorBottomItem;
    QGraphicsItem *continuationIndicatorLeftItem;
    QGraphicsItem *continuationIndicatorRightItem;    

private:
    // Private access for the fute application to enable setting tweaking
    // should be removed at some point
    static HbScrollAreaPrivate *d_ptr(HbScrollArea *scrollArea) {
        Q_ASSERT(scrollArea);
        return scrollArea->d_func();
    }
    friend class ParameterTweakerWidget;

    // Tweakable settings
    qreal mSpeedFactor;
    qreal mInertiaSpeedFactor;
    qreal mMaxScrollSpeed;
    qreal mSpringStrength;
    qreal mSpringDampingFactor;
    qreal mFrictionPerMilliSecond;
    bool mMultiFlickEnabled;
};

#endif // HBSCROLLAREA_P_H
