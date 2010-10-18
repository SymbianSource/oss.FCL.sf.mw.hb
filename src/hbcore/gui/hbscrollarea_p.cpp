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
#include "hbinstance.h"
#include "hbpangesture.h"
#include <hbscrollbar_p.h>

#include <hbscrollbar.h>
#include <hbwidgetfeedback.h>
#include <hbstyleframeprimitivedata.h>

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

const qreal MAX_TIMEDIF_FOR_SIMULATION = 16.0;

static const int INITIAL_SCROLLBAR_HIDE_TIMEOUT = 4000;

static const int PAGE_CHANGE_DURATION = 170;

//keep this half of swapbuffer time so that we dont end up doing too many updates
static const int ANIMATION_INTERVAL = 8;

static const int FRAMEUPDATE_THRESHOLD = 32;
}

HbScrollAreaPrivate::HbScrollAreaPrivate(HbScrollAreaPrivate::ScrollBarsUsed scrollBarsUsed) :
        mScrollFeedbackOngoing(false),
        mBoundaryReached(false),
        mContents(0),
        mScrollDirections(Qt::Vertical),
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
        mTargetAnimationInProgress(false),
        mElapsedTimeInAnimation(0.0),
        mOverallTimeToTarget(0.0),
        mTargetDelta(QPointF(0,0)),
        mAnimationInitialPosition (QPointF(0,0)),
        mAnimationShape (0),
        mScrollBarHideTimerId(0),
        mFrictionEnabled(true),
        mResetAlignment(true),
        mScrollBarsUsed(scrollBarsUsed),
        mClampingStyle(HbScrollArea::BounceBackClamping),
        mScrollingStyle(HbScrollArea::PanWithFollowOn),
        mLayoutDirection(Qt::LeftToRight),
        mAlignment(Qt::AlignLeft | Qt::AlignTop),
        mContinuationIndicators(false),
        mEmitPositionChangedSignal(false),
        mClearCachedRect(true),
        mLazyInitDone(false),
        continuationIndicatorTopItem(0),
        continuationIndicatorBottomItem(0),
        continuationIndicatorLeftItem(0),
        continuationIndicatorRightItem(0),
        // These values can be tweaked with the private access from Fute application
        mSpeedFactor( SCROLLSPEED_FACTOR ),
        mInertiaSpeedFactor( INERTIA_SPEEDFACTOR ),
        mMaxScrollSpeed( MAX_SCROLLSPEED_PER_MILLISECOND ),
        mSpringStrength( SPRING_STRENGTH ),
        mSpringDampingFactor( SPRING_DAMPING_FACTOR ),
        mFrictionPerMilliSecond( FRICTION_PER_MILLISECOND ),
        mMultiFlickEnabled(false)
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
    q->setFlag(QGraphicsItem::ItemHasNoContents, true);
    q->setFocusPolicy(Qt::StrongFocus);

#if QT_VERSION >= 0x040700
    q->grabGesture(Qt::PanGesture, Qt::ReceivePartialGestures | Qt::IgnoredGesturesPropagateToParent);
#else
    q->grabGesture(Qt::PanGesture, Qt::ReceivePartialGestures);
#endif
    q->grabGesture(Qt::TapGesture);

    if (mScrollBarsUsed == HbScrollAreaPrivate::NoScrollBars) {
        mVerticalScrollBar = 0;
        mHorizontalScrollBar = 0;        
    }
    if (mScrollBarsUsed & HbScrollAreaPrivate::VerticalScrollBar) {
        createScrollBars(Qt::Vertical);
    }
    if (mScrollBarsUsed & HbScrollAreaPrivate::HorizontalScrollBar) {
        createScrollBars(Qt::Horizontal);
    }
}

