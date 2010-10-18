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

#include "hbprogresssliderhandle_p.h"
#include "hbstyle_p.h"
#include <hbtooltip.h>
#include <hbstyleoptionprogresssliderhandle_p.h>
#include <hbstyleprimitivedata.h>
#include <hbstyleiconprimitivedata.h>
#include <hbiconitem.h>
#include <hbextendedlocale.h>
#include <QGraphicsSceneMouseEvent>
#include <hbmessagebox.h>
//#define ANIMATION_ON_TRACK_PRESS
#define HBPROGRESSSLIDERHANDLE_TRACES
#ifdef HBPROGRESSSLIDERHANDLE_TRACES
#include <QtDebug>
#endif

#include <hbwidgetfeedback.h>

#ifdef HB_EFFECTS
#include "hbeffect.h"
#include "hbeffectinternal_p.h"
#define HB_PRGRESSSLIDERHANDLE_ITEM_TYPE "HB_PROGRESSSLIDERHANDLE"
#endif

#ifdef HB_GESTURE_FW
#include <hbtapgesture.h>
#include <hbpangesture.h>
#endif

#define   HandleMargin 0
#define   EffectInterval 100
/*!
    \reimp
    \fn int HbProgressSliderHandle::type() const
 */

HbProgressSliderHandle::HbProgressSliderHandle(HbHandleParent *parent) 
    :HbWidget(parent->parentGraphicsItem()),
    q(parent),
    mHandleIcon(),
    mPressedState(false),
    mTimeline(0)
{
    mFlags = 0;
    mFlags |= TextVisible;
    mOutOfBound = false;

    mHandleIconItem = q->parentGraphicsWidget()->style()->createPrimitive(HbStyle::PT_IconItem, "handle-icon",this); 
    qgraphicsitem_cast<HbIconItem*>(mHandleIconItem)->setIconName("qtg_graf_progslider_handle_normal");
    qgraphicsitem_cast<HbIconItem*>(mHandleIconItem)->setAspectRatioMode(Qt::IgnoreAspectRatio);
    
    mTouchItem = q->parentGraphicsWidget()->style()->createPrimitive(HbStyle::PT_TouchArea, "handle-toucharea",this);
    mTouchItem->setFlag(QGraphicsItem::ItemIsFocusable);
    mTouchItem->setZValue(TOUCHAREA_ZVALUE);

    setHandlesChildEvents(true);
    setProperty("state","normal");

    mTimeline = new QTimeLine(250,this);
    mTimeline->setFrameRange(0,EffectInterval);
    connect(mTimeline,SIGNAL(frameChanged(int)), this, SLOT(frameChanged(int)));

#ifdef HB_EFFECTS
    HbEffectInternal::add(HB_PRGRESSSLIDERHANDLE_ITEM_TYPE,"progressslider_handlepress", "progressslider_handlepress");
    HbEffectInternal::add(HB_PRGRESSSLIDERHANDLE_ITEM_TYPE,"progressslider_handlerelease", "progressslider_handlerelease");
    HbEffectInternal::add(HB_PRGRESSSLIDERHANDLE_ITEM_TYPE,"progressslider_handleoutofbound", "progressslider_handleoutofbound");
#endif

#ifdef HB_GESTURE_FW
    grabGesture(Qt::TapGesture);
    grabGesture(Qt::PanGesture);

    mTouchItem->grabGesture(Qt::TapGesture);
    mTouchItem->grabGesture(Qt::PanGesture);

#endif 
}

HbProgressSliderHandle::~HbProgressSliderHandle() 
{
}

void HbProgressSliderHandle::setHandleIcon(const HbIcon& icon)
{
    mHandleIcon= icon;

   updatePrimitives();
}



