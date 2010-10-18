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

#include "hbslider.h"
#include "hbslider_p.h"
#include "hbslidercontrol_p.h"
#include "hbslidercontrol_p_p.h"
#include "hbnamespace_p.h"
#include "hbsliderhandle_p.h"
#include "hbstyle_p.h"
#include "hbstyleoptionslider_p.h"
#include "hbslidertickmarks_p.h"
#include "hbslidertickmarkslabel_p.h"
#include "hbabstractbutton.h"
#include <hbwidgetfeedback.h>
#include <QGraphicsItem>
#include <QGraphicsSceneEvent>
#include <QGraphicsScene>
#include <QDebug>
#ifdef HB_GESTURE_FW 
#include <hbtapgesture.h>
#endif 
#ifdef HB_EFFECTS
#include "hbeffect.h"
#include "hbeffectinternal_p.h"
#define HB_SLIDER_TYPE "HB_SLIDER"
#endif



/*!
    @beta
    @hbwidgets
    \class HbSlider
    \brief HbSlider widget provides a generic vertical or horizontal slider.

    HbSlider is a classic widget for controlling a bounded value.
    It lets the user move a slider handle along a horizontal or vertical
    track and translates the handle's position into an integer value
    within the legal range.

    \image html hbslider.png A horizontal slider.

    The most important functions are setValue( ) to set the slider directly
    to some value; and setMinimum( ) and setMaximum( ) to define the range
    of the slider.

    HbSlider consists of elements. The available slider elements are
    defined by the HbSlider::SliderElement enumeration. A set of elements
    can be set by setSliderElements( ) and currently existing elements can be
    queried by sliderElements( ).All elements are optional
    Example usage:
    \code
    HbSlider *slider = new HbSlider( );
    QList<QVariant> elements;
    elements << QVariant (HbSlider::IncreaseElement)
        << QVariant (HbSlider::TrackElement)
        << QVariant (HbSlider::DecreaseElement)
        << QVariant (HbSlider::IconElement)
        << QVariant (HbSlider::TextElement);
    slider->setSliderElements( elements );
    QMap<QString , QVariant> map;
    // Icon path should be valid
    map.insert(QString("IncreaseElement") , QVariant(":/unittest_hbslider/star_on.png"));
    map.insert(QString("DecreaseElement") , QVariant(":/unittest_hbslider/star_on2.png"));
    map.insert(QString("IconElement") , QVariant(":/unittest_hbslider/star_on3.png"));
    slider->setElementIcons( map );
    \endcode

    \image html verticalslider.png A Custom vertical slider with all elements.

    order in which HbSlider::SliderElement are inserted into QList does not matter.

    HbSlider provides methods for controlling tickmarks.  You can use
    setTickPosition( ) to indicate where you want the tickmarks to be,
    setMajorTickInterval( ) and setMinorTickInterval( ) to indicate how
    many of them you want. The current set tick position and intervals
    can be queried using the tickPosition( ), majorTickInterval( ) and
    minorTickInterval( ) functions, respectively.

    HbSlider provides only integer ranges. Note that although
    HbSlider handles very large numbers, it becomes difficult for users
    to use a slider accurately for very large ranges.

    HbSlider also provide functionality of filling groove ( like progress )
    of slider. However this functionality is configurable and can be changed by
    setTrackFilled(), By Default this will be enabled

    The slider handles keys in the following way:
    \li Left/Right moves a horizontal slider by one single step.
    \li Up/Down moves a vertical slider by one single step.

    Example usage:
    \dontinclude slider/main.cpp
    \skip Start of snippet 1
    \until End of snippet 1

    \sa HbZoomSlider, HbVolumeSlider
*/

/*!
    \fn void HbSlider::valueChanged( int value )

    This signal is emitted when the slider value has changed.

    \sa hasTracking( )
*/

/*!
    \fn void HbSlider::sliderPressed( )

    This signal is emitted when the slider thumb has been pressed.
*/

/*!
    \fn void HbSlider::sliderReleased( )

    This signal is emitted when the slider thumb has been released.
*/

/*!
    \fn void HbSlider::sliderMoved( int value )

    This signal is emitted when the slider position has changed.

    \sa hasTracking( )
*/

/*!
    \fn void HbSlider::rangeChanged( int min, int max )

    This signal is emitted when the slider range has changed.
*/

/*!
    \fn void HbSlider::iconPressed( )

    This signal is emitted when the slider icon has been pressed.
*/

/*!
    \fn void HbSlider::iconReleased( )

    This signal is emitted when the slider icon has been released.
*/

/*!
    \fn void HbSlider::iconClicked( )

    This signal is emitted when the slider icon has been clicked.
*/

/*!
    \fn void HbSlider::iconToggled( bool checked )

    This signal is emitted when the slider icon is checkable and it has been toggled.
*/

/*!
    \reimp
    \fn int HbSlider::type( ) const
 */

/*!
    \enum HbSlider::SliderElement

    This enum defines available slider elements.
 */

/*!
    \var HbSlider::IncreaseElement

    A button to increase the slider value. Usually a plus sign.
 */

/*!
    \var HbSlider::TrackElement

    The slider track that consists of a groove and handle.
 */

/*!
    \var HbSlider::DecreaseElement

    A button to decrease the slider value. Usually a minus sign.
 */

/*!
    \var HbSlider::IconElement

    An icon element associated to the slider.For example mute icon of volume slider
 */

/*!
    \var HbSlider::TextElement

    A text element associated to the slider.
 */

/*!
    \enum HbSlider::SnappingMode

    This enum defines available SnappingModes.
 */

/*!
    \var HbSlider::NoSnapping

    slider do not snap incase of NoSnapping but by deault it will be snapped to minrotick.
 */

/*!
    \var HbSlider::MajorTickSnapping

    slider snaps to major tick incase of drag.
 */

/*!
    \var HbSlider::MinorTickSnapping

    slider snaps to minor tick incase of drag.
 */

HbSliderPrivate::HbSliderPrivate( ) :
    sliderControl( 0 ),
    orientation( Qt::Vertical ),
    pressOnIncrement( false ),
    tickmarksLeft( 0 ), // slider left/top tick mark item
    tickmarksRight( 0 ),// slider right/bottom tick mark item
    tickmarkslabelLeft( 0 ),// slider left/above tick mark label
    tickmarkslabelRight ( 0 )// slider right/bottom tick mark label

{
    elementItemMap.clear();

}

HbSliderPrivate::~HbSliderPrivate( )
{
}


void HbSliderPrivate::init( )
{
    Q_Q( HbSlider );

    sliderControl = new HbSliderControl( orientation, q );
    HbStyle::setItemName( sliderControl, "control" );
    // delegating signal of slider control to slider
    q->connect( sliderControl, SIGNAL( valueChanged( int ) ), q,  SIGNAL( valueChanged( int ) ) );
    q->connect( sliderControl, SIGNAL( sliderPressed( ) ), q, SIGNAL( sliderPressed( ) ) );
    q->connect( sliderControl, SIGNAL( sliderReleased( ) ), q, SIGNAL( sliderReleased( ) ) );
    q->connect( sliderControl, SIGNAL( sliderMoved( int ) ), q, SIGNAL( sliderMoved( int ) ) );
    q->connect( sliderControl, SIGNAL( rangeChanged( int, int ) ), q, SIGNAL( rangeChanged( int, int ) ) );
    setElements( QList<HbSlider::SliderElement>( ) << HbSlider::TrackElement );
    HbStyle::setItemName( q,"this" );
    q->setFlags( QGraphicsItem::ItemIsFocusable );
    q->setProperty("TickLabelPresent",false);
#ifdef HB_GESTURE_FW    
    q->grabGesture(Qt::TapGesture);
#endif 
}

