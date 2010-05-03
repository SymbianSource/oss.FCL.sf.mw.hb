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
#ifndef HBSWIPEGESTURE_H
#define HBSWIPEGESTURE_H

#include <hbglobal.h>
#include <QSwipeGesture>

class HbSwipeGesturePrivate;
class HB_CORE_EXPORT HbSwipeGesture : public QSwipeGesture
{
    Q_OBJECT

    Q_PROPERTY(SwipeDirection sceneHorizontalDirection READ sceneHorizontalDirection STORED false)
    Q_PROPERTY(SwipeDirection sceneVerticalDirection READ sceneVerticalDirection STORED false)
    Q_PROPERTY(qreal sceneSwipeAngle READ sceneSwipeAngle WRITE setSceneSwipeAngle)

    // Deprecated
    Q_PROPERTY(qreal speed READ speed WRITE setSpeed)
    Q_PROPERTY(int touchPointCount READ touchPointCount WRITE setTouchPointCount)

public:
    explicit HbSwipeGesture(QObject *parent = 0);
    virtual ~HbSwipeGesture();

    SwipeDirection sceneHorizontalDirection() const;
    SwipeDirection sceneVerticalDirection() const;

    qreal sceneSwipeAngle() const;
    void setSceneSwipeAngle(qreal value);    

protected:
    HbSwipeGesturePrivate * const d_ptr;
    HbSwipeGesture(HbSwipeGesturePrivate &dd, QObject *parent = 0);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbSwipeGesture)
    Q_DISABLE_COPY(HbSwipeGesture)

    friend class HbSwipeGestureLogic;

public: // Deprecated
    qreal speed() const;
    void setSpeed(qreal speed);
    int touchPointCount() const;
    void setTouchPointCount(int touchPointCount);
};

#endif // HBSWIPEGESTURE_H
