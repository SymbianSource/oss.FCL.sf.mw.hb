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

   \brief HbTapGesture is an extension to Qt standard QTapGesture.

   HbTapGesture extends QTapGesture with additional information related
   to the tap gesture, but most important use for HbTapGesture is
   in widgets needing both tap and tap-and-hold. HbTapGesture
   provides both -- use of Qt::TapAndHoldGesture
   in conjunction with Qt::TapGesture in the same widget makes it
   difficult to handle state updates and finishes in the widget.
   HbTapGesture::tapStylehint() can be used to query whether
   the tap was a normal tap, or tap-and-hold at the time of Qt::GestureUpdated
   of Qt::GestureFinished. A gesture update will be sent at the time
   when the tap-and-hold timer triggers. No updates are sent
   of the finger movement during the tap.

   \sa QTapGesture, HbTapGesture::TapStyleHint

*/

/*!
    \brief HbTapGesture constructor
    \param parent Parent for the gesture
*/
HbTapGesture::HbTapGesture(QObject *parent)
    : QTapGesture(parent), d_ptr(new HbTapGesturePrivate)
{
    DEBUG() << "Creating" << this;
}

/*!
    \brief HbTapGesture constructor
    \param dd Custom private data
    \param parent Parent for the gesture

*/
HbTapGesture::HbTapGesture( HbTapGesturePrivate &dd, QObject *parent )
    : QTapGesture(parent), d_ptr( &dd )
{
    DEBUG() << "Creating" << this;
}

/*!
    \brief HbTapGesture destructor
*/
HbTapGesture::~HbTapGesture()
{
    DEBUG() << "Deleting" << this;
    delete d_ptr;
}

/*!
    \property startPos
    \brief Stores the starting position of the tap gesture in screen coordinates.
*/
QPointF HbTapGesture::startPos() const
{
    Q_D(const HbTapGesture);
    return d->mStartPos;
}

void HbTapGesture::setStartPos(const QPointF &startPos)
{
    Q_D(HbTapGesture);
    d->mStartPos = startPos;
}

/*!
    \property sceneStartPos
    \brief Stores the starting position of the tap gesture in scene coordinates.
*/
QPointF HbTapGesture::sceneStartPos() const
{
    Q_D(const HbTapGesture);
    return d->mSceneStartPos;
}

void HbTapGesture::setSceneStartPos(const QPointF &startPos)
{
    Q_D(HbTapGesture);
    d->mSceneStartPos = startPos;
}

/*!
    \property scenePosition
    \brief Stores the current position of the tap gesture in scene coordinates.
    \sa QTapGesture::position()
*/
QPointF HbTapGesture::scenePosition() const
{
    Q_D(const HbTapGesture);
    return d->mScenePosition;
}

void HbTapGesture::setScenePosition(const QPointF &startPos)
{
    Q_D(HbTapGesture);
    d->mScenePosition = startPos;
}

/*!
    \property tapStyleHint
    \brief Indicates whether tap is normal tap or long press.

    TapStyleHint is by default Tap and in case of long press, the gesture
    update event is sent and TapStyleHint changed to TapAndHold.
*/
HbTapGesture::TapStyleHint HbTapGesture::tapStyleHint() const
{
    Q_D(const HbTapGesture);
    return d->mTapStyleHint;
}
