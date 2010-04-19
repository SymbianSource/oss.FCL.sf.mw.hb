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


#include <hbratingslider.h>
#include "hbratingslider_p.h"
#include <hbtooltip.h>
#include <hbstyleoptionratingslider.h>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>

#ifdef HB_EFFECTS
#include <hbeffect.h>
#include "hbeffectinternal_p.h"
#define HB_RATINGSLIDER_ITEM_TYPE "HB_RATINGSLIDER"

#endif
#define MAX_NUMBER_OF_ICONS 10

HbRatingSliderPrivate::HbRatingSliderPrivate():
	mMousePressed(false),
	mLookupValues(0),
	mReadOnly(false),
	mNumberOfIcons(5),
	mStepCount(5),
	mCurrentValue(0),
	mFrame(0),
	mTrack(0),
	mLayoutItem(0),
	mUnratedIconName(""),
	mTouchArea(0),
	mRatedIconName("")
{	
}

HbRatingSliderPrivate::~HbRatingSliderPrivate()
{
	if(mLookupValues) {
		delete [] mLookupValues;
	}
}


void HbRatingSliderPrivate::init()
{
	Q_Q(HbRatingSlider);
	mLayoutItem = q->style()->createPrimitive(HbStyle::P_RatingSlider_layout,q);
    mFrame = q->style()->createPrimitive(HbStyle::P_RatingSlider_frame,mLayoutItem);
    mTrack = q->style()->createPrimitive(HbStyle::P_RatingSlider_track,mFrame);
    mTouchArea = q->style()->createPrimitive(HbStyle::P_RatingSlider_toucharea, q);

	HbStyle::setItemName(mLayoutItem, "frame");
	HbStyle::setItemName(mTouchArea, "toucharea");
	q->updatePrimitives();

	#ifdef HB_EFFECTS
    HbEffectInternal::add(HB_RATINGSLIDER_ITEM_TYPE,"ratingslider_appear", "ratingslider_appear");
    HbEffectInternal::add(HB_RATINGSLIDER_ITEM_TYPE,"ratingslider_disappear", "ratingslider_disappear");
    #endif

}

void HbRatingSliderPrivate::createLookupTable()
{
	if(mLookupValues) {
		delete [] mLookupValues;
		mLookupValues=0;
	}
	
	mLookupValues = new int[mStepCount];
	qreal width = mFrame->boundingRect().width();
	int bandWidth =(int) (width/mStepCount);
		
	for(int i=0;i < mStepCount;i++) {
		mLookupValues[i] = bandWidth*(i+1);
	}
}

int HbRatingSliderPrivate::calculateProgressValue(qreal pos)
{
	Q_Q(HbRatingSlider);
	
	int count=0;
	for(count=0;count< mStepCount ;count++) {
		if(pos <= mLookupValues[count])
				break;
	}
	if(q->layoutDirection() == Qt::RightToLeft) {
		
		count = mStepCount -count;
	}
	else {
        
		count++;
	}
	
	return count;
	
}

/*!
    @beta
    @HbWidgets
    \class HbRatingSlider
    \brief HbRatingSlider widget provides a Rating control.

    A HbRatingSlider is used to rate a particular movie or a song.The user can drag over the 
	slider to rate. As soon as he releases the pointer from within the area of slider the rating is done.
	The Application can configure the RatingSlider to be ReadOnly/ReadWrite at any point using setReadOnly. 
	The Application can configure a tooltip for rating assistance.

	By default there are 5 stars.User can rate in the range 1-5. By changinng the maximum it is possible to 
	attain any number of ratings. Once the rating is done the HbRatingSlider emits the signal ratingChanged. 
	Parameter of this signal is the new rating value. Which lies in the range min-max.
*/


/*!
    @beta
    Constructs a RatingSlider  a \a parent.
*/

HbRatingSlider::HbRatingSlider(QGraphicsItem *parent) :
HbWidget(*new HbRatingSliderPrivate,parent)
{
    Q_D( HbRatingSlider );
    d->q_ptr = this;
    d->init();
}


/*!
    @beta
    @HbWidgets
    \class HbRatingSlider
    \brief Constructs a basic Rating Slider
*/
HbRatingSlider::HbRatingSlider(HbRatingSliderPrivate &dd,QGraphicsItem *parent) : 
    HbWidget( dd,parent)
{
    Q_D( HbRatingSlider );
    d->init();
}


/*!
    Destructor
*/

HbRatingSlider::~HbRatingSlider()
{
}

/*!
    @beta
    Sets  the read only flag of the Rating slider. If the ReadOnly flag is true then Rating slider is not 
	interactive.Once the Rating is done The application can decide it to allow rating again or not by setting this 
	flag.

*/
void HbRatingSlider::setReadOnly(bool value)
{
	Q_D(HbRatingSlider);
	d->mReadOnly = value;
}

