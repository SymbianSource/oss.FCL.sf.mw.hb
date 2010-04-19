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

#ifndef HB_GESTURE_H
#define HB_GESTURE_H

#include <QObject>
#include "hbglobal.h"
#include <QPointF>

class HbGesture;
class HbGesturePrivate;

class HB_CORE_EXPORT HbGesture : public QObject
{
    Q_OBJECT

public:

    /**
    * Direction of the gesture.
    */
    enum Direction
    {
        /* empty gesture */
        none=0,
        /** Left */
        left= 0x0000001,
        /** Right */
        right= 0x0000002,
        /** Up */
        up= 0x0000004,
        /** Down */
        down= 0x0000008,
        /** Pan */
        pan= 0x0000010,
        /** Longpress */
        longpress= 0x0000020
    };

    /*
    * Defines that the default minimum pixel distance should be used for the gesture.
    * The actual amount of pixels might vary based on the screen resolution.
    */
    static const int HbGestureDefaultMinDistance = -1;

public:

    explicit HbGesture( Direction direction,
    int minDistance = HbGestureDefaultMinDistance,
    QObject *parent = 0 );

    virtual ~HbGesture();

    void callback( int speed );
    void panCallback( QPointF delta );
    void longPressCallback( QPointF delta );

    Direction direction() const;

    int minDistance() const;

signals:

    void triggered( int speed );
    void panned( QPointF delta );
    void longPress( QPointF delta );

private: // Data

    HbGesturePrivate * const d;
};

#endif // HB_GESTURE_H
