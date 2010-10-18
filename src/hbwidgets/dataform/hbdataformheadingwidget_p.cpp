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

#include "hbdataformheadingwidget_p.h"
#include "hbstyle_p.h"

#include <hbcombobox.h>
#include <hbstyleoptiondataform_p.h>
#include <QGraphicsItem>


void HbDataFormHeadingWidget::initStyleOption(HbStyleOptionDataForm *option)
{
    HbWidget::initStyleOption(option);
    option->heading = mHeading;
    option->description = mDescription;
}

HbDataFormHeadingWidget::HbDataFormHeadingWidget(QGraphicsItem* parent):
    HbWidget(parent),
    mHeadingItem(0),
    mDescriptionItem(0),
    mBackgroundItem(0),
    mPageCombo(0),
    mActivePage(-1)
{
}

void HbDataFormHeadingWidget::createPrimitives()
{
    if(!mBackgroundItem) {
        mBackgroundItem = HbStylePrivate::createPrimitive(HbStylePrivate::P_DataForm_heading_background, this);
    }
    if(!mHeading.isEmpty()) {
        if(!mHeadingItem){
            setProperty( "hasHeading", true );
            mHeadingItem = HbStylePrivate::createPrimitive(HbStylePrivate::P_DataForm_heading, this);
        }
    } else {
        setProperty( "hasHeading", false );
        if(mHeadingItem){
            delete mHeadingItem;
            mHeadingItem = 0;
        }
    }

    if(!mDescription.isEmpty()) {
        if(!mDescriptionItem) {            
            setProperty( "hasDescription", true );
            mDescriptionItem = HbStylePrivate::createPrimitive(HbStylePrivate::P_DataForm_description, this);
        }
    } else {                  
        setProperty( "hasDescription", false );
        if(mDescriptionItem) {
            delete mDescriptionItem;
            mDescriptionItem = 0;
        }
    }
}

void HbDataFormHeadingWidget::updatePrimitives()
{
    HbStyleOptionDataForm dataFormOption;
    initStyleOption(&dataFormOption);

    if(mHeadingItem) {
        HbStylePrivate::updatePrimitive( mHeadingItem, HbStylePrivate::P_DataForm_heading, &dataFormOption);
    }

    if(mDescriptionItem) {
        HbStylePrivate::updatePrimitive( mDescriptionItem, HbStylePrivate::P_DataForm_description,
            &dataFormOption);
    }
    if(mBackgroundItem) {
        HbStylePrivate::updatePrimitive(
            mBackgroundItem, HbStylePrivate::P_DataForm_heading_background, &dataFormOption);
    }
}

void HbDataFormHeadingWidget::callPolish()
{
    repolish();
    updatePrimitives();
}

int HbDataFormHeadingWidget::pageIndex(const QString& page) const
{
    if(mPageCombo) {
       return mPageCombo->findText(page);
    }
    return -1;
}

void HbDataFormHeadingWidget::updatePageName(int index, const QString& page)
{
    if(index >= 0 && mPageCombo) {       
        if( mPageCombo->itemText(index) != page)  {          
            mPageCombo->setItemText(index,page);            
        }       
    }      
}


