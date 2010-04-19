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

#include "hbgroupboxcontentwidget_p.h"
#include <hbstyleoption.h>
#include <hbscrollarea.h>
#include <hbgesturefilter.h>
#include <hbgesture.h>
#include <QGraphicsSceneMouseEvent>

/*
    internal

    constructs HbGroupBoxContentWidget

    HbGroupBoxContentWidget is internal to HbGroupBox, and is applicable to RichLabel and
    CollapsingContainer type.
    
    HbGroupBoxContentWidget holds the body content.
*/
HbGroupBoxContentWidget::HbGroupBoxContentWidget(QGraphicsItem *parent ) :
    HbWidget(parent),
    mContent(0),
    mBackgroundItem(0),
    contentPressed(false),
    gestureFilter(0),
    gestureLongpressed(0)
{
    groupBox = qgraphicsitem_cast<HbGroupBox*>( parent );

    createPrimitives();
    createConnection();    
}

/*
    internal

    Destructs HbGroupBoxContentWidget
*/
HbGroupBoxContentWidget::~HbGroupBoxContentWidget()
{
    if( gestureFilter ) {
        removeSceneEventFilter( gestureFilter );
    }
}

/*
    create primitives
*/
void HbGroupBoxContentWidget::createPrimitives()
{
    if( groupBoxType == GroupBoxRichLabel ){
        if ( !mBackgroundItem ) {
            mBackgroundItem = style()->createPrimitive( HbStyle::P_GroupBoxContent_background , this );
        }
    }else if ( mBackgroundItem ) {
            delete mBackgroundItem;
            mBackgroundItem = 0;
    }
}

/*!
    updates the primitives
 */
void HbGroupBoxContentWidget::updatePrimitives()
{
   HbStyleOption option;
   initStyleOption( &option );

    if ( mBackgroundItem ) {
        style()->updatePrimitive( mBackgroundItem , HbStyle::P_GroupBoxContent_background , &option );
    }
}

/*
    internal

    creates signal connection for contentWidget
*/
void HbGroupBoxContentWidget::createConnection()
{
    // Create gesture filter
    gestureFilter = new HbGestureSceneFilter( Qt::LeftButton, this );
    // Add gestures for longpress
    gestureLongpressed = new HbGesture( HbGesture::longpress,5 );
    gestureFilter->addGesture( gestureLongpressed );

    installSceneEventFilter( gestureFilter );

    // to avoid duplicate signals getting emitted from groupBox contentWidget
    disconnect( this , SIGNAL ( clicked() ) , groupBox , SIGNAL ( clicked() ));
    disconnect ( gestureLongpressed , SIGNAL( longPress( QPointF ) ) , groupBox , SIGNAL( longPress( QPointF ) ));

    connect ( this , SIGNAL ( clicked() ) , groupBox , SIGNAL ( clicked() ));
    connect ( gestureLongpressed , SIGNAL( longPress( QPointF ) ) , groupBox , SIGNAL( longPress( QPointF ) ));
}

/*!
    internal

    Sets the groupbox content widgets type.
    Creates primitves if needed based on type set
*/
void HbGroupBoxContentWidget::setType(GroupBoxType type)
{
    groupBoxType = type;
    // set dynamic properties for type
    if(groupBoxType == GroupBoxCollapsingContainer)
        setProperty("groupBoxType",3);
    else if(groupBoxType == GroupBoxRichLabel)
        setProperty("groupBoxType",2);

    if(groupBoxType != GroupBoxSimpleLabel){
       createPrimitives();
       //createConnection();
    }
}

/*!
    internal

    Sets HbGroupBoxContentWidget content
*/
void HbGroupBoxContentWidget::setContentWidget( HbWidget *widget )
{
    // delete old content set
    if ( mContent ) {
        delete mContent; 
        mContent = 0;
    }
     // if NULL widget is passed dont do anything
    if ( !widget   ) {
        return;
    }

    mContent = widget;
    style()->setItemName( mContent , "content" );
    mContent->setParentItem( this);

    if(groupBoxType == GroupBoxRichLabel){
        contentPressed = false;
    }
}

/*!
    Returns the pointer for \a primitive passed.
    Will return NULL if \a primitive passed is invalid
*/
QGraphicsItem* HbGroupBoxContentWidget::primitive(HbStyle::Primitive primitive) const
{
    switch (primitive) {
        case HbStyle::P_GroupBoxContent_background:
            return mBackgroundItem;
        default:
            return 0;
    }
}

/*!
    \reimp
 */
void HbGroupBoxContentWidget::initStyleOption( HbStyleOption *option )const
{
    HbWidget::initStyleOption (option);
    if (contentPressed ) {
        option->state = QStyle::State_On;
    } else {
        option->state = QStyle::State_Off;
    }
}

/*!
    \reimp
 */
QVariant HbGroupBoxContentWidget::itemChange( GraphicsItemChange change, const QVariant &value )
{
    switch( change )
    {
        case ItemChildAddedChange:
        case ItemChildRemovedChange:
            repolish();
             break;
        case ItemSceneHasChanged:		
            updatePrimitives();
            break;
        default:
            break;
    }
    return HbWidget::itemChange( change, value );
}

/*!
    \reimp
 */
void HbGroupBoxContentWidget::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
    HbWidget::mousePressEvent(event);

    contentPressed=!contentPressed;
    updatePrimitives();
    event->accept(); 
}

/*!
    \reimp
 */
void HbGroupBoxContentWidget::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
    HbWidget::mouseReleaseEvent(event);

    contentPressed=!contentPressed;    
    updatePrimitives();
    
    emit clicked();
}

/*!
    \reimp
 */
void HbGroupBoxContentWidget::polish( HbStyleParameters& params )
{
    if(groupBoxType == GroupBoxCollapsingContainer){
        // set dynamic property for contentwidget, if it is scrollable content
        // if content is scrollable, then groupBox leaves out margin spacing b/w heading
        // and content area, otherwise zero margin spacing will be taken
        HbScrollArea* scrollContent = qobject_cast<HbScrollArea*>( mContent );		
        if(scrollContent)
           setProperty("scrollableContent",true);
        else
           setProperty("scrollableContent",false);
        }
    HbWidget::polish(params);
}