void HbSliderPrivate::setElements( QList<HbSlider::SliderElement> elementList)
{
    Q_Q( HbSlider);
    bool oldElementRemoved = false;
    foreach(const HbSlider::SliderElement &element,elements){
        if(!elementList.contains(element)&& elementItemMap.contains(element)) {
            delete elementItemMap[element].item;
            delete elementItemMap[element].touchItem;
            elementItemMap.remove(element);
            oldElementRemoved = true;
        }
    }
    if(oldElementRemoved) {
        q->repolish( );
    }
    elements = elementList;
    // this will delete unwanted element and create new element
    updateElements( );
    // repolish call is required because new elements might be added
}

QSizeF HbSliderPrivate::getHandleSize( )
{
   return sliderControl ->getHandleSize();
}

/*!
    \internal
    This api creates widget for given element   
 */
void HbSliderPrivate::elementWidget( HbSlider::SliderElement element )
{
    Q_Q( HbSlider );
    switch ( element ) {
        case HbSlider::IncreaseElement:
            // create element only if icon is set on that
            if ( icons.contains( element) && !elementItemMap.contains(element)){
                ItemPrimitive itemPrimStrct;
                itemPrimStrct.type = HbStylePrivate::P_SliderElement_increase;
                itemPrimStrct.item = HbStylePrivate::createPrimitive(HbStylePrivate::P_SliderElement_increase,q );
                HbStyle::setItemName( itemPrimStrct.item, "increment-icon" );                    
                itemPrimStrct.touchItem = HbStylePrivate::createPrimitive( HbStylePrivate::P_SliderElement_touchincrease,q );
                HbStyle::setItemName( itemPrimStrct.touchItem, "increment-icon-toucharea" );
                HbStyleOptionSlider option;
                q->initStyleOption( &option );
                HbStylePrivate::updatePrimitive (itemPrimStrct.touchItem,HbStylePrivate::P_SliderElement_touchincrease,&option);
                elementItemMap[HbSlider::IncreaseElement]=itemPrimStrct;
                q->repolish( );
       
            }
            break;
        case HbSlider::DecreaseElement:
            // element will not be created if icon is not set on that element
            if ( icons.contains( element) && !elementItemMap.contains(element)) {
                ItemPrimitive itemPrimStrct;
                itemPrimStrct.item= HbStylePrivate::createPrimitive(HbStylePrivate::P_SliderElement_decrease,q );
                itemPrimStrct.type = HbStylePrivate::P_SliderElement_decrease;
                HbStyle::setItemName( itemPrimStrct.item,"decrement-icon" );
                itemPrimStrct.touchItem = HbStylePrivate::createPrimitive( HbStylePrivate::P_SliderElement_touchdecrease,q );
                HbStyle::setItemName( itemPrimStrct.touchItem, "decrement-icon-toucharea" );
                elementItemMap[HbSlider::DecreaseElement]=itemPrimStrct;
                HbStyleOptionSlider option;
                q->initStyleOption( &option );
                HbStylePrivate::updatePrimitive (itemPrimStrct.touchItem,HbStylePrivate::P_SliderElement_touchdecrease,&option);
                q->repolish( );
                           
            }
            break;
        case HbSlider::IconElement:
            if ( icons.contains( element) && !elementItemMap.contains(element) ) {
                    ItemPrimitive itemPrimStrct;
                    itemPrimStrct.item = HbStylePrivate::createPrimitive(HbStylePrivate::P_SliderElement_icon,q );
                    itemPrimStrct.type = HbStylePrivate::P_SliderElement_icon;
                    HbStyle::setItemName( itemPrimStrct.item, "icon-icon" );
                    HbAbstractButton *sliderIcon = new HbAbstractButton( q );
                    HbStyle::setItemName( sliderIcon, "icon" );
                    itemPrimStrct.touchItem = sliderIcon;
                    elementItemMap [HbSlider::IconElement]=itemPrimStrct;
                    // delegating signal from button to slider
                    q->connect( sliderIcon, SIGNAL( pressed( ) ), q, SIGNAL( iconPressed( ) ) );
                    q->connect( sliderIcon, SIGNAL( released( ) ), q, SIGNAL( iconReleased( ) ) );
                    q->connect( sliderIcon, SIGNAL( clicked( ) ), q, SIGNAL( iconClicked( ) ) );
                    q->connect( sliderIcon, SIGNAL( toggled( bool ) ), q, SIGNAL( iconToggled( bool ) ) );
#ifdef HB_EFFECTS
                    q->connect( sliderIcon, SIGNAL( pressed( ) ), q, SLOT( _q_startIconPressedEffect( ) ) );
                    q->connect( sliderIcon, SIGNAL( released( ) ), q,SLOT( _q_startIconReleasedEffect( ) ) );
#endif
                    q->repolish( );
               
            }
            break;
        case HbSlider::TextElement:
            if( !sliderTextString.isEmpty( ) && !elementItemMap.contains(element)){
                ItemPrimitive itemPrimStrct;
                itemPrimStrct.type = HbStylePrivate::P_SliderElement_text;
                itemPrimStrct.item = HbStylePrivate::createPrimitive( HbStylePrivate::P_SliderElement_text,q );
                HbStyle::setItemName( itemPrimStrct.item,"text_item_element" );
                HbAbstractButton *sliderTextButton = new HbAbstractButton( q ); 
                HbStyle::setItemName( sliderTextButton, "text_element" );
                itemPrimStrct.touchItem = sliderTextButton;   
                q->connect( sliderTextButton, SIGNAL( clicked( ) ), q, SIGNAL( textClicked( ) ) );
#ifdef HB_EFFECTS
                q->connect( sliderTextButton, SIGNAL( clicked( ) ), q, SLOT( _q_startTextClickEffect( ) ) );
#endif
                q->repolish();
                q->setProperty("state", "normal"); 
                elementItemMap[HbSlider::TextElement]=itemPrimStrct;
              }
            break;
        default:
            break;
    }

     HbStyleOptionSlider option;
     q->initStyleOption( &option );
     option.sliderElementIcon =icons.value(element ); 
     option.text = sliderTextString; 
     if (elementItemMap.contains(element)) {
         HbStylePrivate::updatePrimitive( elementItemMap[element].item,elementItemMap[element].type,&option );

     }  //return created;
}


/*!
    \internal
    update element will first delete element which are not required
    and then create new element
 */
void HbSliderPrivate::updateElements( )
{
    foreach ( const HbSlider::SliderElement &element, elements ) {
        // create new element, this api will not create element widget
        // if it is already existing
        elementWidget( element );
    }
}

/*!
    \internal
     This Api will start incrementing slider value by step size
     till the time stopReapetAction( ) is not being called or slider
     value becomes maximum( )
 */
void HbSliderPrivate::startIncrementing( )
{
#ifdef HB_EFFECTS
    HbEffectInternal::add( HB_SLIDER_TYPE,"slidericon_press", "increasepressed" );
    HbEffect::start( elementItemMap[HbSlider::IncreaseElement].item, HB_SLIDER_TYPE, "increasepressed" );  
#endif
    // increase slider value by step size
    sliderControl->triggerAction( HbAbstractSliderControl::SliderSingleStepAdd );
    // start timer to increase slider value by step
    sliderControl->setRepeatAction( HbAbstractSliderControl::SliderSingleStepAdd,sliderControl->maximum( ) );
}

/*!
    \internal
     This Api will start decrementing slider value by step size
     till the time stopReapetAction( ) is not being called or slider
     value becomes minimum( )
 */
