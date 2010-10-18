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

#include "hbswipegesture.h"
#include "hbgestures_p.h"
#include "hbglobal_p.h"
#include <QPointF>
#include <QVariant>

/*!
    @hbcore
    \class HbSwipeGesture

    \brief The HbSwipeGesture class provides support for receiving a swipe
    (flick) gesture.
    
    HbSwipeGesture is an extension to Qt standard QSwipeGesture. It is
    optimized for mobile touch screens, and also supports recognition of
    mouse events for development purposes. Moreover, HbSwipeGesture
    has convenience functions for handling the QSwipeGesture properties
    (horizontalDirection, verticalDirection, swipeAngle) directly in scene
    coordinates. They remove the need for any manual conversions from the
    screen (global) coordinates offered by the QSwipeGesture base class
    properties.
    
    The swipe gesture is designed to be used as a single-shot gesture which
    is activated after a flick and release. No events are sent until the
    gesture is finished. Swipe should be used when feedback during the
    gesture is not possible or desired.

    \section _usecases_hbswipegesture Using the HbSwipeGesture class
    
    This example shows how to make a custom widget recognize the swipe
    gesture. The custom widget in the example derives from HbWidget.
    
    <ol>
    <li>
    Register for swipe gestures by using the base class function
    QGraphicsObject::grabGesture(). QGraphicsObject::grabGesture() can be
    called several times with different arguments, if the widget is
    interested in other gesture types as well.
   
    \code
    // This widget is interested in swipe and tap gestures.
    grabGesture(Qt::SwipeGesture);
    grabGesture(Qt::TapGesture);
    \endcode
    </li>
    
    <li>
    Reimplement HbWidgetBase::gestureEvent() to handle gestures that are
    meaningful for your custom widget.
   
    \code
    void MyWidget::gestureEvent(QGestureEvent *event)
    {
        if (HbSwipeGesture *swipe = qobject_cast<HbSwipeGesture *>
            (event->gesture(Qt::SwipeGesture))) {
       
            switch (swipe->state()) {
           
            case Qt::GestureStarted: // fall-through
            case Qt::GestureUpdated: // fall-through
            case Qt::GestureCanceled:
                break;
            case Qt::GestureFinished:             
                // Emit a signal to show the swipe movement.           
                break;
            default:
                break;
            }           
        }
       
        // Handle other gesture types that have been grabbed. There may be
        // several, since all gestures that are active at the same moment
        // are sent within the same gesture event.
        if (HbTapGesture *tap = qobject_cast<HbTapGesture *>
            (event->gesture(Qt::TapGesture))) {
            // handle the tap gesture
        }
       
    }   
    \endcode
    </li>
    </ol>
        
    \sa QSwipeGesture
*/

const int KHbDirectionThreshold = 45; // degrees

/*!
    Constructor.
    \param parent Owner for gesture
*/
HbSwipeGesture::HbSwipeGesture(QObject *parent)
    : QSwipeGesture(parent), d_ptr(new HbSwipeGesturePrivate)

{   
    d_ptr->mSceneSwipeAngle = 0;
    d_ptr->mTime.start();
}

/*!
    Constructor required by the shared d-pointer paradigm.
    \param dd Private data
    \param parent Owner for gesture
*/
HbSwipeGesture::HbSwipeGesture(HbSwipeGesturePrivate &dd, QObject *parent)
    : QSwipeGesture(parent), d_ptr(&dd)
{

}

/*!
    Destructor.
*/
HbSwipeGesture::~HbSwipeGesture()
{
    delete d_ptr;
}

/*!
    Returns the horizontal direction of the swipe gesture
    relative to scene coordinates.

    \sa QSwipeGesture::horizontalDirection()
*/
QSwipeGesture::SwipeDirection HbSwipeGesture::sceneHorizontalDirection() const
{
    if ((d_ptr->mSceneSwipeAngle <= 90 - KHbDirectionThreshold && d_ptr->mSceneSwipeAngle >= 0) || d_ptr->mSceneSwipeAngle >= 270 + KHbDirectionThreshold)
        return QSwipeGesture::Right;
    else if (d_ptr->mSceneSwipeAngle >= 90 + KHbDirectionThreshold && d_ptr->mSceneSwipeAngle <= 270 - KHbDirectionThreshold)
        return QSwipeGesture::Left;
    else
        return QSwipeGesture::NoDirection;
}
/*!
    Returns the vertical direction of the swipe gesture
    relative to scene coordinates.
    \sa QSwipeGesture::verticalDirection()
*/
QSwipeGesture::SwipeDirection HbSwipeGesture::sceneVerticalDirection() const
{    
    if (d_ptr->mSceneSwipeAngle < 180 - KHbDirectionThreshold && d_ptr->mSceneSwipeAngle > 0 + KHbDirectionThreshold)
        return QSwipeGesture::Up;
    else if (d_ptr->mSceneSwipeAngle > 180 + KHbDirectionThreshold && d_ptr->mSceneSwipeAngle < 360 - KHbDirectionThreshold)
        return QSwipeGesture::Down;
    else
        return QSwipeGesture::NoDirection;
}

/*!
    Returns the angle for the swipe gesture in degrees,
    taking into account any scene transformations.
    \sa setSceneSwipeAngle(), QSwipeGesture::swipeAngle()
*/
qreal HbSwipeGesture::sceneSwipeAngle() const
{
    return d_ptr->mSceneSwipeAngle;
}

/*!
    Sets the angle for the swipe gesture.
    This function is used by the framework gesture recognition logic,
    and it should not be used by the widget receiving the gesture.
    \sa sceneSwipeAngle(), QSwipeGesture::setSwipeAngle()
*/
void HbSwipeGesture::setSceneSwipeAngle(qreal value)
{
    d_ptr->mSceneSwipeAngle = value;
}