void HbScrollAreaPrivate::createScrollBars(Qt::Orientation scrollbar)
{
    Q_Q( HbScrollArea );
    if (scrollbar == Qt::Vertical) {
        // Scrollbar is created when it is shown for the first time.
        mVerticalScrollBar = new HbScrollBar(Qt::Vertical, q);
        HbStyle::setItemName(mVerticalScrollBar, "scrollbar-vertical");
        // make sure the scrollbar is on top
        mVerticalScrollBar->setZValue(q->zValue() + 1);
        mVerticalScrollBar->setVisible(false);
    } else {
        // Scrollbar is created when it is shown for the first time.
        mHorizontalScrollBar = new HbScrollBar(Qt::Horizontal, q);
        HbStyle::setItemName(mHorizontalScrollBar, "scrollbar-horizontal");
        mHorizontalScrollBar->setVisible(false);
        // make sure the scrollbar is on top
        mHorizontalScrollBar->setZValue(q->zValue() + 1);
    }
}

void HbScrollAreaPrivate::doLazyInit()
{
    if (!mLazyInitDone) {
        Q_Q( HbScrollArea );
        if (mVerticalScrollBar) {
            QObject::connect(mVerticalScrollBar, SIGNAL(valueChanged(qreal, Qt::Orientation)), q, SLOT(_q_thumbPositionChanged(qreal, Qt::Orientation)));
            QObject::connect(mVerticalScrollBar, SIGNAL(valueChangeRequested(qreal, Qt::Orientation)), q, SLOT(_q_groovePressed(qreal, Qt::Orientation)));
            QObject::connect(&HbScrollBarPrivate::d_ptr(mVerticalScrollBar)->core, SIGNAL(handlePressed()), q, SLOT(_q_thumbPressed()));
            QObject::connect(&HbScrollBarPrivate::d_ptr(mVerticalScrollBar)->core, SIGNAL(handleReleased()), q, SLOT(_q_thumbReleased()));
            if (mVerticalScrollBar->isVisible())
                HbScrollBarPrivate::d_ptr(mVerticalScrollBar)->loadEffects();
        }

        if (mHorizontalScrollBar) {
            QObject::connect(&HbScrollBarPrivate::d_ptr(mHorizontalScrollBar)->core, SIGNAL(handlePressed()), q, SLOT(_q_thumbPressed()));
            QObject::connect(&HbScrollBarPrivate::d_ptr(mHorizontalScrollBar)->core, SIGNAL(handleReleased()), q, SLOT(_q_thumbReleased()));
            QObject::connect(mHorizontalScrollBar, SIGNAL(valueChanged(qreal, Qt::Orientation)), q, SLOT(_q_thumbPositionChanged(qreal, Qt::Orientation)));
            QObject::connect(mHorizontalScrollBar, SIGNAL(valueChangeRequested(qreal, Qt::Orientation)), q, SLOT(_q_groovePressed(qreal, Qt::Orientation)));
            if (mHorizontalScrollBar->isVisible())
                HbScrollBarPrivate::d_ptr(mHorizontalScrollBar)->loadEffects();
        }
        mLazyInitDone = true;
    }
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

    if ( mContents == 0 ||
         ( qIsNull(qAbs( delta.x() )) && qIsNull(qAbs( delta.y() ) ) ) )
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
        bool scrolled = currentPosition != newPosition;
        if (scrolled) {
            setContentPosition(-newPosition);

            if (mAbleToScrollX && mHorizontalScrollBar && mHorizontalScrollBar->isVisible()) {
                updateScrollBar(Qt::Horizontal);
            }
            if (mAbleToScrollY && mVerticalScrollBar && mVerticalScrollBar->isVisible()) {
                updateScrollBar(Qt::Vertical);
            }
        }

        // if animating stopScrolling will be called from _q_animateScrollTimeout
        if (!mIsAnimating) {
            stopScrolling();
        }

        // must return here, otherwise mIsScrolling will be false
        // below and scrollingStarted will be emitted
        return scrolled;
    } else {
        mBoundaryReached = false;
    }

    bool scrollingStarted = false;
    if (!mIsScrolling && (mAbleToScrollX || mAbleToScrollY)) {
        mIsScrolling = true;
        scrollingStarted = true;
        emit q->scrollingStarted();
    }

    setContentPosition(-newPosition);
    //Its important to setcontents position first as thats used in updateScrollbar.displayScrollbar calls
    //updateScrollbar internally.Other wise we need to update scrollbars twice with different positions.
    if (!mIsScrolling || scrollingStarted) {
        if (mAbleToScrollX && mHorizontalScrollBarPolicy == HbScrollArea::ScrollBarAutoHide) {
            displayScrollBar(Qt::Horizontal);
        }

        if (mAbleToScrollY && mVerticalScrollBarPolicy == HbScrollArea::ScrollBarAutoHide) {
            displayScrollBar(Qt::Vertical);
        }
        stopScrollbarHideTimer();
    }

    if (mAbleToScrollX && mHorizontalScrollBar && mHorizontalScrollBar->isVisible()) {
        updateScrollBar(Qt::Horizontal);
    }

    //if we are here than content position has changed.i.e delta is not zero or
    if (mAbleToScrollY && mVerticalScrollBar && mVerticalScrollBar->isVisible()) {
        updateScrollBar(Qt::Vertical);
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
            mIsAnimating = true;
        }
        resetFrameVariables();
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
    if (mScrollBarHideTimerId == 0 && (mVerticalScrollBar || mHorizontalScrollBar)) {
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

void HbScrollAreaPrivate::resetFrameVariables()
{
    mUnderFramCount = 0;
    mOverFramCount = 0;
}


qreal HbScrollAreaPrivate::timeDifferenceBetweenFrame(qreal difference)
{
    if (difference<MAX_TIMEDIF_FOR_SIMULATION) {
        mOverFramCount = 0;
        mUnderFramCount++;
        if(mUnderFramCount > FRAMEUPDATE_THRESHOLD ) {
            mUnderFramCount = FRAMEUPDATE_THRESHOLD;
            //To have smooth velocity during scrolling avg the ideal time with the
            //actual frame time.
            return qCeil((difference+MAX_TIMEDIF_FOR_SIMULATION)/2);
        }
        else {
            return MAX_TIMEDIF_FOR_SIMULATION;
        }
    } else if (difference>FRAMEUPDATE_THRESHOLD) {
        mUnderFramCount = 0;
        mOverFramCount++;
        if(mOverFramCount > FRAMEUPDATE_THRESHOLD) {
            mOverFramCount = FRAMEUPDATE_THRESHOLD;
            //To have smooth velocity during scrolling avg the ideal time with the
            //actual frame time.
                return qCeil((qreal(difference)+MAX_TIMEDIF_FOR_SIMULATION)/2);
        }
        else {
            return MAX_TIMEDIF_FOR_SIMULATION;
        }
    }
    mUnderFramCount = 0;
    mOverFramCount = 0;
    return MAX_TIMEDIF_FOR_SIMULATION;
}
/*
 * animateScrollTimeout() is a private slot function called during
 * follow-on scroll animation
 */
void HbScrollAreaPrivate::_q_animateScrollTimeout()
{
    Q_Q( HbScrollArea );
    qreal timeDifference(mScrollElapsedTime.elapsed());

    // if no time has elapsed, just return
    // this sometimes happens if the timer accuracy is not that great
    if (qIsNull(timeDifference)) {
        return;
    }


    timeDifference = timeDifferenceBetweenFrame(timeDifference);

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
            }
            if (mAbleToScrollY) {
                // Calculate new velocity to vertical movement
                mScrollSpeed.setY( calculateVelocity ( simulationStepInTime,
                                                       mScrollSpeed.y(),
                                                       currentPosition.y(),
                                                       topBoundary(),
                                                       bottomBoundary() ) );
            }
            overAllDist = mScrollSpeed * simulationStepInTime;
            q->scrollByAmount ( -overAllDist );
            if ( mAbleToScrollY && (qAbs( mScrollSpeed.y() ) < MIN_SPEED_PER_MILLISECOND )) {
                stopVerticalAnimation = clampToBoundaries ( overAllDist.ry(), currentPosition.y(), topBoundary(),bottomBoundary() );
            }

            if ( mAbleToScrollX && (qAbs( mScrollSpeed.x() ) < MIN_SPEED_PER_MILLISECOND )) {
                stopHorizontalAnimation = clampToBoundaries ( overAllDist.rx(), currentPosition.x(), leftBoundary(), rightBoundary() );
            }

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
    mScrollElapsedTime.restart();
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
        if(hbPanGesture->state() == Qt::GestureUpdated || hbPanGesture->state() == Qt::GestureFinished) {

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
            if (mContents->pos() == oldPos && hbPanGesture->state() != Qt::GestureFinished) {
                return false;
            }

        }

        if (panGesture->state() == Qt::GestureFinished) {
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
    bool xAlignmentNeeded = mResetAlignment || (!mAbleToScrollX && ((-alignedPosition.x() < leftBoundary()) ||
                                                 -alignedPosition.x() > rightBoundary()));
    bool yAlignmentNeeded = mResetAlignment || (!mAbleToScrollY && ((-alignedPosition.y() < topBoundary()) ||
                                                 -alignedPosition.y() > bottomBoundary()));
    if (xAlignmentNeeded) {
        if (mAlignment & Qt::AlignHCenter) {
            alignedPosition.setX((scrollAreaBoundingRect.width() - contentsBoundingRect.width()) / 2.0);
        } else if (mAlignment & Qt::AlignRight) {
            alignedPosition.setX(scrollAreaBoundingRect.width() - mContents->size().width());
        } else if ((mAlignment & Qt::AlignLeft) ||
                   mContents->pos().x() + contentsBoundingRect.x() > 0.0) {
            alignedPosition.setX(-contentsBoundingRect.x());
        }
    }
    if (yAlignmentNeeded) {
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
        if (mAbleToScrollX && mHorizontalScrollBar && mHorizontalScrollBar->isVisible()) {
            updateScrollBar(Qt::Horizontal);
        }
        if (mAbleToScrollY && mVerticalScrollBar && mVerticalScrollBar->isVisible()) {
            updateScrollBar(Qt::Vertical);
        }
    }
    if (mContinuationIndicators) {
        updateIndicators(-mContents->pos());
    }
}

void HbScrollAreaPrivate::ensureVisible(const QPointF &position, qreal xMargin, qreal yMargin)
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

void HbScrollAreaPrivate::startTargetAnimation (const QPointF &newPosition, const int time)
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
            > scrollAreaWidth + 1)) {
        mAbleToScrollX = true;
    }

    mAbleToScrollY = false;
    if ((mScrollDirections & Qt::Vertical) && (contentsHeight
            > scrollAreaHeight + 1)) {
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
        if (mHorizontalScrollBar) {
            mHorizontalScrollBar->setPageSize(q->boundingRect().width()
                                              / mContents->boundingRect().width());
            updateScrollBar(orientation);
        }
    } else if(mVerticalScrollBar) {
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
    QRectF contentRect = mContents->boundingRect();
    // The scrollbar "thumb" position is the current position of the
    // contents widget divided by the difference between the width of
    // the contents widget and the width of the scroll area.
    // This formula assumes that the "thumb" of the the scroll bar is
    // sized proportionately to the width of the contents widget.
    if (orientation == Qt::Horizontal) {
        if (mAbleToScrollX && mHorizontalScrollBar) {
            normalizingValue = contentRect.width() - q->boundingRect().width();
            if (!qIsNull(normalizingValue)) {
                thumbPosition = qAbs(contentRect.left() + 
                                     qMin(-contentRect.left(), mContents->pos().x()) )
                    / normalizingValue;
            }
            mHorizontalScrollBar->setValue(thumbPosition);
        }

    } else { // vertical scroll bar

        if (mVerticalScrollBar && mAbleToScrollY) {
            normalizingValue = contentRect.height() - q->boundingRect().height();
            if (!qIsNull(normalizingValue)) {
                thumbPosition = qAbs(contentRect.top() +
                                     qMin(-contentRect.top(), mContents->pos().y()) )
                    / normalizingValue;
            }
            mVerticalScrollBar->setValue(thumbPosition);
        }
    }
}