void HbSliderPrivate::startDecrementing( )
{
#ifdef HB_EFFECTS
    HbEffectInternal::add( HB_SLIDER_TYPE,"slidericon_press", "decreasepressed" );
    HbEffect::start( elementItemMap[HbSlider::DecreaseElement].item, HB_SLIDER_TYPE, "decreasepressed" );  
#endif
    sliderControl->triggerAction( HbAbstractSliderControl::SliderSingleStepSub );
    sliderControl->setRepeatAction( HbAbstractSliderControl::SliderSingleStepSub,sliderControl->minimum( ) );
}

/*!
    \internal
    stops the reapeating action
    reapeat action can be increase slider value by step or decrease slider value by step
 */
void HbSliderPrivate::stopRepeatAction( )
{
    sliderControl->setRepeatAction( HbAbstractSliderControl::SliderNoAction ); 
}


/*!
    \internal
    creates either left tickmarks or right tickmarks or both based on tickposition of slider
 */

void HbSliderPrivate::createTickMarks( )
{
    Q_Q ( HbSlider);
    bool modified = false;
    if( q->majorTickInterval( ) <= 0) {
        return;
    }
    if ( q->tickPosition( ) & Hb::SliderTicksLeft) {
        if (!tickmarksLeft) {
            tickmarksLeft =  new HbSliderTickmarks(q);
            tickmarksLeft->setZValue(sliderControl->zValue()-1);
            tickmarksLeft->setTickPosition (Hb::SliderTicksLeft);
            modified = true;
            if ( orientation == Qt::Vertical ) {
                HbStyle::setItemName( tickmarksLeft, "tick-marksleft" );
            } else {
                HbStyle::setItemName( tickmarksLeft, "tick-marksabove" );
            }
        } 

    } else if ( tickmarksLeft ) {
        delete tickmarksLeft;
        tickmarksLeft = 0;
        modified = true;
    }
    if ( q->tickPosition( ) & Hb::SliderTicksRight) {
        if (!tickmarksRight) {
            tickmarksRight =  new HbSliderTickmarks(q);
            tickmarksRight->setTickPosition (Hb::SliderTicksRight);
            tickmarksRight->setZValue(sliderControl->zValue()-1);
            modified = true;

            if ( orientation == Qt::Vertical ) {
                HbStyle::setItemName( tickmarksRight, "tick-marksright" );
            } else {
                HbStyle::setItemName( tickmarksRight, "tick-marksbelow" );
            }
        }
    } else if ( tickmarksRight ) {
        delete tickmarksRight;
        tickmarksRight = 0;
        modified = true;
    }
    if ( orientation == Qt::Vertical ) {
        if ( q->tickPosition( ) & Hb::SliderTicksAbsolute)  {
            q->setLayoutDirection (Qt::LeftToRight );
        } else {
            q->unsetLayoutDirection( );
        }
    } else {
        q->unsetLayoutDirection( );   
    }
    
    if ( modified) {
        q->repolish( );
    }
}

/*!
    \internal
    creates either left tickmarks or right tickmarksLabel or both based on tickposition of slider
   
 */
void HbSliderPrivate::createTickLabels( )
{   
    Q_Q(HbSlider);
    bool modified = false;
    if( q->majorTickInterval( ) <= 0 || q->majorTickLabels( ).isEmpty( ) ) {
        return;
    }
    if ( q->tickPosition( ) & Hb::SliderTicksLeft) {
        if (!tickmarkslabelLeft) {
            modified = true;
            tickmarkslabelLeft =  new HbSliderTickmarksLabel(q);
            tickmarkslabelLeft->setTickPosition (Hb::SliderTicksLeft);
            if ( orientation == Qt::Vertical ) {
                HbStyle::setItemName( tickmarkslabelLeft, "tick-textsleft" );
            } else {
                HbStyle::setItemName( tickmarkslabelLeft, "tick-textsabove" );
            }
        }
    } else if ( tickmarkslabelLeft ) {
        modified = true;
        delete tickmarkslabelLeft;
        tickmarkslabelLeft = 0;
    }
    if ( q->tickPosition( ) & Hb::SliderTicksRight) {
        if (!tickmarkslabelRight) {
            modified = true;
            tickmarkslabelRight =  new HbSliderTickmarksLabel(q);
            tickmarkslabelRight->setTickPosition (Hb::SliderTicksRight);
            if ( orientation == Qt::Vertical ) {
                HbStyle::setItemName( tickmarkslabelRight, "tick-textsright" );
            } else {
                HbStyle::setItemName( tickmarkslabelRight, "tick-textsbelow" );
            }
        }
    } else if ( tickmarkslabelRight ) {
        delete tickmarkslabelRight;
        modified = true;
        tickmarkslabelRight = 0;
    }
    if ( orientation == Qt::Vertical ) {
        if ( q->tickPosition( ) & Hb::SliderTicksAbsolute)  {
            q->setLayoutDirection (Qt::LeftToRight );
        }
    }
    if ( modified ) {
        q->repolish( );
    }
}

/*!
    \internal
    changes the orientation of slider tick marks or label based on slider's orientation
 */
void HbSliderPrivate::setTickOrientation()
{
    Q_Q(HbSlider);
    if ( orientation == Qt::Vertical ) {
        if ( tickmarksLeft ) {
            HbStyle::setItemName( tickmarksLeft, "tick-marksleft" );
        }
        if ( tickmarksRight) {
            HbStyle::setItemName( tickmarksRight, "tick-marksright" );
        }
        if (tickmarkslabelLeft ) {
            HbStyle::setItemName( tickmarkslabelLeft, "tick-textsleft" );
        }
        if (tickmarkslabelRight ) {
            HbStyle::setItemName( tickmarkslabelRight, "tick-textsright" );
        }
        if ( q->tickPosition( )&Hb::SliderTicksAbsolute) {
            q->setLayoutDirection (Qt::LeftToRight);
        }
    } else {
        if ( tickmarksLeft ) {
            HbStyle::setItemName( tickmarksLeft, "tick-marksabove" );
        }
        if ( tickmarksRight) {
            HbStyle::setItemName( tickmarksRight, "tick-marksbelow" );
        }
        if (tickmarkslabelLeft ) {
            HbStyle::setItemName( tickmarkslabelLeft, "tick-textsabove" );
        }
        if (tickmarkslabelRight ) {
            HbStyle::setItemName( tickmarkslabelRight, "tick-textsbelow" );
        }
        q->unsetLayoutDirection( );


    }

}





/*!
  \internal
  Updates tick and Label.
  */
void HbSliderPrivate::updateTickMarks( )
{
    if (tickmarksLeft) {
        tickmarksLeft->createTicks();
        tickmarksLeft->updateTicks();
    }
    if (tickmarksRight) {
        tickmarksRight->createTicks();
        tickmarksRight->updateTicks();
    }
}


/*!
  \internal
  Updates tick and Label.
  */
void HbSliderPrivate::updateTickLabels( )
{
    if( tickmarkslabelLeft ) {
        tickmarkslabelLeft->updateTickLabels();
    }
    if( tickmarkslabelRight ) {
        tickmarkslabelRight->updateTickLabels( );
    }
}


/*!
  \internal
  deletes slidertickmarks.
  */
void HbSliderPrivate::deleteTickMarks( )
{
    if (tickmarksLeft) {
        HbStyle::setItemName(tickmarksLeft,QString());
        delete tickmarksLeft;
        tickmarksLeft = 0;
    }  
    if (tickmarksRight) {
        HbStyle::setItemName(tickmarksRight,QString());
        delete tickmarksRight;
        tickmarksRight = 0;
    }
}