void HbProgressSliderHandle::mousePressEvent(QGraphicsSceneMouseEvent *event) 
{
#ifndef HB_GESTURE_FW
    HbWidget::mousePressEvent(event);

#ifdef HB_EFFECTS
    HbEffect::start(mHandleIconItem, HB_PRGRESSSLIDERHANDLE_ITEM_TYPE, "progressslider_handlepress");
#endif
    mFlags |= HbProgressSliderHandle::MousePressed;

    mPressedState = true;

    updatePrimitives();

    mMousePressPos = event->scenePos();
    mItemPosAtPress = pos();
   
    HbWidgetFeedback::triggered(q->parentGraphicsWidget(), Hb::InstantPressed, Hb::ModifierSliderHandle);

    event->accept();
    q->emitSliderPressed();   

     if(q->textVisible()) {  // User called it 
         if(!q->toolTipText().isNull()) {
             HbToolTip::showText(q->toolTipText(),this, QRectF(mItemPosAtPress,QSize(0,0)),q->textAlignment());            
         }        
      }
      else {  // show default

         HbExtendedLocale locale;
         HbProgressSlider *slider = (HbProgressSlider*)q->parentGraphicsWidget();
         HbToolTip::showText(locale.toString(slider->sliderValue()),this, QRectF(mItemCurPos,QSize(0,0)),q->textAlignment());

     }
#else
    Q_UNUSED(event)
#endif 
}

void HbProgressSliderHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) 
{
#ifndef HB_GESTURE_FW
    HbWidget::mouseReleaseEvent(event);

    mPressedState = false;

    updatePrimitives();

    if (isHandleMoving()) {
      HbWidgetFeedback::continuousStopped(q->parentGraphicsWidget(), Hb::ContinuousDragged);
    }
    HbWidgetFeedback::triggered(q->parentGraphicsWidget(), Hb::InstantReleased, Hb::ModifierSliderHandle);

#ifdef HB_EFFECTS
    HbEffect::start(mHandleIconItem, HB_PRGRESSSLIDERHANDLE_ITEM_TYPE, "progressslider_handlerelease");
#endif
    mFlags &= ~HbProgressSliderHandle::MousePressed;
    mFlags &=~HandleMoving;
    event->accept();
    setHandlePosForValue(q->progressValue());   
    q->emitSliderReleased();
#else
    Q_UNUSED(event)
#endif 
}

void HbProgressSliderHandle::mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) 
{
#ifndef HB_GESTURE_FW
    HbWidget::mouseMoveEvent(event);
    mFlags |=HandleMoving;
    QPointF scenePos = event->scenePos();
    if(q->orientation() == Qt::Horizontal){
             mItemCurPos = QPointF((event->scenePos().x() - mMousePressPos.x()) + mItemPosAtPress.x(), pos().y());
             HbWidgetFeedback::continuousTriggered(qobject_cast<HbWidget*>(q->parentGraphicsWidget()), Hb::ContinuousDragged);
             if(mItemCurPos.x()+boundingRect().width() < q->boundingRect().width()
                 && mItemCurPos.x()>HandleMargin){
                setPos(mItemCurPos);
             }
             else{
                processItemChange(mItemCurPos);
             }
        }

     if (!mTouchItem->boundingRect().contains(parentItem()->mapFromScene(scenePos))) {
        #ifdef HB_EFFECTS
            HbEffect::start(mHandleIconItem, HB_PRGRESSSLIDERHANDLE_ITEM_TYPE, "progressslider_handleoutofbound");
        #endif
    }

    event->accept();

    q->emitSliderMoved(pointToValue(mItemCurPos));  
    
     if(q->textVisible()) {  // User called it 
         if(q->toolTipText().isNull()) {

             // Dont show any tooltip.
             
         }
         else { 

             HbToolTip::showText(q->toolTipText(),this, QRectF(mItemPosAtPress,QSize(0,0)),q->textAlignment());
            
         }        
      }
      else {  // show default

         HbExtendedLocale locale;
         HbProgressSlider *slider = (HbProgressSlider*)q->parentGraphicsWidget();
         HbToolTip::showText(locale.toString(slider->sliderValue()),this, QRectF(mItemCurPos,QSize(0,0)),q->textAlignment());

     }
#else
    Q_UNUSED(event)
#endif 
}

void HbProgressSliderHandle::handleTrackRelease(QGestureEvent *event)
{
    Q_UNUSED(event);
    setHandlePosForValue(q->progressValue());   
}

void HbProgressSliderHandle::frameChanged(int val)
{
    QPointF myPoint(mStartPoint.x()+ (val*mIncrValue),mStartPoint.y());
    setPos(myPoint);
    processItemChange(myPoint);
}

