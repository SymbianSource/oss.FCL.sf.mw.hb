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
#include "hbswipegesture.h"
#include "hbswipegesture_p.h"
#include "hbglobal_p.h"
#include <QPointF>
#include <QVariant>

HbSwipeGesture::HbSwipeGesture(QObject *parent)
    : QSwipeGesture(parent), d_ptr(new HbSwipeGesturePrivate)

{   
    d_ptr->mSceneSwipeAngle = 0;
}

HbSwipeGesture::HbSwipeGesture(HbSwipeGesturePrivate &dd, QObject *parent)
    : QSwipeGesture(parent), d_ptr(&dd)
{

}

HbSwipeGesture::~HbSwipeGesture()
{
    delete d_ptr;
}

QSwipeGesture::SwipeDirection HbSwipeGesture::sceneHorizontalDirection() const
{
    if (d_ptr->mSceneSwipeAngle < 0 || d_ptr->mSceneSwipeAngle == 90 || d_ptr->mSceneSwipeAngle == 270)
        return QSwipeGesture::NoDirection;
    else if (d_ptr->mSceneSwipeAngle < 90 || d_ptr->mSceneSwipeAngle > 270)
        return QSwipeGesture::Right;
    else
        return QSwipeGesture::Left;
}

QSwipeGesture::SwipeDirection HbSwipeGesture::sceneVerticalDirection() const
{    
    if (d_ptr->mSceneSwipeAngle <= 0 || d_ptr->mSceneSwipeAngle == 180)
        return QSwipeGesture::NoDirection;
    else if (d_ptr->mSceneSwipeAngle < 180)
        return QSwipeGesture::Up;
    else
        return QSwipeGesture::Down;
}

qreal HbSwipeGesture::sceneSwipeAngle() const
{
    return d_ptr->mSceneSwipeAngle;
}

void HbSwipeGesture::setSceneSwipeAngle(qreal value)
{
    d_ptr->mSceneSwipeAngle = value;
}


/*!
    \deprecated
    \property speed

    Stores the speed of the swipe gesture in pixels per milliseconds.
*/
qreal HbSwipeGesture::speed() const
{    
    HB_DEPRECATED("HbSwipeGesture::speed is deprecated");
    return 1;
}

void HbSwipeGesture::setSpeed(qreal speed)
{
    Q_UNUSED (speed);
    HB_DEPRECATED("HbSwipeGesture::setSpeed is deprecated");
}

/*!
    \deprecated
    \property touchPointCount

    Stores the number of touchpoints used in the swipe

*/
int HbSwipeGesture::touchPointCount() const
{
    HB_DEPRECATED("HbSwipeGesture::touchPointCount is deprecated");
    return 0;
}

void HbSwipeGesture::setTouchPointCount(int touchPointCount)
{
    HB_DEPRECATED("HbSwipeGesture::setTouchPointCount is deprecated");
    Q_UNUSED(touchPointCount)
}
