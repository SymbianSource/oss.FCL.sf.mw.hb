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

#include "hbdatagroupheadingwidget_p.h"
#include "hbdataformviewitem_p.h"
#include <hbstyleoptiondatagroupheadingwidget_p.h>
#include "hbdatagroup_p.h"
#include "hbdatagroup_p_p.h"
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <hbwidgetfeedback.h>

#ifdef HB_GESTURE_FW
#include <hbtapgesture.h>
#include <hbpangesture.h>
#endif

HbDataGroupHeadingWidget::HbDataGroupHeadingWidget(QGraphicsItem *parent ) :
    HbWidget(parent),
    mBackgroundItem(0),
    mHeadingItem(0),
    mIconItem(0),
    mDescriptionItem(0),
    mParent(0),
    mExpanded(false),
    mDown(false),
    mLongPressed(false)
{
    
#ifdef HB_GESTURE_FW
    grabGesture( Qt::TapGesture );
#endif
}

HbDataGroupHeadingWidget::~HbDataGroupHeadingWidget()
{
#ifdef HB_GESTURE_FW
    ungrabGesture( Qt::TapGesture );
#endif
}

void HbDataGroupHeadingWidget::createPrimitives()
{   
    QObject::connect(mParent, SIGNAL(longPressed(const QPointF )), this, SLOT(longPressed(const QPointF )));
    if(!mBackgroundItem) {
        mBackgroundItem = HbStylePrivate::createPrimitive(HbStylePrivate::P_DataGroup_background, this);
    }

    //if heading is empty still heading item has to be layouted
    if(!mHeadingItem) {
        mHeadingItem = HbStylePrivate::createPrimitive(HbStylePrivate::P_DataGroup_heading, this);
        setProperty("state","normal");
    }

    if(!mDescription.isEmpty()) {
        if(!mDescriptionItem) {
            mDescriptionItem = HbStylePrivate::createPrimitive(HbStylePrivate::P_DataGroup_description, this);
            setProperty("state","normal");
        }
    } else {
        if(mDescriptionItem) {
            delete mDescriptionItem;
            mDescriptionItem = 0;
        }
    }

    if(!mIconItem) {
        mIconItem = HbStylePrivate::createPrimitive(HbStylePrivate::P_DataGroup_icon, this);
    }

    repolish();
}

void HbDataGroupHeadingWidget::updatePrimitives()
{
    HbStyleOptionDataGroupHeadingWidget settingGroupOption;
    initStyleOption(&settingGroupOption);

    if(mHeadingItem) {
        HbStylePrivate::updatePrimitive( 
            mHeadingItem, HbStylePrivate::P_DataGroup_heading, &settingGroupOption);
    }
    
    if(mDescriptionItem) {
        HbStylePrivate::updatePrimitive( 
            mDescriptionItem, HbStylePrivate::P_DataGroup_description, &settingGroupOption);
    }
    if(mIconItem) {
        HbStylePrivate::updatePrimitive( mIconItem, HbStylePrivate::P_DataGroup_icon, &settingGroupOption);
    }

    if(mBackgroundItem) {
        HbStylePrivate::updatePrimitive(
            mBackgroundItem, HbStylePrivate::P_DataGroup_background, &settingGroupOption);
    }
}

void HbDataGroupHeadingWidget::initStyleOption(HbStyleOptionDataGroupHeadingWidget *option)
{
    HbWidget::initStyleOption(option);
    option->heading = mHeading;
    option->description = mDescription;
    option->expanded = mExpanded;
    option->pressed = mDown;
}

#ifndef HB_GESTURE_FW
void HbDataGroupHeadingWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }
    mDown = true;
 
    HbStyleOptionDataGroupHeadingWidget settingGroupOption;
    initStyleOption(&settingGroupOption);
    if(mBackgroundItem) {
        HbStylePrivate::updatePrimitive(
            mBackgroundItem, HbStylePrivate::P_DataGroup_background, &settingGroupOption);
    }

    Hb::InteractionModifiers modifiers = Hb::ModifierExpandedItem;
    HbWidgetFeedback::triggered(this, Hb::InstantPressed, modifiers);
}

void HbDataGroupHeadingWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Hb::InteractionModifiers modifiers = 0;

    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }
    
    if(mDown && rect().contains(event->pos())) {
        static_cast<HbDataGroup*>(mParent)->setExpanded(
                !static_cast<HbDataGroup*>(mParent)->isExpanded());
        modifiers |= Hb::ModifierExpandedItem;
    }
    mDown = false;

    HbStyleOptionDataGroupHeadingWidget settingGroupOption;
    initStyleOption(&settingGroupOption);
    if(mBackgroundItem) {
        HbStylePrivate::updatePrimitive(
            mBackgroundItem, HbStylePrivate::P_DataGroup_background, &settingGroupOption);
    }

    HbWidgetFeedback::triggered(this, Hb::InstantReleased, modifiers);
}
#endif

#ifdef HB_GESTURE_FW
void HbDataGroupHeadingWidget::gestureEvent(QGestureEvent *event)
{
    Hb::InteractionModifiers modifiers = 0;
    if (event->gesture(Qt::TapGesture)) {
        HbDataGroupPrivate::d_ptr(static_cast<HbDataGroup*>(mParent))->tapTriggered( event );
    }
    if (HbTapGesture *tap = qobject_cast<HbTapGesture *>(event->gesture(Qt::TapGesture))) {
        switch(tap->state()) {

        case Qt::GestureStarted:
            {
                if (scene())
                    scene()->setProperty(HbPrivate::OverridingGesture.latin1(),Qt::TapGesture);
                tap->setProperty(HbPrivate::ThresholdRect.latin1(), mapRectToScene(boundingRect()).toRect());

                mDown = true;
                mLongPressed = false;
                HbStyleOptionDataGroupHeadingWidget settingGroupOption;
                initStyleOption(&settingGroupOption);
                if(mBackgroundItem) {
                    HbStylePrivate::updatePrimitive(
                    mBackgroundItem, HbStylePrivate::P_DataGroup_background, &settingGroupOption);
                }
                modifiers = Hb::ModifierExpandedItem;
                HbWidgetFeedback::triggered(this, Hb::InstantPressed, modifiers);
                break;
            }

        case Qt::GestureFinished:
            {
                modifiers = 0;

                if(mDown && !mLongPressed && rect().contains(mapFromScene(event->mapToGraphicsScene(tap->position())))) {        
                static_cast<HbDataGroup*>(mParent)->setExpanded(
                    !static_cast<HbDataGroup*>(mParent)->isExpanded());
                    modifiers |= Hb::ModifierExpandedItem;
                }
                mDown = false;
                    
                HbStyleOptionDataGroupHeadingWidget settingGroupOption;
                initStyleOption(&settingGroupOption);
                if(mBackgroundItem) {
                    HbStylePrivate::updatePrimitive(
                    mBackgroundItem, HbStylePrivate::P_DataGroup_background, &settingGroupOption);
                }

                HbWidgetFeedback::triggered(this, Hb::InstantReleased, modifiers);

                if (scene()) {
                    scene()->setProperty(HbPrivate::OverridingGesture.latin1(),QVariant());
                }
                break;
            }
        case Qt::GestureCanceled:
        {
            modifiers = 0;
            mDown = false;
            HbStyleOptionDataGroupHeadingWidget settingGroupOption;
            initStyleOption(&settingGroupOption);
            if(mBackgroundItem) {
                HbStylePrivate::updatePrimitive(
                mBackgroundItem, HbStylePrivate::P_DataGroup_background, &settingGroupOption);
            }

            HbWidgetFeedback::triggered(this, Hb::InstantReleased, modifiers);

            if (scene()) {
                scene()->setProperty(HbPrivate::OverridingGesture.latin1(),QVariant());
            }
            break;

         }
            
        default:
            break;
        }
    }
    
}


void HbDataGroupHeadingWidget::longPressed(const QPointF &position)
{
    Q_UNUSED(position);
    mLongPressed = true;
}
#endif