void HbProgressSliderHandle::handleTrackPress(QGestureEvent *event)
{
    HbTapGesture *gesture = static_cast<HbTapGesture *>(event->gesture(Qt::TapGesture));
    QPointF newPos = q->parentGraphicsItem()->mapFromScene(event->mapToGraphicsScene(gesture->position()));

    if((newPos.x() >=  q->boundingRect().x()) && (newPos.x() <=  q->boundingRect().width())) {   
        if(q->orientation() == Qt::Horizontal){
            mItemCurPos = QPointF(newPos.x() - boundingRect().width()/2, pos().y());
        }
        else{
            mItemCurPos = QPointF(pos().x(), newPos.y()-boundingRect().height()/2);
        }

#ifdef ANIMATION_ON_TRACK_PRESS    
        QPointF currentPt = pos();
        mItemCurPos = normalizedPos(mItemCurPos,false);
        qreal diff  = mItemCurPos.x() - currentPt.x();
        mStartPoint = currentPt;
        mIncrValue = (qreal)diff/EffectInterval;
        if(mTimeline->state() == QTimeLine::Running) {
            mTimeline->stop();
        }
        mTimeline->start();
#else

        setPos(mItemCurPos);
        processItemChange(mItemCurPos);
#endif

    }
}


/*!
  reimp

*/
void HbProgressSliderHandle::gestureEvent(QGestureEvent *event)
{ 
    if(HbTapGesture *tap = qobject_cast<HbTapGesture *>(event->gesture(Qt::TapGesture))) {
        switch(tap->state()) {
            case Qt::GestureStarted: {
                mMousePressPos = mapFromScene(event->mapToGraphicsScene(tap->position( )));                
                if(q->textVisible()) {  // User called it 
                    HbToolTip::showText(q->toolTipText(),this, QRectF(mMousePressPos,QSize(0,0)),q->textAlignment());            
                }
                else {  // show default
                    HbExtendedLocale locale;
                    HbProgressSlider *slider = (HbProgressSlider*)q->parentGraphicsWidget();
                    HbToolTip::showText(locale.toString(slider->sliderValue()),this, QRectF(mMousePressPos,QSize(0,0)),q->textAlignment());
                }
                #ifdef HB_EFFECTS
                    HbEffect::start(mHandleIconItem, HB_PRGRESSSLIDERHANDLE_ITEM_TYPE, "progressslider_handlepress");
                #endif
                mFlags |= HbProgressSliderHandle::MousePressed;
                mPressedState = true;
                updatePrimitives();

                mMousePressPos = mapFromScene(event->mapToGraphicsScene(tap->position( )));
                HbWidgetFeedback::triggered(q->parentGraphicsWidget(), Hb::InstantPressed, Hb::ModifierSliderHandle);
                event->accept();

                q->emitSliderPressed();  
            }
            break;
            case Qt::GestureFinished:{
                
                if (isHandleMoving()) {
                    HbWidgetFeedback::continuousStopped(q->parentGraphicsWidget(), Hb::ContinuousDragged);
                }
                HbWidgetFeedback::triggered(q->parentGraphicsWidget(), Hb::InstantReleased, Hb::ModifierSliderHandle);
                #ifdef HB_EFFECTS
                HbEffect::start(mHandleIconItem, HB_PRGRESSSLIDERHANDLE_ITEM_TYPE, "progressslider_handlerelease");
                #endif
                mPressedState = false;
                updatePrimitives();
                mFlags &= ~HbProgressSliderHandle::MousePressed;
                mFlags &=~HandleMoving;
                event->accept();
                setHandlePosForValue(q->progressValue());   
                q->emitSliderReleased();
            }
            break;
            default:
                break;                                  
        }
    }
    if (HbPanGesture *panGesture = qobject_cast<HbPanGesture*>(event->gesture(Qt::PanGesture))) {
        switch(panGesture->state( )) {
            case Qt::GestureStarted: 
                {
                    if(!mPressedState) {
                        #ifdef HB_EFFECTS
                            HbEffect::start(mHandleIconItem, HB_PRGRESSSLIDERHANDLE_ITEM_TYPE, "progressslider_handlepress");
                        #endif
                        mPressedState = true;
                        updatePrimitives();
                        q->emitSliderPressed();
                    }
                }
            case Qt::GestureUpdated:{
                    mFlags |=HandleMoving;
                    QPointF scenePos =mapToParent( mapFromScene(panGesture->sceneOffset( )+panGesture->sceneStartPos( )));
                    if(scenePos.x() == oldCord.x()) {
                        return;
                    }
                    oldCord = scenePos;
                    if(mOutOfBound){
                        HbEffect::start(mHandleIconItem, HB_PRGRESSSLIDERHANDLE_ITEM_TYPE, "progressslider_handlepress");
                        mOutOfBound = false;
                    }
                
                    mItemCurPos = QPointF((scenePos.x() - mMousePressPos.x()), pos().y());
                    HbWidgetFeedback::continuousTriggered(qobject_cast<HbWidget*>(q->parentGraphicsWidget()), Hb::ContinuousDragged);
                    if(mItemCurPos.x()+boundingRect().width() < q->boundingRect().width() && mItemCurPos.x()>q->boundingRect().topLeft().x() ){
                        setPos(mItemCurPos);
                    }
                    else{
                        #ifdef HB_EFFECTS
                        HbEffect::start(mHandleIconItem, HB_PRGRESSSLIDERHANDLE_ITEM_TYPE, "progressslider_handleoutofbound");
                        mOutOfBound = true;
                        #endif
                    }

                    if(q->textVisible()) {  // User called it 
                        HbToolTip::showText(q->toolTipText(),this, QRectF(mItemCurPos,QSize(0,0)),q->textAlignment());                      
                    }
                    else {  // show default
                        HbExtendedLocale locale;
                        HbProgressSlider *slider = (HbProgressSlider*)q->parentGraphicsWidget();
                        HbToolTip::showText(locale.toString(slider->sliderValue()),this, QRectF(mItemCurPos,QSize(0,0)),q->textAlignment());
                    } 
                    q->emitSliderMoved(pointToValue(mItemCurPos));

                    event->accept();
                }
            break;
            case Qt::GestureFinished:
            case Qt::GestureCanceled: {
                if(mPressedState) {
                    mPressedState = false;           
                    updatePrimitives();

                    if (isHandleMoving()) {
                        HbWidgetFeedback::continuousStopped(q->parentGraphicsWidget(), Hb::ContinuousDragged);
                    }
                    else {
                        HbWidgetFeedback::triggered(q->parentGraphicsWidget(), Hb::InstantReleased, Hb::ModifierSliderHandle);
                    }
                    #ifdef HB_EFFECTS
                    HbEffect::start(mHandleIconItem, HB_PRGRESSSLIDERHANDLE_ITEM_TYPE, "progressslider_handlerelease");
                    #endif
                    mFlags &= ~HbProgressSliderHandle::MousePressed;
                    mFlags &=~HandleMoving;
                    event->accept();
                    setHandlePosForValue(q->progressValue());   
                    q->emitSliderReleased();
                }
            }
            break;
            default:
                break;
        }
    }

}

