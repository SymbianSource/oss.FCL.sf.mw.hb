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
#include "hbpinchgesture.h"
#include "hbpinchgesture_p.h"

/*!
    @hbcore
    \class HbPinchGesture

    \brief The HbPinchGesture class provides multitouch support.
    
    HbPinchGesture is an extension to Qt standard QPinchGesture. It offers
    convenience functions for handling the pinch gesture properties directly
    in scene coordinates. This removes any need for manual conversions from
    the screen (global) coordinates of the QPinchGesture base class
    properties.
    
    You can use HbPinchGesture, for example, to let the user change an
    image size in a photo application by dragging the two opposing corners
    of the image at the same time. Another example would be rotating a widget
    with two fingers.
    
    \section _usecases_hbpinchgesture Using the HbPinchGesture class
    
    This example shows how to make a custom widget recognize the pinch
    gesture. The custom widget in the example derives from HbWidget.
    
    <ol>
    <li>
    In order to receive touch events, which are required by the pinch gesture,
    at least one widget in the scene needs to accept the QEvent::TouchBegin event.
    To make sure this is done, you can do this in your widget by reimplementing
    QGraphicsWidget::event().
    
    \code
    bool MyWidget::event(QEvent *e)
    {
        if (e->type() == QEvent::TouchBegin) {
            return true;
        }
        return HbWidget::event(e);
    }    
    \endcode 
    </li>
    
    <li>
    Register for pinch gestures by using the base class function
    QGraphicsObject::grabGesture(). QGraphicsObject::grabGesture() can be
    called several times with different arguments, if the widget is
    interested in other gesture types as well.
   
    \code
    // This widget is interested in pinch and tap gestures.
    grabGesture(Qt::PinchGesture);
    grabGesture(Qt::TapGesture);
    \endcode
    </li>
    
    <li>
    Reimplement HbWidgetBase::gestureEvent() to handle gestures that are
    meaningful for your custom widget.
   
    \code
    void MyWidget::gestureEvent(QGestureEvent *event)
    {
        if (HbPinchGesture *pinch = qobject_cast<HbPinchGesture *>
            (event->gesture(Qt::PinchGesture))) {
       
            switch (pinch->state()) {
           
            case Qt::GestureStarted:
                // Visualize the active state of the widget.
                // Emit a signal to move the content.
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
       
        // Handle other gesture types that have been grabbed. There may be several,
        // since all gestures that are active at the same moment are sent within
        // the same gesture event.
        if (HbTapGesture *tap = qobject_cast<HbTapGesture *>
            (event->gesture(Qt::TapGesture))) {
            // handle the tap gesture
        }
       
    }   
    \endcode
    </li>
    </ol>
    
    \sa QPinchGesture
*/

/*!
    Constructor.
    \param parent Owner for gesture
*/
HbPinchGesture::HbPinchGesture(QObject *parent)
    : QPinchGesture(parent), d_ptr(new HbPinchGesturePrivate)

{
    d_ptr->mIsNewSequence = true;
    setTotalScaleFactor(1);
    setLastScaleFactor(1);
    setScaleFactor(1);
}

/*!
    Constructor required by the shared d-pointer paradigm.
    \param dd Private data
    \param parent Owner for gesture
*/
HbPinchGesture::HbPinchGesture(HbPinchGesturePrivate &dd, QObject *parent)
    : QPinchGesture(parent), d_ptr(&dd)
{
    d_ptr->mIsNewSequence = true;
    setTotalScaleFactor(1);
    setLastScaleFactor(1);
    setScaleFactor(1);
}

/*!
    Destructor.
*/
HbPinchGesture::~HbPinchGesture()
{
    delete d_ptr;
}

/*!
    Returns the total angle covered by the gesture in scene coordinates.
    \sa setSceneTotalRotationAngle(), QPinchGesture::totalRotationAngle()
*/
qreal HbPinchGesture::sceneTotalRotationAngle() const
{
    Q_D(const HbPinchGesture);
    return d->mSceneTotalRotationAngle;
}

