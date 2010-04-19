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
#include "hbtapandholdgesture_p.h"
#include "hbtapandholdgesturerecognizer_p.h"

#include <QWidget>
#include <QGesture>
#include <QEvent>
#include <QVariant>
#include <QGraphicsSceneEvent>
#include <QDebug>

/*!
    \internal
    \class HbTapAndHoldGestureRecognizer

    \brief HbTapAndHoldGestureRecognizer implements a gesture for tap and hold.
*/

/*!
    \internal
    \brief Default constructor.
*/
HbTapAndHoldGestureRecognizer::HbTapAndHoldGestureRecognizer()
    :
    QGestureRecognizer()
{
}

/*!
    \internal
    \brief Create new gesture object.
    \param target Associated target.
*/
QGesture* HbTapAndHoldGestureRecognizer::create(QObject* /*target*/)
{
    return new HbTapAndHoldGesture;
}

/*!
    \internal
    \brief Handle mouse press event.
    \return State change information.

    Mouse press event only needs to record the location and start short timer
    before triggering.
*/
QGestureRecognizer::Result HbTapAndHoldGestureRecognizer::HandleGraphicsSceneMousePress(
	HbTapAndHoldGesture& gesture )
{
    // Last position is automatically recorded before this event call.
    // Press event only means, that gesture is starting, thus last position is
    // also the starting position.
    gesture.setProperty("startPos", gesture.property("position"));
    gesture.priv->mTimerID = startTimer(gesture, HOLDTAP_ACTIVATION_USECS);
    gesture.setHotSpot(gesture.property("startPos").toPointF());

    return QGestureRecognizer::MayBeGesture;
}

