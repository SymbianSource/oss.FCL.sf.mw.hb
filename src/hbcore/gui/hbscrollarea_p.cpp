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

#include "hbscrollarea_p.h"
#include "hbscrollarea.h"
#include "hbstyleoption.h"
#include <hbscrollbar_p.h>

#include <hbgesture.h>
#include <hbgesturefilter.h>
#include <hbscrollbar.h>
#include <hbwidgetfeedback.h>

#include <QGraphicsSceneMouseEvent>
#include <qmath.h>
#include <QVariantAnimation>

#include <QDebug>

#if 0

#include <QGraphicsSceneMouseEvent>
#include <QApplication>
#include <math.h>

#endif // HB_NEW_GESTURE_FW

// These constants control parameters of the inertia and bounce back animations and
// have been selected heuristically to produce good results.  They
// likely do not need to be controlled programatically, so they are just defined
// here.
namespace
{
static const int FLICKMINDISTANCE = 50;
/*  PANMINDISTANCE is also the threshold for longpress.
 Value of 50pixels roughly translates to 5mm/5mm area on 3.2inch nHD screen. */
static const int PANMINDISTANCE = 40;

// Factor that is used to convert flick to pixels per ms
static const qreal SCROLLSPEED_FACTOR = 0.0004;
// With this it's possible to have increased initial speed when
// inertia is enabled
static const qreal INERTIA_SPEEDFACTOR = 2.5;

static const qreal MAX_SCROLLSPEED_PER_MILLISECOND = 3.0; // per millisecond
static const qreal MIN_SPEED_PER_MILLISECOND = 0.005;

static const qreal SPRING_STRENGTH = 0.0013;
static const qreal SPRING_DAMPING_FACTOR = 0.33;

// Friction factor applied to the speed if inertia is enabled
static const qreal FRICTION_PER_MILLISECOND = 0.003;

const qreal MAX_TIMEDIF_FOR_SIMULATION = 30.0;

static const int INITIAL_SCROLLBAR_HIDE_TIMEOUT = 4000;
static const int SCROLLBAR_HIDE_TIMEOUT = 2000;

static const int PAGE_CHANGE_DURATION = 170;

static const int ANIMATION_INTERVAL = 10;
}

HbScrollAreaPrivate::HbScrollAreaPrivate() :
        mScrollFeedbackOngoing(false),
        mBoundaryReached(false),
        mContents(0), mScrollDirections(Qt::Vertical),
        mGestureFilter(0),
        mHorizontalScrollBar(0),
        mVerticalScrollBar(0),
        mVerticalScrollBarPolicy(HbScrollArea::ScrollBarAutoHide),
        mHorizontalScrollBarPolicy(HbScrollArea::ScrollBarAutoHide),
        mAbleToScrollX(false),
        mAbleToScrollY(false),
        mIsScrolling(false),
        mIsAnimating(false),
        mScrollSpeed (QPointF (0,0)),
        mScrollTimer(0),
        mScrollElapsedTime(),
        mLastElapsedTime(0.0),
        mTargetAnimationInProgress(false),
        mElapsedTimeInAnimation(0.0),
        mOverallTimeToTarget(0.0),
        mTargetDelta(QPointF(0,0)),
        mAnimationInitialPosition (QPointF(0,0)),
        mAnimationShape (0),
        mScrollBarHideTimer(0),
        mFrictionEnabled(true),
        mClampingStyle(HbScrollArea::BounceBackClamping),
        mScrollingStyle(HbScrollArea::PanOrFlick),
        mHandleLongPress(false),
        mOrientationChanged(false),
        mEventPositionQueueSize(10),
        mEventPositionQueueLastIndex(0),
        mEventPositionQueueIsFull(false),
        mLayoutDirection(Qt::LeftToRight),
        mAlignment(Qt::AlignLeft | Qt::AlignTop),
        mContinuationIndicators(false),
        continuationIndicatorTopItem(0),
        continuationIndicatorBottomItem(0),
        continuationIndicatorLeftItem(0),
        continuationIndicatorRightItem(0),
        // These values can be tweaked with the private access from Fute application
        mSpeedFactor ( SCROLLSPEED_FACTOR ),
        mInertiaSpeedFactor ( INERTIA_SPEEDFACTOR ),
        mMaxScrollSpeed ( MAX_SCROLLSPEED_PER_MILLISECOND ),
        mSpringStrength ( SPRING_STRENGTH ),
        mSpringDampingFactor ( SPRING_DAMPING_FACTOR ),
        mFrictionPerMilliSecond ( FRICTION_PER_MILLISECOND ),
        mMultiFlickEnabled (false )
{

}

HbScrollAreaPrivate::~HbScrollAreaPrivate()
{
    Q_Q( HbScrollArea );

    if (mGestureFilter) {
        q->removeSceneEventFilter(mGestureFilter);

        delete mGestureFilter;

        mGestureFilter = 0;
    }
}

void HbScrollAreaPrivate::init()
{
    Q_Q( HbScrollArea );

    mScrollTimer.setParent(q);
    mScrollBarHideTimer.setParent(q);
    mScrollBarHideTimer.setSingleShot(true);

    q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    q->setFlag( QGraphicsItem::ItemClipsChildrenToShape, true );
    q->setFocusPolicy(Qt::StrongFocus);

    // Scrollbar is created when it is shown for the first time.
    mVerticalScrollBar = new HbScrollBar(Qt::Vertical, q);
    HbStyle::setItemName(mVerticalScrollBar, "scrollbar-vertical");

    // make sure the scrollbar is on top
    mVerticalScrollBar->setZValue(q->zValue() + 1);
    mVerticalScrollBar->setVisible(false);
    // Scrollbar is created when it is shown for the first time.
    mHorizontalScrollBar = new HbScrollBar(Qt::Horizontal, q);
    HbStyle::setItemName(mHorizontalScrollBar, "scrollbar-horizontal");
    mHorizontalScrollBar->setVisible(false);
    // make sure the scrollbar is on top
    mHorizontalScrollBar->setZValue(q->zValue() + 1);

}

