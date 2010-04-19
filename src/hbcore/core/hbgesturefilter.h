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

#ifndef HB_GESTURE_FILTER_H
#define HB_GESTURE_FILTER_H

#include <hbglobal.h>
#include <hbwidget.h>

#include <QObject>
#include <QEvent>
#include <QRectF>
#include <qnamespace.h>
#include <QPainter>
#include <QWidget>
#include <QGraphicsItem>

class HbGesture;
class HbGestureFilterPrivate;
class HbGestureSceneFilterPrivate;
class HbLongPressVisualizer;

class HB_CORE_EXPORT HbGestureFilter : public QObject
{
    Q_OBJECT

public:

    explicit HbGestureFilter( Qt::MouseButton button = Qt::LeftButton, QObject *parent = 0 );
    ~HbGestureFilter();
    void addGesture( HbGesture *gesture );
    void removeGesture( HbGesture *gesture );

private:

    bool eventFilter( QObject *obj, QEvent *event );

private: // Data

    HbGestureFilterPrivate * d;
};


class HB_CORE_EXPORT HbGestureSceneFilter : public HbWidget
{
    Q_OBJECT

public:
    explicit HbGestureSceneFilter(Qt::MouseButton button = Qt::LeftButton, QGraphicsItem *parent = 0 );
    ~HbGestureSceneFilter();
    void addGesture( HbGesture *gesture );
    void removeGesture( HbGesture *gesture );
    QRectF boundingRect() const { return QRectF(0,0,0,0); }
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) { }
    bool sceneEventFilter ( QGraphicsItem * watched, QEvent * event );

    void setLongpressAnimation(bool animationEnabled);
    int count();

protected:
    // Intentionally by-passing polish loop.
    void polishEvent() { QGraphicsWidget::polishEvent(); }

private slots:
    void startLongPressCounter();
    void completeLongPress();
    void cancelLongPress();

private: //Functions related to panning and long press
    void startLongPressWatcher();

private:
    HbGestureSceneFilterPrivate * const d;
    bool showLongpressAnimation;
};

#endif // HB_GESTURE_FILTER_H

