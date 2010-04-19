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
#include <hbdatagroup_p.h>
#include <QGraphicsItem>

HbDataGroupHeadingWidget::HbDataGroupHeadingWidget(QGraphicsItem *parent ) :
    HbWidget(parent),
    mBackgroundItem(0),
    mHeadingItem(0),
    mIconItem(0),
    mParent(0),
    mExpanded(false),
    mDown(false)
{
}

HbDataGroupHeadingWidget::~HbDataGroupHeadingWidget()
{
}

void HbDataGroupHeadingWidget::createPrimitives()
{
    if(!mBackgroundItem) {
        mBackgroundItem = style()->createPrimitive(HbStyle::P_DataGroup_background, this);
    }

    if(!mHeading.isEmpty()) {
        if(!mHeadingItem) {
            mHeadingItem = style()->createPrimitive(HbStyle::P_DataGroup_heading, this);
        }
    } else {
        if(mHeadingItem) {
            delete mHeadingItem;
            mHeadingItem = 0;
        }
    }

    if(!mIconItem) {
        mIconItem = style()->createPrimitive(HbStyle::P_DataGroup_icon, this);
    }

    repolish();
}

void HbDataGroupHeadingWidget::updatePrimitives()
{
    HbStyleOptionDataGroupHeadingWidget settingGroupOption;
    initStyleOption(&settingGroupOption);

    if(mHeadingItem) {
        style()->updatePrimitive( 
            mHeadingItem, HbStyle::P_DataGroup_heading, &settingGroupOption);
    }

    if(mIconItem) {
        style()->updatePrimitive( mIconItem, HbStyle::P_DataGroup_icon, &settingGroupOption);
    }

    if(mBackgroundItem) {
        style()->updatePrimitive(
            mBackgroundItem, HbStyle::P_DataGroup_background, &settingGroupOption);
    }
}

void HbDataGroupHeadingWidget::initStyleOption(HbStyleOptionDataGroupHeadingWidget *option)
{
    HbWidget::initStyleOption(option);
    option->heading = mHeading;
    option->expanded = mExpanded;
    option->pressed = mDown;
}


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
        style()->updatePrimitive(
            mBackgroundItem, HbStyle::P_DataGroup_background, &settingGroupOption);
    }
 }

void HbDataGroupHeadingWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }
    
    if(mDown && rect().contains(event->pos())) {        
        static_cast<HbDataGroup*>(mParent)->setExpanded(
                !static_cast<HbDataGroup*>(mParent)->isExpanded());
    }
    mDown = false;

    HbStyleOptionDataGroupHeadingWidget settingGroupOption;
    initStyleOption(&settingGroupOption);
    if(mBackgroundItem) {
        style()->updatePrimitive(
            mBackgroundItem, HbStyle::P_DataGroup_background, &settingGroupOption);
    }
}

