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
#include "hbpinchgesture.h"
#include "hbpinchgesture_p.h"

HbPinchGesture::HbPinchGesture(QObject *parent)
    : QPinchGesture(parent), d_ptr(new HbPinchGesturePrivate)

{
    d_ptr->mIsNewSequence = true;
    setTotalScaleFactor(1);
    setLastScaleFactor(1);
    setScaleFactor(1);
}

HbPinchGesture::HbPinchGesture(HbPinchGesturePrivate &dd, QObject *parent)
    : QPinchGesture(parent), d_ptr(&dd)
{
    d_ptr->mIsNewSequence = true;
    setTotalScaleFactor(1);
    setLastScaleFactor(1);
    setScaleFactor(1);
}

HbPinchGesture::~HbPinchGesture()
{
    delete d_ptr;
}

qreal HbPinchGesture::sceneTotalRotationAngle() const
{
    Q_D(const HbPinchGesture);
    return d->mSceneTotalRotationAngle;
}

void HbPinchGesture::setSceneTotalRotationAngle(qreal value)
{
    Q_D(HbPinchGesture);
    d->mSceneTotalRotationAngle = value;
}

qreal HbPinchGesture::sceneLastRotationAngle() const
{
    Q_D(const HbPinchGesture);
    return d->mSceneLastRotationAngle;
}

void HbPinchGesture::setSceneLastRotationAngle(qreal value)
{
    Q_D(HbPinchGesture);
    d->mSceneLastRotationAngle = value;
}

qreal HbPinchGesture::sceneRotationAngle() const
{
    Q_D(const HbPinchGesture);
    return d->mSceneRotationAngle;
}

void HbPinchGesture::setSceneRotationAngle(qreal value)
{
    Q_D(HbPinchGesture);
    d->mSceneRotationAngle = value;
}

QPointF HbPinchGesture::sceneStartCenterPoint() const
{
    Q_D(const HbPinchGesture);
    return d->mSceneStartCenterPoint;
}

void HbPinchGesture::setSceneStartCenterPoint(const QPointF &value)
{
    Q_D(HbPinchGesture);
    d->mSceneStartCenterPoint = value;
}

QPointF HbPinchGesture::sceneLastCenterPoint() const
{
    Q_D(const HbPinchGesture);
    return d->mSceneLastCenterPoint;
}

void HbPinchGesture::setSceneLastCenterPoint(const QPointF &value)
{
    Q_D(HbPinchGesture);
    d->mSceneLastCenterPoint = value;
}

QPointF HbPinchGesture::sceneCenterPoint() const
{
    Q_D(const HbPinchGesture);
    return d->mSceneCenterPoint;
}

void HbPinchGesture::setSceneCenterPoint(const QPointF &value)
{
    Q_D(HbPinchGesture);
    d->mSceneCenterPoint = value;
}
