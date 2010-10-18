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

#ifndef HBSLIDERTRACKITEM_P_H
#define HBSLIDERTRACKITEM_P_H

#include <hbframeitem.h>

class HbSliderTrackItem : public HbFrameItem
{
public:
    explicit HbSliderTrackItem(QGraphicsItem *parent = 0);
    explicit HbSliderTrackItem(HbFrameDrawer *drawer, QGraphicsItem *parent = 0);
    void setMinimum(int min);
    void setMaximum(int max);
    void setValue(int val);
    void setInverted(bool inv);
    void setMaskWidth(qreal width);
    void setOrientation(Qt::Orientation);
    void setSpan( qreal span);
    void paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );
    void setHandleRect(QRectF handleRect);
    int minimum;
    int maximum;
    int value;
    bool inverted;
    qreal maskWidth;
    Qt::Orientation mOrientation;
    qreal trackSpan;
    bool setMask;
    QRectF handleRect;
protected:
    void changeEvent(QEvent *event);
};

#endif // HBSLIDERTRACKITEM_P_H
