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

#include "hbgroupboxheadingwidget_p.h"

#include <hbstyleoptiongroupbox_p.h>
#include <hbwidgetfeedback.h>

#ifdef HB_EFFECTS
#include <hbeffect.h>
#include "hbeffectinternal_p.h"
#define HB_GROUPBOX_HEADING_TYPE "HB_GROUPBOX_HEADING"
#endif
#ifdef HB_GESTURE_FW
#include <hbtapgesture.h>
#include <hbnamespace_p.h>
#include <QGesture>
#include <QGraphicsScene>
#endif 

#include <QGraphicsSceneMouseEvent>

/*
    internal
    constructs HbGroupBoxHeadingWidget

    HbGroupBoxHeadingWidget is internal to HbGroupBox, and is applicable to SimpleLabel and
    CollapsingContainer type.
    
    HbGroupBoxHeadingWidget holds the heading text and allows the groupBox to set marquee on it.

*/
HbGroupBoxHeadingWidget::HbGroupBoxHeadingWidget( QGraphicsItem *parent ) :
    HbWidget( parent ),
    mIconItem( 0 ),
    mTextItem( 0 ),
    mBackgroundItem( 0 ),
    marqueeEnabled( false ),
    headingPressed(false),
    groupBoxType( GroupBoxTypeUnknown )
{
    groupBox = qgraphicsitem_cast<HbGroupBox*>( parent );
    init( );
    setProperty( "state", "normal" );
}

/*
    internal
    Destructs HbGroupBoxHeadingWidget
*/
HbGroupBoxHeadingWidget::~HbGroupBoxHeadingWidget( )
{
}

/*
    internal
    init
*/
void HbGroupBoxHeadingWidget::init( )
{
    createPrimitives( );
#ifdef HB_GESTURE_FW
    grabGesture( Qt::TapGesture );
#endif 
}

/*
  create primitives
*/
void HbGroupBoxHeadingWidget::createPrimitives()
{
    if( groupBoxType == GroupBoxCollapsingContainer && groupBox->isCollapsable() ){
        if ( !mIconItem ) {
            mIconItem = HbStylePrivate::createPrimitive( HbStylePrivate::P_GroupBoxHeading_icon , this );
        }
    }
    else if ( mIconItem ) {
        delete mIconItem;
        mIconItem = 0;
    }
    if ( mTextItem ) {
        delete mTextItem;
        mTextItem = 0;
    }
    if ( groupBoxType == GroupBoxCollapsingContainer ) {
        mTextItem = HbStylePrivate::createPrimitive( HbStylePrivate::P_GroupBoxHeading_text , this );
    }
    else {
        mTextItem = HbStylePrivate::createPrimitive( HbStylePrivate::P_GroupBoxMarquee_text , this );
    }

    if ( !mBackgroundItem ) {
        mBackgroundItem = HbStylePrivate::createPrimitive( HbStylePrivate::P_GroupBoxHeading_background , this );
        HbStyle::setItemName( mBackgroundItem , "background" );
    }
}

/*!
    update primitives
 */
void HbGroupBoxHeadingWidget::updatePrimitives()
{
    HbStyleOptionGroupBox option;
    initStyleOption( &option );
   
    if ( mIconItem ) {
            HbStylePrivate::updatePrimitive( mIconItem , HbStylePrivate::P_GroupBoxHeading_icon , &option );
    }       
    if ( mTextItem ) {
        if ( groupBoxType == GroupBoxCollapsingContainer ) {
            HbStylePrivate::updatePrimitive( mTextItem , HbStylePrivate::P_GroupBoxHeading_text , &option );
        }
        else {
            HbStylePrivate::updatePrimitive( mTextItem , HbStylePrivate::P_GroupBoxMarquee_text , &option );
        }        
    }
    if ( mBackgroundItem ) {
        HbStylePrivate::updatePrimitive( mBackgroundItem , HbStylePrivate::P_GroupBoxHeading_background , &option );
    }
}

/*
    internal
    creates signal connection for headingwidget based on type
*/
void HbGroupBoxHeadingWidget::createConnection()
{

#ifdef HB_EFFECTS
    HbEffectInternal::add( HB_GROUPBOX_HEADING_TYPE,"groupbox_icon_click", "iconclick" );
#endif 

    connect(this, SIGNAL(clicked(bool)), groupBox, SLOT(setCollapsed(bool)));
}

