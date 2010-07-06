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
#include "hbtapgesturelogic_p.h"
#include "hbtapgesture.h"
#include "hbtapgesture_p.h"
#include "hbnamespace_p.h"

#include <hbdeviceprofile.h>
#include <QEvent>
#include <QMouseEvent>
#include <QGesture>
#include <QDebug>

//#define GESTURE_DEBUG
#ifndef GESTURE_DEBUG
# define DEBUG if (0) qDebug
#else
# define DEBUG qDebug
#endif

/*!
   @hbcore
    \internal
    \class HbTapGestureRecognizer

    \brief HbTapGestureRecognizer implements a gesture recognizer for simple taps.

*/


/*!
    \internal
    \brief
    \return

*/
HbTapGestureLogic::HbTapGestureLogic()
    :
    mTapRadius(0)
{
}

HbTapGestureLogic::~HbTapGestureLogic() {}

/*!
    \internal
    \brief
    \return

*/
void HbTapGestureLogic::resetGesture(HbTapGesture *gesture)
{
    gesture->setStartPos(QPointF());
    gesture->setSceneStartPos(QPointF());
    gesture->setProperty(HbPrivate::TapRadius.latin1(), QVariant());
    gesture->setProperty(HbPrivate::ThresholdRect.latin1(), QVariant());
    gesture->setProperty(HbPrivate::VerticallyRestricted.latin1(), QVariant());
    gesture->setProperty(HbPrivate::HorizontallyRestricted.latin1(), QVariant());
}

/*!
    \internal
    \brief
    \return

*/
QGestureRecognizer::Result HbTapGestureLogic::handleMousePress(
        Qt::GestureState gestureState,
        HbTapGesture *gesture,
        QObject *watched,
        QMouseEvent *me )
{
    DEBUG() << gesture << "PRESS";

    if ( gestureState == Qt::NoGesture && me->button() == Qt::LeftButton)
    {
        gesture->setPosition(me->globalPos());
        gesture->setHotSpot(me->globalPos());
        gesture->setStartPos(me->globalPos());
        gesture->setScenePosition(HbGestureUtils::mapToScene(watched, me->globalPos()));
        gesture->setSceneStartPos(HbGestureUtils::mapToScene(watched, me->globalPos()));
        mTapRadius = (int)(HbDefaultTapRadius * HbDeviceProfile::current().ppmValue());        

        HbTapGesturePrivate* d_ptr = gesture->d_func();
        d_ptr->mTapStyleHint = HbTapGesture::Tap;
        if ( d_ptr->mTimerId ) gesture->killTimer(d_ptr->mTimerId);
        d_ptr->mTimerId = gesture->startTimer(HbTapAndHoldTimeout);

        return QGestureRecognizer::TriggerGesture;
    }
    else
    {
        DEBUG() << gesture << "IGNORES" << me;
        return QGestureRecognizer::Ignore;
    }
}

