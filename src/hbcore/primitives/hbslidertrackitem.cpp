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

#include "hbslidertrackitem_p.h"
#include "hbframedrawer.h"
#include <QPainter>
#include <QWidget>

HbSliderTrackItem::HbSliderTrackItem(QGraphicsItem* parent)
            :HbFrameItem(parent)
{
    minimum = 0;
    maximum = 0;
    value = 0;
    maskWidth = 0;
    trackSpan = 0;
    setMask   = true;
}

HbSliderTrackItem::HbSliderTrackItem(HbFrameDrawer *drawer, QGraphicsItem *parent)
    :HbFrameItem(drawer,parent)
{
    minimum = 0;
    maximum = 0;
    value = 0;
    maskWidth = 0;
    trackSpan = 0;
    setMask   = true;
}

void HbSliderTrackItem::setMinimum(int min)
{
    if ( minimum != min ) {
        minimum = min;
        setMask = true;
    }
}

void HbSliderTrackItem::setMaximum(int max )
{
    if ( maximum != max ) {
        maximum = max;
        setMask = true;
    }
}

void HbSliderTrackItem::setValue(int val )
{
    if ( value != val ) {
        value = val;
        setMask = true;
    }
}

void HbSliderTrackItem::setInverted(bool inv)
{
    if ( inverted != inv ) {
        inverted = inv;
        setMask = true;
    }
}

void HbSliderTrackItem::setMaskWidth(qreal width)
{
    if ( maskWidth != width ) {
        maskWidth = width;
        setMask = true;
    }
}

void HbSliderTrackItem::setOrientation(Qt::Orientation orientation)
{
    if ( mOrientation != orientation ) {
        mOrientation = orientation;
        setMask = true;
    }
}

void HbSliderTrackItem::setSpan( qreal span )
{
    if ( trackSpan != span ) {
        trackSpan = span;
        setMask = true;
    }
}

void HbSliderTrackItem::paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{

    if ( setMask ) {
        QSize size = boundingRect().size().toSize();
     
        QPixmap mask(size);
        mask.fill(Qt::white);
        QPainter p;
        p.begin(&mask);

        p.setBrush(QBrush(Qt::black));
        if(maximum != minimum) {
            if(maskWidth == 0) {
                if(mOrientation == Qt::Horizontal){
                    qreal left = (qreal)boundingRect().topLeft().x();
                    if(inverted) {
                        left = (qreal)boundingRect().width()* ((maximum - value)/(qreal) (maximum - minimum));
                    }
                    if ( layoutDirection() == Qt::RightToLeft ) {

                        p.drawRect(QRectF(
                            left,
                            (qreal)boundingRect().topLeft().y(),
                            (qreal)boundingRect().width()* ((value -  minimum)/(qreal) (maximum - minimum)),
                            (qreal)boundingRect().height()));

                    }
                    else {
                        p.drawRect(QRectF(
                            left,
                            (qreal)boundingRect().topLeft().y(),
                            (qreal)trackSpan* ((value -  minimum)/(qreal) (maximum - minimum)),
                            (qreal)boundingRect().height()));
                    }

                }
                else{
                    qreal start = boundingRect().bottom() -(qreal)trackSpan*((value -  minimum)/(qreal) (maximum - minimum));
                    qreal end = boundingRect().bottom();

                    if(inverted) {
                        start = boundingRect().top();
                        end = start+(qreal)trackSpan*((value -  minimum)/(qreal) (maximum - minimum));
                    }
                    p.drawRect(QRectF(
                        (qreal)boundingRect().topLeft().x(),
                        start,
                        (qreal)boundingRect().width(),
                        end ));
                }
            }
        }
        p.end();        
                  
        frameDrawer().setMask(mask); 
        setMask = false; 
    }
    HbFrameItem::paint(painter,option,widget);

}

 /*!
    \reimp
 */
void HbSliderTrackItem::changeEvent(QEvent *event)
{
    HbFrameItem::changeEvent( event );   
    if ( event->type() == QEvent::LayoutDirectionChange ) {
        setMask = true;
    }
   
}

