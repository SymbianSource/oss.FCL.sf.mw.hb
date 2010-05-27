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

#include "hbgroupbox.h"
#include "hbgroupbox_p.h"
#include "hbgroupboxheadingwidget_p.h"
#include "hbgroupboxcontentwidget_p.h"
#include <hbstyle.h>
#include <hbstyleoption_p.h>

#ifdef HB_EFFECTS
#include <hbeffect.h>
#include "hbeffectinternal_p.h"
#define HB_GROUPBOX_TYPE "HB_GROUPBOX"
#endif

#include <QGraphicsSceneMouseEvent>
#include <QDebug>

QT_BEGIN_NAMESPACE
class QGraphicsItem;
QT_END_NAMESPACE


/*
    HbGroupBoxPrivate
    private class constructor     
*/
HbGroupBoxPrivate::HbGroupBoxPrivate()
    :HbWidgetPrivate(),
    mContentWidget( 0 ),
    mHeadingWidget( 0 ),
    mGroupBoxType( GroupBoxTypeUnknown )
{
}

/*
    private class destructor
*/
HbGroupBoxPrivate::~HbGroupBoxPrivate()
{
}

/*
    \internal
    creates groupbox HeadingWidget
*/
void HbGroupBoxPrivate::createHeadingWidget()
{
    Q_Q( HbGroupBox );

    mHeadingWidget = new HbGroupBoxHeadingWidget(q);
    HbStyle::setItemName( mHeadingWidget , "headingwidget");
}

/*
    \internal
    creates groupbox Contentwidget
*/
void HbGroupBoxPrivate::createContentWidget()
{
    Q_Q( HbGroupBox );

    mContentWidget = new HbGroupBoxContentWidget(q);
    HbStyle::setItemName( mContentWidget , "contentwidget");
}

/*!
    \internal
    Sets the groupbox type
*/
void HbGroupBoxPrivate::setGroupBoxType( GroupBoxType type )
{
    Q_Q( HbGroupBox );
    
    // set dynamic property based on type
    q->setProperty("groupBoxType",(int)type);

    if ( mGroupBoxType == type ) 
        return;

    mGroupBoxType = type;
  
    // set the type and makes necesary primitive creation/deletion
    switch(mGroupBoxType) {
        case GroupBoxSimpleLabel:
            {
            if(mHeadingWidget){
                mHeadingWidget->setType(type);				
                mHeadingWidget->setVisible(true);
             
            }else{
                createHeadingWidget();
            }

            if(mContentWidget){
                mContentWidget->setVisible(false);
                HbStyle::setItemName( mContentWidget , "");
            }
            
            }
            break;
        case GroupBoxRichLabel:
            {
            if(mHeadingWidget){
                mHeadingWidget->setVisible(false);

            }
            if(mContentWidget){
                mContentWidget->setType(type);
                mContentWidget->setVisible(true);
                HbStyle::setItemName( mContentWidget , "contentwidget");
            }else{
                createContentWidget();
            }

            }
            break;
        case GroupBoxCollapsingContainer:
            {
            if((mHeadingWidget)){
                mHeadingWidget->setType(type);
                mHeadingWidget->setVisible(true);                
            }else{
                createHeadingWidget();
            }

            if(mContentWidget){
                mContentWidget->setType(type);
                if(!q->isCollapsed()){
                    mContentWidget->setVisible(true);
                    HbStyle::setItemName( mContentWidget , "contentwidget");
                }else{
                    mContentWidget->setVisible(false);
                    HbStyle::setItemName( mContentWidget , "");
                }
            }else{
                createContentWidget();
            }
            }
            break;
        default:
            break;
    }

    q->updatePrimitives();
    q->repolish();
}


