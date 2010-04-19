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

#include "hbtapandholdgesture_p.h"
#include "hbtapandholdgesture.h"

#include <QGraphicsSceneEvent>
#include <QVariant>
#include <QDebug>
#include <QPoint>
#include <QLine>

/*!
    \internal
    \class HbTapAndHoldGesture

    \brief HbTapAndHoldGesture implements a gesture for tap and hold.
*/

HbTapAndHoldGesture::HbTapAndHoldGesture(QObject* parent)
    :
    QTapAndHoldGesture(parent),
    priv(new HbTapAndHoldGesturePrivate())
{
}

HbTapAndHoldGesture::~HbTapAndHoldGesture()
{
    delete priv; priv = NULL;
}

/*!
    \internal
    \brief Stores relevant values from the event.
    \param event Event to be read.

    Gesture needs to know its position all the time, and that information
    is provided during the event.
*/
void HbTapAndHoldGesture::update(QEvent& event)
{
    if ( event.type() != QEvent::Timer )
    {
        QGraphicsSceneMouseEvent* me = static_cast<QGraphicsSceneMouseEvent*>(&event);
        setProperty("position", me ? me->screenPos() : property("startPos"));
    }
}

bool HbTapAndHoldGesture::outsideThreshold()
{
    QPointF startPos = property("startPos").toPoint();
    QPointF lastPos = property("position").toPoint();
    return QLineF(startPos, lastPos).length() > DELTA_TOLERANCE;
}