/*!
  \internal
  deletes ticklabels.
  */
void HbSliderPrivate::deleteTickLabels( )
{
    if (tickmarkslabelLeft) {
        HbStyle::setItemName(tickmarkslabelLeft,QString());
        delete tickmarkslabelLeft;
        tickmarkslabelLeft = 0;
    }  
    if (tickmarkslabelRight) {
        HbStyle::setItemName(tickmarkslabelRight,QString());
        delete tickmarkslabelRight;
        tickmarkslabelRight = 0;
    }
 }

#ifdef HB_EFFECTS

/*!
    \internal
    start effect for on icon press
 */
void HbSliderPrivate::_q_startIconPressedEffect( )
{
    HbEffectInternal::add( HB_SLIDER_TYPE,"slidericon_press", "iconpressed" );
    HbEffect::start( elementItemMap[HbSlider::IconElement].item, HB_SLIDER_TYPE, "iconpressed" );  
}

/*!
   \internal
   start effect on icon release
 */
void HbSliderPrivate::_q_startIconReleasedEffect( )
{
    HbEffectInternal::add( HB_SLIDER_TYPE,"slidericon_release", "iconrelease" );
    HbEffect::start( elementItemMap[HbSlider::IconElement].item, HB_SLIDER_TYPE, "iconrelease" );  
}

/*!
   start text click effect
 */
void HbSliderPrivate::_q_startTextClickEffect( )
{
    HbEffectInternal::add( HB_SLIDER_TYPE,"slidertext_click", "textclick" );
    HbEffect::start( elementItemMap[HbSlider::TextElement].item, HB_SLIDER_TYPE, "textclick" );  
}
#endif

/*!
    @beta
    Constructs a slider with \a parent.

    The orientation is \b Qt::Vertical and the slider contains only track( groove and handle ) element by default.

    default range for HbSlider is 0 to 100 and singleStepSize is 1

    \sa setOrientation( ), setElements( )
*/
HbSlider::HbSlider( QGraphicsItem *parent ) :
    HbWidget( *new HbSliderPrivate, parent )
{
    Q_D( HbSlider );
    d->q_ptr = this;
    d->init( );
}

/*!
    @beta
    Constructs a slider with a \a orientation and \a parent.

    The slider contains only track( groove and handle ) element by default.

    default range for HbSlider is 0 to 100 and singleStepSize is 1

    \sa setElements( )
*/
HbSlider::HbSlider( Qt::Orientation orientation, QGraphicsItem *parent ) :
    HbWidget( *new HbSliderPrivate, parent )
{
    Q_D( HbSlider );
    d->q_ptr = this;
    d->init( );
    setOrientation( orientation );
}

/*!
    @beta
    \internal
 */
HbSlider::HbSlider( HbSliderPrivate &dd, QGraphicsItem *parent ) :
    HbWidget( dd, parent )
{
    Q_D( HbSlider );
    d->init( );
    
}

/*!
    Destructs the slider.
*/
HbSlider::~HbSlider( )
{
}

/*!
    @beta
    Returns the list of slider elements as QVariant 
    
    \note it is safe to type-cast element to HbSlider::SliderElement.  
   
    The slider contains only track element by default.

    \sa setSliderElements()
*/
QList<QVariant> HbSlider::sliderElements() const
{
    Q_D( const HbSlider );
    QList<QVariant> elementList;
    foreach(const HbSlider::SliderElement &element,d->elements){
        elementList.append ( QVariant ( element ) );
    }
    return elementList;
}

/*!
    @beta
    Sets the elements of the slider.

    \note Duplicate elements will be ignored.

    \note element order cant be changed

    Eg Usages:
    HbSlider slider;
    QList<QVariant> list;
    list.append( QVariant( HbSlider::IncreaseElement));
    list.append( QVariant( HbSlider::DecreaseElement));
    list.append( QVariant( HbSlider::IconElement));
    slider.setElements( list);

    The available slider elements are
    defined by the HbSlider::SliderElement enumeration

    \sa sliderElements()

*/
void HbSlider::setSliderElements(const QList<QVariant> &elementlist)
{
    Q_D( HbSlider );
    QList<HbSlider::SliderElement> elements;
    foreach (const QVariant &elementVar , elementlist ) {
        elements.append( static_cast<HbSlider::SliderElement>(elementVar.toInt()));
    }
    d->setElements( elements );
}

/*!
    @beta
    Sets the icons for elements

    key of \a elements is element name  and value is icon 
    name to set on given element
    Use following string for specifying elements
    "IncreaseElement" for increase element
    "DecreaseElement" for decrease element
    "IconElement" for icon element
    
    Eg Usage:
    HbSlider slider;
    QMap<QString , QVariant> map;
    map.insert(QString("IncreaseElement") , QVariant("example1.png"));
    map.insert(QString("DecreaseElement") , QVariant("example2.png"));
    slider.setElementIcons( map );

    \warning Setting icon to a non-existing element has no effect.

    \sa elementIcons() setSliderElements() setElementIcon()
*/
void HbSlider::setElementIcons(const QMap<QString,QVariant> &elements)
{
    Q_D( HbSlider );
    QMapIterator<QString,QVariant> i(elements);
    while (i.hasNext()) {
        i.next();
        HbIcon icon = HbIcon( i.value().toString());
        if ( !icon.isNull() ) {
            if( !i.key().compare("IncreaseElement")) {
                d->icons[HbSlider::IncreaseElement] = icon;
                if ( d->elements.contains( HbSlider::IncreaseElement ) ) {
                    // create icon element if it was not existing
                    d->elementWidget( HbSlider::IncreaseElement ); 
                }
            }
            else if( !i.key().compare( "DecreaseElement")) {
                d->icons[HbSlider::DecreaseElement] = icon;
                if ( d->elements.contains( HbSlider::DecreaseElement ) ) {
                    // create icon element if it was not existing
                    d->elementWidget( HbSlider::DecreaseElement ); 
                }
            }
             else if( !i.key().compare( "IconElement")) {
                d->icons[HbSlider::IconElement] = icon;
                if ( d->elements.contains( HbSlider::IconElement ) ) {
                    // create icon element if it was not existing
                    d->elementWidget( HbSlider::IconElement ); 
                }
            }
        }
    }
}

/*!
    @beta
    Returns the map , which consist of element name as key and icon name  as value

    returns NULL map if none of the element has icon

    \sa setElementIcons()
*/
QMap<QString,QVariant> HbSlider::elementIcons( )const
{
    Q_D( const HbSlider );
    QMap<QString , QVariant> map;
    QHashIterator<HbSlider::SliderElement , HbIcon> i(d->icons);
    while( i.hasNext()) {
        i.next();
        QString  name;
        switch( i.key() ){
            case IncreaseElement:
                name = QString("IncreaseElement");
                break;
            case DecreaseElement:
                name = QString("DecreaseElement");
                break;
            case IconElement:
                name = QString("IconElement");
                break;
            default:
                name = QString("");

        }
        map.insert( name, QVariant(i.value().iconName()));
    }
    return map;
}


/*!
    @beta
    Returns the slider text.

    returns empty text if text element does not exist

    \sa setText( )
*/
QString HbSlider::text( ) const
{
    Q_D( const HbSlider );
    return d->sliderTextString;
}


/*!
    @beta
    Sets the slider \a text for text element as well as tooltip.

    tooltip will be visible only if setTooltip( true ) is called.

    Slider will not take care of localization.

    \warning Setting text to a non-existing text element has no effect.
   
    \sa text( ), setToolTipVisible( ), setToolTipAlignment( )
*/
void HbSlider::setText( const QString &text )
{
    Q_D( HbSlider );
    if( d->sliderTextString!= text) {
        d->sliderTextString = text;
        if ( d->elements.contains( HbSlider::TextElement ))  {
            d->elementWidget( HbSlider::TextElement );  
        }
    }
}

