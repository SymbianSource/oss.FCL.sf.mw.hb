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

#include <hbprogressslider.h>
#include "hbprogressslider_p.h"

#include <hbstyleoptionprogressbar.h>
#include <hbtooltip.h>
#include <hbwidgetfeedback.h>

#include <QGraphicsSceneMouseEvent>
#include <QApplication>

#ifdef HB_EFFECTS
#include <hbeffect.h>
#include "hbeffectinternal_p.h"
#define HB_PRGRESSSLIDER_ITEM_TYPE "HB_PROGRESSSLIDER"
#endif

HbProgressSliderPrivate::HbProgressSliderPrivate()
{
    downState=false;
    handle = 0;
    mSliderValue = 0;
    thumbIcon = HbIcon();
    state = HbProgressSlider::SliderStatePlayNormal;
}


HbProgressSliderPrivate::~HbProgressSliderPrivate()
{
    if (handle) {
        delete handle;
    }
}


void HbProgressSliderPrivate::init()
{
    Q_Q(HbProgressSlider);
    mSliderGraphicItem  = q->style()->createPrimitive(HbStyle::P_ProgressBar_slidertrack,mFrame);
	   	// Show the touch area which is created in ProgressBar//
    mTouchAreaItem = q->style()->createPrimitive(HbStyle::P_ProgressBar_toucharea,q);
	mFrame->setZValue(mTouchAreaItem->zValue()+1);
#ifdef HB_EFFECTS
    HbEffectInternal::add(HB_PRGRESSSLIDER_ITEM_TYPE,"progressslider_trackpress", "progressslider_trackpress");
    HbEffectInternal::add(HB_PRGRESSSLIDER_ITEM_TYPE,"progressslider_trackrelease", "progressslider_trackrelease");
#endif
}

void HbProgressSliderPrivate::emitSliderPressed()
{
    Q_Q(HbProgressSlider);
    emit q->sliderPressed();
}


void HbProgressSliderPrivate::emitSliderReleased()
{
    Q_Q(HbProgressSlider);
    emit q->sliderReleased();
}

void HbProgressSliderPrivate::emitSliderMoved(int newValue)
{
    Q_Q(HbProgressSlider);
    if (handle->isHandlePressed()) {
        HbWidgetFeedback::continuousTriggered(q, Hb::ContinuousDragged);
    }
    emit q->sliderMoved(newValue);
}

QRectF HbProgressSliderPrivate::boundingRect()const
{
    return mFrame->boundingRect();
}

HbStyle* HbProgressSliderPrivate::style() const
{
     Q_Q(const HbProgressSlider);
     return q->style();
}

QGraphicsItem* HbProgressSliderPrivate::parentGraphicsItem() const
{
    return mFrame;
}

HbWidget* HbProgressSliderPrivate::parentGraphicsWidget() const
{
    Q_Q(const HbProgressSlider);
    return const_cast<HbProgressSlider*> (q);
}

int HbProgressSliderPrivate::progressValue()const
{
    Q_Q(const HbProgressSlider);
    return q->sliderValue();
}

int HbProgressSliderPrivate::maximum() const 
{
    Q_Q(const HbProgressSlider);
    return q->maximum();
}
 
int HbProgressSliderPrivate::minimum() const 
{
    Q_Q(const HbProgressSlider);
    return q->minimum();
}
 
bool HbProgressSliderPrivate::invertedAppearance() const 
{
    Q_Q(const HbProgressSlider);
    return q->invertedAppearance();
}

QString HbProgressSliderPrivate::toolTipText() const
{
    return mTooltipText;
}

Qt::Alignment HbProgressSliderPrivate::textAlignment() const
{
    return Qt::AlignTop;
}
Qt::Orientation HbProgressSliderPrivate::orientation()
{
    return mOrientation;
}

void HbProgressSliderPrivate::updateMaskWidth(qreal width)
{
    Q_Q( HbProgressSlider );
    HbStyleOptionProgressBar progressBarOption;
    q->initStyleOption(&progressBarOption);
    progressBarOption.maskWidth = width;

    if(mTrack) {
        q->style()->updatePrimitive(mTrack, HbStyle::P_ProgressBar_slidertrack, &progressBarOption);
    }
}

void HbProgressSliderPrivate::setRange(int minimum, int maximum)
{
    Q_Q( HbProgressSlider );
    HbProgressBarPrivate::setRange(minimum, maximum);
    q->setSliderValue(mSliderValue);
}

