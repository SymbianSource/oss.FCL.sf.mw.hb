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

#include "hbgestures_p.h"

#include <QPointF>

#include <QDebug>
//#define VELOCITY_DEBUG
#ifndef VELOCITY_DEBUG
# define DEBUG if (0) qDebug
#else
# define DEBUG qDebug
#endif

/*!
   @hbcore
   \internals
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
QPointF HbVelocityCalculator::velocity( qint64 currentTime ) const
{
    QPointF velocity(0.0, 0.0);

    velocity.setX(calculate_velocity(mListX, currentTime));
    velocity.setY(calculate_velocity(mListY, currentTime));

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
        qint64 currentTime) const
{
    if (list.count() < 2) {
        return 0.0;
    }

    if (currentTime-list.lastTime() >= HbVelocityStopTime) {
        return 0.0;
    }

    // Accumulate the distance from previous point until we have sufficient sample
    qreal delta = 0.0;
    qint64 timeDelta = 0;
    int i = list.count();
    while (timeDelta < HbVelocitySampleTime && i > 0) {
        i--;
        timeDelta = currentTime - list.at(i).second;
    }
    if(timeDelta <= 0) {
        return 0.0;
    }

    delta = list.lastPoint() - list.at(i).first;

    return delta / (qreal)(timeDelta);
}
