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

#include "hbgestures_p.h"
#include "hbtapandholdgesture.h"
#include "hbtapandholdgesture_p.h"
#include "hbtapandholdgesturelogic_p.h"

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
    \brief Starts brand new timer.
    \param msecs Timer runtime in microseconds
    \return ID of the timer
*/
int HbTapAndHoldGestureLogic::startTimer(
        HbTapAndHoldGesture* gesture,
        int msecs)
{
    gesture->priv->mRunningTime = msecs;
    return gesture->startTimer(msecs);
}

/*!
    \internal
    \brief
    \return

*/
void HbTapAndHoldGestureLogic::resetGesture(HbTapAndHoldGesture *gesture)
{
    if ( gesture->priv->mTimerID ) {
        gesture->killTimer(gesture->priv->mTimerID);
    }

    gesture->setProperty("startPos", QPointF(0,0));
    gesture->setProperty("tapRadius", QPointF(0,0));
    gesture->setProperty("position", QPointF(0,0));
    gesture->setProperty("scenePosition", QPointF(0,0));

    gesture->priv->mTimerID = 0;
    gesture->priv->mRunningTime = 0;
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
    gesture->priv->mTimerID = startTimer(gesture, HOLDTAP_ACTIVATION_USECS);
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

    // Before anything, check if there is even left button pressed.
    if (me->buttons() != Qt::LeftButton || !gesture->priv->mRunningTime){
        DEBUG() << gesture << QGestureRecognizer::Ignore;
        return QGestureRecognizer::Ignore;
    }

    gesture->setProperty("position", me->globalPos());
    gesture->setProperty("scenePosition", HbGestureUtils::mapToScene(watched, me->globalPos()));

    // Makes sure that finger remains inside the movement threshold.
    if (outsideThreshold(gesture)){        
        // Finger has moved outside, so cancel this gesture
        gesture->killTimer(gesture->priv->mTimerID);
        return QGestureRecognizer::CancelGesture;
    }

    // Move events should be just ignored.
    DEBUG() << gesture << QGestureRecognizer::MayBeGesture;
    return QGestureRecognizer::MayBeGesture;
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
    Q_UNUSED(gestureState);
    Q_UNUSED(me);
    Q_UNUSED(watched);

    // Check if the gesture is already been cancelled. This is an unknown state.
    if (!gesture->priv->mRunningTime) {
        DEBUG() << gesture << QGestureRecognizer::Ignore;
        return QGestureRecognizer::Ignore;
    }

    // If release happens, before timer has expired, cancel the gesture.
    if (gesture->priv->mTimerID) {
        gesture->killTimer(gesture->priv->mTimerID);
        return QGestureRecognizer::CancelGesture;
    } else {
        // Gesture has already been executed. Just ignore the event and don't
        // bother UI about it.
        gesture->priv->mTimerID = 0;
        gesture->priv->mRunningTime = 0;

        DEBUG() << gesture << QGestureRecognizer::Ignore;
        return QGestureRecognizer::Ignore;
    }
}			
		
/*!
    \internal
    \brief Handle timer event.
    \return State change information.

    Timer is a heart of the tap and hold gesture and dictates its
    behavior. There are three phases: not started - started - finished.
    When the timer event is invoked, the state of the gesture is changed
    and the timer event is consumed.

    \see HbTapAndHoldGestureLogic::HandleGesture()
*/
QGestureRecognizer::Result HbTapAndHoldGestureLogic::handleTimer(
        HbTapAndHoldGesture *gesture,
        QTimerEvent* te)
{
    // React only to own timer event, please.
    if ( gesture->priv->mTimerID == te->timerId() ) {
        // Consume the timer event as nobody will be interested about this.
        QGestureRecognizer::Result result = QGestureRecognizer::ConsumeEventHint;

        // Handle the event and consume the timer event as it doesn't belong
        // to anybody else.
        switch ( gesture->priv->mRunningTime )
        {
        // Time to invoke the started event.
        case HOLDTAP_ACTIVATION_USECS:
            gesture->priv->mTimerID = startTimer(gesture, HOLDTAP_DURATION_USECS);
            result |= QGestureRecognizer::TriggerGesture;
            break;

        // Time to invoke finish event.
        case HOLDTAP_DURATION_USECS:
            gesture->priv->mTimerID = 0;
            gesture->priv->mRunningTime = 0;
            result |= QGestureRecognizer::FinishGesture;
            break;

        default:
            result |= QGestureRecognizer::Ignore;
            break;
        }

        DEBUG() << gesture << result;
        return result;
    } else {
        // Not our business.
        DEBUG() << gesture << QGestureRecognizer::Ignore;
        return QGestureRecognizer::Ignore;
    }
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
    if (!gesture || !watched || !event )
    {
        DEBUG() << "WARNING: Ignoring tap and hold gesture because of invalid arguments from gesture fw.";
        return QGestureRecognizer::Ignore;
    }

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
        return handleTimer(gesture, static_cast<QTimerEvent*>(event));

    default: break;
    }

    DEBUG() << gesture << QGestureRecognizer::Ignore;
    return QGestureRecognizer::Ignore;
}
