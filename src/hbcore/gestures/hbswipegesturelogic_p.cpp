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

#include <QEvent>
#include <QGestureRecognizer>
#include <QGraphicsView>
#include <QMouseEvent>

#include "hbswipegesture.h"
#include "hbswipegesture_p.h"
#include "hbswipegesturelogic_p.h"

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
    gesture->d_func()->mStartTime = QTime::currentTime();  

    gesture->d_func()->mStartPos = me->globalPos();
    gesture->d_func()->mSceneStartPos = HbGestureUtils::mapToScene(watched, me->globalPos());

    gesture->setHotSpot(me->globalPos());

    return QGestureRecognizer::MayBeGesture;
}

/*!
    \internal
    \brief
    \return

*/
QGestureRecognizer::Result HbSwipeGestureLogic::handleMouseRelease(
        Qt::GestureState gestureState,
        HbSwipeGesture *gesture,
        QObject *watched,
        QMouseEvent *me )
{   
    Q_UNUSED(gesture);
    Q_UNUSED(watched);
    Q_UNUSED(gestureState);

    QPointF totalOffset = me->globalPos() - gesture->d_func()->mStartPos.toPoint();

    QPointF velocity = totalOffset / gesture->d_func()->mStartTime.elapsed();

    gesture->setSwipeAngle(QLineF(gesture->d_func()->mStartPos, me->globalPos()).angle());
    gesture->setSceneSwipeAngle(QLineF(gesture->d_func()->mSceneStartPos, HbGestureUtils::mapToScene(watched, me->globalPos())).angle());

    if (totalOffset.manhattanLength() >= HbSwipeMinOffset && velocity.manhattanLength() >= HbSwipeMinSpeed && me->button() == Qt::LeftButton) {
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
        QEvent *event )
{
    // Record the time right away.
    mCurrentTime = QTime::currentTime();
    
    if ( isMouseEvent(event->type()) )
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        switch(event->type())
        {
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseButtonPress:
            return handleMousePress(gestureState, gesture, watched, me);

        case QEvent::MouseMove:
            if (me->buttons().testFlag(Qt::LeftButton))
                return QGestureRecognizer::MayBeGesture;
            else
                return QGestureRecognizer::Ignore;
        case QEvent::MouseButtonRelease:
            return handleMouseRelease(gestureState, gesture, watched, me);

        default: break;
        }
    }
    return QGestureRecognizer::Ignore;
}