void HbScrollAreaPrivate::doLazyInit()
{
    Q_Q( HbScrollArea );
    updateGestures();
    QObject::connect(mVerticalScrollBar, SIGNAL(valueChanged(qreal, Qt::Orientation)), q, SLOT(_q_thumbPositionChanged(qreal, Qt::Orientation)));
    QObject::connect(mVerticalScrollBar, SIGNAL(valueChangeRequested(qreal, Qt::Orientation)), q, SLOT(_q_groovePressed(qreal, Qt::Orientation)));
    QObject::connect(mHorizontalScrollBar, SIGNAL(valueChanged(qreal, Qt::Orientation)), q, SLOT(_q_thumbPositionChanged(qreal, Qt::Orientation)));
    QObject::connect(mHorizontalScrollBar, SIGNAL(valueChangeRequested(qreal, Qt::Orientation)), q, SLOT(_q_groovePressed(qreal, Qt::Orientation)));
#if 0
    QApplication::setAttribute(Qt::AA_EnableGestures);
    grabGesture(Qt::PanGesture);
    mFlickTimer = new QTime();
    mPanDirection = Qt::NoDirection;
#endif // HB_NEW_GESTURE_FW
    QObject::connect(&(mScrollTimer), SIGNAL(timeout()), q, SLOT(_q_animateScrollTimeout()));
    QObject::connect(&(mScrollBarHideTimer), SIGNAL(timeout()), q, SLOT(_q_hideScrollBars()));
}

void HbScrollAreaPrivate::replaceScrollBar(Qt::Orientation orientation, HbScrollBar *scrollBar)
{
    Q_Q(HbScrollArea);
    if (orientation == Qt::Horizontal && scrollBar != mHorizontalScrollBar) {
        delete mHorizontalScrollBar;
        mHorizontalScrollBar = scrollBar;
    } else if (orientation == Qt::Vertical && scrollBar != mVerticalScrollBar) {
        delete mVerticalScrollBar;
        mVerticalScrollBar = scrollBar;
    } else {
        return;
    }

    scrollBar->setParentItem(q);

    // make sure the scrollbar is on top
    scrollBar->setZValue(q->zValue() + 1);

    prepareScrollBars();

    QObject::connect(scrollBar, SIGNAL(valueChanged(qreal, Qt::Orientation)), q, SLOT(_q_thumbPositionChanged(qreal, Qt::Orientation)));
}

void HbScrollAreaPrivate::changeLayoutDirection(
        Qt::LayoutDirection aNewDirection)
{
    mLayoutDirection = aNewDirection;
}

void HbScrollAreaPrivate::updateGestures()
{
    Q_Q( HbScrollArea );

    // Gestures are scene events, if we don't have the scene, no need to take action
    if (q->scene() == 0) {
        return;
    }

    if (mGestureFilter != 0) {
        q->removeSceneEventFilter(mGestureFilter);

        // ??? need to delete the filter ???
        delete mGestureFilter;

        mGestureFilter = 0;
    }

    mGestureFilter = new HbGestureSceneFilter(Qt::LeftButton, q);

    HbGesture* gesture = 0;

    // add flick gestures only for PanOrFlick scrolling style
    if (mScrollingStyle == HbScrollArea::PanOrFlick) {
        if (mScrollDirections & Qt::Vertical) {
            gesture = new HbGesture(HbGesture::up, FLICKMINDISTANCE);
            mGestureFilter->addGesture(gesture);
            QObject::connect(gesture, SIGNAL(triggered(int)), q, SLOT(upGesture(int)));

            gesture = new HbGesture(HbGesture::down, FLICKMINDISTANCE);
            mGestureFilter->addGesture(gesture);
            QObject::connect(gesture, SIGNAL(triggered(int)), q, SLOT(downGesture(int)));
        }

        if (mScrollDirections & Qt::Horizontal) {
            gesture = new HbGesture(HbGesture::left, FLICKMINDISTANCE);
            mGestureFilter->addGesture(gesture);
            QObject::connect(gesture, SIGNAL(triggered(int)), q, SLOT(leftGesture(int)));

            gesture = new HbGesture(HbGesture::right, FLICKMINDISTANCE);
            mGestureFilter->addGesture(gesture);
            QObject::connect(gesture, SIGNAL(triggered(int)), q, SLOT(rightGesture(int)));
        }
    }

    gesture = new HbGesture(HbGesture::pan, PANMINDISTANCE);
    mGestureFilter->addGesture(gesture);
    QObject::connect(gesture, SIGNAL(panned(QPointF)), q, SLOT(panGesture(QPointF)));

    if (mHandleLongPress) {
        gesture = new HbGesture(HbGesture::longpress, 0);
        mGestureFilter->addGesture(gesture);
        QObject::connect(gesture, SIGNAL(longPress(QPointF)), q, SLOT(longPressGesture(QPointF)));
        mGestureFilter->setLongpressAnimation(true);
    }

#if 1
    q->installSceneEventFilter(mGestureFilter);
#endif //HB_NEW_GESTURE_FW

}

bool HbScrollAreaPrivate::scrollByAmount(const QPointF& delta)
{
    Q_Q( HbScrollArea );

    if ( mContents == 0 ||
         ( qIsNull(qAbs( delta.x() )) && qIsNull(qAbs( delta.y() ) ) ) )
        return false;

    QPointF currentPosition = -mContents->pos();
    QPointF newPosition = currentPosition;
    qreal newXPosition;
    qreal newYPosition;

#if 0
    mAbleToScrollY = true;
#endif //HB_NEW_GESTURE_FW

    bool scrollingHitBoundary(false);
    if (mAbleToScrollX) {
        newXPosition = currentPosition.x() + delta.x();

        if (mClampingStyle == HbScrollArea::StrictClamping) {
            if ( newXPosition < leftBoundary () ) {
                newXPosition = leftBoundary();
                mScrollSpeed.setX( 0.0 );
                scrollingHitBoundary = true;
            } else if ( newXPosition > rightBoundary() ) {
                newXPosition = rightBoundary();
                mScrollSpeed.setX( 0.0 );
                scrollingHitBoundary = true;
            }
        }

        newPosition.setX(newXPosition);
    }

    if (mAbleToScrollY) {
        newYPosition = currentPosition.y() + delta.y();

        if (mClampingStyle == HbScrollArea::StrictClamping) {
            if ( newYPosition < topBoundary () ) {
                newYPosition = topBoundary();
                mScrollSpeed.setY( 0.0 );
                scrollingHitBoundary = true;
            } else if ( newYPosition > bottomBoundary() ) {
                newYPosition = bottomBoundary();
                mScrollSpeed.setY( 0.0 );
                scrollingHitBoundary = true;
            }
        }
        newPosition.setY(newYPosition);
    }

    if (mContinuationIndicators) {
        updateIndicators(newPosition);
    }

    if (scrollingHitBoundary) {
        if (!mBoundaryReached) {
            mBoundaryReached = true;
            if (mScrollFeedbackOngoing) {
                HbWidgetFeedback::triggered(q, Hb::InstantBoundaryReached);
            }
        }
    }

    setContentPosition(-newPosition);

    if (!mIsScrolling) {
        mIsScrolling = true;
        emit q->scrollingStarted();

        if (mAbleToScrollX && mHorizontalScrollBarPolicy == HbScrollArea::ScrollBarAutoHide) {
            displayScrollBar(Qt::Horizontal);
        }

        if (mAbleToScrollY && mVerticalScrollBarPolicy == HbScrollArea::ScrollBarAutoHide) {
            displayScrollBar(Qt::Vertical);
        }
    }  
    if (mAbleToScrollX && mHorizontalScrollBar->isVisible()) {
        updateScrollBar(Qt::Horizontal);
    }

    if (mAbleToScrollY && mVerticalScrollBar->isVisible()) {
        updateScrollBar(Qt::Vertical);
    }

    bool positionChanged = currentPosition != newPosition;
    if (positionChanged && mScrollBarHideTimer.isActive()) {
        mScrollBarHideTimer.stop();
    }

    return (positionChanged);
}

