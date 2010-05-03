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

#include "hbgesturefilter.h"
#include "hbgesturefilter_p.h"
#include "hbgesture.h"
#include "hbgraphicsscenemouseevent.h"
#include "hbinstance.h"

#include "hbglobal_p.h"

#include <QDebug>
#include <QTimeLine>
#include <QGraphicsSceneMouseEvent>

static const int HB_FLICK_MAX_DURATION = 200; // ms
static const int HB_LONG_PRESS_ANIMATION_DELAY = 200; // ms
static const int HB_LONG_PRESS_TOTAL_TRESHOLD = 300; // ms
static const int HB_LONG_PRESS_PIXEL_TRESHOLD = 20; // pixels

/*!
	@proto
    @hbcore
	\class HbGestureFilter
    \brief HbGestureFilter is a class that is used as event filter.

	Gesture filter for implementing touch gesture actions. This filter can be installed
	as an event filter by calling QObject::installEventFilter for the targeted UI object.
	The actions for the gestures can be hooked by connecting to the \a triggered() or 
	\a panned() signals of the added gestures.

	Example of adding gestures:
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,4}

	\a HbGestureFilter uses Gestures to compare user inputs and when match is detected,
	appropriate callback function is called. Unmatched user input is propagated onwards.

	\sa HbGesture
	\sa HbGestureSceneFilter
	
*/

/*!
	\class HbGestureSceneFilter
	\brief HbGestureSceneFilter is a gesture filter for a graphics scene.

	Gesture filter for implementing touch gesture actions. This filter can be installed
	as an event filter for a scene by calling QGraphicsItem::installSceneEventFilter for
	the targeted UI object. The actions for the gestures can be hooked by connecting to
	the \a triggered() or \a panned() signals of the added gestures.

    Typically user defines the gestures for graphics scene in the \a itemChange() function
	if it is defined. The filter needs to be installed for every GraphicsItem.

	Example of installing the gesture filter to graphics scene:
    \snippet{itemviews/hblistview.cpp,1}

	\a HbGestureSceneFilter uses Gestures to compare user inputs and when match is detected,
	appropriate callback function is called. Unmatched user input is propagated onwards.

    \sa HbGesture
	\sa HbGestureFilter
	
*/


HbGestureFilterPrivate::HbGestureFilterPrivate(Qt::MouseButton button) :
    button(button),
    pressPoint(),
    pressTime(),
    touchDownScenePos(),
    panLastScenePos(),
    gestureTimer(),
    allGestures(HbGesture::none),
    longPressTimer(0),
    longPressDelayTimer(0)
{
}

HbGestureFilterPrivate::~HbGestureFilterPrivate()
{
    qDeleteAll(gestures);
    gestures.clear();
    delete longPressTimer;
    delete longPressDelayTimer;
}


/*!
  \deprecated HbGestureFilter::HbGestureFilter(Qt::MouseButton, QObject*)
     is deprecated. Please use Qt gesture framework instead.

	Constructs new HbGestureFilter with two parameters.
	\a Button specifies, which button is used for the gesture.
	\a Parent is the parent object.
 */
HbGestureFilter::HbGestureFilter( Qt::MouseButton button, QObject *parent ) :
    QObject( parent )
{
    HB_DEPRECATED("HbGestureFilter is deprecated. Use Qt gesture fw instead.");
    d = new HbGestureFilterPrivate(button);
}

/*!
	Destructs the gesture filter.
 */
HbGestureFilter::~HbGestureFilter()
{
    delete d;
}

/*!
	Adds a gesture to this filter.
	The action for the gesture can be triggered by connecting
 	to the gesture's \a triggered() or \a panned() signal.

	\note All the added gestures are deleted when the filter is deleted.
	\note The same object instance may not be added more than once,
	doing so will cause a double deletion crash.
 */
void HbGestureFilter::addGesture(HbGesture *gesture)
{
    d->gestures.append(gesture);
}

void HbGestureFilter::removeGesture(HbGesture *gesture)
{
    foreach ( HbGesture* iterator, d->gestures) {
        if (iterator == gesture){
            qDebug() << "Removing gesture";
            d->gestures.removeAll(iterator);
        }

    }
}

/*!
	The event filter function.
	\a obj Parameter not currently used, \a event parameter is the mouse event.

	\internal
 */