/*!
    \internal
    \brief Handle mouse move event.
    \return State change information.

    Mousemove -event should cancel the gesture, when finger has moved outside
    the threshold.
*/
QGestureRecognizer::Result HbTapAndHoldGestureRecognizer::HandleGraphicsSceneMouseMove(
	HbTapAndHoldGesture& gesture )
{
    // In case timer is not running, just cancel gesture.priv-> This is an unknown state.
    if ( !gesture.priv->mRunningTime ) return QGestureRecognizer::CancelGesture;

    // Makes sure that finger remains inside the movement threshold.
    if ( gesture.outsideThreshold() )
    {
        // Finger has moved outside, so reset and cancel this gesture.priv->
        reset(&gesture);
        return QGestureRecognizer::CancelGesture;
    }

    // Move events should be just ignored.
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
QGestureRecognizer::Result HbTapAndHoldGestureRecognizer::HandleGraphicsSceneMouseRelease(
	HbTapAndHoldGesture& gesture )
{
    // Check if the gesture is already been cancelled. This is an unknown state.
    if ( !gesture.priv->mRunningTime ) return QGestureRecognizer::CancelGesture;

    // If release happens, before timer has expired, cancel the gesture.priv->
    if ( gesture.priv->mTimerID )
    {
        reset(&gesture);
        return QGestureRecognizer::CancelGesture;
    }
    else
    {
        // Gesture has succesfully executed. Reward the UI with finished event.
        gesture.priv->mTimerID = 0;
        gesture.priv->mRunningTime = 0;
        return QGestureRecognizer::FinishGesture;
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

    \see HbTapAndHoldGestureRecognizer::HandleGesture()
*/
QGestureRecognizer::Result HbTapAndHoldGestureRecognizer::HandleTimer(
	HbTapAndHoldGesture& gesture,
	QEvent& event)
{
    // React only to own timer event, please.
    if ( gesture.priv->mTimerID == GetTimerID(event) )
    {
        // Handle the event and consume the timer event as it doesn't belong
        // to anybody else.
        return HandleGesture(gesture) | QGestureRecognizer::ConsumeEventHint;
    }
    else
    {
        // Not our business.
        return QGestureRecognizer::Ignore;
    }
}

/*!
    \internal
    \brief Handle event invoked by the timer.
    \return State change information.

    Changes the hold and tap -gestures state, dictated by the expired timer.
    Changes the state from Not started to started and from started to finished.

    \see HbTapAndHoldGestureRecognizer::HandleTimer()
*/
QGestureRecognizer::Result HbTapAndHoldGestureRecognizer::HandleGesture(
	HbTapAndHoldGesture& gesture)
{
    switch ( gesture.priv->mRunningTime )
    {
        // Time to invoke the started event.
        case HOLDTAP_ACTIVATION_USECS:
            gesture.priv->mTimerID = startTimer(gesture, HOLDTAP_DURATION_USECS);
            return QGestureRecognizer::TriggerGesture;

        // Time to invoke finish event.
        case HOLDTAP_DURATION_USECS:
            gesture.priv->mTimerID = 0;
            return QGestureRecognizer::FinishGesture;

        default: break;
    }

    return QGestureRecognizer::Ignore;
}

/*!
    \internal
    \brief Checks whether the events needs to be reacted or not
    \param event Event to be analyzed.
    \return True, if the event is mouse event or timer event, otherwise False.
*/
bool HbTapAndHoldGestureRecognizer::IsInterestingEvent(QEvent& event)
{
    return ( event.type() >= QEvent::GraphicsSceneMouseMove &&
			 event.type() <= QEvent::GraphicsSceneMouseRelease ) ||
           event.type() == QEvent::Timer;
}

/*!
    \internal
    \brief Retrieves the timer event from the event from the event.
    \param event Event for investigation.
    \return Valid timer ID
*/
int HbTapAndHoldGestureRecognizer::GetTimerID(QEvent& event)
{
	// It is already safe to presume the event is timer event, because
	// the test is already made.
    QTimerEvent* te = static_cast<QTimerEvent*>(&event);
    return te->timerId();
}

/*!
    \internal
    \brief Recognizes and handles events and converts them to gesture events.
    \param state Associated gesture.priv->
    \param watched Object that needs attention.
    \param event Event invoked the this function call.
    \return State change information.
    \relates QGestureRecognizer
*/
QGestureRecognizer::Result HbTapAndHoldGestureRecognizer::recognize(
        QGesture *state, QObject * /*watched*/, QEvent *event)
{
    // Don't even try to handle cases, when any of the pointers is NULL.
    if (!state || !event) return QGestureRecognizer::Ignore;
    // Ignore events which are not gesture events.
    if ( !IsInterestingEvent(*event) ) return QGestureRecognizer::Ignore;

    HbTapAndHoldGesture& gesture = *static_cast<HbTapAndHoldGesture*>(state);
    gesture.update(*event);

    QGestureRecognizer::Result result(QGestureRecognizer::Ignore);

    switch (event->type()) {
    case QEvent::GraphicsSceneMousePress:   return HandleGraphicsSceneMousePress(gesture);
    case QEvent::GraphicsSceneMouseRelease: return HandleGraphicsSceneMouseRelease(gesture);
    case QEvent::GraphicsSceneMouseMove:    return HandleGraphicsSceneMouseMove(gesture);
    case QEvent::Timer:                     return HandleTimer(gesture, *event);

    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd: return QGestureRecognizer::Ignore;
    default: break;
    }
    return QGestureRecognizer::Ignore;
}

/*!
    \internal
    \brief Clears all values for recognizer and gesture.priv->
    \param state Associated gesture.priv->
    \relates QGestureRecognizer
*/
void HbTapAndHoldGestureRecognizer::reset(QGesture *state)
{
    QGestureRecognizer::reset(state);

	HbTapAndHoldGesture& gesture = *static_cast<HbTapAndHoldGesture*>(state);
    if ( gesture.priv->mTimerID )
    {
        gesture.killTimer(gesture.priv->mTimerID);
    }

	gesture.setProperty("startPos", QVariant());
    gesture.setProperty("tapRadius", QVariant());

    gesture.priv->mTimerID = 0;
    gesture.priv->mRunningTime = 0;
}

/*!
    \internal
    \brief Starts brand new timer.
    \param msecs Timer runtime in microseconds
    \return ID of the timer
*/
int HbTapAndHoldGestureRecognizer::startTimer(
	HbTapAndHoldGesture& gesture,
	int msecs)
{
    gesture.priv->mRunningTime = msecs;
    return gesture.startTimer(msecs);
}

