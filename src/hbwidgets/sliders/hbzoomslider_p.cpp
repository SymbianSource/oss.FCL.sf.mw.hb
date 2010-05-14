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

#include "hbzoomslider_p.h"
#include "hbslider_p.h"
#include "hbnamespace_p.h"
#include "hbinstance.h"
#include "hbwidget.h"
#include "hbslidercontrol_p.h"
#include <QGraphicsSceneEvent>
#include <QGraphicsScene>

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
    \class HbZoomSlider
    \brief HbZoomSlider widget provides a vertical zoom slider.

    HbZoomSlider is provided for convenience. HbZoomSlider sets appropriate
    icons, properties and provides zoom slider specific functionality like a
    zoom level indicator label and appropriate elements out of the box.
    HbZoomSlider sets HbSlider::text  and consists of following
    slider elements by default:
    \li HbSlider::IncreaseElement
    \li HbSlider::TrackElement
    \li HbSlider::DecreaseElement
    \li HbSlider::TextElement

    \image html hbzoomslider.png A vertical zoom slider.

    The elements can be changed by calling HbSlider::setElements( ) later at any time.

    \note order of element can not be changed

    Orientation of HbZoomSlider can not be changed. If orientation change is need, 
    then first create HbSlider and set needed elements.

    Note:: if setElement is called on this slider , application is reponsible for
    inconsitent UI.
*/


class HbZoomSliderPrivate : public HbSliderPrivate
{
    Q_DECLARE_PUBLIC( HbZoomSlider )

public:
    HbZoomSliderPrivate( );
    virtual ~HbZoomSliderPrivate( );
    void init( );
    // zoom slider can have default level set by Apps
    bool mDefaultSet;
    int mDefaultZoomLevel;
    void _q_resetToDefault( );
};


HbZoomSliderPrivate::HbZoomSliderPrivate( )
{
 
}


HbZoomSliderPrivate::~HbZoomSliderPrivate( )
{
}

/*!
    creates slider element and set default values
 */
void HbZoomSliderPrivate::init( )
{
    Q_Q( HbZoomSlider );
    mDefaultSet = false;

    QList<HbSlider::SliderElement> elements;
    elements << HbSlider::IncreaseElement 
        << HbSlider::TrackElement 
        << HbSlider::DecreaseElement 
        << HbSlider::TextElement;
    setElements( elements );

     // create element because HbSlider will not create element for NULL Icon
    // and zoom slider Icon is set in style

    if ( !elementItemMap.contains(HbSlider::IncreaseElement) ) {
        elementItemMap[HbSlider::IncreaseElement].item = 
            q->style( )->createPrimitive( HbStyle::P_SliderElement_increase, q);
        HbStyle::setItemName( elementItemMap[HbSlider::IncreaseElement].item, "increment-icon" );
        elementItemMap[HbSlider::IncreaseElement].type= HbStyle::P_SliderElement_increase;
        elementItemMap[HbSlider::IncreaseElement].touchItem = 
            q->style( )->createPrimitive( HbStyle::P_SliderElement_touchincrease,q );
        HbStyle::setItemName( elementItemMap[HbSlider::IncreaseElement].touchItem, "increment-icon-toucharea" );
    }

    if ( !elementItemMap.contains(HbSlider::DecreaseElement) ) {
        elementItemMap[HbSlider::DecreaseElement].item = 
            q->style( )->createPrimitive( HbStyle::P_SliderElement_decrease, q);
        elementItemMap[HbSlider::DecreaseElement].type = HbStyle::P_SliderElement_decrease;
        HbStyle::setItemName( elementItemMap[HbSlider::DecreaseElement].item, "decrement-icon" );
        elementItemMap[HbSlider::DecreaseElement].touchItem = q->style( )->createPrimitive( HbStyle::P_SliderElement_touchdecrease,q ) ;
        HbStyle::setItemName( elementItemMap[HbSlider::DecreaseElement].touchItem, "decrement-icon-toucharea" );
    }

    HbStyleOptionSlider option;
    q->initStyleOption( &option );
    option.sliderElementIcon = icons.value( HbSlider::IncreaseElement ); 
    q->style( )->updatePrimitive(elementItemMap[HbSlider::IncreaseElement].item ,HbStyle::P_SliderElement_increase,&option );
    option.sliderElementIcon = icons.value( HbSlider::DecreaseElement ); 
    q->style( )->updatePrimitive(elementItemMap[HbSlider::DecreaseElement].item ,HbStyle::P_SliderElement_decrease,&option );
    q->setFlags( QGraphicsItem::ItemIsFocusable );
    q->connect( q, SIGNAL( textClicked( ) ), q, SLOT( _q_resetToDefault( ) ) );
    q->setProperty("changeincrementState" ,false);
    q->setProperty("changedecrementState" ,false);
    q->setProperty("state", "normal");
}

