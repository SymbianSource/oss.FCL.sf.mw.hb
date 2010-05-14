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

    \brief HbPanGesture is an extension to Qt standard QPanGesture
    \sa QPanGesture
*/

/*!
    \brief HbPanGesture constructor
    \param parent Owner for gesture

*/
HbPanGesture::HbPanGesture(QObject *parent) : QPanGesture(parent), d_ptr(new HbPanGesturePrivate)
{
    d_ptr->q_ptr = this;
}

/*!
    \brief HbPanGesture constructor
    \param dd Private data
    \param parent Owner for gesture

*/
HbPanGesture::HbPanGesture( HbPanGesturePrivate &dd, QObject *parent )
    : QPanGesture(parent), d_ptr( &dd )
{
    d_ptr->q_ptr = this;
}

/*!
    \brief HbPanGesture destructor
*/
HbPanGesture::~HbPanGesture()
{
    delete d_ptr;
}

/*!
    \property startPos
    \brief Starting position for gesture in global coordinates.
    \sa HbPanGesture::sceneStartPos
*/
QPointF HbPanGesture::startPos() const
{
    Q_D(const HbPanGesture);
    return d->mStartPos;
}

void HbPanGesture::setStartPos(const QPointF &startPos)
{
    Q_D(HbPanGesture);
    d->mStartPos = startPos;
}

/*!
    \property velocity
    \brief Panning velocity in global coordinates.
    \sa HbPanGesture::sceneVelocity
*/
QPointF HbPanGesture::velocity() const
{
    Q_D(const HbPanGesture);
    return HbVelocityCalculator( d->mAxisX, d->mAxisY ).velocity(QTime::currentTime());
}

void HbPanGesture::setVelocity(const QPointF &)
{
    Q_ASSERT(false);
}

/*!
    \property sceneLastOffset
    \brief The total offset from start position to second last position in scene coordinates.
    \sa QPanGesture::lastOffset()
*/
QPointF HbPanGesture::sceneLastOffset() const
{
    Q_D(const HbPanGesture);
    return d->mSceneLastOffset;
}

void HbPanGesture::setSceneLastOffset(const QPointF &lastOffset)
{
    Q_D(HbPanGesture);
    d->mSceneStartPos = lastOffset;
}

/*!
    \property sceneOffset
    \brief The total offset from start position to current position in scene coordinates.
    \sa QPanGesture::offset()
*/
QPointF HbPanGesture::sceneOffset() const
{
    Q_D(const HbPanGesture);
    return d->mSceneOffset;
}

void HbPanGesture::setSceneOffset(const QPointF &offset)
{
    Q_D(HbPanGesture);
    d->mSceneOffset = offset;
}

/*!
    \property sceneStartPos
    \brief Starting position for gesture in scene coordinates.
    \sa HbPanGesture::startPos()
*/
QPointF HbPanGesture::sceneStartPos() const
{
    Q_D(const HbPanGesture);
    return d->mSceneStartPos;
}

void HbPanGesture::setSceneStartPos(const QPointF &startPos)
{
    Q_D(HbPanGesture);
    d->mSceneStartPos = startPos;
}

/*!
    \property sceneVelocity
    \brief Panning velocity in scene coordinates.
    \sa HbPanGesture::velocity()
*/
QPointF HbPanGesture::sceneVelocity() const
{
    Q_D(const HbPanGesture);
    return HbVelocityCalculator(d->mSceneAxisX, d->mSceneAxisY).velocity( d->mLastTimeStamp );
}

/*!
    \property sceneAcceleration
    \brief Panning acceleration in scene coordinates.
    \sa QPanGesture::acceleration()
*/
QPointF HbPanGesture::sceneAcceleration() const
{
    return QPointF(0,0);
}

/*!
    \property sceneDelta
    \brief Distance between last two points in scene coordinates.
    \sa QPanGesture::delta()
*/
QPointF HbPanGesture::sceneDelta() const
{
    Q_D(const HbPanGesture);
    return d->mSceneOffset - d->mSceneLastOffset;
}

