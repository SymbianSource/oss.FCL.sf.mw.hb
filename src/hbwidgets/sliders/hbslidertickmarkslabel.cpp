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

#include "hbslidertickmarkslabel_p.h"
#include "hbslidercontrol_p.h"
#include "hbslider_p.h"
#include "hbwidget_p.h"
#include <hbinstance.h>
#include <hbstyle.h>
#include <hbstyleoptionslider_p.h>
#include <hbapplication.h>
#include <hbtextitem.h>
#include <hbslider.h>
#include <QList>
#include <QStringList>
#include <QEvent>
#include <QGraphicsItem>


class HbSliderTickmarksLabelPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC( HbSliderTickmarksLabel )

public:
    HbSliderTickmarksLabelPrivate();
    void createTickLabels( );
    HbStyleOptionSlider sliderOption;
    QList<QGraphicsWidget *> tickmarkmajorIconItemsLabel;
    QList<QGraphicsWidget *> tickmarkminorIconItemsLabel;
    HbSlider *slider;
    Hb::SliderTickPositions tickPosition;
    bool createText;

};


 HbSliderTickmarksLabelPrivate::HbSliderTickmarksLabelPrivate() :HbWidgetPrivate(){
    tickmarkmajorIconItemsLabel.clear();
    tickmarkminorIconItemsLabel.clear();
    slider = 0;
    tickPosition = Hb::NoSliderTicks;
    createText = true;


}


void  HbSliderTickmarksLabelPrivate::createTickLabels(  )
{
    Q_Q ( HbSliderTickmarksLabel );
    if (!createText) {
        return;
    }
    bool textItemCreated = false;
    
    int minimum = slider->minimum();
    int maximum = slider->maximum();
    QStringList majorLabelList = slider->majorTickLabels( );
    QStringList minorLabelList = slider->minorTickLabels( );
    int majorTickInterval = slider->majorTickInterval ( );
    int minorTickInterval = slider->minorTickInterval ( );
    if (majorTickInterval) {
        int totalMajorTicksLabel = ((maximum-minimum)/majorTickInterval)+1;
        if (majorLabelList.length( ) < totalMajorTicksLabel ) {
            totalMajorTicksLabel = majorLabelList.length( );
        }
        int majorLabelListLength =  tickmarkmajorIconItemsLabel.length();
        for (int i=majorLabelListLength;i<totalMajorTicksLabel;i++) {
            QGraphicsItem *textItem = q->style()->createPrimitive(HbStyle::P_SliderTickMark_majorlabel, q);
            textItemCreated = true;
            Q_ASSERT(textItem->isWidget());
            tickmarkmajorIconItemsLabel.append(static_cast<QGraphicsWidget *>(textItem));//add newly defind primitive
        }
        while ( totalMajorTicksLabel < tickmarkmajorIconItemsLabel.length() ) {
            QGraphicsWidget *textItem = tickmarkmajorIconItemsLabel.at(totalMajorTicksLabel);
            tickmarkmajorIconItemsLabel.removeAll(textItem);
            delete textItem;
        }
    }
    if ( minorTickInterval > 0) {
        int totalMinorTicksLabel = ((maximum-minimum)/minorTickInterval)+1;
        if ( majorTickInterval ) {
            int maximumMinorTicksLabel = totalMinorTicksLabel;
            for (int i=0;i< maximumMinorTicksLabel;i++ ) {
                if ((i*minorTickInterval)%majorTickInterval==0) {
                    totalMinorTicksLabel--;
                }
            }
        }
        if (minorLabelList.length( ) < totalMinorTicksLabel ) {
            totalMinorTicksLabel = minorLabelList.length( );
        }

        int minorIconLabelListLength =  tickmarkminorIconItemsLabel.length();
        for (int i=minorIconLabelListLength;i<totalMinorTicksLabel;i++) {
            QGraphicsItem *textItem = q->style()->createPrimitive(HbStyle::P_SliderTickMark_minorlabel, q);
            textItemCreated = true;
            Q_ASSERT(textItem->isWidget());
            tickmarkminorIconItemsLabel.append(static_cast<QGraphicsWidget *>(textItem));//add newly defind primitive
        }
        while (totalMinorTicksLabel < tickmarkminorIconItemsLabel.length() ){
            QGraphicsWidget *textItem = tickmarkminorIconItemsLabel.at(totalMinorTicksLabel);
            tickmarkminorIconItemsLabel.removeAll(textItem);
            delete textItem;
        }
    }else {
        while (tickmarkminorIconItemsLabel.length() > 0){
            QGraphicsWidget *textItem = tickmarkminorIconItemsLabel.at(0);
            tickmarkminorIconItemsLabel.removeAll(textItem);
            delete textItem;
        }
    }
    q->setProperty("state","normal");
    if( textItemCreated ) {
       q->repolish();
}
}

