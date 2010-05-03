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

#ifndef HBGESTUREFILTER_P_H
#define HBGESTUREFILTER_P_H

#include "hbgesturefilter.h"
#include "hbgesture.h"
#include <QTime>
#include <QPointer>
#include <QGraphicsWidget>

QT_BEGIN_NAMESPACE
class QTimeLine;
QT_END_NAMESPACE

typedef QList<HbGesture*> GestureList;

class HbGestureFilterPrivate
{
public:
    HbGestureFilterPrivate(Qt::MouseButton button);
    ~HbGestureFilterPrivate();

public:
    GestureList gestures;
    Qt::MouseButton button;
    QPoint pressPoint;
    QTime pressTime;
    QPointF touchDownScenePos;
    QPointF panLastScenePos;
    QTime gestureTimer;
    HbGesture::Direction allGestures;
    QTimeLine *longPressTimer;
    QTimeLine *longPressDelayTimer;
};

class HbGestureSceneFilterPrivate
{
public:
    HbGestureSceneFilterPrivate(Qt::MouseButton button);
    ~HbGestureSceneFilterPrivate();
public:
    HbGestureFilterPrivate p;
    QPointer<QGraphicsWidget> widget;
    bool panning;
    bool mouseReleased;
    bool longPressDelayCancelled;
    bool longPressCompleted;
    int panThreshold;
    bool withinThreshold;
};

#endif // HBGESTUREFILTER_P_H

