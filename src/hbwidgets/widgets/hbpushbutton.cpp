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

#include "hbpushbutton.h"
#include "hbpushbutton_p.h"
#include "hbstyle.h"
#include "hbstyleoptionpushbutton_p.h"
#include "hbframedrawerpool_p.h"
#include "hbnamespace.h"
#ifdef HB_EFFECTS
#include "hbeffect.h"
#include "hbeffectinternal_p.h"
#endif

#ifdef HB_GESTURE_FW
#include <hbtapgesture.h>
#endif

#include <QGraphicsSceneMouseEvent>
#include <QTimer>
#include <QGraphicsItem>
#include <QDebug>


/*!
    @beta
    @hbwidgets
    \class HbPushButton

    \brief The HbPushButton widget provides a button with command.
    
    \image html hbpushbutton.png A pushbutton with text ,icon and tooltip provided . 
    
    A push button enable users to perform important commands easily.
    A push button has a rectangular shape and typically displays a text label describing its command.
    Push Button can appear in any container except toolbars where ToolButton HbToolButton is used and palettes.
    A simple push button can be created as shown in the example below.
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,16}

    A Push buttons display a textual label. Pushbutton also
    can have icon.These can be set using the constructors and changed later using setText(),setIcon( ).
    In addition to this a push button also displays additional text label.
    Additional text can be set using setAdditionalText().
    Aligning of icon , text and additional text can be changed using setStretched()true/false property.
    Default with setStretched()=false it will place icon and text vertically.

    \image html buttonIconTextV.png A pushbutton with icon ,text.

    and along with additional text will place icon horizontally to both texts ,where both text comes vertically aligned.    
    
    \image html buttonIconTextAdditionalTextV.png A pushbutton with icon ,text and additional text.

    For setStretched()=true, it will stretch layout to have icon and text horizontally,side by side.

    \image html buttonIconTextH.png A pushbutton with icon ,text.

    and along with additional text will place all of them to Horizontally in one line .
    
    \image html buttonIconTextAdditionalTextH.png A pushbutton with custom icon ,text and additional text.

    Pushbutton can have both text and additoinal text placed vertically,this can be used in case Dialer button or 
    Virtual key pad buttons with stretched = false.Pushbutton doesn't supports both text in one line 
    stretched = true is not valid.
    Pushbutton will not have stretch layout by default.
       
    \image html buttonTextAdditional.png A pushbutton with additional text and text.

    Pushbutton can have different states like normal, pressed, disabled, latched and focused.
    A push button emits the signal clicked() when it is activated by tapping it.
    Push buttons also provide less commonly used signals, for example, pressed() and
    released() and emits longPress() on long press.

    Alignment of the text within the text and additionaltext items can be set using 
    setTextAlignment() and setAdditionalText(), default text alignment is Qt::AlignHCenter | Qt::AlignVCenter

    The push button is derived from HbWidget so it can be added to any QGraphicsLayout
    instance for layouting.
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,17}

    Push button can also be used as toggle button , following is code snippet
     \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,30}

    and onOffButton as 
     \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,29}

    Push button will come up with additional text following is code snippet.
    1. Push button with icon , text and additionalText
     \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,35}

    2. Pushbutton with additionalText and text.
     \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,36}

    \sa HbAbstractButton , HbToolButton
*/
    

/*!
    \reimp
    \fn int HbPushButton::type() const
 */

/*!
    \fn void HbPushButton::longPress( QPointF )
    This signal is emitted in case of longpress.
 */

HbPushButtonPrivate::HbPushButtonPrivate() :
    textItem(0),
    additionalTextItem(0),
    touchArea(0),
    iconItem(0),
    frameItem(0),
    focusItem(0),
    backgroundFrameDrawer(0),
    longPressTimer(0),
    textAlignment(Qt::AlignHCenter | Qt::AlignVCenter ),
    additionalTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter ),
    stretched(false),
    navigationKeyPress(false),
    hasTextAlignment(false),
    hasAdditionalTextAlignment(false)
{
#ifdef HB_EFFECTS
     //HbEffectInternal::add(HB_PUSHBUTTON_TYPE,"pushbutton_pressed.fxml", "pressed");
     HbEffectInternal::add(HB_PUSHBUTTON_TYPE,"pushbutton_released", "released");
#endif
  
}

