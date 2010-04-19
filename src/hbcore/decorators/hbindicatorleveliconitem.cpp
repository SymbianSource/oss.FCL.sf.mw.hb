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

#include "hbindicatorleveliconitem_p.h"
#include <QPainter>

HbIndicatorLevelIconItem::HbIndicatorLevelIconItem(QGraphicsItem *parent) :
    HbIconItem(parent),
    mValue(0),
    mIndicatorDirection(LeftToRight)
{
}
void HbIndicatorLevelIconItem::setValue(int val)
{
    mValue = val;
}

void HbIndicatorLevelIconItem::setDirection(IndicatorDirection direction)
{
    mIndicatorDirection = direction;
}

void HbIndicatorLevelIconItem::paint(QPainter *painter,
                                     const QStyleOptionGraphicsItem *option,
                                     QWidget *widget)
{
	Q_UNUSED(widget);

    QRectF iconRect(boundingRect());
    
    // we save (& restore) the painter state here, so that we can enable the 
    // QGraphicsView::DontSavePainterState flag by default in the Orbit project
    painter->save();
    
    // solve clipping rect according to level value
    qreal clipWidth = iconRect.width();
    if (mValue >= 0 && mValue <= 100) {
        clipWidth = ((qreal)mValue / 100) * clipWidth;
    }

    if ((mIndicatorDirection == LeftToRight && layoutDirection() == Qt::LeftToRight) || 
         (mIndicatorDirection == RightToLeft && layoutDirection() == Qt::RightToLeft)) {
        QRectF clipRect(iconRect.left(), iconRect.top(),
                        clipWidth, iconRect.height());
        painter->setClipRect(clipRect);
    } else { // RightToLeft
        QRectF clipRect(iconRect.right() - clipWidth, iconRect.top(),
                        clipWidth, iconRect.height());
        painter->setClipRect(clipRect);
    }

    HbIconItem::paint(painter, option);
    painter->restore();
}

