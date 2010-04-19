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

/*!
//
//  W A R N I N G
//  -------------
//
// This implementation of Gesture filter is most probably removed in later releases.
// It exists purely as an implementation detail.
// This implementation may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
*/

#include "hbswipegesturerecognizer_p.h"
#include "hbswipegesture.h"
#include "hbswipegesture_p.h"

#include <QEvent>
#include <QTouchEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPoint>
#include <QLine>
#include <QVariant>

#include <QDebug>

const int KMilliSecsToSecs = 1000;

/*!
	@proto
    @hbcore
	\class HbSwipeGestureRecognizer
    \brief HbSwipeGestureRecognizer Gesture recognizer for swipes (flicks).
	
*/


/*!
	Constructs new HbSwipeGestureRecognizer.
 */
HbSwipeGestureRecognizer::HbSwipeGestureRecognizer()
{
}

HbSwipeGestureRecognizer::~HbSwipeGestureRecognizer()
{
}

QGesture* HbSwipeGestureRecognizer::create(QObject *)
{
    return new HbSwipeGesture();
}

/*!
	The event filter function.
	\a obj Parameter not currently used.

	\internal
 */
QGestureRecognizer::Result HbSwipeGestureRecognizer::recognize(QGesture *state, QObject *, QEvent *event)
{
    HbSwipeGesture *q = qobject_cast<HbSwipeGesture *>(state);
    if (!q) {
        return QGestureRecognizer::Ignore;
    }
    if (QTouchEvent *touchEvent = dynamic_cast<QTouchEvent *>(event)) {
        return recognizeTouchEvent(q, touchEvent);
    }
    else if (QGraphicsSceneMouseEvent *mouseEvent = dynamic_cast<QGraphicsSceneMouseEvent *>(event)){
        return recognizeMouseEvent(q, mouseEvent);
    }

    return QGestureRecognizer::Ignore;
}

