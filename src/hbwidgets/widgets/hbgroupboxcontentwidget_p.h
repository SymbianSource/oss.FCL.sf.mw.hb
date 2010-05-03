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

#ifndef HBGROUPBOXCONTENTWIDGET_P_H
#define HBGROUPBOXCONTENTWIDGET_P_H

#include "hbnamespace_p.h"
#include <hbwidget.h>
#include <hbgroupbox.h>
#include "hbgroupbox_p.h"

class HbGestureSceneFilter;
class HbGesture;
class QGestureEvent;

class HB_AUTOTEST_EXPORT HbGroupBoxContentWidget : public HbWidget
{
    Q_OBJECT

public:
    HbGroupBoxContentWidget(QGraphicsItem *parent = 0);
    ~HbGroupBoxContentWidget();

    void createPrimitives();
    void updatePrimitives();

    void createConnection();
    void setType(GroupBoxType type);
    void setContentWidget(HbWidget* widget);

    QGraphicsItem* primitive(HbStyle::Primitive primitive) const;

    enum { Type = HbPrivate::ItemType_GroupBoxContentWidget };
    int type() const { return Type; }

signals:
    void clicked();
    void longPress(QPointF point);

protected:
    void initStyleOption( HbStyleOption *option ) const;
    QVariant itemChange( GraphicsItemChange change, const QVariant &value );
    void mousePressEvent( QGraphicsSceneMouseEvent *event );
    void mouseReleaseEvent( QGraphicsSceneMouseEvent *event );
    void polish( HbStyleParameters& params );
#ifdef HB_GESTURE_FW
    void gestureEvent(QGestureEvent *event);
#endif 

public:
    HbWidget *mContent;
    QGraphicsItem *mBackgroundItem;

    GroupBoxType groupBoxType;
    bool contentPressed;            
    HbGroupBox *groupBox;
};

#endif // HBGROUPBOXCONTENTWIDGET_H
