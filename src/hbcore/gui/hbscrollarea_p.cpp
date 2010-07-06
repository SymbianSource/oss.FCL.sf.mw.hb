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
#include "hbstyleoption_p.h"
#include "hbinstance.h"
#include "hbpangesture.h"
#include <hbscrollbar_p.h>

#include <hbscrollbar.h>
#include <hbwidgetfeedback.h>

#include <QGraphicsSceneMouseEvent>
#include <qmath.h>
#include <QVariantAnimation>
#include <QGesture>

#include <QDebug>

// These constants control parameters of the inertia and bounce back animations and
// have been selected heuristically to produce good results.  They
// likely do not need to be controlled programatically, so they are just defined
// here.
namespace
{

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
        mHorizontalScrollBar(0),
        mVerticalScrollBar(0),
        mVerticalScrollBarPolicy(HbScrollArea::ScrollBarAutoHide),
        mHorizontalScrollBarPolicy(HbScrollArea::ScrollBarAutoHide),
        mAbleToScrollX(false),
        mAbleToScrollY(false),
        mIsScrolling(false),
        mIsAnimating(false),
        mScrollSpeed (QPointF (0,0)),
        mScrollTimerId(0),
        mScrollElapsedTime(),
        mLastElapsedTime(0.0),
        mTargetAnimationInProgress(false),
        mElapsedTimeInAnimation(0.0),
        mOverallTimeToTarget(0.0),
        mTargetDelta(QPointF(0,0)),
        mAnimationInitialPosition (QPointF(0,0)),
        mAnimationShape (0),
        mScrollBarHideTimerId(0),
        mFrictionEnabled(true),
        mResetAlignment(true),
        mClampingStyle(HbScrollArea::BounceBackClamping),
        mScrollingStyle(HbScrollArea::PanWithFollowOn),
        mEventPositionQueueSize(10),
        mEventPositionQueueLastIndex(0),
        mEventPositionQueueIsFull(false),
        mLayoutDirection(Qt::LeftToRight),
        mAlignment(Qt::AlignLeft | Qt::AlignTop),
        mContinuationIndicators(false),
        mEmitPositionChangedSignal(false),
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
    delete mAnimationShape;
}

void HbScrollAreaPrivate::init()
{
    Q_Q( HbScrollArea );

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

#if QT_VERSION >= 0x040700
    q->grabGesture(Qt::PanGesture, Qt::ReceivePartialGestures | Qt::IgnoredGesturesPropagateToParent);
#else
    q->grabGesture(Qt::PanGesture, Qt::ReceivePartialGestures);
#endif
    q->grabGesture(Qt::TapGesture);

}

void HbScrollAreaPrivate::doLazyInit()
{
    Q_Q( HbScrollArea );
    QObject::connect(mVerticalScrollBar, SIGNAL(valueChanged(qreal, Qt::Orientation)), q, SLOT(_q_thumbPositionChanged(qreal, Qt::Orientation)));
    QObject::connect(mVerticalScrollBar, SIGNAL(valueChangeRequested(qreal, Qt::Orientation)), q, SLOT(_q_groovePressed(qreal, Qt::Orientation)));
    QObject::connect(&HbScrollBarPrivate::d_ptr(mVerticalScrollBar)->core, SIGNAL(handlePressed()), q, SLOT(_q_thumbPressed()));
    QObject::connect(&HbScrollBarPrivate::d_ptr(mVerticalScrollBar)->core, SIGNAL(handleReleased()), q, SLOT(_q_thumbReleased()));
    QObject::connect(&HbScrollBarPrivate::d_ptr(mHorizontalScrollBar)->core, SIGNAL(handlePressed()), q, SLOT(_q_thumbPressed()));
    QObject::connect(&HbScrollBarPrivate::d_ptr(mHorizontalScrollBar)->core, SIGNAL(handleReleased()), q, SLOT(_q_thumbReleased()));
    QObject::connect(mHorizontalScrollBar, SIGNAL(valueChanged(qreal, Qt::Orientation)), q, SLOT(_q_thumbPositionChanged(qreal, Qt::Orientation)));
    QObject::connect(mHorizontalScrollBar, SIGNAL(valueChangeRequested(qreal, Qt::Orientation)), q, SLOT(_q_groovePressed(qreal, Qt::Orientation)));


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
    Q_Q(HbScrollArea);
    mLayoutDirection = aNewDirection;
    Qt::Alignment currentAlignment = mAlignment;
    if ((mAlignment & Qt::AlignLeft) | (mAlignment & Qt::AlignRight)) {
        currentAlignment ^= Qt::AlignLeft;
        currentAlignment ^= Qt::AlignRight;        
    }
    q->setAlignment(currentAlignment);
}

