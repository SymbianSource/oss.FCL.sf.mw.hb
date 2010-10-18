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

#include "hbpinchgesture.h"
#include "hbpinchgesturelogic_p.h"
#include "hbgestures_p.h"

#include <QEvent>
#include <QGestureRecognizer>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QDebug>


/*!
   @hbcore
   \internal
   \class HbPinchGestureLogic

   \brief

*/


/*!
    \internal
    \brief
    \return

*/
HbPinchGestureLogic::HbPinchGestureLogic()
{
}

HbPinchGestureLogic::~HbPinchGestureLogic() {}

/*!
    \internal
    \brief
    \return

*/
void HbPinchGestureLogic::resetGesture(HbPinchGesture *gesture)
{
    HbPinchGesturePrivate *d = gesture->d_func();

    gesture->setTotalChangeFlags(0);
    gesture->setChangeFlags(0);

    gesture->setStartCenterPoint(QPointF());
    gesture->setLastCenterPoint(QPointF());
    gesture->setCenterPoint(QPointF());

    d->mSceneStartCenterPoint = QPointF();
    d->mSceneLastCenterPoint = QPointF();
    d->mSceneCenterPoint = QPointF();

    gesture->setTotalScaleFactor(1);
    gesture->setLastScaleFactor(1);
    gesture->setScaleFactor(1);

    gesture->setTotalRotationAngle(0);
    gesture->setLastRotationAngle(0);
    gesture->setRotationAngle(0);

    d->mSceneTotalRotationAngle = 0;
    d->mSceneLastRotationAngle = 0;
    d->mSceneRotationAngle = 0;

    d->mIsNewSequence = true;
}

/*!
    \internal
    \brief
    \return

*/
QPointF HbPinchGestureLogic::mapToScene( QObject* watched, const QPointF &pos )
{
    QGraphicsView* view = qobject_cast<QGraphicsView*>(watched->parent());

    if ( view )
    {
        return view->mapToScene(view->mapFromGlobal(pos.toPoint()));
    }

    return QPointF();
}

/*!
    \internal
    \brief
    \return

*/
QGestureRecognizer::Result HbPinchGestureLogic::recognize(Qt::GestureState gestureState,
                                                          HbPinchGesture *gesture,
                                                          QObject *watched,
                                                          QEvent *event)
{
    HbPinchGesturePrivate *d = gesture->d_func();

    if (!watched->isWidgetType()) {
        return QGestureRecognizer::Ignore;
    }

    const QTouchEvent *ev = static_cast<const QTouchEvent *>(event);

    QGestureRecognizer::Result result;

    switch (event->type()) {
    case QEvent::TouchBegin: {
            result = QGestureRecognizer::MayBeGesture;
            break;
        }
    case QEvent::TouchEnd: {
            if (gestureState != Qt::NoGesture) {
                result = QGestureRecognizer::FinishGesture;
            } else {
                result = QGestureRecognizer::CancelGesture;
            }
            break;
        }
    case QEvent::TouchUpdate: {
            gesture->setChangeFlags(0);
            if (ev->touchPoints().size() == 2) {
                QTouchEvent::TouchPoint p1 = ev->touchPoints().at(0);
                QTouchEvent::TouchPoint p2 = ev->touchPoints().at(1);

                gesture->setHotSpot(p1.screenPos());

                QLineF line(p1.screenPos(), p2.screenPos());
                QLineF tmp(line);
                tmp.setLength(line.length() / 2.);
                QPointF centerPoint = tmp.p2();

                if (p1.state() == Qt::TouchPointPressed ||
                    p2.state() == Qt::TouchPointPressed) {
                    gesture->setStartCenterPoint(centerPoint);
                    d->mSceneStartCenterPoint = mapToScene(watched, centerPoint);

                    result = QGestureRecognizer::MayBeGesture;

                    // start recording
                    break;
                }

                QLineF currentPinchPos = QLineF(p1.screenPos(), p2.screenPos());
                //qDebug() << "distance" << currentPinchPos.length();
                //qDebug() << "angle" << currentPinchPos.angle();
                //qDebug() << "center point" << centerPoint;

                gesture->setLastCenterPoint(gesture->centerPoint());
                d->mSceneLastCenterPoint = mapToScene(watched, gesture->centerPoint());
                gesture->setCenterPoint(centerPoint);
                d->mSceneCenterPoint = mapToScene(watched, centerPoint);
                gesture->setChangeFlags(gesture->changeFlags() |= QPinchGesture::CenterPointChanged);

                const qreal scaleFactor =
                        QLineF(p1.screenPos(), p2.screenPos()).length()
                        / QLineF(p1.startScreenPos(),  p2.startScreenPos()).length();
                if (d->mIsNewSequence) {
                    gesture->setLastScaleFactor(scaleFactor);
                } else {
                    gesture->setLastScaleFactor(gesture->scaleFactor());
                }
                gesture->setScaleFactor(scaleFactor);
                gesture->setTotalScaleFactor(gesture->totalScaleFactor() + gesture->scaleFactor() - gesture->lastScaleFactor());
                gesture->setChangeFlags(gesture->changeFlags() |= QPinchGesture::ScaleFactorChanged);

                qreal angle = QLineF(p1.screenPos(), p2.screenPos()).angle();
                if (angle > 180) {
                    angle -= 360;
                }
                qreal sceneAngle = QLineF(p1.scenePos(), p2.scenePos()).angle();
                if (sceneAngle > 180) {
                    sceneAngle -= 360;
                }
                qreal startAngle = QLineF(p1.startScreenPos(), p2.startScreenPos()).angle();
                if (startAngle > 180) {
                    startAngle -= 360;
                }
                qreal sceneStartAngle = QLineF(p1.startScenePos(), p2.startScenePos()).angle();
                if (sceneStartAngle > 180) {
                    sceneStartAngle -= 360;
                }
                const qreal rotationAngle = startAngle - angle;
                const qreal sceneRotationAngle = sceneStartAngle - sceneAngle;
                if (d->mIsNewSequence) {
                    gesture->setLastRotationAngle(rotationAngle);
                    d->mSceneLastRotationAngle = sceneRotationAngle;
                }
                else {
                    gesture->setLastRotationAngle(gesture->rotationAngle());
                    d->mSceneLastRotationAngle = d->mSceneRotationAngle;
                }
                gesture->setRotationAngle(rotationAngle);
                d->mSceneRotationAngle = sceneRotationAngle;
                gesture->setTotalRotationAngle(gesture->totalRotationAngle() + gesture->rotationAngle() - gesture->lastRotationAngle());
                d->mSceneTotalRotationAngle = d->mSceneTotalRotationAngle + d->mSceneRotationAngle - d->mSceneLastRotationAngle;
                gesture->setChangeFlags(gesture->changeFlags() |= QPinchGesture::RotationAngleChanged);
                gesture->setTotalChangeFlags(gesture->totalChangeFlags() |= gesture->changeFlags());
                d->mIsNewSequence = false;
                result = QGestureRecognizer::TriggerGesture;
            } else {
                d->mIsNewSequence = true;
                if (gestureState == Qt::NoGesture)
                    result = QGestureRecognizer::Ignore;
                else
                    result = QGestureRecognizer::FinishGesture;
            }
            break;
        }
    case QEvent::MouseButtonPress:
    case QEvent::MouseMove:
    case QEvent::MouseButtonRelease:
        result = QGestureRecognizer::Ignore;
        break;
    default:
        result = QGestureRecognizer::Ignore;
        break;
    }
    return result;
}
