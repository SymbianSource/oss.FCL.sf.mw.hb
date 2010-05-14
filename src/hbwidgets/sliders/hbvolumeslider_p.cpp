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

#include "hbvolumeslider_p.h"
#include "hbslider_p.h"
#include "hbnamespace_p.h"
#include "hbslidercontrol_p_p.h"
#include "hbinstance.h"
#include "hbabstractbutton.h"
#include "hbslidercontrol_p.h"
#include "hbstyleoptionslider_p.h"
#include <QGraphicsSceneEvent>
#include <QGraphicsScene>
#include <hbwidgetfeedback.h>

/*!
    Example usage:
    \code
    HbSlider *slider = new HbSlider(Qt::Horizontal);
    QList<HbSlider::SliderElement> sliderElements;
    sliderElements.append(HbSlider::DecreaseElement);
    sliderElements.append(HbSlider::TrackElement);
    sliderElements.append(HbSlider::IncreaseElement);
    sliderElements.append(HbSlider::IconElement);
    sliderElements.append(HbSlider::TextElement);
    slider->setElements(sliderElements);
    slider->setRange(0, 100);
    slider->setMajorTickInterval(50);
    slider->setTickPosition(Hb::SliderTicksLeft);
    // decreaseIcon is valid HbIcon
    slider->setIcon(HbSlider::DecreaseElement, decreaseIcon);
    // increaseIcon is valid HbIcon
    slider->setIcon(HbSlider:: IncreaseElement, increaseIcon);
    // icon is valid HbIcon
    slider->setIcon(HbSlider:: IncreaseElement,icon);
    slider->setText( "1%");
    \endcode
    @hbwidgets
    \class HbVolumeSlider
    \brief HbVolumeSlider widget provides a vertical volume slider.

    HbVolumeSlider is provided for convenience. HbVolumeSlider sets appropriate
    icons and provides volume slider specific functionality like a
    checkable mute button and appropriate elements out of the box. HbVolumeSlider
    consists of following slider elements by default:
    \li HbSlider::IncreaseElement
    \li HbSlider::TrackElement
    \li HbSlider::DecreaseElement
    \li HbSlider::IconElement

    \image html hbvolumeslider.png A volume slider.

    The elements can be changed by calling HbSlider::setElements( ) later at any time.

    \note order of element can not be changed

    By default HbVolumeSlider will have ticks on right side ( Hb::SliderTicksRight) .

    Orientation of HbVolumeSlider can not be changed. If orientation change is needed, 
    then first create HbSlider and set needed elements.

    Example usage:
    \code
    HbVolumeSlider *slider = new HbVolumeSlider( this );
    connect( slider, SIGNAL( valueChanged( int ) ), receiver, SLOT( volumeSliderChanged( int ) ) );
    connect( slider, SIGNAL( iconToggled( bool ) ), receiver, SLOT( volumeMuted( bool ) ) );
    \endcode

    Note:: if setElement is called on this slider , application is reponsible for
    inconsitent UI.
*/


class HbVolumeSliderPrivate : public HbSliderPrivate
{
    Q_DECLARE_PUBLIC( HbVolumeSlider )

public:
    HbVolumeSliderPrivate( );
    virtual ~HbVolumeSliderPrivate( );
    void init( );
    void _q_valueChanged( int value );
    void _q_muteToggled( bool state );
    bool currentValueVisible;
    // store previous value for un mute
    int previousValue;
};


HbVolumeSliderPrivate::HbVolumeSliderPrivate( )
{

}


HbVolumeSliderPrivate::~HbVolumeSliderPrivate( )
{
}


