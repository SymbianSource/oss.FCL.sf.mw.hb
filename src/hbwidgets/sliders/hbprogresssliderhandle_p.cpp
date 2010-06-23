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
#include <hbtooltip.h>
#include <hbstyleoptionprogresssliderhandle_p.h>
#include <hbextendedlocale.h>
#include <QGraphicsSceneMouseEvent>

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

/*!
    \reimp
    \fn int HbProgressSliderHandle::type() const
 */

HbProgressSliderHandle::HbProgressSliderHandle(HbHandleParent *parent) 
    :HbWidget(parent->parentGraphicsItem()),
    q(parent),
    mHandleIcon()
{
    mFlags = 0;
    mFlags |= TextVisible;

    HbStyle *style = static_cast<HbStyle*>(q->style());

    mHandleIconItem = style->createPrimitive(HbStyle::P_ProgressSliderHandle_icon,this);
    HbStyle::setItemName(mHandleIconItem , "icon");

    mTouchItem = style->createPrimitive(HbStyle::P_ProgressSliderHandle_toucharea, this); 
    HbStyle::setItemName(mTouchItem , "toucharea");
    setProperty("state","normal");

#ifdef HB_EFFECTS
    HbEffectInternal::add(HB_PRGRESSSLIDERHANDLE_ITEM_TYPE,"progressslider_handlepress", "progressslider_handlepress");
    HbEffectInternal::add(HB_PRGRESSSLIDERHANDLE_ITEM_TYPE,"progressslider_handlerelease", "progressslider_handlerelease");
    HbEffectInternal::add(HB_PRGRESSSLIDERHANDLE_ITEM_TYPE,"progressslider_handleoutofbound", "progressslider_handleoutofbound");
#endif

#ifdef HB_GESTURE_FW
    grabGesture(Qt::TapGesture);
    grabGesture(Qt::PanGesture);

    if(QGraphicsObject *touchArea = mTouchItem->toGraphicsObject()) {
        touchArea->grabGesture(Qt::TapGesture);
        touchArea->grabGesture(Qt::PanGesture);
    }
#endif 
}

HbProgressSliderHandle::~HbProgressSliderHandle() 
{
}

void HbProgressSliderHandle::setHandleIcon(const HbIcon& icon)
{
    if(icon.isNull()) {
        return;
    }
    HbStyle *style = qobject_cast<HbStyle*>(q->style());
    mHandleIcon= icon;

    HbStyleOptionProgressSliderHandle option;
    initStyleOption(&option);

    style->updatePrimitive(mHandleIconItem, HbStyle::P_ProgressSliderHandle_icon, &option);
}


void HbProgressSliderHandle::gestureEvent(QGestureEvent *event)
{
    Q_UNUSED(event);
    // HbWidgetBase::gestureEvent() ignores, overriding to accept
}

void HbProgressSliderHandle::mousePressEvent(QGraphicsSceneMouseEvent *event) 
{
    HbWidget::mousePressEvent(event);

#ifdef HB_EFFECTS
    HbEffect::start(this, HB_PRGRESSSLIDERHANDLE_ITEM_TYPE, "progressslider_handlepress");
#endif
    mFlags |= HbProgressSliderHandle::MousePressed;

    HbStyleOptionProgressSliderHandle option;
    initStyleOption(&option);
    option.pressedState = true;
    if (mHandleIconItem) {
        style()->updatePrimitive(mHandleIconItem, HbStyle::P_ProgressSliderHandle_icon, &option);
    }

    mMousePressPos = event->scenePos();
    mItemPosAtPress = pos();
   
    HbWidgetFeedback::triggered(q->parentGraphicsWidget(), Hb::InstantPressed, Hb::ModifierSliderHandle);

    event->accept();
    q->emitSliderPressed();   

     if(q->textVisible()) {
        HbToolTip::showText(q->toolTipText(),this, QRectF(mItemPosAtPress,QSize(0,0)),q->textAlignment());
    }
    else {
        HbExtendedLocale locale;
        HbProgressSlider *slider = (HbProgressSlider*)q->parentGraphicsWidget();
        HbToolTip::showText(locale.toString(slider->sliderValue()),this, QRectF(mItemCurPos,QSize(0,0)),q->textAlignment());
    }

}

void HbProgressSliderHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) 
{
    HbWidget::mouseReleaseEvent(event);

    HbStyleOptionProgressSliderHandle option;
    initStyleOption(&option);
    option.pressedState = false;
    if (mHandleIconItem) {
        style()->updatePrimitive(mHandleIconItem, HbStyle::P_ProgressSliderHandle_icon, &option);
    }

    if (isHandleMoving()) {
      HbWidgetFeedback::continuousStopped(q->parentGraphicsWidget(), Hb::ContinuousDragged);
    }
    HbWidgetFeedback::triggered(q->parentGraphicsWidget(), Hb::InstantReleased, Hb::ModifierSliderHandle);

#ifdef HB_EFFECTS
    HbEffect::start(this, HB_PRGRESSSLIDERHANDLE_ITEM_TYPE, "progressslider_handlerelease");
#endif
    mFlags &= ~HbProgressSliderHandle::MousePressed;
    mFlags &=~HandleMoving;
    event->accept();
    setHandlePosForValue(q->progressValue());   
    q->emitSliderReleased();
}