/*
  Destructor.
 */
HbPushButtonPrivate::~HbPushButtonPrivate()
{

}

/*
  createPrimitives.
 */
void HbPushButtonPrivate::createPrimitives()
{
    Q_Q(HbPushButton);

    if ( !frameItem ) {
        frameItem = q->style()->createPrimitive( HbStyle::P_PushButton_background, q );
    }
    if ( !text.isNull() ) {
        if ( !textItem ) {
            textItem = q->style()->createPrimitive( HbStyle::P_PushButton_text, q );
        }
    } else {
        if( textItem ){
            delete textItem;
        }
        textItem = 0;
    }
    if ( !additionalText.isNull() ) {
        if ( !additionalTextItem ) {
            additionalTextItem = q->style()->createPrimitive( HbStyle::P_PushButton_additionaltext, q );
        }
    } else {
        if( additionalTextItem ){
            delete additionalTextItem;
        }
        additionalTextItem = 0;
    }

    if ( !icon.isNull() ) {
        if ( !iconItem ) {
            iconItem = q->style()->createPrimitive( HbStyle::P_PushButton_icon, q);
        }
    } else {
        if( iconItem ) {
            delete iconItem;
        }
        iconItem = 0;
    }

    if( !touchArea ) {
        touchArea = q->style()->createPrimitive( HbStyle::P_PushButton_toucharea, q );
        if(QGraphicsObject *ta = qgraphicsitem_cast<QGraphicsObject*>(touchArea)) {
            ta->grabGesture(Qt::TapGesture);
            q->ungrabGesture(Qt::TapGesture);
        } else {
            q->grabGesture(Qt::TapGesture);
        }
    }

    if( !focusItem ) {
        focusItem = q->style()->createPrimitive(HbStyle::P_PushButton_focus, q);
        //focus primitive is will not be visible by default, 
        //only in case of keyboard focus events it will be Visible.
        focusItem->setVisible( false );
    }
}

void HbPushButtonPrivate::_q_handleLongKeyPress( )
{
    Q_Q( HbPushButton );
    if(!longPress) {
        longPress = true;
        emit q->longPress( q->pos( ) );
    }
}

void HbPushButtonPrivate::initialize()
{
    Q_Q( HbPushButton );
    q_ptr = q;
    createPrimitives( );
}

/*!
    @beta
    Constructs a new HbPushButton with \a parent.
 */
HbPushButton::HbPushButton( QGraphicsItem *parent )
    : HbAbstractButton( *new HbPushButtonPrivate, parent )
{
    Q_D( HbPushButton );
    d->initialize( );

    setProperty( "state", "normal" );
     
}

/*!
    @beta
    Constructs a new HbPushButton with \a text and \a parent.
 */
HbPushButton::HbPushButton( const QString &text, QGraphicsItem *parent )
    : HbAbstractButton( *new HbPushButtonPrivate, parent )
{
    Q_D( HbPushButton );
    d->text = text;
    d->initialize( );

    setProperty( "state", "normal" );
     

}

/*!
    @beta
    Constructs a new HbPushButton with \a icon, \a text and \a parent.
 */
HbPushButton::HbPushButton( const HbIcon &icon, const QString &text, QGraphicsItem *parent )
    : HbAbstractButton( *new HbPushButtonPrivate, parent )
{
    Q_D( HbPushButton );
    d->icon = icon;
    d->text = text;
    d->initialize( );

    setProperty( "state", "normal" );

}

/*!
    Destructs the push button.
 */
HbPushButton::~HbPushButton( )
{

}

/*!
    @beta
    Sets the \a background of the button.

    \note The background can contain different images for pressed and released states.

    \sa background()
 */
