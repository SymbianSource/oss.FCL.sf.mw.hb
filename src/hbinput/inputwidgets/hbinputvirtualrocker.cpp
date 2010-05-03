/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbInput module of the UI Extensions for Mobile.
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
#include <QGraphicsSceneMouseEvent>
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QCoreApplication>
#include <QDir>
#include <hbicon.h>
#include <hbinputsettingproxy.h>
#include <hbwidgetfeedback.h>

#include "hbinputvkbwidget.h"
#include "hbinputvirtualrocker.h"

/// @cond

const qreal HbRockerXThreshold = 5.0;
const qreal HbRockerYThreshold = 50.0;
const qreal HbRockerDimOpacity = 1.0;
const qreal HbRockerNormalOpacity = 1.0;
const qreal HbRockerWidth = 50.0;
const int HbIconWidth = 30;
const int HbPointerWidth = 15;


class HbInputVirtualRockerPrivate
{
public:
    explicit HbInputVirtualRockerPrivate(HbInputVirtualRocker *rocker, HbInputVkbWidget* parent = 0);
    ~HbInputVirtualRockerPrivate();
    int rockerEventRepeats(qreal distance);
    void setCenter();

public:
    HbInputVirtualRocker *q_ptr;
    HbIcon* mIconNormal;
    HbInputVirtualRocker::RockerSelectionMode mShifted;
    QPointF mLastPoint;
    QPointF mCenterPosition;
    QPointF mPointerPosition;
    QPointF mMousePressPoint;
    HbInputVkbWidget* mKeyboard;
    bool mPressed;
};

HbInputVirtualRockerPrivate::HbInputVirtualRockerPrivate(HbInputVirtualRocker *rocker, HbInputVkbWidget* parent)
        : q_ptr(rocker),
          mIconNormal(0),
          mShifted(HbInputVirtualRocker::RockerSelectionModeOff),
          mLastPoint(0.0,0.0),
          mCenterPosition(0.0,0.0),
          mMousePressPoint(0.0,0.0),
          mKeyboard(parent),
          mPressed(false)
{    
    mIconNormal = new HbIcon("qtg_graf_trackpoint_normal" );
    mIconNormal->setSize( QSizeF( HbIconWidth, HbIconWidth ));

    q_ptr->grabGesture(Qt::SwipeGesture);    
    q_ptr->grabGesture(Qt::TapGesture);
    q_ptr->grabGesture(Qt::PanGesture);
}

HbInputVirtualRockerPrivate::~HbInputVirtualRockerPrivate()
{
    delete mIconNormal;
}

int HbInputVirtualRockerPrivate::rockerEventRepeats(qreal distance)
{
    // cursor move multiplier for cursor moving signals, depending on the rocker move speed
    int repeats = 1;
    if (distance > 30){
        repeats = 30;
    } else if (distance > 20) {
        repeats = 10;
    } else if(distance > 10) {
        repeats = 2;
    }
    return repeats;
}

void HbInputVirtualRockerPrivate::setCenter()
{
    if(mCenterPosition.isNull()){
        mCenterPosition.setX(q_ptr->pos().x()+HbRockerWidth/2);
        mCenterPosition.setY(q_ptr->pos().y()+HbRockerWidth/2);
    }
    mPointerPosition.setX((HbRockerWidth - HbPointerWidth)/2);
    mPointerPosition.setY((HbRockerWidth - HbPointerWidth)/2);
}

/// @endcond

/*!
@alpha
@hbinput
\class HbInputVirtualRocker
\brief Virtual rocker widget for generating direction events from virtual keyboards.

Virtual rocker is a virtual version of a mouse pointer decive commonly used in many laptop PC keyboards.
It is typically embedded into virtual keyboards. It generates direction events which can be used for
moving editor cursor, highlighting text, etc.

User first presses virtual rocker widget and then slides finger to desired direction. The widget
will emit direction events based on the direction of finger momvement, relative to the starting point.
When the finger is lifted, rocker widget returns to normal state and stops sending events.

Double clicking will start selection mode. Rocker appearance changes and direction events have shift modifier on
but otherwise it behaves in similar manner as described above.

\sa HbInputVkbWidget
*/
/*!
\enum HbInputVirtualRocker::RockerDirection

This enum defines virtual rocker directions.
*/
/*!
\enum HbInputVirtualRocker::RockerSelectionMode

This enum defines virtual rocker selection modes.
*/

/*!
Constructs the object.
*/
HbInputVirtualRocker::HbInputVirtualRocker(HbInputVkbWidget* parent)
    : HbWidget(parent), d_ptr(new HbInputVirtualRockerPrivate(this, parent))
{
    setOpacity(HbRockerDimOpacity);
}

/*!
Constructs the object.
*/
HbInputVirtualRocker::HbInputVirtualRocker(HbInputVirtualRockerPrivate &dd, QGraphicsWidget* parent)
    : HbWidget(parent), d_ptr(&dd)
{
    setOpacity(HbRockerDimOpacity);
}

/*!
Destroys the widget.
*/
HbInputVirtualRocker::~HbInputVirtualRocker()
{
    delete d_ptr;
}

