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

#ifndef HBSCROLLBAR_P_H
#define HBSCROLLBAR_P_H

#include <QBasicTimer>

#include <hbscrollbar.h>
#include <hbwidget_p.h>

class HbStyleFramePrimitiveData;

class HbScrollBarPrivateCore : public QObject
{
    Q_OBJECT

    signals:
        void handlePressed();
        void handleReleased();

    friend class HbScrollBar;
    friend class HbScrollAreaPrivate;    
};

class HbScrollBarPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC(HbScrollBar)
public:
    HbScrollBarPrivate();
    virtual ~HbScrollBarPrivate();

    void init();
    void createPrimitives();
    void updatePosition();
    void sizeHelper();

    void groovePrimitiveData(HbStyleFramePrimitiveData *data);
    void handlePrimitiveData(HbStyleFramePrimitiveData *data);

    void startShowEffect();
    void startHideEffect();
    void loadEffects();

    Qt::Orientation mOrientation;
    qreal mCurrentPosition;
    qreal mPageSize;
    bool mActivated;
    QPointF mPressPosition;
    bool mThumbPressed;
    bool mGroovePressed;
    bool mInteractive;

    QBasicTimer repeatActionTimer;
    qreal mPressedTargetValue;

    QGraphicsObject *grooveItem;
    QGraphicsObject *handleItem;
    QGraphicsObject *mTouchArea;
    qreal mLimitingFactor;
    qreal mTopLeft;
    bool hasEffects;
    QPointF lastEmittedPos;
    bool emittedPos;
    HbScrollBarPrivateCore core;
    static bool effectsLoaded;

    inline bool isPressed() {
        return ( mThumbPressed || mGroovePressed );
    }

private:
    static HbScrollBarPrivate *d_ptr( HbScrollBar *bar ) {
        Q_ASSERT(bar);
        return bar->d_func();
    }
    friend class HbScrollAreaPrivate;
    friend class HbMenuListViewPrivate;
};

#endif // HBSCROLLBAR_P_H
