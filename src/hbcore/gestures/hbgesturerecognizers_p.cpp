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
#include "hbpangesture.h"
#include "hbtapgesture.h"
#include "hbtapandholdgesture.h"
#include "hbpinchgesture.h"
#include "hbswipegesture.h"

#include <QGesture>

//#define RECOGNIZERS_DEBUG
#ifdef RECOGNIZERS_DEBUG
#define DEBUG qDebug
#else
#define DEBUG if (0) qDebug
#endif

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
QGesture* HbPanGestureRecognizer::create(QObject *)
{
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
    return HbPanGestureLogic::recognize(state->state(), static_cast<HbPanGesture *>(state), watched, event);
}

/*!
    \internal
    \brief
    \return

*/
void HbPanGestureRecognizer::reset(QGesture *state)
{
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
HbTapGestureRecognizer::HbTapGestureRecognizer(int tapRadius)
{
    HbTapGestureLogic::mTapRadius = tapRadius;
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
QGesture* HbTapGestureRecognizer::create(QObject *)
{
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
    return HbTapGestureLogic::recognize( state->state(), static_cast<HbTapGesture *>(state), watched, event );
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
HbTapAndHoldGestureRecognizer::HbTapAndHoldGestureRecognizer(int tapRadius)
    :
    QGestureRecognizer(),
    HbTapAndHoldGestureLogic()
{
    HbTapAndHoldGestureLogic::mTapRadius = tapRadius;
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
QGesture* HbTapAndHoldGestureRecognizer::create(QObject *)
{
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
QGesture* HbPinchGestureRecognizer::create(QObject *)
{
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
QGesture* HbSwipeGestureRecognizer::create(QObject *)
{
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
    return HbSwipeGestureLogic::recognize(state->state(), static_cast<HbSwipeGesture *>(state), watched, event);
}

/*!
    \internal
    \brief
    \return

*/
void HbSwipeGestureRecognizer::reset(QGesture *state)
{
    HbSwipeGestureLogic::resetGesture(static_cast<HbSwipeGesture *>(state));
    QGestureRecognizer::reset(state);
}
