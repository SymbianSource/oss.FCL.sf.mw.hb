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

#include "hbpangesture.h"
#include "hbpangesturelogic_p.h"
#include "hbgestures_p.h"

#include "hbnamespace_p.h"
#include <hbdeviceprofile.h>

#include <QEvent>
#include <QGestureRecognizer>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QGraphicsScene>

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
bool HbPanGestureLogic::isTouchEvent(QEvent::Type eventType)
{
    return eventType == QEvent::TouchBegin ||
            eventType == QEvent::TouchEnd ||
            eventType == QEvent::TouchUpdate;
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
    gesture->d_ptr->mAxisX.resetRecorder();
    gesture->d_ptr->mAxisY.resetRecorder();
    gesture->d_ptr->mSceneAxisX.resetRecorder();
    gesture->d_ptr->mSceneAxisY.resetRecorder();

    gesture->setLastOffset(QPointF());
    gesture->setOffset(QPointF(0,0));
    gesture->setAcceleration(0);
    gesture->setStartPos(QPointF());

    gesture->d_ptr->mIgnoreMouseEvents = false;
    gesture->d_ptr->mFollowedTouchPointId = -1;
}

/*!
    \internal
    \brief
    \return

*/
QGestureRecognizer::Result HbPanGestureLogic::handlePress(
        Qt::GestureState,
        HbPanGesture *gesture,
        QObject *watched,
        const QPointF &globalPos,
        qint64 currentTime )
{
    QPointF scenePos = HbGestureUtils::mapToScene(watched, globalPos);

    gesture->setHotSpot( globalPos );
    gesture->setStartPos( globalPos );
    gesture->setOffset( QPointF( 0,0 ) );
    gesture->setLastOffset( QPointF( 0,0 ) );
    gesture->d_ptr->mSceneStartPos       = scenePos;
    gesture->d_ptr->mSceneOffset         = QPointF( 0,0 );
    gesture->d_ptr->mSceneLastOffset     = QPointF( 0,0 );
    gesture->d_ptr->mLastTimeStamp = currentTime;

    gesture->d_ptr->mThresholdSquare = HbDefaultPanThreshold * HbDeviceProfile::current().ppmValue();
    gesture->d_ptr->mThresholdSquare = gesture->d_ptr->mThresholdSquare * gesture->d_ptr->mThresholdSquare;

    qreal velocityThreshold = HbPanVelocityUpdateThreshold * HbDeviceProfile::current().ppmValue();

    gesture->d_ptr->mAxisX.resetRecorder(velocityThreshold);
    gesture->d_ptr->mAxisY.resetRecorder(velocityThreshold);
    gesture->d_ptr->mSceneAxisX.resetRecorder(velocityThreshold);
    gesture->d_ptr->mSceneAxisY.resetRecorder(velocityThreshold);
    gesture->d_ptr->mAxisX.record( globalPos.x(), currentTime );
    gesture->d_ptr->mAxisY.record( globalPos.y(), currentTime );
    gesture->d_ptr->mSceneAxisX.record( scenePos.x(), currentTime );
    gesture->d_ptr->mSceneAxisY.record( scenePos.y(), currentTime );

    return QGestureRecognizer::MayBeGesture;
}