/*!

*/
QGestureRecognizer::Result HbSwipeGestureRecognizer::recognizeTouchEvent(HbSwipeGesture *q, QTouchEvent *touchEvent)
{
    QGestureRecognizer::Result result;

    switch (touchEvent->type()) {
    case QEvent::TouchBegin: {
        q->d_func()->mSpeed = 1;
        q->d_func()->mTouchPointCount = 1;
        q->d_func()->mTime = QTime::currentTime();
        result = QGestureRecognizer::MayBeGesture | QGestureRecognizer::ConsumeEventHint;
        break;
    }
    case QEvent::TouchEnd: {
        if (q->state() != Qt::NoGesture) {
            result = QGestureRecognizer::FinishGesture;
            q->setProperty("speed", (int)(q->d_func()->mSpeed*KMilliSecsToSecs));
        } else {
            result = QGestureRecognizer::CancelGesture;
        }
        break;
    }
    case QEvent::TouchUpdate: {
        if(q->d_func()->mInitialPoint == QPointF()) {
            q->d_func()->mInitialPoint = touchEvent->touchPoints().first().lastScreenPos();
        }
        
        q->setHotSpot(touchEvent->touchPoints().first().screenPos());

        int touchPointCount = touchEvent->touchPoints().size();

        qreal xDistance = 0;
        for (int i = 0; i < touchPointCount; i++) {
            qreal distance = touchEvent->touchPoints().at(i).screenPos().x() - touchEvent->touchPoints().at(i).lastScreenPos().x();
            if(qAbs(distance) > qAbs(xDistance)) {
                xDistance = distance;
            }
        }

        qreal yDistance = 0;
        for (int i = 0; i < touchPointCount; i++) {
            qreal distance = touchEvent->touchPoints().at(i).screenPos().y() - touchEvent->touchPoints().at(i).lastScreenPos().y();
            if(qAbs(distance) > qAbs(yDistance)) {
                yDistance = distance;
            }
        }

        xDistance = qAbs(xDistance);
        yDistance = qAbs(yDistance);

        const qreal distance = xDistance >= yDistance ? xDistance : yDistance;
        int elapsedTime = q->d_func()->mTime.msecsTo(QTime::currentTime());
        if (!elapsedTime)
            elapsedTime = 1;
        q->d_func()->mSpeed = 0.9 * q->d_func()->mSpeed + distance / elapsedTime;
        q->d_func()->mTime = QTime::currentTime();
        q->setSwipeAngle(QLineF(q->d_func()->mInitialPoint, touchEvent->touchPoints().first().screenPos()).angle());

        static const int MoveThreshold = 5;
        if (!touchMoveEvent(touchEvent)) {
            if (q->state() != Qt::NoGesture) {
                result = QGestureRecognizer::TriggerGesture/* | QGestureRecognizer::ConsumeEventHint*/;
            } else {
                result = QGestureRecognizer::MayBeGesture/* | QGestureRecognizer::ConsumeEventHint*/;
            }
        }
        else if (xDistance > MoveThreshold || yDistance > MoveThreshold) {
            // measure the distance to check if the direction changed
            QSwipeGesture::SwipeDirection horizontal = QSwipeGesture::NoDirection;
            if(xDistance >= MoveThreshold)
                horizontal = touchEvent->touchPoints().first().screenPos().x() - touchEvent->touchPoints().first().lastScreenPos().x() > 0 ? QSwipeGesture::Right : QSwipeGesture::Left;
            QSwipeGesture::SwipeDirection vertical = QSwipeGesture::NoDirection;
            if(yDistance >= MoveThreshold)
                vertical = touchEvent->touchPoints().first().screenPos().y() - touchEvent->touchPoints().first().lastScreenPos().y() > 0 ? QSwipeGesture::Down : QSwipeGesture::Up;
            if ((yDistance > MoveThreshold && q->d_func()->mVerticalDirection != QSwipeGesture::NoDirection &&  q->d_func()->mVerticalDirection != vertical) ||
                (xDistance > MoveThreshold && q->d_func()->mHorizontalDirection != QSwipeGesture::NoDirection && q->d_func()->mHorizontalDirection != horizontal)) {
                // the user has changed the direction!
                result = QGestureRecognizer::CancelGesture;
            }
            else {
                result = QGestureRecognizer::TriggerGesture/* | QGestureRecognizer::ConsumeEventHint*/;
            }
            q->d_func()->mVerticalDirection = vertical;
            q->d_func()->mHorizontalDirection = horizontal;

        } else {
            if (q->state() != Qt::NoGesture)
                result = QGestureRecognizer::CancelGesture;
            else
                result = QGestureRecognizer::MayBeGesture/* | QGestureRecognizer::ConsumeEventHint*/;
        }
        q->d_func()->mTouchPointCount = touchPointCount;
        break;
    }
    default:
        result = QGestureRecognizer::Ignore;
        break;    
    }
    return result;
}

