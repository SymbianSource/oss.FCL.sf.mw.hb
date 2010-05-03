/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbInput module of the UI Extensions for Mobile.
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
#ifndef HB_INPUT_VIRTUAL_ROCKER_H
#define HB_INPUT_VIRTUAL_ROCKER_H

#include <hbwidget.h>
#include <hbinputdef.h>

class HbInputVkbWidget;
class HbInputVirtualRockerPrivate;

class HB_INPUT_EXPORT HbInputVirtualRocker : public HbWidget
{
    Q_OBJECT

public:
    enum RockerDirection {
        HbRockerDirectionNone = 0,
        HbRockerDirectionLeft,
        HbRockerDirectionRight,
        HbRockerDirectionUp,
        HbRockerDirectionDown
    };

    enum RockerSelectionMode {
        RockerSelectionModeOff = 0,
        RockerSelectionModeOn
    };

public:
    explicit HbInputVirtualRocker(HbInputVkbWidget *parent = 0);
    virtual ~HbInputVirtualRocker();

    RockerSelectionMode selectionMode() const;

    enum { Type = Hb::ItemType_VirtualTrackPoint };
    int type() const { return Type; }

protected: // From QGraphicsItem
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent ( QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    virtual void gestureEvent(QGestureEvent *event);

signals:
    void rockerDirection(int aDirection, HbInputVirtualRocker::RockerSelectionMode aSelectionMode);

protected:
    HbInputVirtualRocker(HbInputVirtualRockerPrivate &dd, QGraphicsWidget* parent = 0);
    HbInputVirtualRockerPrivate* const d_ptr;

private:
    Q_DISABLE_COPY(HbInputVirtualRocker)
    Q_DECLARE_PRIVATE_D(d_ptr, HbInputVirtualRocker)
};

#endif // HB_INPUT_VIRTUAL_ROCKER_H

// End of file
