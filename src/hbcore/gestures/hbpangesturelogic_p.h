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

#ifndef HBPANGESTURELOGIC_P_H
#define HBPANGESTURELOGIC_P_H

#include "hbglobal.h"

#include <QEvent>
#include <QGestureRecognizer>
#include <QList>
#include <QTouchEvent>

class QPoint;
class HbPanGesture;
class QMouseEvent;

class HB_CORE_PRIVATE_EXPORT HbPanGestureLogic
{
public:
    HbPanGestureLogic();
    ~HbPanGestureLogic();

    bool isMouseEvent(QEvent::Type eventType);
    bool isTouchEvent(QEvent::Type eventType);
    void resetGesture(HbPanGesture *gesture);
    QTouchEvent::TouchPoint followedTouchPoint(QTouchEvent *te, HbPanGesture *gesture);
    QTouchEvent::TouchPoint getNextTouchPoint(QTouchEvent *te);
    QGestureRecognizer::Result handlePress(
            Qt::GestureState gestureState,
            HbPanGesture *gesture,
            QObject *watched,
            const QPointF &globalPos,
            qint64 currentTime );
    QGestureRecognizer::Result handleMove(
            Qt::GestureState gestureState,
            HbPanGesture *gesture,
            QObject *watched,
            const QPointF &globalPos,
            qint64 currentTime );
    QGestureRecognizer::Result handleRelease(
            Qt::GestureState gestureState,
            HbPanGesture *gesture,
            QObject *watched,
            const QPointF &globalPos,
            qint64 currentTime );
    QGestureRecognizer::Result recognize(
            Qt::GestureState gestureState,
            HbPanGesture *gesture,
            QObject *watched,
            QEvent *event,
            qint64 currentTime);
};

#endif // HBPANGESTURELOGIC_P_H
