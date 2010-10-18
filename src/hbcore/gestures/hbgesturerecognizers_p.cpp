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
#include "hbgesturerecognizers_p.h"

#include "hbtapgesture.h"
#include "hbpangesture.h"
#include "hbtapandholdgesture.h"
#include "hbswipegesture.h"
#include "hbpinchgesture.h"

#include "hbnamespace_p.h"

#include <QGesture>
#include <QGraphicsObject>
#include <QDebug>

//#define RECOGNIZERS_DEBUG
#ifdef RECOGNIZERS_DEBUG
#define DEBUG qDebug
#else
#define DEBUG if (0) qDebug
#endif

#define GESTURE_D(Class, ptr) Class##Private * const d = static_cast<Class *>(ptr)->d_func()

////////////////////////////////////////////////////////////////////////////
// Pan gesture
////////////////////////////////////////////////////////////////////////////

/*!
    \internal
    \brief
    \return

*/
HbPanGestureRecognizer::HbPanGestureRecognizer()
{
    DEBUG() << "Creating HbPanGestureRecognizer" << this;
}

/*!
    \internal
    \brief
    \return

*/
HbPanGestureRecognizer::~HbPanGestureRecognizer()
{
    DEBUG() << "Destroying HbPanGestureRecognizer" << this;
}

/*!
    \internal
    \brief
    \return

*/
QGesture* HbPanGestureRecognizer::create(QObject *target)
{
    if (target && target->isWidgetType()) {
        static_cast<QWidget *>(target)->setAttribute(Qt::WA_AcceptTouchEvents);
    }
    if (qobject_cast<QGraphicsObject *>(target)){
        return 0;
    }
    return new HbPanGesture;
}

/*!
    \internal
    \brief
    \return

*/
QGestureRecognizer::Result HbPanGestureRecognizer::recognize(QGesture *state, QObject *watched, QEvent *event)
{
    // This HbPanGestureRecognizer works only as a small stub layer
    // for QT gesture framework's gesture recognizers. The reason for
    // this is to enable testability within logic classes.
    // QGesture contains gesture state, which cannot be modified by
    // anything else, but QGestureManager itself.
    GESTURE_D(HbPanGesture, state);
    Q_ASSERT(d->mTime.isValid());
    return HbPanGestureLogic::recognize(state->state(), static_cast<HbPanGesture *>(state), watched, event, d->mTime.elapsed());
}

/*!
    \internal
    \brief
    \return

*/
void HbPanGestureRecognizer::reset(QGesture *state)
{
    GESTURE_D(HbPanGesture, state);
    d->mTime.restart();
    HbPanGestureLogic::resetGesture(static_cast<HbPanGesture *>(state));
    QGestureRecognizer::reset(state);
}

////////////////////////////////////////////////////////////////////////////
// Tap gesture
////////////////////////////////////////////////////////////////////////////

/*!
    \internal
    \brief
    \return

*/
HbTapGestureRecognizer::HbTapGestureRecognizer()
{    
    DEBUG() << "Creating HbTapGestureRecognizer" << this;
}

/*!
    \internal
    \brief
    \return

*/
HbTapGestureRecognizer::~HbTapGestureRecognizer()
{
    DEBUG() << "Destroying HbTapGestureRecognizer" << this;
}

/*!
    \internal
    \brief
    \return

*/
QGesture* HbTapGestureRecognizer::create(QObject *target)
{    
    if(qobject_cast<QGraphicsObject *>(target))
        return 0;
    return new HbTapGesture;
}

/*!
    \internal
    \brief
    \return

*/
QGestureRecognizer::Result HbTapGestureRecognizer::recognize(QGesture *state, QObject *watched, QEvent *event)
{
    // This HbTapGestureRecognizer works only as a small stub layer
    // for QT gesture framework's gesture recognizers. The reason for
    // this is to enable testability within logic classes.
    // QGesture contains gesture state, which cannot be modified by
    // anything else, but QGestureManager itself.
    QGestureRecognizer::Result result = HbTapGestureLogic::recognize( state->state(),
                                                                      static_cast<HbTapGesture *>(state),
                                                                      watched, event );

    // If a widget sets override property to scene but gets deleted
    // or event is filtered => clear the property here always on
    // gesture transition from active to inactive
    if (result == QGestureRecognizer::CancelGesture ||
        result == QGestureRecognizer::FinishGesture ||
        (state->state() != Qt::NoGesture && result == QGestureRecognizer::MayBeGesture)) {
        QGraphicsView* view = qobject_cast<QGraphicsView*>(watched->parent());
        if (view) {
            QGraphicsScene* scene = view->scene();
            if (scene && scene->property(HbPrivate::OverridingGesture.latin1()).isValid() &&
                scene->property(HbPrivate::OverridingGesture.latin1()).toUInt() == state->gestureType()) {
                scene->setProperty(HbPrivate::OverridingGesture.latin1(), QVariant());
            }
        }
    }

    return result;
}

/*!
    \internal
    \brief
    \return

*/
void HbTapGestureRecognizer::reset(QGesture *state)
{
    HbTapGestureLogic::resetGesture(static_cast<HbTapGesture *>(state));
    QGestureRecognizer::reset(state);
}

////////////////////////////////////////////////////////////////////////////
// Tap & hold gesture
////////////////////////////////////////////////////////////////////////////