void HbScrollAreaPrivate::animateScroll(QPointF speed)
{
    if (mContents == 0)
        return;

    if (mTargetAnimationInProgress) {
        mTargetAnimationInProgress = false;
    }

    mScrollPosition = -mContents->pos();

    if ( mFrictionEnabled ) {
        speed.rx() *= mInertiaSpeedFactor;
        speed.ry() *= mInertiaSpeedFactor;
    }
    speed.rx() = qBound ( -mMaxScrollSpeed, speed.rx(), mMaxScrollSpeed );
    speed.ry() = qBound ( -mMaxScrollSpeed, speed.ry(), mMaxScrollSpeed );

    if ( !mMultiFlickEnabled ) {
        // normal behavior:
        mScrollSpeed = speed;
    } else {
        // Prototype multiflick behavior:
        if ( ( mScrollSpeed.x() < 0.0 && speed.x() < 0.0 ) ||
             ( mScrollSpeed.x() > 0.0 && speed.x() > 0.0 ) ) {
            mScrollSpeed.rx() += speed.x();
        } else {
            mScrollSpeed.setX( speed.x() );
        }

        if ( ( mScrollSpeed.y() < 0.0 && speed.y() < 0.0 )  ||
             ( mScrollSpeed.y() > 0.0 && speed.y() > 0.0 ) ) {
            mScrollSpeed.ry() += speed.y();
        } else {
            mScrollSpeed.setY( speed.y() );
        }
    }

    startAnimating();
}

void HbScrollAreaPrivate::startAnimating()
{
    // Launch the animation timer
    if (!mScrollTimer.isActive()) {
        mScrollElapsedTime.restart();
        mLastElapsedTime = 0.0;
        mScrollTimer.start(ANIMATION_INTERVAL);
        mIsAnimating = true;
    }
}

void HbScrollAreaPrivate::stopScrolling()
{
    Q_Q( HbScrollArea );
    if (mHorizontalScrollBarPolicy == HbScrollArea::ScrollBarAutoHide ||
        mVerticalScrollBarPolicy == HbScrollArea::ScrollBarAutoHide) {
        mScrollBarHideTimer.start(SCROLLBAR_HIDE_TIMEOUT);
    }
    if (mIsScrolling) {
        if (mScrollFeedbackOngoing) {
            HbWidgetFeedback::continuousStopped(q, Hb::ContinuousScrolled);
            mScrollFeedbackOngoing = false;
        }
        mBoundaryReached = false;
        mIsScrolling = false;
        emit q->scrollingEnded();
    }
}

void HbScrollAreaPrivate::stopAnimating()
{
    mScrollTimer.stop();

    mIsAnimating = false;

    stopScrolling();
}

/*
 * animateScrollTimeout() is a private slot function called during
 * follow-on scroll animation
 */
void HbScrollAreaPrivate::_q_animateScrollTimeout()
{
    Q_Q( HbScrollArea );
    qreal elapsedTime = qreal(mScrollElapsedTime.elapsed());

    qreal timeDifference = elapsedTime - mLastElapsedTime;

    // if no time has elapsed, just return
    // this sometimes happens if the timer accuracy is not that great
    if (qIsNull(timeDifference)) {
        return;
    }

    mLastElapsedTime = elapsedTime;

    if ( mTargetAnimationInProgress ) {
        mElapsedTimeInAnimation += timeDifference;
        qreal progressFactor = mAnimationShape->valueForProgress(qMin ( qreal(1.0), mElapsedTimeInAnimation / mOverallTimeToTarget));
        QPointF newPosition = mAnimationInitialPosition + mTargetDelta * progressFactor;

        q->scrollByAmount( newPosition + mContents->pos() );

        if ( mElapsedTimeInAnimation >= mOverallTimeToTarget ){
            mTargetAnimationInProgress = false;
        }
    } else {

        // calculations are split so that the same amount of simulation steps
        // are made in varying framerates.
        do {
            qreal simulationStepInTime = qMin ( MAX_TIMEDIF_FOR_SIMULATION, timeDifference);

            // Calculate new velocity to horizontal movement
            mScrollSpeed.setX( calculateVelocity ( simulationStepInTime,
                                                   mScrollSpeed.x(),
                                                   -mContents->pos().x(),
                                                   leftBoundary(),
                                                   rightBoundary() ) );

            // Calculate new velocity to vertical movement
            mScrollSpeed.setY( calculateVelocity ( simulationStepInTime,
                                                   mScrollSpeed.y(),
                                                   -mContents->pos().y(),
                                                   topBoundary(),
                                                   bottomBoundary() ) );

            QPointF overAllDist = mScrollSpeed * simulationStepInTime;

            bool stopVerticalAnimation = false;
            if ( qAbs( mScrollSpeed.y() ) < MIN_SPEED_PER_MILLISECOND ) {
                stopVerticalAnimation = clampToBoundaries ( overAllDist.ry(), -mContents->pos().y(), topBoundary(), bottomBoundary() );
            }

            bool stopHorizontalAnimation = false;
            if ( qAbs( mScrollSpeed.x() ) < MIN_SPEED_PER_MILLISECOND ) {
                stopHorizontalAnimation = clampToBoundaries ( overAllDist.rx(), -mContents->pos().x(), leftBoundary(), rightBoundary() );
            }

            q->scrollByAmount ( -overAllDist );

            if ( ( !mAbleToScrollY || stopVerticalAnimation ) &&
                 ( !mAbleToScrollX || stopHorizontalAnimation ) ) {
                stopAnimating();
                break;
            }

            timeDifference -= MAX_TIMEDIF_FOR_SIMULATION;

        } while ( timeDifference > MAX_TIMEDIF_FOR_SIMULATION );
    }
    if ( mContents->pos() != -mScrollPosition && mIsAnimating) {
        // Feedback from panning is started in HbScrollArea::panGesture method,
        // feedback from flick is started here
        HbWidgetFeedback::continuousTriggered(q, Hb::ContinuousScrolled, mContents->pos() + mScrollPosition);
        mScrollFeedbackOngoing = true;
    }
}