/*!
    @beta
    Returns the icon of the \a element.

    returns NULL if element does not exist or icon was not set for this element

    \sa setElementIcon( )
*/
HbIcon HbSlider::elementIcon( SliderElement element ) const
{
    Q_D( const HbSlider );
    return d->icons.value( element );
}

/*!
    @beta
    Sets the \a icon if the \a element.

    Supported elements:
    \li HbSlider::IncreaseElement
    \li HbSlider::DecreaseElement
    \li HbSlider::IconElement

    \warning Setting icon to a non-existing element has no effect.

    \sa elementIcon( ) setElements( )
*/
void HbSlider::setElementIcon( SliderElement element, const HbIcon &icon )
{
    Q_D( HbSlider );
    if ( element != TextElement &&!icon.isNull()) {
        d->icons[element] = icon;
        if ( d->elements.contains( element ) ) {
            // create icon element if it was not existing
            d->elementWidget( element ); 
            // update icon primitive
            
        } 

    } else {
        qWarning( "HbSlider::setIcon: non-supported element or null icon passed" );
    }
}

/*!
    @beta
    Returns \c true whether the icon element is checkable.

    The default value is \c false.

    \sa setIconCheckable( )
*/
bool HbSlider::isIconCheckable( ) const
{
    Q_D( const HbSlider );

    if ( d->elementItemMap.contains (HbSlider::IconElement ) ){
        HbAbstractButton *sliderIcon = 
            static_cast<HbAbstractButton *>(d->elementItemMap[HbSlider::IconElement].touchItem);
        return sliderIcon->isCheckable( );
    }
    return false;
}


/*!
    @beta
    Sets whether the tooltip is visible .

    tooltip is different then current value

    tooltip text can be set by \li setText( ) api

    if \a value is true then tooltip will be display on thumb drag

    \sa isToolTipVisible( ) , setText( )
*/
void HbSlider::setToolTipVisible( bool value )
{
    Q_D( const HbSlider );
    d->sliderControl->setToolTipVisible( value );
}


/*!
    @beta
    Returns \c true if tooltip is visible.

    The default value is \c true.

    \sa setToolTipVisible( )
*/
bool HbSlider::isToolTipVisible( )const
{
    Q_D( const HbSlider );
    return d->sliderControl->isToolTipVisible( );
}



/*!
   @beta
   sets the toolTip Alignment with respect to the thumb.

    \sa toolTipAlignment( )
*/
void HbSlider::setToolTipAlignment( Qt::Alignment alignment )
{
    Q_D( HbSlider );
    d->sliderControl->setToolTipAlignment( alignment );
}


/*!
    @beta
    returns the tooltip Alignment with respect to the thumb

    \sa setToolTipAlignment( )
*/
Qt::Alignment HbSlider::toolTipAlignment( ) const
{
    Q_D( const HbSlider );
    return d->sliderControl->toolTipAlignment( );
}

/*!
    @beta
    Sets whether the icon element is checkable.

    checkable icon emits toggled( bool ) signal

    By default, the icon is not checkable.

    \sa isIconCheckable( )
*/
void HbSlider::setIconCheckable( bool checkable )
{
    Q_D( HbSlider );
    if ( d->elementItemMap.contains (HbSlider::IconElement )) {
        HbAbstractButton *sliderIcon = 
            static_cast<HbAbstractButton *>(d->elementItemMap[HbSlider::IconElement].touchItem);
        sliderIcon->setCheckable ( checkable );
    }
}

/*!
    @beta
    Returns \c true whether the icon element is checked.

    The default value is \c false.

    \sa setIconChecked( )
*/
bool HbSlider::isIconChecked( ) const
{
    Q_D( const HbSlider );
    if ( d->elementItemMap.contains (HbSlider::IconElement ) ){
        HbAbstractButton *sliderIcon = 
            static_cast<HbAbstractButton *>(d->elementItemMap[HbSlider::IconElement].touchItem);
        return sliderIcon->isChecked( );
    }
    return false;
}

/*!
    @beta
    Sets whether the icon element is checked or not.

    \sa isIconChecked( )
*/
void HbSlider::setIconChecked( bool checked )
{
    Q_D( HbSlider );
    if ( d->elementItemMap.contains (HbSlider::IconElement )) {
        HbAbstractButton *sliderIcon = 
            static_cast<HbAbstractButton *>(d->elementItemMap[HbSlider::IconElement].touchItem);
        sliderIcon->setChecked ( checked );
    }
}

/*!
    @beta
    Returns \c true whether the icon element is enabled.

    The default value is \c true.

    \sa setIconEnabled( )
*/
bool HbSlider::isIconEnabled( ) const
{
    Q_D( const HbSlider );
    if ( d->elementItemMap.contains (HbSlider::IconElement ) ) {
        HbAbstractButton *sliderIcon = 
            static_cast<HbAbstractButton *>(d->elementItemMap[HbSlider::IconElement].touchItem);
        return sliderIcon->isEnabled( );
    }
    return false;
}

/*!
    @beta
    Sets whether the icon element is enabled or not.

    \sa isIconEnabled( )
*/
void HbSlider::setIconEnabled( bool enabled )
{
    Q_D( HbSlider );
    if ( d->elementItemMap.contains (HbSlider::IconElement )) {
        HbAbstractButton *sliderIcon = 
            static_cast<HbAbstractButton *>(d->elementItemMap[HbSlider::IconElement].touchItem);
        sliderIcon->setEnabled ( enabled );
        d->elementItemMap[HbSlider::IconElement].item->setEnabled( enabled );

    }
}

/*!
    @beta
    Sets the \a orientation of the slider.

    \sa orientation( )
*/
void HbSlider::setOrientation( Qt::Orientation orientation )
{
    Q_D( HbSlider );
    if ( d->orientation != orientation ) {
        d->orientation = orientation;
        d->sliderControl->setOrientation( orientation );
        d->setTickOrientation( );
        repolish( );
    }
}

/*!
    @beta
    Returns the orientation of the slider.

    The default value is \c Qt::Vertical.

    \sa setOrientation( )
 */
Qt::Orientation HbSlider::orientation( ) const
{
    Q_D( const HbSlider );
    return d->orientation;
}

/*!
    @beta
    Returns \c true whether the slider track is inverted.

    The default value is \c false.

    If this property is \c false, the minimum and maximum will
    be shown in its classic position. If the value is \c true,
    the minimum and maximum appear at their opposite location.

    \sa setInvertedAppearance( )
*/
bool HbSlider::invertedAppearance( ) const
{
    Q_D( const HbSlider );
    return d->sliderControl->invertedAppearance( );
}

/*!
    @beta
    Sets whether the slider track is \a inverted.

    This  holds whether or not a slider shows its values inverted.

    If \a inverted. false ( the default ), the minimum and maximum will be shown in its classic position 
    
    for the inherited widget. If the value is true, the minimum and maximum appear at
    
    their opposite location.
    
    Note: This property makes most sense for sliders.

    \sa invertedAppearance( )

*/
void HbSlider::setInvertedAppearance( bool inverted )
{
    Q_D( HbSlider );
    d->sliderControl->setInvertedAppearance( inverted );
    int dec = d->elements.indexOf( HbSlider::DecreaseElement );
    int inc = d->elements.indexOf( HbSlider::IncreaseElement );
    if ( dec != -1 && inc != -1 ) {
        d->elements.swap( dec, inc );
        d->updateElements( );
    }
}

