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

#include "hbtapgesturerecognizer_p.h"
#include "hbtapgesture.h"

#include <QEvent>
#include <QGraphicsSceneMouseEvent>
#include <QMouseEvent>
#include <QGesture>
#include <QDebug>

/*!
    \internal
    \class HbTapGestureRecognizer

    \brief HbTapGestureRecognizer implements a gesture recognizer for simple taps.

*/

/*!
    \internal
    \param tapRadius maximum allowed radius for jitter
*/
HbTapGestureRecognizer::HbTapGestureRecognizer(int tapRadius) : mTapRadius(tapRadius)
{
}

/*!
    \internal

*/
HbTapGestureRecognizer::~HbTapGestureRecognizer()
{
}

/*!
    \internal

    \reimp
*/
QGesture *HbTapGestureRecognizer::create(QObject *)
{
    QGesture *gesture = new HbTapGesture;
    return gesture;
}

/*!
    \internal

    \property tapRadius

    Unsupported feature: Setting a tapRadius property for the received HbTapGesture overrides
    the default tap radius.  Using this will invoke a warning.
*/

/*!
    \internal

    \reimp

*/
QGestureRecognizer::Result HbTapGestureRecognizer::recognize(QGesture *state, QObject *, QEvent *event)
{
    HbTapGesture *tap = static_cast<HbTapGesture *>(state);
    QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent*>(event);

    QGestureRecognizer::Result result = QGestureRecognizer::CancelGesture;

    if(event->type() == QEvent::TouchBegin || event->type() == QEvent::TouchUpdate) {
        QTouchEvent *te = static_cast<QTouchEvent*>(event);
        if(te->touchPoints().count() > 1) {
            // Cancel tap on multiple fingers
            return result;
        }
    }


    int tapRadius(mTapRadius);
    if(tap->property("tapRadius").isValid()) {
        qWarning("WARNING using widget specific properties in HbTapGestureRecognizer");
        tapRadius = tap->property("tapRadius").toInt();
    }

    switch(event->type()) {
    case QEvent::GraphicsSceneMousePress:
        tap->setPosition(me->screenPos());
        tap->setHotSpot(me->screenPos());
        tap->setStartPos(me->screenPos());
        result = QGestureRecognizer::TriggerGesture;
        break;
    case QEvent::GraphicsSceneMouseMove:
    case QEvent::GraphicsSceneMouseRelease: {
            if(tap->state() != Qt::NoGesture) {
                QPointF delta = me->screenPos() - tap->startPos();
                tap->setPosition(me->screenPos());
                tap->setHotSpot(me->screenPos());
                if(delta.manhattanLength() <= tapRadius) {
                    if(event->type() == QEvent::GraphicsSceneMouseRelease) {
                        result = QGestureRecognizer::FinishGesture;
                    } else {
                        result = QGestureRecognizer::TriggerGesture;
                    }
                }
            }
        }
        break;
    default:
        result = QGestureRecognizer::Ignore;
        break;
    }

    return result;
}

/*!
    \internal

*/
void HbTapGestureRecognizer::reset(QGesture *state)
{
    HbTapGesture *tap = qobject_cast<HbTapGesture *>(state);
    if(tap) {
        tap->setStartPos(QPointF());
        tap->setProperty("tapRadius", QVariant());
    }
    QGestureRecognizer::reset(state);
}