bool HbGestureFilter::eventFilter( QObject *obj, QEvent *event )
{
    Q_UNUSED(obj);
    if ( event->type() == QEvent::MouseButtonPress &&
		static_cast<QMouseEvent *>(event)->button() == d->button ) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        d->pressPoint = mouseEvent->pos();
        d->pressTime = QTime::currentTime();
    }
    else if ( event->type() == QEvent::MouseButtonRelease &&
		static_cast<QMouseEvent *>(event)->button() == d->button ) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        int xDistance = qAbs(d->pressPoint.x() - mouseEvent->x());
        int yDistance = qAbs(d->pressPoint.y() - mouseEvent->y());

        int distance;
        HbGesture::Direction dir;
        
        // If horizontal movement is longer, it is either left or right "flick"
        if ( xDistance >= yDistance ) {
            distance = xDistance;
            dir = d->pressPoint.x() >= mouseEvent->x() ? HbGesture::left : HbGesture::right;
        }
        // If vertical movement is longer, it is either up or down "flick"
        else {
            distance = yDistance;
            dir = d->pressPoint.y() >= mouseEvent->y() ? HbGesture::up : HbGesture::down;
        }

        int time = d->pressTime.elapsed();
        if ( !time ) {
            time = 1; // to avoid division by zero
        }

        int speed = (int)distance * 1000 / time;

        for ( GestureList::iterator i = d->gestures.begin();
              i != d->gestures.end(); ++i ) {
            HbGesture *gesture = *i;

            // If gesture matches, do a callback, which emits the signal.
            if ( gesture->direction() == dir && distance >= gesture->minDistance() ) {
                gesture->callback( speed );
            }

        }
    }
    return false;
}

HbGestureSceneFilterPrivate::HbGestureSceneFilterPrivate(Qt::MouseButton button) :
    p(button),
    widget(0),
    panning(false),
    mouseReleased(true),
    longPressDelayCancelled(false),
    longPressCompleted(false),
    panThreshold(HB_LONG_PRESS_PIXEL_TRESHOLD),
    withinThreshold(true)
{
}

HbGestureSceneFilterPrivate::~HbGestureSceneFilterPrivate()
{
}

// HbGestureScene Filter

/*!
  \deprecated HbGestureSceneFilter::HbGestureSceneFilter(Qt::MouseButton, QGraphicsItem*)
    is deprecated. Please use Qt gesture framework instead.

	Constructor for	HbGestureSceneFilter with 
	\a Button Specifies, which button is used for the gesture.
	\a Parent is the parent object.
 */
HbGestureSceneFilter::HbGestureSceneFilter(Qt::MouseButton button, QGraphicsItem *parent) :
    HbWidget(parent),
    d(new HbGestureSceneFilterPrivate(button))
{
    HB_DEPRECATED("HbGestureSceneFilter is deprecated. Use Qt gesture fw instead.");
    showLongpressAnimation = false;
}

/*!
	Destructs the gesture scene filter.
 */
HbGestureSceneFilter::~HbGestureSceneFilter()
{
    delete d;
}

/*!
	Adds new HbGesture to gesture filter.
 */
void HbGestureSceneFilter::addGesture( HbGesture *gesture )
{
    d->p.gestures.append( gesture );
    d->p.allGestures = (HbGesture::Direction)(gesture->direction() | d->p.allGestures);
    if ( gesture->direction() == HbGesture::pan )
        d->panThreshold = gesture->minDistance();

}

void HbGestureSceneFilter::removeGesture(HbGesture *gesture)
{

    foreach ( HbGesture* iterator, d->p.gestures) {
        if (iterator == gesture){
            d->p.gestures.removeAll(iterator);
            d->p.allGestures = (HbGesture::Direction)(gesture->direction() ^ d->p.allGestures);
            if ( gesture->direction() == HbGesture::pan )
                d->panThreshold = HB_LONG_PRESS_PIXEL_TRESHOLD;
        }

    }
}

int HbGestureSceneFilter::count()
{
    return d->p.gestures.count();
}


/*!
        Starts the longpress watcher.
 */
void HbGestureSceneFilter::startLongPressWatcher()
{
    d->longPressDelayCancelled = false;
	//Create delay timer before starting longpress 250ms
    if ( !d->p.longPressDelayTimer )
    {
        d->p.longPressDelayTimer = new QTimeLine(HB_LONG_PRESS_ANIMATION_DELAY, 0);
        //Once the timer finishes, start the actual longpress
        connect( d->p.longPressDelayTimer, SIGNAL(finished()), this, SLOT(startLongPressCounter()) );
    }
    //Start the delay timer
	d->p.longPressDelayTimer->start();
}

/*!
        Starts the longpress counter.

        \internal
 */
void HbGestureSceneFilter::startLongPressCounter()
{
    if (!d->longPressDelayCancelled) {

        //Delay timer has exceeded, start the long press.
        if ( !d->p.longPressTimer )
        {
            d->p.longPressTimer = new QTimeLine(HB_LONG_PRESS_TOTAL_TRESHOLD, 0);
            connect( d->p.longPressTimer, SIGNAL(finished()), this, SLOT(completeLongPress()) );
        }

        d->p.longPressTimer->setCurrentTime(0);
        d->p.longPressTimer->start();

    }

    else {

    }
}

