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

#include "hbswipegesture.h"
#include "hbswipegesture_p.h"
#include "hbswipegesturelogic_p.h"
#include "hbpointrecorder_p.h"
#include "hbvelocitycalculator_p.h"

#include <hbdeviceprofile.h>

#include <QEvent>
#include <QGestureRecognizer>
#include <QGraphicsView>
#include <QMouseEvent>

/*!
   @hbcore
   \internal
   \class HbSwipeGestureLogic

   \brief

*/


/*!
    \internal
    \brief
    \return

*/
HbSwipeGestureLogic::HbSwipeGestureLogic()
{
    mCurrentTime = QTime();
}

HbSwipeGestureLogic::~HbSwipeGestureLogic() {}

/*!
    \internal
    \brief
    \return

*/
bool HbSwipeGestureLogic::isMouseEvent(QEvent::Type eventType)
{
    return eventType == QEvent::MouseButtonPress ||
           eventType == QEvent::MouseMove ||
           eventType == QEvent::MouseButtonDblClick ||
           eventType == QEvent::MouseButtonRelease;
}

/*!
    \internal
    \brief
    \return

*/
void HbSwipeGestureLogic::resetGesture(HbSwipeGesture *gesture)
{   
    gesture->setSwipeAngle(0);
    gesture->setSceneSwipeAngle(0);

    gesture->d_func()->mStartPos = QPointF();
    gesture->d_func()->mSceneStartPos = QPointF();    
}

/*!
    \internal
    \brief
    \return

*/
QGestureRecognizer::Result HbSwipeGestureLogic::handleMousePress(
        Qt::GestureState gestureState,
        HbSwipeGesture *gesture,
        QObject *watched,
        QMouseEvent *me )
{
    // Just ignore situations that are not interesting at all.
    if (!(gestureState == Qt::NoGesture && me->button() == Qt::LeftButton)) {
        return QGestureRecognizer::Ignore;
    }
    gesture->d_func()->mStartTime = mCurrentTime;

    gesture->d_func()->mStartPos = me->globalPos();
    gesture->d_func()->mSceneStartPos = HbGestureUtils::mapToScene(watched, me->globalPos());

    gesture->setHotSpot(me->globalPos());
    gesture->d_ptr->mAxisX.clear();
    gesture->d_ptr->mAxisY.clear();
    gesture->d_ptr->mAxisX.record(me->globalPos().x(), mCurrentTime);
    gesture->d_ptr->mAxisY.record(me->globalPos().y(), mCurrentTime);

    return QGestureRecognizer::MayBeGesture;
}

/*!
    \internal
    \brief
    \return

*/
QGestureRecognizer::Result HbSwipeGestureLogic::handleMouseMove(
        Qt::GestureState gestureState,
        HbSwipeGesture *gesture,
        QObject *watched,
        QMouseEvent *me )
{
    Q_UNUSED(watched);
    Q_UNUSED(gestureState);

    if (!me->buttons().testFlag(Qt::LeftButton)){
        return QGestureRecognizer::Ignore;
    }

    gesture->d_ptr->mAxisX.record(me->globalPos().x(), mCurrentTime);
    gesture->d_ptr->mAxisY.record(me->globalPos().y(), mCurrentTime);

    return QGestureRecognizer::MayBeGesture;
}

/*!
    \internal
    \brief
    \return

*/
#include <QDebug>
QGestureRecognizer::Result HbSwipeGestureLogic::handleMouseRelease(
        Qt::GestureState gestureState,
        HbSwipeGesture *gesture,
        QObject *watched,
        QMouseEvent *me )
{   
    Q_UNUSED(gesture);
    Q_UNUSED(watched);
    Q_UNUSED(gestureState);

    if (me->button() != Qt::LeftButton) {
        return QGestureRecognizer::Ignore;
    }

    QPoint totalOffset = me->globalPos() - gesture->d_func()->mStartPos.toPoint();

    int deltaTime = gesture->d_func()->mStartTime.msecsTo(mCurrentTime);
    QPointF velocity = deltaTime != 0 ? totalOffset / deltaTime : QPointF(0,0);

    gesture->setSwipeAngle(QLineF(gesture->d_func()->mStartPos, me->globalPos()).angle());
    gesture->setSceneSwipeAngle(QLineF(gesture->d_func()->mSceneStartPos, HbGestureUtils::mapToScene(watched, me->globalPos())).angle());    
    bool movedEnough = totalOffset.manhattanLength() >= (int)(HbSwipeMinOffset * HbDeviceProfile::current().ppmValue());
    bool fastEnough = velocity.manhattanLength() >= HbSwipeMinSpeed * HbDeviceProfile::current().ppmValue();
    bool notStoppedAtEnd = HbVelocityCalculator(gesture->d_ptr->mAxisX, gesture->d_ptr->mAxisY).velocity(mCurrentTime) != QPointF(0,0);

    if (movedEnough && fastEnough && notStoppedAtEnd) {
        return QGestureRecognizer::FinishGesture;
    } else {
        return QGestureRecognizer::Ignore;
    }
}

/*!
    \internal
    \brief
    \return

*/
QGestureRecognizer::Result HbSwipeGestureLogic::recognize(
        Qt::GestureState gestureState,
        HbSwipeGesture *gesture,
        QObject *watched,
        QEvent *event,
        QTime currentTime)
{
    // Record the time right away.
    mCurrentTime = currentTime;
    
    if ( isMouseEvent(event->type()) )
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        switch(event->type())
        {
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseButtonPress:
            return handleMousePress(gestureState, gesture, watched, me);

        case QEvent::MouseMove:
            return handleMouseMove(gestureState, gesture, watched, me);

        case QEvent::MouseButtonRelease:
            return handleMouseRelease(gestureState, gesture, watched, me);

        default: break;
        }
    }
    return QGestureRecognizer::Ignore;
}