void  HbPushButton::setBackground( const HbIcon &background )
{
    Q_D( HbPushButton );
    if ( d->background != background ) {
        d->background = background;
        d->backgroundFrameDrawer = 0;
        HbStyleOptionPushButton buttonOption;
        initStyleOption( &buttonOption );
        style()->updatePrimitive( d->frameItem, HbStyle::P_PushButton_background, &buttonOption );
    }
}

/*!
    @beta
    Returns the background of the button.

    \sa setBackground()
 */
HbIcon HbPushButton::background( ) const
{
    Q_D( const HbPushButton );
    return d->background;
}

/*!
    @beta
    Sets the \a framebackground of the button.

    \note The background can contain different types of frame drawers(Nine piece,
    three piece or single).Ownership is transferred to the Pushbutton

    \sa frameBackground()
 */
void  HbPushButton::setFrameBackground( HbFrameDrawer *backgroundFrameDrawer )
{
    Q_D( HbPushButton );
    if ( d->backgroundFrameDrawer != backgroundFrameDrawer ) {
        d->backgroundFrameDrawer = backgroundFrameDrawer;
        d->background = HbIcon( );
        HbStyleOptionPushButton buttonOption;
        initStyleOption( &buttonOption );
        style( )->updatePrimitive( d->frameItem, HbStyle::P_PushButton_background, &buttonOption );
    }
}

/*!
    @beta
    Returns the background of the button.

    \sa setBackground()
 */
HbFrameDrawer *HbPushButton::frameBackground( ) const
{
    Q_D( const HbPushButton );
    return ( d->backgroundFrameDrawer );
}


/*!
    @beta
    Sets the \a text shown on the button.

    \sa text()
 */
void HbPushButton::setText( const QString &text )
{
    Q_D( HbPushButton );
    if ( d->text != text ) {
        bool doPolish = text.isEmpty() || d->text.isEmpty();
        d->text = text;
        d->createPrimitives( );
        //updatePrimitives();
        if( d->textItem ) {
            HbStyleOptionPushButton buttonOption;
            initStyleOption( &buttonOption );
            style( )->updatePrimitive( d->textItem, HbStyle::P_PushButton_text, &buttonOption);
        }
        if(doPolish) {
            repolish( );
        }
    }
}


/*!
    @beta
    Returns the text shown on the button.

    \sa setText()
 */
QString HbPushButton::text( ) const
{
    Q_D( const HbPushButton );
    return d->text;
}


/*!
    @beta
    Sets the \a additional text shown on the button.
    Additional text is only shown in case 
    1. with icon and text in Qt::Horizontal alignment.
    2. with additionalText and text in Qt::Vertical alignment.

    \sa additionalText()
 */
void HbPushButton::setAdditionalText( const QString &additionalText )
{
    Q_D( HbPushButton );
    if ( d->additionalText != additionalText ) {
        bool doPolish = additionalText.isEmpty() || d->additionalText.isEmpty();
        d->additionalText = additionalText;
        d->createPrimitives( );
        // updatePrimitives();
        if( d->additionalTextItem ) {
            HbStyleOptionPushButton buttonOption;
            initStyleOption( &buttonOption );
            style( )->updatePrimitive( d->additionalTextItem, HbStyle::P_PushButton_additionaltext, &buttonOption);
        }
        if( doPolish ) {
            repolish();
        }
    }
}

/*!
    @beta
    Returns the additional text shown on the button.

    \sa setAdditionalText()
 */
QString HbPushButton::additionalText( ) const
{
    Q_D( const HbPushButton );
    return d->additionalText;
}

/*!
    @beta
    Sets the \a icon shown on the button.

    \note The icon can contain images for different modes.

    \sa icon()
 */
void HbPushButton::setIcon( const HbIcon &icon )
{
    Q_D(HbPushButton);

    if ( d->icon != icon ) {
        bool doPolish = icon.isNull( ) || d->icon.isNull();
        d->icon = icon;
        d->createPrimitives( );
        //updatePrimitives();
        if( d->iconItem ) {
            HbStyleOptionPushButton buttonOption;
            initStyleOption( &buttonOption );
            style()->updatePrimitive( d->iconItem, HbStyle::P_PushButton_icon, &buttonOption );
            if ( isEnabled() ) {
                setProperty("state", "normal");
            } 

        }
        if( doPolish ) {
            repolish();
        }
    }
}