/*!
    \internal
    \brief
    \return

*/
QGestureRecognizer::Result HbTapGestureLogic::handleMouseMove(
        Qt::GestureState gestureState,
        HbTapGesture *gesture,
        QObject *watched,
        QMouseEvent *me )
{
    if(gestureState != Qt::NoGesture && gestureState != Qt::GestureCanceled) {         
        gesture->setPosition(me->globalPos());
        gesture->setScenePosition(HbGestureUtils::mapToScene(watched, me->globalPos()));
        gesture->setHotSpot(me->globalPos());

        int tapRadiusSquare(mTapRadius * mTapRadius);
        if(gesture->property(HbPrivate::TapRadius.latin1()).isValid()) {
            qWarning("WARNING using widget specific properties in HbTapGestureRecognizer");
            int tapRadius = gesture->property(HbPrivate::TapRadius.latin1()).toInt();
            tapRadiusSquare = tapRadius * tapRadius;
        }
        QPointF delta = me->globalPos() - gesture->startPos();

        QRect thresholdRect = gesture->property(HbPrivate::ThresholdRect.latin1()).toRect();
        if (thresholdRect.isValid() &&
            !(gesture->property(HbPrivate::VerticallyRestricted.latin1()).toBool() && gesture->property(HbPrivate::HorizontallyRestricted.latin1()).toBool())) {
            // cancel long press with radius
            if((delta.x() * delta.x() + delta.y() * delta.y()) > tapRadiusSquare) {                
                if (gesture->d_func()->mTimerId) gesture->killTimer(gesture->d_func()->mTimerId);
                gesture->d_func()->mTimerId = 0;
            }

            thresholdRect.adjust(-mTapRadius, -mTapRadius, mTapRadius, mTapRadius);

            if (gesture->property(HbPrivate::VerticallyRestricted.latin1()).toBool()) {
                thresholdRect.setTop(gesture->sceneStartPos().y() - mTapRadius);
                thresholdRect.setBottom(gesture->sceneStartPos().y() + mTapRadius);
            }
            if (gesture->property(HbPrivate::HorizontallyRestricted.latin1()).toBool()){
                thresholdRect.setLeft(gesture->sceneStartPos().x() - mTapRadius);
                thresholdRect.setRight(gesture->sceneStartPos().x() + mTapRadius);
            }

            if (!thresholdRect.contains(gesture->scenePosition().toPoint())) {
                return QGestureRecognizer::CancelGesture;
            }
        } else {            
            if((delta.x() * delta.x() + delta.y() * delta.y()) > tapRadiusSquare) {
                return QGestureRecognizer::CancelGesture;
            }
        }
    }
    return QGestureRecognizer::Ignore;
}

/*!
    \internal
    \brief
    \return

*/
QGestureRecognizer::Result HbTapGestureLogic::handleMouseRelease(
        Qt::GestureState gestureState,
        HbTapGesture *gesture,
        QObject *watched,
        QMouseEvent *me )
{
    DEBUG() << gesture << "MOVE/RELEASE";
    if(gestureState != Qt::NoGesture) {
        gesture->setPosition(me->globalPos());
        gesture->setScenePosition(HbGestureUtils::mapToScene(watched, me->globalPos()));
        gesture->setHotSpot(me->globalPos());
        if(me->type() == QEvent::MouseButtonRelease &&
           me->button() == Qt::LeftButton) {
            if (gesture->d_func()->mTimerId) gesture->killTimer(gesture->d_func()->mTimerId);
            gesture->d_func()->mTimerId = 0;

            DEBUG() << gesture << "FINISHES" << me;
            return QGestureRecognizer::FinishGesture;
        }
    }

    return QGestureRecognizer::Ignore;
}

/*!
    \internal
    \brief
    \return

*/
QGestureRecognizer::Result HbTapGestureLogic::handleTimerEvent(
        Qt::GestureState gestureState,
        HbTapGesture *gesture,
        QObject *)
{
    QGestureRecognizer::Result result = QGestureRecognizer::ConsumeEventHint;
    gesture->killTimer(gesture->d_func()->mTimerId);
    gesture->d_func()->mTimerId = 0;
    if(gestureState != Qt::NoGesture) {
        gesture->d_func()->mTapStyleHint = HbTapGesture::TapAndHold;
        result |= QGestureRecognizer::TriggerGesture;
    }

    return result;
}

/*!
    \internal
    \brief
    \return

*/
QGestureRecognizer::Result HbTapGestureLogic::recognize(
        Qt::GestureState gestureState,
        HbTapGesture *gesture,
        QObject *watched,
        QEvent *event )
{
    if (!gesture || !watched || !event )
    {
        DEBUG() << "WARNING: Ignoring tap gesture because of invalid arguments from gesture fw.";
        return QGestureRecognizer::Ignore;
    }

    switch(event->type())
    {
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
        return handleMousePress(gestureState, gesture, watched, toMouseEvent(event));

    case QEvent::MouseMove:
        return handleMouseMove(gestureState, gesture, watched, toMouseEvent(event));

    case QEvent::MouseButtonRelease:
        return handleMouseRelease(gestureState, gesture, watched, toMouseEvent(event));

    case QEvent::Timer:
        return handleTimerEvent(gestureState, gesture, watched);

    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:        
        if(toTouchEvent(event)->touchPoints().count() > 1 && gestureState != Qt::NoGesture) {
            // Cancel tap on multiple fingers
            return QGestureRecognizer::CancelGesture;
        }

    default: break;
    }

    return QGestureRecognizer::Ignore;
}
