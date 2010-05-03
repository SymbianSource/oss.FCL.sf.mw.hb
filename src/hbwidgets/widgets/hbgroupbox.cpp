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

#include <hbgroupbox.h>
#include "hbgroupbox_p.h"
#include "hbgroupboxheadingwidget_p.h"
#include "hbgroupboxcontentwidget_p.h"
#include <hbstyle.h>
#include <hbstyleoption.h>

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
    mHeadingWidget( 0 )
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
    Sets the group box type
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
    @alpha
    @hbwidgets
    \class HbGroupBox

    \brief HbGroupBox shows the user that a set of controls belong together.
    
    HbGroupBox is a container that provides the following : 

    \li Heading: text only
    \li Body content: arbitrary content (any HbWidget)
    \li Disclosure mechanism: expands and collapses the body content;

    There are three types of GroupBox:

    \li Simple Label - it's only function is to show relationship between items.
    simple Label shows a heading with marquee, no disclosure mechanism, and 
    no body content. Marquee is disabled by default.Also it is not focusable. 

    Example usage: For SimpleLabel type groupbox
    \code
    // create groupBox and set only heading; without any body content
    HbGroupBox *simpleLabel = new HbGroupBox();
    simpleLabel->setHeading("Simple label groupBox comes with marquee disabled by default");
    \endcode 
    
    \image html simpleLabelgroupbox.png A SimpleLabel groupbox


    \li Rich Label - does not show a heading and all content is in the body area 
    with no marquee and no disclosure control.Body Content must describe its own behavior and layout. 

    Example usage: For RichLabel type groupbox
    \code
    // create groupBox and set only content; without any heading
    HbGroupBox *richHeading = new HbGroupBox();
    // content widget can be any HbWidget
    // layouting and interaction behaviour inside Content widget is application's responsiblity
    HbPushButton *button = new HbPushButton(HbIcon(":/icons/ovi.png"),"Ovi");
    button->setAdditionalText("Launch Ovi Music store");
    button->setOrientation(Qt::Vertical);
    button->setTextAlignment(Qt::AlignLeft);
    richHeading->setContentWidget(button);
    \endcode 

    \image html richLabelgroupbox.png A RichLabel groupbox.
    In RichLabel type, groupbox provides background for body content.

    \li Collapsing container - also allows the user to show or hide the content of the groupBox.
    It always has a heading and body content; optionally has a disclosure mechanism.
    The heading does not marquee.The collapse/expand disclosure mechanism is located 
    in the heading and is the chief utility of this type of group box.

    If disclosure mechanism is Off, then heading will appear without expand/collapse indication icon 
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
    depending on whether or not the group box is collapsed.

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
    and consumes mouse press event, then clicked signal will not get emitted from groupBox in that case.
 */

/*!
    \fn void HbGroupBox::toggled(bool)

    This signal is emitted only in case of collapsing container groupbox,
    whenever groupbox is collapsed/expanded
 */

/*!
    @alpha
    Constructs a group box with the given \a parent.
*/
HbGroupBox::HbGroupBox( QGraphicsItem *parent)
    : HbWidget(*new HbGroupBoxPrivate, parent)
{
    Q_D( HbGroupBox );
    d->q_ptr = this;
}

/*! Constructs a group box with the given \a title and \a parent.

    \deprecated HbGroupBox::HbGroupBox(const QString&, QGraphicsItem*)
        is deprecated.This version of overloaded constructor is deprecated and cease to exist in the near future    
 */
