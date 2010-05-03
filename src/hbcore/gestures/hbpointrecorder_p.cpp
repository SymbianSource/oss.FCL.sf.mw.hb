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

#include "hbpointrecorder_p.h"

#include <QDebug>
//#define VELOCITY_DEBUG
#ifndef VELOCITY_DEBUG
# define DEBUG if (0) qDebug
#else
# define DEBUG qDebug
#endif

/*!
   @hbcore
   \internal
   \class HbPointRecorder

   \brief Class to store and hold list of points and timestamps.

*/

/*!
    \internal
    \brief Constructor for HbPointRecorder
    \return

*/
HbPointRecorder::HbPointRecorder()
{
}

/*!
    \internal
    \brief Destructor for HbPointRecorder
    \return

*/
HbPointRecorder::~HbPointRecorder()
{
}

/*!
    \internal
    \brief Records point to list with timestamp.
    \param point Point to be recorded.
    \param time Time to be recorded.
    \return Nothing.

*/
void HbPointRecorder::record(qreal point, QTime time)
{
    // No point to record a point, if timestamp is less or equal with previous.
    if ( !isEmpty() && lastTime().msecsTo(time) == 0 )
    {
        DEBUG() << "Ignoring point, because no difference in time stamps.";
        return;
    }

    // In case the list contains two or more points, direction can be
    // determined. Each new point added needs to be checked for direction
    // change.
    if ( mPoints.length() > 1 )
    {
        // Clear list, on direction change. Leave the last recorded point
        // to the list, as it can be considered as first point for new direction.
        if ( dirChanged( point ) )
        {
            qreal tempPoint = lastPoint();
            QTime tempTime = lastTime();

            clear();

            mPoints.append( tempPoint );
            mTimes.append( tempTime );
        }
    }

    // Finally check, if the position has changed. Don't record point, when no position
    // change.
    if ( isEmpty() || point != lastPoint() )
    {
        // Add point and time to list.
        mPoints.append( point );
        mTimes.append( time );
    }
    else
    {
        DEBUG() << "Ignoring point, because it equals previous.";
    }
}

/*!
    \internal
    \brief
    \return True, when no recorded items.

*/
bool HbPointRecorder::isEmpty() const
{
    // This situation should be impossible to even happen, but in case
    // the lists are out of sync. The result of empty list dictates the
    // result of this test.
    return !mTimes.length() && !mPoints.length();
}

/*!
    \internal
    \brief
    \return Last recorded point.

*/
qreal HbPointRecorder::lastPoint() const
{
    Q_ASSERT(!isEmpty());
    return mPoints.last();
}

/*!
    \internal
    \brief
    \return Last recorded timestamp.

*/
const QTime& HbPointRecorder::lastTime() const
{
    Q_ASSERT(!isEmpty());
    return mTimes.last();
}

/*!
    \internal
    \brief Checks if new point causes direction change.
    \param point The point suspected cause direction change.
    \return True, when direction changes.
*/
bool HbPointRecorder::dirChanged( qreal point ) const
{
    qreal x0 = mPoints.at(mPoints.length()-2);
    qreal x1 = mPoints.at(mPoints.length()-1);
    qreal dir0 = x1 - x0;
    qreal dir1 = point - x1;

    // Check for '+' and '-' -signs in directions. Opposite signs means
    // direction change.
    return ( dir0 < 0 && dir1 >= 0 ) || ( dir0 > 0 && dir1 <= 0 );
}

/*!
    \internal
    \param T type of items in the list.
    \brief Returns given number of items from the end.
    \return List of items.

    This function gets items from the given list from the end by the
    amount of items specified by 'number' parameter. In case, when the
    length of the list is less than required number of items, complete
    list is provided instead.
*/
template <class T>
QList<T> HbPointRecorder::getLastItems( QList<T> list, int number ) const
{
    if ( list.length() <= number )
    {
        return list;
    }
    else
    {
        QList<T> tempList;
        for ( int i = list.length(); --i >= list.length()-number; )
        {
            tempList.insert(0, list.at(i));
        }
        return tempList;
    }
}

/*!
    \internal
    \copydoc HbPointRecorder::getLastItems
*/
QList<qreal> HbPointRecorder::getLastPoints( int number ) const
{
    return getLastItems(mPoints, number);
}

/*!
    \internal
    \copydoc HbPointRecorder::getLastItems
*/
QList<QTime> HbPointRecorder::getLastTimes( int number ) const
{
    return getLastItems(mTimes, number);
}

/*!
    \internal
    \brief
    \return Last recorded timestamp.

*/
void HbPointRecorder::clear()
{
    mPoints.clear();
    mTimes.clear();
}
