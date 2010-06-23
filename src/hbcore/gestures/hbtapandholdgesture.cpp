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
#include "hbtapandholdgesture_p.h"
#include "hbtapandholdgesture.h"

#include <QObject>

/*!
    @hbcore
    \class HbTapAndHoldGesture

    \brief The HbTapAndHoldGesture class provides support for receiving
    tap-and-hold gestures.
    
    HbTapAndHoldGesture is an extension to Qt standard QTapAndHoldGesture.
    It is optimized for mobile touch screens, and also supports recognition
    of mouse events for development purposes. It also gives information about
    the tap-and-hold gesture position directly in scene coordinates, removing
    any need for manual conversions from the screen (global) coordinates
    offered by QTapAndHoldGesture.
    
    Use HbTapAndHoldGesture for a custom widget that is only interested in
    the tap-and-hold (long tap) gesture. If you want your custom widget to
    receive both short and long taps, use HbTapGesture instead, since it
    supports both.
    
    \section _usecases_hbtapandholdgesture Using the HbTapAndHoldGesture class
    
    This example shows how to make a custom widget recognize the tap-and-hold
    gesture. The custom widget in the example derives from HbWidget.
    
    <ol>
    <li>
    Register for tap-and-hold gestures by using the base class function
    QGraphicsObject::grabGesture(). QGraphicsObject::grabGesture() can be
    called several times with different arguments, if the widget is
    interested in other gesture types as well.
   
    \code
    // This widget is interested in tap-and-hold and pan gestures.
    grabGesture(Qt::TapAndHoldGesture);
    grabGesture(Qt::PanGesture);
    \endcode
    </li>
    
    <li>
    Reimplement HbWidgetBase::gestureEvent() to handle gestures that are
    meaningful for your custom widget.
   
    \code
    void MyWidget::gestureEvent(QGestureEvent *event)
    {
        if (HbTapAndHoldGesture *tapAndHold =
            qobject_cast<HbTapAndHoldGesture *>
                (event->gesture(Qt::TapAndHoldGesture))) {
       
            switch (tapAndHold->state()) {
           
            case Qt::GestureStarted:
                // Visualize the active state of the widget.
                break;
                
            // No GestureUpdated events are sent for this gesture type,
            // so no handling is needed for those
             
            case Qt::GestureCanceled:
                // Visualize the non-active state of the widget.
                break;
            case Qt::GestureFinished:              
                // Visualize the non-active state of the widget.
                // Emit a long tap signal.              
                break;
            default:
                break;
            }           
        }
       
        // Handle other gesture types that have been grabbed. There may be
        // several, since all gestures that are active at the same moment
        // are sent within the same gesture event.
        if (HbPanGesture *pan = qobject_cast<HbPanGesture *>
            (event->gesture(Qt::PanGesture))) {
            // handle the pan gesture
        }
       
    }   
    \endcode
    </li>
    </ol>
   
    \sa HbTapGesture, QTapAndHoldGesture
*/

/*!
    Constructor.
    \param parent Parent for the gesture
*/
HbTapAndHoldGesture::HbTapAndHoldGesture(QObject* parent)
    :
    QTapAndHoldGesture(parent)
{
    priv = new HbTapAndHoldGesturePrivate(this);
}

/*!
    Constructor required by the shared d-pointer paradigm.
    \param dd Custom private data
    \param parent Parent for the gesture
*/
HbTapAndHoldGesture::HbTapAndHoldGesture(HbTapAndHoldGesturePrivate* dd, QObject* parent)
    :
    QTapAndHoldGesture(parent),
    priv(dd)
{
    priv->q_ptr = this;
}

/*!
    Destructor.
*/
HbTapAndHoldGesture::~HbTapAndHoldGesture()
{
    delete priv; priv = NULL;
}

/*!
    Returns the current position of the gesture in scene coordinates.
    \sa setScenePosition(), QTapAndHoldGesture::position()
*/
QPointF HbTapAndHoldGesture::scenePosition() const
{
    return priv->mScenePos;
}

/*!
    Sets the current position of the gesture in scene coordinates.
    This function is used by the framework gesture recognition logic,
    and it should not be used by the widget receiving the gesture.
    \sa scenePosition(), QTapAndHoldGesture::setPosition()
*/
void HbTapAndHoldGesture::setScenePosition(const QPointF& pos)
{
    priv->mScenePos = pos;
}
