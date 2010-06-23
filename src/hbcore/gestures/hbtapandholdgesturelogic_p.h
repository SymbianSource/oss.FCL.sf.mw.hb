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

#ifndef HBTAPANDHOLDGESTURELOGIC_P_H
#define HBTAPANDHOLDGESTURELOGIC_P_H

#include "hbgestures_p.h"

#include <QObject>
#include <QGestureRecognizer>
#include <hbglobal.h>

class QEvent;
class QMouseEvent;
class QTimerEvent;
class HbTapAndHoldGesture;

class HB_CORE_PRIVATE_EXPORT HbTapAndHoldGestureLogic
{
public:
    HbTapAndHoldGestureLogic();
    ~HbTapAndHoldGestureLogic();

    bool outsideThreshold(HbTapAndHoldGesture *gesture);

    void resetGesture(HbTapAndHoldGesture *gesture);
    QGestureRecognizer::Result handleMousePress(
        Qt::GestureState gestureState, HbTapAndHoldGesture *gesture, QObject *watched, QMouseEvent *me );
    QGestureRecognizer::Result handleMouseMove(
        Qt::GestureState gestureState, HbTapAndHoldGesture *gesture, QObject *watched, QMouseEvent *me );
    QGestureRecognizer::Result handleMouseRelease(
        Qt::GestureState gestureState, HbTapAndHoldGesture *gesture, QObject *watched, QMouseEvent *me );
    QGestureRecognizer::Result handleTimer(
        Qt::GestureState gestureState, HbTapAndHoldGesture *gesture, QObject *watched, QTimerEvent* te);
    QGestureRecognizer::Result recognize(
        Qt::GestureState gestureState, HbTapAndHoldGesture *gesture, QObject *watched, QEvent* event );

    int mTapRadius;
};

#endif // HBTAPANDHOLDGESTURELOGIC_P_H