/*!
    \reimp
    \sa QGraphicsWidget.
*/
void HbInputVirtualRocker::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    Q_D(HbInputVirtualRocker);

    QPointF position = event->pos();
    QPointF delta = position - QPointF(HbRockerWidth/2, HbRockerWidth/2);

    qreal squareDistance = delta.y()*delta.y() + delta.x()*delta.x();
    qreal squareRadius = HbRockerWidth*HbRockerWidth/4;
    if (squareRadius > squareDistance) {
        // the touch point is inside circle which diameter is HbRockerWidth
        d->setCenter();
        d->mLastPoint = position;
        d->mMousePressPoint = position;
        if (d->mKeyboard) {
            d->mKeyboard->setKeyboardDimmed(true);
        }
        HbWidgetFeedback::triggered(this, Hb::InstantPressed);
        setOpacity(HbRockerNormalOpacity);
        d->mPressed = true;
    } else {
        // outside of circle: give event to a push button
        event->ignore();
    }
}

/*!
    \reimp
    \sa QGraphicsWidget.
*/
void HbInputVirtualRocker::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event)
    Q_D(HbInputVirtualRocker);

    if (d->mKeyboard) {
        d->mKeyboard->setKeyboardDimmed(false);
        setOpacity(HbRockerDimOpacity);
    }
    d->mPressed = false;
    update();
    d->setCenter();
    d->mShifted = RockerSelectionModeOff;
    HbWidgetFeedback::triggered(this, Hb::InstantReleased);
}

/*!
    \reimp
    \sa QGraphicsWidget.
*/
void HbInputVirtualRocker::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    Q_D(HbInputVirtualRocker);

    if (event->buttons() != 0) {
        QPointF delta = event->pos() - d->mLastPoint;
        QPointF deltaPressLoc = event->pos() - d->mMousePressPoint;
        int repeats = 1;

        HbWidgetFeedback::triggered(this, Hb::InstantDraggedOver);

        if (delta.x() > HbRockerXThreshold) {
            repeats = d->rockerEventRepeats( delta.x() );
            for (int i = 0; i < repeats; i++) {
                emit rockerDirection(HbRockerDirectionRight, d->mShifted);
            }
            d->mLastPoint = event->pos();
        } else if (delta.x() < -HbRockerXThreshold) {
            repeats = d->rockerEventRepeats( -delta.x() );
            for (int i = 0; i < repeats; i++) {
                emit rockerDirection(HbRockerDirectionLeft, d->mShifted);
            }
            d->mLastPoint = event->pos();
        }

        if (delta.y() > HbRockerYThreshold) {
            repeats = d->rockerEventRepeats(delta.y());
            emit rockerDirection(HbRockerDirectionDown, d->mShifted);
            d->mLastPoint = event->pos();
        } else if (delta.y() < -HbRockerYThreshold) {
            emit rockerDirection(HbRockerDirectionUp, d->mShifted);
            d->mLastPoint = event->pos();
        }

        d->mPointerPosition = HbIconWidth * deltaPressLoc / d->mCenterPosition.x() / 2
            + QPointF((HbRockerWidth-HbPointerWidth)/2, (HbRockerWidth-HbPointerWidth)/2);

        update();

    }
}

/*!
    \reimp
    \sa QGraphicsWidget.
*/
void HbInputVirtualRocker::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event)
    Q_D(HbInputVirtualRocker);

    if (d->mShifted == RockerSelectionModeOff) {
        d->mShifted = RockerSelectionModeOn;
    } else {
        d->mShifted = RockerSelectionModeOff;
    }

    // dim the keypad.
    if (d->mKeyboard) {
        d->mKeyboard->setKeyboardDimmed(true);
    }
    setOpacity(HbRockerNormalOpacity);
}

/*!
    \reimp
    \sa QGraphicsWidget.
*/
void HbInputVirtualRocker::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    Q_D(HbInputVirtualRocker);

    painter->setRenderHint(QPainter::Antialiasing, true);

    if (d->mIconNormal &&
        !d->mIconNormal->isNull()) {
        // We have icon, lets draw it.

        d->mIconNormal->paint(painter, rect(), Qt::IgnoreAspectRatio);
        if (d->mPressed) {
            painter->setBrush(Qt::blue);
            painter->setPen(Qt::blue);
            painter->drawEllipse(static_cast<int>(d->mPointerPosition.x()),
                                 static_cast<int>(d->mPointerPosition.y()),
                                 HbPointerWidth, HbPointerWidth);
        }

    } else {
        // Otherwise just draw a white ellipse as a fallback.
        painter->setBrush(Qt::white);
        painter->setPen(Qt::white);
        painter->drawEllipse(boundingRect());
    }
}

/*!
Returns true if virtual rocker is in selection state, ie. it sends event with shift modifier
on.
*/
HbInputVirtualRocker::RockerSelectionMode HbInputVirtualRocker::selectionMode() const
{
    Q_D(const HbInputVirtualRocker);
    return d->mShifted;
}

void HbInputVirtualRocker::gestureEvent(QGestureEvent *event)
{
    Q_UNUSED (event);
}

// End of file
