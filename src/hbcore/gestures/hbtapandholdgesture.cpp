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
#include "hbtapandholdgesture_p.h"
#include "hbtapandholdgesture.h"

#include <QObject>

/*!
    @hbcore
    \class HbTapAndHoldGesture

    \brief HbTapAndHoldGesture is an extension to Qt standard QTapAndHoldGesture
    \sa QTapAndHoldGesture
*/

/*!
    \brief HbTapAndHoldGesture constructor
    \param parent Parent for the gesture
*/
HbTapAndHoldGesture::HbTapAndHoldGesture(QObject* parent)
    :
    QTapAndHoldGesture(parent)
{
    priv = new HbTapAndHoldGesturePrivate(this);
}

/*!
    \brief HbTapAndHoldGesture constructor
    \param dd Custom private data
    \param parent Parent for the gesture
*/
HbTapAndHoldGesture::HbTapAndHoldGesture(HbTapAndHoldGesturePrivate* dd, QObject* parent)
    :
    QTapAndHoldGesture(parent),
    priv(dd)
{
    priv->q_ptr = this;
}

/*!
    \brief HbTapAndHoldGesture destructor
*/
HbTapAndHoldGesture::~HbTapAndHoldGesture()
{
    delete priv; priv = NULL;
}

/*!
    \property scenePosition

    Current position of the gesture.
    \sa QTapAndHoldGesture::position()
*/
QPointF HbTapAndHoldGesture::scenePosition() const
{
    return priv->mScenePos;
}

void HbTapAndHoldGesture::setScenePosition(const QPointF& pos)
{
    priv->mScenePos = pos;
}