void HbScrollAreaPrivate::prepareScrollBars()
{
    bool scrollBarsVisible(false);

    if (mHorizontalScrollBar) {
        if ((mAbleToScrollX && mHorizontalScrollBarPolicy != HbScrollArea::ScrollBarAlwaysOff) ||
            mHorizontalScrollBarPolicy == HbScrollArea::ScrollBarAlwaysOn) {
            displayScrollBar(Qt::Horizontal);
            scrollBarsVisible = true;
        } else if((!mAbleToScrollX || mHorizontalScrollBarPolicy == HbScrollArea::ScrollBarAlwaysOff)
            && mHorizontalScrollBar->isVisible()){
            mHorizontalScrollBar->setVisible(false);
        }
    }

    if (mVerticalScrollBar) {
        if ((mAbleToScrollY && mVerticalScrollBarPolicy != HbScrollArea::ScrollBarAlwaysOff) ||
            mVerticalScrollBarPolicy == HbScrollArea::ScrollBarAlwaysOn) {
            displayScrollBar(Qt::Vertical);
            scrollBarsVisible = true;
        } else if((!mAbleToScrollY || mVerticalScrollBarPolicy == HbScrollArea::ScrollBarAlwaysOff)
            && mVerticalScrollBar->isVisible()){
            mVerticalScrollBar->setVisible(false);
        }
    }

    if (scrollBarsVisible) {
        startScrollbarHideTimer();
    }
}