/*!
    @beta
    Returns the icon shown on the button.
    \sa setIcon()
 */

HbIcon HbPushButton::icon( ) const
{
    Q_D( const HbPushButton );
    return d->icon;
}

/*!
    @beta
    Set the text alignment for primarytext  and additional text.
    The default alignment is Qt::AlignHCenter | Qt::AlignVCenter.
    \sa textAlignment()
*/
void HbPushButton::setTextAlignment( Qt::Alignment alignment )
{
    Q_D( HbPushButton ); 

    //HbWidgetBase* textItem = static_cast<HbWidgetBase*>(d->textItem);		
    if( !d->textItem ) {
        //need to create text item if user call alignment api before setText.
        d->textItem = style( )->createPrimitive( HbStyle::P_PushButton_text, this);
    }
    //HbWidgetBasePrivate *textItem_p = HbWidgetBasePrivate::d_ptr(textItem);	
    // check for textitem and api protection flag
    if( alignment != d->textAlignment ){
        //take the alignment 
        d->textAlignment = alignment;
        //set the api protection flag 
        //HbWidgetBasePrivate::d_ptr(textItem)->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextAlign,true);
        d->hasTextAlignment = true;
        HbStyleOptionPushButton buttonOption;            
        initStyleOption( &buttonOption );
        style()->updatePrimitive( 
        d->textItem, HbStyle::P_PushButton_text, &buttonOption );
    }
    d->hasTextAlignment = true;
}

/*!
    @beta
    Returns the text alignment for primary text and additional text
    \sa setTextAlignment()
*/
Qt::Alignment HbPushButton::textAlignment( ) const
{
    Q_D(const HbPushButton);
    return d->textAlignment;
}


/*!
    @beta
    Set the text alignment for additional text.
    The default alignment is Qt::AlignHCenter | Qt::AlignVCenter.
    \sa additionalTextAlignment()
*/
void HbPushButton::setAdditionalTextAlignment( Qt::Alignment alignment )
{
    Q_D( HbPushButton ); 

    //HbWidgetBase* additionalTextItem = static_cast<HbWidgetBase*>(d->additionalTextItem);	
    if(!d->additionalTextItem) {
        //need to create text item if user call alignment api before setAdditionalText.
        d->additionalTextItem = style( )->createPrimitive( HbStyle::P_PushButton_additionaltext, this);
    }
    // check for textitem and api protection flag
    if( alignment != d->additionalTextAlignment ) {
        //take the alignment 
        d->additionalTextAlignment = alignment;
        //set the api protection flag 
        //HbWidgetBasePrivate::d_ptr(additionalTextItem)->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextAlign,true);
        d->hasAdditionalTextAlignment = true;
        HbStyleOptionPushButton buttonOption;            
        initStyleOption( &buttonOption );
        style( )->updatePrimitive( 
        d->additionalTextItem, HbStyle::P_PushButton_additionaltext, &buttonOption );
    }
}

/*!
    @beta
    Returns the text alignment for additional text
    \sa setAdditionalTextAlignment()
*/
Qt::Alignment HbPushButton::additionalTextAlignment( ) const
{
    Q_D(const HbPushButton);
    return d->additionalTextAlignment;
}

/*!
    @beta
    Sets the stretched layout for button by default it set to true. setStretched true/false.
    For the case icon and text with value false will appear icon and text vertically aligned.
    and with value true will appear icon and text horizontally aligned.
    For the case icon , text and additional text with value false will appear like 
    both text will be vertically aligned and icon will be horizontally.
    and with value true icon , text and additionaltext will come horizontally aligned in one line
    For the case text and additonaltext(dialer button case ) only value false is applicable and both texts will 
    appeare vertically 

    \sa isStretched()
*/
void HbPushButton::setStretched( bool stretched )
{
    Q_D(HbPushButton);
    if( d->stretched != stretched ) {
        d->stretched = stretched;
        repolish();
    }
}

