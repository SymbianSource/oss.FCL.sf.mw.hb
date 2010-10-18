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
#include "hbprogresstrackitem_p.h"
#include <hbtooltip.h>
#include <hbwidgetfeedback.h>
#include "hbglobal_p.h"
#include <hbtoucharea.h>
#include <QGraphicsSceneMouseEvent>
#include <QApplication>

#ifdef HB_EFFECTS
#include <hbeffect.h>
#include "hbeffectinternal_p.h"
#define HB_PRGRESSSLIDER_ITEM_TYPE "HB_PROGRESSSLIDER"
#endif

#ifdef HB_GESTURE_FW
#include <hbtapgesture.h>
#endif

HbProgressSliderPrivate::HbProgressSliderPrivate()
{
    mDownState=false;
    handle = 0;
    mSliderValue = 0;
    mHandlePath.clear();
    mToolTipTextVisibleUser = false;
}

HbProgressSliderPrivate::~HbProgressSliderPrivate()
{
    if (handle) {
        delete handle;
    }
}
bool HbProgressSliderPrivate::textVisible() const
{
    return mToolTipTextVisibleUser;
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
        HbEffect::start(mTrack, HB_PRGRESSSLIDER_ITEM_TYPE, "progressbar_progress_complete");
#endif
    }
    else if (value < mMinimum) {
        value = mMinimum;
    }
    
    mProgressValue=value;

    //redraw track
   updateProgressTrack();

    emit q->valueChanged(value);
}

void HbProgressSliderPrivate::setEnableFlag(bool flag)
{
    Q_Q(HbProgressSlider);
    Q_UNUSED(flag);
    q->updatePrimitives();
    if(flag) {
        handle->setHandleNormalState();
    }
}

void HbProgressSliderPrivate::init()
{
    Q_Q(HbProgressSlider);

    mSliderGraphicItem = new HbProgressTrackItem(mFrame);
    HbStyle::setItemName(mSliderGraphicItem, "slider-track");
    qgraphicsitem_cast<HbProgressTrackItem*>(mSliderGraphicItem)->frameDrawer().setFillWholeRect(true);
    mSliderGraphicItem->setZValue(-1);
    // Show the touch area which is created in ProgressBar//

    mTouchAreaItem =  q->style()->createPrimitive(HbStyle::PT_TouchArea, "toucharea",q);
    mTouchAreaItem->setFlag(QGraphicsItem::ItemIsFocusable);
    mTouchAreaItem->setZValue(TOUCHAREA_ZVALUE);

    mFrame->setZValue(mTouchAreaItem->zValue()+1);

#ifdef HB_EFFECTS
    HbEffectInternal::add(HB_PRGRESSSLIDER_ITEM_TYPE,"progressslider_trackpress", "progressslider_trackpress");
    HbEffectInternal::add(HB_PRGRESSSLIDER_ITEM_TYPE,"progressslider_trackrelease", "progressslider_trackrelease");
#endif

#ifdef HB_GESTURE_FW
    mTouchAreaItem->grabGesture(Qt::TapGesture);
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
    if(mDownState){
        mDownState = false;

        if (mFrame) {
            HbStyleFramePrimitiveData data; 
            q->initPrimitiveData(&data, mFrame); 
            q->style()->updatePrimitive(mFrame, &data, q);
        }
        HbWidgetFeedback::triggered(q, Hb::InstantReleased);

#ifdef HB_EFFECTS
        HbEffect::start(q, HB_PRGRESSSLIDER_ITEM_TYPE, "progressslider_trackrelease");
#endif
    }

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

    updateSliderTrack();

    updateProgressTrack();
    
    q->setSliderValue(mSliderValue);
}

/*
    \internal
    Update progress track primitive
*/
void HbProgressSliderPrivate::updateProgressTrack()
{
    Q_Q(HbProgressSlider);

    if (mTrack) {
            HbProgressTrackItem* frameItem = qgraphicsitem_cast<HbProgressTrackItem*>(mTrack);
            if(!frameItem->isVisible()) {
                return;
            }

            frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
            if(!q->isEnabled()){
                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_progslider_loaded_disabled"));
            }
            else
                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_progslider_loaded"));

            frameItem->setMaximum(mMaximum);
            frameItem->setMinimum(mMinimum);
            frameItem->setValue(mProgressValue);
            frameItem->setInverted(mInvertedAppearance);
            frameItem->setOrientation(mOrientation);
            frameItem->update();
        }
}