/*!
    \internal
    \brief
    \return

*/
QGestureRecognizer::Result HbPanGestureLogic::handleMove(
        Qt::GestureState gestureState,
        HbPanGesture *gesture,
        QObject *watched,
        const QPointF &globalPos,
        qint64 currentTime )
{
    QPointF scenePos = HbGestureUtils::mapToScene(watched,globalPos);

    gesture->d_ptr->mLastTimeStamp = currentTime;
    gesture->d_ptr->mAxisX.record( globalPos.x(), currentTime );
    gesture->d_ptr->mAxisY.record( globalPos.y(), currentTime );
    gesture->d_ptr->mSceneAxisX.record( scenePos.x(), currentTime );
    gesture->d_ptr->mSceneAxisY.record( scenePos.y(), currentTime );

    QGraphicsView* view = qobject_cast<QGraphicsView*>(watched->parent());
    if (view) {
        QGraphicsScene* scene = view->scene();
        if (scene && scene->property(HbPrivate::OverridingGesture.latin1()).isValid() &&
            scene->property(HbPrivate::OverridingGesture.latin1()).toInt() != Qt::PanGesture) {
            return QGestureRecognizer::MayBeGesture;
        }
    }

    QPointF offset = globalPos - gesture->startPos().toPoint();
    QPointF sceneOffset = scenePos - gesture->d_ptr->mSceneStartPos;

    if (gestureState == Qt::NoGesture && (offset.x() * offset.x() + offset.y() * offset.y()) <= gesture->d_ptr->mThresholdSquare) {
        return QGestureRecognizer::MayBeGesture;
    }

    // Hotspot is updated on the press and on events after the gesture started.
    // Here we are checking the previously set gestureState.
    if (gestureState == Qt::GestureStarted || gestureState == Qt::GestureUpdated) {
        gesture->setHotSpot( globalPos );
    }

    gesture->setLastOffset( gesture->offset().toPoint() );
    gesture->d_ptr->mSceneLastOffset = gesture->d_ptr->mSceneOffset;
    if (gestureState == Qt::NoGesture) {
        qreal threshold = HbDefaultTapRadius * HbDeviceProfile::current().ppmValue();

        gesture->setOffset(QPointF (qBound(-threshold, offset.x(), threshold), qBound(-threshold, offset.y(), threshold)));
        gesture->d_ptr->mSceneOffset = QPointF (qBound(-threshold, sceneOffset.x(), threshold), qBound(-threshold, sceneOffset.y(), threshold));
    } else {
        gesture->setOffset( offset );
        gesture->d_ptr->mSceneOffset = sceneOffset;
    }       

    return QGestureRecognizer::TriggerGesture;
}

