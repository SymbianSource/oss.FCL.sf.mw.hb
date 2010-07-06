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
#include "hbinputvirtualrocker.h"

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

/// @cond

const qreal HbRockerXThreshold = 5.0;
const qreal HbRockerYThreshold = 50.0;
const qreal HbRockerDimOpacity = 1.0;
const qreal HbRockerNormalOpacity = 1.0;
const qreal HbRockerWidth = 50.0;
const qreal HbNormalSizeInUnits = 5;
const qreal HbPressedSizeInUnits = 9.5;
const qreal HbActivatedSizeInUnits = 9.5;

class HbInputVirtualRockerPrivate
{
public:
    explicit HbInputVirtualRockerPrivate(HbInputVirtualRocker *rocker);
    ~HbInputVirtualRockerPrivate();
    int rockerEventRepeats(qreal distance);

public:
    HbInputVirtualRocker *q_ptr;
    HbIcon mIconNormal;
    HbIcon mIconPressed;
    HbIcon mIconActivated;
    HbInputVirtualRocker::RockerSelectionMode mShifted;
    QPointF mLastPoint;
    QPointF mMousePressPoint;
    bool mPressed;
};

HbInputVirtualRockerPrivate::HbInputVirtualRockerPrivate(HbInputVirtualRocker *rocker)
    : q_ptr(rocker),
      mShifted(HbInputVirtualRocker::RockerSelectionModeOff),
      mLastPoint(0.0, 0.0),
      mMousePressPoint(0.0, 0.0),
      mPressed(false)
{
    mIconNormal = HbIcon("qtg_graf_trackpoint_normal" );
    mIconPressed = HbIcon("qtg_graf_trackpoint_pressed" );
    mIconActivated = HbIcon("qtg_graf_trackpoint_activated" );

    q_ptr->grabGesture(Qt::SwipeGesture);
    q_ptr->grabGesture(Qt::TapGesture);
    q_ptr->grabGesture(Qt::PanGesture);
}

HbInputVirtualRockerPrivate::~HbInputVirtualRockerPrivate()
{
}

int HbInputVirtualRockerPrivate::rockerEventRepeats(qreal distance)
{
    // cursor move multiplier for cursor moving signals, depending on the rocker move speed
    int repeats = 1;
    if (distance > 30) {
        repeats = 30;
    } else if (distance > 20) {
        repeats = 10;
    } else if (distance > 10) {
        repeats = 2;
    }
    return repeats;
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
\deprecated HbInputVirtualRocker::HbInputVirtualRocker(HbInputVkbWidget*)
    is deprecated.

Constructs the object.
*/
HbInputVirtualRocker::HbInputVirtualRocker(HbInputVkbWidget *parent)
    : HbWidget(parent), d_ptr(new HbInputVirtualRockerPrivate(this))
{
    setOpacity(HbRockerDimOpacity);
}

/*!
\deprecated HbInputVirtualRocker::HbInputVirtualRocker(HbInputVirtualRockerPrivate&, QGraphicsWidget*)
    is deprecated.

Constructs the object.
*/
HbInputVirtualRocker::HbInputVirtualRocker(HbInputVirtualRockerPrivate &dd, QGraphicsWidget *parent)
    : HbWidget(parent), d_ptr(&dd)
{
    setOpacity(HbRockerDimOpacity);
}

/*!
\deprecated HbInputVirtualRocker::~HbInputVirtualRocker()
    is deprecated.

Destroys the widget.
*/
HbInputVirtualRocker::~HbInputVirtualRocker()
{
    delete d_ptr;
}

/*!
\deprecated HbInputVirtualRocker::mousePressEvent(QGraphicsSceneMouseEvent*)
    is deprecated.
*/
void HbInputVirtualRocker::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(HbInputVirtualRocker);

    QPointF position = event->pos();
    QPointF delta = position - QPointF(HbRockerWidth / 2, HbRockerWidth / 2);

    qreal squareDistance = delta.y() * delta.y() + delta.x() * delta.x();
    qreal squareRadius = HbRockerWidth * HbRockerWidth / 4;
    if (squareRadius > squareDistance) {
        // the touch point is inside circle which diameter is HbRockerWidth
        d->mLastPoint = position;
        d->mMousePressPoint = position;
        emit rockerDirection(HbRockerDirectionPress, d->mShifted);
        HbWidgetFeedback::triggered(this, Hb::InstantPressed);
        setOpacity(HbRockerNormalOpacity);
        d->mPressed = true;
    } else {
        // outside of circle: give event to a push button
        event->ignore();
    }
}