/*!
    @beta
    @hbwidgets
    \class HbProgressSlider
    \brief Constructs a basic progress slider.
    ProgressSlider is a basic slider but the track is like a progressbar indicating how much progress
    has been done. its a slider with progressbar as its track with some additional behaviour. 
    There is also a progressValue which indicates the amount of buffered data.General use
    for this widget would be for playing music online which indicates sliderValue as currentTime and 
    progressValue as the buffered amount.

    HbProgressSlider is derived from HbProgressBar so it supports all the features supported by HbProgressBar.
    infinite and rating progress bar feature is disabled in HbProgressSlider.
    \sa HbProgressBar
*/
HbProgressSlider::HbProgressSlider(QGraphicsItem *parent) :
    HbProgressBar(*new HbProgressSliderPrivate,HbProgressBar::SimpleProgressBar,parent)
{
    Q_D( HbProgressSlider );
    d->q_ptr = this;
    d->init();


    setFocusPolicy(Qt::FocusPolicy(qApp->style()->styleHint(QStyle::SH_Button_FocusPolicy)));
    d->handle =new HbProgressSliderHandle(d);
    if(!flags().testFlag(ItemIsFocusable)) {
        d->handle->setVisible(false);
    }
    if(d->mOrientation == Qt::Horizontal){
		qreal x = boundingRect().height();
		qreal y = d->handle->size().height();
		qreal a = qreal (x-y)/2;
        d->handle->setPos(0,a);

    }
    else{
        d->handle->setPos(0,boundingRect().height());
    }
}


/*!
    @beta
    Constructs a progressslider  of a given \a parent.
*/
HbProgressSlider::HbProgressSlider(HbProgressSliderPrivate &dd,QGraphicsItem *parent) : 
    HbProgressBar( dd,HbProgressBar::SimpleProgressBar,parent)
{
    Q_D( HbProgressSlider );
    d->init();

	setFocusPolicy(Qt::FocusPolicy(qApp->style()->styleHint(QStyle::SH_Button_FocusPolicy)));
    d->handle =new HbProgressSliderHandle(d);
	d->handle->setZValue(1010);
    if(d->mOrientation == Qt::Horizontal){
		qreal posY = qreal (boundingRect().height()-d->handle->size().height())/2;
        d->handle->setPos(0,posY);
    }
    else{
        d->handle->setPos(0,boundingRect().height());
    }
}


/*!
    @beta
    Destructor for the progressslider.
*/
HbProgressSlider::~HbProgressSlider()
{
}

/*!
    @beta
    Sets the current value of the progress slider.

    The progress slider forces the value to be within the legal range: \b
    minimum <= \c value <= \b maximum.

    \sa value()
*/
void HbProgressSlider::setSliderValue(int value)
{
    Q_D( HbProgressSlider );

    if(d->handle->isHandlePressed() && !d->handle->isHandleMoving()){
        return ;
    }
    if (value>maximum()) {
        value = maximum();
    }

    if (value<minimum()) {
        value = minimum();
    }
    d->mSliderValue = value;

    HbStyleOptionProgressBar progressBarOption;
    initStyleOption(&progressBarOption);
    if (d->mSliderGraphicItem) {
        style()->updatePrimitive(d->mSliderGraphicItem, HbStyle::P_ProgressBar_slidertrack, &progressBarOption);
       /* if( value == d->mMaximum ) {
            d->handle->changeIcon();
        }*/
    }
    d->handle->setHandlePosForValue(sliderValue());
}

/*!
    @beta
    Returns the current slider value . 

    The default value is \c 0.

    \sa setSliderValue()
*/
int HbProgressSlider::sliderValue() const
{
    Q_D(const HbProgressSlider );
    return d->mSliderValue;
}


/*!
    @beta
    Returns \c true whether the slider is pressed down.
*/

bool HbProgressSlider::isSliderDown() const
{
    Q_D( const HbProgressSlider );
    return d->handle->isHandlePressed();
}


/*!
    @beta
    Sets the inverted appearance flag. 
*/
void HbProgressSlider::setInvertedAppearance(bool inverted)
{
    Q_D( HbProgressSlider );
    if(d->mInvertedAppearance != inverted) {
        d->mInvertedAppearance=inverted;
        d->handle->setHandlePosForValue(sliderValue());
        updatePrimitives();
    }
}