/*!
    @beta
    Returns the maximum value of the slider.

    The default value is \c 100.

    \sa setMaximum( ), setRange( )
*/
int HbSlider::maximum( ) const
{
    Q_D( const HbSlider );
    return d->sliderControl->maximum( );
}

/*!
    @beta
    Sets the maximum value of the slider.

    \note When setting this property, the minimum is adjusted if

    necessary to ensure that the range remains valid.  Also the

    slider's current value is adjusted to be within the new range.

    \sa maximum( )
*/
void HbSlider::setMaximum( int max )
{
    Q_D( HbSlider );
    d->sliderControl->setMaximum( max );
}

/*!
    @beta
    Returns the minimum value of the slider.

    The default value is \c 0.

    \sa setMinimum( )
*/
int HbSlider::minimum( ) const
{
    Q_D( const HbSlider );
    return d->sliderControl->minimum( );
}

/*!
    @beta
    Sets the minimum value of the slider.

    \note When setting this property, the maximum is adjusted if
    necessary to ensure that the range remains valid. Also the
    slider's current value is adjusted to be within the new range.

    \sa minimum( )
*/
void HbSlider::setMinimum( int min )
{
    Q_D( HbSlider );
    d->sliderControl->setMinimum( min );
}

/*!
    @beta
    This function is provided for convenience.

    Sets the slider's minimum to \a min and its maximum to \a max.

    If maximum is smaller than minimum, minimum becomes the only legal value.

    \sa setMinimum( ), setMaximum( )
*/
void HbSlider::setRange( int min, int max )
{
    Q_D( HbSlider );
    d->sliderControl->setRange( min, max );
}

/*!
    @beta
    Returns the page step of the slider.

    The default value is \c 10.

    The larger of two natural steps that a slider provides
    and typically corresponds to the user pressing PageUp or PageDown.

    \sa setPageStep( )
*/
int HbSlider::pageStep( ) const
{
    Q_D( const HbSlider );
    return d->sliderControl->pageStep( );
}

/*!
    @beta
    Sets the page \a step of the slider.

    \sa pageStep( )
*/
void HbSlider::setPageStep( int step )
{
    Q_D( HbSlider );
    d->sliderControl->setPageStep( step );
}

/*!
    @beta
    Returns the single step of the slider.

    The default value is \c 1.

    The smaller of two natural steps that an
    abstract sliders provides and typically corresponds to the user
    pressing an arrow key.

    \sa setSingleStep( )
*/
int HbSlider::singleStep( ) const
{
    Q_D( const HbSlider );
    return d->sliderControl->singleStep( );
}

/*!
    @beta
    Sets the single \a step of the slider.

    \sa singleStep( )
*/
void HbSlider::setSingleStep( int step )
{
    Q_D( HbSlider );
    d->sliderControl->setSingleStep( step );
}

/*!
    @beta
    Returns the current slider position.

    If tracking is enabled ( the default ), this is identical to the value.

    \sa setSliderPosition( )
*/
int HbSlider::sliderPosition( ) const
{
    Q_D( const HbSlider );
    return d->sliderControl->sliderPosition( );
}

/*!
    @beta
    Sets the current slider position.

    \sa sliderPosition( )
*/
void HbSlider::setSliderPosition( int pos )
{
    Q_D( HbSlider );
    d->sliderControl->setSliderPosition( pos );
}

/*!
    @beta
    Returns \c true whether slider tracking is enabled.

    The default value is \c true.

    If tracking is enabled, the slider emits the
    valueChanged( ) signal while the slider is being dragged. If
    tracking is disabled, the slider emits the valueChanged( ) signal
    only when the user releases the slider.

    \sa setTracking( )
*/
bool HbSlider::hasTracking( ) const
{
    Q_D( const HbSlider );
    return d->sliderControl->hasTracking( );
}

/*!
    @beta
    Sets whether the slider tracking is enabled.

    \sa hasTracking( )
*/
void HbSlider::setTracking( bool enable )
{
    Q_D( HbSlider );
    d->sliderControl->setTracking( enable );
}

/*!
    @beta
    Returns the current value of the slider.

    The default value is \c 0.
*/
int HbSlider::value( ) const
{
    Q_D( const HbSlider );
    return d->sliderControl->value( );
}

/*!
    @beta
    Sets the current value of the slider.

    The slider forces the value to be within the legal range: \b
    minimum <= \c value <= \b maximum.

    Changing the value also changes the \b sliderPosition.

    \sa value( )
*/
void HbSlider::setValue( int value )
{
    Q_D( HbSlider );
    d->sliderControl->setValue( value );
}

/*!
    @beta
    Returns the tickmark position of the slider.

    The default value is \c Hb::NoTicks.

    \sa setTickPosition( )
*/
Hb::SliderTickPositions HbSlider::tickPosition( ) const
{
    Q_D( const HbSlider );
    return d->sliderControl->tickPosition( );
}

/*!
    @beta
    Sets the tickmark position of the slider.

    The valid values are described by the Hb::SliderTickPositions enum.

    The default value is Hb::NoTicks.

    Note:: if major tick and minor tick interval is zero
    then no ticks will be drawn

    Example usage:
    \code
    HbSlider *slider = new HbSlider( );
    slider->setMinorTickInterval( 10 );
    slider->setTickPosition( Hb::SliderTicksRight ) ;
    \endcode

    \sa tickPosition( ), setMajorTickInterval( ) , setMinorTickInterval( )
 */
void HbSlider::setTickPosition( Hb::SliderTickPositions position )
{
    Q_D( HbSlider );
    d->sliderControl->setTickPosition( position );
    d->createTickMarks( );
    d->createTickLabels( );
    d->updateTickMarks( );
    d->updateTickLabels( );
}

/*!
    @beta
    Returns the SnappingMode of the slider.

    The default value is \c HbSlider::MinorTickSnapping.

    \sa setSnappingMode( ) , HbSlider::SnappingMode.
*/
HbSlider::SnappingMode HbSlider::snappingMode( ) const 
{
    Q_D( const HbSlider );
    return ( HbSlider::SnappingMode )( d->sliderControl->snappingMode( ) );
}

/*!
    @beta
    Sets the snappingMode of the slider.

    \sa snappingMode( )
*/
void HbSlider::setSnappingMode( SnappingMode mode )
{
    Q_D( HbSlider );    
    d->sliderControl->setSnappingMode( ( HbSliderControl::SnappingMode )mode );
}

/*!
    @beta
    Returns the interval between major tickmarks.

    The default value is \c 0.

    \sa setMajorTickInterval( )
*/
int HbSlider::majorTickInterval( ) const
{
    Q_D( const HbSlider );
    return d->sliderControl->majorTickInterval( );
}

/*!
    @beta
    Sets the interval between major tickmarks.

    absolute value will be taken for tick interval

    \default value is 0

    \sa majorTickInterval( )
*/
void HbSlider::setMajorTickInterval( int interval )
{
    Q_D( HbSlider );
    d->sliderControl->setMajorTickInterval( interval );
    if (interval <=0 ) {
        d->deleteTickMarks( );
        d->deleteTickLabels( );
    }
    else {
        d->createTickMarks( );
        d->createTickLabels( );
        d->updateTickMarks( );
        d->updateTickLabels( );
    }
}

/*!
    @beta
    Returns the interval between minor tickmarks.

    The default value is \c 0.

    \sa setMinorTickInterval( )
*/
int HbSlider::minorTickInterval( ) const
{
    Q_D( const HbSlider );
    return d->sliderControl->minorTickInterval( );
}

