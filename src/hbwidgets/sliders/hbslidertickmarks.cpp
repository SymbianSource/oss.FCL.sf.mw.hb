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

#include "hbslidertickmarks_p.h"
#include "hbwidget_p.h"
#include "hbsliderhandle_p.h"
#include "hbslider_p.h"
#include <hbstyle.h>
#include <hbstyleoptionslider_p.h>
#include <hbstyle.h>
#include <hbapplication.h>
#include <hbiconitem.h>
#include <hbslider.h>
#include <QList>
#include <QGraphicsItem>
#include <QGraphicsSceneEvent>
#include <QGraphicsScene>


#ifdef HB_EFFECTS
#include "hbeffect.h"
#include "hbeffectinternal_p.h"
#define HB_SLIDER_TYPE "HB_SLIDER"
#endif






class HbSliderTickmarksPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC( HbSliderTickmarks )

public:
    HbSliderTickmarksPrivate();
    void createTicks( );
    HbStyleOptionSlider sliderOption;
    QList<QGraphicsWidget *> tickmarkmajorIcons;
    QList<QGraphicsWidget *> tickmarkminorIcons;
    HbSlider *slider;
    Hb::SliderTickPositions tickPosition;
    bool createIcons;
    int majorTickWidth;
    int minorTickWidth;
    int majorTickHeight;
    int minorTickHeight;
};


 HbSliderTickmarksPrivate::HbSliderTickmarksPrivate() :HbWidgetPrivate(){
    tickmarkmajorIcons.clear();
    tickmarkminorIcons.clear();
    slider = 0;
    tickPosition = Hb::NoSliderTicks;
    createIcons = true;
    majorTickWidth = 0;
    minorTickWidth = 0;
    majorTickHeight = 0;
    minorTickHeight = 0;
}


void  HbSliderTickmarksPrivate::createTicks(  )
{
    Q_Q ( HbSliderTickmarks );
    if(!createIcons){
        return;
    }
    int minimum = slider->minimum();
    int maximum = slider->maximum();
    int majorTickInterval = slider->majorTickInterval ( );
    int minorTickInterval = slider->minorTickInterval ( );
    if (majorTickInterval) {
        int totalMajorTicks = ((maximum-minimum)/majorTickInterval)+1;
        int majorIconListLength =  tickmarkmajorIcons.length();
        for (int i=majorIconListLength;i<totalMajorTicks;i++) {
            QGraphicsItem *iconItem = q->style()->createPrimitive(HbStyle::P_SliderTickMark_majoricon, q);
            Q_ASSERT(iconItem->isWidget());
            tickmarkmajorIcons.append(static_cast<QGraphicsWidget *>(iconItem));//add newly defind primitive
        }
        while ( totalMajorTicks < tickmarkmajorIcons.length() ) {
            QGraphicsWidget *iconItem = tickmarkmajorIcons.at(totalMajorTicks);
            tickmarkmajorIcons.removeAll(iconItem);
            delete iconItem;
        }
    }
    if ( minorTickInterval > 0) {
        int totalMinorTicks = ((maximum-minimum)/minorTickInterval)+1;
        if ( majorTickInterval ) {
            int maximumMinorTicks = totalMinorTicks;
            for (int i=0;i< maximumMinorTicks;i++ ) {
                if ((i*minorTickInterval)%majorTickInterval==0) {
                    totalMinorTicks--;
                }
            }
        }
        int minorIconListLength =  tickmarkminorIcons.length();
        for (int i=minorIconListLength;i<totalMinorTicks;i++) {
            QGraphicsItem *iconItem =  q->style()->createPrimitive(HbStyle::P_SliderTickMark_minoricon, q);
            Q_ASSERT(iconItem->isWidget());
            tickmarkminorIcons.append(static_cast<QGraphicsWidget *>(iconItem));//add newly defind primitive
        }
        while (totalMinorTicks < tickmarkminorIcons.length() ){
            QGraphicsWidget *iconItem = tickmarkminorIcons.at(totalMinorTicks);
            tickmarkminorIcons.removeAll(iconItem);
            delete iconItem;
        }
    } else {
        while (tickmarkminorIcons.length() > 0 ){
            QGraphicsWidget *iconItem = tickmarkminorIcons.at(0);
            tickmarkminorIcons.removeAll(iconItem);
            delete iconItem;
        }
    }
    q->setProperty("state", "normal"); 
}


void HbSliderTickmarks::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_UNUSED (event);
    HbWidget::resizeEvent( event );
    repolish();
}


/*!
    This class is internal to slider ,
    this creates ticks mark widget in slider
*/

/*!
    constructor
*/
HbSliderTickmarks::HbSliderTickmarks( QGraphicsItem *parent )
    : HbWidget( *new HbSliderTickmarksPrivate, parent )
{
    Q_D( HbSliderTickmarks );
    d->q_ptr = this;
    d->slider=dynamic_cast<HbSlider*>( parentItem() );
    d->createTicks();
}