HbGroupBox::HbGroupBox(const QString &titleText, QGraphicsItem *parent )
    : HbWidget(*new HbGroupBoxPrivate, parent)
{
    Q_UNUSED(titleText);
    Q_UNUSED(parent);
    qDebug() << "this version of constructor is deprecated and will cease to exist in the near future."; 
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
    Destructs the group box.
*/
HbGroupBox::~HbGroupBox()
{
}

/*!
    @alpha
    
    Sets the group box heading

    Note: heading property is valid for simpleLabel & collapsing container type.
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
    @alpha

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
    @alpha

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
    @alpha

    Returns whether the groupbox is collapsable or not

    By default, group boxes are collapsable.

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
    @alpha

    Sets whether the groupbox collapsed or expanded

    If the groupbox is collapsed,the group box's content widget are hidden; 
    otherwise they will be visible

    setCollapsed on groupbox will emit signal toggled( bool ) 
    upon collapse\expand of content widget

    Only collapsable groupboxes can be collapsed. (i.e)this API will not do anything 
    if group box is not collapsable.By default, group boxes are not collapsed.

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
    }
}

/*!
    @alpha

    Returns whether the group box is collapsed or expanded

    By default, groupboxes are not collapsed.
     
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
    @alpha

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
    @alpha

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
    @alpha

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
    @alpha

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
    return NULL;
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
            return NULL;
    }
    return NULL;	
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

/*! 
    Sets the group box title text

    There is no default title text.

    Note: titletext property is valid for simpleLabel & collapsing container type
    If titletext is set on richLabel type groupBox, it will be ignored

    \deprecated HbGroupBox::setTitleText(const QString&)
        is deprecated.Please use HbGroupBox::setHeading(const QString&) instead

    \sa titleText
*/
void HbGroupBox::setTitleText( const QString &text )
{
    qDebug() << "This API is deprecated, please use HbGroupBox::setHeading( const QString &text ) instead.";
    setHeading(text);  
}

/*! 
    Returns title text shown on group box

    Note: If the groupBox type is RichLabel, then this will return NULL string

    \deprecated HbGroupBox::titleText() const
        is deprecated. Please use HbGroupBox::heading() const instead

    \sa setTitleText 
*/
QString HbGroupBox::titleText( ) const
{
    qDebug() << "This API is deprecated, please use HbGroupBox::heading( ) instead.";
    return heading();
}

/*! 
    Sets the group box title widget

    There is no default title widget.

    Note: 1)if user set title text after this  \li setTitleText then,title widget will be set to null .
     Either title text or title widget can be set, both cant se set.
     2) GroupBox takes ownership of titlewidget and deletes the old title widget

     \deprecated HbGroupBox::setTitleWidget(HbWidget*) 
        is deprecated. TitleWidget concept is removed. GroupBox nomore supports widget in the heading part

     \sa setTitleText \sa titleWidget
 */
void HbGroupBox::setTitleWidget( HbWidget* widget )
{
   Q_UNUSED(widget);
   qDebug() << "This API is deprecated and will cease to exist in the near future.";
}

/*! 
    Returns group box title widget

    There is no default title widget.

    Note: If title text is set, then this will return HbLabel

    \deprecated HbGroupBox::titleWidget() const
        is deprecated. TitleWidget concept is removed. GroupBox nomore supports widget in the heading part

    \sa setTitleWidget
 */
HbWidget* HbGroupBox::titleWidget( ) const
{
    qDebug() << "This API is deprecated and will cease to exist in the near future.";
    return NULL;
}

/*! 
    Returns the alignment of the group box title.

    The default alignment is Qt::AlignLeft.

    \deprecated HbGroupBox::textAlignment() const
        is deprecated. GroupBox heading will always be left aligned

    \sa Qt::Alignment
 */
Qt::Alignment HbGroupBox::textAlignment() const
{
    qDebug() << "This API is deprecated and will cease to exist in the near future.";
    return NULL;
}

/*! 
    Sets the alignment of the group box title.

    Most styles place the title at the top of the frame. The horizontal
    alignment of the title can be specified using single values from
    the following list:

    \list
    \i Qt::AlignLeft aligns the title text on the left-hand side .
    \i Qt::AlignRight aligns the title text on the right-hand side .
    \i Qt::AlignHCenter aligns the title text with the horizontal center of the group box.
    \endlist

    The default alignment is Qt::AlignLeft.

    Note: This API will not work if heading is set as widget \li setTitleWidget
    This alignment is only of heading text \li setTitleText

    \deprecated HbGroupBox::setTextAlignment(QFlags<Qt::AlignmentFlag>)
        is deprecated. GroupBox heading will always be left aligned.

    \sa Qt::Alignment
 */
void HbGroupBox::setTextAlignment(Qt::Alignment alignment)
{
    Q_UNUSED(alignment);
    qDebug() << "This API is deprecated and will cease to exist in the near future.";
}

#include "moc_hbgroupbox.cpp"
