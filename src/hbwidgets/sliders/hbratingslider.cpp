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
#include <hbstyleoptionratingslider_p.h>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <hbtoucharea.h>

#ifdef HB_GESTURE_FW
#include <hbtapgesture.h>
#include <hbpangesture.h>
#endif 

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

    q->updatePrimitives();

    #ifdef HB_EFFECTS
    HbEffectInternal::add(HB_RATINGSLIDER_ITEM_TYPE,"ratingslider_appear", "ratingslider_appear");
    HbEffectInternal::add(HB_RATINGSLIDER_ITEM_TYPE,"ratingslider_disappear", "ratingslider_disappear");
    #endif

    #ifdef HB_GESTURE_FW
    q->grabGesture(Qt::TapGesture);
    q->grabGesture(Qt::PanGesture);

    if(QGraphicsObject *touchArea = mTouchArea->toGraphicsObject()) {
        touchArea->grabGesture(Qt::TapGesture);
        touchArea->grabGesture(Qt::PanGesture);
    }
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

    if (pos > mLookupValues[mStepCount-1]) {
        return -1;
    }
    
    return count;
    
}

/*!
    \class HbRatingSlider
    \brief A control for user to do rating.

    This is a general rating widget where user will be able to do different 
    ratings for things like Music ,Video etc. 

    By default there are 5 ratings ( 5 stars ). This can be configured also.
    The interval , number of icons etc can be configured.

    Apart from rating the this can used for showing cumulative rating also.

    example code example:
    \code
    HbRatingSlider *object = new HbRatingSlider(parent);
    \endcode

    The below  code can be used to show some rating e.g. 2.5/5
    by default the  stepcount =5
    
    \code
    HbRatingSlider *slider = new HbRatingSlider();
    slider->setStepCount(100); //5 *20//
    slider->setCurrentRating(50); //2.5*20 it shows 50 / 100 which is same as 2.5/5
    \endcode
    
    This will show as 2.5/5. Now if one the same ratingslider 
    if the Application wants to configure a rating slider with range 1-5
    on emitting the signal rating changed it can set to 
    slider->setStepCount(5);
    slider->setCurrentRating(0)
    
    When the rating is done it emits a signal called ratingDone and when rating is 
    changed by the user by draging the pointer ratingChanged signal is emitted.   
    
 */



/*!
    @beta
    Constructor of  RatingSlider.
    \param parent. Parent widget

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
    Constructor of  RatingSlider.
    \param parent. Parent widget
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
    Sets the read only property. It disables the interaction with widget

    \param value true or false.

    \sa readOnly()
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

    \param number. A value between 1 and 10 

    \sa numberOfIcons()
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

    \sa setNumberOfIcons()
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
    
    \param count. A value between 1 and 100. This can be considerd as the maximum rating possible. 

    \sa numberOfIcons()

*/
void HbRatingSlider::setStepCount(int count)
{
    Q_D(HbRatingSlider);
    if( (count <= 0) || (count > 100) ) {
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
    It sets the current rating value.
    \param count. A value between 1 and stepcount. 
    \sa currentRating()

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
    the grpahics can be a single star kind of or multi star image. If it is single star then use setNumberOfIcons for 
    setting number of stars.
    
    \param name. The graphics name along with the path. 
    \sa unRatedIconName()
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
    
    It sets the rated graphics name.This is the graphics shown when rating is on going.
    the grpahics can be a single star kind of or multi star image. If it is single star then use setNumberOfIcons for 
    setting number of stars.
    
    \param name. The graphics name along with the path. 
    \sa unRatedIconName()
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

#ifndef HB_GESTURE_FW
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
#else
void HbRatingSlider::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
}
#endif

#ifdef HB_GESTURE_FW
void HbRatingSlider::gestureEvent(QGestureEvent *event)
{
    Q_D (HbRatingSlider);
    if(event->gesture(Qt::TapGesture)) {
            HbTapGesture *tap = qobject_cast<HbTapGesture *>(event->gesture(Qt::TapGesture));
            switch(tap->state()) {
            case Qt::GestureStarted:
                {
                qreal xVal = mapFromScene(event->mapToGraphicsScene(tap->position( ))).x();
                int rating = d->calculateProgressValue(xVal);
                if(rating == -1) {
                    return;
                }
                if(d->mReadOnly) {    
                    event->ignore();
                    return;
                }

                d->mMousePressed = true;
                event->accept();
                }
                break;
 
            case Qt::GestureFinished: // Reset state 
                {
            qreal xVal = mapFromScene(event->mapToGraphicsScene(tap->position( ))).x();
            QRectF rect = d->mTouchArea->boundingRect();
            int rating=0;
            if(rect.contains(xVal,0 )) {
                if(d->mReadOnly) {
                    event->ignore();
                    return;
                }

                if(!d->mMousePressed){
                    return;
                }

                if(xVal <0) {    
                    setCurrentRating(0);
                    emit ratingDone (d->mCurrentValue);
                    return;
                }

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
            break;
            default: break;
            } 
    }else if(event->gesture(Qt::PanGesture)) {
                HbPanGesture *pan = qobject_cast<HbPanGesture *>(event->gesture(Qt::PanGesture));
                switch(pan->state()) {
                    case Qt::GestureUpdated:
                        {
                        if(!d->mMousePressed) {
                            return;
                        }
                        qreal xVal = mapFromScene(event->mapToGraphicsScene( pan->startPos()+pan->offset())).x();
                            QRectF rect = d->mTouchArea->boundingRect();
                            int rating=0;
                            if(rect.contains(xVal,0 )) {
                                if(d->mReadOnly) {
                                event->ignore();
                                return;
                            }
                            
                            if(xVal <0) {    
                                setCurrentRating(0);
                                return;
                            }

                                rating = d->calculateProgressValue(xVal);
                                
                                if(toolTip() != QString()) {
                                    HbToolTip::showText(toolTip(),this);
                                }    
                                setCurrentRating(rating);
                                emit ratingChanged (d->mCurrentValue);
                                event->accept();
                            }
                            else {
                                setCurrentRating(0);
                            }
                        }
                        break;
                    case Qt::GestureFinished: // Reset state 
                    {                          
                         qreal xVal = mapFromScene(event->mapToGraphicsScene( pan->startPos()+pan->offset())).x();
                         QRectF rect = d->mTouchArea->boundingRect();
                         int rating=0;
                         if(rect.contains(xVal,0 )) {
                            if(d->mReadOnly) {
                               event->ignore();
                               return;
                             }
                        }

                        if(!d->mMousePressed) {
                             return;
                        }

                       if(xVal <0) {    
                          setCurrentRating(0);
                          emit ratingDone (d->mCurrentValue);
                          return;
                        }

                        rating = d->calculateProgressValue(xVal);
                        setCurrentRating(rating);
                        if(d->mCurrentValue) {
                           emit ratingDone (d->mCurrentValue);
                        }
                        d->mMousePressed = false;
                        event->accept();
                     }
                     default:
                     break;
                }
    }
}
#endif 

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

    \deprecated HbRatingSlider::primitive(HbStyle::Primitive)
        is deprecated.

    Provides access to primitives of HbRatingSlider. 
    \param primitive is the type of the requested primitive. The available 
    primitives are P_RatingSlider_frame,P_RatingSlider_track and P_RatingSlider_layout.

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