/*!
    @beta
    Sets the interval between minor tickmarks.

    absolute value will be taken for tick interval

    \default value is 0

    \sa minorTickInterval( )
*/
void HbSlider::setMinorTickInterval( int interval )
{
    Q_D( HbSlider );
    d->sliderControl->setMinorTickInterval( interval );
    d->updateTickMarks( );
    d->updateTickLabels( );
}

/*!
    @beta
    sets the icon path for handle 

    \sa handleIcon( )
   
*/
void HbSlider::setHandleIcon( const QString& handlePath )
{
    Q_D( HbSlider );
    d->sliderControl->setHandleIcon( handlePath );
}

/*!
    @beta
    returns icon of handle

    \sa handleIcosetHandleIconn( )
     
*/
QString HbSlider::handleIcon( ) const
{
    Q_D( const HbSlider );
    return d->sliderControl->handleIcon( );
}


/*!
    @beta
    Sets whether to display progress track or not

    \default value is true

    \sa isTrackFilled( )
*/
void HbSlider::setTrackFilled(bool trackVisible )
{
    
    Q_D( const HbSlider );
    return d->sliderControl->setTrackFilled( trackVisible );

}

/*!
    @beta
    returns whether progress track is visible or not

    \sa setTrackFilled( )
     
*/
bool HbSlider::isTrackFilled() const
{
    Q_D( const HbSlider );
    return d->sliderControl->isTrackFilled();

}

/*!
    @beta
    Returns the major ticklabels of the slider.
  
    \sa setMajorTickLabels( )
*/
QStringList HbSlider::majorTickLabels( ) const
{
    Q_D( const HbSlider );
    return d->sliderControl->majorTickLabels( );
}

/*!
    @beta
    Sets the major ticklabels of the slider.

    Detailed description:

    Strings from the stringlist will be taken sequentially for drawing
    LabelItem,there exists a one-to-one mapping between tickmarkitem &
    tickmarklabelitem.

    case 1: if strings provided > tickmarks, labels are drawn for the
    strings which maps the tickmarkitems.If there are extra strings 
    available in the list beyond 1-1 mapping, those strings will be left out.

    case 2: if strings provided < tickmarks, labels are drawn only for strings
    provided.Rest of the tickmarkitems will be drawn without labelItem.

    case 3: If some tickmarkitems to be drawn without labelItem in the middle,
    specify empty string ( "" ) for that item  in the string list.

    case 4: if both major & minor tickinterval are same,then only majortickLabel
    strings are taken into account for drawing the labelItem

    case 5: If you want to remove the whole tickLabel then set the empty string as the argument.

    Note:: if the major tickinterval of the slider is very small, the tick labels may overlap,
    application needs to take care of this.

    \sa majorTickLabels( )
*/
void HbSlider::setMajorTickLabels( const QStringList &majorTickLabels )
{
    Q_D( HbSlider );
    d->sliderControl->setMajorTickLabels( majorTickLabels );
    if(majorTickLabels.isEmpty( )) {
        d->deleteTickLabels( );
    } else {
        d->createTickLabels( );
        d->updateTickLabels( );
    }
}

/*!
    @beta
    Returns the minor ticklabels of the slider.
  
    \sa setMinorTickLabels( )
*/
QStringList HbSlider::minorTickLabels( ) const
{
    Q_D( const HbSlider );
    return d->sliderControl->minorTickLabels( );
}

/*!
    @beta
    Sets the minor ticklabels of the slider.
    see setMajorTickLabels for detailed description.
    Note:: if the minor tickinterval of the slider is very small, the tick labels may overlap,
    application needs to take care of this.


    \sa minorTickLabels( ),setMajorTickLabels( )
*/
void HbSlider::setMinorTickLabels( const QStringList &minorTickLabels )
{
    Q_D( HbSlider );
    d->sliderControl->setMinorTickLabels( minorTickLabels );
    d->updateTickLabels( );
}


/*!

    \deprecated HbSlider::primitive(HbStyle::Primitive)
        is deprecated.

    \reimp
 */
QGraphicsItem *HbSlider::primitive( HbStyle::Primitive primitive ) const
{
    Q_D( const HbSlider );
    switch ( primitive ) {
        case HbStylePrivate::P_Slider_thumb:
            return HbSliderControlPrivate::d_ptr( d->sliderControl )->handle->primitive( 
                (HbStyle::Primitive)HbStylePrivate::P_Slider_thumb );
        case HbStylePrivate::P_SliderElement_text:
            if (d->elementItemMap.contains (HbSlider::TextElement) ){
                return d->elementItemMap[HbSlider::TextElement].item;
            }
            break;
        case HbStylePrivate::P_SliderElement_icon:
            if (d->elementItemMap.contains (HbSlider::IconElement)) {
                return d->elementItemMap[HbSlider::IconElement].item;
            }
            break;
        case HbStylePrivate::P_SliderElement_increase:
            if (d->elementItemMap.contains (HbSlider::IncreaseElement)) {
                return d->elementItemMap[HbSlider::IncreaseElement].item;
            }
            break;
        case HbStylePrivate::P_SliderElement_decrease:
            if (d->elementItemMap.contains (HbSlider::DecreaseElement)) {
                return d->elementItemMap[HbSlider::DecreaseElement].item;
            }
            break;
        case HbStylePrivate::P_Slider_groove:
            return HbSliderControlPrivate::d_ptr( d->sliderControl )->groove;
        case  HbStylePrivate::P_SliderElement_touchgroove:
            return  HbSliderControlPrivate::d_ptr( d->sliderControl )->grooveTouchArea;
        
        case HbStylePrivate::P_SliderElement_touchhandle:
            return HbSliderControlPrivate::d_ptr( d->sliderControl )->handle->primitive( 
                (HbStyle::Primitive)HbStylePrivate::P_SliderElement_touchhandle );
            
        default:
            break;
    }
    return 0;
}

/*!
    \reimp
 */
void HbSlider::updatePrimitives( )
{
    HbWidget::updatePrimitives();
    Q_D( const HbSlider );
    HbStyleOptionSlider option;
    initStyleOption( &option );

    if ( d->elementItemMap.contains(HbSlider::TextElement)) {
        option.text = d->sliderTextString; 
        HbStylePrivate::updatePrimitive( d->elementItemMap[HbSlider::TextElement].item,d->elementItemMap[HbSlider::TextElement].type,&option );
    }
    if ( d->elementItemMap.contains(HbSlider::IconElement)) {
        option.sliderElementIcon =d->icons.value( HbSlider::IconElement ); 
        HbStylePrivate::updatePrimitive( d->elementItemMap[HbSlider::IconElement].item,HbStylePrivate::P_SliderElement_icon,&option );
    }
    if ( d->elementItemMap.contains(HbSlider::IncreaseElement) ) {
        option.sliderElementIcon =d->icons.value( HbSlider::IncreaseElement ); 
        HbStylePrivate::updatePrimitive( d->elementItemMap[HbSlider::IncreaseElement].item,HbStylePrivate::P_SliderElement_increase,&option );
        HbStylePrivate::updatePrimitive(  d->elementItemMap[HbSlider::IncreaseElement].touchItem ,HbStylePrivate::P_SliderElement_touchincrease,&option );

    }
    if ( d->elementItemMap.contains (HbSlider::DecreaseElement) ) {
        option.sliderElementIcon =d->icons.value( HbSlider::DecreaseElement ); 
        HbStylePrivate::updatePrimitive( d->elementItemMap[HbSlider::DecreaseElement].item,HbStylePrivate::P_SliderElement_decrease,&option );
        HbStylePrivate::updatePrimitive( d->elementItemMap[HbSlider::DecreaseElement].touchItem ,HbStylePrivate::P_SliderElement_touchdecrease,&option );
    }
    if ( d->sliderControl ) {
        d->sliderControl->updatePrimitives( );
    }
}