/*!
    destructor
*/
HbSliderTickmarks::~HbSliderTickmarks()
{
}


/*!
    updates the ticks whenever there is change in position or number of ticks
*/

void HbSliderTickmarks::updateTicks( )
{
    Q_D ( HbSliderTickmarks );
    if(!d->createIcons) {
        return;
    }
    d->createTicks();
    int minimum = d->slider->minimum();
    int maximum = d->slider->maximum();
    int majorTickInterval = d->slider->majorTickInterval ( );
    int minorTickInterval = d->slider->minorTickInterval ( );
    qreal span = 0;
    bool rtlLayout = (((d->slider->orientation()!=Qt::Vertical)&&
        (HbApplication::layoutDirection() == Qt::LeftToRight))?false:true);
    HbSliderPrivate *sliderPrivate = dynamic_cast<HbSliderPrivate*>(HbSliderPrivate::d_ptr(d->slider));
    QSizeF handleSize(0.0,0.0);
    if( sliderPrivate) {
        handleSize = sliderPrivate->getHandleSize( );
    } else {
        return;
    }
    if ( d->slider->orientation() == Qt::Horizontal) {
        span = boundingRect().width();
        span-=handleSize.width();
    }
    if ( d->slider->orientation() == Qt::Vertical) {
        span = boundingRect().height();
        span-=handleSize.height();
    }
    if (majorTickInterval) {
        int totalMajorTicks = ((maximum-minimum)/majorTickInterval)+1;
        for (int i=0;i<totalMajorTicks;i++) {
             QGraphicsWidget *iconItem = d->tickmarkmajorIcons.at ( i);
             HbStyleOptionSlider opt;
             initStyleOption(&opt);
             opt.orientation = d->slider->orientation();
             style()->updatePrimitive(iconItem,HbStyle::P_SliderTickMark_majoricon,&opt);
            int pos = QStyle::sliderPositionFromValue( minimum, maximum,
                minimum+majorTickInterval*i,static_cast<int>( span ), rtlLayout );

            qreal correctedPosX = 0;
            qreal correctedPosY = 0;
            if ( d->slider->orientation() == Qt::Horizontal) {
                correctedPosX = handleSize.width()/2+pos; 
            } else {
                correctedPosY = handleSize.height()/2+pos;
            }
            iconItem->setGeometry (QRectF( correctedPosX,correctedPosY ,d->majorTickWidth,d->majorTickHeight));
            iconItem->update();
        }
    }
    if (minorTickInterval) {
        int totalminorTicks = ((maximum-minimum)/minorTickInterval)+1;
        int minorIndex = 0;
        for (int i=0;i<totalminorTicks;i++) {
            if (majorTickInterval ) {
                if (i*minorTickInterval%majorTickInterval== 0) {
                    continue;
                }
            }
            QGraphicsWidget *iconItem = d->tickmarkminorIcons.at ( minorIndex);
            minorIndex++;
            HbStyleOptionSlider opt;
            initStyleOption(&opt);
            opt.orientation = d->slider->orientation();
            style()->updatePrimitive(iconItem,HbStyle::P_SliderTickMark_minoricon,&opt);
            int pos = QStyle::sliderPositionFromValue( minimum, maximum,
                minimum+minorTickInterval*i,static_cast<int>( span ), rtlLayout );
            qreal correctedPosX = 0;
            qreal correctedPosY = 0;

            if ( d->slider->orientation() == Qt::Horizontal) {
                correctedPosX = handleSize.width()/2+pos; 
            } else {
                correctedPosY = handleSize.height()/2+pos;
            }
            iconItem->setGeometry (QRectF( correctedPosX,correctedPosY ,d->majorTickWidth,d->majorTickHeight));
            iconItem->update();
        }
    }
 }

/* !
    Sets the position of current tick
*/

void HbSliderTickmarks::setTickPosition(Hb::SliderTickPositions position)
{
    Q_D(HbSliderTickmarks);
    d->tickPosition = position;
}


/*!
    \reimp

    This api update the primitive when ever item enable has changed
 */
QVariant HbSliderTickmarks::itemChange( GraphicsItemChange change, const QVariant &value )
{
    switch( change )
    {
        case ItemChildAddedChange:
        case ItemChildRemovedChange:
            repolish();
            break;
        default:
            break;
    }
    return HbWidget::itemChange( change, value );
}



void HbSliderTickmarks::polish( HbStyleParameters& params )
{
    Q_D (HbSliderTickmarks);
    params.addParameter("fixed-width-major");
    params.addParameter("fixed-height-major");
    params.addParameter("fixed-width-minor");
    params.addParameter("fixed-height-minor");
    HbWidget::polish(params);
    d->majorTickWidth = params.value("fixed-width-major").toInt();
    d->majorTickHeight = params.value("fixed-height-major").toInt();
    d->minorTickWidth = params.value("fixed-width-minor").toInt();
    d->minorTickHeight = params.value("fixed-height-minor").toInt();
    updateTicks();
}

 //end of file

