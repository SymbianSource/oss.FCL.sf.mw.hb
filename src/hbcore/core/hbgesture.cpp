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

#include "hbgesture.h"
#include "hbgesture_p.h"
#include <hbinstance.h>
#include <hbwidget.h>

#include "hbglobal_p.h"

/*!
//
//  W A R N I N G
//  -------------
//
// This implementation of Gesture filter is most probably removed in later releases.
// It exists purely as an implementation detail.
// This implementation may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
*/

/*!
	@proto
    @hbcore
	\class HbGesture
    \brief HbGesture is a class that is used to register gestures to HbGestureFilter.

    HbGesture itself only contains very little, it has callback functions that are used
	to emit "triggered" and "panned" signals and basic constructors and destructors.

	Normally user connects to either triggered() or panned() signals and handles in the
	application	what is the desired result of those signals.

    Typically user defines the gestures in the \a itemChange() function if it is defined.
	The filter needs to be installed for every GraphicsItem.

	Example of registering new gestures:
    \snippet{itemviews/hblistview.cpp,1}

	\a GestureFilter uses these Gestures to compare user inputs and when match is detected,
	appropriate callback function is called. Unmatched user input is propagated onwards.

    HbGesture currently supports the following types of gestures:
     - Hb::up
     - Hb::down
     - Hb::left
     - Hb::right
     - Hb::pan
     - Hb::longpress
*/

/*!
    \fn void HbGesture::triggered( int speed );

    This signal is emitted when the when gesture matches with a flick gesture.
*/

/*!
    \fn void HbGesture::panned( QPointF delta );

    This signal is emitted when the when gesture matches with a pan gesture.
*/

/*!
    \fn void HbGesture::longPress( QPointF delta )

    This signal is emitted when the when gesture matches with a longPress gesture.
*/

/*!
  \deprecated HbGesture::HbGesture(HbGesture::Direction, int, QObject*)
       is deprecated.

	Constructor that requires three parameters, direction, minimum distance
	and parent object.
	\a Direction defines the direction for the gesture. \a minDistance defines
	the minimum pixel distance for the gesture. \a  Parent is the parent object.
 */
HbGesture::HbGesture( Direction direction, int minDistance, QObject *parent ) :
    QObject( parent ), d(new HbGesturePrivate)
{
    HB_DEPRECATED("HbGesture::HbGesture is deprecated. Use Qt gesture fw instead.");
    d->mDirection = direction;

    if ( minDistance == HbGestureDefaultMinDistance ) {
        // Same hardcoded value is used for all resolutions here.
        // This could be also defined in UI metrics, but then also metrics change signals
        // would need to be listened in this class and the value updated upon them.
        // Currently, this is not considered worth doing since there will be Gesture FW coming in QT,
        // which will replace this implementation.
        minDistance = 15;
    }

    d->mMinDistance = minDistance;
}

/*!
	Destructs the gesture.
 */
HbGesture::~HbGesture()
{
    delete d;
}

/*!
	Called by the gesture filter when a gesture match occurs.
	\a Speed is the relative speed of the gesture.
 */
void HbGesture::callback( int speed )
{
    emit triggered( speed );
}

/*!
	Called by the gesture filter when a gesture match occurs.
	\a Delta is the direction and distance of the gesture.
 */
void HbGesture::panCallback( QPointF delta )
{
    emit panned( delta );
}

/*!
	Called by the gesture filter when a gesture match occurs.
	\a Delta is the direction and distance of the gesture.
 */
void HbGesture::longPressCallback( QPointF delta )
{

    // And emit singal
    emit longPress( delta );
}

/*!
	Returns the direction of the gesture
 */
HbGesture::Direction HbGesture::direction() const
{
    return d->mDirection;
}

/*!
	Returns the minimum pixel distance for the gesture
 */
int HbGesture::minDistance() const
{
    return d->mMinDistance;
}

