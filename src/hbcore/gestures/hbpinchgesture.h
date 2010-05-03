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
#ifndef HBPINCHGESTURE_H
#define HBPINCHGESTURE_H

#include <hbglobal.h>
#include <QPinchGesture>

class HbPinchGesturePrivate;
class HB_CORE_EXPORT HbPinchGesture : public QPinchGesture
{
    Q_OBJECT

    Q_PROPERTY(qreal sceneTotalRotationAngle READ sceneTotalRotationAngle WRITE setSceneTotalRotationAngle)
    Q_PROPERTY(qreal sceneLastRotationAngle READ sceneLastRotationAngle WRITE setSceneLastRotationAngle)
    Q_PROPERTY(qreal sceneRotationAngle READ sceneRotationAngle WRITE setSceneRotationAngle)
    Q_PROPERTY(QPointF sceneStartCenterPoint READ sceneStartCenterPoint WRITE setSceneStartCenterPoint)
    Q_PROPERTY(QPointF sceneLastCenterPoint READ sceneLastCenterPoint WRITE setSceneLastCenterPoint)
    Q_PROPERTY(QPointF sceneCenterPoint READ sceneCenterPoint WRITE setSceneCenterPoint)

public:
    explicit HbPinchGesture(QObject *parent = 0);
    virtual ~HbPinchGesture();

    qreal sceneTotalRotationAngle() const;
    void setSceneTotalRotationAngle(qreal value);

    qreal sceneLastRotationAngle() const;
    void setSceneLastRotationAngle(qreal value);

    qreal sceneRotationAngle() const;
    void setSceneRotationAngle(qreal value);

    QPointF sceneStartCenterPoint() const;
    void setSceneStartCenterPoint(const QPointF &value);

    QPointF sceneLastCenterPoint() const;
    void setSceneLastCenterPoint(const QPointF &value);

    QPointF sceneCenterPoint() const;
    void setSceneCenterPoint(const QPointF &value);

protected:
    HbPinchGesturePrivate * const d_ptr;
    HbPinchGesture(HbPinchGesturePrivate &dd, QObject *parent = 0);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbPinchGesture)
    Q_DISABLE_COPY(HbPinchGesture)

    friend class HbPinchGestureLogic;
};

#endif // HBPINCHGESTURE_H