/*
    \internal
    Update slider track primitive
*/
void HbProgressSliderPrivate::updateSliderTrack()
{
    Q_Q(HbProgressSlider);

    if (mSliderGraphicItem) {
            HbProgressTrackItem* frameItem = qgraphicsitem_cast<HbProgressTrackItem*>(mSliderGraphicItem);
            if(!frameItem->isVisible()) {
                return;
            }

            frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
            if(!q->isEnabled()){
                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_progslider_played_disabled"));
            }
            else
                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_progslider_played"));

            frameItem->setMaximum(mMaximum);
            frameItem->setMinimum(mMinimum);
            frameItem->setValue(mSliderValue);
            frameItem->setInverted(mInvertedAppearance);
            frameItem->setOrientation(mOrientation);
            frameItem->update();
        }
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
    void trackPressed();
    void trackReleased();

    sliderPressed is emitted when the handle is pressed. 
    sliderReleased is emitted when the handle is released.
    sliderMoved is emitted when the handle is moved in any direction.
    trackPressed is emitted when the track is pressed. 
    trackReleased is emitted when the track is released.

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
    d->handle->setZValue(1010);
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

    d->updateSliderTrack();
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
#ifndef HB_GESTURE_FW
    Q_D(HbProgressSlider);
 
    QRectF rect = d->mTouchAreaItem->sceneBoundingRect( );
    // return if point is outside track touch area
    if ( !rect.contains( event->scenePos( ) ) ) {
        event->ignore( );
        return;
    }

    if(flags().testFlag(ItemIsFocusable)) {
        d->mDownState = true;
        if (d->mFrame) {
            HbStyleFramePrimitiveData data; 
            initPrimitiveData(&data, d->mFrame); 

            style()->updatePrimitive(d->mFrame, &data, this);
        }
        
        HbWidgetFeedback::triggered(this, Hb::InstantPressed);
        emit trackPressed();
        d->handle->handleTrackPress(event);
        event->accept();
        #ifdef HB_EFFECTS
            HbEffect::start(this, HB_PRGRESSSLIDER_ITEM_TYPE, "progressslider_trackpress");
        #endif
    } else {
        event->ignore();
    }
#else
    Q_UNUSED(event)
#endif 
}

/*!
    \reimp
 */
void HbProgressSlider::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) 
{
#ifndef HB_GESTURE_FW
    Q_D(HbProgressSlider);

    if(flags().testFlag(ItemIsFocusable)) {
        d->mDownState = false;

        if (d->mFrame) {
            HbStyleFramePrimitiveData data; 
            initPrimitiveData(&data, d->mFrame); 

            style()->updatePrimitive(d->mFrame, &data, this);
        }

        HbWidgetFeedback::triggered(this, Hb::InstantReleased);
        d->handle->handleTrackRelease(event);
        emit trackReleased();
        event->accept();
#ifdef HB_EFFECTS
        HbEffect::start(this, HB_PRGRESSSLIDER_ITEM_TYPE, "progressslider_trackrelease");
#endif
    } else {
        event->ignore();
    }
#else
    Q_UNUSED(event)
#endif 
}

