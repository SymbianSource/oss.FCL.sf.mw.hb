/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbWidgets module of the UI Extensions for Mobile.
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

#ifndef HBPROGRESSSLIDERHANDLE_P_H
#define HBPROGRESSSLIDERHANDLE_P_H

#include "hbnamespace_p.h"
#include <hbwidget.h>
#include <hbprogressslider.h>

class HbStyleOptionProgressSliderHandle;

class HbHandleParent
{
public:
    virtual HbStyle* style() const = 0;
    virtual void emitSliderPressed() = 0;
    virtual void emitSliderMoved(int vlaue) = 0;
    virtual void emitSliderReleased() = 0;
    virtual QRectF boundingRect() const = 0;
    virtual QGraphicsItem* parentGraphicsItem() const = 0;
    virtual HbWidget* parentGraphicsWidget() const = 0;

    virtual ~HbHandleParent(){};
    virtual int progressValue()const = 0;
    virtual int maximum() const = 0;
    virtual int minimum() const = 0;
    virtual bool invertedAppearance() const = 0;
    virtual QString toolTipText() const= 0;
    virtual Qt::Alignment textAlignment() const = 0;
    virtual bool textVisible()const = 0;    
    virtual Qt::Orientation orientation() = 0;
};

class HbProgressSliderHandle : public HbWidget
{
     Q_OBJECT

public:
    enum StateFlag{
        TextVisible   = 0x01,
        MousePressed  = 0x02,
        HandleMoving  = 0x04,
        UserItem      = 0x08,
        MouseMove     = 0x10
    };

    Q_DECLARE_FLAGS(StateFlags, StateFlag)

    explicit HbProgressSliderHandle(HbHandleParent *parent);
    virtual ~HbProgressSliderHandle();

    int pointToValue(QPointF point) const ;
    QPointF valueToHandlePos(int value) const;
    enum { Type = HbPrivate::ItemType_ProgressSliderHandle };
    int type() const { return Type; }
    void setHandleIcon(const HbIcon &icon);
    HbIcon thumbIcon() const;
    void handleTrackRelease(QGraphicsSceneMouseEvent * event);
    void handleTrackPress(QGraphicsSceneMouseEvent * event);
    QVariant processItemChange(const QVariant &value);
    void setHandlePosForValue(int progressValue);
    QPointF normalizedPos(const QPointF&  pos,bool inverted) const;
    bool isHandlePressed() const;
    bool isHandleMoving() const;
    HbProgressSlider::SliderState mSliderHandleState;

    QGraphicsItem* primitive(HbStyle::Primitive primitive) const;

public slots:
    void updatePrimitives();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
    virtual void gestureEvent(QGestureEvent *event);
    void initStyleOption( HbStyleOptionProgressSliderHandle *option ) const;

private:
    StateFlags mFlags;
    HbHandleParent *q;
    QPointF mMousePressPos;
    QPointF mItemPosAtPress;
    QPointF mItemCurPos;
    QSizeF mHandleSize;
    QGraphicsItem *mHandleIconItem; 
    QGraphicsItem *mTouchItem;
    HbIcon mHandleIcon;
    uint mPlayThumb : 1;
    uint mThumbEnabled;
};

#endif  //HBPROGRESSSLIDERHANDLE_P_H

