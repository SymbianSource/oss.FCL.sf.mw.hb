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

#include "hbprogressslider_p.h"
#include <hbprogressslider.h>
#include <hbstyleoptionprogressslider_p.h>
#include <hbtooltip.h>
#include <hbwidgetfeedback.h>
#include "hbglobal_p.h"

#include <QGraphicsSceneMouseEvent>
#include <QApplication>

#ifdef HB_EFFECTS
#include <hbeffect.h>
#include "hbeffectinternal_p.h"
#define HB_PRGRESSSLIDER_ITEM_TYPE "HB_PROGRESSSLIDER"
#endif

HbProgressSliderPrivate::HbProgressSliderPrivate()
{
    mDownState=false;
    handle = 0;
    mSliderValue = 0;
    mHandlePath.clear();
}

HbProgressSliderPrivate::~HbProgressSliderPrivate()
{
    if (handle) {
        delete handle;
    }
}

void HbProgressSliderPrivate::setProgressValue(int value)
{
   Q_Q(HbProgressSlider);
   if (mProgressValue == value) {
        return;
    }
    if (value >= mMaximum) {
        value = mMaximum;
#ifdef HB_EFFECTS
       // HbEffect::start(mTrack, HB_PRGRESSSLIDER_ITEM_TYPE, "progressbar_progress_complete");
#endif
    }
    else if (value < mMinimum) {
        value = mMinimum;
    }
    
    mProgressValue=value;

    //redraw track
    HbStyleOptionProgressSlider sliderOption;
    q->initStyleOption(&sliderOption);
    if(mTrack) {
        q->style()->updatePrimitive(mTrack, HbStyle::P_ProgressSlider_track, &sliderOption);
    }

    emit q->valueChanged(value);
}

void HbProgressSliderPrivate::setEnableFlag(bool flag)
{
    Q_Q(HbProgressSlider);
	Q_UNUSED(flag);
    HbStyleOptionProgressSlider option;
    q->initStyleOption(&option);
    q->updatePrimitives();
}