/*!
    \internal
    \brief
    \return

*/
QGestureRecognizer::Result HbPanGestureLogic::handleRelease(
        Qt::GestureState gestureState,
        HbPanGesture *gesture,
        QObject *watched,
        const QPointF &globalPos,
        qint64 currentTime)
{
    gesture->d_ptr->mLastTimeStamp = currentTime;

    if (gestureState == Qt::GestureStarted || gestureState == Qt::GestureUpdated) {
        gesture->setLastOffset( gesture->offset().toPoint() );
        gesture->d_ptr->mSceneLastOffset = gesture->d_ptr->mSceneOffset;

        if(gestureState == Qt::GestureStarted) {
            QPointF offset = globalPos - gesture->startPos().toPoint();
            QPointF sceneOffset = HbGestureUtils::mapToScene(watched, globalPos) - gesture->d_ptr->mSceneStartPos;
            gesture->setOffset( offset );
            gesture->d_ptr->mSceneOffset = sceneOffset;
        }

        return QGestureRecognizer::FinishGesture;
    } else {
        QPointF offset = globalPos - gesture->startPos().toPoint();
        QPointF sceneOffset = HbGestureUtils::mapToScene(watched, globalPos) - gesture->d_ptr->mSceneStartPos;

        bool thresholdExceeded = (offset.x() * offset.x() + offset.y() * offset.y()) > gesture->d_ptr->mThresholdSquare;
        if (thresholdExceeded) {
            QGraphicsView* view = qobject_cast<QGraphicsView*>(watched->parent());
            if (view) {
                QGraphicsScene* scene = view->scene();
                if (scene && scene->property(HbPrivate::OverridingGesture.latin1()).isValid() &&
                    scene->property(HbPrivate::OverridingGesture.latin1()).toInt() != Qt::PanGesture) {

                    return QGestureRecognizer::CancelGesture;
                }
            }
            qreal threshold = HbDefaultTapRadius * HbDeviceProfile::current().ppmValue();
            gesture->setLastOffset( QPointF (qBound(-threshold, offset.x(), threshold), qBound(-threshold, offset.y(), threshold)) );
            gesture->d_ptr->mSceneLastOffset = QPointF (qBound(-threshold, sceneOffset.x(), threshold), qBound(-threshold, sceneOffset.y(), threshold));
            gesture->setOffset( offset );
            gesture->d_ptr->mSceneOffset = sceneOffset;
            return QGestureRecognizer::FinishGesture;
        }

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
        qint64 currentTime)
{
    if ( isMouseEvent(event->type()) )
    {
        if (gesture->d_ptr->mIgnoreMouseEvents) {
            return QGestureRecognizer::Ignore;
        }
        QMouseEvent* me = static_cast<QMouseEvent*>(event);

        if( !(me->button() == Qt::LeftButton || me->buttons().testFlag(Qt::LeftButton)) ) {
            return QGestureRecognizer::Ignore;
        }

        switch(event->type())
        {
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseButtonPress:
            // Just ignore situations that are not interesting at all.
            if ( gestureState != Qt::NoGesture )
            {
                return QGestureRecognizer::Ignore;
            }
            return handlePress(gestureState, gesture, watched, me->globalPos(), currentTime);
        case QEvent::MouseMove:
            return handleMove(gestureState, gesture, watched, me->globalPos(), currentTime);
        case QEvent::MouseButtonRelease:
            return handleRelease(gestureState, gesture, watched, me->globalPos(), currentTime);
        default: break;
        }
    }
    if (isTouchEvent(event->type()) && watched->isWidgetType() ) {
        QTouchEvent *te = static_cast<QTouchEvent *>(event);
        QTouchEvent::TouchPoint currentTouchPoint = followedTouchPoint(te, gesture);

        switch(event->type()) {
        case QEvent::TouchBegin:
            // Store panning finger id
            gesture->d_ptr->mFollowedTouchPointId = currentTouchPoint.id();
            return handlePress(gestureState, gesture, watched, currentTouchPoint.screenPos(), currentTime);

        case QEvent::TouchUpdate:
            // If we get touch updates, we can ignore mouse events.
            gesture->d_ptr->mIgnoreMouseEvents = true;

            // followed finger has been released
            if (currentTouchPoint.state() == Qt::TouchPointReleased) {
                // Store panning finger id. There is always one available,
                // otherwise this would be TouchEnd
                gesture->d_ptr->mFollowedTouchPointId = (currentTouchPoint = getNextTouchPoint(te)).id();
                (void) handlePress(gestureState, gesture, watched, currentTouchPoint.screenPos(), currentTime);

                // This was a finger switch -> do not trigger update
                return gestureState == Qt::NoGesture ? QGestureRecognizer::MayBeGesture : QGestureRecognizer::Ignore;

            } else if (currentTouchPoint.state() == Qt::TouchPointMoved) {
                return handleMove(gestureState, gesture, watched, currentTouchPoint.screenPos(), currentTime);
            }
            break;
        case QEvent::TouchEnd:
            gesture->d_ptr->mIgnoreMouseEvents = false;
            return handleRelease(gestureState, gesture, watched, currentTouchPoint.screenPos(), currentTime);
        default:
            break;
        }
    }
    return QGestureRecognizer::Ignore;
}

/*!
    \internal
    \brief
    \return

*/
QTouchEvent::TouchPoint HbPanGestureLogic::followedTouchPoint(QTouchEvent *te, HbPanGesture *gesture)
{
    if (gesture->d_ptr->mFollowedTouchPointId == -1) {
        return getNextTouchPoint(te);
    } else {
        QList<QTouchEvent::TouchPoint>::const_iterator it;
        for (it = te->touchPoints().constBegin(); it != te->touchPoints().constEnd(); ++it) {
            if( (*it).id() == gesture->d_ptr->mFollowedTouchPointId )
                return *it;
        }
    }
    return QTouchEvent::TouchPoint();
}

/*!
    \internal
    \brief
    \return

*/
QTouchEvent::TouchPoint HbPanGestureLogic::getNextTouchPoint(QTouchEvent *te)
{
    QList<QTouchEvent::TouchPoint>::const_iterator it;
    for (it = te->touchPoints().constBegin(); it != te->touchPoints().constEnd(); ++it) {
        if( (*it).state() != Qt::TouchPointReleased )
            return *it;
    }
    Q_ASSERT(false);
    return QTouchEvent::TouchPoint();
}