/*!
	Called when longpress is completed, it removes the animation.

	\internal
 */
void HbGestureSceneFilter::completeLongPress()
{
	//If the timer reached end
	if (d->p.longPressTimer->currentFrame() == d->p.longPressTimer->endFrame() && !d->longPressDelayCancelled)	{
		
            for ( GestureList::iterator i = d->p.gestures.begin(); i != d->p.gestures.end(); ++i ) {
                HbGesture *gesture = *i;

                // If gesture matches, do a callback, which emits the signal.
                if ( gesture->direction() == HbGesture::longpress ) {
                    //Delete the long press animation before the callback.
                    d->longPressCompleted = true;

                    // Create custom Hb Event and add position data
                    HbGraphicsSceneMouseEvent longPressEvent(HbGraphicsSceneMouseEvent::LongPress);
                    longPressEvent.setAccepted(false);
                    longPressEvent.setPos(d->p.touchDownScenePos);

                    //Get all items under cursor
                    QList<QGraphicsItem *> items;
                    //Needed to prevent testing fault
                    if(scene()) {
                        items = scene()->items(d->p.touchDownScenePos);
                        // Go through all items and send event to widget
                        foreach(QGraphicsItem *item, items){
                            if(item->isWidget()) {
                                HbWidget *obj = static_cast<HbWidget*>(item);
                                QCoreApplication::sendEvent(obj, &longPressEvent);
                                if (longPressEvent.isAccepted())
                                    break;
                            }
                        }
                    }

                    gesture->longPressCallback( d->p.touchDownScenePos );
                }
            }

	}

	else {
            d->longPressCompleted = false;
	}
}

/*!
	Called when longpress is cancelled, it removes the animation.

	\internal
 */
void HbGestureSceneFilter::cancelLongPress()
{
    d->longPressCompleted = false;
    d->longPressDelayCancelled = true;

    if (d->p.longPressDelayTimer)
        d->p.longPressDelayTimer->stop();
    if (d->p.longPressTimer)
        d->p.longPressTimer->stop();
}

/*!
	Event filter function for the scene.
	Parameter \a watched is the GraphicsItem that is related to the \a event.

	\internal
 */