qreal HbScrollAreaPrivate::calculateVelocity ( qreal timeDifference, qreal curVelocity, qreal pos, qreal min, qreal max )
    {
    Q_Q( HbScrollArea );
    qreal ret(curVelocity);
    qreal overShoot(0.0);
    if ( pos < min ) {
        overShoot = min - pos;
    } else if ( pos > max ) {
        overShoot = max - pos;
    }
    
    if ( !qIsNull(overShoot)&& !mBoundaryReached) {
        HbWidgetFeedback::triggered(q, Hb::InstantBoundaryReached);
        mBoundaryReached = true;
    }
    if ( !qIsNull(overShoot) && mClampingStyle == HbScrollArea::BounceBackClamping ){
        ret -= mSpringStrength * overShoot - ( mSpringDampingFactor * -curVelocity );
    } else if ( mFrictionEnabled ) {
        // Apply friction if enabled
        ret *= 1.0 - ( mFrictionPerMilliSecond * timeDifference );

        if ( qAbs ( ret ) < MIN_SPEED_PER_MILLISECOND ) {
            ret = 0.0;
        }
    }

    return ret;
}

bool HbScrollAreaPrivate::clampToBoundaries ( qreal& deltaRef, qreal pos, qreal min, qreal max )
{
    bool ret = false;
    qreal clampToValue(0.0);
    if ( pos < min ) {
        clampToValue = pos - min;
    } else if ( pos > max ) {
        clampToValue = pos - max;
    }
    if ( qAbs( clampToValue ) < 1.0 ) {
        deltaRef = clampToValue;
        ret = true;
    }
    return ret;
}

void HbScrollAreaPrivate::addPositionToQueue(const QPointF& position,
        qreal timestamp, bool isRelative)
{
    // queue up the event
    // if isRelative is true, there should have already been an event entered in the queue
    if (isRelative) {
        int prevIndex = (mEventPositionQueueLastIndex + mEventPositionQueueSize
                - 1) % mEventPositionQueueSize;
        mEventPositionQueue[mEventPositionQueueLastIndex]
                = mEventPositionQueue[prevIndex] + position;
    }
    else {
        mEventPositionQueue[mEventPositionQueueLastIndex] = position;
    }

    mEventTimeQueue[mEventPositionQueueLastIndex] = timestamp;

    mEventPositionQueueLastIndex = (mEventPositionQueueLastIndex + 1)
            % mEventPositionQueueSize;
    if (mEventPositionQueueLastIndex == 0) {
        mEventPositionQueueIsFull = true;
    }
}

/*

 * sets position of content and emit signal about this change
 * @param newPosition
 */
void HbScrollAreaPrivate::setContentPosition(const QPointF &newPosition)
{
    Q_ASSERT_X(mContents,"HbScrollAreaPrivate::setContentPosition","wrong use - content is not set");
    QPointF oldPosition( mContents->pos() );
    if ( newPosition!=oldPosition ) {
        Q_Q(HbScrollArea);

        mContents->setPos(newPosition);
        emit q->scrollPositionChanged(-newPosition);
    }
}

/*
* returns true if the current scrollposition is out of bounds
*/
bool HbScrollAreaPrivate::positionOutOfBounds () {
    if ( ( mAbleToScrollX &&
         (-mContents->pos().x() < leftBoundary() ||
          -mContents->pos().x() > rightBoundary() ) ) ||
         ( mAbleToScrollY &&
         (-mContents->pos().y() < topBoundary() ||
          -mContents->pos().y() > bottomBoundary() ) ) ) {
        return true;
    } else {
        return false;
    }

}

void HbScrollAreaPrivate::mousePressEvent(QGraphicsSceneMouseEvent *event)
{

    if (mIsAnimating && !positionOutOfBounds() && !mMultiFlickEnabled) {
        stopAnimating();
        stopScrolling();
    }

    if (mScrollingStyle == HbScrollArea::PanWithFollowOn) {
        mDragElapsedTime.restart();
        mEventPositionQueueLastIndex = 0;
        mEventPositionQueueIsFull = false;
        addPositionToQueue(event->pos(), 0, false);
    }
    event->accept();
}