/*!
    @beta
    @hbwidgets
    \class HbGroupBox

    \brief HbGroupBox shows the user that a set of controls belong together.
    
    HbGroupBox is a container, which can have following Elementes: 

    \li Heading: A heading contains one row text and disclosure indicator if the disclosure mechanism is on.
    \li Body content: Can have arbitrary content (any HbWidget)and application is responsible for its layout.
    \li Disclosure Indicator: Indicates the expands and collapses state of body;

    There are three types of GroupBox:

    \li Simple Label - Simple Label is to indicate the users position in the application hierarchy 
    or to display a textual label and its noninteractive. 
    Simple Label has a heading text with marquee, no disclosure mechanism, and 
    no body content.Simple label only has heading element. 
    Marquee is disabled by default, can be enabled using setMarqueeHeading () .Also it is not focusable. 

    Example usage: For SimpleLabel type groupbox
    \code
    // create groupBox and set only heading; without any body content
    HbGroupBox *simpleLabel = new HbGroupBox();
    simpleLabel->setHeading("Simple label groupBox comes with marquee disabled by default");
    //to make marqee enabled
    simpleLabel->setMarqueeHeading(true);
    \endcode 
    
    \image html simpleLabelgroupbox.png A SimpleLabel groupbox


    \li Rich Label - Rich label can contain dynamic content,which themselves can be interactive,or informative
    with no heading ,no marquee and no disclosure control.Body Content must describe its own behavior and layout. 

    Example usage: For RichLabel type groupbox
    \code
    // create groupBox and set only content; without any heading
    HbGroupBox *richLabel = new HbGroupBox();
    HbPushButton *button = new HbPushButton(HbIcon(QString("qtg_large_info")),"Ovi");
    button->setAdditionalText("Launch Ovi Music store");
    richLabel->setContentWidget(button);
    mainlayout->addItem(richLabel);
    \endcode 

    \image html richLabelgroupbox.png A RichLabel groupbox.
    In RichLabel type, groupbox provides background for body content.

    \li Collapsing container - Collapsing container allows the user to show or hide the content of the groupBox.
    It always has a heading and body content; optionally has a disclosure mechanism.
    The heading does not marquee.The body content must describe its own behavior and layout.
    The collapse/expand disclosure mechanism is located in the heading and is the chief utility of this type of groupbox.

    If disclosure mechanism is Off,then heading will appear without expand/collapse indication icon 
    heading.Also the user will not be able to expand/collapse the body content.

    Example usage:For collapsingContainer groupbox
    \code    
    // create groupBox and set both heading and content
    HbGroupBox *collapsingContainer = new HbGroupBox();
    HbPushButton *button = new HbPushButton("Collapsing container content");
    button->setMaximumHeight(50);
    // content widget can be any HbWidget
    // layouting and interaction behaviour inside Content widget is application's responsiblity
    collapsingContainer->setContentWidget(button);
    collapsingContainer->setHeading("collapsing container");
    \endcode

    \image html collapsableContainergroupbox.png A Collapsing container groupbox.

    In this type, groupBox body content can be expanded/collapsed, 
    depending on whether or not the groupbox is collapsed.

    CollapsingContainer type groupBox comes with disclosure mechanism On by default.

    Setting heading and body content decides type of the groupbox.

    Groupbox type determines the default visualization, associated properties and suggest usages.    
*/

/*!
    \fn void HbGroupBox::longPress( QPointF )

    This signal is emitted only in case of richLabel and collapsing container groupbox,
    when the long press happened on body content.
  */

/*!
    \fn void HbGroupBox::clicked()

    This signal is emitted only in case of richLabel and collapsing container groupbox,
    whenever click happened on body content.If the body content set is an interactive widget
    and consumes mouse press event, then in that case clicked signal will not get emitted from groupBox.
 */

/*!
    \fn void HbGroupBox::toggled(bool)

    This signal is emitted only in case of collapsing container groupbox,
    whenever groupbox is collapsed/expanded
 */

/*!
    @beta
    Constructs a groupbox with the given \a parent.
*/
HbGroupBox::HbGroupBox( QGraphicsItem *parent)
    : HbWidget(*new HbGroupBoxPrivate, parent)
{
    Q_D( HbGroupBox );
    d->q_ptr = this;
}

/*!
    protected constructor for derived class
*/
HbGroupBox::HbGroupBox(HbGroupBoxPrivate &dd, QGraphicsItem *parent)
    :HbWidget( dd, parent )
{
    Q_D( HbGroupBox );
    d->q_ptr = this;
}

/*!
    Destructs the groupbox.
*/
HbGroupBox::~HbGroupBox()
{
}

/*!
    @beta
    
    Sets the groupbox heading

    Note: heading property is valid for simpleLabel & collapsing container type.
    For Collapsing container,
    if body content is collapsible ,heading will appear along with Disclosure indicator.
    If heading is set on richLabel type groupBox, it will be ignored

    \sa heading
*/
void HbGroupBox::setHeading( const QString &text )
{
    Q_D( HbGroupBox );

    if(!d->mHeadingWidget)
        d->createHeadingWidget();
    
    d->mHeadingWidget->setHeading(text);

    if(d->mContentWidget){
        d->setGroupBoxType(GroupBoxCollapsingContainer);
    }else
        d->setGroupBoxType(GroupBoxSimpleLabel);
}