void HbVolumeSliderPrivate::init( )
{
    Q_Q( HbVolumeSlider );
    q->connect( q, SIGNAL( valueChanged( int ) ), q, SLOT( _q_valueChanged( int ) ) );
    q->connect( q, SIGNAL( iconToggled( bool ) ), q, SLOT( _q_muteToggled( bool ) ) );
    QList<HbSlider::SliderElement> elements;
    elements << HbSlider::IncreaseElement
        << HbSlider::TrackElement
        << HbSlider::DecreaseElement
        << HbSlider::IconElement;
    if( currentValueVisible )
        elements<<HbSlider::TextElement;
    setElements( elements );
    // create element because HbSlider will not create element for NULL Icon
    // and volume slider Icon is set in style
    if ( !elementItemMap.contains(HbSlider::IncreaseElement) ) {
        elementItemMap [HbSlider::IncreaseElement].item =
            q->style( )->createPrimitive( HbStyle::P_SliderElement_increase, q);
        HbStyle::setItemName( elementItemMap [HbSlider::IncreaseElement].item, "increment-icon" );
        elementItemMap[HbSlider::IncreaseElement].type = HbStyle::P_SliderElement_increase;
        elementItemMap [HbSlider::IncreaseElement].touchItem =  q->style( )->createPrimitive( HbStyle::P_SliderElement_touchincrease, q );
            // create touch area for increment
        HbStyle::setItemName( elementItemMap [HbSlider::IncreaseElement].touchItem, "increment-icon-toucharea" );
    }

    if ( !elementItemMap.contains(HbSlider::DecreaseElement) ) {
        elementItemMap[HbSlider::DecreaseElement].item =  
            q->style( )->createPrimitive( HbStyle::P_SliderElement_decrease, q );
        elementItemMap[HbSlider::DecreaseElement].type = HbStyle::P_SliderElement_decrease;
        HbStyle::setItemName( elementItemMap[HbSlider::DecreaseElement].item, "decrement-icon" );

        // create touch area for decrement
        elementItemMap[HbSlider::DecreaseElement].touchItem =  q->style( )->createPrimitive( HbStyle::P_SliderElement_touchdecrease, q ) ;
            HbStyle::setItemName( elementItemMap[HbSlider::DecreaseElement].touchItem, "decrement-icon-toucharea" );
    }
    if( !elementItemMap.contains(HbSlider::IconElement) ) {
        elementItemMap[HbSlider::IconElement].item = q->style( )->createPrimitive( HbStyle::P_SliderElement_icon,q );
        elementItemMap[HbSlider::IconElement].type = HbStyle::P_SliderElement_icon;        
        HbStyle::setItemName( elementItemMap[HbSlider::IconElement].item, "icon-icon" );
    // creating a dummy button , because slider icon 
    // need to emit all signal which is emitted by button
    // there is no need for touch item for slider icon because
    // button can act as touch area
       HbAbstractButton *sliderIcon = new HbAbstractButton( q );
       elementItemMap [HbSlider::IconElement].touchItem=sliderIcon;
       HbStyle::setItemName( sliderIcon, "icon" );
       q->connect( sliderIcon, SIGNAL( pressed( ) ), q, SIGNAL( iconPressed( ) ) );
       q->connect( sliderIcon, SIGNAL( released( ) ), q, SIGNAL( iconReleased( ) ) );
       q->connect( sliderIcon, SIGNAL( clicked( ) ), q, SIGNAL( iconClicked( ) ) );
       q->connect( sliderIcon, SIGNAL( toggled( bool ) ), q, SIGNAL( iconToggled( bool ) ) );

#ifdef HB_EFFECTS
       q->connect( sliderIcon, SIGNAL( pressed( ) ), q, SLOT( _q_startIconPressedEffect( ) ) );
       q->connect( sliderIcon, SIGNAL( released( ) ), q,SLOT( _q_startIconReleasedEffect( ) ) );
#endif
    }

    HbStyleOptionSlider option;
    q->initStyleOption( &option );
    option.sliderElementIcon = icons.value( HbSlider::IncreaseElement ); 
    q->style( )->updatePrimitive( elementItemMap [HbSlider::IncreaseElement].item,HbStyle::P_SliderElement_increase,&option );
    option.sliderElementIcon = icons.value( HbSlider::DecreaseElement ); 
    q->style( )->updatePrimitive( elementItemMap[HbSlider::DecreaseElement].item,HbStyle::P_SliderElement_decrease,&option );
    option.sliderElementIcon = icons.value( HbSlider::IconElement ); 
    q->style( )->updatePrimitive( elementItemMap[HbSlider::IconElement].item,elementItemMap[HbSlider::IconElement].type,&option );
    q->setIconCheckable( true );
    q->setFlags( QGraphicsItem::ItemIsFocusable );
    q->setProperty("changeincrementState" ,false);
    q->setProperty("changedecrementState" ,false);
    q->setProperty("state", "normal");
}