const int VELOCITY_SAMPLE_PERIOD=250;
void HbScrollAreaPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_Q ( HbScrollArea );

    bool hasFollowOn = false;
    int  currIndex = 0;

    // Q: Could there be following classes, which takes some responsibilities
    // away from the HbScrollArea/HbScrollAreaPrivate. Such as:
    // A) A circular buffer class
    // B) A average time keeper class, where one can change the VELOCITY_SAMPLE_PERIOD and/or
    // the method to calculate the average time.
    // Time keeper class would use the circular buffer object.
    // Then, HbScrollArea/HbScrollAreaPrivate would hand the position/time of the events over to
    // the Time keeper and ask the average in the end.
    if ( mScrollingStyle == HbScrollArea::PanWithFollowOn ) {
        // queue the release position
        addPositionToQueue(event->pos(), mDragElapsedTime.elapsed(), false);

        // there has to be at least one value in the queue since we just added one

        /* UNCOMMENT TO PRINT QUEUE VALUES */
        /*
                currIndex = d->mEventPositionQueueFirstIndex;
                int ii = 0;
                do {
                        qDebug() << "POSITION QUEUE" << ii++ << d->mEventPositionQueue[currIndex] << " TIME " << d->mEventTimeQueue[currIndex];
                        currIndex = (currIndex + 1) % d->mEventPositionQueueSize;
                } while (currIndex != d->mEventPositionQueueLastIndex);
                */

        // go through the data and compute the average speed in pixels per second over the
        // last VELOCITY_SAMPLE_PERIOD ms
        int lastIndex = (mEventPositionQueueLastIndex + mEventPositionQueueSize - 1) % mEventPositionQueueSize;
        qreal lastTime = mEventTimeQueue[lastIndex];
        QPointF lastPoint = mEventPositionQueue[lastIndex];

        qreal startAvgTime(lastTime);
        QPointF startAvgPoint(lastPoint);

        if (mEventPositionQueueIsFull) {
            // go backward through the queue to average the velocity over the last N milliseconds
            currIndex = (lastIndex + mEventPositionQueueSize - 1) % mEventPositionQueueSize;
            while (currIndex != lastIndex) {
                if ((lastTime - mEventTimeQueue[currIndex]) > VELOCITY_SAMPLE_PERIOD)
                    break;

                startAvgTime = mEventTimeQueue[currIndex];
                startAvgPoint = mEventPositionQueue[currIndex];

                currIndex = (currIndex + mEventPositionQueueSize - 1) % mEventPositionQueueSize;
            }
        } else {
            currIndex = (lastIndex - 1);
            while (currIndex >= 0) {
                if ((lastTime - mEventTimeQueue[currIndex]) > VELOCITY_SAMPLE_PERIOD)
                    break;

                startAvgTime = mEventTimeQueue[currIndex];
                startAvgPoint = mEventPositionQueue[currIndex];

                currIndex -= 1;
            }
        }

        //		qDebug() << "TIME DIFF" << (lastTime - startAvgTime) << "POS DIFF" << (lastPoint - startAvgPoint);

        // ??? need to handle both directions at once
        qreal timeDiff(lastTime - startAvgTime);
        QPointF posDiff(lastPoint - startAvgPoint);

        if (timeDiff > 0.0) {
            QPointF speed = posDiff / timeDiff;

            if (!mScrollDirections.testFlag(Qt::Horizontal)) {
                speed.setX(0.0f);
            }

            if (!mScrollDirections.testFlag(Qt::Vertical)) {
                speed.setY(0.0f);
            }

            QRectF contentRect = mContents->boundingRect();
            if (contentRect.width() < q->size().width()) {
                speed.setX(0.0);
            }

            if (contentRect.height() < q->size().height()) {
                speed.setY(0.0);
            }

            if (!qIsNull(speed.x()) || !qIsNull(speed.y())) {
                hasFollowOn = true;
                animateScroll(speed);
            }
        }

        // may need to bounce back if not animating the scrolling
        if (!hasFollowOn &&
            mClampingStyle == HbScrollArea::BounceBackClamping &&
            positionOutOfBounds () ) {
                if (!mBoundaryReached) {
                    HbWidgetFeedback::triggered(q, Hb::InstantBoundaryReached);
                    mBoundaryReached = true;
                }
            // display return animation if necessary
            animateScroll( QPointF(0,0) );
        } else if ( !hasFollowOn && mIsScrolling ){
            stopAnimating();
            stopScrolling();
        }
    } else if (mClampingStyle == HbScrollArea::BounceBackClamping &&
               positionOutOfBounds() ) {
        // display return animation if necessary
        if (!mBoundaryReached) {
            HbWidgetFeedback::triggered(q, Hb::InstantBoundaryReached);
            mBoundaryReached = true;
        }
        animateScroll( QPointF(0,0) );
    } else {
        if ( mIsAnimating && !positionOutOfBounds() && !mTargetAnimationInProgress ) {
            stopAnimating();
        } else if (mIsScrolling && !mTargetAnimationInProgress ) {
            stopScrolling ();
        }
    }

    event->accept();
}

void HbScrollAreaPrivate::adjustContent()
{
    Q_Q(HbScrollArea);

    updateScrollMetrics();

    QRectF scrollAreaBoundingRect = q->boundingRect();

    if (!mContents || scrollAreaBoundingRect.isNull() ||
        !scrollAreaBoundingRect.isValid())
        return;    

    QRectF contentsBoundingRect = mContents->boundingRect();
    QPointF alignedPosition = mContents->pos();
    // Set the content alignment if content size is smaller than the area
    if (!mAbleToScrollX && mAlignment & Qt::AlignHCenter) {
        alignedPosition.setX((scrollAreaBoundingRect.width() - contentsBoundingRect.width()) / 2.0);
    } else if (!mAbleToScrollX && mAlignment & Qt::AlignRight) {
         alignedPosition.setX(scrollAreaBoundingRect.width() - mContents->size().width());
    } else if ((!mAbleToScrollX && mAlignment & Qt::AlignLeft) ||
               mContents->pos().x() + contentsBoundingRect.x() > 0.0) {
        alignedPosition.setX(-contentsBoundingRect.x());
    }

    if (!mAbleToScrollY && mAlignment & Qt::AlignVCenter) {
        alignedPosition.setY((scrollAreaBoundingRect.height() - contentsBoundingRect.height()) / 2.0);
    } else if (!mAbleToScrollY && mAlignment & Qt::AlignBottom) {
        alignedPosition.setY(scrollAreaBoundingRect.height() - mContents->size().height());
    } else if ((!mAbleToScrollY && mAlignment & Qt::AlignTop) ||
               mContents->pos().y() + contentsBoundingRect.y() > 0.0) {
        alignedPosition.setY(-contentsBoundingRect.y());
    }
    if (mAbleToScrollX && (-alignedPosition.x() < leftBoundary())) {
       alignedPosition.setX(-leftBoundary());
    } else if (mAbleToScrollX && (-alignedPosition.x() > rightBoundary())) {
       alignedPosition.setX(-rightBoundary());
    }
    if (mAbleToScrollY && (-alignedPosition.y() < topBoundary())) {
        alignedPosition.setY(-topBoundary());
    } else if (mAbleToScrollY && (-alignedPosition.y() > bottomBoundary())) {
        alignedPosition.setY(-bottomBoundary());
    }
    if (alignedPosition != mContents->pos()) {
        setContentPosition(alignedPosition);
    }
    if (mContinuationIndicators) {
        updateIndicators(-mContents->pos());
    }
 }