/*!
    @beta

    Returns text shown on the groupBox heading.

    There is no default heading string set.

    Note: If groupBox type is richLabel then this will return NULL string

    \sa setHeading
*/
QString HbGroupBox::heading( ) const
{
    Q_D( const HbGroupBox );

    if(d->mHeadingWidget && d->mGroupBoxType != GroupBoxRichLabel)
        return d->mHeadingWidget->headingText;
    return QString();
}

/*!
    @beta

    Sets whether the groupbox is collapsable or not

    If this property is true, then disclosure mechanism is On.    

    Note: collapsable property is valid only for collapsing container type.
    If collapsable property is set on simpleLabel & richLabel type groupBox, it will be ignored

    \sa setCollapsed \sa isCollapsable
*/
void HbGroupBox::setCollapsable( bool collapsable )
{
    Q_D( HbGroupBox );

    if(d->mGroupBoxType == GroupBoxCollapsingContainer){
        if(d->mHeadingWidget->collapsable  == collapsable)
        {
            return;
        }
        d->mHeadingWidget->collapsable  = collapsable;

        d->mHeadingWidget->createPrimitives();

        // make it expand otherwise groupBox can't be expanded at all, after this scenario
        if(!collapsable && d->mHeadingWidget->collapsed){
            d->mContentWidget->setVisible(true);
            HbStyle::setItemName( d->mContentWidget , "contentwidget");
            d->mHeadingWidget->collapsed  = false;            
        }
        d->mHeadingWidget->updatePrimitives();
        repolish();
    }
}

/*!
    @beta

    Returns whether the groupbox is collapsable or not

    By default, groupbox is collapsable.

    \sa setCollapsable
*/
bool HbGroupBox::isCollapsable( ) const
{
    Q_D( const HbGroupBox );
    if(d->mHeadingWidget && d->mGroupBoxType == GroupBoxCollapsingContainer)
        return d->mHeadingWidget->collapsable;
    return false;
}

/*!
    @beta

    Sets whether the groupbox collapsed or expanded

    If the groupbox is collapsed,the groupbox's content widget are hidden; 
    otherwise they will be visible

    setCollapsed on groupbox will emit signal toggled( bool ) 
    upon collapse\expand of content widget

    Only collapsable groupbox can be collapsed. (i.e)this API will not do anything 
    if groupbox is not collapsable.By default, groupbox is not collapsed.

    Note: collapsed property is valid only for collapsing container type.
    If collapsed is set on simpleLabel or richLabel type groupBox, it will be ignored

    \sa isCollapsed \sa setCollapsable
*/
void HbGroupBox::setCollapsed( bool collapsed )
{
    Q_D( HbGroupBox );
    if(d->mGroupBoxType == GroupBoxCollapsingContainer){
        if( d->mContentWidget && d->mHeadingWidget->collapsable) {
            if ( d->mHeadingWidget->collapsed == collapsed )
                return;

            d->mHeadingWidget->collapsed = collapsed;

            #ifdef HB_EFFECTS
            HbEffectInternal::add(HB_GROUPBOX_TYPE,"groupbox_expand", "expand");
            //HbEffectInternal::add(HB_GROUPBOX_TYPE,"groupbox_collapse", "collapse");
            #endif

            if ( d->mHeadingWidget->collapsed ) {
                #ifdef HB_EFFECTS
                HbEffect::start( d->mContentWidget, HB_GROUPBOX_TYPE, "collapse");  
                #endif
                HbStyle::setItemName( d->mContentWidget , "");
                d->mContentWidget->setVisible(false);
            }
            else {
                #ifdef HB_EFFECTS
                HbEffect::start( d->mContentWidget, HB_GROUPBOX_TYPE, "expand");  
                #endif
                HbStyle::setItemName( d->mContentWidget , "contentwidget");
                d->mContentWidget->setVisible(true);
            }
            d->mHeadingWidget->updatePrimitives();
            emit toggled( d->mHeadingWidget->collapsed );
        }
        repolish();
    }    
}