/*!
    this slot will be called when ever slider value changes
    This wil change icon of mute /unmute button based on slider value
 */
void HbVolumeSliderPrivate::_q_valueChanged( int value )
{
    Q_Q( HbVolumeSlider );
    if( q->minimum( )==value ) {
        // volume is mute
        q->setIconChecked( true );
    } else {
        // unmute volume
        q->setIconEnabled( true );
        q->setIconChecked( false );
    }
}

/*!
   This slot will be called when slider value reaches to non zero to zero
   or vice versa
 */
void HbVolumeSliderPrivate::_q_muteToggled( bool state )
{
    Q_Q( HbVolumeSlider );
    HbStyleOptionSlider option;
    q->initStyleOption( &option );
    sliderControl->setPreviousValue( false , 0 );
    if( state ) {
        previousValue=q->value( );
        if ( previousValue == q->minimum( ) ) {
            // mute via drag
            sliderControl->setPreviousValue( true , previousValue + q->singleStep( ) );
        }
        else {
            //mute by clicking on mute icon
            sliderControl->setPreviousValue( true , previousValue );
        }
        q->setValue( q->minimum( ) );
        option.state |= QStyle::State_Sunken;
        if( previousValue==q->minimum( ) ){
            q->setIconEnabled( false );
        }
    } else {
        // If Icon is toggled by pressing on it
        sliderControl->setPreviousValue( false , 0 );
        if( q->value( )==q->minimum( ) ) {
            q->setValue( previousValue );
        }
        option.state&= ~( QStyle::State_Sunken );
    }
    option.sliderElementIcon = icons.value( HbSlider::IconElement ); 
    q->style( )->updatePrimitive( elementItemMap[HbSlider::IconElement].item,HbStyle::P_SliderElement_icon,&option );

}


/*!
    The orientation is \b Qt::Vertical and the slider contains increase, track,

    decrease and icon elements by default.

    \sa setOrientation( ), setElements( )

    show current value if \a currentValueVisible is true
*/
HbVolumeSlider::HbVolumeSlider( bool currentValueVisible,QGraphicsItem *parent )
    : HbSlider( *new HbVolumeSliderPrivate, parent )
{
    Q_D( HbVolumeSlider );
    d->q_ptr = this;
    d->currentValueVisible = currentValueVisible;
    d->init( );
    // The default values for volume slider are specified by UX
    setRange( 0,100 );
    setSingleStep( 10 );
    setMajorTickInterval( 10 );
    setSliderPosition( 80 );
    setTickPosition( Hb::SliderTicksRight );
    updatePrimitives( );
}

/*!
    Destroys the volume slider.
*/
HbVolumeSlider::~HbVolumeSlider( )
{
}


/*!
    Sets whether the text element is visible.

    \sa isCurrentValueVisible( )
*/
void HbVolumeSlider::setCurrentValueVisible( bool visible )
{
    Q_D( HbVolumeSlider );
    if( visible != d->currentValueVisible ) {
        // add/delete text element
        d->currentValueVisible  = visible;
        QList<HbSlider::SliderElement> elements;
        elements << HbSlider::IncreaseElement
            << HbSlider::TrackElement
            << HbSlider::DecreaseElement
            << HbSlider::IconElement;
        if( visible ) {
            elements<<HbSlider::TextElement;
        }
        //reset all value specified by UX
        d->setElements( elements );
        setRange( 0,100 ); 
        setSingleStep( 10 );
        setMajorTickInterval( 10 );
        setSliderPosition( 80 );
        setTickPosition( Hb::SliderTicksRight );
        updatePrimitives( );
    }
}


/*!
    Returns \c true whether the text element is visible.

    The default value is \c false.

    \sa setCurrentValueVisible( )
*/
bool HbVolumeSlider::isCurrentValueVisible( ) const
{
    Q_D( const HbVolumeSlider );
    return d->currentValueVisible ;
}

