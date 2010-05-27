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

#include "hbprogresstrackitem_p.h"
#include "hbframedrawer.h"
#include <QPainter>
#include "hbframedrawer_p.h"
#include <QBitmap>

HbProgressTrackItem::HbProgressTrackItem(QGraphicsItem* parent)
            :HbFrameItem(new HbFrameDrawer(this),parent)
{
    minimum = 0;
    maximum = 0;
    value = 0;
    maskWidth = 0;
}

HbProgressTrackItem::HbProgressTrackItem(HbFrameDrawer *drawer, QGraphicsItem *parent)
    :HbFrameItem(drawer,parent)
{
    minimum = 0;
    maximum = 0;
    value = 0;
    maskWidth = 0;
    mDiff = maximum - minimum;
}
void HbProgressTrackItem::setMinimum(int min)
{
    minimum = min;
    mDiff = maximum - minimum;
}
void HbProgressTrackItem::setMaximum(int max )
{
    maximum = max;
    mDiff = maximum - minimum;
}
void HbProgressTrackItem::setValue(int val )
{
    value = val;
}
void HbProgressTrackItem::setInverted(bool inv)
{
    inverted = inv;
}

void HbProgressTrackItem::setMaskWidth(qreal width)
{
    maskWidth = width;
}
void HbProgressTrackItem::setOrientation(Qt::Orientation orientation)
{
    mOrientation = orientation;
}
void HbProgressTrackItem::paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED(widget);
	Q_UNUSED(option);

    if(boundingRect()!= parentItem()->boundingRect()){
         setGeometry(parentItem()->boundingRect());
    }
 /*   QSize size = parentItem()->boundingRect().size().toSize();
    if(size.width() == 0 || size.height() == 0){
        size.setWidth((int)boundingRect().width());
        size.setHeight((int)boundingRect().height());
    }*/
    QRectF maskRect;
    if(maximum != minimum) {
        if(maskWidth == 0) {
            if(mOrientation == Qt::Horizontal){
                qreal left = (qreal)boundingRect().topLeft().x();
                if(inverted) {
                    left = (qreal)boundingRect().width()* ((maximum - value)/(qreal) (mDiff));
                }
                maskRect = QRectF(
                        left,
                        (qreal)boundingRect().topLeft().y(),
                        (qreal)boundingRect().width()* ((value -  minimum)/(qreal) (mDiff)),
                        (qreal)boundingRect().height()
                    );

            }
            else{
            
				qreal start = boundingRect().bottom() -(qreal)boundingRect().height()*((value -  minimum)/(qreal) (mDiff));
				 maskRect = QRectF(
                        (qreal)boundingRect().topLeft().x(),
                        start,
                        (qreal)boundingRect().width(),
                        boundingRect().height()-start
                    );
            }
        }
    }

	QPainterPath path;
	path.addRect(maskRect);
	frameDrawer().setClipPath(path);         
    HbFrameItem::paint(painter,option,widget);
}