#if 0
bool HbScrollAreaPrivate::sceneEvent(QEvent *event)
{
    Q_Q(HbScrollArea);
      if (event->type() == QEvent::GraphicsSceneGesture) {
        QGraphicsSceneGestureEvent *gestureEvent = static_cast<QGraphicsSceneGestureEvent*>(event);
        if (const QGesture *gesture = gestureEvent->gesture(Qt::PanGesture)) {
            const QPanningGesture *pannningGesture = static_cast<const QPanningGesture*>(gesture);

            if (gesture->state() == Qt::GestureStarted){
                qDebug()  << "HbScrollArea::sceneEvent: PAN EVENT -> GestureStarted";
                panDirection = pannningGesture->direction();
                mFlickTimer->restart();
            }

            if (gesture->state() == Qt::GestureUpdated){
                if (panDirection != pannningGesture->direction() ){
                    qDebug()  << "PAN EVENT -> GestureUpdated: Flick timer restart";
                    panDirection = pannningGesture->direction();
                    mFlickTimer->restart();
                }
            }

            if (gesture->state() == Qt::GestureFinished){
                if (flickTimer->elapsed() < 400 ){
                    double trueLength = sqrt(pow(gesture->pos().x(), 2) + pow(gesture->pos().y(), 2));
                    int speed  = trueLength *1000 / flickTimer->elapsed();

                    if (pannningGesture->direction() == Qt::UpDirection ){
                        qDebug() << "UP_FLICK -> Speed: " << speed;
                        q->upGesture(speed);
                    } else if (pannningGesture->direction() == Qt::DownDirection ){
                        qDebug() << "DOWN_FLICK -> Speed: " << speed;
                       q->downGesture(speed);
                    }
                    panDirection = Qt::NoDirection;
                    event->accept();
                    return true;
                }
            }
            q->panGesture(gesture->pos() - gesture->lastPos());
            event->accept();
            return true;
            }
        }
      return false;
}
#endif // HB_NEW_GESTURE_FW

void HbScrollAreaPrivate::ensureVisible(QPointF position, qreal xMargin, qreal yMargin)
{
    if(mContents) {
        Q_Q(HbScrollArea);
        QPointF currentPosition = -mContents->pos();
        QPointF newPosition = currentPosition;

        if (position.x() - xMargin < currentPosition.x()) {
            newPosition.setX(qMax(leftBoundary(), position.x() - xMargin));
        } else if (position.x() > currentPosition.x() + q->boundingRect().width() - xMargin) {
            newPosition.setX(qMin(position.x() - q->boundingRect().width() + xMargin, rightBoundary()));
        }

        if (position.y() - yMargin < currentPosition.y()) {
            newPosition.setY(qMax(topBoundary(), position.y() - yMargin));
        } else if (position.y() > currentPosition.y() + q->boundingRect().height() - yMargin) {
            newPosition.setY(qMin(position.y() - q->boundingRect().height() + yMargin, bottomBoundary()));
        }
        setContentPosition(-newPosition);
    }
}

void HbScrollAreaPrivate::startTargetAnimation (QPointF newPosition, const int time)
{
    mTargetAnimationInProgress = true;
    mTargetDelta = newPosition - (-mContents->pos());
    mElapsedTimeInAnimation = 0.0;
    mOverallTimeToTarget = qreal(time);
    mAnimationInitialPosition = -mContents->pos();
    mScrollSpeed = QPointF(0,0);

    if (!mAnimationShape) {
        mAnimationShape = new QEasingCurve(QEasingCurve::InOutCubic);
    }
    startAnimating();
}


/*
 * updateScrollMetrics() is a private function called when
 * scrolling starts to set the metrics needed in scrolling.
 */
void HbScrollAreaPrivate::updateScrollMetrics()
{
    Q_Q(HbScrollArea);

    QRectF scrollAreaBoundingRect = q->boundingRect();

    if (!mContents || scrollAreaBoundingRect.isNull() ||
         !scrollAreaBoundingRect.isValid())
        return;

    QRectF contentsBoundingRect = mContents->boundingRect();

    qreal scrollAreaWidth = scrollAreaBoundingRect.width();
    qreal scrollAreaHeight = scrollAreaBoundingRect.height();

    qreal contentsWidth = contentsBoundingRect.width();
    qreal contentsHeight = contentsBoundingRect.height();

    mAbleToScrollX = false;
    if ((mScrollDirections & Qt::Horizontal) && (contentsWidth
            > scrollAreaWidth)) {
        mAbleToScrollX = true;
    }

    mAbleToScrollY = false;
    if ((mScrollDirections & Qt::Vertical) && (contentsHeight
            > scrollAreaHeight)) {
        mAbleToScrollY = true;
    }
    prepareScrollBars();
}

void HbScrollAreaPrivate::setScrollBarMetrics(Qt::Orientation orientation)
{
    Q_Q( HbScrollArea );

    if (!mContents)
        return;

    if (orientation == Qt::Horizontal) {
        // Set handle size
        mHorizontalScrollBar->setPageSize( qBound ( qreal(0.0),
                                             qreal (q->boundingRect().width())
                                                / qreal (mContents->boundingRect().width()),
                                              qreal(1.0)));

        updateScrollBar(orientation);
    }
    else {
        // Set handle size
        mVerticalScrollBar->setPageSize(qBound ( qreal(0.0),
                                             qreal (q->boundingRect().height())
                                                / qreal (mContents->boundingRect().height()),
                                              qreal(1.0)));

        updateScrollBar(orientation);
    }
}