bool HbScrollAreaPrivate::scrollByAmount(const QPointF& delta)
{
    Q_Q( HbScrollArea );

    if ( qIsNull(qAbs( delta.y() )) && qIsNull(qAbs( delta.x() ) ) )
        return false;

    QPointF currentPosition = -mContents->pos();
    QPointF newPosition = currentPosition;
    qreal newXPosition;
    qreal newYPosition;

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
        stopScrolling(); 
        // must return here, otherwise mIsScrolling will be false
        // below and scrollingStarted will be emitted
        return currentPosition != newPosition;
    } else {
        mBoundaryReached = false;
    }

    setContentPosition(-newPosition);
    //Its important to setcontents position first as thats used in updateScrollbar.displayScrollbar calls
    //updateScrollbar internally.Other wise we need to update scrollbars twice with different positions.
    if (!mIsScrolling) {
        if (mAbleToScrollX || mAbleToScrollY) {
            mIsScrolling = true;
            emit q->scrollingStarted();
        }
        if (mAbleToScrollX && mHorizontalScrollBarPolicy == HbScrollArea::ScrollBarAutoHide) {
            displayScrollBar(Qt::Horizontal);
        }

        if (mAbleToScrollY && mVerticalScrollBarPolicy == HbScrollArea::ScrollBarAutoHide) {
            displayScrollBar(Qt::Vertical);
        }
        stopScrollbarHideTimer();

    } else {

        if (mAbleToScrollX && mHorizontalScrollBar->isVisible()) {
            updateScrollBar(Qt::Horizontal);
        }

        if (mAbleToScrollY && mVerticalScrollBar->isVisible()) {
            updateScrollBar(Qt::Vertical);
        }
    }
    //if we are here than content position has changed.i.e delta is not zero or
    //we havent reached the boundaries.
    return true;
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
    if (!mContents)
        return;
    if (mScrollTimerId == 0) {
        Q_Q( HbScrollArea );
        mScrollTimerId = q->startTimer(ANIMATION_INTERVAL);

        //make sure we where able to start the timer
        if (mScrollTimerId != 0) {
            mScrollElapsedTime.restart();
            mLastElapsedTime = 0.0;
            mIsAnimating = true;
        }
    }
}