/*!
\deprecated HbInputVirtualRocker::mouseReleaseEvent(QGraphicsSceneMouseEvent*)
    is deprecated.
*/
void HbInputVirtualRocker::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    Q_D(HbInputVirtualRocker);

    emit rockerDirection(HbRockerDirectionRelease, d->mShifted);
    setOpacity(HbRockerDimOpacity);
    d->mPressed = false;
    update();
    d->mShifted = RockerSelectionModeOff;
    HbWidgetFeedback::triggered(this, Hb::InstantReleased);
}

/*!
\deprecated HbInputVirtualRocker::mouseMoveEvent(QGraphicsSceneMouseEvent*)
    is deprecated.
*/
void HbInputVirtualRocker::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(HbInputVirtualRocker);

    if (event->buttons() != 0) {
        QPointF delta = event->pos() - d->mLastPoint;
        QPointF deltaPressLoc = event->pos() - d->mMousePressPoint;
        int repeats = 1;

        HbWidgetFeedback::triggered(this, Hb::InstantDraggedOver);

        if (delta.x() > HbRockerXThreshold) {
            repeats = d->rockerEventRepeats(delta.x());
            for (int i = 0; i < repeats; i++) {
                emit rockerDirection(HbRockerDirectionRight, d->mShifted);
            }
            d->mLastPoint = event->pos();
        } else if (delta.x() < -HbRockerXThreshold) {
            repeats = d->rockerEventRepeats(-delta.x());
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
        update();
    }
}

/*!
\deprecated HbInputVirtualRocker::mouseDoubleClickEvent(QGraphicsSceneMouseEvent*)
    is deprecated.
*/
void HbInputVirtualRocker::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    Q_D(HbInputVirtualRocker);

    if (d->mShifted == RockerSelectionModeOff) {
        d->mShifted = RockerSelectionModeOn;
    } else {
        d->mShifted = RockerSelectionModeOff;
    }

    emit rockerDirection(HbRockerDirectionDoubleClick, d->mShifted);
    setOpacity(HbRockerNormalOpacity);
}

/*!
\deprecated HbInputVirtualRocker::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*)
    is deprecated.
*/
void HbInputVirtualRocker::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    Q_D(HbInputVirtualRocker);

    painter->setRenderHint(QPainter::Antialiasing, true);

    qreal unitValue = HbDeviceProfile::profile(mainWindow()).unitValue();
    if (selectionMode() == RockerSelectionModeOn) {
        d->mIconActivated.setSize(QSizeF(HbActivatedSizeInUnits * unitValue, HbActivatedSizeInUnits * unitValue));
        d->mIconActivated.paint(painter, rect());
    } else if (d->mPressed) {
        d->mIconPressed.setSize(QSizeF(HbPressedSizeInUnits * unitValue, HbPressedSizeInUnits * unitValue));
        d->mIconPressed.paint(painter, rect());
    } else {
        d->mIconNormal.setSize(QSizeF(HbNormalSizeInUnits * unitValue, HbNormalSizeInUnits * unitValue));
        d->mIconNormal.paint(painter, rect());
    }
}

/*!
\deprecated HbInputVirtualRocker::selectionMode() const
    is deprecated.
*/
HbInputVirtualRocker::RockerSelectionMode HbInputVirtualRocker::selectionMode() const
{
    Q_D(const HbInputVirtualRocker);
    return d->mShifted;
}

/*!
\deprecated HbInputVirtualRocker::gestureEvent(QGestureEvent*)
    is deprecated.
*/
void HbInputVirtualRocker::gestureEvent(QGestureEvent *event)
{
    Q_UNUSED(event);
}

// End of file