/*!
    @beta
    Returns the whether button has streteched layout or not.
    \sa setStretched ()
*/
bool HbPushButton::isStretched() const
{
    Q_D(const HbPushButton);
    return d->stretched;
}

/*!

    \deprecated HbPushButton::primitive(HbStyle::Primitive)
        is deprecated.

    \reimp
 */
QGraphicsItem *HbPushButton::primitive( HbStyle::Primitive primitive ) const
{
    Q_D( const HbPushButton );

    switch ( primitive ) {
        case HbStyle::P_PushButton_background:
            return d->frameItem;
        case HbStyle::P_PushButton_icon:
            return d->iconItem;
        case HbStyle::P_PushButton_text:
            return d->textItem;
        case HbStyle::P_PushButton_toucharea:
            return d->touchArea;
        case HbStyle::P_PushButton_additionaltext:
            return d->additionalTextItem;
        case HbStyle::P_PushButton_focus:
            return d->focusItem;
        default:
            return 0;
    }
}

/*!
    \reimp
 */
void HbPushButton::recreatePrimitives()
{
   Q_D( HbPushButton );
   HbWidget::recreatePrimitives();

   delete d->frameItem;
   d->frameItem = 0;
   delete d->iconItem;
   d->iconItem = 0;
   delete d->textItem;
   d->textItem = 0;
   delete d->touchArea;
   d->touchArea = 0;
   delete d->additionalTextItem;
   d->additionalTextItem = 0;
   delete d->focusItem;
   d->focusItem = 0;
   
   d->createPrimitives( );
   setFrameBackground( 0 );
}
/*!
    \reimp
 */
void HbPushButton::updatePrimitives()
{
    Q_D( HbPushButton );
    HbWidget::updatePrimitives( );

    HbStyleOptionPushButton buttonOption;
    initStyleOption(&buttonOption);
    if ( d->textItem ) {
        style()->updatePrimitive( d->textItem, HbStyle::P_PushButton_text, &buttonOption );
    }
    if ( d->touchArea ) {
        style()->updatePrimitive( d->touchArea, HbStyle::P_PushButton_toucharea, &buttonOption );
    }
    if( d->additionalTextItem ) {
        style()->updatePrimitive(
            d->additionalTextItem,HbStyle::P_PushButton_additionaltext,&buttonOption );
    }
    if ( d->iconItem) {
        style()->updatePrimitive( d->iconItem, HbStyle::P_PushButton_icon, &buttonOption );
    }
    if ( d->frameItem ) {
        style()->updatePrimitive( d->frameItem, HbStyle::P_PushButton_background, &buttonOption );
    }
    // update will happen only for keyevents when focusItem will be visible.
    if( d->focusItem && hasFocus() && d->focusItem->isVisible() ) {
        style()->updatePrimitive( d->focusItem,HbStyle::P_PushButton_focus, &buttonOption );
    }
}

/*!
    \internal
 */
HbPushButton::HbPushButton(HbPushButtonPrivate &dd, QGraphicsItem *parent) :
    HbAbstractButton(dd, parent)
{
}

/*!
    \reimp
    Initializes \a option with the values from this HbPushButton. 
    This method is useful for subclasses when they need a HbStyleOptionPushButton,
    but don't want to fill in all the information themselves.
 */
void HbPushButton::initStyleOption( HbStyleOptionPushButton *option ) const
{
    Q_D( const HbPushButton );

    HbAbstractButton::initStyleOption(option);

    Q_ASSERT(option);
    option->background = d->background;
    option->text = d->text;
    option->additionalText = d->additionalText;
    option->icon = d->icon;
    option->isCheckable = d->checkable;
    option->textAlignment = d->textAlignment;
    option->additionalTextAlignment = d->additionalTextAlignment;
    option->hasTextAlignment = d->hasTextAlignment;
    option->hasAdditionalTextAlignment = d->hasAdditionalTextAlignment;
    if( option->backgroundFrameDrawer ) {
        HbFrameDrawerPool::release( option->backgroundFrameDrawer );
        option->backgroundFrameDrawer = 0;
    }
    option->backgroundFrameDrawer = ( d->backgroundFrameDrawer );
}