/*!
    \internal
    \brief
    \return

*/
HbTapAndHoldGestureRecognizer::HbTapAndHoldGestureRecognizer()
    :
    QGestureRecognizer(),
    HbTapAndHoldGestureLogic()
{    
    DEBUG() << "Creating HbTapAndHoldGestureRecognizer" << this;
}

/*!
    \internal
    \brief
    \return

*/
HbTapAndHoldGestureRecognizer::~HbTapAndHoldGestureRecognizer()
{
    DEBUG() << "Destroying HbTapAndHoldGestureRecognizer" << this;
}

/*!
    \internal
    \brief
    \return

*/
QGesture* HbTapAndHoldGestureRecognizer::create(QObject *target)
{    
    if (qobject_cast<QGraphicsObject *>(target)){
        return 0;
    }
    return new HbTapAndHoldGesture;
}

/*!
    \internal
    \brief
    \return

*/
QGestureRecognizer::Result HbTapAndHoldGestureRecognizer::recognize(QGesture *state, QObject *watched, QEvent *event)
{
    // This HbTapGestureRecognizer works only as a small stub layer
    // for QT gesture framework's gesture recognizers. The reason for
    // this is to enable testability within logic classes.
    // QGesture contains gesture state, which cannot be modified by
    // anything else, but QGestureManager itself.
    return HbTapAndHoldGestureLogic::recognize( state->state(), static_cast<HbTapAndHoldGesture*>(state), watched, event );
}

/*!
    \internal
    \brief
    \return

*/
void HbTapAndHoldGestureRecognizer::reset(QGesture *state)
{
    HbTapAndHoldGestureLogic::resetGesture(static_cast<HbTapAndHoldGesture*>(state));
    QGestureRecognizer::reset(state);
}

////////////////////////////////////////////////////////////////////////////
// Pinch gesture
////////////////////////////////////////////////////////////////////////////

/*!
    \internal
    \brief
    \return

*/
HbPinchGestureRecognizer::HbPinchGestureRecognizer()
{
    DEBUG() << "Creating HbPinchGestureRecognizer" << this;
}

/*!
    \internal
    \brief
    \return

*/
HbPinchGestureRecognizer::~HbPinchGestureRecognizer()
{
    DEBUG() << "Destroying HbPinchGestureRecognizer" << this;
}

/*!
    \internal
    \brief
    \return

*/
QGesture* HbPinchGestureRecognizer::create(QObject *target)
{
    if (target && target->isWidgetType()) {
        static_cast<QWidget *>(target)->setAttribute(Qt::WA_AcceptTouchEvents);
    }
    if (qobject_cast<QGraphicsObject *>(target)){
        return 0;
    }
    return new HbPinchGesture;
}

/*!
    \internal
    \brief
    \return

*/
QGestureRecognizer::Result HbPinchGestureRecognizer::recognize(QGesture *state, QObject *watched, QEvent *event)
{
    // This HbTapGestureRecognizer works only as a small stub layer
    // for QT gesture framework's gesture recognizers. The reason for
    // this is to enable testability within logic classes.
    // QGesture contains gesture state, which cannot be modified by
    // anything else, but QGestureManager itself.
    return HbPinchGestureLogic::recognize(state->state(), static_cast<HbPinchGesture *>(state), watched, event);
}

/*!
    \internal
    \brief
    \return

*/
void HbPinchGestureRecognizer::reset(QGesture *state)
{
    HbPinchGestureLogic::resetGesture(static_cast<HbPinchGesture *>(state));
    QGestureRecognizer::reset(state);
}

////////////////////////////////////////////////////////////////////////////
// Swipe gesture
////////////////////////////////////////////////////////////////////////////

/*!
    \internal
    \brief
    \return

*/
HbSwipeGestureRecognizer::HbSwipeGestureRecognizer()
{
    DEBUG() << "Creating HbSwipeGestureRecognizer" << this;
}

/*!
    \internal
    \brief
    \return

*/
HbSwipeGestureRecognizer::~HbSwipeGestureRecognizer()
{
    DEBUG() << "Destroying HbSwipeGestureRecognizer" << this;
}

/*!
    \internal
    \brief
    \return

*/
QGesture* HbSwipeGestureRecognizer::create(QObject *target)
{
    if(qobject_cast<QGraphicsObject*>(target)) {
        return 0;
    }
    return new HbSwipeGesture;
}

/*!
    \internal
    \brief
    \return

*/
QGestureRecognizer::Result HbSwipeGestureRecognizer::recognize(QGesture *state, QObject *watched, QEvent *event)
{
    // This HbSwipeGestureRecognizer works only as a small stub layer
    // for QT gesture framework's gesture recognizers. The reason for
    // this is to enable testability within logic classes.
    // QGesture contains gesture state, which cannot be modified by
    // anything else, but QGestureManager itself.
    GESTURE_D(HbSwipeGesture, state);
    Q_ASSERT(d->mTime.isValid());
    return HbSwipeGestureLogic::recognize(state->state(), static_cast<HbSwipeGesture *>(state), watched, event, d->mTime.elapsed());
}

/*!
    \internal
    \brief
    \return

*/
void HbSwipeGestureRecognizer::reset(QGesture *state)
{
    GESTURE_D(HbSwipeGesture, state);
    d->mTime.restart();
    HbSwipeGestureLogic::resetGesture(static_cast<HbSwipeGesture *>(state));
    QGestureRecognizer::reset(state);
}