/*!
    internal
    Sets the groupbox heading widgets type.
    Create primitves & connect signals if needed based on type set
*/
void HbGroupBoxHeadingWidget::setType(GroupBoxType type)
{
    if ( groupBoxType != type ){
        groupBoxType = type;
        // setting dynamic properties for type
        if( groupBoxType == GroupBoxCollapsingContainer )
            setProperty( "groupBoxType",3 );
        else if( groupBoxType == GroupBoxSimpleLabel )
            setProperty( "groupBoxType",1 );

        if( groupBoxType != GroupBoxRichLabel ){
            createPrimitives( );
        }

        if( groupBoxType == GroupBoxCollapsingContainer ){
            createConnection(); 
            // collapsed is false by default for CollapsingContainer
            //collapsed = false; 
            // marquee is disabled by default for CollapsingContainer
            marqueeEnabled = false;
        }

        if( groupBoxType == GroupBoxSimpleLabel ){
            // marquee is disabled by default for simple label
        marqueeEnabled = false;         
        }
    }

}

/*!
    internal
    Sets the groupbox heading widgets text
*/
void HbGroupBoxHeadingWidget::setHeading( const QString &text )
{
    if( headingText != text ){
        headingText = text;

        HbStyleOptionGroupBox groupBoxOption;
        initStyleOption( &groupBoxOption );
        if ( groupBoxType == GroupBoxCollapsingContainer ) {
            HbStylePrivate::updatePrimitive( mTextItem , HbStylePrivate::P_GroupBoxHeading_text , &groupBoxOption );
        }
        else {
           HbStylePrivate::updatePrimitive( mTextItem , HbStylePrivate::P_GroupBoxMarquee_text , &groupBoxOption );
        }
    }
}

/*!
    internal
    Sets marquee for groupbox heading
*/
void HbGroupBoxHeadingWidget::setMarqueeHeading( bool marquee )
{
    if( marqueeEnabled != marquee ){
        marqueeEnabled  = marquee;

        HbStyleOptionGroupBox groupBoxOption;
        initStyleOption( &groupBoxOption );
        HbStylePrivate::updatePrimitive( mTextItem, HbStylePrivate::P_GroupBoxMarquee_text, &groupBoxOption);
    }
}

/*!

    \deprecated HbGroupBoxHeadingWidget::primitive(HbStyle::Primitive)
        is deprecated.

    Returns the pointer for \a primitive passed.
    Will return NULL if \a primitive passed is invalid
*/
QGraphicsItem *HbGroupBoxHeadingWidget::primitive( HbStyle::Primitive primitive ) const
{
    switch (primitive) {
        case HbStylePrivate::P_GroupBoxHeading_icon:
            return mIconItem;
        case HbStylePrivate::P_GroupBoxHeading_text:
        case HbStylePrivate::P_GroupBoxMarquee_text:
            return mTextItem;
        case HbStylePrivate::P_GroupBoxHeading_background:
            return mBackgroundItem;
        default:
            return 0;
    }
}

/*!
    \reimp
    Initialize \a option with the values from this HbGroupBox. This method
    is useful for subclasses when they need a HbStyleOptionGroupBox, but don't want
    to fill in all the information themselves.
*/
void HbGroupBoxHeadingWidget::initStyleOption( HbStyleOptionGroupBox *option ) const
{
    HbWidget::initStyleOption( option );
    //option->collapsed = collapsed; 
    option->collapsed = groupBox->isCollapsed();
    option->heading = headingText;
    option->marqueeEnabled = marqueeEnabled;
    // state & type info reqd fo background primitve updation
    if ( headingPressed ) {
        option->state = QStyle::State_On;
    } else {
        option->state = QStyle::State_Off;
    }
    if( groupBoxType == GroupBoxCollapsingContainer )
        option->groupBoxType = HbStyleOptionGroupBox::GroupBoxCollapsingContainer;
    else if( groupBoxType == GroupBoxSimpleLabel )
        option->groupBoxType = HbStyleOptionGroupBox::GroupBoxSimpleLabel;
}

/*!
    \reimp
 */
QVariant HbGroupBoxHeadingWidget::itemChange( GraphicsItemChange change, const QVariant &value )
{
    switch ( change ) {
        case ItemVisibleChange: 
        case ItemSceneHasChanged: 
            updatePrimitives();
            break;
        case ItemChildAddedChange:
        case ItemChildRemovedChange:
            repolish();
            break;
        default:
            break;
    }
    return HbWidget::itemChange( change, value );
}

/*!
    \reimp
 */
void HbGroupBoxHeadingWidget::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
#ifdef HB_GESTURE_FW
    Q_UNUSED( event )
#else 
    HbWidget::mousePressEvent( event );
    if ( !( groupBox->isCollapsable( ) ) ){
        event->ignore();
        return;
    }

    if (groupBoxType == GroupBoxCollapsingContainer) {
        HbWidgetFeedback::triggered( this, Hb::InstantPressed, Hb::ModifierCollapsedItem );
    }
    else {
        HbWidgetFeedback::triggered( this, Hb::InstantPressed );
    }

    event->accept();
    // background primitive updation, upon mouse press
    headingPressed = true;

    HbStyleOptionGroupBox groupBoxOption;
    initStyleOption( &groupBoxOption );
    HbStylePrivate::updatePrimitive( mBackgroundItem , HbStylePrivate::P_GroupBoxHeading_background , &groupBoxOption );

    setProperty( "state", "pressed" );
