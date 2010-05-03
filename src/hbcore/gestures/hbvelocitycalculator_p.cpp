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

#include "hbvelocitycalculator_p.h"
#include "hbpointrecorder_p.h"

#include <QPointF>
#include <QTime>

#include <QDebug>
//#define VELOCITY_DEBUG
#ifndef VELOCITY_DEBUG
# define DEBUG if (0) qDebug
#else
# define DEBUG qDebug
#endif

// Number of points through considered relevant to calculate speed.
const int KHbPoints = 4;

// Number to used to adjust the speed to look better for eyes. Simulates mass effect.
const qreal KHbAdjust = 4.0;

const int KHbSampleTime = 80; // ms
const int KHbStopTime = 30; // ms

/*!
   @hbcore
   \internal
   \class HbVelocityCalculator

   \brief Class to calculate velocity from point.

*/

/*!
    \internal
    \brief
    \return

*/
HbVelocityCalculator::HbVelocityCalculator(
    const HbPointRecorder& listX,
    const HbPointRecorder& listY )
    :
    mListX( listX ),
    mListY( listY )
{
}

/*!
    \internal
    \brief
    \return

*/
QPointF HbVelocityCalculator::velocity( const QTime& time ) const
{
    QPointF velocity(0.0, 0.0);

    velocity.setX(calculate_velocity(mListX, time));
    velocity.setY(calculate_velocity(mListY, time));

    DEBUG() << "Velocity: " << velocity;

    return velocity;
}

/*!
    \internal
    \brief
    \return

*/
qreal HbVelocityCalculator::calculate_velocity(
    const HbPointRecorder &list,
    const QTime& time ) const
{
    Q_UNUSED(time)

    // Accumulate the distance from previous point until we have sufficient sample
    qreal delta = 0.0;
    int timeDelta = 0;
    if (!(list.mTimes.count() > 1 && list.mPoints.count() > 1) ||
        (list.mTimes.last().msec()-time.msec() > KHbStopTime)) {
        return 0.0;
    }
    int i = list.mTimes.count();
    while (timeDelta < KHbSampleTime && i > 0) {
        i--;
        timeDelta = time.msec() - list.mTimes.at(i).msec();
    }
    delta = list.mPoints.last() - list.mPoints.at(i);

    return delta / (qreal)(list.mTimes.at(i).msecsTo(time));
    /*// Make decisions based on the last few points.
    QList<int> points = list.getLastPoints( KHbPoints );
    QList<QTime> times = list.getLastTimes( KHbPoints );
    qreal velocity = 0.0;

    // In case of empty list or in case the sizes of the list are different
    // consider this movement being stopped.
    if ( !points.length() || points.length() != times.length() )
    {
        DEBUG() << "Cancelling velocity calculation, because points.length() == " << points.length() << " and times.length() == " << times.length();
        return velocity;

    }
    else
    {
        DEBUG() << "Number of points recorded: " << points.length();
    }

    // Sum the velocities between timedeltas to get the final speed.
    qreal avg_dt = 0.0;
    for ( int i = 0; i<points.length()-1; i++)
    {
        qreal t = times[i].msecsTo(times[i+1]);
        avg_dt += t;
        velocity += ( points[i+1] - points[i] ) / t;
        velocity /= 2.0;
    }
    avg_dt /= times.length();

    // Calculate time between release and last update point.
    qreal dt = list.lastTime().msecsTo(time);

    if ( !dt ) { dt = avg_dt; }

    // Scale the velocity correctly and adjust it with magic.
    return velocity * dt / KHbAdjust;*/
}