/*!
    Sets the Icon for the progressslider thumb.

    \deprecated HbProgressSlider::setThumbIcon(const HbIcon&)
        is deprecated. 
*/
void HbProgressSlider::setThumbIcon(const HbIcon &icon)
{
    Q_D( HbProgressSlider );
    d->handle->setThumbIcon(icon);
}

/*!
    Returns the Icon for the progressslider thumb.

    \deprecated HbProgressSlider::thumbIcon() const
        is deprecated. 
*/
HbIcon HbProgressSlider::thumbIcon() const
{
    Q_D( const HbProgressSlider );
    return d->handle->thumbIcon();
}

void HbProgressSlider::mousePressEvent(QGraphicsSceneMouseEvent *event) 
{
    Q_D(HbProgressSlider);
    if(flags().testFlag(ItemIsFocusable)) {
        qreal temp = event->scenePos().x();
        if((d->mMinMaxTextVisible) && (d->mMinMaxTextAlignment== Qt::AlignCenter)) {
		    temp -=  d->mMinTextItem->boundingRect().width();
        }
        if( (temp > d->handle->pos().x()) && (temp < (d->handle->boundingRect().width()+d->handle->pos().x())) ) {
            event->ignore();
            return;
        }
        
        HbWidgetFeedback::triggered(this, Hb::InstantPressed);
        d->handle->handleTrackPress(event);
        event->accept();
		#ifdef HB_EFFECTS
	        HbEffect::start(this, HB_PRGRESSSLIDER_ITEM_TYPE, "progressslider_trackpress");
        #endif
    } else {
        event->ignore();
    }
}


void HbProgressSlider::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) 
{
    Q_D(HbProgressSlider);
    if(flags().testFlag(ItemIsFocusable)) {
        HbWidgetFeedback::triggered(this, Hb::InstantReleased);
        d->handle->handleTrackRelease(event);
        event->accept();
#ifdef HB_EFFECTS
        HbEffect::start(this, HB_PRGRESSSLIDER_ITEM_TYPE, "progressslider_trackrelease");
#endif
    } else {
        event->ignore();
    }
  
}

void HbProgressSlider::setGeometry(const QRectF & rect)
{
    Q_D(HbProgressSlider);
    HbProgressBar::setGeometry(rect);
    d->handle->setHandlePosForValue(sliderValue());
    updatePrimitives();
}


void HbProgressSlider::initStyleOption(HbStyleOption *hboption) const
{
    Q_ASSERT(hboption);
    Q_D(const HbProgressSlider);
    HbProgressBar::initStyleOption(hboption);
    HbStyleOptionProgressBar *option = 0;
    if ((option = qstyleoption_cast< HbStyleOptionProgressBar *>(hboption)) != 0){
        option->secondoryType = true;
        option->sliderValue = d->mSliderValue;
        option->inverted = d->mInvertedAppearance;
        option->maximum = d->mMaximum;
        option->minimum = d->mMinimum;        
    }
}


void HbProgressSlider::updatePrimitives()
{
    HbProgressBar::updatePrimitives();

    Q_D(HbProgressSlider);
    if(isVisible()){
        d->mWaitTrack->setVisible(false);
        d->mTrack->setVisible(true);
        HbStyleOptionProgressBar progressBarOption;
        initStyleOption(&progressBarOption);
        if (d->mSliderGraphicItem) {
            style()->updatePrimitive(d->mSliderGraphicItem, HbStyle::P_ProgressBar_slidertrack, &progressBarOption);
        }

        if(d->handle)
              d->handle->setHandlePosForValue(sliderValue());
    }
    
}

void HbProgressSlider::showEvent( QShowEvent * event )
{
	Q_D(const HbProgressSlider);
	if(d->mTouchAreaItem && scene()) {
        d->mTouchAreaItem->removeSceneEventFilter(this);
        d->mTouchAreaItem->installSceneEventFilter(this);
    }

    HbProgressBar::showEvent(event);
}
QVariant HbProgressSlider::itemChange(GraphicsItemChange change,const QVariant & value)
{ 
    Q_D(HbProgressSlider);
    if (change == ItemFlagsChange) {
        if(value.toInt() & ItemIsFocusable) {
            if(!flags().testFlag(ItemIsFocusable) && d->handle) {
                d->handle->setVisible(true);

            }
        } else {
            if(flags().testFlag(ItemIsFocusable) && d->handle) {
                d->handle->setVisible(false);
            }
        }

    }
    return HbProgressBar::itemChange(change, value);
}

