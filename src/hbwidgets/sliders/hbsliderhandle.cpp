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

#include "hbsliderhandle_p.h"
#include "hbslidercontrol_p.h"
#include "hbslidercontrol_p_p.h"
#include <hbwidgetfeedback.h>
#include <hbstyle.h>
#include <hbstyleoptionslider.h>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>

#ifdef HB_EFFECTS
#include "hbeffect.h"
#include "hbeffectinternal_p.h"
#define HB_SLIDERHANDLE_TYPE "HB_SLIDERHANDLE"
#endif

/*!
    This is internal class for HbSlider
    this created slider handle
    It also has event handling for handle press/move and release

*/


/*!
    Constructs a slider handle for \a slider.
*/
HbSliderHandle::HbSliderHandle(HbSliderControl *slider)
    : HbWidget(slider),
    sliderControl(slider), 
    gItem(0) ,
    handleDragPos(0),
    touchItem(0),
    mHandleItem(0)
{
    // create handle icon item
    gItem = slider->style()->createPrimitive(HbStyle::P_Slider_thumb, this); 
    HbStyle::setItemName(gItem , "icon");

    // create touch area for handle
    touchItem = slider->style()->createPrimitive(HbStyle::P_SliderElement_touchhandle, this); 
    HbStyle::setItemName(touchItem , "toucharea");

    setZValue(slider->zValue() + 1);
    

#ifdef HB_EFFECTS
    // horizontal thumb press
   // HbEffectInternal::add(HB_SLIDERHANDLE_TYPE,"sliderhandle_h_press", "h_thumbpress");
    //horizonal thumb release
    //HbEffectInternal::add(HB_SLIDERHANDLE_TYPE,"sliderhandle_h_releasel", "h_thumbrelease");
    // vertical thumb press
    //HbEffectInternal::add(HB_SLIDERHANDLE_TYPE,"sliderhandle_v_press", "v_thumbpress");
    //vertical thumb release
    //HbEffectInternal::add(HB_SLIDERHANDLE_TYPE,"sliderhandle_v_release", "v_thumbrelease");
    // horizontal out of bound
    //HbEffectInternal::add(HB_SLIDERHANDLE_TYPE,"sliderhandle_h_outofbound", "h_outofbound");
    //vertical out of bound
    //HbEffectInternal::add(HB_SLIDERHANDLE_TYPE,"sliderhandle_v_outofbound", "v_outofbound");
#endif
}

/*!
    Destroys the slider handle.
*/
HbSliderHandle::~HbSliderHandle()
{
	if(mHandleItem) {
		mHandleItem->removeSceneEventFilter(this);
	}
}


/*!
  reimp

*/
/*void HbSliderHandle::polish( HbStyleParameters& params )
{
    updatePrimitives();
    HbWidget::polish(params);
}*/

/*!
  This Api set handle position and update handle icon to press ot normal
*/
void HbSliderHandle::forceSetPos(const QPointF &pos)
{
    setPos(pos);
    HbStyleOptionSlider opt;
    sliderControl->initStyleOption(&opt);
    if( scene()){
        if (true == sliderControl->isSliderDown()){
            opt.state |= QStyle::State_Sunken;
            sliderControl->style()->updatePrimitive(
                sliderControl->primitive(HbStyle::P_Slider_groove), HbStyle::P_Slider_groove, &opt);
          } else {
            opt.state &= ~QStyle::State_Sunken;
        } 
    } else {
        opt.state &= ~QStyle::State_Sunken;
    }

    opt.boundingRect=boundingRect();
    sliderControl->style()->updatePrimitive(gItem, HbStyle::P_Slider_thumb, &opt); 
}

void HbSliderHandle::setHandleItem(QGraphicsItem *item)
{
	if(!item) {
		return;
	}
	if(mHandleItem) {
		delete mHandleItem;
        mHandleItem =0;
	}

    setHandlesChildEvents(false);

    mHandleItem = item;  
	mHandleItem->setParentItem(this);
	sliderControl->style()->setItemName(mHandleItem,"widget");
    //((QGraphicsWidget*)mHandleItem)->setGeometry(gItem->geometry());//TODO:geometry from css

    if(scene()) {
        mHandleItem->removeSceneEventFilter(this);
        mHandleItem->installSceneEventFilter(this);
    }
	mHandleItem->setFlag(QGraphicsItem::ItemIsMovable,true);//required as the press has to be accepted to filter move events.

}

QGraphicsItem *HbSliderHandle::handleItem() const
{
    return mHandleItem;
}

void HbSliderHandle::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    if(scene()) {
        if(gItem) {
            //gItem->removeSceneEventFilter(this);
            //gItem->installSceneEventFilter(this);
        }
        if(mHandleItem && scene()) {
            mHandleItem->removeSceneEventFilter(this);
            mHandleItem->installSceneEventFilter(this);
        }
           
    }
}
bool HbSliderHandle::sceneEventFilter(QGraphicsItem *obj,QEvent *event)
{
    //TODO: touch area doesnt work with the current filtering mechanism. find better solution
	if( obj == mHandleItem) {
		if(event->type() == QEvent::GraphicsSceneMouseMove){
			mouseMoveEvent ( (QGraphicsSceneMouseEvent *) event ) ;
			return true;
		}
		else if (event->type() == QEvent::GraphicsSceneMousePress){
			mousePressEvent((QGraphicsSceneMouseEvent *) event);
		}
		else if (event->type() == QEvent::GraphicsSceneMouseRelease){
			mouseReleaseEvent((QGraphicsSceneMouseEvent *) event);
		}
	} 
	return false;
}
 