/*!
    \reimp
 */
void HbPushButton::keyPressEvent(QKeyEvent *event)
{
    Q_D( HbPushButton );

    switch ( event->key() ) {
        case Qt::Key_Select:
        case Qt::Key_Enter:
        case Qt::Key_Return:{
                if (!event->isAutoRepeat( ) && !d->autoRepeat && !d->longPressTimer) {
                    d->longPressTimer = new QTimer();
                    d->longPressTimer->setInterval(300);
                    connect( d->longPressTimer, SIGNAL( timeout() ), this, SLOT(_q_handleLongKeyPress()) );
                    d->longPressTimer->start();
                }else {
                    #ifdef HB_EFFECTS
                        HbEffect::start( this, HB_PUSHBUTTON_TYPE, "pressed" );
                    #endif
                }
            }
            break;
        case Qt::Key_Up:
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Down:{
                d->navigationKeyPress = true;
            }
            break;       
    }
    HbAbstractButton::keyPressEvent(event);
}


/*!
    \reimp
 */
void HbPushButton::keyReleaseEvent(QKeyEvent *event)
{
    Q_D(HbPushButton);
    switch(event->key()){
        case Qt::Key_Select:
        case Qt::Key_Enter:
        case Qt::Key_Return:{
                #ifdef HB_EFFECTS
                    HbEffect::start( this, HB_PUSHBUTTON_TYPE, "released" );
                #endif                
                if( d->longPressTimer && d->longPressTimer->isActive() ) {                    
                    d->longPressTimer->stop();
                    delete d->longPressTimer;
                    d->longPressTimer = 0;                    
                } 
            }
            break;
            case Qt::Key_Up:
            case Qt::Key_Left:
            case Qt::Key_Right:
            case Qt::Key_Down:{
                    d->navigationKeyPress = false;
                }
            break;       
    }
    HbAbstractButton::keyReleaseEvent( event );
}

/*!
    \reimp
 */
void HbPushButton::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
#ifndef HB_GESTURE_FW
    Q_D( HbPushButton );
    HbAbstractButton::mousePressEvent( event );
    HbStyleOptionPushButton buttonOption;
    initStyleOption( &buttonOption );
    if( d->frameItem ) {
        style()->updatePrimitive( d->frameItem, HbStyle::P_PushButton_background, &buttonOption );
    }
#ifdef HB_EFFECTS
    if ( hitButton(event->pos()) ) {
        HbEffect::start( this, HB_PUSHBUTTON_TYPE, "pressed" );
    }
#endif
    setProperty( "state", "pressed" );    
#else
    Q_UNUSED( event )
#endif
}

#ifndef HB_GESTURE_FW
/*!
    \reimp
 */
void HbPushButton::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
    Q_D( HbPushButton );
    HbAbstractButton::mouseReleaseEvent( event );
    HbStyleOptionPushButton buttonOption;
    initStyleOption( &buttonOption );     
    if( d->frameItem ) {
        style()->updatePrimitive( d->frameItem, HbStyle::P_PushButton_background, &buttonOption );
    }
#ifdef HB_EFFECTS
    if ( hitButton(event->pos()) ) {
        HbEffect::start( this, HB_PUSHBUTTON_TYPE, "released" );
    }
#endif
    setProperty( "state", "normal" ); 
}

/*!
    \reimp
 */
void HbPushButton::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
    Q_D( HbPushButton );
    HbAbstractButton::mouseMoveEvent( event );
    HbStyleOptionPushButton buttonOption;
    initStyleOption( &buttonOption );
    if( d->frameItem ) {
        style()->updatePrimitive( d->frameItem, HbStyle::P_PushButton_background, &buttonOption );
    }
    if (d->down) {
        setProperty( "state", "pressed" );
    } else {
        setProperty( "state", "normal" );
    }
}
#endif