#ifdef HB_GESTURE_FW
void HbProgressSlider::gestureEvent(QGestureEvent *event)
{
    Q_D (HbProgressSlider);

    if(event->gesture(Qt::TapGesture)) {

        HbTapGesture *tapGesture = static_cast<HbTapGesture *>(event->gesture(Qt::TapGesture));
        switch(tapGesture->state()) {
            case Qt::GestureStarted :{           
                    QRectF rect = d->mTouchAreaItem->sceneBoundingRect( );
                    // return if point is outside track touch area
                    if ( !rect.contains( event->mapToGraphicsScene(tapGesture->position( ) ) ) ) {
                        event->ignore( );
                        return;
                    }
                    if(flags().testFlag(ItemIsFocusable)) {
                        #ifdef HB_EFFECTS
                            HbEffect::start(this, HB_PRGRESSSLIDER_ITEM_TYPE, "progressslider_trackpress");
                        #endif
                        d->mDownState = true;
                         if (d->mFrame) {
                            HbStyleFramePrimitiveData data; 
                            initPrimitiveData(&data, d->mFrame); 

                            style()->updatePrimitive(d->mFrame, &data, this);
                        }
                        
                        HbWidgetFeedback::triggered(this, Hb::InstantPressed);
                        emit trackPressed();
                        d->handle->handleTrackPress(event);
                        event->accept();

                    } 
                    else {                    
                        event->ignore();
                    }
            }
            break;
            case Qt::GestureCanceled:
            case Qt::GestureFinished:{       
                if(d->mDownState) {
                    if(flags().testFlag(ItemIsFocusable)) {                    
                            #ifdef HB_EFFECTS
                                HbEffect::start(this, HB_PRGRESSSLIDER_ITEM_TYPE, "progressslider_trackrelease");
                            #endif

                            d->mDownState = false;
                            if (d->mFrame) {
                                HbStyleFramePrimitiveData data; 
                                initPrimitiveData(&data, d->mFrame); 

                                style()->updatePrimitive(d->mFrame, &data, this);
                            }
                            HbWidgetFeedback::triggered(this, Hb::InstantReleased);
                           
                            d->handle->handleTrackRelease(event);
                            emit trackReleased();
                            event->accept();
                            } else {
                            event->ignore();
                      }
                }

            }
            break;
            default:break;
        }
    }

}
#endif 


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
void HbProgressSlider::initPrimitiveData(HbStylePrimitiveData *primitiveData, const QGraphicsObject *primitive)
{
    Q_D(HbProgressSlider);

    QString itemName = HbStyle::itemName(primitive);
    if (itemName == QLatin1String("frame")) {
        HbStyleFramePrimitiveData *data = hbstyleprimitivedata_cast<HbStyleFramePrimitiveData*>(primitiveData);

        data->frameType = HbFrameDrawer::ThreePiecesHorizontal;

        if (!isEnabled() ) {
            data->frameGraphicsName = QLatin1String("qtg_fr_progslider_frame_disabled");
            }
            else {
                if(d->mDownState) {
                    data->frameGraphicsName = QLatin1String("qtg_fr_progslider_frame_pressed");
                }
                else {
                    data->frameGraphicsName = QLatin1String("qtg_fr_progslider_frame_normal");
                }
            }
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

        if (d->mFrame) {
            HbStyleFramePrimitiveData data; 
            initPrimitiveData(&data, d->mFrame); 

            style()->updatePrimitive(d->mFrame, &data, this);
        }

         // update progress value mask
        d->updateProgressTrack();
     
        // update slider value mask
        d->updateSliderTrack();
                
        if(d->mMinTextItem && d->mMinMaxTextVisible) {
            HbStyleTextPrimitiveData data; 
            HbProgressBar::initPrimitiveData(&data, d->mMinTextItem); 

            style()->updatePrimitive(d->mMinTextItem, &data, this);
        }

        if(d->mMaxTextItem && d->mMinMaxTextVisible) {
            HbStyleTextPrimitiveData data; 
            HbProgressBar::initPrimitiveData(&data, d->mMaxTextItem); 

            style()->updatePrimitive(d->mMaxTextItem, &data, this);
        }

        if(d->handle)
            d->handle->setHandlePosForValue(sliderValue());
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
void HbProgressSlider::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
#ifndef HB_GESTURE_FW
    Q_D(HbProgressSlider); 
    QRectF rect = d->mTouchAreaItem->sceneBoundingRect( );
    // return if point is outside track touch area
    if ( !rect.contains( event->scenePos( ) ) ) {
        event->ignore( );
        return;
    }

    if(flags().testFlag(ItemIsFocusable)) {
        d->handle->handleTrackPress(event);
        event->accept();
    } else {
        event->ignore();
    }
#else
    Q_UNUSED(event)
#endif 
}

/*!
    @beta
    Sets the tooltip for the Slider handle. By default it shows the slider value.
    The application can customize the tooltip text using this API. setting NULL string
    will disable the tooltip.

    \param text tooltip text

    \sa sliderToolTip()
*/
void HbProgressSlider::setSliderToolTip(const QString &text)
{
    Q_D(HbProgressSlider);
    d->mTooltipText = text;
    d->mToolTipTextVisibleUser = true;
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

bool HbProgressSlider::sceneEventFilter(QGraphicsItem *obj,QEvent *event)
{
    Q_D(HbProgressSlider);
    bool accepted = false;

    if( obj == d->mTouchAreaItem) 
        if (!isEnabled() ) {
            return false;
        }
        
    if(obj == static_cast<HbTouchArea*>(d->mTouchAreaItem)) {
        if(event->type() == QEvent::Gesture ) {
             gestureEvent(static_cast<QGestureEvent *>( event ));
        }
    }
    return accepted;
}

QGraphicsItem *HbProgressSlider::primitive(const QString &itemName) const
{
    Q_D(const HbProgressSlider);

    if(!itemName.compare(QString("slider-track"))){
        return d->mSliderGraphicItem;
    }
    if(!itemName.compare(QString("toucharea"))){
        return d->mTouchAreaItem;
    }
    if(!itemName.compare(QString("handle-icon")) ||
        !itemName.compare(QString("handle-toucharea"))){
            return d->handle->primitive(itemName);
    }

    return HbProgressBar::primitive(itemName);
}