/*!
    \reimp

    This is not supported in HbVolumeSlider
 */
void HbVolumeSlider::setToolTipVisible( bool /*b*/ )
{
    Q_ASSERT_X( false, "HbZoomSlider::setToolTipVisible( )", 
        "Current value display as tooltip is not supported in volume slider" );
}

/*!
    \reimp
    unmute the volume to previous store value in case unmute happens
    by clicking increment/decrement icon
 */
void HbVolumeSlider::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
    Q_D( HbVolumeSlider );
    if( d->elementItemMap.contains(HbSlider::IncreaseElement) ) {
        if ( d->elementItemMap[HbSlider::IncreaseElement].touchItem->isUnderMouse( ) ) {
            d->pressOnIncrement = true;
            HbWidgetFeedback::triggered( d->sliderControl, Hb::InstantPressed, Hb::ModifierSliderElement );
            event->accept( );
            // if volume was mute
            if( isIconChecked( ) ){
                // take to previous value and increase one step
                setValue( d->previousValue+singleStep( ) );
                // start timer for repeat action
                d->sliderControl->setRepeatAction( 
                    HbAbstractSliderControl::SliderSingleStepAdd, d->sliderControl->maximum( ) );
            } else {// simply increase by one step
                d->startIncrementing( );
            }
            setProperty("changeincrementState" ,true);
            setProperty("changedecrementState" ,false);
            setProperty("state", "pressed");
            return;
        }
    }
    if( d->elementItemMap.contains(HbSlider::DecreaseElement) ) {
        if ( d->elementItemMap[HbSlider::DecreaseElement].touchItem->isUnderMouse( ) ) {
            d->pressOnIncrement = false;
            HbWidgetFeedback::triggered( d->sliderControl, Hb::InstantPressed, Hb::ModifierSliderElement );
            event->accept( );
            if( isIconChecked( ) ){
                //set value to previous value and decrement by one step
                setValue( d->previousValue-singleStep( ) );
                if( value( )==minimum( ) ){
                    d->previousValue = value( );
                    setIconEnabled( false );
                }
                //start timer for repeat action
                d->sliderControl->setRepeatAction( 
                HbAbstractSliderControl::SliderSingleStepSub, d->sliderControl->minimum( ) );
            } else {
                d->startDecrementing( );
            }
            setProperty("changeincrementState" ,false);
            setProperty("changedecrementState" ,true);
            setProperty("state", "pressed"); 
            return;
        }
    }
   HbSlider::mousePressEvent( event );
 }

/*!
    \reimp
    stop the timer and change image for increment element /decrement element
 */
void HbVolumeSlider::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
    Q_D( HbVolumeSlider );
    HbSlider::mouseReleaseEvent( event );
    d->stopRepeatAction( );
    setProperty("state", "normal"); 
}

/*!
    \reimp
 */
QVariant HbVolumeSlider::itemChange( GraphicsItemChange change, const QVariant &value )
{
    Q_D( HbVolumeSlider );
    switch ( static_cast<HbPrivate::HbItemChangeValues>( change ) ) {
        case ItemVisibleHasChanged:
            if( !value.toBool( ) ){
                d->stopRepeatAction( );
                if( d->sliderControl ){
                    d->sliderControl->setSliderDown( false );
                }
                setProperty("changeincrementState" ,false);
                setProperty("changedecrementState" ,false);
                setProperty("state", "normal");   
            }
            break;
        default:
            break;
    }
    return HbSlider::itemChange( change, value );
}

/*!
    \reimp
 */
void HbVolumeSlider::initStyleOption( HbStyleOptionSlider *option ) const
{
    option->sliderType=HbStyleOptionSlider::VolumeType;
    HbSlider::initStyleOption( option );

}

/*!
    \reimp
 */
void HbVolumeSlider::focusInEvent( QFocusEvent* /*event*/ )
{
    Q_D( HbVolumeSlider );
    d->sliderControl->setFocus( );
}

#include "moc_hbvolumeslider_p.cpp"