void HbProgressSliderPrivate::init()
{
    Q_Q(HbProgressSlider);

    mSliderGraphicItem  = q->style()->createPrimitive(HbStyle::P_ProgressSlider_slidertrack,mFrame);
    // Show the touch area which is created in ProgressBar//
    mTouchAreaItem = q->style()->createPrimitive(HbStyle::P_ProgressSlider_toucharea,q);
    mFrame->setZValue(mTouchAreaItem->zValue()+1);

#ifdef HB_EFFECTS
    HbEffectInternal::add(HB_PRGRESSSLIDER_ITEM_TYPE,"progressslider_trackpress", "progressslider_trackpress");
    HbEffectInternal::add(HB_PRGRESSSLIDER_ITEM_TYPE,"progressslider_trackrelease", "progressslider_trackrelease");
#endif

    q->grabGesture(Qt::TapGesture);

    if(QGraphicsObject *touchArea = mTouchAreaItem->toGraphicsObject()) {
        touchArea->grabGesture(Qt::TapGesture);
    }
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

void HbProgressSliderPrivate::setRange(int minimum, int maximum)
{
	Q_Q(HbProgressSlider);
    if( minimum > maximum ){
        maximum = minimum ;
    }
    mMinimum = minimum;
    mMaximum = maximum;

    if ( mProgressValue < mMinimum){
        mProgressValue = mMinimum;
    }

    if(mProgressValue > mMaximum){
        mProgressValue = mMaximum;
    }

    HbStyleOptionProgressSlider progressSliderOption;
    q->initStyleOption(&progressSliderOption);

    if (mSliderGraphicItem) {
            q->style()->updatePrimitive(mSliderGraphicItem, HbStyle::P_ProgressSlider_slidertrack, &progressSliderOption);
    }

    if (mTrack) {
            q->style()->updatePrimitive(mTrack, HbStyle::P_ProgressSlider_track, &progressSliderOption);
     }
    
	q->setSliderValue(mSliderValue);
}

/*!
    @beta
    @hbwidgets
    \class HbProgressSlider
    \brief ProgressSlider is used to indicate the current position of a playing music or video. It can show the progress 
    of a progressing music or video along with the status of the buffered data.

    \image html progressslider.png  "A Progress Slider with Min-Max text at bottom "
    \image html progressslider2.png  "A Progress Slider with Min-Max text at bottom and with pregress value"


    ProgressSlider has a track like Progress Bar indicating how much progress
    has been done. It is a  Progress Bar with some additional behaviour. 
    There is also a progressValue which indicates the amount of buffered data.General use
    for this widget would be for playing music online which indicates sliderValue as currentTime and 
    progressValue as the buffered amount.

    HbProgressSlider is derived from HbProgressBar so it supports all the features supported by HbProgressBar.
    But It supports only horizontal orientation.

    HbProgressSlider emits below signals 

    void sliderPressed();
    void sliderReleased();
    void sliderMoved(int value);

    sliderPressed is emitted when the track is pressed. 
    sliderReleased is emitted when the track is released.
    sliderMoved is emitted when the handle is moved in any direction.

    The Application can customize the Slider behaviour by listening to the signals sliderPressed and sliderReleased.By default there 
    is no behaviour defined by HbProgressSlider for these actions.

    By default the min value is 0 and max value is 100. The application can set the progressValue (buffer data) and 
    sliderValue (Progress Slider position) according to the situation.

    Example code for creating and using Progress Slider:
    \code
    HbProgressSlider *mySlider = new HbProgressSlider(parent);
    connect(mySlider,SIGNAL(sliderMoved(int)), mySlider ,SLOT(setSliderValue(int)));
    //This sets the buffered data progress
    mySlider->setProgressValue(45);
    \endcode


    Example code for creating and using Progress Slider along with Min-Max text:
    \code
    HbProgressSlider *mySlider = new HbProgressSlider(parent);
    connect(mySlider,SIGNAL(sliderMoved(int)), mySlider ,SLOT(setSliderValue(int)));
    //This sets the buffered data progress
    mySlider->setProgressValue(45);
    mySlider->setMinText("0");
    mySlider->setMaxText("100");
    //This sets the slider position
    mySlider->setSliderValue(20);
    \endcode
    
*/

/*!
    @beta
    Constructs a Progress Slider with a  parent.
*/

HbProgressSlider::HbProgressSlider(QGraphicsItem *parent) :
    HbProgressBar(*new HbProgressSliderPrivate,parent)
{
    Q_D( HbProgressSlider );
    d->q_ptr = this;
    d->init();
    setMinMaxTextVisible(true);

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
    Constructs a Progress Bar with the given parent.
    \param parent The parent of ProgressBar
*/
HbProgressSlider::HbProgressSlider(HbProgressSliderPrivate &dd,QGraphicsItem *parent) : 
    HbProgressBar( dd,parent)
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
    Destructor for the Progress Slider.
*/
HbProgressSlider::~HbProgressSlider()
{
}

/*!
    @beta
    Sets the current value of the progress slider.

    The progress slider forces the value to be within the legal range: \b
    minimum <= \c value <= \b maximum.

    \param value slider value.

    \sa sliderValue()
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

    HbStyleOptionProgressSlider option;
    initStyleOption(&option);

    if (d->mSliderGraphicItem) {
        style()->updatePrimitive(d->mSliderGraphicItem, HbStyle::P_ProgressSlider_slidertrack, &option);
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
    Returns true of false depending on whether the slider is pressed down or not.
*/

bool HbProgressSlider::isSliderDown() const
{
    Q_D( const HbProgressSlider );
    return d->handle->isHandlePressed();
}


/*!
    @beta
    Sets the inverted appearance of the slider.
    If inverted the slider increases from right to left.

    \param inverted true or false

    \sa invertedAppearance()
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
    \reimp
 */
void HbProgressSlider::mousePressEvent(QGraphicsSceneMouseEvent *event) 
{
    Q_D(HbProgressSlider);
 
    QRectF rect = d->mTouchAreaItem->sceneBoundingRect( );
    // return if point is outside track touch area
    if ( !rect.contains( event->scenePos( ) ) ) {
        event->ignore( );
        return;
    }

    if(flags().testFlag(ItemIsFocusable)) {
        d->mDownState = true;
        HbStyleOptionProgressSlider option;
        initStyleOption(&option);
        if (d->mFrame) {
            style()->updatePrimitive(d->mFrame, HbStyle::P_ProgressSlider_frame, &option);          
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

/*!
    \reimp
 */
void HbProgressSlider::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) 
{
    Q_D(HbProgressSlider);

    if(flags().testFlag(ItemIsFocusable)) {
        d->mDownState = false;

        HbStyleOptionProgressSlider option;
        initStyleOption(&option);
        if (d->mFrame) {
            style()->updatePrimitive(d->mFrame, HbStyle::P_ProgressSlider_frame, &option);          
        }

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
/*!
    \reimp
 */
void HbProgressSlider::setGeometry(const QRectF & rect)
{
    Q_D(HbProgressSlider);
    HbProgressBar::setGeometry(rect);
    d->handle->setHandlePosForValue(sliderValue());
    //updatePrimitives();
}

/*!
    \reimp
 */
void HbProgressSlider::initStyleOption( HbStyleOptionProgressSlider *option ) const
{
    Q_D(const HbProgressSlider);
    HbProgressBar::initStyleOption(option);

    option->secondaryType = true;
    option->progressSliderValue = d->mSliderValue;  
    option->pressedState = d->mDownState;
    if(isEnabled()) {
        option->disableState = false;
    }
    else {
        option->disableState = true;
    }
}
/*!
    \reimp
 */
void HbProgressSlider::updatePrimitives()
{
    Q_D(HbProgressSlider);
    if(isVisible()){
        d->mWaitTrack->setVisible(false);
        d->mTrack->setVisible(true);

        HbStyleOptionProgressSlider option;
        initStyleOption(&option);

        if (d->mSliderGraphicItem) {
            style()->updatePrimitive(d->mSliderGraphicItem, HbStyle::P_ProgressSlider_slidertrack, &option);
        }

        if(d->handle)
              d->handle->setHandlePosForValue(sliderValue());
    
        if (d->mFrame) {
            style()->updatePrimitive(d->mFrame, HbStyle::P_ProgressSlider_frame, &option);          
        }
     
        if (d->mTrack) {
                style()->updatePrimitive(d->mTrack, HbStyle::P_ProgressSlider_track, &option);
        }
                
        if(d->mMinTextItem && d->mMinMaxTextVisible) {
            style()->updatePrimitive(d->mMinTextItem,HbStyle::P_ProgressBar_mintext,&option);    
        }

        if(d->mMaxTextItem && d->mMinMaxTextVisible) {
            style()->updatePrimitive(d->mMaxTextItem,HbStyle::P_ProgressBar_maxtext,&option);    
        }
    }
}
/*!
    \reimp
 */
void HbProgressSlider::showEvent( QShowEvent * event )
{
    Q_D(const HbProgressSlider);
    if(d->mTouchAreaItem && scene()) {
        d->mTouchAreaItem->removeSceneEventFilter(this);
        d->mTouchAreaItem->installSceneEventFilter(this);
    }

    HbProgressBar::showEvent(event);
}
/*!
    \reimp
 */
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
/*!
    \reimp
 */
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
    @beta
    Sets the tooltip for the Slider handle. By default it shows the slider value.
    The application can customize the tooltip text using this API.

    \param text tooltip text

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
    Sets the icon for handle. By default it has theme based icon. Application 
    can configure this icon via this API

    \param handlePath path of the graphics

    \sa handleIcon()
*/
void HbProgressSlider::setHandleIcon(const QString& handlePath )
{
    
    Q_D( HbProgressSlider );
    if(handlePath != d->mHandlePath) {
        d->mHandlePath =handlePath;
        d->handle->setHandleIcon(HbIcon(handlePath));
    }
}

/*!
    @beta

    Returns the icon  handle path

    \sa setHandleIcon()
*/
QString HbProgressSlider::handleIcon() const
{
    Q_D(const HbProgressSlider);
    return d->mHandlePath;
}

/*!

    \deprecated HbProgressSlider::primitive(HbStyle::Primitive)
        is deprecated.

    Returns the pointer for \a primitive passed.
    Will return NULL if \a primitive passed is invalid
*/
QGraphicsItem* HbProgressSlider::primitive(HbStyle::Primitive primitive) const
{
    Q_D(const HbProgressSlider);

    switch (primitive) {
        case HbStyle::P_ProgressSlider_frame:
            return d->mFrame;
        case HbStyle::P_ProgressSlider_track:
            return d->mTrack;
        case HbStyle::P_ProgressSlider_slidertrack:
            return d->mSliderGraphicItem;
        case HbStyle::P_ProgressSlider_toucharea:
            return d->mTouchAreaItem; 
        case HbStyle::P_ProgressSliderHandle_icon:
        case HbStyle::P_ProgressSliderHandle_toucharea:
            return d->handle->primitive(primitive);
        default:
            return 0;
    }
}

