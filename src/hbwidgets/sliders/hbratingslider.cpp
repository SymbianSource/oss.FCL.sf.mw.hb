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

#include "hbratingslider_p.h"
#include "hbrepeatitem_p.h"
#include <hbratingslider.h>
#include <hbtooltip.h>
#include <hbstyleoptionratingslider_p.h>
#include <hbtoucharea.h>
#include <hbwidgetfeedback.h>

#ifdef HB_GESTURE_FW
#include <hbtapgesture.h>
#include <hbpangesture.h>
#endif 

#ifdef HB_EFFECTS
#include <hbeffect.h>
#include "hbeffectinternal_p.h"
#define HB_RATINGSLIDER_ITEM_TYPE "HB_RATINGSLIDER"
#endif

#include <QGraphicsSceneMouseEvent>

#define MAX_NUMBER_OF_ICONS 10

HbRatingSliderPrivate::HbRatingSliderPrivate():
    mMousePressed(false),
    mLookupValues(0),
    mReadOnly(false),
    mNumberOfIcons(5),
    mStepCount(5),
    mCurrentValue(0),
    mUnRatedItem(0),
    mRatedItem(0),
    mBackGroundItem(0),
    mTouchArea(0),
    mToolTipArea(0)
{    
}

HbRatingSliderPrivate::~HbRatingSliderPrivate()
{
    if(mLookupValues) {
        delete [] mLookupValues;
    }
}

/*
    Initializes widget primitives
*/
void HbRatingSliderPrivate::init()
{
    Q_Q(HbRatingSlider);

    mBackGroundItem = new HbWidgetBase(q);
    HbStyle::setItemName(mBackGroundItem, "background");

    mUnRatedItem = new HbRepeatItem(mBackGroundItem);
    HbStyle::setItemName(mUnRatedItem, "unrated-item");

    mRatedItem = new HbRepeatMaskItem(mUnRatedItem);
    HbStyle::setItemName(mRatedItem, "rated-item");

    mTouchArea =  q->style()->createPrimitive(HbStyle::PT_TouchArea, "toucharea",q);
    mTouchArea->setFlag(QGraphicsItem::ItemIsFocusable);
    q->setHandlesChildEvents(true);

    //q->updatePrimitives();

    #ifdef HB_EFFECTS
    HbEffectInternal::add(HB_RATINGSLIDER_ITEM_TYPE,"ratingslider_appear", "ratingslider_appear");
    HbEffectInternal::add(HB_RATINGSLIDER_ITEM_TYPE,"ratingslider_disappear", "ratingslider_disappear");
    #endif

    #ifdef HB_GESTURE_FW
    q->grabGesture(Qt::TapGesture);
    q->grabGesture(Qt::PanGesture);

    mTouchArea->grabGesture(Qt::TapGesture);
    mTouchArea->grabGesture(Qt::PanGesture);
    #endif 

}

/*
    Creates a lookup table which stores the bandwidth calculation for each star.

    Decision will be made based on this LookupTable values for touch 
    point/position identification.
*/
void HbRatingSliderPrivate::createLookupTable()
{
    if(mLookupValues) {
        delete [] mLookupValues;
        mLookupValues=0;
    }
    
    mLookupValues = new int[mStepCount];
    qreal width = mUnRatedItem->boundingRect().width();
    int bandWidth =(int) (width/mStepCount);
        
    for(int i=0;i < mStepCount;i++) {
        mLookupValues[i] = bandWidth*(i+1);
    }
}