bool HbGestureSceneFilter::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    if (!event) return false;

    //Set panning directions
    Qt::Orientations panDirs = (Qt::Horizontal|Qt::Vertical);
    int timestamp;
    
    //Mouse press event.
    //Handle double click events like normal events
    if ( ( event->type() == QEvent::GraphicsSceneMousePress ) 
        || ( event->type() == QEvent::GraphicsSceneMouseDoubleClick ) ) {

        d->longPressCompleted = false;
        d->mouseReleased = false;

        if (watched->isWidget())
            d->widget = static_cast<QGraphicsWidget*>(watched);
        else {
            d->widget = 0;
            return false;
        }

        QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
        d->p.pressPoint = mouseEvent->pos().toPoint();

        if (d->widget && !d->widget->isUnderMouse()) {
            d->widget = 0;
            return false;
        }


        d->p.gestureTimer.restart();
        timestamp = 0;
        
        //Mark the position
        d->p.touchDownScenePos = mouseEvent->scenePos();
        d->p.panLastScenePos = d->p.touchDownScenePos;

        if (d->p.allGestures & HbGesture::longpress) {
            startLongPressWatcher();
        } else {
            // make sure long press delay cancelled is always cleared
            d->longPressDelayCancelled = false;
        }
        
        d->withinThreshold = true;
        return false;


    }
    //Mouse move event
    else if ( event->type() == QEvent::GraphicsSceneMouseMove) {

        QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);

        //WHere the "mouse" was moved to
        QPointF scenePos = mouseEvent->scenePos();

        //Check threshold for mouse move during longpress.
        //Uses hbgesture::pan minDistance as the threshold.
        int xDistance = qAbs(d->p.touchDownScenePos.toPoint().x() - mouseEvent->scenePos().toPoint().x());
        int yDistance = qAbs(d->p.touchDownScenePos.toPoint().y() - mouseEvent->scenePos().toPoint().y());

        //See if the distance exceeds the threshold and cancel the longpress if so.
        if ( xDistance > d->panThreshold || yDistance > d->panThreshold) {
            d->withinThreshold = false;
            cancelLongPress();
        }

    	//See if the panning is registered as gesture
        if (!d->mouseReleased && (d->p.allGestures & HbGesture::pan)) d->panning = true;
        else d->panning = false;



        //Based on timestamp we decide if we are panning or flicking
        timestamp = d->p.gestureTimer.elapsed();

        //Mouse panning
        if (d->panning && !d->mouseReleased && timestamp > HB_FLICK_MAX_DURATION) {
            QPointF lastPanPos(scenePos);

            // Exit if panning outside widget
            if (watched->isWidget()) {
                QGraphicsWidget *w = static_cast<QGraphicsWidget*>(watched);
                if (w && !w->isUnderMouse()) {
                    return false;
                }
            }

            // Handle unidirectional panning
            if (panDirs==Qt::Vertical) {
                lastPanPos.setX(d->p.touchDownScenePos.x());
            }
            if (panDirs==Qt::Horizontal) {
                lastPanPos.setY(d->p.touchDownScenePos.y());
            }
    
            QLineF delta( d->p.touchDownScenePos, lastPanPos );
    
            //This does not make much sense
            const QPointF &newScenePos = lastPanPos;
            QPointF deltaPoint = newScenePos - d->p.panLastScenePos;

            // Until the threshold has been passed, keep the panLastScenePos
            // at the touch down position.  This makes the point under
            // your finger the same as where you first touched, which
            // is what the user expects.
            if (!d->withinThreshold) {
                d->p.panLastScenePos = newScenePos;
            }
             
            bool gestureFound = false;
            for ( GestureList::iterator i = d->p.gestures.begin(); i != d->p.gestures.end(); ++i ) {
                HbGesture *gesture = *i;
    
                // If gesture matches, do a callback, which emits the signal.
                if ( gesture->direction() == HbGesture::pan ) {
                    //Take mouse move event location
                    int moveX = mouseEvent->pos().toPoint().x();
                    int moveY = mouseEvent->pos().toPoint().y();

                    //Calculate the distance between mouse press and release
                    int xDistance = qAbs( d->p.pressPoint.x() - moveX );
                    int yDistance = qAbs( d->p.pressPoint.y() - moveY );

                    //If longpress already cancelled, so panning is the only option
                    if ( d->longPressDelayCancelled ) {
                        gestureFound = true;
                        gesture->panCallback( deltaPoint );
                        return true;
                    }
                    //Because we still might be having longpress,
                    //check that panning gesture minimum distance is exceeded
                    else if ( xDistance > d->panThreshold || yDistance > d->panThreshold ) {
                        gestureFound = true;
                        gesture->panCallback( deltaPoint );
                        return true;
                    }
                }
            }
            return gestureFound;
        } //panning
        else d->panning = false;
    }//mouse move
        
    //Mouse release event
    else if ( event->type() == QEvent::GraphicsSceneMouseRelease ) {

        if (d->longPressCompleted == false) {
            d->mouseReleased = true;
            cancelLongPress();

            if (d->panning) {
                d->panning = false;
                return false;
            }

            QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);

            int ex = mouseEvent->pos().toPoint().x();
            int ey = mouseEvent->pos().toPoint().y();

            //Calculate the distance between mouse press and release
            int xDistance = qAbs(d->p.pressPoint.x() - ex);
            int yDistance = qAbs(d->p.pressPoint.y() - ey);

            int distance;
            HbGesture::Direction dir;

            // If horizontal movement is longer, it is either lefmetaObjectt or right "flick"
            if ( xDistance >= yDistance ) {
                distance = xDistance;
                dir = d->p.pressPoint.x() >= ex ? HbGesture::left : HbGesture::right;
            }
            // If vertical movement is longer, it is either up or down "flick"
            else {
                distance = yDistance;
                dir = d->p.pressPoint.y() >= ey ? HbGesture::up : HbGesture::down;
            }

            int time = d->p.gestureTimer.elapsed();
            if ( !time ) {
                time = 1; // to avoid division by zero
            }

            int speed = (int)distance * 1000 / time;
            bool flag = false;

            for ( GestureList::iterator i = d->p.gestures.begin();
            i != d->p.gestures.end(); ++i ) {
                HbGesture *gesture = *i;

                // If gesture matches, do a callback, which emits the signal.
                if ( gesture->direction() == dir && distance >= gesture->minDistance() ) {
                    gesture->callback( speed );
                    flag = true;
                }
            }
            return flag;
        } // !longPressCompleted

        //If this is the release event from a succesfull long press.
        else if (d->longPressCompleted) {
            d->longPressCompleted = false;
            // return false so that the event can be handled in menu
            // -> case when releasing outside context menu -> closes the menu
            return false;
        }

    } //Mouse release event

    return false;
} //HbGestureSceneFilter

void HbGestureSceneFilter::setLongpressAnimation(bool animationEnabled)
{
    showLongpressAnimation = animationEnabled;
}

// End of File
