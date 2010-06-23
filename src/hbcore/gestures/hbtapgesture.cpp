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
#include "hbtapgesture.h"
#include "hbtapgesture_p.h"

#include <QPointF>
#include <QVariant>
#include <QDebug>

//#define TAPGESTURE_DEBUG
#ifndef TAPGESTURE_DEBUG
# define DEBUG if (0) qDebug
#else
# define DEBUG qDebug
#endif


HbTapGesturePrivate::HbTapGesturePrivate(): mTapStyleHint(HbTapGesture::Tap), mTimerId(0)
{
}

/*!
   @proto
   @hbcore
   \class HbTapGesture

   \brief The HbTapGesture class provides support for widgets needing both
   the tap and the tap-and-hold gestures.
   
   HbTapGesture is an extension to the Qt standard QTapGesture. It is
   optimized for mobile touch screens, and also supports recognition of
   mouse events for development purposes. Moreover, HbTapGesture extends
   QTapGesture by supporting both tap and tap-and-hold gestures. Use of
   Qt::TapAndHoldGesture in conjunction with Qt::TapGesture in the same
   widget would make it difficult to handle state updates and finishes in
   the widget.
   
   Compared to QTapGesture, HbTapGesture also provides additional information
   about the tap gesture position. Moreover, it has convenience functions for
   handling the position information directly in scene coordinates, without
   any need for manual conversions from global to scene coordinates. 
   
   Some of the existing Hb widgets (for example HbPushButton) already
   support tap gestures by emitting a signal such as clicked() or longPress()
   when they are tapped. If, however, you are implementing a custom widget
   whose base class does not emit the needed signals, you need to add
   tap gesture handling by using either QTapGesture or HbTapGesture.
   HbTapGesture is a recommended choice if your widget needs both tap and
   tap-and-hold gestures.
   
   HbTapGesture does not support double tap.
   
   \section _usecases_hbtapgesture Using the HbTapGesture class
      
   This example shows how to make a custom widget recognize the tap and
   tap-and-hold gestures. The custom widget in the example derives from
   HbWidget.

   <ol>
   <li>   
   Register for tap gestures by using the base class function
   QGraphicsObject::grabGesture(). QGraphicsObject::grabGesture() can be
   called several times with different arguments, if the widget is interested
   in other gesture types as well.
   
   \code
   // This widget is interested in tap and pan gestures.
   grabGesture(Qt::TapGesture);
   grabGesture(Qt::PanGesture);
   \endcode
   </li>
   
   <li> 
   Reimplement HbWidgetBase::gestureEvent() to handle gestures that are
   meaningful for your custom widget.
   
   HbTapGesture::tapStyleHint() can be used to query whether the tap was
   a normal tap or tap-and-hold at the time of Qt::GestureUpdated or
   Qt::GestureFinished. A gesture update will be sent at the time when
   the tap-and-hold timer triggers. No updates are sent of the finger
   movement during the tap.
     
   \code
   void MyWidget::gestureEvent(QGestureEvent *event)
   {
       if (HbTapGesture *tap = qobject_cast<HbTapGesture *>
           (event->gesture(Qt::TapGesture))) {
       
           switch (tap->state()) {
           
           case Qt::GestureStarted:
               // Visualize the active state of the widget.
               break;
           case Qt::GestureUpdated:
           
               // Long tap is triggered if the gesture takes
               // more than 0.5 seconds. Handle it here.
               if (tap->tapStyleHint() == HbTapGesture::TapAndHold) {               
                   // Emit a long tap signal.               
               }
               
               break;
           case Qt::GestureCanceled:
               // Visualize the non-active state of the widget.
               break;
           case Qt::GestureFinished:
           
               // Visualize the non-active state of the widget.
               
               // Short tap is handled when the gesture is finished.
               if (tap->tapStyleHint() == HbTapGesture::Tap) {               
                   // Emit a short tap signal.               
               }
               
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

   \sa TapStyleHint, QTapGesture

*/

/*!
    \enum HbTapGesture::TapStyleHint
    Defines the tap style.
*/

/*!
    \var HbTapGesture::TapStyleHint HbTapGesture::Tap
    Normal (short) tap.
*/

/*!
    \var HbTapGesture::TapStyleHint HbTapGesture::TapAndHold
    Long press (tap-and-hold).
*/

/*!
    Constructor.
    
    \param parent Parent for the gesture.
*/
HbTapGesture::HbTapGesture(QObject *parent)
    : QTapGesture(parent), d_ptr(new HbTapGesturePrivate)
{
    DEBUG() << "Creating" << this;
}

/*!
    Constructor required by the shared d-pointer paradigm.
    \param dd Custom private data.
    \param parent Parent for the gesture.

*/
HbTapGesture::HbTapGesture( HbTapGesturePrivate &dd, QObject *parent )
    : QTapGesture(parent), d_ptr( &dd )
{
    DEBUG() << "Creating" << this;
}

/*!
    Destructor.
*/
HbTapGesture::~HbTapGesture()
{
    DEBUG() << "Deleting" << this;
    delete d_ptr;
}

/*!
    Returns the starting position of the tap gesture in screen coordinates.
    
    \sa setStartPos()
*/
QPointF HbTapGesture::startPos() const
{
    Q_D(const HbTapGesture);
    return d->mStartPos;
}

/*!
    Sets the starting position of the tap gesture in screen coordinates.
    This function is used by the framework gesture recognition logic,
    and it should not be used by the widget receiving the gesture.
    
    \sa startPos()
*/
void HbTapGesture::setStartPos(const QPointF &startPos)
{
    Q_D(HbTapGesture);
    d->mStartPos = startPos;
}

/*!
    Returns the starting position of the tap gesture in scene coordinates.
    
    \sa setSceneStartPos()
*/
QPointF HbTapGesture::sceneStartPos() const
{
    Q_D(const HbTapGesture);
    return d->mSceneStartPos;
}

/*!
    Sets the starting position of the tap gesture in scene coordinates.
    This function is used by the framework gesture recognition logic,
    and it should not be used by the widget receiving the gesture.
    
    \sa sceneStartPos()
*/
void HbTapGesture::setSceneStartPos(const QPointF &startPos)
{
    Q_D(HbTapGesture);
    d->mSceneStartPos = startPos;
}

/*!
    Returns the current position of the tap gesture in scene coordinates.
    
    \sa setScenePosition(), QTapGesture::position()
*/
QPointF HbTapGesture::scenePosition() const
{
    Q_D(const HbTapGesture);
    return d->mScenePosition;
}

/*!
    Sets the current position of the tap gesture in scene coordinates.
    This function is used by the framework gesture recognition logic,
    and it should not be used by the widget receiving the gesture.
    
    \sa scenePosition(), QTapGesture::position()
*/
void HbTapGesture::setScenePosition(const QPointF &startPos)
{
    Q_D(HbTapGesture);
    d->mScenePosition = startPos;
}

/*!
    Returns information about whether the tap is a short tap or long press
    (tap-and-hold).

    The tapStyleHint property is by default Tap and in case of long press,
    a gesture update event is sent and tapStyleHint changed to TapAndHold.
*/
HbTapGesture::TapStyleHint HbTapGesture::tapStyleHint() const
{
    Q_D(const HbTapGesture);
    return d->mTapStyleHint;
}
