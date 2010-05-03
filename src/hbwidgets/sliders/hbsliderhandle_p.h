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

#ifndef HBSLIDERHANDLE_P_H
#define HBSLIDERHANDLE_P_H

#include "hbnamespace.h"
#include "hbnamespace_p.h"
#include "hbwidget.h"

class HbSliderControl;
class QGraphicsItem;

QT_BEGIN_NAMESPACE
class QGraphicsWidget;
QT_END_NAMESPACE


class HB_AUTOTEST_EXPORT HbSliderHandle : public HbWidget
{
    Q_OBJECT
  
public:
    explicit HbSliderHandle(HbSliderControl *parent = 0);
    virtual ~HbSliderHandle();
    void forceSetPos(const QPointF &pos);    
    virtual QGraphicsItem * primitive(HbStyle::Primitive primitive) const;
    enum { Type = HbPrivate::ItemType_SliderHandle };
    int type() const { return Type; }

    void setHandleItem(QGraphicsItem *widget);
    QGraphicsItem *handleItem() const;

    void showEvent(QShowEvent *event);
    bool sceneEventFilter(QGraphicsItem *obj,QEvent *event);

    HbSliderControl *sliderControl;
    QGraphicsItem* gItem;
    int handleDragPos;
    int handleInitialPos;
    QGraphicsItem* touchItem;
    QGraphicsItem *mHandleItem;

public slots:
    virtual void updatePrimitives();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void gestureEvent(QGestureEvent *event);
    //void updatePrimitive();
    //virtual void polish( HbStyleParameters& params );

private:
    Q_DISABLE_COPY(HbSliderHandle)
    friend class HbSliderControl;
};
#endif // HBSLIDERHANDLE_P_H