void HbScrollAreaPrivate::stopScrolling()
{
    Q_Q( HbScrollArea );
    if (mHorizontalScrollBarPolicy == HbScrollArea::ScrollBarAutoHide ||
        mVerticalScrollBarPolicy == HbScrollArea::ScrollBarAutoHide) {
        startScrollbarHideTimer();
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
    if (mIsAnimating) {
        Q_Q( HbScrollArea );
        q->killTimer(mScrollTimerId);
        mScrollTimerId = 0;
        mIsAnimating = false;
        stopScrolling();
    }
}

void HbScrollAreaPrivate::startScrollbarHideTimer()
{
    if (mScrollBarHideTimerId == 0){
        Q_Q( HbScrollArea );
        mScrollBarHideTimerId = q->startTimer(INITIAL_SCROLLBAR_HIDE_TIMEOUT);
    }

}

void HbScrollAreaPrivate::stopScrollbarHideTimer()
{
    if (mScrollBarHideTimerId != 0) {
        Q_Q( HbScrollArea );
        q->killTimer(mScrollBarHideTimerId);
        mScrollBarHideTimerId = 0;
    }
}

void HbScrollAreaPrivate::reStartScrollbarHideTimer()
{
  stopScrollbarHideTimer();
  startScrollbarHideTimer();
}

/*
 * animateScrollTimeout() is a private slot function called during
 * follow-on scroll animation
 */
void HbScrollAreaPrivate::_q_animateScrollTimeout()
{
    Q_Q( HbScrollArea );
    qreal elapsedTime(qreal(mScrollElapsedTime.elapsed()));

    qreal timeDifference(elapsedTime - mLastElapsedTime);

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
        qreal simulationStepInTime;
        QPointF overAllDist(0,0);
        QPointF currentPosition(-mContents->pos());
        bool stopVerticalAnimation(false);
        bool stopHorizontalAnimation(false);
        // calculations are split so that the same amount of simulation steps
        // are made in varying framerates.
        do {
            simulationStepInTime = qMin ( MAX_TIMEDIF_FOR_SIMULATION, timeDifference);
            currentPosition = -mContents->pos();
            if (mAbleToScrollX) {
                // Calculate new velocity to horizontal movement
                mScrollSpeed.setX( calculateVelocity ( simulationStepInTime,
                                                       mScrollSpeed.x(),
                                                       currentPosition.x(),
                                                       leftBoundary(),
                                                       rightBoundary() ) );

                if ( qAbs( mScrollSpeed.x() ) < MIN_SPEED_PER_MILLISECOND ) {
                    stopHorizontalAnimation = clampToBoundaries ( overAllDist.rx(), currentPosition.x(), leftBoundary(), rightBoundary() );
                }                
            }
            if (mAbleToScrollY) {
                // Calculate new velocity to vertical movement
                mScrollSpeed.setY( calculateVelocity ( simulationStepInTime,
                                                       mScrollSpeed.y(),
                                                       currentPosition.y(),
                                                       topBoundary(),
                                                       bottomBoundary() ) );
                if ( qAbs( mScrollSpeed.y() ) < MIN_SPEED_PER_MILLISECOND ) {
                    stopVerticalAnimation = clampToBoundaries ( overAllDist.ry(), currentPosition.y(), topBoundary(),bottomBoundary() );
                }
            }
            overAllDist = mScrollSpeed * simulationStepInTime;
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
     if (mEmitPositionChangedSignal)
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


/*!
 * Pans the scroll area
 */
bool HbScrollAreaPrivate::pan(QPanGesture* panGesture)
{
    if (!mContents)
        return false;
    Q_Q ( HbScrollArea );

    HbPanGesture *hbPanGesture = qobject_cast<HbPanGesture *>(panGesture);

    if (hbPanGesture) {
        if(hbPanGesture->state() == Qt::GestureUpdated) {

            if (mIsAnimating) stopAnimating();

            QPointF delta(hbPanGesture->sceneDelta());

            // Panning against the bounceback spring
            if (mClampingStyle == HbScrollArea::BounceBackClamping) {
                if (-mContents->pos().y() < topBoundary() && delta.y() > 0.0f) {
                    delta.setY(delta.y() / 2);
                } else if (-mContents->pos().y() > bottomBoundary() && delta.y() < 0.0f) {
                    delta.setY(delta.y() / 2);
                }
                if (-mContents->pos().x() < leftBoundary() && delta.x() > 0.0f) {
                    delta.setX(delta.x() / 2);
                } else if (-mContents->pos().x() > rightBoundary() && delta.x() < 0.0f) {
                    delta.setX(delta.x() / 2);
                }
            }
            QPointF oldPos = mContents->pos();
            q->scrollByAmount(-delta);
            if (mContents->pos() == oldPos) {
                return false;
            }

        } else if (panGesture->state() == Qt::GestureFinished) {
            if ((!mAbleToScrollX && !mAbleToScrollY) || mBoundaryReached)
                return false;

            if (mScrollingStyle == HbScrollArea::PanWithFollowOn && hbPanGesture) {
                animateScroll(hbPanGesture->sceneVelocity());
            } else if (mScrollingStyle == HbScrollArea::Pan && positionOutOfBounds()) {
                animateScroll(QPointF(0,0));
            } else {
                stopAnimating();
            }
        }
    }
    return true;
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
    bool xAlignmentNeeded = (!mAbleToScrollX && ((-alignedPosition.x() < leftBoundary()) ||
                                                 -alignedPosition.x() > rightBoundary()));
    bool yAlignmentNeeded = (!mAbleToScrollY && ((-alignedPosition.y() < topBoundary()) ||
                                                 -alignedPosition.y() > bottomBoundary()));
    if (mResetAlignment || xAlignmentNeeded || yAlignmentNeeded) {
        if (mAlignment & Qt::AlignHCenter) {
            alignedPosition.setX((scrollAreaBoundingRect.width() - contentsBoundingRect.width()) / 2.0);
        } else if (mAlignment & Qt::AlignRight) {
            alignedPosition.setX(scrollAreaBoundingRect.width() - mContents->size().width());
        } else if ((mAlignment & Qt::AlignLeft) ||
                   mContents->pos().x() + contentsBoundingRect.x() > 0.0) {
            alignedPosition.setX(-contentsBoundingRect.x());
        }

        if (mAlignment & Qt::AlignVCenter) {
            alignedPosition.setY((scrollAreaBoundingRect.height() - contentsBoundingRect.height()) / 2.0);
        } else if (mAlignment & Qt::AlignBottom) {
            alignedPosition.setY(scrollAreaBoundingRect.height() - mContents->size().height());
        } else if ((mAlignment & Qt::AlignTop) ||
                   mContents->pos().y() + contentsBoundingRect.y() > 0.0) {
            alignedPosition.setY(-contentsBoundingRect.y());
        }
    }
    mResetAlignment = false;
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

void HbScrollAreaPrivate::ensureVisible(QPointF position, qreal xMargin, qreal yMargin)
{
    if(mContents) {
        Q_Q(HbScrollArea);
        // if contentwidget size has changed, xxxBoundary() functions
        // won't work without this
        QCoreApplication::sendPostedEvents(q, QEvent::LayoutRequest);

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
        mHorizontalScrollBar->setPageSize(q->boundingRect().width()
                                          / mContents->boundingRect().width());
        updateScrollBar(orientation);
    }
    else {
        // Set handle size
        mVerticalScrollBar->setPageSize(q->boundingRect().height()
                                        / mContents->boundingRect().height());
        updateScrollBar(orientation);
    }
}

void HbScrollAreaPrivate::updateScrollBar(Qt::Orientation orientation)
{
    Q_Q( HbScrollArea );

    qreal thumbPosition(0);
    qreal normalizingValue(0);
    // The scrollbar "thumb" position is the current position of the contents widget divided
    // by the difference between the width of the contents widget and the width of the scroll area.
    // This formula assumes that the "thumb" of the the scroll bar is sized proportionately to
    // the width of the contents widget.
    if ((orientation == Qt::Horizontal) && mHorizontalScrollBar) {
        if (mAbleToScrollX) {
            normalizingValue = mContents->boundingRect().width()
                               - q->boundingRect().width();
            qreal lBoundary(leftBoundary());
            if (!qIsNull(normalizingValue)) {
                thumbPosition = qAbs( lBoundary + qMin ( -lBoundary, mContents->pos().x() ) )
                                / (normalizingValue);
            }
        }
        mHorizontalScrollBar->setValue(thumbPosition);
    }
    else if (mVerticalScrollBar) {
        if (mAbleToScrollY) {
            normalizingValue = mContents->boundingRect().height()
                               - q->boundingRect().height();
            qreal tBoundary(topBoundary());
            if (!qIsNull(normalizingValue)) {
                thumbPosition = qAbs( tBoundary + qMin ( -tBoundary, mContents->pos().y() ) )
                                / (normalizingValue);
            } 
        }
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

    if (scrollBarsVisible) {
        startScrollbarHideTimer();
    }
}

void HbScrollAreaPrivate::displayScrollBar(Qt::Orientation orientation)
{
    Q_Q(HbScrollArea);
    if (orientation == Qt::Horizontal) {

        // Layout the scrollbar
        setScrollBarMetrics(orientation);

        // Activate the scrollbar
        if ( !mHorizontalScrollBar->isVisible() && q->isVisible()) {
            mHorizontalScrollBar->setVisible(true);
        }
    }
    else if (orientation == Qt::Vertical) {

        // Layout the scrollbar
        setScrollBarMetrics(orientation);

        // Activate the scrollbar
        if ( !mVerticalScrollBar->isVisible() && q->isVisible()) {
            mVerticalScrollBar->setVisible(true);
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
            startScrollbarHideTimer();
        } else if(mHorizontalScrollBarPolicy != HbScrollArea::ScrollBarAlwaysOn
                  && mHorizontalScrollBar->isVisible()){
            stopScrollbarHideTimer();
            mHorizontalScrollBar->setVisible(false);

        }
    }
    if (mVerticalScrollBar && mVerticalScrollBarPolicy == HbScrollArea::ScrollBarAutoHide) {
        if (HbScrollBarPrivate::d_ptr(mVerticalScrollBar)->isPressed() ||
            (mHorizontalScrollBar && HbScrollBarPrivate::d_ptr(mHorizontalScrollBar)->isPressed())) {
            startScrollbarHideTimer();
        } else if(mVerticalScrollBarPolicy != HbScrollArea::ScrollBarAlwaysOn
                  && mVerticalScrollBar->isVisible()){
            stopScrollbarHideTimer();
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

    reStartScrollbarHideTimer();
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

    reStartScrollbarHideTimer();
    if (mContinuationIndicators) {
        updateIndicators(-mContents->pos());
    }
}

void HbScrollAreaPrivate::_q_thumbPressed()
{
    Q_Q(HbScrollArea);
    mIsScrolling = true;
    emit q->scrollingStarted();
}

void HbScrollAreaPrivate::_q_thumbReleased()
{
    Q_Q(HbScrollArea);
    mIsScrolling = false;
    emit q->scrollingEnded();
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
        q->scrollContentsTo(-newPosition, PAGE_CHANGE_DURATION);
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