#ifdef HB_GESTURE_FW
void HbPushButton::gestureEvent( QGestureEvent *event )
{
    Q_D(HbPushButton);
    if(HbTapGesture *tap = qobject_cast<HbTapGesture *>(event->gesture(Qt::TapGesture))) {
        switch(tap->state()) {
        case Qt::GestureStarted:{
#ifdef HB_EFFECTS
                HbEffect::start( this, HB_PUSHBUTTON_TYPE, "pressed" );
#endif
                if( d->checkable && !d->checked) {
                    setProperty( "state", "latched" );
                }else if(!d->checkable) {
                    setProperty( "state", "pressed" );
                }
            }
            break;
        case Qt::GestureUpdated:
            if(tap->tapStyleHint() == HbTapGesture::TapAndHold) {
                d->longPress = true;
                emit longPress( event->mapToGraphicsScene(tap->position()) );
            }
            break;
        case Qt::GestureCanceled:
            setProperty( "state", "normal" );
            break;
        case Qt::GestureFinished:{
#ifdef HB_EFFECTS
                HbEffect::start( this, HB_PUSHBUTTON_TYPE, "released" );
#endif
                if( d->checkable && !d->checked) {
                    setProperty( "state", "latched" );
                }else {                    
                    setProperty( "state", "normal" );
                }

            }
            break;
        default:
            break;
        }
    }

    HbAbstractButton::gestureEvent( event );

}
#endif


/*!
    \reimp
 */
void HbPushButton::resizeEvent( QGraphicsSceneResizeEvent *event )
{
    HbAbstractButton::resizeEvent( event );
}
/*!
  reimp

*/
void HbPushButton::polish( HbStyleParameters &params )
{
    Q_D( HbPushButton );
    if( d->stretched && ( d->textItem && d->additionalTextItem && !d->iconItem ) ) {
        d->stretched = false;
        qWarning() << "Warning::Invalid Layout: Text and additonalText horizontal appearance not supported";
    }
    HbAbstractButton::polish( params );
}


/*!
    \reimp
 */
void HbPushButton::focusInEvent( QFocusEvent *event )
{
    Q_D( HbPushButton );
    if( ( event->reason() == Qt::TabFocusReason || event->reason() == Qt::BacktabFocusReason ) 
        && ( d->navigationKeyPress ) ) {
        if( d->focusItem ) {
            d->focusItem->setVisible( true );
            if( hasFocus() ) {
                HbStyleOptionPushButton buttonOption;
                initStyleOption( &buttonOption );
                style()->updatePrimitive( d->focusItem,HbStyle::P_PushButton_focus, &buttonOption );
            }
        } else {
            d->focusItem = style()->createPrimitive(HbStyle::P_PushButton_focus, this);
        }
    }
    HbAbstractButton::focusInEvent( event );
}

/*!
    \reimp
 */
void HbPushButton::focusOutEvent( QFocusEvent *event )
{
    Q_D( HbPushButton );
    if( d->focusItem ) {
        d->focusItem->setVisible( false );
    }
    HbAbstractButton::focusOutEvent( event );
}

/*!
    \reimp
 */
QVariant HbPushButton::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch ( change ) {
        case ItemEnabledHasChanged:
        case ItemVisibleChange: {
                updatePrimitives( );
            }
            break;
        case ItemSceneHasChanged: {
                updatePrimitives();
            }
            break;
        default:
            break;
    }
    return HbAbstractButton::itemChange( change, value );
}

/*!
  Overloaded hit detection to include touch area
 */
bool HbPushButton::hitButton( const QPointF &pos ) const
{
    Q_D( const HbPushButton );
    QRectF compRect = d->touchArea->boundingRect( );
    compRect.translate( d->touchArea->pos() );
    return compRect.contains( pos );
}

#include "moc_hbpushbutton.cpp"