int HbProgressSliderHandle::pointToValue(QPointF point) const
{
    QRectF rect = q->boundingRect();
    qreal effectiveWidth;
    point = normalizedPos(point,q->invertedAppearance());
    qreal givenPixel;
 
    effectiveWidth = rect.width() - boundingRect().width();
    givenPixel = point.x();
    
    qreal tickPerPixel = (qreal)(q->maximum()-q->minimum())/effectiveWidth;
    qreal tickForGivenPixel = givenPixel * tickPerPixel;
    tickForGivenPixel = qRound(tickForGivenPixel);

    int value;
    if(!q->invertedAppearance()) {
        value = q->minimum() + (int)tickForGivenPixel;
    } else {
        value = q->maximum() - (int)tickForGivenPixel;
    }

    if(value>q->maximum()) {
        return q->maximum();
    }
    else if(value<q->minimum()) {
        return q->minimum();
    }
    return value;
}

// converts value to handle coordinates
QPointF HbProgressSliderHandle::valueToHandlePos(int value) const
{
    QRectF r1 = q->boundingRect();
    QRectF r2 = boundingRect();

    qreal width;
    width = r1.width() - r2.width();
    if(q->maximum() != q->minimum()){
        qreal pixelpertick = width/(qreal)(q->maximum()-q->minimum());
        qreal noOfTicks = qreal(value - q->minimum());
        qreal xpos =  noOfTicks * pixelpertick;
        return QPointF(xpos, r1.top());
    }
    else{
        return QPointF(0,0);
    }
}

