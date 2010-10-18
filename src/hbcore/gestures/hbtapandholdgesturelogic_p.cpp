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

#include "hbtapandholdgesture.h"
#include "hbtapandholdgesturelogic_p.h"
#include "hbgestures_p.h"

#include <hbdeviceprofile.h>

#include <QEvent>
#include <QGestureRecognizer>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QDebug>

//#define TAPANDHOLD_DEBUG
#ifdef TAPANDHOLD_DEBUG
#define DEBUG qDebug
#else
#define DEBUG if (0) qDebug
#endif

/*!
    @hbcore
    \internal
    \class HbTapAndHoldGestureLogic

    \brief HbTapAndHoldGestureLogic implements a gesture for tap and hold.
*/


/*!
    \internal
    \brief
    \return

*/
HbTapAndHoldGestureLogic::HbTapAndHoldGestureLogic()
    :
    mTapRadius(0)
{
}

/*!
    \internal
    \brief
    \return

*/
HbTapAndHoldGestureLogic::~HbTapAndHoldGestureLogic() {}

/*!
    \internal
    \brief Checks if point has moved beyond finger threshold.

    After user has touched the screen and intends invoke 'tap and hold' -gesture,
    he must keep finger still until the timer expires. If finger position moves
    beyond the given threshold for moving, timer needs to cancel. This function
    checks, if the finger is still inside the threshold.
*/
bool HbTapAndHoldGestureLogic::outsideThreshold(HbTapAndHoldGesture *gesture)
{
    QPointF startPos = gesture->property("startPos").toPointF();
    QPointF lastPos = gesture->property("position").toPointF();

    QPointF delta = lastPos - startPos;

    int movementThresholdSquare = mTapRadius * mTapRadius;
    if ( gesture->property("tapRadius").isValid() ) {
        movementThresholdSquare = gesture->property("tapRadius").toInt() * gesture->property("tapRadius").toInt();
    }    
    return (delta.x() * delta.x() + delta.y() * delta.y()) > movementThresholdSquare;
};

/*!
    \internal
    \brief
    \return

*/
void HbTapAndHoldGestureLogic::resetGesture(HbTapAndHoldGesture *gesture)
{    
    if ( gesture->priv->mTimerID ) {
        gesture->killTimer(gesture->priv->mTimerID);
        gesture->priv->mTimerID = 0;
    }    
    gesture->setProperty("startPos", QPointF(0,0));
    gesture->setProperty("tapRadius", QVariant());
    gesture->setProperty("position", QPointF(0,0));
    gesture->setProperty("scenePosition", QPointF(0,0));
}

/*!
    \internal
    \brief Handle mouse press event.
    \return State change information.

    Mouse press event only needs to record the location and start short timer
    before triggering.
*/
QGestureRecognizer::Result HbTapAndHoldGestureLogic::handleMousePress(
        Qt::GestureState gestureState,
        HbTapAndHoldGesture *gesture,
        QObject *watched,
        QMouseEvent *me )
{
    Q_UNUSED(gestureState);
    // Accept only press events from left mouse button.
    if ( me->button() != Qt::LeftButton ) {
        DEBUG() << gesture << QGestureRecognizer::Ignore;
        return QGestureRecognizer::Ignore;
    }

    // Last position is automatically recorded before this event call.
    // Press event only means, that gesture is starting, thus last position is
    // also the starting position.
    gesture->setHotSpot(me->globalPos());
    gesture->setProperty("startPos", me->globalPos());
    gesture->setProperty("position", me->globalPos());
    gesture->setProperty("scenePosition", HbGestureUtils::mapToScene(watched, me->globalPos()));

    Q_ASSERT(gestureState == Qt::NoGesture);

    // If timer was started, kill it first
    if(gesture->priv->mTimerID) {
        gesture->killTimer(gesture->priv->mTimerID);
        gesture->priv->mTimerID = 0;
    }
    // start timer
    gesture->priv->mTimerID = gesture->startTimer(HbTapAndHoldTimeout);
    mTapRadius = (int)(HbDefaultTapRadius * HbDeviceProfile::current().ppmValue());

    DEBUG() << gesture << QGestureRecognizer::MayBeGesture;
    return QGestureRecognizer::MayBeGesture;
}

