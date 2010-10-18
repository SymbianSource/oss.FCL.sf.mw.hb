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

#ifndef HBGESTURES_P_H
#define HBGESTURES_P_H

#include "hbtapgesture.h"
#include "hbpointrecorder_p.h"

#include "hbglobal.h"

#include <QPointF>
#include <QObject>
#include <QGraphicsView>

#if QT_VERSION >= 0x040700
#include <QElapsedTimer>
#define ELAPSED_TIMER QElapsedTimer
#else
#include <QTime>
#define ELAPSED_TIMER QTime
#endif

const qreal HbDefaultPanThreshold = 1.0; // mm
const qreal HbPanVelocityUpdateThreshold = 3.3; // mm

const qreal HbDefaultTapRadius = 3.3; //mm

const qreal HbSwipeMinOffset = 7.5; // mm
const qreal HbSwipeMinSpeed = 0.06; // mm / ms

const int HbVelocitySampleTime = 80; // ms
const int HbVelocityStopTime = 70; // ms

const int HbTapAndHoldTimeout = 800; // ms
class HbGestureUtils
{
public:
    /*!
        \internal
        \brief
        \return

    */
    static QPointF mapToScene( QObject* watched, const QPointF &pos )
    {
        QGraphicsView* view = qobject_cast<QGraphicsView*>(watched->parent());

        if ( view )
        {
            return view->mapToScene(view->mapFromGlobal(pos.toPoint()));
        }

        return QPointF();
    }       
};

class HB_CORE_PRIVATE_EXPORT HbPanGesturePrivate
{
public:
    QPointF mStartPos;

    QPointF mSceneStartPos;
    QPointF mSceneLastOffset;
    QPointF mSceneOffset;

    // for the recognizer
    QPointF mDeltaSinceLastTimeStamp;
    QPointF mSceneDeltaSinceLastTimeStamp;
    qint64 mLastTimeStamp;

    HbPointRecorder mAxisX;
    HbPointRecorder mAxisY;
    HbPointRecorder mSceneAxisX;
    HbPointRecorder mSceneAxisY;

    qreal mThresholdSquare;

    bool mIgnoreMouseEvents;
    int mFollowedTouchPointId;

    ELAPSED_TIMER mTime;
};

class HB_CORE_PRIVATE_EXPORT HbPinchGesturePrivate
{
public:
    bool mIsNewSequence;

    qreal mSceneTotalRotationAngle;
    qreal mSceneLastRotationAngle;
    qreal mSceneRotationAngle;

    QPointF mSceneStartCenterPoint;
    QPointF mSceneLastCenterPoint;
    QPointF mSceneCenterPoint;
};

class HB_CORE_PRIVATE_EXPORT HbSwipeGesturePrivate
{
public:
    QPointF mStartPos;
    QPointF mSceneStartPos;

    qint64 mStartTime;

    qreal mSceneSwipeAngle;

    HbPointRecorder mAxisX;
    HbPointRecorder mAxisY;

    ELAPSED_TIMER mTime;
};


class HB_CORE_PRIVATE_EXPORT HbTapAndHoldGesturePrivate
{
public:
    HbTapAndHoldGesturePrivate() : mTimerID(0) {}

    QPointF mScenePos;
    int mRunningTime;
    int mTimerID;
};

class HB_CORE_PRIVATE_EXPORT HbTapGesturePrivate
{
public:
    HbTapGesturePrivate() : mTimerId(0) {}

    QPointF mStartPos;
    QPointF mSceneStartPos;
    QPointF mScenePosition;
    HbTapGesture::TapStyleHint mTapStyleHint;
    int mTimerId;
};

#endif // HBGESTURES_P_H