/*!
    @beta
    Sets the number of icons. In a Rating scenario you may have number of repeated icons. This API can be used to set 
	the number of icons required. For Example the default image is "*" and you have 5 stars. You can set the number of 
	stars  using this. By default this value is 5.

*/

void HbRatingSlider::setNumberOfIcons(int number)
{
	Q_D(HbRatingSlider);
	if ( (number <= 0) || (number > MAX_NUMBER_OF_ICONS) ){
		return;
	}
	d->mNumberOfIcons = number;
	updatePrimitives();
	d->createLookupTable();
}

/*!
    @beta
    Returns the number of icons set.

*/
int HbRatingSlider::numberOfIcons() const
{
	Q_D(const HbRatingSlider);
	return d->mNumberOfIcons;
}

/*!
    @beta
    Sets the step count for the rating slider. If the number of icons is 5 and step count is 10 then it is possible to have 10 ratings.
	one rating will be half star (by default). If the number of icons is 5 and step count is 5 then 5 ratings are possible. In this 
	case one rating will be one complete star. By default this value is 5.

*/
void HbRatingSlider::setStepCount(int count)
{
	Q_D(HbRatingSlider);
	if( (count <= 0) || (count >= 20) ) {
		return;
	}
	d->mStepCount = count;
	d->createLookupTable();	
	
	HbStyleOptionRatingSlider option;
    initStyleOption(&option);
	if (d->mTrack) {
           style()->updatePrimitive(d->mTrack, HbStyle::P_RatingSlider_track, &option);
    }

}

/*!
    @beta
    Returns the step count.

*/	
int HbRatingSlider::stepCount() const
{
	Q_D(const HbRatingSlider);
	return d->mStepCount;
}

/*!
    @beta
    Returns the read only flag of the Rating slider
*/
bool HbRatingSlider::isReadOnly() const
{
	Q_D(const HbRatingSlider);
	return d->mReadOnly;
}

/*!
    @beta
    Sets the current rating value.In future this will be qreal value :).

*/
void  HbRatingSlider::setCurrentRating(int rating)
{
	Q_D(HbRatingSlider);
	if( rating >d->mStepCount ) {
		rating = d->mStepCount;
	}
	if( (rating == d->mCurrentValue) || (rating < 0) ) {
		return;
	}

	d->mCurrentValue = rating;
	
	HbStyleOptionRatingSlider option;
    initStyleOption(&option);
	if (d->mTrack) {
           style()->updatePrimitive(d->mTrack, HbStyle::P_RatingSlider_track, &option);
    }
}

/*!
    @beta
    Returns the the current rating value.
*/
int HbRatingSlider::currentRating() const
{
	Q_D(const HbRatingSlider);
	return d->mCurrentValue;
}

/*!
    @beta
    It sets the unrated graphics name.This is the graphics shown when rating slider is displayed.
*/
void HbRatingSlider::setUnRatedIconName(const QString name)
{
	Q_D(HbRatingSlider);
	if(d->mUnratedIconName != name) {
		d->mUnratedIconName =name;

		HbStyleOptionRatingSlider option;
		initStyleOption(&option);
		updatePrimitives();
	}
	
}

/*!
    @beta
    Returns the unrated graphics name .
*/
QString HbRatingSlider::unRatedIconName() const
{
	Q_D(const HbRatingSlider);
	return d->mUnratedIconName;

}

/*!
    @beta
    It sets the rated graphics name.This is the graphics shown when rating is done. 
*/
void HbRatingSlider::setRatedIconName(const QString name)
{
	Q_D(HbRatingSlider);
	if(d->mRatedIconName != name) {
		d->mRatedIconName = name;
		HbStyleOptionRatingSlider option;
		initStyleOption(&option);
		updatePrimitives();
	}
}

/*!
    @beta
    Returns the rated graphics name .
*/
QString HbRatingSlider::ratedIconName() const 
{
	Q_D(const HbRatingSlider);
	return d->mRatedIconName;
}

