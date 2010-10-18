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

#ifndef HBTAPANDHOLDGESTURE_P_H
#define HBTAPANDHOLDGESTURE_P_H

#include <hbglobal.h>
#include <QGesture>

class HbTapAndHoldGesturePrivate;

class HB_CORE_EXPORT HbTapAndHoldGesture : public QTapAndHoldGesture
{
    Q_OBJECT

    Q_PROPERTY(QPointF scenePosition READ scenePosition WRITE setScenePosition)

public:
    HbTapAndHoldGesture(QObject* parent = NULL);
    virtual ~HbTapAndHoldGesture();

    QPointF scenePosition() const;
    void setScenePosition(const QPointF &pos);

protected:
    HbTapAndHoldGesture(HbTapAndHoldGesturePrivate* data, QObject* parent = NULL);

private:
    Q_DECLARE_PRIVATE_D(priv, HbTapAndHoldGesture)
    HbTapAndHoldGesturePrivate* priv;
    friend class HbTapAndHoldGestureLogic;
    friend class HbTapAndHoldGestureRecognizer;
};

#endif // HBTAPANDHOLDGESTURE_P_H

