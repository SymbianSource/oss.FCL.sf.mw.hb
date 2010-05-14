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

#include "hbpangesture.h"
#include "hbpangesture_p.h"
#include "hbpangesturelogic_p.h"

const int KPanThreshold = 20;

/*!
   @hbcore
   \internal
   \class HbPanGestureLogic

   \brief

*/


/*!
    \internal
    \brief
    \return

*/
HbPanGestureLogic::HbPanGestureLogic()
{
    mCurrentTime = QTime();
}

HbPanGestureLogic::~HbPanGestureLogic() {}

/*!
    \internal
    \brief
    \return

*/
bool HbPanGestureLogic::isMouseEvent(QEvent::Type eventType)
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
void HbPanGestureLogic::resetGesture(HbPanGesture *gesture)
{
    gesture->d_ptr->mStartPos                     = QPointF(0,0);
    gesture->d_ptr->mDeltaSinceLastTimeStamp      = QPointF(0,0);
    gesture->d_ptr->mSceneStartPos                = QPointF(0,0);
    gesture->d_ptr->mSceneLastOffset              = QPointF(0,0);
    gesture->d_ptr->mSceneOffset                  = QPointF(0,0);
    gesture->d_ptr->mSceneDeltaSinceLastTimeStamp = QPointF(0,0);
    gesture->d_ptr->mAxisX.clear();
    gesture->d_ptr->mAxisY.clear();
    gesture->d_ptr->mSceneAxisX.clear();
    gesture->d_ptr->mSceneAxisY.clear();

    gesture->setLastOffset(QPointF());
    gesture->setOffset(QPointF(0,0));
    gesture->setAcceleration(0);
    gesture->setStartPos(QPointF());
}

/*!
    \internal
    \brief
    \return

*/
QGestureRecognizer::Result HbPanGestureLogic::handleMousePress(
        Qt::GestureState gestureState,
        HbPanGesture *gesture,
        QObject *watched,
        QMouseEvent *me )
{
    // Just ignore situations that are not interesting at all.
    if ( !( gestureState == Qt::NoGesture && me->button() == Qt::LeftButton ) )
    {
        return QGestureRecognizer::Ignore;
    }
    
    gesture->setHotSpot( me->globalPos() );
    gesture->setStartPos( me->globalPos() );
    gesture->setOffset( QPointF( 0,0 ) );
    gesture->setLastOffset( QPointF( 0,0 ) );
    QPointF scenePos = HbGestureUtils::mapToScene(watched, me->globalPos());
    gesture->d_ptr->mSceneStartPos       = scenePos;
    gesture->d_ptr->mSceneOffset         = HbGestureUtils::mapToScene(watched, QPointF(0,0));
    gesture->d_ptr->mSceneLastOffset     = HbGestureUtils::mapToScene(watched, QPointF(0,0));
    gesture->d_ptr->mLastTimeStamp = mCurrentTime;
        
    gesture->d_ptr->mAxisX.resetRecorder(HbDefaultPanThreshold);
    gesture->d_ptr->mAxisY.resetRecorder(HbDefaultPanThreshold);
    gesture->d_ptr->mSceneAxisX.resetRecorder(HbDefaultPanThreshold);
    gesture->d_ptr->mSceneAxisY.resetRecorder(HbDefaultPanThreshold);
    gesture->d_ptr->mAxisX.record( me->globalPos().x(), mCurrentTime );
    gesture->d_ptr->mAxisY.record( me->globalPos().y(), mCurrentTime );
    gesture->d_ptr->mSceneAxisX.record( scenePos.x(), mCurrentTime );
    gesture->d_ptr->mSceneAxisY.record( scenePos.y(), mCurrentTime );

    return QGestureRecognizer::MayBeGesture;
}

/*!
    \internal
    \brief
    \return

*/
QGestureRecognizer::Result HbPanGestureLogic::handleMouseMove(
        Qt::GestureState gestureState,
        HbPanGesture *gesture,
        QObject *watched,
        QMouseEvent *me )
{
    if ( !me->buttons().testFlag(Qt::LeftButton) )
    {
        return QGestureRecognizer::Ignore;
    }

    QPointF offset = me->globalPos() - gesture->startPos().toPoint();

    if (gestureState == Qt::NoGesture && offset.manhattanLength() <= KPanThreshold )
    {
        return QGestureRecognizer::MayBeGesture;
    }

    // Hotspot is updated on the press and on events after the gesture started.
    // Here we are checking the previously set gestureState.
    if (gestureState == Qt::GestureStarted || gestureState == Qt::GestureUpdated) {
        gesture->setHotSpot( me->globalPos() );
    }

    gesture->setLastOffset( gesture->offset().toPoint() );
    gesture->setOffset( offset );
    gesture->d_ptr->mSceneLastOffset = gesture->d_ptr->mSceneOffset;
    gesture->d_ptr->mSceneOffset =
            HbGestureUtils::mapToScene(watched, me->globalPos()) - gesture->d_ptr->mSceneStartPos;
    gesture->d_ptr->mLastTimeStamp = mCurrentTime;

    QPointF scenePos = HbGestureUtils::mapToScene(watched, me->globalPos());
    gesture->d_ptr->mAxisX.record( me->globalPos().x(), mCurrentTime );
    gesture->d_ptr->mAxisY.record( me->globalPos().y(), mCurrentTime );
    gesture->d_ptr->mSceneAxisX.record( scenePos.x(), mCurrentTime );
    gesture->d_ptr->mSceneAxisY.record( scenePos.y(), mCurrentTime );

    return QGestureRecognizer::TriggerGesture;
}

/*!
    \internal
    \brief
    \return

*/
QGestureRecognizer::Result HbPanGestureLogic::handleMouseRelease(
        Qt::GestureState gestureState,
        HbPanGesture *gesture,
        QObject *watched,
        QMouseEvent *me )
{   
    Q_UNUSED(me->globalPos());
    Q_UNUSED(gesture);
    Q_UNUSED(watched);
	
    gesture->d_ptr->mLastTimeStamp = mCurrentTime;
		
    if ( gestureState == Qt::GestureStarted || gestureState == Qt::GestureUpdated )
    {
        return QGestureRecognizer::FinishGesture;
    }
    else
    {
        return QGestureRecognizer::CancelGesture;
    }
}

/*!
    \internal
    \brief
    \return

*/
QGestureRecognizer::Result HbPanGestureLogic::recognize(
        Qt::GestureState gestureState,
        HbPanGesture *gesture,
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