/*!
    \reimp
*/
void HbRatingSlider::mousePressEvent(QGraphicsSceneMouseEvent *event) 
{
	
    Q_D(HbRatingSlider);
	if(d->mTouchArea->isUnderMouse()) {

		if(d->mReadOnly) {	
			event->ignore();
			return;
		}
		d->mMousePressed = true;
		event->accept();

	}

}
/*!
    \reimp
*/
void HbRatingSlider::mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) 
{
    Q_D(HbRatingSlider);

	if(!d->mMousePressed) {
		return;
	}
	QPointF layoutItemPos = d->mLayoutItem->pos();
    QPointF frameItemPos = d->mFrame->pos();   
	qreal xVal = event->pos().x() - layoutItemPos.x()+ frameItemPos.x();
	if(d->mTouchArea->isUnderMouse()) {
			
		if(d->mReadOnly) {
			event->ignore();
			return;
		}
		
		if(xVal <0) {	
			setCurrentRating(0);
			return;
		}
		
		QRectF rect = d->mTouchArea->boundingRect();
		int rating=0;
		if(rect.contains(xVal,0 )) {
			rating = d->calculateProgressValue(xVal);
			if(toolTip() != QString()) {
				HbToolTip::showText(toolTip(),this);
			}	
			setCurrentRating(rating);
			emit ratingChanged (d->mCurrentValue);
			event->accept();
		}
		

	}
	else {
			setCurrentRating(0);
		}

}
/*!
    \reimp
*/
void HbRatingSlider::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) 
{
    Q_D(HbRatingSlider);
   
	QPointF layoutItemPos = d->mLayoutItem->pos();
    QPointF frameItemPos = d->mFrame->pos();

    qreal xVal = event->pos().x() - layoutItemPos.x()+ frameItemPos.x();
	if(d->mTouchArea->isUnderMouse()) {
			
		if(d->mReadOnly) {
			event->ignore();
			return;
		}
		
		if(xVal <0) {	
			setCurrentRating(0);
			return;
		}
		
		QRectF rect = d->mTouchArea->boundingRect();
		int rating=0;
		if(rect.contains(xVal,0 )) {
			rating = d->calculateProgressValue(xVal);
			if(toolTip() != QString()) {
				HbToolTip::showText(toolTip(),this);
			}	
			setCurrentRating(rating);
			if(d->mCurrentValue) {
			    emit ratingDone (d->mCurrentValue);
			}
			event->accept();
			d->mMousePressed = false;
		}
	
	}		

}
/*!
    \reimp
*/
void HbRatingSlider::setGeometry(const QRectF & rect)
{
	Q_D(HbRatingSlider);
	HbWidget::setGeometry(rect);
	updatePrimitives();
	d->createLookupTable();
}

void HbRatingSlider::initStyleOption(HbStyleOption *hboption) const
{
	Q_D( const HbRatingSlider );
	 HbWidget::initStyleOption(hboption); 
    HbStyleOptionRatingSlider *option = 0;
    if ((option = qstyleoption_cast< HbStyleOptionRatingSlider *>(hboption)) != 0) {
		option->noOfStars = d->mNumberOfIcons;
		option->noOfIntervals = d->mStepCount;
		option->unRatedGraphicsName = d->mUnratedIconName;
		option->ratedGraphicsName = d->mRatedIconName;
		option->progressValue = d->mCurrentValue;
	}
}

/*!
    @beta
    Returns the primitives.
*/
QGraphicsItem* HbRatingSlider::primitive(HbStyle::Primitive primitive) const
{
	Q_D(const HbRatingSlider);
    switch (primitive) {
        case HbStyle::P_RatingSlider_frame:
            return d->mFrame;
        case HbStyle::P_RatingSlider_track:
            return d->mTrack;  
		case HbStyle::P_RatingSlider_layout:
			return d->mLayoutItem;
         default:
            return 0;
    }
}

void HbRatingSlider::changeEvent(QEvent *event)
{
    HbWidget::changeEvent(event);
    switch (event->type()) {
    case QEvent::LayoutDirectionChange:
		updatePrimitives();
        break;
    default:
        break;
    }
}
void HbRatingSlider::updatePrimitives()
{
	Q_D(HbRatingSlider);
	HbStyleOptionRatingSlider option;
    initStyleOption(&option);
	if (d->mFrame) {
            style()->updatePrimitive(d->mFrame, HbStyle::P_RatingSlider_frame, &option);
    }
  
    if (d->mTrack) {
           style()->updatePrimitive(d->mTrack, HbStyle::P_RatingSlider_track, &option);
    }

	if (d->mTouchArea) {
        style()->updatePrimitive(d->mTouchArea, HbStyle::P_CheckBox_toucharea, &option);
    }
	
}

QVariant HbRatingSlider::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == ItemVisibleHasChanged && value.toBool()){
        updatePrimitives();
    }
#ifdef HB_EFFECTS
    if(change == QGraphicsItem::ItemVisibleChange){
        if(value.toBool()) {

            HbEffect::start(this, HB_RATINGSLIDER_ITEM_TYPE, "ratingslider_appear");
        }
		else
		{
			 HbEffect::start(this, HB_RATINGSLIDER_ITEM_TYPE, "ratingslider_disappear");
		}
	}

#endif//HB_EFFECTS

   return HbWidget::itemChange(change,value);
}

