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
#include <QCoreApplication>
#include <QApplication>

/*!
    @beta
    @hbwidgets

    \class HbPushButton

    \brief The HbPushButton class provides a push button widget, which enables the
    user to perform a command.
    
    A push button widget enables a user to perform an important command. 
    A push button has a rectangular shape and typically there is a text 
    describing its command, an icon, and a tooltip shown on the button as in the 
    following picture.
    
    \image html hbpushbutton.png A push button with a text, an icon, and a tooltip. 
    
    You can set the text and the icon of a push button with HbPushButton() 
    constructors and change them later with setText() and setIcon(). A push 
    button also can have an additional text which you can set with 
    setAdditionalText(). You can set the alignment of the text and additional text 
    with setTextAlignment() and setAdditionalTextAlignment(). The default value 
    for the alignment of the text and additional text is the horizontally and vertically 
    centered alignment. The push button layouts with \a stretched value \c true 
    and \c false (default) are the following:
    
    - Icon and text:
      - \a stretched = \c true:  The icon and the text are horizontally aligned.
          \image html buttonIconTextH.png A stretched push button with an icon and a text.
      - \a stretched = \c false: The icon and the text are vertically aligned.
          \image html buttonIconTextV.png A non-stretched push button with an icon and a text.
    - Icon, text and additional text:
      - \a stretched = \c true:  The icon and both texts are horizontally aligned in one 
      line.
          \image html buttonIconTextAdditionalTextH.png A stretched push button with an icon, a text and an additional text.
      - \a stretched = \c false: Both texts are vertically aligned and an icon is 
      horizontally aligned in relation to the texts.
          \image html buttonIconTextAdditionalTextV.png A non-stretched push button with an icon, a text and an additional text.
    - Text and additional text
      - \a stretched = \c true:  Not applicable.
      - \a stretched = \c false: Both texts are vertically aligned. You can use this 
      layout for a dialer or a virtual keypad button, for example.          
          \image html buttonTextAdditional.png A non-stretched push button with a text and an additional text.
          
    Note that a push button does not have a stretched layout by default.

    A toggle button is a special type of a push button. The push button becomes 
    a 'toggle button' -type push button, a 'toggle button' for short, if you set 
    the \c Checkable property value of a push button to \c true. A toggle button 
    can be in \c normal, \c highlighted , \c disabled, or \c latched state (see 
    state descriptions below). Normally you use a toggle button as an on-off 
    button that varies between \c normal (off) and \c latched (on) states as the 
    user presses the toggle button.

    A push button can have the following states:
    - normal: the push button does not have focus (i.e. it is not highlighted) 
    but the user can press it down.
    - highlighted: the push button has focus.
    - pressed: the push button is pressed down.
    - latched: the push button stays pressed down even though the user does not 
    press it.
    - disabled: the user cannot press down the push button.

    A push button emits the pressed(), released(), clicked() and toggled() 
    signals which are inherited from HbAbstractButton as well as 
    HbPushButton::longPress() signal when a push button is pressed for a long 
    time. You can use a push button in any container except toolbars with 
    HbToolButton objects and palettes.
    
    
    \section _usecases_hbpushbutton Using the HbPushButton class
    
    \subsection _uc_hbpushbutton_001 Creating a push button.
    
    The following code snippet creates a simple push button.
    
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,16}
    
    \subsection _uc_hbpushbutton_002 Adding buttons to the layout.
    
    The push button is derived from HbWidget so it can be added to any 
    QGraphicsLayout object for defining the layout. The following code snippet 
    adds a button with "Stop" text and an another button with an image and "Play" 
    text to a QGraphicsLinearLayout object. Note that QGraphicsLinearLayout 
    inherits QGraphicsLayout.

    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,17}

    \subsection _uc_hbpushbutton_003 Using a push button as a toggle button.
    
    The following code snippet creates a push button which is used as a toggle 
    button. The button has a text, an icon, a tooltip, and its \c Checkable 
    property value is set to \c true.

    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,30}

    \subsection _uc_hbpushbutton_004 Using a push button as an on-off button.
    
    The following code snippet creates a push button which is used as an on-off 
    button. The button has an icon, a text, and a tooltip.

    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,29}

    \subsection _uc_hbpushbutton_005 Creating a push button with an icon, a text, and an additional text.
    
    The following code snippet creates a push button with with an icon, a text 
    and, an additional text.

     \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,35}

    \subsection _uc_hbpushbutton_006 Creating a push button with a text and an additional text.
    
    The following code snippet creates a push button with a text, and an additional text.

     \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,36}

    \sa HbAbstractButton , HbToolButton
*/
    