void HbProgressSliderHandle::handleTrackRelease(QGraphicsSceneMouseEvent * event)
{
    Q_UNUSED(event);
    setHandlePosForValue(q->progressValue());   
}

void HbProgressSliderHandle::handleTrackPress(QGraphicsSceneMouseEvent * event)
{
    QPointF newPos = q->parentGraphicsItem()->mapFromScene(event->scenePos());
    if((newPos.x() >=  q->boundingRect().x()) && (newPos.x() <=  q->boundingRect().width())) {   
        if(q->orientation() == Qt::Horizontal){
            mItemCurPos = QPointF(newPos.x() - boundingRect().width()/2, pos().y());
        }
        else{
            mItemCurPos = QPointF(pos().x(), newPos.y()-boundingRect().height()/2);
        }

        mItemCurPos = normalizedPos(mItemCurPos,false);
        setPos(mItemCurPos);
        processItemChange(mItemCurPos);
    }
}

void HbProgressSliderHandle::mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) 
{
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
            HbEffect::start(this, HB_PRGRESSSLIDERHANDLE_ITEM_TYPE, "progressslider_handleoutofbound");
        #endif
    }

    event->accept();

    q->emitSliderMoved(pointToValue(mItemCurPos));  
    
    if(q->textVisible()) {
        HbToolTip::showText(q->toolTipText(),this, QRectF(mItemCurPos,QSize(0,0)),q->textAlignment());
    }
    else {
        HbExtendedLocale locale;
        HbProgressSlider *slider = (HbProgressSlider*)q->parentGraphicsWidget();
        HbToolTip::showText(locale.toString(slider->sliderValue()),this, QRectF(mItemCurPos,QSize(0,0)),q->textAlignment());
    }
}

int HbProgressSliderHandle::pointToValue(QPointF point) const
{
    QRectF rect = q->boundingRect();
    qreal effectiveWidth;
    point = normalizedPos(point,q->invertedAppearance());
    qreal givenPixel;
    if(q->orientation() == Qt::Horizontal){
        effectiveWidth = rect.width() - boundingRect().width();
        givenPixel = point.x();
    }
    else{
        effectiveWidth = rect.height() - boundingRect().height();
        givenPixel = effectiveWidth-point.y();
    }

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
    if(q->orientation()== Qt::Horizontal){
        width = r1.width() - r2.width();
        
    }
    else{
        width = r1.height() - r2.height();
       
    }
    if(q->maximum() != q->minimum()){
        qreal pixelpertick = width/(qreal)(q->maximum()-q->minimum());
        qreal noOfTicks = qreal(value - q->minimum());
        qreal xpos =  noOfTicks * pixelpertick;
        if(q->orientation() == Qt::Horizontal){
            return QPointF(xpos, r1.top());
        }
        else{
            return QPointF(r1.left(),width-xpos);
        }
    }
    else{
        return QPointF(0,0);
    }
}

QPointF HbProgressSliderHandle::normalizedPos(const QPointF&  pos,bool inverted) const 
{
    Q_UNUSED(inverted);
    QPointF newPos = pos;
    if(q->orientation() == Qt::Horizontal){
        if (newPos.x() < HandleMargin) {
            newPos.setX( HandleMargin );
        }

        if (newPos.x() > q->boundingRect().width() - boundingRect().width() - HandleMargin) {
            newPos.setX(q->boundingRect().width() - boundingRect().width() - HandleMargin);
        }
    }
    else{
        if (newPos.y() < HandleMargin) {
            newPos.setY( HandleMargin );
        }

        if (newPos.y() > q->boundingRect().height() - boundingRect().height() - HandleMargin) {
            newPos.setY(q->boundingRect().height() - boundingRect().height() - HandleMargin);
        }
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

QVariant HbProgressSliderHandle::processItemChange(const QVariant &value)
{
        // value is the new position
        QPointF pt = value.toPointF();
        int newValue = pointToValue(pt);
        if(q->orientation() == Qt::Horizontal){
            pt.setY(q->boundingRect().top());
        }
        else{
            pt.setX(q->boundingRect().left());
        }
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
            if(q->orientation() == Qt::Horizontal){
                qreal xVal = q->boundingRect().width() - pos.x() - boundingRect().width();
                pos.setX(xVal);
            }
            else{
                qreal yVal = q->boundingRect().height() - pos.y() - boundingRect().height();
                pos.setY(yVal);
            }
           
        }

        qreal yPos = qreal (q->boundingRect().height()-boundingRect().height()) /2 ;
        setPos(pos.x(),yPos);
    }   
   
}


void  HbProgressSliderHandle::updatePrimitives()
{    
    HbStyleOptionProgressSliderHandle option;
    initStyleOption(&option);

    if (mHandleIconItem) {
        style()->updatePrimitive(mHandleIconItem, HbStyle::P_ProgressSliderHandle_icon, &option);
    }

}

/*!
    Returns the pointer for \a primitive passed.
    Will return NULL if \a primitive passed is invalid
*/
QGraphicsItem* HbProgressSliderHandle::primitive(HbStyle::Primitive primitive) const
{
    switch (primitive) {
        case HbStyle::P_ProgressSliderHandle_icon:
            return mHandleIconItem;
        case HbStyle::P_ProgressSliderHandle_toucharea:
            return mTouchItem;
        default:
            return 0;
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