bool HbProgressSlider::sceneEventFilter(QGraphicsItem *obj,QEvent *event)
{
    Q_D(HbProgressSlider);
	if( obj == d->mTouchAreaItem) {
       if (!isEnabled() ) {
            return false;
        }
		if (event->type() == QEvent::GraphicsSceneMousePress){
			mousePressEvent((QGraphicsSceneMouseEvent *) event);
             return true;
		}
		else if (event->type() == QEvent::GraphicsSceneMouseRelease){
			mouseReleaseEvent((QGraphicsSceneMouseEvent *) event);
            return true;
		}
	} 
	return false;
}

/*!
    set the tooltip text . 
    
    \deprecated HbProgressSlider::setHandleToolTip(const QString&)
        is deprecated. Please use HbProgressSlider::setSliderToolTip(const QString &text) instead.

    \sa handleToolTip()
*/
void HbProgressSlider::setHandleToolTip(const QString &text)
{
    setSliderToolTip(text);
}


/*!
    Returns the current tooltip text value.
    
    \deprecated HbProgressSlider::handleToolTip() const
        is deprecated. Please use HbProgressSlider::sliderToolTip() const instead.

    \sa setHandleToolTip()
*/
QString HbProgressSlider::handleToolTip() const
{
    return sliderToolTip();
}

/*!
    Sets the state of the handle as normal play,pressed play,normal pause,pressed pause etc. 

    \deprecated HbProgressSlider::setHandleState(HbProgressSlider::HandleState)
        is deprecated. Please use HbProgressSlider::setSliderState(HbProgressSlider::SliderState state) instead.

*/
void HbProgressSlider::setHandleState(HbProgressSlider::HandleState state)
{
    setSliderState((HbProgressSlider::SliderState)state);
}
/*!
    Returns the state of the handle. 

    \deprecated HbProgressSlider::handleState() const
        is deprecated. Please use HbProgressSlider::sliderState() const instead.
*/
HbProgressSlider::HandleState HbProgressSlider::handleState() const
{
    return (HbProgressSlider::HandleState)sliderState();
}


/*!
    @beta

    Sets the tooltip text . 
    \sa sliderToolTip()
*/
void HbProgressSlider::setSliderToolTip(const QString &text)
{
    Q_D(HbProgressSlider);
    d->mTooltipText = text;
}


/*!
    @beta

    Returns the current tooltip text value . 
    \sa setSliderToolTip()
*/
QString HbProgressSlider::sliderToolTip() const
{
    Q_D(const HbProgressSlider);
    return d->mTooltipText;
}

/*!
    @beta
    
    Sets the state of the handle as SliderStatePlayNormal, SliderStatePlayPressed,
    SliderStatePauseNormal, SliderStatePausePressed.

    \sa sliderState()

*/
void HbProgressSlider::setSliderState(HbProgressSlider::SliderState state)
{
	Q_D(HbProgressSlider);
	if(d->state != state) {
		d->state = state;
		if (d->handle) {
			d->handle->mSliderState = state;

            switch(d->handle->mSliderState) {
               
               case HbProgressSlider::SliderStatePlayNormal:
                   d->handle->setProperty("state","normal");
                   break;
               
               case HbProgressSlider::SliderStatePlayPressed:
                   d->handle->setProperty("state","pressed");
                   break;
               
               case HbProgressSlider::SliderStatePauseNormal:
                    d->handle->setProperty("state","normal");
                   break;
               
               case HbProgressSlider::SliderStatePausePressed:
                    d->handle->setProperty("state","pressed");
                   break;
            }

			d->handle->updatePrimitives();
			
		}
	}
}
/*!
    @beta

    Returns the state of the handle. 

    \sa setSliderState()
*/
HbProgressSlider::SliderState HbProgressSlider::sliderState() const
{
	Q_D(const HbProgressSlider);
	return d->state;

}


/*!
    Returns the pointer for \a primitive passed.
    Will return NULL if \a primitive passed is invalid
*/
QGraphicsItem* HbProgressSlider::primitive(HbStyle::Primitive primitive) const
{
    Q_D(const HbProgressSlider);

    switch (primitive) {
        case HbStyle::P_ProgressBar_slidertrack:
            return d->mSliderGraphicItem;
        case HbStyle::P_ProgressBar_toucharea:
            return d->mTouchAreaItem;  
        default:
            return 0;
    }
}