/*!
  reimp

*/
void HbSliderHandle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    qreal span = 0;
    HbWidget::mousePressEvent(event);
    QRectF sliderBounds=sliderControl->boundingRect();
    QRectF handleBounds=boundingRect();
    sliderControl->setSliderDown(true);
    HbWidgetFeedback::triggered(sliderControl, Hb::InstantPressed, Hb::ModifierSliderHandle);
    if (scene()){
        HbStyleOptionSlider opt;
        sliderControl->initStyleOption(&opt);
        if (sliderControl->orientation() == Qt::Horizontal) {
#ifdef HB_EFFECTS
            HbEffect::start(gItem, HB_SLIDERHANDLE_TYPE, "h_thumbpress");
#endif
            sliderBounds.adjust(0, 0, -handleBounds.width(), 0);
            span = sliderBounds.width();
            handleInitialPos = static_cast<int>(mapToParent(event->pos()).rx());
        } else {
#ifdef HB_EFFECTS
            HbEffect::start(gItem, HB_SLIDERHANDLE_TYPE, "v_thumbpress");
#endif
            sliderBounds.adjust(0, 0, 0, -handleBounds.height());
            span = sliderBounds.height();
            handleInitialPos = static_cast<int>(mapToParent(event->pos()).ry());
        }
        handleDragPos= QStyle::sliderPositionFromValue(opt.minimum, opt.maximum,
            sliderControl->sliderPosition(), static_cast<int>(span),opt.upsideDown);
        updatePrimitives();
    }
    event->accept();
}

/*!
  reimp

*/
void HbSliderHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    HbWidget::mouseReleaseEvent(event);
#ifdef HB_EFFECTS
    if( sliderControl->orientation() == Qt::Horizontal ) {
        HbEffect::start(gItem, HB_SLIDERHANDLE_TYPE, "h_thumbrelease");
    }
    else {
        HbEffect::start(gItem, HB_SLIDERHANDLE_TYPE, "v_thumbrelease");
    }        
#endif
    HbWidgetFeedback::triggered(sliderControl, Hb::InstantReleased, Hb::ModifierSliderHandle);
    sliderControl->setSliderDown(false);
    QRectF controlRect = sliderControl->sceneBoundingRect();
    if(!controlRect.contains(event->scenePos().x() , event->scenePos().y()) ){
        sliderControl->releasedOutside(true);
    }
    if(sliderControl->singleStep() != 0) {
        HbWidgetFeedback::continuousStopped(sliderControl, Hb::ContinuousDragged);
    }
    event->accept();
    if( sliderControl->snappingMode()!= HbSliderControl::NoSnapping ){
        sliderControl->updateSliderPosToTick();
    }
    updatePrimitives();
}

/*!
  reimp

*/
void HbSliderHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    qreal span = 0;
    HbWidget::mouseMoveEvent(event);
    QRectF sliderBounds=sliderControl->boundingRect();
    QRectF handleBounds=boundingRect();
    HbStyleOptionSlider opt;
    sliderControl->initStyleOption(&opt);
    int sliderPos;
    if ( sliderControl->orientation() == Qt::Horizontal ) {
#ifdef HB_EFFECTS
        if( sliderBounds.topLeft().x() > mapToParent(event->pos()).x() ||
            sliderBounds.bottomRight().x() < mapToParent(event->pos()).x()) {
            HbEffect::start(gItem, HB_SLIDERHANDLE_TYPE, "h_outofbound");
        }
#endif
        sliderBounds.adjust(0, 0, -handleBounds.width(), 0);
        span = sliderBounds.width();
        sliderPos = static_cast<int>(
            handleDragPos+(mapToParent(event->pos()).rx()-handleInitialPos));
    } else {
#ifdef HB_EFFECTS
        if( sliderBounds.topLeft().y() > mapToParent(event->pos()).y() ||
            sliderBounds.bottomRight().y() < mapToParent(event->pos()).y()) {
            HbEffect::start(gItem, HB_SLIDERHANDLE_TYPE, "v_outofbound");
        }
#endif
        sliderBounds.adjust(0, 0, 0, -handleBounds.height());
        span = sliderBounds.height();
        sliderPos = static_cast<int>(
            handleDragPos+(mapToParent(event->pos()).ry()-handleInitialPos));
    }
    int pressValue= QStyle::sliderValueFromPosition(opt.minimum, opt.maximum,
        sliderPos, static_cast<int>(span),opt.upsideDown);
    sliderControl->setSliderPosition(pressValue);
    sliderControl->showToolTip();
}

/*!
  reimp

*/
void HbSliderHandle::updatePrimitives()
{
    HbStyleOptionSlider opt;
    sliderControl->initStyleOption(&opt);
    if( scene()){
        if (true == sliderControl->isSliderDown()){
            opt.state |= QStyle::State_Sunken;
            sliderControl->style()->updatePrimitive(
                sliderControl->primitive(HbStyle::P_Slider_groove), HbStyle::P_Slider_groove, &opt);
        } else {
            opt.state &= ~QStyle::State_Sunken;
        } 
    } else {
        opt.state &= ~QStyle::State_Sunken;
    }

    opt.boundingRect=boundingRect();
    sliderControl->style()->updatePrimitive(gItem, HbStyle::P_Slider_thumb, &opt); 
    sliderControl->style()->updatePrimitive(touchItem , HbStyle::P_SliderElement_touchhandle ,&opt);
    sliderControl->update(boundingRect());
}

/*!
  reimp

*/
QGraphicsItem * HbSliderHandle::primitive(HbStyle::Primitive primitive) const
 {
       switch (primitive) {
        case HbStyle::P_Slider_thumb:
            return gItem;
        case HbStyle::P_SliderElement_touchhandle:
            return touchItem;
        default:
            return NULL;
    }
 }