/*!
    Sets the total angle covered by the gesture in scene coordinates.
    This function is used by the framework gesture recognition logic,
    and it should not be used by the widget receiving the gesture.
    \sa sceneTotalRotationAngle(), QPinchGesture::setTotalRotationAngle()
*/
void HbPinchGesture::setSceneTotalRotationAngle(qreal value)
{
    Q_D(HbPinchGesture);
    d->mSceneTotalRotationAngle = value;
}

/*!
    Returns the last reported angle covered by the gesture in scene
    coordinates.
    \sa setSceneLastRotationAngle(), QPinchGesture::lastRotationAngle()
*/
qreal HbPinchGesture::sceneLastRotationAngle() const
{
    Q_D(const HbPinchGesture);
    return d->mSceneLastRotationAngle;
}

/*!
    Sets the last reported angle covered by the gesture in scene coordinates.
    This function is used by the framework gesture recognition logic,
    and it should not be used by the widget receiving the gesture.
    \sa sceneLastRotationAngle(), QPinchGesture::setLastRotationAngle()
*/
void HbPinchGesture::setSceneLastRotationAngle(qreal value)
{
    Q_D(HbPinchGesture);
    d->mSceneLastRotationAngle = value;
}

/*!
    Returns the angle covered by the gesture since last update
    in scene coordinates.
    \sa setSceneRotationAngle(), QPinchGesture::rotationAngle()
*/
qreal HbPinchGesture::sceneRotationAngle() const
{
    Q_D(const HbPinchGesture);
    return d->mSceneRotationAngle;
}

/*!
    Sets the angle covered by the gesture since last update
    in scene coordinates.
    This function is used by the framework gesture recognition logic,
    and it should not be used by the widget receiving the gesture.
    \sa sceneRotationAngle(), QPinchGesture::setRotationAngle()
*/
void HbPinchGesture::setSceneRotationAngle(qreal value)
{
    Q_D(HbPinchGesture);
    d->mSceneRotationAngle = value;
}

/*!
    Returns the starting position of the center point in scene coordinates.
    \sa setSceneStartCenterPoint(), QPinchGesture::startCenterPoint()
*/
QPointF HbPinchGesture::sceneStartCenterPoint() const
{
    Q_D(const HbPinchGesture);
    return d->mSceneStartCenterPoint;
}

/*!
    Sets the starting position of the center point in scene coordinates.
    This function is used by the framework gesture recognition logic,
    and it should not be used by the widget receiving the gesture.
    \sa sceneStartCenterPoint(), QPinchGesture::setStartCenterPoint()
*/
void HbPinchGesture::setSceneStartCenterPoint(const QPointF &value)
{
    Q_D(HbPinchGesture);
    d->mSceneStartCenterPoint = value;
}

/*!
    Returns the last position of the center point recorded for the gesture
    in scene coordinates.
    \sa setSceneLastCenterPoint(), QPinchGesture::lastCenterPoint()
*/
QPointF HbPinchGesture::sceneLastCenterPoint() const
{
    Q_D(const HbPinchGesture);
    return d->mSceneLastCenterPoint;
}

/*!
    Sets the last position of the center point recorded for the gesture
    in scene coordinates.
    This function is used by the framework gesture recognition logic,
    and it should not be used by the widget receiving the gesture.
    \sa sceneLastCenterPoint(), QPinchGesture::setLastCenterPoint()
*/
void HbPinchGesture::setSceneLastCenterPoint(const QPointF &value)
{
    Q_D(HbPinchGesture);
    d->mSceneLastCenterPoint = value;
}

/*!
    Returns the current center point in scene coordinates.
    \sa setSceneCenterPoint(), QPinchGesture::centerPoint()
*/
QPointF HbPinchGesture::sceneCenterPoint() const
{
    Q_D(const HbPinchGesture);
    return d->mSceneCenterPoint;
}

/*!
    Sets the current center point in scene coordinates.
    This function is used by the framework gesture recognition logic,
    and it should not be used by the widget receiving the gesture.
    \sa sceneCenterPoint(), QPinchGesture::setCenterPoint()
*/
void HbPinchGesture::setSceneCenterPoint(const QPointF &value)
{
    Q_D(HbPinchGesture);
    d->mSceneCenterPoint = value;
}