void HbScrollAreaPrivate::displayScrollBar(Qt::Orientation orientation)
{
    Q_Q(HbScrollArea);

    // Layout the scrollbar
    setScrollBarMetrics(orientation);

    if (orientation == Qt::Horizontal) {
        // Activate the scrollbar
        if ( mHorizontalScrollBar && !mHorizontalScrollBar->isVisible() && q->isVisible()) {
            mHorizontalScrollBar->setVisible(true);
        }
    }
    else if (orientation == Qt::Vertical) {
        // Activate the scrollbar
        if ( mVerticalScrollBar && !mVerticalScrollBar->isVisible() && q->isVisible()) {
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
    if (mHorizontalScrollBar &&  mHorizontalScrollBarPolicy == HbScrollArea::ScrollBarAlwaysOff &&
        mHorizontalScrollBar->isVisible()){
        mHorizontalScrollBar->setVisible(false);
    }
    if (mVerticalScrollBar && mVerticalScrollBarPolicy == HbScrollArea::ScrollBarAlwaysOff &&
        mVerticalScrollBar->isVisible()){
        mVerticalScrollBar->setVisible(false);
    }
    if(policy != HbScrollArea::ScrollBarAlwaysOff){
        updateScrollMetrics();
    }
}

void HbScrollAreaPrivate::_q_hideScrollBars()
{
    if (mHorizontalScrollBar) {
        if (mHorizontalScrollBarPolicy == HbScrollArea::ScrollBarAutoHide) {
            if (HbScrollBarPrivate::d_ptr(mHorizontalScrollBar)->isPressed() ||
                (mVerticalScrollBar && HbScrollBarPrivate::d_ptr(mVerticalScrollBar)->isPressed())) {
                startScrollbarHideTimer();
            } else if(mHorizontalScrollBarPolicy != HbScrollArea::ScrollBarAlwaysOn){
                stopScrollbarHideTimer();
                mHorizontalScrollBar->setVisible(false);
            }
        }
    }
    if (mVerticalScrollBar) {
        if (mVerticalScrollBarPolicy == HbScrollArea::ScrollBarAutoHide) {
            if (HbScrollBarPrivate::d_ptr(mVerticalScrollBar)->isPressed() ||
                (mHorizontalScrollBar && HbScrollBarPrivate::d_ptr(mHorizontalScrollBar)->isPressed())) {
                startScrollbarHideTimer();
            } else if(mVerticalScrollBarPolicy != HbScrollArea::ScrollBarAlwaysOn){
                stopScrollbarHideTimer();
                mVerticalScrollBar->setVisible(false);
            }
        }
    }
}

void HbScrollAreaPrivate::_q_thumbPositionChanged(qreal value, Qt::Orientation orientation)
{
    Q_Q( HbScrollArea );
    // TODO Making syncronous asynhronous...Qt::QueuedConnection in signal slot connection would have done the job. 
    // Doing it requires Qt::Orientation to be registered as meta type. When done, handling Qt::Orientation as 
    // QMetaEnum is broken. When fixed by Qt, this workaround can be replaced by Qt::QueuedConnection.
    QMetaObject::invokeMethod(q, "_q_thumbPositionChanged2", Qt::QueuedConnection, Q_ARG(qreal, value), Q_ARG(int, orientation));
}

/**
 *
 */
void HbScrollAreaPrivate::_q_thumbPositionChanged2(qreal value, int orientation)
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

    setContentPosition(value, (Qt::Orientation)orientation, false);

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

        continuationIndicatorTopItem = q->style()->createPrimitive(HbStyle::PT_FrameItem, "continuation-indicator-top", q);
        continuationIndicatorTopItem->setZValue(-1);

        if (continuationIndicatorBottomItem) {
            delete continuationIndicatorBottomItem;
        }

        continuationIndicatorBottomItem = q->style()->createPrimitive(HbStyle::PT_FrameItem, "continuation-indicator-bottom", q);
        continuationIndicatorBottomItem->setZValue(-1);

        if (continuationIndicatorLeftItem) {
            delete continuationIndicatorLeftItem;
        }

        continuationIndicatorLeftItem = q->style()->createPrimitive(HbStyle::PT_FrameItem, "continuation-indicator-left", q);
        continuationIndicatorLeftItem->setZValue(-1);

        if (continuationIndicatorRightItem) {
            delete continuationIndicatorRightItem;
        }

        continuationIndicatorRightItem = q->style()->createPrimitive(HbStyle::PT_FrameItem, "continuation-indicator-right", q);
        continuationIndicatorRightItem->setZValue(-1);
    }
}