void HbScrollAreaPrivate::updateScrollBar(Qt::Orientation orientation)
{
    Q_Q( HbScrollArea );

    qreal thumbPosition(0);

    if ((orientation == Qt::Horizontal) && mHorizontalScrollBar) {
        // The scrollbar "thumb" position is the current position of the contents widget divided
        // by the difference between the width of the contents widget and the width of the scroll area.
        // This formula assumes that the "thumb" of the the scroll bar is sized proportionately to
        // the width of the contents widget.

        //if (mLayoutDirection == Qt::LeftToRight)
        //{
        if (!qIsNull(mContents->boundingRect().width()
            - q->boundingRect().width())) {
            thumbPosition = qAbs( leftBoundary() + qMin ( -leftBoundary(), mContents->pos().x() ) )
                            / (mContents->boundingRect().width()
                               - q->boundingRect().width());
        }
        /* }
        else
            thumbPosition = (mContents->boundingRect().width()
                    + mContents->pos().x())
                    / (mContents->boundingRect().width()
                            - q->boundingRect().width());*/

        if (thumbPosition < 0.0)
            thumbPosition = 0.0;
        else if (thumbPosition > 1.0)
            thumbPosition = 1.0;
        mHorizontalScrollBar->setValue(thumbPosition);
    }
    else if (mVerticalScrollBar) {
        // The scrollbar "thumb" position is the current position of the contents widget divided
        // by the difference between the height of the contents widget and the height of the scroll area.
        // This formula assumes that the "thumb" of the the scroll bar is sized proportionately to
        // the height of the contents widget.
        if (!qIsNull(mContents->boundingRect().height()
            - q->boundingRect().height())) {
            thumbPosition = qAbs( topBoundary() + qMin ( -topBoundary(), mContents->pos().y() ) )
                            / (mContents->boundingRect().height()
                               - q->boundingRect().height());
        }
        if (thumbPosition < 0.0)
            thumbPosition = 0.0;
        else if (thumbPosition > 1.0)
            thumbPosition = 1.0;
        mVerticalScrollBar->setValue(thumbPosition);
    }
}


void HbScrollAreaPrivate::prepareScrollBars()
{
    bool scrollBarsVisible(false);

    if ((mAbleToScrollX && mHorizontalScrollBarPolicy != HbScrollArea::ScrollBarAlwaysOff) ||
        mHorizontalScrollBarPolicy == HbScrollArea::ScrollBarAlwaysOn) {
        displayScrollBar(Qt::Horizontal);
        scrollBarsVisible = true;
    }
    else if((!mAbleToScrollX || mHorizontalScrollBarPolicy == HbScrollArea::ScrollBarAlwaysOff)
               && mHorizontalScrollBar->isVisible()){
            mHorizontalScrollBar->setVisible(false);
    }

    if ((mAbleToScrollY && mVerticalScrollBarPolicy != HbScrollArea::ScrollBarAlwaysOff) ||
        mVerticalScrollBarPolicy == HbScrollArea::ScrollBarAlwaysOn) {
        displayScrollBar(Qt::Vertical);
        scrollBarsVisible = true;
    }
    else if((!mAbleToScrollY || mVerticalScrollBarPolicy == HbScrollArea::ScrollBarAlwaysOff)
            && mVerticalScrollBar->isVisible()){
            mVerticalScrollBar->setVisible(false);
    }

    if (scrollBarsVisible && !mScrollBarHideTimer.isActive()) {
        mScrollBarHideTimer.start(INITIAL_SCROLLBAR_HIDE_TIMEOUT);
    }
}

void HbScrollAreaPrivate::displayScrollBar(Qt::Orientation orientation)
{
    Q_Q(HbScrollArea);
    if (orientation == Qt::Horizontal) {
        
        // Layout the scrollbar
        setScrollBarMetrics(orientation);
        
        // Activate the scrollbar
        if ( (!mHorizontalScrollBar->isVisible() || mOrientationChanged )&& q->isVisible()) {
            mHorizontalScrollBar->setVisible(true);
            if (mOrientationChanged ) {
                HbScrollBarPrivate::d_ptr(mHorizontalScrollBar)->startShowEffect();
            }
        }
    }
    else if (orientation == Qt::Vertical) {
        
        // Layout the scrollbar
        setScrollBarMetrics(orientation);

        // Activate the scrollbar
        if ( (!mVerticalScrollBar->isVisible() || mOrientationChanged )&& q->isVisible()) {
            mVerticalScrollBar->setVisible(true);
            if (mOrientationChanged ) {
                HbScrollBarPrivate::d_ptr(mVerticalScrollBar)->startShowEffect();
            }
        }
    }
}

void HbScrollAreaPrivate::setScrollBarPolicy(Qt::Orientation orientation, HbScrollArea::ScrollBarPolicy policy)
{
    if (orientation == Qt::Horizontal && policy != mHorizontalScrollBarPolicy) {
        mHorizontalScrollBarPolicy = policy;
    } else if (orientation == Qt::Vertical && policy != mVerticalScrollBarPolicy) {
        mVerticalScrollBarPolicy = policy;
    }
    if (mHorizontalScrollBarPolicy == HbScrollArea::ScrollBarAlwaysOff &&
        mHorizontalScrollBar->isVisible()){
        mHorizontalScrollBar->setVisible(false);
    }
    if (mVerticalScrollBarPolicy == HbScrollArea::ScrollBarAlwaysOff &&
        mVerticalScrollBar->isVisible()){
        mVerticalScrollBar->setVisible(false);
    }
    if(policy != HbScrollArea::ScrollBarAlwaysOff){
        updateScrollMetrics();
    }
}

void HbScrollAreaPrivate::_q_hideScrollBars()
{

    if (mHorizontalScrollBar && mHorizontalScrollBarPolicy == HbScrollArea::ScrollBarAutoHide) {
        if (HbScrollBarPrivate::d_ptr(mHorizontalScrollBar)->isPressed() ||
            (mVerticalScrollBar && HbScrollBarPrivate::d_ptr(mVerticalScrollBar)->isPressed())) {
            mScrollBarHideTimer.start();
        } else if(mHorizontalScrollBarPolicy != HbScrollArea::ScrollBarAlwaysOn
                  && mHorizontalScrollBar->isVisible()){
            mHorizontalScrollBar->setVisible(false);

        }
    }
    if (mVerticalScrollBar && mVerticalScrollBarPolicy == HbScrollArea::ScrollBarAutoHide) {
        if (HbScrollBarPrivate::d_ptr(mVerticalScrollBar)->isPressed() ||
            (mHorizontalScrollBar && HbScrollBarPrivate::d_ptr(mHorizontalScrollBar)->isPressed())) {
            mScrollBarHideTimer.start();
        } else if(mVerticalScrollBarPolicy != HbScrollArea::ScrollBarAlwaysOn
                  && mVerticalScrollBar->isVisible()){
            mVerticalScrollBar->setVisible(false);
        }
    }
}

/**
 *
 */
void HbScrollAreaPrivate::_q_thumbPositionChanged(qreal value, Qt::Orientation orientation)
{
    if ( mTargetAnimationInProgress ) {
        stopAnimating();
    }

    // if scrolling, just stop it.
    if (mIsAnimating) {
        if (!positionOutOfBounds()) {
            stopAnimating();
            stopScrolling();
        }
        return;
    }
    if (!mContents) {
        return;
    }

    setContentPosition(value, orientation, false);

    if (mScrollBarHideTimer.isActive()) {
        mScrollBarHideTimer.stop();
        mScrollBarHideTimer.start();
    }
    if (mContinuationIndicators) {
        updateIndicators(-mContents->pos());
    }
}