/*!
    \internal
    \brief Handle mouse move event.
    \return State change information.

    Mousemove -event should cancel the gesture, when finger has moved outside
    the threshold.
*/
QGestureRecognizer::Result HbTapAndHoldGestureLogic::handleMouseMove(
        Qt::GestureState gestureState,
        HbTapAndHoldGesture *gesture,
        QObject *watched,
        QMouseEvent *me )
{
    Q_UNUSED(gestureState);

    // Before anything, check if there is left button pressed.
    if (!(me->buttons() & Qt::LeftButton) ){
        return QGestureRecognizer::Ignore;
    }

    // If timer is not running we can ignore move events
    if(!gesture->priv->mTimerID) {
        return QGestureRecognizer::Ignore;
    }

    gesture->setProperty("position", me->globalPos());
    gesture->setProperty("scenePosition", HbGestureUtils::mapToScene(watched, me->globalPos()));

    // Makes sure that finger remains inside the movement threshold.
    if (outsideThreshold(gesture)){        
        // Finger has moved outside, so cancel this gesture
        gesture->killTimer(gesture->priv->mTimerID);
        gesture->priv->mTimerID = 0;
        DEBUG() <<  gesture << "threshold exceeded";
        return QGestureRecognizer::CancelGesture;
    }
    // moving within threshold
    return QGestureRecognizer::Ignore;
}
            
/*!
    \internal
    \brief Handles mouse release event.
    \return State change information.

    When release happens, any timer running dictates, whether the gesture
    should be considered as cancelled or finished. Both events needs to be
    sent, so that UI may react correctly.
*/
QGestureRecognizer::Result HbTapAndHoldGestureLogic::handleMouseRelease(
        Qt::GestureState gestureState,
        HbTapAndHoldGesture *gesture,
        QObject *watched,
        QMouseEvent *me )
{
    Q_UNUSED(me);
    Q_UNUSED(watched);
    Q_UNUSED(gestureState);

    // Before anything, check if left button was released.
    if (!(me->button() == Qt::LeftButton) ){
        DEBUG() << gesture << QGestureRecognizer::Ignore;
        return QGestureRecognizer::Ignore;
    }

    // make sure we not in invalid state
    Q_ASSERT(!(gestureState == Qt::GestureFinished && !gesture->priv->mTimerID));

    // Mouse release can only cancel or ignore gesture since timers handle
    // triggering

    // If timer was started, kill it
    if(gesture->priv->mTimerID) {
        gesture->killTimer(gesture->priv->mTimerID);
        gesture->priv->mTimerID = 0;
        // Gesture state in Gesture Manager is MaybeGesture
        return QGestureRecognizer::CancelGesture;
    }
    DEBUG() << gesture << QGestureRecognizer::Ignore;
    return QGestureRecognizer::Ignore;

}           
        
/*!
    \internal
    \brief Handle timer event.
    \return State change information.

    Timer is a heart of the tap and hold gesture and dictates its
    behavior. After tap-and-hold timeout, the state of the gesture is
    changed to finished, and the timer event is consumed. Gesture manager
    takes care that gesture started event is also delivered.

    \see HbTapAndHoldGestureLogic::HandleGesture()
*/
QGestureRecognizer::Result HbTapAndHoldGestureLogic::handleTimer(
        Qt::GestureState gestureState,
        HbTapAndHoldGesture *gesture,
        QObject *watched,
        QTimerEvent *te )
{
    Q_UNUSED(watched);
    Q_UNUSED(te);

    // React only to own timer event
    Q_ASSERT(gesture->priv->mTimerID == te->timerId());

    // Consume the timer event as nobody will be interested about this.
    QGestureRecognizer::Result result = QGestureRecognizer::ConsumeEventHint;
    gesture->killTimer(gesture->priv->mTimerID);
    gesture->priv->mTimerID = 0;

    if(gestureState == Qt::NoGesture) {
        result |= QGestureRecognizer::FinishGesture;
    } else {
        // invalid state
        Q_ASSERT(false);
    }

    DEBUG() << gesture << result;
    return result;

}

/*!
    \internal
    \brief Recognizes and handles events and converts them to gesture events.
    \param state Associated gesture
    \param watched Object that needs attention.
    \param event Event invoked the this function call.
    \return State change information.
    \relates QGestureRecognizer
*/
QGestureRecognizer::Result HbTapAndHoldGestureLogic::recognize(
        Qt::GestureState gestureState,
        HbTapAndHoldGesture *gesture,
        QObject *watched,
        QEvent *event )
{    
    switch( event->type() )
    {
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
        return handleMousePress(
                gestureState, gesture, watched, static_cast<QMouseEvent*>(event));

    case QEvent::MouseMove:
        return handleMouseMove(
                gestureState, gesture, watched, static_cast<QMouseEvent*>(event));

    case QEvent::MouseButtonRelease:
        return handleMouseRelease(
                gestureState, gesture, watched, static_cast<QMouseEvent*>(event));

    case QEvent::Timer:
        return handleTimer(
                gestureState, gesture, watched, static_cast<QTimerEvent*>(event));

    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
        if(static_cast<QTouchEvent*>(event)->touchPoints().count() > 1) {
            // Cancel tap and hold on multiple fingers
            // If timer was started, kill it
            if(gesture->priv->mTimerID) {
                gesture->killTimer(gesture->priv->mTimerID);
                gesture->priv->mTimerID = 0;
                return QGestureRecognizer::CancelGesture;
            }
        }

    default: break;
    }
    return QGestureRecognizer::Ignore;
}