/*!
    @beta

    Returns whether the groupbox is collapsed or expanded

    By default, groupbox is not collapsed.
     
    \sa setCollapsed \sa setCollapsable
*/
bool HbGroupBox::isCollapsed( ) const
{
    Q_D ( const HbGroupBox );
    if(d->mGroupBoxType == GroupBoxCollapsingContainer)
        return d->mHeadingWidget->collapsed;
		
    return false;
}

/*!
    @beta

    Enables the marquee for heading if marqueeHeading is true, otherwise the 
    heading will not marquee.

    Note: marqueeHeading property is valid  only for simpleLabel type.
    If marqueeHeading is set on richLabel or collapsing container type groupBox, it will be ignored

    \sa marqueeHeading
*/
void HbGroupBox::setMarqueeHeading( bool marquee )
{
    Q_D( HbGroupBox );
    if((d->mHeadingWidget && d->mGroupBoxType == GroupBoxSimpleLabel)){
        d->mHeadingWidget->setMarqueeHeading( marquee );
    }

}

/*!
    @beta

    Returns true if marquee is enabled for  groupbox heading; 
    otherwise returns false.

    The default value is false.

    \sa setMarqueeHeading
*/
bool HbGroupBox::marqueeHeading( ) const
{
    Q_D( const HbGroupBox );
    if(d->mHeadingWidget && d->mGroupBoxType == GroupBoxSimpleLabel)
        return d->mHeadingWidget->marqueeEnabled;
		
    return false;
}


/*!
    @beta

    Sets the groupbox content widget

    Groupbox can set only one content widget at a time.
    Ownership of the content widget is transferred to groupbox.

    If \a widget to set is NULL then content is removed.

    contentWidget is valid only for richLabel & collapsing container type.
    If content Widget is set on simpleLabel type groupBox, it will be ignored

    Note:
    1 ) GroupBox will not take care of layouting/scrolling inside content widget
    2 ) If no default height is set on content widget, then Application is responsible 
    for inconsitent UI.

    \sa contentWidget
*/
void HbGroupBox::setContentWidget( HbWidget *widget )
{
    Q_D( HbGroupBox );
   
    if(!d->mContentWidget)
        d->createContentWidget();
    
    d->mContentWidget->setContentWidget(widget);

    if(d->mHeadingWidget){
        d->setGroupBoxType(GroupBoxCollapsingContainer);
    }else
        d->setGroupBoxType(GroupBoxRichLabel);

     // collapsed property is set before setContentWidget
    if ( d->mGroupBoxType == GroupBoxCollapsingContainer && d->mHeadingWidget->collapsed ) {	
        d->mContentWidget->setVisible(false);
        HbStyle::setItemName( d->mContentWidget , "");
    }
    // update content widget primitve
    d->mContentWidget->updatePrimitives();
    repolish();
}

/*!
    @beta

    Returns groupbox content widget
    
    There is no default content widget.	

    GroupBox takes care of the ownership of the content widget being set

    Note: if \li setContentWidget is called more then once,
    then this will return last set content widget

    \sa setContentWidget
*/
HbWidget* HbGroupBox::contentWidget( ) const
{
    Q_D( const HbGroupBox );
    if(d->mContentWidget && d->mGroupBoxType != GroupBoxSimpleLabel)
        return d->mContentWidget->mContent; 
    return 0;
}

/*!

    \deprecated HbGroupBox::primitive(HbStyle::Primitive)
        is deprecated.

    Returns the pointer for \a primitive passed.
    Will return NULL if \a primitive passed is invalid
*/
QGraphicsItem* HbGroupBox::primitive(HbStyle::Primitive primitive) const
{
    Q_D( const HbGroupBox );

    switch (primitive) {
        case HbStyle::P_GroupBoxHeading_icon:
        case HbStyle::P_GroupBoxHeading_text:
        case HbStyle::P_GroupBoxHeading_background:
             if(d->mHeadingWidget){
                return d->mHeadingWidget->primitive(primitive);
                }
            break;
        case HbStyle::P_GroupBoxContent_background:
            if(d->mContentWidget)
                return d->mContentWidget->primitive(primitive);
            break;
        default:
            return 0;
    }
    return 0;	
}

/*!
    \reimp
 */
void HbGroupBox::updatePrimitives()
{
    Q_D( const HbGroupBox );

    if(d->mHeadingWidget)
        d->mHeadingWidget->updatePrimitives();

    if(d->mContentWidget)
        d->mContentWidget->updatePrimitives();
}

#include "moc_hbgroupbox.cpp"