/*!
    This slot will be called when ever there is click on text
    this will set zoom level to default if specified by Application
 */
void HbZoomSliderPrivate::_q_resetToDefault( )
{
    Q_Q( HbZoomSlider );
    if( mDefaultSet ) {
        q->setValue( q->defaultZoomLevel( ) );
    }
}


/*!   
    Constructs a zoom slider with \a parent.

    The orientation is \b Qt::Vertical and the slider contains increase, track, 
    decrease and text elements by default.
    \sa setOrientation( ), setElements( )
*/

HbZoomSlider::HbZoomSlider( QGraphicsItem *parent )
    : HbSlider( *new HbZoomSliderPrivate, parent )
{
    Q_D( HbZoomSlider );
    d->q_ptr = this;
    d->init( );
    // UX specifies that the default step value for zoom slider should be 10
    setSingleStep( 10 );
}

/*!
    Destroys the zoom slider.
*/
HbZoomSlider::~HbZoomSlider( )
{
}


/*!
    Returns the default Zoom Level of the zoom slider if set by application
    otherwise returns minimum value. 

    \sa setDefaultZoomLevel( )

*/
int HbZoomSlider::defaultZoomLevel( ) const
{
    Q_D( const HbZoomSlider );
    if( d->mDefaultSet ) {
         return d->mDefaultZoomLevel;
    } else {
         return minimum( );
    }
}


/*!   
    sets the default Zoom Level of the zoom slider with \a value.
    if not set minimum( ) value will be used for default value.

    \sa defaultZoomLevel( )
*/
void HbZoomSlider::setDefaultZoomLevel( int value )
{
    Q_D( HbZoomSlider );
    d->mDefaultSet=true;
    //bound check
    if( value > minimum( ) && value < maximum( ) ) {
        d->mDefaultZoomLevel=value;
    }
}


/*!   
    \reimp
    This is not supported in HbZoomSlider
 */
void HbZoomSlider::setToolTipVisible( bool /*b*/ )
{
    Q_ASSERT_X( false, "HbZoomSlider::setToolTipVisible( )", 
        "Current value display as tooltip is not supported in zoom slider" );

}

/*!
    \reimp
 */
void HbZoomSlider::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
    Q_D( HbZoomSlider );
    if( d->elementItemMap.contains(HbSlider::IncreaseElement) ) {
        if ( d->elementItemMap[HbSlider::IncreaseElement].touchItem->isUnderMouse( ) ) {
            setProperty("changeincrementState" ,true);
            setProperty("changedecrementState" ,false);
            setProperty("state", "pressed"); 
        }
    }
    if( d->elementItemMap.contains(HbSlider::DecreaseElement) ) {
        if ( d->elementItemMap[HbSlider::DecreaseElement].touchItem->isUnderMouse( ) ) {
            setProperty("changeincrementState" ,false);
            setProperty("changedecrementState" ,true);
            setProperty("state", "pressed"); 
        }
    }
   
    HbSlider::mousePressEvent( event );
}

/*!
    \reimp

 */
void HbZoomSlider::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
    Q_D( HbZoomSlider );
    HbSlider::mouseReleaseEvent( event );
    d->stopRepeatAction( );
    setProperty("changeincrementState" ,false);
    setProperty("changedecrementState" ,false);
    setProperty("state", "normal"); 
}

/*!
    \reimp
 */
QVariant HbZoomSlider::itemChange( GraphicsItemChange change, const QVariant &value )
{
    Q_D( HbZoomSlider );
    switch ( static_cast<HbPrivate::HbItemChangeValues>( change ) ) {
      case ItemVisibleHasChanged:
            if( !value.toBool( ) ){
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
void HbZoomSlider::initStyleOption( HbStyleOptionSlider *option ) const
{
    option->sliderType=HbStyleOptionSlider::ZoomType;
    HbSlider::initStyleOption( option );

}

/*!
    \reimp
 */
void HbZoomSlider::focusInEvent( QFocusEvent *event )
{
    Q_UNUSED( event );
    Q_D( HbZoomSlider );
    d->sliderControl->setFocus( );
}

#include "moc_hbzoomslider_p.cpp"