/*!
    
    \fn int HbPushButton::type() const
 */

/*!
    \fn void HbPushButton::longPress( QPointF )
    
    This signal is emitted when the user presses the push button for a long 
    time.
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
        frameItem = HbStylePrivate::createPrimitive( HbStylePrivate::P_PushButton_background, q );
    }
    if ( !text.isNull() ) {
        if ( !textItem ) {
            textItem = HbStylePrivate::createPrimitive( HbStylePrivate::P_PushButton_text, q );
        }
    } else {
        if( textItem ){
            delete textItem;
        }
        textItem = 0;
    }
    if ( !additionalText.isNull() ) {
        if ( !additionalTextItem ) {
            additionalTextItem = HbStylePrivate::createPrimitive( HbStylePrivate::P_PushButton_additionaltext, q );
        }
    } else {
        if( additionalTextItem ){
            delete additionalTextItem;
        }
        additionalTextItem = 0;
    }

    if ( !icon.isNull() ) {
        if ( !iconItem ) {
            iconItem = HbStylePrivate::createPrimitive( HbStylePrivate::P_PushButton_icon, q);
        }
    } else {
        if( iconItem ) {
            delete iconItem;
        }
        iconItem = 0;
    }

    if( !touchArea ) {
        touchArea = HbStylePrivate::createPrimitive( HbStylePrivate::P_PushButton_toucharea, q );
        if(QGraphicsObject *ta = qgraphicsitem_cast<QGraphicsObject*>(touchArea)) {
            ta->grabGesture(Qt::TapGesture);
            q->ungrabGesture(Qt::TapGesture);
        } else {
            q->grabGesture(Qt::TapGesture);
        }
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
    Constructs a push button with \a parent.
 */
HbPushButton::HbPushButton( QGraphicsItem *parent )
    : HbAbstractButton( *new HbPushButtonPrivate, parent )
{
    Q_D( HbPushButton );
    d->initialize( );

    setProperty( "state", "normal" );
     
}

/*!
    Constructs a push button with \a text and \a parent.
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
    Constructs a push button with \a icon, \a text, and \a parent.
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
    Destructor.
 */
HbPushButton::~HbPushButton( )
{

}

/*!
    Sets the \a background shown on a push button. The background of a push 
    button can have different images for pressed and released states.

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
        HbStylePrivate::updatePrimitive( d->frameItem, HbStylePrivate::P_PushButton_background, &buttonOption );
    }
}

/*!
    Returns the background shown on a push button.

    \sa setBackground()
 */
HbIcon HbPushButton::background( ) const
{
    Q_D( const HbPushButton );
    return d->background;
}

/*!
    Sets the \a frame background shown on a push button. The frame background 
    of a push button can have the following types of frame drawers: 9-piece, 3-
    piece horizontal, 3-piece vertical, and single. The ownership is transferred 
    to the push button.

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
        HbStylePrivate::updatePrimitive( d->frameItem, HbStylePrivate::P_PushButton_background, &buttonOption );
    }
}

/*!
    Returns the background shown on a push button.

    \sa setBackground()
 */
HbFrameDrawer *HbPushButton::frameBackground( ) const
{
    Q_D( const HbPushButton );
    return ( d->backgroundFrameDrawer );
}


/*!
    Sets the \a text shown on a push button.

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
            HbStylePrivate::updatePrimitive( d->textItem, HbStylePrivate::P_PushButton_text, &buttonOption);
            if ( isEnabled() ) {
                setProperty("state", "normal");
            }
        }
        if(doPolish) {
            repolish( );
        }
    }
}


/*!
    Returns the text shown on a push button.

    \sa setText()
 */
QString HbPushButton::text( ) const
{
    Q_D( const HbPushButton );
    return d->text;
}


