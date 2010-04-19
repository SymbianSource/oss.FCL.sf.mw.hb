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

#ifndef HBSWIPEGESTURERECOGNIZER_H
#define HBSWIPEGESTURERECOGNIZER_H

#include <hbglobal.h>
#include <QGesture>
#include <QGestureRecognizer>
#include <QPoint>
#include <QTime>
#include <QEvent>
#include <QTouchEvent>

class HbSwipeGesture;
/*class QTouchEvent;
class TouchPoint;*/
class QGraphicsSceneMouseEvent;

class HB_CORE_PRIVATE_EXPORT HbSwipeGestureRecognizer : public QGestureRecognizer
{
public:

    HbSwipeGestureRecognizer();
    virtual ~HbSwipeGestureRecognizer();

    virtual QGesture* create(QObject *target);
    virtual QGestureRecognizer::Result recognize(QGesture *state, QObject *watched, QEvent *event);
    virtual void reset(QGesture *state);

private:
    QGestureRecognizer::Result recognizeTouchEvent(HbSwipeGesture *state, QTouchEvent *touchEvent);
    QGestureRecognizer::Result recognizeMouseEvent(HbSwipeGesture *state, QGraphicsSceneMouseEvent *mouseEvent);
    bool touchMoveEvent(QTouchEvent *touchEvent);
    QPointF touchPointsCenterPoint(QTouchEvent *touchEvent);
};

#endif // HBSWIPEGESTURERECOGNIZER_H

