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
// This implementation of Mouse events is most probably removed in later releases.
// It exists purely as an implementation detail.
// This implementation may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
*/

#include "hbgraphicsscenemouseevent.h"

const int HbGraphicsSceneMouseEvent::LongPress = registerEventType();

/*!
    @deprecated,
    @hbcore
	\class HbGraphicsSceneMouseEvent
    \brief HbGraphicsSceneMouseEvent is a class having support for Hb specific events.


	NOTE: TO BE REMOVED / DEPRECATED when the  new Qt Gesture Filter is fully taken into use
    This class supports following extra types of events besides those supported by QGraphicsSceneMouseEvent

    LongPress - This event is normally sent by gesture framework to the widget which will 
                    resceive longpress event. Widget needs to reimplement bool event(QEvent *e) 
                    method.

    Example of how to send HbGraphicsSceneMouseEvent
    \snippet{core/hbgesturefilter.cpp,1}
    
    Example of how to receive HbGraphicsSceneMouseEvent
    \code
	bool ContentWidget::event(QEvent *e)
	{
		if ( e->type() == HbGraphicsSceneMouseEvent::LongPress ) {
			QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(e);
			HbAbstractViewItem* listViewItem = NULL;
			Q_UNUSED(listViewItem);
			onLongPressed(listViewItem, mouseEvent->pos());
			e->accept();
			return true;
		}
		return QGraphicsWidget::event(e);
	}
	\endcode

*/

/*!
    Constructs a HbGraphicsSceneMouseEvent with \a type.
    \a eventType - type of event
*/

HbGraphicsSceneMouseEvent::HbGraphicsSceneMouseEvent(int eventType)
: QGraphicsSceneMouseEvent((QGraphicsSceneMouseEvent::Type)eventType)
{
}
