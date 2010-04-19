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

/*!
//
//  W A R N I N G
//  -------------
//
// This implementation of longpress visualizer is most probably removed in later releases.
// It exists purely as an implementation detail.
// This implementation may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
*/

#include "hblongpressvisualizer_p.h"

#include <QPen>
#include <QPainter>

/* 
	HbLongPressVisualizer
	HbLongPressVisualizer is part of the internal implementation. It is not supposed to be in public API.
	
	
	HbLongPressVisualizer is a visualizer for the long-press gesture.

	HbGestureFilter
*/


/*
	Constructs sample long-press visualization widget with a parent.
*/

HbLongPressVisualizer::HbLongPressVisualizer( QGraphicsItem *parent ) : HbWidget(parent), active(false), spanAngle(0)
{
}

/*
	Reimplemented from QGraphicsItem::paint().
*/
void HbLongPressVisualizer::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    if (active) {
        QPen pen( Qt::lightGray );
        pen.setWidth(5);
        painter->setPen(pen);
        painter->drawArc(rect, 90*16, -spanAngle*16);
    }
}

/*
	Start HbLongPressVisualizer.

	Currently position is hardcoded.
*/
void HbLongPressVisualizer::start(QPointF scenePos)
{
    prepareGeometryChange();
    rect = QRect( 0, 0, 30, 30);

    if (scenePos.y() < 60 ) {
        //Draw the animation below of the touch point
        rect.moveCenter( QPointF(scenePos.x(), scenePos.y()+50));
    }
    else {
        //Draw the animation above of the touch point
        rect.moveCenter( QPointF(scenePos.x(), scenePos.y()-50));
    }

    setFrame(0);
    active = true;
}

/*
	Stop HbLongPressVisualizer.
*/
void HbLongPressVisualizer::stop()
{
    active = false;
    update();
}


/*
	Set frame for HbLongPressVisualizer
*/
void HbLongPressVisualizer::setFrame(int frame)
{
    spanAngle = frame*360/100;
    update();
}

/*
	Reimplemented from QGraphicsItem::boundingRect().

	Currently returns the default bounding rect.

*/
QRectF HbLongPressVisualizer::boundingRect() const 
{
    return rect;
}
