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

#ifndef HBTOOLTIPLABEL_P_P_H
#define HBTOOLTIPLABEL_P_P_H

#include "hbpopup_p.h"
#include <QBasicTimer>
#include <QRectF>
#include <QPointF>
#include <QGraphicsItem>
#include <QFont>

class HbTextItem;
QT_FORWARD_DECLARE_CLASS(QGraphicsSceneMouseEvent)

class HbToolTipLabelPrivate :public HbPopupPrivate
{
    Q_DECLARE_PUBLIC(HbToolTipLabel)

public:
    HbToolTipLabelPrivate();
    void init();
    void checkForToolTip(const QPointF& scenePos, const QPoint& screenPos);
    void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent);
    void mouseReleaseEvent (QGraphicsSceneMouseEvent* mouseEvent);
    void mouseMoveEvent (QGraphicsSceneMouseEvent* mouseEvent);
    void showText(QGraphicsItem *item, Qt::Alignment preferredAlignment);
    bool toolTipBlockItem(QGraphicsItem *item);
    QFont defaultFont();
    void addPopupEffects();
    QBasicTimer tooltipTimer;
    QBasicTimer hideTimer;
    QPointF pressPointScenePos;
    QPoint  pressPointScreenPos;
    QRectF rect;
    HbTextItem *label;
    bool aboutToHide;
    QGraphicsItem *mItem;
    Qt::Alignment mPreferredAlignment;
    qreal mHorizontalMargin;
    qreal mVerticalMargin;
    bool mPolishLayoutRequest;

private:

    class GraphicsItem: public QGraphicsItem
    {
        friend class HbToolTipLabelPrivate;
    };

    static HbToolTipLabelPrivate *d_ptr(HbToolTipLabel *tooltip) {
        Q_ASSERT(tooltip);
        return tooltip->d_func();
    }
};

#endif // HBTOOLTIPLABEL_P_H