/*!
    Sets the additional text shown on a push button, defined by \a 
    additionalText. Additional text is only shown on the button in the following 
    cases: 
    
    - Icon, text, and additional text are horizontally aligned in one line.
    - Text and additional text are vertically aligned and icon is horizontally 
    aligned in relation to the texts.
    - Text and additional text are vertically aligned.

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
            HbStylePrivate::updatePrimitive( d->additionalTextItem, HbStylePrivate::P_PushButton_additionaltext, &buttonOption);
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
    Returns the additional text shown on a push button.

    \sa setAdditionalText()
 */
QString HbPushButton::additionalText( ) const
{
    Q_D( const HbPushButton );
    return d->additionalText;
}

/*!
    
    Sets the \a icon shown on a push button. Each icon mode can have a 
    different image.

    \sa icon()
 */
void HbPushButton::setIcon( const HbIcon &icon )
{
    Q_D(HbPushButton);

    if ( d->icon != icon ) {
        //checking for d->polished to avoid extra polish loop
        bool doPolish = (icon.isNull( ) || d->icon.isNull()) && d->polished;
        d->icon = icon;
        d->createPrimitives( );
        //updatePrimitives();
        if( d->iconItem ) {
            HbStyleOptionPushButton buttonOption;
            initStyleOption( &buttonOption );
            HbStylePrivate::updatePrimitive( d->iconItem, HbStylePrivate::P_PushButton_icon, &buttonOption );
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
    Returns the icon shown on a push button.
    
    \sa setIcon()
 */

HbIcon HbPushButton::icon( ) const
{
    Q_D( const HbPushButton );
    return d->icon;
}

/*!
    Sets the alignment for the text. The default alignment is 
    horizontally and vertically aligned.
    
    \sa textAlignment()
*/
void HbPushButton::setTextAlignment( Qt::Alignment alignment )
{
    Q_D( HbPushButton ); 

    //HbWidgetBase* textItem = static_cast<HbWidgetBase*>(d->textItem);     
    if( !d->textItem ) {
        //need to create text item if user call alignment api before setText.
        d->textItem = HbStylePrivate::createPrimitive( HbStylePrivate::P_PushButton_text, this);
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
        HbStylePrivate::updatePrimitive( 
        d->textItem, HbStylePrivate::P_PushButton_text, &buttonOption );
    }
    d->hasTextAlignment = true;
}

/*!
    Returns the text alignment for the text.
    
    \sa setTextAlignment()
*/
Qt::Alignment HbPushButton::textAlignment( ) const
{
    Q_D(const HbPushButton);
    return d->textAlignment;
}


/*!
    Sets the \a alignment for the additional text. The default alignment is 
    horizontally and vertically centered alignment.

    \sa additionalTextAlignment()
*/
void HbPushButton::setAdditionalTextAlignment( Qt::Alignment alignment )
{
    Q_D( HbPushButton ); 

    //HbWidgetBase* additionalTextItem = static_cast<HbWidgetBase*>(d->additionalTextItem); 
    if(!d->additionalTextItem) {
        //need to create text item if user call alignment api before setAdditionalText.
        d->additionalTextItem = HbStylePrivate::createPrimitive( HbStylePrivate::P_PushButton_additionaltext, this);
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
        HbStylePrivate::updatePrimitive( 
        d->additionalTextItem, HbStylePrivate::P_PushButton_additionaltext, &buttonOption );
    }
}

/*!
    Returns the text alignment for the additional text.
    
    \sa setAdditionalTextAlignment()
*/
Qt::Alignment HbPushButton::additionalTextAlignment( ) const
{
    Q_D(const HbPushButton);
    return d->additionalTextAlignment;
}

/*!
   Sets the alignment of an icon, a text and an additional text for stretched and non-
   stretched layouts of a push button.  The push button layouts with \a 
   stretched value \c true and \c false (default) are the following:
   
    - Icon and text:
      - \c true:  Icon and text are horizontally aligned.
      - \c false: Icon and text are vertically aligned.
    - Icon, text and additional text:
      - \c true:  Icon and both texts are horizontally aligned in one line.
      - \c false: Both texts are vertically aligned and icon is horizontally aligned.
    - Text and additonal text (a dialer push button case):
      - \c true:  Not applicable.
      - \c false: Both texts are vertically aligned.
    
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
    Returns \c true if the push button has streteched layout, otherwise returns 
    \c false.
    
    \sa setStretched ()
*/
bool HbPushButton::isStretched() const
{
    Q_D(const HbPushButton);
    return d->stretched;
}

/*!

    \deprecated HbPushButton::primitive(HbStyle::Primitive) is deprecated.

    
 */
QGraphicsItem *HbPushButton::primitive( HbStyle::Primitive primitive ) const
{
    Q_D( const HbPushButton );

    switch ( primitive ) {
        case HbStylePrivate::P_PushButton_background:
            return d->frameItem;
        case HbStylePrivate::P_PushButton_icon:
            return d->iconItem;
        case HbStylePrivate::P_PushButton_text:
            return d->textItem;
        case HbStylePrivate::P_PushButton_toucharea:
            return d->touchArea;
        case HbStylePrivate::P_PushButton_additionaltext:
            return d->additionalTextItem;
        case HbStylePrivate::P_PushButton_focus:
            return d->focusItem;
        default:
            return 0;
    }
}

/*!
    
 */
void HbPushButton::recreatePrimitives()
{
   Q_D( HbPushButton );
   HbWidget::recreatePrimitives();
   if ( d->frameItem ) {
       delete d->frameItem;
       d->frameItem = 0;
   }
   if ( d->iconItem ) {
       delete d->iconItem;
       d->iconItem = 0;
   }
   if ( d->textItem ) {
       delete d->textItem ;
       d->textItem = 0;
   }
   if ( d->touchArea ) {
       delete d->touchArea ;
       d->touchArea = 0;
   }
   if ( d->additionalTextItem ) {
       delete d->additionalTextItem ;
       d->additionalTextItem = 0;
   }
   if ( d->focusItem ) {
       delete d->focusItem ;
       d->focusItem = 0;
   }
   d->createPrimitives( );
   setFrameBackground( 0 );
}

/*!
    
 */
void HbPushButton::updatePrimitives()
{
    Q_D( HbPushButton );
    HbWidget::updatePrimitives( );
    HbStyleOptionPushButton buttonOption;
    initStyleOption(&buttonOption);
    //update the button color property information 
    if ( d->checkable ) { 
        if ( d->checked ) {
            setProperty( "state", "latched" );
        }
        else {
            setProperty( "state", "normal" );
        }
    }
    else{
        if ( d->down ) {
            setProperty( "state", "pressed" );
        }
        else {
            setProperty( "state", "normal" );
        }
    }
    if ( d->textItem ) {
        HbStylePrivate::updatePrimitive( d->textItem, HbStylePrivate::P_PushButton_text, &buttonOption );
    }
    if ( d->touchArea ) {
        HbStylePrivate::updatePrimitive( d->touchArea, HbStylePrivate::P_PushButton_toucharea, &buttonOption );
    }
    if( d->additionalTextItem ) {
        HbStylePrivate::updatePrimitive(
            d->additionalTextItem,HbStylePrivate::P_PushButton_additionaltext,&buttonOption );
    }
    if ( d->iconItem) {
        HbStylePrivate::updatePrimitive( d->iconItem, HbStylePrivate::P_PushButton_icon, &buttonOption );
    }
    if ( d->frameItem ) {
        HbStylePrivate::updatePrimitive( d->frameItem, HbStylePrivate::P_PushButton_background, &buttonOption );
    }
    // update will happen only for keyevents when focusItem will be visible.
    if( d->focusItem && hasFocus() && d->focusItem->isVisible() ) {
        HbStylePrivate::updatePrimitive( d->focusItem,HbStylePrivate::P_PushButton_focus, &buttonOption );
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

    Initializes the style option push button defined by \a option with the push 
    button values. This method is useful for setting the style option push button 
    values of the subclasses.
    
    \param option Style option push button to be initialized.
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
        case Qt::Key_Down:
            if( d->keyNavigation()) {
                d->navigationKeyPress = true;
            }
            break;       
    }
    HbAbstractButton::keyPressEvent(event);
}


/*!

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
            case Qt::Key_Down:
                if ( d->keyNavigation() ) {
                    d->navigationKeyPress = false;
                }
            break;       
    }
    HbAbstractButton::keyReleaseEvent( event );
}

/*!

 */
void HbPushButton::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
#ifndef HB_GESTURE_FW
    Q_D( HbPushButton );
    HbAbstractButton::mousePressEvent( event );
    HbStyleOptionPushButton buttonOption;
    initStyleOption( &buttonOption );
    if( d->frameItem ) {
        HbStylePrivate::updatePrimitive( d->frameItem, HbStylePrivate::P_PushButton_background, &buttonOption );
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
    
 */
void HbPushButton::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
    Q_D( HbPushButton );
    HbAbstractButton::mouseReleaseEvent( event );
    HbStyleOptionPushButton buttonOption;
    initStyleOption( &buttonOption );     
    if( d->frameItem ) {
        HbStylePrivate::updatePrimitive( d->frameItem, HbStylePrivate::P_PushButton_background, &buttonOption );
    }
#ifdef HB_EFFECTS
    if ( hitButton(event->pos()) ) {
        HbEffect::start( this, HB_PUSHBUTTON_TYPE, "released" );
    }
#endif
    setProperty( "state", "normal" ); 
}

/*!

 */
void HbPushButton::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
    Q_D( HbPushButton );
    HbAbstractButton::mouseMoveEvent( event );
    HbStyleOptionPushButton buttonOption;
    initStyleOption( &buttonOption );
    if( d->frameItem ) {
        HbStylePrivate::updatePrimitive( d->frameItem, HbStylePrivate::P_PushButton_background, &buttonOption );
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
        bool hit = hitButton(mapFromScene(event->mapToGraphicsScene(tap->position())));
        switch(tap->state()) {
            case Qt::GestureStarted:
                if( hit ){
#ifdef HB_EFFECTS
                    HbEffect::start( this, HB_PUSHBUTTON_TYPE, "pressed" );
#endif
                   /* if( d->checkable && !d->checked) {
                        setProperty( "state", "latched" );
                    }else if(!d->checkable) {
                        setProperty( "state", "pressed" );
                    }*/
                }
                break;
            case Qt::GestureUpdated:
                if(tap->tapStyleHint() == HbTapGesture::TapAndHold && hit) {
                    d->longPress = true;
                    emit longPress( event->mapToGraphicsScene(tap->position()) );
                }
                break;
            case Qt::GestureCanceled:
                setProperty( "state", "normal" );
                break;
            case Qt::GestureFinished:
                if( hit ){
#ifdef HB_EFFECTS
                    HbEffect::start( this, HB_PUSHBUTTON_TYPE, "released" );
#endif
                }
                /*if( d->checkable && !d->checked) {
                    setProperty( "state", "latched" );
                }else {
                    setProperty( "state", "normal" );
                }*/
                break;
            default:
                break;
        }
    }
    HbAbstractButton::gestureEvent( event );
}
#endif


/*!
    
 */
void HbPushButton::resizeEvent( QGraphicsSceneResizeEvent *event )
{
    HbAbstractButton::resizeEvent( event );
}

/*!


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

 */
void HbPushButton::focusInEvent( QFocusEvent *event )
{
    Q_D( HbPushButton );
    if( ( event->reason() == Qt::TabFocusReason || event->reason() == Qt::BacktabFocusReason ) 
        && ( d->navigationKeyPress ) ) {
        if( !d->focusItem ) {
            d->focusItem = HbStylePrivate::createPrimitive(HbStylePrivate::P_PushButton_focus, this);
        } else {
            d->focusItem->setVisible( true );
            if( hasFocus() ) {
                HbStyleOptionPushButton buttonOption;
                initStyleOption( &buttonOption );
                HbStylePrivate::updatePrimitive( d->focusItem,HbStylePrivate::P_PushButton_focus, &buttonOption );
            }            
        }
    }
    HbAbstractButton::focusInEvent( event );
}

/*!

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
  Overloaded hit detection to include touch area.
 */
bool HbPushButton::hitButton( const QPointF &pos ) const
{
    Q_D( const HbPushButton );
    QRectF compRect = d->touchArea->boundingRect( );
    compRect.translate( d->touchArea->pos() );
    return compRect.contains( pos );
}

#include "moc_hbpushbutton.cpp"