void HbScrollAreaPrivate::_q_groovePressed(qreal value, Qt::Orientation orientation)
{
    if ( mTargetAnimationInProgress ) {
        stopAnimating();
    }

    // if scrolling, just stop it.
    if (mIsAnimating) {
        if (!positionOutOfBounds()) {
            stopAnimating();
            stopScrolling();
        }
        return;
    }
    if (!mContents) {
        return;
    }

    setContentPosition(value, orientation, true);

    if (mScrollBarHideTimer.isActive()) {
        mScrollBarHideTimer.stop();
        mScrollBarHideTimer.start();
    }
    if (mContinuationIndicators) {
        updateIndicators(-mContents->pos());
    }
}

void HbScrollAreaPrivate::setContentPosition( qreal value, Qt::Orientation orientation, bool animate )
{
    Q_Q(HbScrollArea);

    QPointF newPosition = mContents->pos();
    if (orientation == Qt::Horizontal && mAbleToScrollX) {
        newPosition.setX(-((mContents->boundingRect().width() - q->boundingRect().width()) * value + mContents->boundingRect().left()));
        mScrollSpeed.setX(0.0);
    } else if (orientation == Qt::Vertical && mAbleToScrollY){
        newPosition.setY(-((mContents->boundingRect().height() - q->boundingRect().height()) * value + mContents->boundingRect().top()));
        mScrollSpeed.setY(0.0);
    }

	if (animate) {
	    q->scrollContentsTo (-newPosition, PAGE_CHANGE_DURATION);
	} else {
    	setContentPosition(newPosition);	
    }
}
	

void HbScrollAreaPrivate::createPrimitives()
{
    Q_Q(HbScrollArea);
    if (mContinuationIndicators) {
        if (continuationIndicatorTopItem) {
            delete continuationIndicatorTopItem;
        }
        continuationIndicatorTopItem =  q->style()->createPrimitive(HbStyle::P_ScrollArea_continuationtop, q);
        if (continuationIndicatorBottomItem) {
            delete continuationIndicatorBottomItem;
        }
        continuationIndicatorBottomItem =  q->style()->createPrimitive(HbStyle::P_ScrollArea_continuationbottom, q);
        if (!continuationIndicatorLeftItem) {
            delete continuationIndicatorLeftItem;
        }
        continuationIndicatorLeftItem = q->style()->createPrimitive(HbStyle::P_ScrollArea_continuationleft, q);

        if (!continuationIndicatorRightItem) {
            delete continuationIndicatorRightItem;
        }
        continuationIndicatorRightItem = q->style()->createPrimitive(HbStyle::P_ScrollArea_continuationright, q);
    }
}

void HbScrollAreaPrivate::updatePrimitives()
{
    Q_Q(HbScrollArea);
    if (mContinuationIndicators) {
        HbStyleOption option;
        q->initStyleOption(&option);
        if (continuationIndicatorTopItem) {
            q->style()->updatePrimitive(continuationIndicatorTopItem, HbStyle::P_ScrollArea_continuationtop, &option);
        }
        if (continuationIndicatorBottomItem) {
            q->style()->updatePrimitive(continuationIndicatorBottomItem, HbStyle::P_ScrollArea_continuationbottom, &option);
        }
        if (continuationIndicatorLeftItem) {
            q->style()->updatePrimitive(continuationIndicatorLeftItem, HbStyle::P_ScrollArea_continuationleft, &option);
        }
        if (continuationIndicatorRightItem) {
            q->style()->updatePrimitive(continuationIndicatorRightItem, HbStyle::P_ScrollArea_continuationright, &option);
        }
    } else {
        if (continuationIndicatorTopItem) {
            delete continuationIndicatorTopItem;
            continuationIndicatorTopItem = 0;
        }
        if (continuationIndicatorBottomItem) {
            delete continuationIndicatorBottomItem;
            continuationIndicatorBottomItem = 0;
        }
        if (continuationIndicatorLeftItem) {
            delete continuationIndicatorLeftItem;
            continuationIndicatorLeftItem = 0;
        }
        if (continuationIndicatorRightItem) {
            delete continuationIndicatorRightItem;
            continuationIndicatorRightItem = 0;
        }
    }
}

void HbScrollAreaPrivate::updateIndicators(QPointF newPosition)
{
    if (mAbleToScrollX) {
        if (newPosition.x() >= rightBoundary()) {
            continuationIndicatorRightItem->hide();
        } else {
            continuationIndicatorRightItem->show();
        }
        if (newPosition.x() <= leftBoundary()) {
            continuationIndicatorLeftItem->hide();
        } else {
            continuationIndicatorLeftItem->show();
        }
    } else {
        continuationIndicatorLeftItem->hide();
        continuationIndicatorRightItem->hide();
    }
    if (mAbleToScrollY) {
        if (newPosition.y() <= topBoundary()) {
            continuationIndicatorTopItem->hide();
        } else {
            continuationIndicatorTopItem->show();
        }
        if (newPosition.y() >= bottomBoundary()) {
            continuationIndicatorBottomItem->hide();
        } else {
            continuationIndicatorBottomItem->show();
        }
    } else {
        continuationIndicatorTopItem->hide();
        continuationIndicatorBottomItem->hide();
    }
}

void HbScrollAreaPrivate::hideChildComponents()
{
    if (continuationIndicatorTopItem) {
        continuationIndicatorTopItem->setVisible(false);
    }
    if (continuationIndicatorBottomItem) {
        continuationIndicatorBottomItem->setVisible(false);
    }
    if (continuationIndicatorLeftItem) {
        continuationIndicatorLeftItem->setVisible(false);
    }
    if (continuationIndicatorRightItem) {
        continuationIndicatorRightItem->setVisible(false);
    }
    if(mHorizontalScrollBar) {
        mHorizontalScrollBar->setVisible(false);
    }
    if(mVerticalScrollBar) {
        mVerticalScrollBar->setVisible(false);
    }
}

void HbScrollAreaPrivate::orientationChanged()
{
    mOrientationChanged = true;
    adjustContent();
    mOrientationChanged = false;
}