/*
    Returns the exact count for the touch position based
    on the LookupTable array values.
*/
int HbRatingSliderPrivate::calculateProgressValue(qreal pos)
{
    Q_Q(HbRatingSlider);
    
    if (pos > mLookupValues[mStepCount-1]) {
        return -1;
    }

    int count=0;
    for(;count< mStepCount ;count++) {
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

/*
    updates rated icon item
*/
void HbRatingSliderPrivate::updateRatedIconItem()
{
    Q_Q(HbRatingSlider);

    if (mRatedItem) {
        HbRepeatMaskItem *repeatItem = static_cast<HbRepeatMaskItem*>(mRatedItem);
        repeatItem->setMaskValue(mCurrentValue);
        repeatItem->setMaximum(mStepCount);
        repeatItem->setInverted((q->layoutDirection() == Qt::RightToLeft));
        repeatItem->setRepeatingNumber(mNumberOfIcons);
        if (!mRatedIconName.isEmpty()) {
            repeatItem->setName(mRatedIconName);
        }
        else {
            if(!q->isEnabled()) {
                repeatItem->setName(QLatin1String("qtg_graf_ratingslider_rated_disabled"));
            }
            else {
                 if(mMousePressed) {
                     repeatItem->setName(QLatin1String("qtg_graf_ratingslider_rated_pressed"));
                 }
                 else {
                     repeatItem->setName(QLatin1String("qtg_graf_ratingslider_rated"));
                 }
            }
        }
        repeatItem->setGeometry(q->boundingRect());
        repeatItem->update();
    }
}

/*
    updates unrated icon item
*/
void HbRatingSliderPrivate::updateUnRatedIconItem()
{
    Q_Q(HbRatingSlider);

    if (mUnRatedItem) {
        HbRepeatItem *repeatItem = static_cast<HbRepeatItem*>(mUnRatedItem);
        repeatItem->setRepeatingNumber(mNumberOfIcons);
        if (!mUnratedIconName.isEmpty()) {
            repeatItem->setName(mUnratedIconName);
        }
        else {
            if(!q->isEnabled()) {
                 repeatItem->setName(QLatin1String("qtg_graf_ratingslider_unrated_disabled"));
            }
            else {
                if(mMousePressed) {
                    repeatItem->setName(QLatin1String("qtg_graf_ratingslider_unrated_pressed"));
                 }
                 else {
                     repeatItem->setName(QLatin1String("qtg_graf_ratingslider_unrated"));
                 }
            }
        }
        repeatItem->setGeometry(q->boundingRect());
        repeatItem->update();
    }
}

/*!
    \class HbRatingSlider
    \brief This is a widget that enables a user to rate contents like videos , music etc.
    \image html ratingslider.png  "A Rating Slider with rating done"

    The default version of HbRatingSlider contains 5 repeated icons drawn side by side, using a single themed graphics.
    The application can replace the themed graphic with a custom graphic.
    The custom graphics should contain only one icon (eg one star)  which will be multipled by the API \a setNumberOfIcons().
    By default it is 5 and maximum number of icons are 10.

    Along with the rating HbRatingSlider can be used to show the cumulative rating also.

    To use HbRatingSlider with default settings it just needs to be created.
    example code:
    \code
    HbRatingSlider *object = new HbRatingSlider(parent);
    \endcode

    HbRatingSlider emits below signals 

    void ratingDone(int ratingValue);
    void ratingChanged(int ratingValue);
    
    ratingDone is emitted when the user does the rating and releases the finger. 
    ratingChanged is emitted when the user presses and drags the finger on Rating Slider.

    To use HbRatingSlider with default settings it just needs to be created. 
    example code: 
    \code 
    HbMainWindow window;
    HbRatingSlider *rs = new HbRatingSlider();
    window.addView(rs);
    \endcode 

    HbRatingSlider supports integer ratings.But using the API \a setStepCount() fraction ratings can also be 
    shown on Rating Slider

    The below  code can be used to show some rating e.g. 2.5/5       
    \code
    //2.5/5 can be set as  25/50
    HbRatingSlider *slider = new HbRatingSlider();
    slider->setStepCount(50); //5 *10//
    slider->setCurrentRating(25); //2.5*10 it shows 25/50 which is same as 2.5/5
    \endcode
    
    This will show as 2.5/5. Now if on the same HbRatingSlider 
    the Application wants to configure a Rating Slider with range 1-5
    on emitting the signal rating changed it can set to 5.
 */


/*!
    @beta
     Constructs a Rating Slider bar with the given parent.
    \param parent Parent Item.

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
    Protected constructor
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
    Sets the number of icons. There can be n number of repeated icons. This method can be used to set 
    the number of icons required.The default image is "*" and has 5 stars.

    \param number. A value between 1 and 10 

    \sa numberOfIcons()
*/
void HbRatingSlider::setNumberOfIcons(int number)
{
    Q_D(HbRatingSlider);

    if(number != d->mNumberOfIcons){
        if ( (number <= 0) || (number > MAX_NUMBER_OF_ICONS) ){
            return;
        }
        d->mNumberOfIcons = number;
        updatePrimitives();

        d->createLookupTable();
    }
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
    Sets the step count for the Rating Slider.This indicates the interval of the rating. Eg. If step count is 10
    then 10 rating is possible.
    
    \param count. A value between 1 and 100. This can be considerd as the maximum rating possible. 

    \sa numberOfIcons()

*/
void HbRatingSlider::setStepCount(int count)
{
    Q_D(HbRatingSlider);

    if(count != d->mStepCount){
        if( (count <= 0) || (count > 100) ) {
            return;
        }
        d->mStepCount = count;
        d->createLookupTable();    

        d->updateRatedIconItem();
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

    if( (rating == d->mCurrentValue) || (rating < 0) ) {
        return;
    }

    if( rating > d->mStepCount ) {
        rating = d->mStepCount;
    }

    d->mCurrentValue = rating;

    d->updateRatedIconItem();
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
    
    It sets the unrated graphics name.This is the graphics shown when Rating Slider is displayed.
    the graphicscan be a single star kind of or multi star image. If it is single star then use setNumberOfIcons for 
    setting number of stars.
    
    \param name. The graphics name along with the path. 
    \sa unRatedIconName()
*/
void HbRatingSlider::setUnRatedIconName(const QString name)
{
    Q_D(HbRatingSlider);

    if(name != d->mUnratedIconName) {
        d->mUnratedIconName =name;
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
    the graphicscan be a single star kind of or multi star image. If it is single star then use setNumberOfIcons for 
    setting number of stars.
    
    \param name. The graphics name along with the path. 
    \sa unRatedIconName()
*/
void HbRatingSlider::setRatedIconName(const QString name)
{
    Q_D(HbRatingSlider);

    if(name != d->mRatedIconName) {
        d->mRatedIconName = name;
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
        updatePrimitives();

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
    QPointF layoutItemPos = d->mBackGroundItem->pos();
    QPointF frameItemPos = d->mUnRatedItem->pos();   
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
            if(!mToolTipText.isNull()) {
                HbToolTip::showText(mToolTipText,this);
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
   
    QPointF layoutItemPos = d->mBackGroundItem->pos();
    QPointF frameItemPos = d->mUnRatedItem->pos();

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
            if(!mToolTipText.isNull()) {
                HbToolTip::showText(mToolTipText,this);
            }    
            setCurrentRating(rating);
            if(d->mCurrentValue) {
                emit ratingDone (d->mCurrentValue);
            }
            event->accept();
            d->mMousePressed = false;
        }
        updatePrimitives();
    
    }        
}
#else
/*!
    \reimp
 */
void HbRatingSlider::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
}
#endif


#ifdef HB_GESTURE_FW
/*!
    \reimp
 */
void HbRatingSlider::gestureEvent(QGestureEvent *event)
{
    Q_D (HbRatingSlider);

    if ( (!isEnabled()) || (d->mReadOnly)) {
        event->ignore();
        return;
    }

    if(HbTapGesture *tap = qobject_cast<HbTapGesture *>(event->gesture(Qt::TapGesture))){
        switch(tap->state()) {
            case Qt::GestureStarted:
            {
                qreal xVal = mapFromScene(event->mapToGraphicsScene(tap->position( ))).x();
                int rating = d->calculateProgressValue(xVal);
                if(rating == -1) {
                    event->ignore();
                    return;
                }
                  
                QRectF rect = d->mTouchArea->boundingRect();
                if(rect.contains(xVal,0 )) {
                    HbWidgetFeedback::triggered(this, Hb::InstantPressed);
                    d->mMousePressed = true;
                    updatePrimitives();
                    rating = d->calculateProgressValue(xVal);
                    setCurrentRating(rating);
                    emit ratingChanged (d->mCurrentValue);
                    if(!d->mToolTipArea)
                        d->mToolTipArea = new HbTouchArea(d->mUnRatedItem);
                    d->mToolTipArea->setPos(xVal,0);
                    d->mToolTipArea->setSize(QSize(1,1));
                    if(!d->mToolTipText.isNull()) {
                        HbToolTip::showText(d->mToolTipText,d->mToolTipArea);
                    }
                    event->accept();
                }
                else {
                    event->ignore();
                }                
            }
            break;
 
            case Qt::GestureFinished: // Reset state 
            {
                qreal xVal = mapFromScene(event->mapToGraphicsScene(tap->position( ))).x();
                QRectF rect = d->mTouchArea->boundingRect();
                int rating=0;
                if(rect.contains(xVal,0 )) {
                    if(!d->mMousePressed){
                        event->ignore();
                        return;
                    }
                    if(xVal <0) {    
                        setCurrentRating(0);
                        emit ratingDone (d->mCurrentValue);
                        return;
                    }
                    rating = d->calculateProgressValue(xVal);
                    setCurrentRating(rating);
                    HbWidgetFeedback::triggered(this, Hb::InstantReleased);
                    if(d->mCurrentValue) {
                        emit ratingDone (d->mCurrentValue);
                    }
                    event->accept();
                    d->mMousePressed = false;
                    updatePrimitives();
                }            
                else {
                        d->mMousePressed = false;
                        updatePrimitives();
                        if(xVal <rect.x() )  {
                            setCurrentRating(0);
                            emit ratingDone (d->mCurrentValue);
                        }
            
                    }        
            }
            break;
            default: break;
            } 
    }
    else if(HbPanGesture *pan = qobject_cast<HbPanGesture *>(event->gesture(Qt::PanGesture))){
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
                    if(xVal <0) {    
                        setCurrentRating(0);
                        return;
                    }

                    rating = d->calculateProgressValue(xVal);
                    if(!d->mToolTipArea)
                        d->mToolTipArea = new HbTouchArea(d->mUnRatedItem); //Need to show the tooltip at the touch point
                    if(rating!=-1) {
                        d->mToolTipArea->setPos(xVal,0);
                        d->mToolTipArea->setSize(QSize(1,1));
                        if(!d->mToolTipText.isNull()) {
                            HbToolTip::showText(d->mToolTipText,d->mToolTipArea);
                        }
                    }
                    setCurrentRating(rating);
                    HbWidgetFeedback::continuousTriggered(this, Hb::ContinuousDragged);
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
                d->mMousePressed = false;
                updatePrimitives();
                int rating=0;
                if(rect.contains(xVal,0 )) {             
                 if(xVal <0) {    
                     setCurrentRating(0);
                     emit ratingDone (d->mCurrentValue);
                     return;
                 }
                 rating = d->calculateProgressValue(xVal);
                 setCurrentRating(rating);
                 HbWidgetFeedback::triggered(this, Hb::InstantReleased);
                 if(d->mCurrentValue) {
                    emit ratingDone (d->mCurrentValue);
                 }                       
                 event->accept();
                }
                        
            }
            break;
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

/*!
    @beta
    Sets the tooltip for the rating slider.
    
    By default it shows nothing.

    The application can customize the tooltip text using this API.
    Setting NULL string will disable the tooltip.

    \param tooltip tooltip text

    \sa toolTipText()
*/
void HbRatingSlider::setToolTipText(const QString tooltip)
{
    Q_D(HbRatingSlider);

    d->mToolTipText = tooltip;
}

/*!
    @beta

    Returns the current tooltip text value. 

    \sa setToolTipText()
*/
QString HbRatingSlider::toolTipText() const
{
    Q_D(const HbRatingSlider);

    return d->mToolTipText;
}

/*!
    \reimp
*/
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
        option->disableState = !isEnabled();
        option->pressedState = d->mMousePressed;
        if(layoutDirection() == Qt::RightToLeft) {
            option->inverted = true;
        }
        else {
            option->inverted = false;
        }
    }
}

/*!
    \reimp
 */
void HbRatingSlider::changeEvent(QEvent *event)
{
    HbWidget::changeEvent(event);

    switch (event->type()) {
    case QEvent::LayoutDirectionChange:
        {
         updatePrimitives();
        }
        break;
    case QEvent::EnabledChange:
         updatePrimitives();
         break;
    default:
        break;
    }
}

/*!
    \reimp
 */
void HbRatingSlider::updatePrimitives()
{
    Q_D(HbRatingSlider);

    // update unrated icon item
    d->updateUnRatedIconItem();

    // update rated icon item
    d->updateRatedIconItem();
}

/*!
    \reimp
 */
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
        else {
            HbEffect::start(this, HB_RATINGSLIDER_ITEM_TYPE, "ratingslider_disappear");
        }
    }
#endif//HB_EFFECTS

   return HbWidget::itemChange(change,value);
}

/*!
    \reimp
 */
QGraphicsItem *HbRatingSlider::primitive(const QString &itemName) const
{
    Q_D(const HbRatingSlider);

    if(!itemName.compare(QString("background"))){
        return d->mBackGroundItem;
    }
    if(!itemName.compare(QString("unrated-item"))){
        return d->mUnRatedItem;
    }
    if(!itemName.compare(QString("rated-item"))){
        return d->mRatedItem;
    }
    if(!itemName.compare(QString("toucharea"))){
        return d->mTouchArea;
    }

    return HbWidget::primitive(itemName);
}

