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
    \class HbPanGesture

    \brief HbPanGesture contains data and functionality for pan gesture.
*/

/*!
    \brief
    \return

*/
HbPanGesture::HbPanGesture(QObject *parent) : QPanGesture(parent), d_ptr(new HbPanGesturePrivate)
{
    d_ptr->q_ptr = this;
}

/*!
    \brief
    \return

*/
HbPanGesture::HbPanGesture( HbPanGesturePrivate &dd, QObject *parent )
    : QPanGesture(parent), d_ptr( &dd )
{
    d_ptr->q_ptr = this;
}

/*!
    \brief
    \return

*/
HbPanGesture::~HbPanGesture()
{
    delete d_ptr;
}

/*!
    \brief
    \return

*/
QPointF HbPanGesture::startPos() const
{
    Q_D(const HbPanGesture);
    return d->mStartPos;
}

/*!
    \brief
    \return

*/
void HbPanGesture::setStartPos(const QPointF &startPos)
{
    Q_D(HbPanGesture);
    d->mStartPos = startPos;
}

/*!
    \brief
    \return

*/
QPointF HbPanGesture::velocity() const
{
    Q_D(const HbPanGesture);
    return HbVelocityCalculator( d->mAxisX, d->mAxisY ).velocity(QTime::currentTime());
}

/*!
    \brief
    \return

*/
void HbPanGesture::setVelocity(const QPointF &)
{
    // Q_D(HbPanGesture);
    // d->mVelocity = velocity;
}

/*!
    \brief
    \return

*/
QPointF HbPanGesture::sceneLastOffset() const
{
    Q_D(const HbPanGesture);
    return d->mSceneLastOffset;
}

/*!
    \brief
    \return

*/
void HbPanGesture::setSceneLastOffset(const QPointF &lastOffset)
{
    Q_D(HbPanGesture);
    d->mSceneStartPos = lastOffset;
}

/*!
    \brief
    \return

*/
QPointF HbPanGesture::sceneOffset() const
{
    Q_D(const HbPanGesture);
    return d->mSceneOffset;
}

/*!
    \brief
    \return

*/
void HbPanGesture::setSceneOffset(const QPointF &offset)
{
    Q_D(HbPanGesture);
    d->mSceneOffset = offset;
}

/*!
    \brief
    \return

*/
QPointF HbPanGesture::sceneStartPos() const
{
    Q_D(const HbPanGesture);
    return d->mSceneStartPos;
}

/*!
    \brief
    \return

*/
void HbPanGesture::setSceneStartPos(const QPointF &startPos)
{
    Q_D(HbPanGesture);
    d->mSceneStartPos = startPos;
}

/*!
    \brief
    \return

*/
QPointF HbPanGesture::sceneVelocity() const
{
    Q_D(const HbPanGesture);
    return HbVelocityCalculator(d->mSceneAxisX, d->mSceneAxisY).velocity( d->mLastTimeStamp );
}

/*!
    \brief
    \return

*/
QPointF HbPanGesture::sceneAcceleration() const
{
    //Q_D(const HbPanGesture);
    //return d->mSceneAcceleration;
    return QPointF(0,0);
}

/*!
    \brief
    \return

*/
QPointF HbPanGesture::sceneDelta() const
{
    Q_D(const HbPanGesture);
    return d->mSceneOffset - d->mSceneLastOffset;
}

