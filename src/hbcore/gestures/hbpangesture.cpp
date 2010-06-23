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

#include "hbgestures_p.h"
#include "hbpangesture.h"
#include "hbpangesture_p.h"
#include "hbvelocitycalculator_p.h"

#include <QPointF>
#include <QVariant>
#include <QDebug>

/*!
    @hbcore
    \class HbPanGesture

    \brief The HbPanGesture class provides support for receiving a dragging
    (pan) gesture.
    
    HbPanGesture is an extension to the Qt standard QPanGesture. It is
    optimized for mobile touch screens, and also supports recognition of
    mouse events for development purposes. Moreover, HbPanGesture
    adds some new properties (startPos and velocity) to the existing
    QPanGesture properties, and also provides variants of all these expressed
    in scene coordinates. This removes any need for manual conversions from
    the screen (global) coordinates of the QPanGesture base class properties.
    
    A pan gesture is used in situations where the user drags content to a new
    position. Make sure your application gives instant feedback to the user
    with each gesture update by moving the content, not only when the gesture
    is finished.
    
    \section _usecases_hbpangesture Using the HbPanGesture class
    
    This example shows how to make a custom widget recognize the pan
    gesture. The custom widget in the example derives from HbWidget.
    
    <ol>
    <li>
    Register for pan gestures by using the base class function
    QGraphicsObject::grabGesture(). QGraphicsObject::grabGesture() can be
    called several times with different arguments, if the widget is
    interested in other gesture types as well.
   
    \code
    // This widget is interested in pan and tap gestures.
    grabGesture(Qt::PanGesture);
    grabGesture(Qt::TapGesture);
    \endcode
    </li>
    
    <li>
    Reimplement HbWidgetBase::gestureEvent() to handle gestures that are
    meaningful for your custom widget.
   
    \code
    void MyWidget::gestureEvent(QGestureEvent *event)
    {
        if (HbPanGesture *pan = qobject_cast<HbPanGesture *>
            (event->gesture(Qt::PanGesture))) {
       
            switch (pan->state()) {
           
            case Qt::GestureStarted:
                // Visualize the active state of the widget.
                // Emit a signal to move the content
                break;
            case Qt::GestureUpdated:
                // Emit a signal to move the content.
                break;
            case Qt::GestureCanceled:
                // Visualize the non-active state of the widget.
                // Emit a signal to return the content to the starting
                // position.
                break;
            case Qt::GestureFinished:
                // Visualize the non-active state of the widget.             
                // Emit a signal to move the content.   
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
    
    \sa QPanGesture
*/

/*!
    Constructor.
    \param parent Owner for gesture

*/
HbPanGesture::HbPanGesture(QObject *parent) : QPanGesture(parent), d_ptr(new HbPanGesturePrivate)
{
    d_ptr->q_ptr = this;
}

/*!
    Constructor required by the shared d-pointer paradigm.
    \param dd Private data
    \param parent Owner for gesture

*/
HbPanGesture::HbPanGesture( HbPanGesturePrivate &dd, QObject *parent )
    : QPanGesture(parent), d_ptr( &dd )
{
    d_ptr->q_ptr = this;
}

/*!
    Destructor.
*/
HbPanGesture::~HbPanGesture()
{
    delete d_ptr;
}

/*!
    Returns the starting position for the gesture in global coordinates.
    \sa setStartPos(), sceneStartPos()
*/
QPointF HbPanGesture::startPos() const
{
    Q_D(const HbPanGesture);
    return d->mStartPos;
}

/*!
    Sets the starting position for the gesture in global coordinates.
    This function is used by the framework gesture recognition logic,
    and it should not be used by the widget receiving the gesture.
    \sa startPos(), setSceneStartPos()
*/
void HbPanGesture::setStartPos(const QPointF &startPos)
{
    Q_D(HbPanGesture);
    d->mStartPos = startPos;
}

/*!
    Returns the panning velocity in global coordinates.
    The unit is pixels per millisecond.
    \sa setVelocity(), sceneVelocity()
*/
QPointF HbPanGesture::velocity() const
{
    Q_D(const HbPanGesture);
    return HbVelocityCalculator( d->mAxisX, d->mAxisY ).velocity(QTime::currentTime());
}

/*!
    Sets the panning velocity in global coordinates.
    This function is used by the framework gesture recognition logic,
    and it should not be used by the widget receiving the gesture.
    \sa velocity()
*/
void HbPanGesture::setVelocity(const QPointF &)
{
    Q_ASSERT(false);
}

/*!
    Returns the total offset from start position to second last position
    in scene coordinates.
    \sa setSceneLastOffset(), QPanGesture::lastOffset()
*/
QPointF HbPanGesture::sceneLastOffset() const
{
    Q_D(const HbPanGesture);
    return d->mSceneLastOffset;
}

/*!
    Sets the total offset from start position to second last position
    in scene coordinates.
    This function is used by the framework gesture recognition logic,
    and it should not be used by the widget receiving the gesture.
    \sa sceneLastOffset(), QPanGesture::setLastOffset()
*/
void HbPanGesture::setSceneLastOffset(const QPointF &lastOffset)
{
    Q_D(HbPanGesture);
    d->mSceneStartPos = lastOffset;
}

/*!
    Returns the total offset from start position to current position
    in scene coordinates.
    \sa setSceneOffset(), QPanGesture::offset()
*/
QPointF HbPanGesture::sceneOffset() const
{
    Q_D(const HbPanGesture);
    return d->mSceneOffset;
}

/*!
    Sets the total offset from start position to current position
    in scene coordinates.
    This function is used by the framework gesture recognition logic,
    and it should not be used by the widget receiving the gesture.
    \sa sceneOffset(), QPanGesture::setOffset()
*/
void HbPanGesture::setSceneOffset(const QPointF &offset)
{
    Q_D(HbPanGesture);
    d->mSceneOffset = offset;
}

/*!
    Returns the starting position for the gesture in scene coordinates.
    \sa setSceneStartPos(), startPos()
*/
QPointF HbPanGesture::sceneStartPos() const
{
    Q_D(const HbPanGesture);
    return d->mSceneStartPos;
}

/*!
    Sets the starting position for the gesture in scene coordinates.
    This function is used by the framework gesture recognition logic,
    and it should not be used by the widget receiving the gesture.
    \sa sceneStartPos(), setStartPos()
*/
void HbPanGesture::setSceneStartPos(const QPointF &startPos)
{
    Q_D(HbPanGesture);
    d->mSceneStartPos = startPos;
}

/*!
    Returns the panning velocity in scene coordinates.
    The unit is pixels per millisecond.
    \sa velocity()
*/
QPointF HbPanGesture::sceneVelocity() const
{
    Q_D(const HbPanGesture);
    return HbVelocityCalculator(d->mSceneAxisX, d->mSceneAxisY).velocity( d->mLastTimeStamp );
}

/*!
    Returns the panning acceleration in scene coordinates.
    \sa QPanGesture::acceleration()
*/
QPointF HbPanGesture::sceneAcceleration() const
{
    return QPointF(0,0);
}

/*!
    Returns the distance between the last two points in scene coordinates.
    \sa QPanGesture::delta()
*/
QPointF HbPanGesture::sceneDelta() const
{
    Q_D(const HbPanGesture);
    return d->mSceneOffset - d->mSceneLastOffset;
}