/*!

*/
QGestureRecognizer::Result HbSwipeGestureRecognizer::recognizeMouseEvent(HbSwipeGesture *q, QGraphicsSceneMouseEvent *mouseEvent)
{
    QGestureRecognizer::Result result;

    switch (mouseEvent->type()) {
    case QEvent::GraphicsSceneMousePress: {
        q->d_func()->mSpeed = 1;
        q->d_func()->mTime = QTime::currentTime();
        result = QGestureRecognizer::MayBeGesture;
        break;
    }
    case QEvent::GraphicsSceneMouseRelease: {
        if (q->state() != Qt::NoGesture) {
            result = QGestureRecognizer::FinishGesture;
            q->setProperty("speed", (int)(q->d_func()->mSpeed*KMilliSecsToSecs));
        } else {
            result = QGestureRecognizer::CancelGesture;
        }
        break;
    }
    case QEvent::GraphicsSceneMouseMove: {
        if(q->d_func()->mInitialMousePoint == QPoint()) {
            q->d_func()->mInitialMousePoint = mouseEvent->lastScreenPos();
        }
        
        q->setHotSpot(mouseEvent->screenPos());

        int xDistance = qAbs(mouseEvent->lastScreenPos().x() - mouseEvent->screenPos().x());
        int yDistance = qAbs(mouseEvent->lastScreenPos().y() - mouseEvent->screenPos().y());

        const int distance = xDistance >= yDistance ? xDistance : yDistance;
        int elapsedTime = q->d_func()->mTime.msecsTo(QTime::currentTime());
        if (!elapsedTime)
            elapsedTime = 1;
        q->d_func()->mSpeed = 0.9 * q->d_func()->mSpeed + distance / elapsedTime;
        q->d_func()->mTime = QTime::currentTime();
        q->setSwipeAngle(QLineF(QLine(q->d_func()->mInitialMousePoint, mouseEvent->screenPos())).angle());

        static const int MoveThreshold = 5;
        if (xDistance > MoveThreshold || yDistance > MoveThreshold) {
            // measure the distance to check if the direction changed
            QSwipeGesture::SwipeDirection horizontal = QSwipeGesture::NoDirection;
            if(xDistance >= MoveThreshold)
                horizontal = mouseEvent->screenPos().x() - mouseEvent->lastScreenPos().x() > 0 ? QSwipeGesture::Right : QSwipeGesture::Left;
            QSwipeGesture::SwipeDirection vertical = QSwipeGesture::NoDirection;
            if(yDistance >= MoveThreshold)
                vertical = mouseEvent->screenPos().y() - mouseEvent->lastScreenPos().y() > 0 ? QSwipeGesture::Down : QSwipeGesture::Up;
            if ((yDistance > MoveThreshold && q->d_func()->mVerticalDirection != QSwipeGesture::NoDirection &&  q->d_func()->mVerticalDirection != vertical) ||
                (xDistance > MoveThreshold && q->d_func()->mHorizontalDirection != QSwipeGesture::NoDirection && q->d_func()->mHorizontalDirection != horizontal)) {
                // the user has changed the direction!
                result = QGestureRecognizer::CancelGesture;
            }
            else {
                result = QGestureRecognizer::TriggerGesture;
            }
            q->d_func()->mVerticalDirection = vertical;
            q->d_func()->mHorizontalDirection = horizontal;

        } else {
            if (q->state() != Qt::NoGesture)
                result = QGestureRecognizer::CancelGesture;
            else
                result = QGestureRecognizer::MayBeGesture;
        }
        break;
    }
    default:
        result = QGestureRecognizer::Ignore;
        break;    
    }
    return result;
}

/*!

*/
void HbSwipeGestureRecognizer::reset(QGesture* state)
{
    HbSwipeGesture *q = qobject_cast<HbSwipeGesture *>(state);
    if(q) {
        q->d_func()->mVerticalDirection = QSwipeGesture::NoDirection;
        q->d_func()->mHorizontalDirection = QSwipeGesture::NoDirection;
        q->setSwipeAngle(0);

        q->d_func()->mInitialPoint = QPointF();
        q->d_func()->mInitialMousePoint = QPoint();
        q->d_func()->mSpeed = 0;
        q->d_func()->mTime = QTime();
        q->setProperty("speed", QVariant());
        q->d_func()->mTouchPointCount= 0;
    }
    QGestureRecognizer::reset(state);
}

/*!

*/
bool HbSwipeGestureRecognizer::touchMoveEvent(QTouchEvent *touchEvent)
{
    QTouchEvent::TouchPoint touchPoint;
    foreach (touchPoint, touchEvent->touchPoints()) {
        if (QLineF(touchPoint.screenPos(), touchPoint.lastScreenPos()).length() > 2) {
            return true;
        }
    }
    return false;
}

/*!

*/
QPointF HbSwipeGestureRecognizer::touchPointsCenterPoint(QTouchEvent *touchEvent)
{
    qreal x = 0;
    for(int i = 0; i < touchEvent->touchPoints().size(); i++) {
        x += touchEvent->touchPoints().at(i).screenPos().x();
    }
    x /= touchEvent->touchPoints().size();

    qreal y = 0;
    for(int i = 0; i < touchEvent->touchPoints().size(); i++) {
        y += touchEvent->touchPoints().at(i).screenPos().y();
    }
    y /= touchEvent->touchPoints().size();

    return QPointF(x, y);
}

// End of File