#endif  
}

/*!
    \reimp
 */
void HbGroupBoxHeadingWidget::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
#ifdef HB_GESTURE_FW
    Q_UNUSED(event)
#else 
    HbWidget::mouseReleaseEvent( event );

    if (groupBoxType == GroupBoxCollapsingContainer) {
        HbWidgetFeedback::triggered( this, Hb::InstantReleased, Hb::ModifierCollapsedItem );
    }
    else {
        HbWidgetFeedback::triggered( this, Hb::InstantReleased );
    }

    if ( this->isUnderMouse() ) {
        if ( mIconItem && mIconItem->isUnderMouse() ) {
    #ifdef HB_EFFECTS
        HbEffect::start( mIconItem, HB_GROUPBOX_HEADING_TYPE, "iconclick");
    #endif
        }
        emit clicked( !collapsed );
    }
    // background primitive updation, upon mouse release
    headingPressed = false;

    HbStyleOptionGroupBox groupBoxOption;
    initStyleOption( &groupBoxOption );
    HbStylePrivate::updatePrimitive( mBackgroundItem , HbStylePrivate::P_GroupBoxHeading_background , &groupBoxOption );

    setProperty( "state", "normal" );
#endif 
}

#ifdef HB_GESTURE_FW
void HbGroupBoxHeadingWidget::gestureEvent(QGestureEvent *event)
{
    if(HbTapGesture *tap = qobject_cast<HbTapGesture*>( event->gesture( Qt::TapGesture ) ) ) {
        //only in case of collapsing container heading will take events
        if(groupBoxType == GroupBoxCollapsingContainer){
            switch( tap->state() ) {
                case Qt::GestureStarted:  //
                    {
                        scene()->setProperty(HbPrivate::OverridingGesture.latin1(),Qt::TapGesture);
                        if (!tap->property(HbPrivate::ThresholdRect.latin1()).toRect().isValid()) {
                            tap->setProperty(HbPrivate::ThresholdRect.latin1(), mapRectToScene(boundingRect()).toRect());
                        }

                        if ( !(groupBox->isCollapsable( )) ){
                            event->ignore( tap );
                            return;
                        }
                        if ( groupBox->isCollapsable( ) ) {
                            HbWidgetFeedback::triggered( this, Hb::InstantPressed, Hb::ModifierCollapsedItem );
                        }
                        else {
                            HbWidgetFeedback::triggered( this, Hb::InstantPressed );
                        }

                        // background primitive updation, upon mouse press
                        headingPressed = true;

                        HbStyleOptionGroupBox groupBoxOption;
                        initStyleOption( &groupBoxOption );
                        HbStylePrivate::updatePrimitive( mBackgroundItem , HbStylePrivate::P_GroupBoxHeading_background , &groupBoxOption );

                        setProperty( "state", "pressed" );
                    }

                    break;
                case Qt::GestureCanceled: // Reset state
                    {
                        scene()->setProperty(HbPrivate::OverridingGesture.latin1(),QVariant());

                        headingPressed = false;

                        HbStyleOptionGroupBox groupBoxOption;
                        initStyleOption( &groupBoxOption );
                        HbStylePrivate::updatePrimitive( mBackgroundItem , HbStylePrivate::P_GroupBoxHeading_background , &groupBoxOption );

                        setProperty( "state", "normal" );
                        break;
                    }
                case Qt::GestureFinished: // emit clicked
                    {
                        scene()->setProperty(HbPrivate::OverridingGesture.latin1(),QVariant());

                        if ( groupBox->isCollapsable( ) ) {
                            HbWidgetFeedback::triggered( this, Hb::InstantReleased, Hb::ModifierCollapsedItem );
                        } else {
                            HbWidgetFeedback::triggered( this, Hb::InstantReleased );
                        }

                        if ( this->isUnderMouse() ) {
                            if ( mIconItem && mIconItem->isUnderMouse( ) ) {
                            #ifdef HB_EFFECTS
                                HbEffect::start( mIconItem, HB_GROUPBOX_HEADING_TYPE, "iconclick" );
                            #endif
                            }
                            emit clicked( !(groupBox->isCollapsed()) );
                        }
                        // background primitive updation, upon mouse release
                        headingPressed = false;

                        HbStyleOptionGroupBox groupBoxOption;
                        initStyleOption( &groupBoxOption );
                        HbStylePrivate::updatePrimitive( mBackgroundItem, HbStylePrivate::P_GroupBoxHeading_background, &groupBoxOption );

                        setProperty( "state", "normal" );
                    }
                    break;
                default:
                    break;
            }

        }
    }
}
#endif 