QPointF HbProgressSliderHandle::normalizedPos(const QPointF&  pos,bool inverted) const 
{
    Q_UNUSED(inverted);
    
    QPointF newPos = pos;
    
    if (newPos.x() < HandleMargin) {
            newPos.setX( HandleMargin );
    }

    if (newPos.x() > q->boundingRect().width() - boundingRect().width() - HandleMargin) {
            newPos.setX(q->boundingRect().width() - boundingRect().width() - HandleMargin);
    }
    
    return newPos;
}

bool HbProgressSliderHandle::isHandlePressed() const
{
    return mFlags.testFlag(HbProgressSliderHandle::MousePressed);
}


bool HbProgressSliderHandle::isHandleMoving() const
{
    return mFlags.testFlag(HbProgressSliderHandle::HandleMoving);
}
/*!
    \reimp
 */
void HbProgressSliderHandle::setGeometry(const QRectF & rect)
{
    HbWidget::setGeometry(rect);
    QPointF point = pos();
    qreal x = q->boundingRect().height();
    qreal y = size().height();
    qreal a = qreal (x-y)/2;
    setPos(point.x(),a);
}

QVariant HbProgressSliderHandle::processItemChange(const QVariant &value)
{
    // value is the new position
    QPointF pt = value.toPointF();
    int newValue = pointToValue(pt);
    pt.setY(q->boundingRect().top());
    q->emitSliderMoved(newValue);
    QPointF newPos = pt;
    return normalizedPos(newPos,false);
}

void HbProgressSliderHandle::setHandlePosForValue(int progressValue)
{
    if(!mFlags.testFlag(HbProgressSliderHandle::MousePressed)){

        QPointF newPos = valueToHandlePos(progressValue);
        QPointF pos = normalizedPos(newPos,q->invertedAppearance());
        if(q->invertedAppearance()) {
                qreal xVal = q->boundingRect().width() - pos.x() - boundingRect().width();
                pos.setX(xVal);
        }
        qreal yPos = qreal (q->boundingRect().height()-boundingRect().height()) /2 ;
        setPos(pos.x(),yPos);
    }    
}


void  HbProgressSliderHandle::updatePrimitives()
{    
    if (mHandleIconItem) {
        HbStyleIconPrimitiveData data;
        initPrimitiveData(&data, mHandleIconItem);
        style()->updatePrimitive(mHandleIconItem,&data,this);
    }
}

/*!
    Initializes \a option with the values from this HbProgressSliderHandle. 
    This method is useful for subclasses when they need a HbStyleOptionProgressSliderHandle,
    but don't want to fill in all the information themselves.
 */
void HbProgressSliderHandle::initStyleOption(HbStyleOptionProgressSliderHandle *option) const
{
    HbWidget::initStyleOption(option);
    option->handleIcon = mHandleIcon;
    option->pressedState = false;
}

/*!
    \reimp
*/
void HbProgressSliderHandle::initPrimitiveData(HbStylePrimitiveData *primitiveData, const QGraphicsObject *primitive)
{
    HbWidgetBase::initPrimitiveData(primitiveData, primitive);
    QString itemName = HbStyle::itemName(primitive);
    if (itemName == "handle-icon") {
        HbStyleIconPrimitiveData *data = hbstyleprimitivedata_cast<HbStyleIconPrimitiveData*>(primitiveData);
        HbProgressSlider *slider = (HbProgressSlider*)q->parentGraphicsWidget();
        data->icon = slider->handleIcon();

        if(data->icon.value().isNull()) {
            if(mPressedState)
                data->icon = HbIcon(QLatin1String("qtg_graf_progslider_handle_pressed"));
            else
                data->icon = HbIcon(QLatin1String("qtg_graf_progslider_handle_normal"));
        }
        else
        {
            data->icon = slider->handleIcon();
        }        
    }
}

void HbProgressSliderHandle::setHandleNormalState()
{
    if(mPressedState) {
        mPressedState=false;
        #ifdef HB_EFFECTS
             HbEffect::start(mHandleIconItem, HB_PRGRESSSLIDERHANDLE_ITEM_TYPE, "progressslider_handlerelease");
        #endif
        updatePrimitives();
    }
}
/*!
    reimp
*/
QGraphicsItem *HbProgressSliderHandle::primitive(const QString &itemName) const
{
    if(!itemName.compare(QString("handle-icon"))){
        return mHandleIconItem;
    }
    if(!itemName.compare(QString("handle-toucharea"))){
        return mTouchItem;
    }

    return HbWidget::primitive(itemName);
}