void HbScrollAreaPrivate::updatePrimitives()
{
    Q_Q(HbScrollArea);
    if (mContinuationIndicators) {
        if (continuationIndicatorTopItem) {
            HbStyleFramePrimitiveData data;
            q->initPrimitiveData(&data, continuationIndicatorTopItem);
            q->style()->updatePrimitive(continuationIndicatorTopItem, &data, q);
            continuationIndicatorTopItem->setZValue(1);
        }
        if (continuationIndicatorBottomItem) {
            HbStyleFramePrimitiveData data;
            q->initPrimitiveData(&data, continuationIndicatorBottomItem);
            q->style()->updatePrimitive(continuationIndicatorBottomItem, &data, q);
            continuationIndicatorBottomItem->setZValue(1);
        }
        if (continuationIndicatorLeftItem) {
            HbStyleFramePrimitiveData data;
            q->initPrimitiveData(&data, continuationIndicatorLeftItem);
            q->style()->updatePrimitive(continuationIndicatorLeftItem, &data, q);
            continuationIndicatorLeftItem->setZValue(1);
        }
        if (continuationIndicatorRightItem) {
            HbStyleFramePrimitiveData data;
            q->initPrimitiveData(&data, continuationIndicatorRightItem);
            q->style()->updatePrimitive(continuationIndicatorRightItem, &data, q);
            continuationIndicatorRightItem->setZValue(1);
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

void HbScrollAreaPrivate::updateIndicators(const QPointF &newPosition)
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

void HbScrollAreaPrivate::reCalculateCachedValue() const
{
    Q_Q(const HbScrollArea);
    mBoundingRect = q->HbWidget::boundingRect();
    mShape = QPainterPath();
    mShape.addRect(mBoundingRect);
    mClearCachedRect = false;
}
