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
#include "hbslidercontrol_p.h"
#include "hbsliderhandle_p.h"
#include <hbstyle.h>
#include <hbstyleoptionslider_p.h>
#include <hbstyle.h>
#include <hbapplication.h>
#include <hbiconitem.h>
#include <QList>






class HbSliderTickmarksPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC( HbSliderTickmarks )

public:
    HbSliderTickmarksPrivate();
    void createTicks( );
    void updateTickSize( );
    HbStyleOptionSlider sliderOption;
    QList<QGraphicsWidget *> tickmarkmajorIcons;
    QList<QGraphicsWidget *> tickmarkminorIcons;
    HbSliderControl *sliderControl;
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
    sliderControl = 0;
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
    int minimum = sliderControl->minimum();
    int maximum = sliderControl->maximum();
    int majorTickInterval = sliderControl->majorTickInterval ( );
    int minorTickInterval = sliderControl->minorTickInterval ( );
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

void HbSliderTickmarksPrivate::updateTickSize()
{
    for(int i=0;i<tickmarkmajorIcons.length();i++) {
        tickmarkmajorIcons.at(i)->setMinimumSize(majorTickWidth,majorTickHeight);
        tickmarkmajorIcons.at(i)->setMaximumSize(majorTickWidth,majorTickHeight);
    }
    for(int i=0;i<tickmarkminorIcons.length();i++) {
        tickmarkminorIcons.at(i)->setMinimumSize(minorTickWidth,minorTickHeight);
        tickmarkminorIcons.at(i)->setMaximumSize(minorTickWidth,minorTickHeight);
    }


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
    d->sliderControl=dynamic_cast<HbSliderControl*>( parentItem() );
    d->createTicks();
}

/*!
    destructor
*/
HbSliderTickmarks::~HbSliderTickmarks()
{
}

void HbSliderTickmarks::createIcons( bool create ) 
{

    Q_D(HbSliderTickmarks);
    d->createIcons = create;
    if (!create) {
        while ( d->tickmarkmajorIcons.length() > 0) {
            QGraphicsWidget *iconItem = d->tickmarkmajorIcons.at(0);
            d->tickmarkmajorIcons.removeAll(iconItem);
            delete iconItem;
        }
        while ( d->tickmarkminorIcons.length() > 0) {
            QGraphicsWidget *iconItem = d->tickmarkminorIcons.at(0);
            d->tickmarkminorIcons.removeAll(iconItem);
            delete iconItem;
        }
    } else {
        d->createTicks();
    }
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
    d->updateTickSize();
    int minimum = d->sliderControl->minimum();
    int maximum = d->sliderControl->maximum();
    int majorTickInterval = d->sliderControl->majorTickInterval ( );
    int minorTickInterval = d->sliderControl->minorTickInterval ( );
    qreal span = 0;
    bool rtlLayout = (((d->sliderControl->orientation()!=Qt::Vertical)&&
        (HbApplication::layoutDirection() == Qt::LeftToRight))?false:true);
    HbSliderHandle *handle = dynamic_cast <HbSliderHandle *> (d->sliderControl->primitive (HbStyle::P_Slider_thumb));
    if ( d->sliderControl->orientation() == Qt::Horizontal) {
        span = d->sliderControl->size().width();
        span-=handle->size().width();
    }
    if ( d->sliderControl->orientation() == Qt::Vertical) {
        span = d->sliderControl->size().height();
        span-=handle->size().height();
    }
    if (majorTickInterval) {
        int totalMajorTicks = ((maximum-minimum)/majorTickInterval)+1;
        for (int i=0;i<totalMajorTicks;i++) {
             QGraphicsWidget *iconItem = d->tickmarkmajorIcons.at ( i);
             HbStyleOptionSlider opt;
             initStyleOption(&opt);
             opt.orientation = d->sliderControl->orientation();
             style()->updatePrimitive(iconItem,HbStyle::P_SliderTickMark_majoricon,&opt);
            int pos = QStyle::sliderPositionFromValue( minimum, maximum,
                minimum+majorTickInterval*i,static_cast<int>( span ), rtlLayout );

            if ( d->sliderControl->orientation() == Qt::Horizontal) {
                qreal correctedPosX = handle->size().width()/2+pos; 
                qreal correctedPosY = 0;
                iconItem->setPos ( correctedPosX,correctedPosY );
                iconItem->update();
            } else {
                qreal correctedPosY = handle->size().height()/2+pos;
                qreal correctedPosX =0;
                iconItem->setPos ( correctedPosX,correctedPosY );
                iconItem->update();
            }
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
            opt.orientation = d->sliderControl->orientation();
            style()->updatePrimitive(iconItem,HbStyle::P_SliderTickMark_minoricon,&opt);
            int pos = QStyle::sliderPositionFromValue( minimum, maximum,
                minimum+minorTickInterval*i,static_cast<int>( span ), rtlLayout );
 
            if ( d->sliderControl->orientation() == Qt::Horizontal) {
                qreal correctedPosX = handle->size().width()/2+pos; 
                qreal correctedPosY = 0;
                iconItem->setPos ( correctedPosX,correctedPosY );
                iconItem->update();

            } else {
                qreal correctedPosY = handle->size().height()/2+pos;
                qreal correctedPosX =0;
                iconItem->setPos ( correctedPosX,correctedPosY );
                iconItem->update();

            }
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