/*!
    \reimp
 */
void HbSlider::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
#ifndef HB_GESTURE_FW 
    Q_D( HbSlider );
    HbWidget::mousePressEvent( event );
    if( d->elementItemMap.contains (HbSlider::IncreaseElement) ) {
        if ( d->elementItemMap[HbSlider::IncreaseElement].touchItem->isUnderMouse( ) ) {
            // this is used for effect
            d->pressOnIncrement  = true;
            HbWidgetFeedback::triggered( d->sliderControl, Hb::InstantPressed, Hb::ModifierSliderElement );
            event->accept( );
            // increment and start reapeat timer 
            d->startIncrementing( );
            return;
        }
    }
    if( d->elementItemMap.contains(HbSlider::DecreaseElement) ) {
        if ( d->elementItemMap[HbSlider::DecreaseElement].touchItem->isUnderMouse( ) ) {
            d->pressOnIncrement  = false;
            HbWidgetFeedback::triggered( d->sliderControl, Hb::InstantPressed, Hb::ModifierSliderElement );
            event->accept( );
            //decrement and start decrement timer
            d->startDecrementing( );
            return;
        }
    }
#else 
    Q_UNUSED(event);
#endif 
}


/*!
    \reimp

    This Api start the releae effect.
 */
void HbSlider::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
#ifndef HB_GESTURE_FW 
    Q_D( HbSlider );
    HbWidget::mouseReleaseEvent( event );
    if( d->elementItemMap.contains(HbSlider::IncreaseElement) ) {
        if ( d->pressOnIncrement ) {
            if( d->elementItemMap[HbSlider::IncreaseElement].touchItem->isUnderMouse( ) ) {
                HbWidgetFeedback::triggered( d->sliderControl, Hb::InstantReleased, Hb::ModifierSliderElement );
            }
#ifdef HB_EFFECTS
            HbEffectInternal::add( HB_SLIDER_TYPE,"slidericon_release", "increaserelease" );
            HbEffect::start( d->elementItemMap[HbSlider::IncreaseElement].item, HB_SLIDER_TYPE, "increaserelease" );  
#endif
        }
    }
    if( d->elementItemMap.contains(HbSlider::DecreaseElement) ) {
        if ( !d->pressOnIncrement ) {
            if( d->elementItemMap[HbSlider::DecreaseElement].touchItem->isUnderMouse( ) ) {
                HbWidgetFeedback::triggered( d->sliderControl, Hb::InstantReleased, Hb::ModifierSliderElement );
            }
#ifdef HB_EFFECTS
            HbEffectInternal::add( HB_SLIDER_TYPE,"slidericon_release", "decreaserelease" );
            HbEffect::start( d->elementItemMap[HbSlider::DecreaseElement].item, HB_SLIDER_TYPE, "decreaserelease" );  
#endif
        }
    }
    d->stopRepeatAction( );
#else
    Q_UNUSED( event )
#endif 
}

/*!
  reimp

*/

void HbSlider::gestureEvent(QGestureEvent *event)
{ 
    Q_D(HbSlider);
    //consume the event if gesture is on increment or decrement,It is being handled in mouse press and mouse release
    //If it is on mute/control igonore the gesture as they are handled separately by HbAbstractButton and HbSlidercontrol classes
    if (HbTapGesture *tap = qobject_cast<HbTapGesture *>(event->gesture(Qt::TapGesture))) {
        QPointF pos = event->mapToGraphicsScene(tap->position());
        bool consumeEvent = false;
        switch( tap->state( ) ) {
        case Qt::GestureStarted: {
            if ( d->elementItemMap.contains (HbSlider::IncreaseElement ) ) {
                if (d->elementItemMap[HbSlider::IncreaseElement].touchItem ) {
                    if (d->elementItemMap[HbSlider::IncreaseElement].touchItem->sceneBoundingRect().contains(pos)) {
                        consumeEvent = true;
                        d->pressOnIncrement  = true;
                        HbWidgetFeedback::triggered( d->sliderControl, Hb::InstantPressed, Hb::ModifierSliderElement );
                        event->accept( );
                        // increment and start reapeat timer 
                        d->startIncrementing( );
                    }
                }
            }
            if ( d->elementItemMap.contains (HbSlider::DecreaseElement ) ) {
                if (d->elementItemMap[HbSlider::DecreaseElement].touchItem ) {
                    if (d->elementItemMap[HbSlider::DecreaseElement].touchItem->sceneBoundingRect().contains(pos))  {
                        consumeEvent = true;
                        d->pressOnIncrement  = false;
                        HbWidgetFeedback::triggered( d->sliderControl, Hb::InstantPressed, Hb::ModifierSliderElement );
                        event->accept( );
                        //decrement and start decrement timer
                        d->startDecrementing( );

                    }
                }
            }
        }
        break;
        case Qt::GestureCanceled:
        case Qt::GestureFinished:            
            if ( d->elementItemMap.contains (HbSlider::IncreaseElement ) ) {
                if (d->elementItemMap[HbSlider::IncreaseElement].touchItem ) {
                    if ( d->pressOnIncrement ) {
                        if( d->elementItemMap[HbSlider::IncreaseElement].touchItem->sceneBoundingRect().contains(pos)) {
                            HbWidgetFeedback::triggered( d->sliderControl, Hb::InstantReleased, Hb::ModifierSliderElement );
                            #ifdef HB_EFFECTS
                            HbEffectInternal::add( HB_SLIDER_TYPE,"slidericon_release", "increaserelease" );
                            HbEffect::start( d->elementItemMap[HbSlider::IncreaseElement].item, HB_SLIDER_TYPE, "increaserelease" );  
                            #endif
                        }
                    }
                }
            }
            if ( d->elementItemMap.contains (HbSlider::DecreaseElement ) ) {
                if (d->elementItemMap[HbSlider::DecreaseElement].touchItem ) {
                    if( d->elementItemMap.contains(HbSlider::DecreaseElement) ) {
                        if ( !d->pressOnIncrement ) {
                            if( d->elementItemMap[HbSlider::DecreaseElement].touchItem->sceneBoundingRect().contains(pos) ) {
                                HbWidgetFeedback::triggered( d->sliderControl, Hb::InstantReleased, Hb::ModifierSliderElement );
                                #ifdef HB_EFFECTS
                                HbEffectInternal::add( HB_SLIDER_TYPE,"slidericon_release", "decreaserelease" );
                                HbEffect::start( d->elementItemMap[HbSlider::DecreaseElement].item, HB_SLIDER_TYPE, "decreaserelease" );  
                                #endif
                            }
                        }
                    }
                }
            }
            d->stopRepeatAction( );
            break;
        default: break;
        }    
        if(!consumeEvent) {
           event->ignore();
           HbWidget::gestureEvent(event);
       }
    }
        
}

/*!
    \reimp

    This api update the primitive when ever item enable has changed
 */
QVariant HbSlider::itemChange( GraphicsItemChange change, const QVariant &value )
{
    switch( change )
    {
        case ItemEnabledHasChanged:
            updatePrimitives( );
            break;
        default:
            break;
    }
    return HbWidget::itemChange( change, value );
}

/*!
    \reimp
 */
void HbSlider::focusInEvent( QFocusEvent* event )
{
    Q_D( HbSlider );
    HbWidget::focusInEvent( event );
    d->sliderControl->setFocus( );    
}



#include "moc_hbslider.cpp"