void HbSliderTickmarksLabel::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_UNUSED (event);
    repolish ( );
    updateTickLabels();
    HbWidget::resizeEvent(event);
}

void HbSliderTickmarksLabel::updateTickLabels( )
{
    Q_D ( HbSliderTickmarksLabel );
    if (!d->createText) {
        return;
    }
    d->createTickLabels();
    int totalMajorTicksLabel = d->tickmarkmajorIconItemsLabel.length();
    for (int i=0;i<totalMajorTicksLabel;i++) {
         QGraphicsWidget *textItem = d->tickmarkmajorIconItemsLabel.at ( i);
         HbStyleOptionSlider opt;
         initStyleOption(&opt);
         opt.orientation = d->slider->orientation();
         opt.text = (d->slider->majorTickLabels( )).at(i);
         style()->updatePrimitive(textItem,HbStyle::P_SliderTickMark_majorlabel,&opt);
	}
    int minimum = d->slider->minimum();
    int maximum = d->slider->maximum();
    int majorTickInterval = d->slider->majorTickInterval ( );
    int minorTickInterval = d->slider->minorTickInterval ( );
    qreal span = 0;
    bool rtlLayout = (((d->slider->orientation( ) != Qt::Vertical)
        &&(HbApplication::layoutDirection() == Qt::LeftToRight))?false:true);
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
    int minPos = QStyle::sliderPositionFromValue( minimum, maximum,
                minimum,static_cast<int>( span ), rtlLayout );
    int firstMajorIntervalPos = QStyle::sliderPositionFromValue( minimum, maximum,
                minimum+majorTickInterval,static_cast<int>( span ), rtlLayout );

    int firstMinorIntervalPos = QStyle::sliderPositionFromValue( minimum, maximum,
                minimum+minorTickInterval,static_cast<int>( span ), rtlLayout );

    qreal totalMajorTextWidth = abs(firstMajorIntervalPos-minPos);
    qreal totalMinorTextWidth = abs(firstMinorIntervalPos-minPos);

    if (majorTickInterval) {
        int totalMajorTicksLabel = d->tickmarkmajorIconItemsLabel.length();
        for (int i=0;i<totalMajorTicksLabel;i++) {
             QGraphicsWidget *textItem = d->tickmarkmajorIconItemsLabel.at ( i);
             int pos = QStyle::sliderPositionFromValue( minimum, maximum,
                minimum+majorTickInterval*i,static_cast<int>( span ), rtlLayout );

             if ( d->slider->orientation() == Qt::Horizontal) {
                textItem->update();
                qreal correctedPosX = handleSize.width()/2+pos;
                qreal tickWidth = totalMajorTextWidth;
                correctedPosX -= tickWidth/2;
                qreal correctedPosY = 0;
                qreal tickHeight = textItem->boundingRect().size().height();
                textItem->setGeometry( QRectF(correctedPosX,correctedPosY,tickWidth,tickHeight));
            } else {
                qreal correctedPosY = handleSize.height()/2+pos;
                qreal tickHeight = textItem->boundingRect().size().height();
                correctedPosY-=tickHeight/2;
                qreal correctedPosX =0;
                qreal tickWidth = boundingRect().width();
                textItem->setGeometry ( QRectF(correctedPosX,correctedPosY,tickWidth,tickHeight ));
                textItem->setLayoutDirection (layoutDirection());
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
            if ( minorIndex >= d->tickmarkminorIconItemsLabel.length() ) {
                continue;
            } else {
                QGraphicsWidget *textItem = d->tickmarkminorIconItemsLabel.at ( minorIndex);
                HbStyleOptionSlider opt;
                initStyleOption(&opt);
                opt.orientation = d->slider->orientation();
                opt.text = (d->slider->minorTickLabels( )).at(minorIndex);
                style()->updatePrimitive(textItem,HbStyle::P_SliderTickMark_minorlabel,&opt);
                minorIndex++;
                int pos = QStyle::sliderPositionFromValue( minimum, maximum,
                    minimum+minorTickInterval*i,static_cast<int>( span ), rtlLayout );
                 if ( d->slider->orientation() == Qt::Horizontal) {
                    qreal correctedPosX = handleSize.width()/2+pos; 
                    correctedPosX -= totalMinorTextWidth/2;
                    qreal correctedPosY = 0;
                    qreal tickHeight = textItem->boundingRect().size().height();
                    textItem->setGeometry( QRectF(correctedPosX,correctedPosY,totalMinorTextWidth,tickHeight));
 
                } else {
                    qreal correctedPosY = handleSize.height()/2+pos;
                    qreal tickHeight = textItem->boundingRect().size().height();
                    correctedPosY-=tickHeight/2;
                    textItem->setLayoutDirection (layoutDirection());
                    qreal correctedPosX =0;
                    qreal tickWidth = boundingRect().width();
                    textItem->setGeometry ( QRectF(correctedPosX,correctedPosY,tickWidth,tickHeight ));

  
                }
            }
        }
    }
 }



 void HbSliderTickmarksLabel::setTickPosition(Hb::SliderTickPositions position)
{
    Q_D(HbSliderTickmarksLabel);
    d->tickPosition = position;
    if( d->tickPosition & Hb::SliderTicksLeft ) {
         setProperty("TicksLeft",true);
         setProperty("TicksRight",false);
    }
    else if(d->tickPosition & Hb::SliderTicksRight) {
         setProperty("TicksRight",true);
         setProperty("TicksLeft",false);
    }
}


/*!
    This class is internal to slider ,
    this creates  ticks mark label widget in slider
*/

/*!
    constructor
*/
HbSliderTickmarksLabel::HbSliderTickmarksLabel( QGraphicsItem *parent )
    : HbWidget( *new HbSliderTickmarksLabelPrivate, parent )
{
    Q_D( HbSliderTickmarksLabel );
    d->q_ptr = this;
    d->slider=dynamic_cast<HbSlider*>( parentItem() );
    d->createTickLabels();
    setFlag(QGraphicsItem::ItemSendsGeometryChanges,true);
}

/*!
    destructor
*/
HbSliderTickmarksLabel::~HbSliderTickmarksLabel()
{
}


void HbSliderTickmarksLabel::polish( HbStyleParameters& params )
{
    HbWidget::polish(params);
    updateTickLabels();
}

/*!
    \reimp

    This api update the primitive when ever item enable has changed
 */
QVariant HbSliderTickmarksLabel::itemChange( GraphicsItemChange change, const QVariant &value )
{
    switch( change )
    {
        case ItemChildAddedChange:
        case ItemChildRemovedChange:
            repolish();
            break;
        case ItemPositionHasChanged:
            repolish( );
            updateTickLabels();
            break;

        default:
            break;
    }
    return HbWidget::itemChange( change, value );
}

bool  HbSliderTickmarksLabel::event(QEvent *e)
{
    if(e->type( ) == QEvent::LayoutDirectionChange) {
        updateTickLabels( );
    }
    return HbWidget::event(e);
}


void HbSliderTickmarksLabel::initStyleOption( HbStyleOptionSlider *option ) const
{
    Q_D( const HbSliderTickmarksLabel );
    if ( !option ) {
        return;
    }
    HbWidget::initStyleOption( option );
    option->orientation = d->slider->orientation( );
    option->tickPosition =  d->slider->tickPosition( );
    option->upsideDown = ( d->slider->orientation( ) == Qt::Horizontal ) 
        ? ( d->slider->invertedAppearance( ) != ( option->direction == Qt::RightToLeft ) )
            : ( !d->slider->invertedAppearance( ) );
    // we use the upsideDown option instead
}

 //end of file

