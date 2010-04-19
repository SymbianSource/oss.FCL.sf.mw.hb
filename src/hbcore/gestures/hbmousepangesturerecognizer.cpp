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


#include "hbmousepangesturerecognizer_p.h"

#include <QEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGesture>
#include <QVariant>

HbMousePanGestureRecognizer::HbMousePanGestureRecognizer()
{
}

HbMousePanGestureRecognizer::~HbMousePanGestureRecognizer()
{
}

QGesture* HbMousePanGestureRecognizer::create(QObject *)
{
    return new QPanGesture;
}

QGestureRecognizer::Result HbMousePanGestureRecognizer::recognize(QGesture *state, QObject *, QEvent *event)
{
    QPanGesture *g = static_cast<QPanGesture *>(state);
    QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(event);
    QPoint pos;

    switch(event->type()) {
    case QEvent::GraphicsSceneMousePress:
        g->setOffset(QPointF(0,0));
        g->setProperty("startPos", me->screenPos());
        g->setProperty("pressed", QVariant::fromValue<bool>(true));
    case QEvent::GraphicsSceneMouseMove:
    case QEvent::GraphicsSceneMouseRelease:
        pos = me->screenPos();
        g->setHotSpot(pos);
        g->setLastOffset(g->offset());
        break;
    default:
        return QGestureRecognizer::Ignore;
    }

    if (event->type() == QEvent::GraphicsSceneMousePress || event->type() == QEvent::GraphicsSceneMouseDoubleClick) {
        return QGestureRecognizer::MayBeGesture;
    } else if (event->type() == QEvent::GraphicsSceneMouseMove) {
        if (g->property("pressed").toBool()) {
            QPoint offset = pos - g->property("startPos").toPoint();
            g->setOffset(offset);
            return QGestureRecognizer::TriggerGesture;
        }
        return QGestureRecognizer::CancelGesture;
    } else if (event->type() == QEvent::GraphicsSceneMouseRelease) {
        if(g->state() >= Qt::GestureStarted) {
            QPoint offset = pos - g->property("startPos").toPoint();
            g->setOffset(offset);
            g->setProperty("pressed", QVariant::fromValue<bool>(false));
            return QGestureRecognizer::FinishGesture;
        } else {
            return QGestureRecognizer::CancelGesture;
        }

    }
    return QGestureRecognizer::Ignore;
}

void HbMousePanGestureRecognizer::reset(QGesture *state)
{
    QPanGesture *g = static_cast<QPanGesture *>(state);
    g->setLastOffset(QPointF());
    g->setOffset(QPointF(0,0));
    g->setAcceleration(0);
    g->setProperty("startPos", QVariant());
    g->setProperty("pressed", QVariant::fromValue<bool>(false));
    QGestureRecognizer::reset(state);
}
