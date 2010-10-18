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
#ifndef HBPANGESTURE_H
#define HBPANGESTURE_H

#include <hbglobal.h>
#include <QPanGesture>

class HbPanGesturePrivate;
class HB_CORE_EXPORT HbPanGesture : public QPanGesture
{
    Q_OBJECT

    Q_PROPERTY(QPointF startPos READ startPos WRITE setStartPos)
    Q_PROPERTY(QPointF velocity READ velocity WRITE setVelocity)
    Q_PROPERTY(QPointF sceneStartPos READ sceneStartPos WRITE setSceneStartPos)
    Q_PROPERTY(QPointF sceneLastOffset READ sceneLastOffset WRITE setSceneLastOffset)
    Q_PROPERTY(QPointF sceneOffset READ sceneOffset WRITE setSceneOffset)
    Q_PROPERTY(QPointF sceneVelocity READ sceneVelocity STORED false)
    Q_PROPERTY(QPointF sceneAcceleration READ sceneAcceleration STORED false)
    Q_PROPERTY(QPointF sceneDelta READ sceneDelta STORED false)

public:
    explicit HbPanGesture(QObject *parent = 0);
    virtual ~HbPanGesture();

    QPointF startPos() const;
    void setStartPos(const QPointF &startPos);

    QPointF velocity() const;
    void setVelocity(const QPointF &velocity);

    // Scene spesific methods

    QPointF sceneStartPos() const;
    void setSceneStartPos(const QPointF &startPos);

    QPointF sceneLastOffset() const;
    void setSceneLastOffset(const QPointF &value);

    QPointF sceneOffset() const;
    void setSceneOffset(const QPointF &value);

    QPointF sceneVelocity() const;
    QPointF sceneAcceleration() const;
    QPointF sceneDelta() const;

protected:
    HbPanGesturePrivate * const d_ptr;
    HbPanGesture( HbPanGesturePrivate &dd, QObject *parent = 0 );

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbPanGesture)
    Q_DISABLE_COPY(HbPanGesture)
    friend class HbPanGestureLogic;
    friend class HbPanGestureRecognizer;
};

#endif // HBPANGESTURE_H
