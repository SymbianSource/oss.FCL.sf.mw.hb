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

#include "hbslidercontrol_p.h"
#include "hbslidercontrol_p_p.h"
#include "hbabstractslidercontrol_p.h"
#include "hbsliderhandle_p.h"
#include "hbstyleoptionslider_p.h"
#include "hbslidertickmarkslabel_p.h"
#include "hbslidertickmarkslabel_p.h"
#include <hbstyle.h>
#include <hbinstance.h>
#include <hbtheme.h>
#include <hbtooltip.h>
#include <hbwidgetfeedback.h>
#include <hbgraphicsscene.h>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#ifdef HB_GESTURE_FW
#include "hbtapgesture.h"
#include "hbpangesture.h"
#endif 

#ifdef HB_EFFECTS
#include "hbeffect.h"
#include "hbeffectinternal_p.h"
#define HB_SLIDERCONTROL_TYPE "HB_SLIDERCONTROL"
#endif



/*!
    private class  
*/
HbSliderControlPrivate::HbSliderControlPrivate( ) :
    handle( 0 ), // slider handle
    tickPosition( Hb::NoSliderTicks ), // slider tick position
    snappingMode( HbSliderControl::MinorTickSnapping ), // snapping mode
    majorTickInterval( 0 ),// major tick interval
    minorTickInterval( 0 ),// minor tick interval
    groove( 0 ), //slider groove
    progressGroove( 0 ),//progress mask top of groove
    displayCurrValueToolTip( true ), // holds whether to show current value tooltip or not
    toolTipAlignment( Qt::AlignTop|Qt::AlignRight ), // tooltip alignment
    groovePressed( false ), // hold whether groove is pressed or not
    setDefault( false ), // holds whther default value for track press is set
    previousValue( 0 ),  // default value for track press
    trackHandlingEnable( false ),// this variable holds whether track will be interective or not
    handleMoving( false ),
    grooveTouchArea ( 0 ),
    enableProgressTrack ( true ),
    userDefinedTooltipAlign ( false )
{

    majorLabel.clear();
    minorLabel.clear();
    
}


HbSliderControlPrivate::~HbSliderControlPrivate( )
{
   

}


/*!
   \internal
   This api created required primitive( handle,groove, touch area )
*/
void HbSliderControlPrivate::init( )
{
    Q_Q( HbSliderControl );
   
    q->enableTrackEventHandling( true );
#ifdef HB_GESTURE_FW
    q->grabGesture(Qt::TapGesture);
    q->grabGesture(Qt::PanGesture,Qt::ReceivePartialGestures);
    q->setFiltersChildEvents(true) ;
#endif    
    // creating handle
    handle = createHandle();
    if(handle) {
        HbStyle::setItemName( handle, "handle" );
    }
    q->setFocusPolicy( Qt::FocusPolicy( ( qApp->style( ) )->styleHint( QStyle::SH_Button_FocusPolicy ) ) );
    //creating groove
    groove = createGroove();
    if(groove) {
        HbStyle::setItemName( groove, "groove" );
    }
    //filled item top of groove
    if ( enableProgressTrack ) {
        progressGroove = createProgressGroove();
        if(progressGroove) {
            HbStyle::setItemName( progressGroove, "progressgroove");
        }    
    }
    q->connect( hbInstance->theme( ), SIGNAL( changed( ) ), q, SLOT( updateTheme( ) ) );
    q->connect( q , SIGNAL( actionTriggered( int ) ), q , SLOT( showToolTip( ) ) );
    q->connect( q , SIGNAL( sliderReleased( ) ), q , SLOT( hideToolTip( ) ) );
    q->setFlags( QGraphicsItem::ItemIsFocusable );
}



/*!
  \internal
  This is used to create the handle, is virtual and can be overridden to create different handle.
  */
HbSliderHandle *HbSliderControlPrivate::createHandle()
{
    Q_Q( HbSliderControl );
    HbSliderHandle *handle = new HbSliderHandle( q );
    return handle;
}

/*!
  \internal
  This is used to create the groove. can be overrideen by derived class
  */
QGraphicsItem *HbSliderControlPrivate::createGroove()
{
    Q_Q( HbSliderControl );
    QGraphicsItem *groove = qgraphicsitem_cast<QGraphicsItem*>( q->style( )->createPrimitive(
        HbStyle::P_Slider_groove, q ) );
    return groove;
}
/*!
  \internal
  This is used to create the masked progress groove. can be overrideen by derived class
  */
QGraphicsItem *HbSliderControlPrivate::createProgressGroove()
{
    Q_Q( HbSliderControl );
    QGraphicsItem *proggroove = qgraphicsitem_cast<QGraphicsItem*>( q->style( )->createPrimitive(
        HbStyle::P_Slider_progressgroove, q ) );//comes on top of groove
    return proggroove;
}

/*!
   This api adjust the handle within slider area
   and position it depending upon slider value
*/
void HbSliderControlPrivate::adjustHandle( )
{
    Q_Q( HbSliderControl );
    handle->adjustSize( );
    QRectF bounds = q->boundingRect( );
    QRectF handleBounds = handle->boundingRect( );
    qreal span = 0;
    if ( q->orientation( ) == Qt::Horizontal ) {
        bounds.adjust( 0, 0, -handleBounds.width( ), 0 );
        // calculating span
        span = bounds.width( );
    } else {
        bounds.adjust( 0, 0, 0, -handleBounds.height( ) );
        // calculating span
        span = bounds.height( );
    }

    HbStyleOptionSlider opt;
    q->initStyleOption( &opt );
    // calculating pixel position depending on slider position
    int pos = QStyle::sliderPositionFromValue( opt.minimum, opt.maximum,
        opt.sliderValue, static_cast<int>( span ), opt.upsideDown );
    // align the rect to the centre of groove
    QRect rect = QStyle::alignedRect( opt.direction, Qt::AlignCenter,
        handleBounds.size( ).toSize( ), bounds.toRect( ) );
    if ( q->orientation( ) == Qt::Horizontal ) {
        rect.moveLeft( pos );
    } else {
        rect.moveTop( pos );
    }
    //set handle position
    handle->forceSetPos( rect.topLeft( ) );
    //progress groove should be adjusted whenever handle position is changed
    if ( progressGroove ) {
        q->style( )->updatePrimitive( progressGroove, HbStyle::P_Slider_progressgroove, &opt );
    }
}

/*!
   This api returns true if pos in on handle rect
*/
bool HbSliderControlPrivate::onHandle( QPointF pos )
{
    Q_Q( HbSliderControl );
    QRectF handleRect = handle->touchItem->sceneBoundingRect( );
    bool result = false;
    switch ( q->orientation( ) ) {
        case Qt::Horizontal:
            if ( handleRect.contains( pos.x( ) , handleRect.y( ) ) ) {
                result = true;
            }
            break;
        case Qt::Vertical:
            if ( handleRect.contains( handleRect.x( ),pos.y( ) ) ) {
                result = true;
            }
            break;
    }
    return result;
}


/*!
   This api returns near by tick to the sliderPosition
*/
int HbSliderControlPrivate::getNearbyTick( )
{
    Q_Q( HbSliderControl );
    // calculating slider position with respect to minimum( )
    int correctedSliderPos = q->sliderPosition( ) - q->minimum( );
    // if snapping mode is NoSnapping then there is no need to calculate
    //near by tick
    if ( !( snappingMode & HbSliderControl::NoSnapping ) ) {
        if ( snappingMode & HbSliderControl::MinorTickSnapping  ) {
            if ( minorTickInterval > 0 ) {
                // calculating tick number
               int tickNumber = correctedSliderPos / minorTickInterval;
               // check if reminder is grater then half of 
               //tick interval then increment tick number by 1
               if ( ( correctedSliderPos % minorTickInterval ) >= qRound( minorTickInterval/2.0 ) ) {
                    if ( ( ( tickNumber+1 ) * minorTickInterval ) <= ( q->maximum( ) - q->minimum( ) ) ) {
                        tickNumber++;
                    }
               }
               // return sliderPosition of near by tick
               return ( ( tickNumber * minorTickInterval ) + q->minimum( ) );
            }
        } else {
            if ( majorTickInterval > 0 ) {
                // calculating tick number
                int tickNumber = correctedSliderPos / majorTickInterval;                
                // check if reminder is grater then half of 
               //tick interval then increment tick number by 1
                if ( ( correctedSliderPos % majorTickInterval ) >= qRound( majorTickInterval/2.0 ) ) {
                    if ( ( ( tickNumber+1 ) * majorTickInterval ) <= ( q->maximum( ) - q->minimum( ) ) ) {
                        tickNumber++;
                    }
                }
                // return sliderPosition of near by tick
                return ( ( tickNumber * majorTickInterval ) + q->minimum( ) );
            }
        }
    }
    // return sliderPosition( ) if snapping mode is NoSnapping
    return ( q->sliderPosition( ) );
}

/*!
    Constructs a slider control with \a parent.
*/
HbSliderControl::HbSliderControl( QGraphicsItem *parent )
    : HbAbstractSliderControl( *new HbSliderControlPrivate, parent )
{
    Q_D( HbSliderControl );
    d->q_ptr = this;
    d->init( );
}

/*!
    Constructs a slider control with \a orientation and \a parent.
*/
HbSliderControl::HbSliderControl( Qt::Orientation orientation, QGraphicsItem *parent )
    : HbAbstractSliderControl( *new HbSliderControlPrivate, parent )
{
    Q_D( HbSliderControl );
    d->q_ptr = this;
    d->init( );
    setOrientation( orientation );
}

/*!
  Protected constructor for initialization from derived ctor.
*/
HbSliderControl::HbSliderControl(HbSliderControlPrivate &dd,QGraphicsItem *parent)
	:HbAbstractSliderControl(dd,parent)
{
}

/*!
    When theme is changed this is called.
*/
void HbSliderControl::updateTheme( )
{
    Q_D( HbSliderControl );
    // HbSliderHandle::boundingRect( ) result depends on current theme
    d->adjustHandle( );
}


/*!
    Destroys the slider.
*/
HbSliderControl::~HbSliderControl( )
{
  
}

/*!
    Returns the tickmark position of the slider.

    The default value is \c Hb::NoTicks.

    \sa setTickPosition( )
*/
Hb::SliderTickPositions HbSliderControl::tickPosition( ) const
{
    Q_D( const HbSliderControl );
    return d->tickPosition;
}

/*!
    Sets the tickmark position of the slider.

    \sa tickPosition( )
 */
void HbSliderControl::setTickPosition( Hb::SliderTickPositions position )
{
    Q_D( HbSliderControl );
    if ( d->tickPosition != position ) {
        d->tickPosition = position;
     }
}

/*!
    Returns the interval between major tickmarks.

    The default value is \c 0.

    \sa setMajorTickInterval( )
*/
int HbSliderControl::majorTickInterval( ) const
{
    Q_D( const HbSliderControl );
    return d->majorTickInterval;
}

/*!
    Sets the interval between major tickmarks.

    Special values:
    \li a negative value means no major ticks will be drawn
    \li value \c 0 means that the interval is automatically calculated

    \sa majorTickInterval( )
*/
void HbSliderControl::setMajorTickInterval( int interval )
{
    Q_D( HbSliderControl );
    if ( d->majorTickInterval != interval ) {
        d->majorTickInterval = qAbs ( interval );

    }
}

/*!
    Returns the minor ticklabels of the slider.
  
    \sa setMinorTickLabels( )
*/
QStringList HbSliderControl::minorTickLabels( ) const
{
    Q_D( const HbSliderControl );
    return  d->minorLabel;
}

/*!
    Sets the interval between minor tickmarks.

    Special values:
    \li a negative value means no minor ticks will be drawn
    \li value \c 0 means that the interval is automatically calculated

    \sa minorTickInterval( )
*/
void HbSliderControl::setMinorTickInterval( int interval )
{
    Q_D( HbSliderControl );
    if ( d->minorTickInterval != interval ) {
        d->minorTickInterval = qAbs ( interval );
        if ( d->snappingMode == MinorTickSnapping && d->minorTickInterval > 0 && d->tickPosition!= Hb::NoSliderTicks ) {
            updateSliderPosToTick( );
        }
    }
}

/*!
    Returns the interval between minor tickmarks.

    The default value is \c 0.

    \sa setMinorTickInterval( )
*/
int HbSliderControl::minorTickInterval( ) const
{
    Q_D( const HbSliderControl );
    return d->minorTickInterval;
}

/*!
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

    \sa majorTickLabels( )
*/
void HbSliderControl::setMajorTickLabels( const QStringList &majorTickLabels )
{
    Q_D( HbSliderControl );
    d->majorLabel = majorTickLabels;

}

/*!
    Returns the minor ticklabels of the slider.
  
    \sa setMinorTickLabels( )
*/
QStringList HbSliderControl::majorTickLabels( ) const
{
    Q_D( const HbSliderControl );
    return d->majorLabel;
   // return static_cast<HbSliderTickmarksLabel*>( d->tickmarkslabel )->majorTickLabels( );
}

/*!
    Sets the minor ticklabels of the slider.
    see setMajorTickLabels for detailed description

    \sa minorTickLabels( ),setMajorTickLabels( )
*/
void HbSliderControl::setMinorTickLabels( const QStringList &minorTickLabels )
{
    Q_D( HbSliderControl );
    d->minorLabel = minorTickLabels;
}


/*!
    Returns the SnappingMode of the slider.

    The default value is \c HbSliderControl::MinorTickSnapping.

    \sa setSnappingMode( )
*/
HbSliderControl::SnappingMode HbSliderControl::snappingMode( ) const
{
    Q_D( const HbSliderControl );
    return d->snappingMode;
}

/*!
    Sets the snappingMode of the slider.

    \sa snappingMode( )
*/
void HbSliderControl::setSnappingMode( SnappingMode mode )
{
    Q_D( HbSliderControl );
    if ( d->snappingMode != mode ) {
        d->snappingMode = mode;
        if ( d->snappingMode != NoSnapping ) {
            updateSliderPosToTick( );
        }
    }
}

/*!
    This will enable/disable current value diplay as tooltip
    on thumb drag
*/
 void HbSliderControl::setToolTipVisible( bool value )
 {
    Q_D( HbSliderControl );
    if ( d->displayCurrValueToolTip != value ) {
        d->displayCurrValueToolTip = value;
    }
 }


 /*!
    returns whether tooltip is Enable or disable for current value
*/
bool HbSliderControl::isToolTipVisible( )const
{
    Q_D( const HbSliderControl );
    return d->displayCurrValueToolTip;
}


/*!
    sets the toolTip Alignment with respect to the thumb
*/
void HbSliderControl::setToolTipAlignment( Qt::Alignment alignment )
{
    Q_D( HbSliderControl );
    if ( d->toolTipAlignment != alignment ) {
        d->toolTipAlignment=alignment;
        d->userDefinedTooltipAlign = true;
    }
}


/*!
    returns the tooltip Alignment with respect to the thumb
*/
Qt::Alignment HbSliderControl::toolTipAlignment( ) const
{
    Q_D( const HbSliderControl );
    return d->toolTipAlignment;
}

void HbSliderControl::changeEvent( QEvent *event )
{
    Q_D( HbSliderControl );
    switch ( event->type( ) ) {
        case QEvent::LayoutDirectionChange:
            d->adjustHandle( );
            break;
        case QEvent::StyleChange:
            // HbSlider::boundingRect( ) result depends on current style
            d->handle->prepareGeometryChange( );
            d->adjustHandle( );
            break;
        default:
            break;
    }
    HbAbstractSliderControl::changeEvent( event );
}

/*!
    This api will hide the tooltip , this will

    be called on mouse release
*/
void HbSliderControl::hideToolTip( )
{
    HbToolTip::hideText( qobject_cast<HbGraphicsScene *>( scene( ) ) );
}


/*!
  This will display tooltip when ever there

  is thumb drag.

*/
void HbSliderControl::showToolTip( )
{
    Q_D( HbSliderControl );
    if ( isSliderDown( ) && d->displayCurrValueToolTip ) {
        HbToolTip::showText( toolTip( ) , d->handle , d->toolTipAlignment );
    }
}

/*!
  reimp

*/
void HbSliderControl::mousePressEvent( QGraphicsSceneMouseEvent *event )
{ 
#ifndef HB_GESTURE_FW
    Q_D( HbSliderControl );
    HbAbstractSliderControl::mousePressEvent( event );
    if ( !d->trackHandlingEnable ) {
        event->ignore( );
        return;
    }
    // bound check
    if ( maximum( ) == minimum( ) || ( event->buttons( ) ^ event->button( ) ) ) {
        event->ignore( );
        return;
    }
    #if defined( QT_KEYPAD_NAVIGATION ) && !defined( Q_OS_SYMBIAN )
    if ( QApplication::keypadNavigationEnabled( ) ) {
        setEditFocus( true );
    }
    #endif
 
    QRectF bounds = boundingRect( );
    QRectF handleBounds = d->handle->boundingRect( );
    bounds.adjust( 0, 0, -handleBounds.width( ), -handleBounds.height( ) );

    // Define area events are accepted
    QRectF eventRect = d->grooveTouchArea->sceneBoundingRect( );
    // return if point is outside track touch area
    if ( !eventRect.contains( event->scenePos( ) ) ) {
        event->ignore( );
        return;
    }
    event->accept( );
    if( d->onHandle( event->scenePos( ) ) ) {
        HbWidgetFeedback::triggered( this, Hb::InstantPressed, Hb::ModifierSliderHandle );
        setSliderDown( true );
        d->handle->updatePrimitives( );
        d->handleMoving = true;
    }
    else {
    // effect
#ifdef HB_EFFECTS
    if( d->grooveTouchArea->sceneBoundingRect( ).contains( event->scenePos( ) ) ) {
        if( orientation( ) == Qt::Horizontal ) {   
        // effect for horizontal track press
            HbEffectInternal::add( HB_SLIDERCONTROL_TYPE,"slider_h_trackpress", "h_trackpress" );
            HbEffect::start( d->groove, HB_SLIDERCONTROL_TYPE, "h_trackrpress" );
        }
        else {
            HbEffectInternal::add( HB_SLIDERCONTROL_TYPE,"slider_v_trackpress", "v_trackpress" );
            HbEffect::start( d->groove, HB_SLIDERCONTROL_TYPE, "v_trackpress" );
        }  
        d->handle->updatePrimitives( );
    }
#endif
        HbWidgetFeedback::triggered( this, Hb::InstantPressed );
        qreal handlePos = 0;
        qreal span = 0;
        // calculate handle position and span
        switch ( orientation( ) ) {
            case Qt::Horizontal:
                handlePos = event->pos( ).x( ) - handleBounds.width( ) / 2;
                span = bounds.width( );
                break;
            case Qt::Vertical:
                handlePos = event->pos( ).y( ) - handleBounds.height( ) / 2;
                span = bounds.height( );
                break;
        }

        HbStyleOptionSlider opt;
        d->groovePressed = true;
        initStyleOption( &opt );
        // calculate pixel value for event position
        int pressValue = QStyle::sliderValueFromPosition( opt.minimum, opt.maximum,
            static_cast<int>( handlePos ),static_cast<int>( span ),opt.upsideDown );
        //update the groove and touch item
        style( )->updatePrimitive( d->groove, HbStyle::P_Slider_groove, &opt );  
        // if default is set then dont increment or decrement slider value
        // just set default value to slider
        if ( d->setDefault ) {
            setValue( d->previousValue );
            if ( pressValue > value( ) ) {
                setRepeatAction( SliderSingleStepAdd, pressValue );
             }
             else {
                setRepeatAction( SliderSingleStepSub, pressValue );
             }
             d->setDefault = false;
        } 
        else {      
            if( singleStep( ) != 0 ) {
                HbWidgetFeedback::continuousTriggered( this, Hb::ContinuousDragged );
            }
            setSliderPosition( pressValue );
            triggerAction( SliderMove );
            setRepeatAction( SliderNoAction, pressValue );
        }
    }
#else
    Q_UNUSED(event)
#endif
    
}

/*!
  reimp

*/
void HbSliderControl::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
#ifndef HB_GESTURE_FW
    HbAbstractSliderControl::mouseReleaseEvent( event );
    Q_D( HbSliderControl );
    d->handleMoving = false;
    if ( !d->trackHandlingEnable ) {
        event->ignore( );
        return;
    }
    if ( d->groovePressed ) {

#ifdef HB_EFFECTS    
        if( d->grooveTouchArea->sceneBoundingRect( ).contains( event->scenePos( ) ) ) {
            if( orientation( ) == Qt::Horizontal ) {   
                HbEffectInternal::add( HB_SLIDERCONTROL_TYPE,"slider_h_trackrelease", "h_trackrelease" );
                HbEffect::start( d->groove, HB_SLIDERCONTROL_TYPE, "h_trackrelease" );
            }
            else {
                HbEffectInternal::add( HB_SLIDERCONTROL_TYPE,"slider_v_trackrelease", "v_trackrelease" );
                HbEffect::start( d->groove, HB_SLIDERCONTROL_TYPE, "v_trackrelease" );
            }    
            d->handle->updatePrimitives( );
        }
#endif
        HbStyleOptionSlider opt;
        d->groovePressed = false;
        initStyleOption( &opt );    
        // update primitive from press to normal
        style( )->updatePrimitive( d->groove, HbStyle::P_Slider_groove, &opt );
    }
    if ( event->buttons( ) ) {
        event->ignore( );
        return;
    }
    event->accept( );
    // stop the timer 
    setRepeatAction( SliderNoAction );
    // snapping
    if( isSliderDown( ) ) {
        if ( d->snappingMode != NoSnapping ) {
            updateSliderPosToTick( );
        }
        if( singleStep( ) != 0 ) {
            HbWidgetFeedback::continuousStopped( this, Hb::ContinuousDragged );
        }
        HbWidgetFeedback::triggered( this, Hb::InstantReleased, Hb::ModifierSliderHandle );
    } else {
        HbWidgetFeedback::triggered( this, Hb::InstantReleased );
    }
    // update handle
    setSliderDown( false );
    d->handle->updatePrimitives( );
    HbToolTip::hideText( qobject_cast<HbGraphicsScene *>( scene( ) ) );
#else
    Q_UNUSED(event);
#endif 
}


#ifdef HB_GESTURE_FW
void HbSliderControl::gestureEvent(QGestureEvent *event)
{
    Q_D(HbSliderControl);
    if(HbTapGesture *tap = qobject_cast<HbTapGesture *>(event->gesture(Qt::TapGesture))) {
            if( d->onHandle( event->mapToGraphicsScene(tap->position( ) ) ) ){
                event->ignore();
                return;
            }
        switch(tap->state()) {
        case Qt::GestureStarted: {
            QRectF eventRect = d->grooveTouchArea->sceneBoundingRect( );
            if ( !d->trackHandlingEnable  || maximum( ) == minimum( ) || 
                !eventRect.contains( event->mapToGraphicsScene(tap->position( ) ) ) ){
                event->ignore( );
                return;
            }
            
#if defined( QT_KEYPAD_NAVIGATION ) && !defined( Q_OS_SYMBIAN )
            if ( QApplication::keypadNavigationEnabled( ) ) {
                setEditFocus( true );
            }
#endif
            event->accept( );
        // effect
#ifdef HB_EFFECTS
            if( orientation( ) == Qt::Horizontal ) {   
            // effect for horizontal track press
                HbEffectInternal::add( HB_SLIDERCONTROL_TYPE,"slider_h_trackpress", "h_trackpress" );
                HbEffect::start( d->groove, HB_SLIDERCONTROL_TYPE, "h_trackrpress" );
            }
            else {
                HbEffectInternal::add( HB_SLIDERCONTROL_TYPE,"slider_v_trackpress", "v_trackpress" );
                HbEffect::start( d->groove, HB_SLIDERCONTROL_TYPE, "v_trackpress" );
            }  
    
#endif
            HbStyleOptionSlider opt;
            d->groovePressed = true;
            initStyleOption( &opt );
            style( )->updatePrimitive( d->groove, HbStyle::P_Slider_groove, &opt );  
            HbWidgetFeedback::triggered( this, Hb::InstantPressed );
        }
        break;
        case Qt::GestureFinished:{
            if ( !d->trackHandlingEnable ) {
                event->ignore( );
                return;
            }
            qreal handlePos = 0;
            qreal span = 0;
            QRectF bounds = boundingRect( );
            QRectF handleBounds = d->handle->boundingRect( );
            bounds.adjust( 0, 0, -handleBounds.width( ), -handleBounds.height( ) );
            QPointF relativePos = mapFromScene( event->mapToGraphicsScene(tap->position( ) ) );
            // calculate handle position and span
            switch ( orientation( ) ) {
                case Qt::Horizontal:
                    handlePos = relativePos.x( ) - handleBounds.width( ) / 2;
                    span = bounds.width( );
                    break;
                case Qt::Vertical:
                    handlePos = relativePos.y( ) - handleBounds.height( ) / 2;
                    span = bounds.height( );
                    break;
                default:
                    break;
            }
            HbStyleOptionSlider opt;
            initStyleOption( &opt );

            int pressValue = QStyle::sliderValueFromPosition( opt.minimum, opt.maximum,
                static_cast<int>( handlePos ),static_cast<int>( span ),opt.upsideDown );


            // if default is set then dont increment or decrement slider value
            // just set default value to slider
            setSliderPosition( pressValue );
            triggerAction( SliderMove );
            setRepeatAction( SliderNoAction, pressValue );
            HbWidgetFeedback::triggered( this, Hb::InstantReleased );
            if ( d->groovePressed ) {
#ifdef HB_EFFECTS    
                if( orientation( ) == Qt::Horizontal ) {   
                    HbEffectInternal::add( HB_SLIDERCONTROL_TYPE,"slider_h_trackrelease", "h_trackrelease" );
                    HbEffect::start( d->groove, HB_SLIDERCONTROL_TYPE, "h_trackrelease" );
                }  else {
                    HbEffectInternal::add( HB_SLIDERCONTROL_TYPE,"slider_v_trackrelease", "v_trackrelease" );
                    HbEffect::start( d->groove, HB_SLIDERCONTROL_TYPE, "v_trackrelease" );
                }    
#endif
                HbStyleOptionSlider opt;
                d->groovePressed = false;
                initStyleOption( &opt );    
                // update primitive from press to normal
                style( )->updatePrimitive( d->groove, HbStyle::P_Slider_groove, &opt );
            }
         }
         break;

        case Qt::GestureCanceled: {
            if ( d->groovePressed ) {
#ifdef HB_EFFECTS    
                if( orientation( ) == Qt::Horizontal ) {   
                    HbEffectInternal::add( HB_SLIDERCONTROL_TYPE,"slider_h_trackrelease", "h_trackrelease" );
                    HbEffect::start( d->groove, HB_SLIDERCONTROL_TYPE, "h_trackrelease" );
                }  else {
                    HbEffectInternal::add( HB_SLIDERCONTROL_TYPE,"slider_v_trackrelease", "v_trackrelease" );
                    HbEffect::start( d->groove, HB_SLIDERCONTROL_TYPE, "v_trackrelease" );
                }    
#endif
                HbStyleOptionSlider opt;
                d->groovePressed = false;
                initStyleOption( &opt );    
                // update primitive from press to normal
                style( )->updatePrimitive( d->groove, HbStyle::P_Slider_groove, &opt );
            }
        }
        break;

        default:
            break;
        }
    }
    if (HbPanGesture *panGesture = qobject_cast<HbPanGesture*>(event->gesture(Qt::PanGesture))) {
        switch(panGesture->state( )) {
            case Qt::GestureStarted: 
            case Qt::GestureUpdated:{
                QPointF startPoint = event->mapToGraphicsScene(panGesture->offset()+panGesture->startPos( ) );
                if( ( d->onHandle( startPoint) && d->grooveTouchArea->sceneBoundingRect( ).contains( startPoint))||isSliderDown( ) ) {
                    qreal handlePos = 0;
                    qreal span = 0;
                    QRectF bounds = boundingRect( );
                    QRectF handleBounds = d->handle->boundingRect( );
                    bounds.adjust( 0, 0, -handleBounds.width( ), -handleBounds.height( ) );
                    QPointF relativePos = mapFromScene( event->mapToGraphicsScene(panGesture->startPos( ) + panGesture->offset()) );
                    // calculate handle position and span
                    switch ( orientation( ) ) {
                        case Qt::Horizontal:
                            handlePos = relativePos.x( ) - handleBounds.width( ) / 2;
                            span = bounds.width( );
                            break;
                        case Qt::Vertical:
                            handlePos = relativePos.y( ) - handleBounds.height( ) / 2;
                            span = bounds.height( );
                            break;
                        default:
                            break;
                    }
                    HbStyleOptionSlider opt;
                    initStyleOption( &opt );

                    int pressValue = QStyle::sliderValueFromPosition( opt.minimum, opt.maximum,
                        static_cast<int>( handlePos ),static_cast<int>( span ),opt.upsideDown ); 
                    setRepeatAction( SliderNoAction,static_cast<int>( pressValue ) );
                    setSliderDown( true );
                    setSliderPosition( pressValue );
                    showToolTip( );
                    d->groovePressed = false;
                    updatePrimitives();
                    d->handleMoving = true ;
                    break;
                } else if(d->grooveTouchArea->sceneBoundingRect().contains(startPoint) ){
                    HbStyleOptionSlider opt;
                    d->groovePressed = true;
                    initStyleOption( &opt );
                    style( )->updatePrimitive( d->groove, HbStyle::P_Slider_groove, &opt );  
                    HbWidgetFeedback::triggered( this, Hb::InstantPressed );
                    event->ignore();
                    break;
                } else {
                    setSliderDown( false );
                    d->groovePressed = false;
                    updatePrimitives( );
                    d->handle->updatePrimitives();
                    d->handleMoving = false;
                    event->ignore();
                    HbAbstractSliderControl::gestureEvent(event);
                }

            }
            break;
            case Qt::GestureFinished:
            case Qt::GestureCanceled: {
                setSliderDown( false );
                d->groovePressed = false;
                updatePrimitives( );
                d->handle->updatePrimitives();
                d->handleMoving = false;
                event->ignore();
                HbAbstractSliderControl::gestureEvent(event);
            }
            break;
            default:
                break;
        }
    }

  //  HbAbstractSliderControl::gestureEvent(event);
}
#endif


bool HbSliderControl::sceneEventFilter(QGraphicsItem *obj,QEvent *event)
{
    Q_D(HbSliderControl);
    if( obj == d->grooveTouchArea) {
        if (!isEnabled() ) {
            return false;
        }
	    if (event->type() == QEvent::Gesture){
            gestureEvent( (QGestureEvent *) (event));
            return true;
        }
    }
    else if ( obj == d->handle) {
        event->ignore();
    }
	return false;
}


/*!
  
*/
void HbSliderControl::releasedOutside( bool outside )
{
    Q_UNUSED( outside )
    emit releasedOutside( );
}

/*!
  reimp

*/
void HbSliderControl::resizeEvent( QGraphicsSceneResizeEvent *event )
{
    Q_D( HbSliderControl );
    QGraphicsWidget::resizeEvent( event );
    // for Bug Fix::Ticks are not getting updated after 
    // element is added to slider
    updatePrimitives( );
    repolish();
    d->adjustHandle( );  
}
/*!
  reimp

*/
void HbSliderControl::polish( HbStyleParameters& params )
{
    Q_D( HbSliderControl );
    HbStyleOptionSlider option;
    initStyleOption( &option );
    HbAbstractSliderControl::polish( params );
    d->adjustHandle( );
    updatePrimitives( );
}

/*!
 
*/
void HbSliderControl::initStyleOption( HbStyleOptionSlider *option ) const
{
    Q_D( const HbSliderControl );
    if ( !option ) {
        return;
    }
    HbAbstractSliderControl::initStyleOption( option );
    option->subControls = QStyle::SC_None;
    option->activeSubControls = QStyle::SC_None;
    option->orientation = orientation( );
    option->maximum = maximum( );
    option->minimum = minimum( );
    option->tickPosition =  d->tickPosition;
    option->tickInterval = d->majorTickInterval;
    option->minorTickInterval = d->minorTickInterval;
    option->thumbPath = d->handleIcon;
    option->upsideDown = ( orientation( ) == Qt::Horizontal ) 
        ? ( invertedAppearance( ) != ( option->direction == Qt::RightToLeft ) )
            : ( !invertedAppearance( ) );
    // we use the upsideDown option instead
    option->direction = Qt::LeftToRight;
    option->sliderPosition = sliderPosition( );
    option->sliderValue = value( );
    option->singleStep = singleStep( );
    option->pageStep = pageStep( );
    option->groovePressed = d->groovePressed;
    if ( orientation( ) == Qt::Horizontal ) {
        option->state |= QStyle::State_Horizontal;
    }
    option->handleRect = d->handle->boundingRect( );
    QRectF bounds = boundingRect( );
    QRectF handleBounds = d->handle->boundingRect( );
    qreal span = 0;
    if ( orientation( ) == Qt::Horizontal ) {
        bounds.adjust( 0, 0, -handleBounds.width( )/2, 0 );
        // calculating span
        span = bounds.width( );
    } else {
        bounds.adjust( 0, 0, 0, -handleBounds.height( )/2 );
        // calculating span
        span = bounds.height( );
    }
    option->span = span;
}

/*!
  sets the icon path for handle 

  /sa handleIcon( )
   
*/
 void HbSliderControl::setHandleIcon( const QString &handleIcon )
{
    Q_D( HbSliderControl );
    d->handleIcon=handleIcon;
    if( d->handle ) {
        d->handle->updatePrimitives( );
    }
}

/*!
    returns icon of handle

    /sa setHandleIcon( )   
*/
QString HbSliderControl::handleIcon( ) const
{
    Q_D( const HbSliderControl );
    return d->handleIcon;
}


void HbSliderControl::setHandleItem(QGraphicsItem *item)
{
    Q_D( HbSliderControl );
    if( d->handle ) {
        d->handle->setHandleItem(item);
        d->handle->updatePrimitives();
    }
}

QGraphicsItem* HbSliderControl::handleItem( ) const
{
    Q_D( const HbSliderControl );
	if(d->handle) {
		return d->handle->handleItem();
	}
	return NULL;
}

void HbSliderControl::setHandleVisible(bool isVisible)
{
    Q_D( HbSliderControl );
	if(isVisible) {
		d->handle->setVisible(true);
	} else {
		d->handle->setVisible(false);
	}
}

bool HbSliderControl::handleVisible( ) const
{
    Q_D( const HbSliderControl );
	return d->handle->isVisible();
}
 
/*!
  adjust the handle
*/
void HbSliderControl::sliderChange( SliderChange  change)
{
    Q_D( HbSliderControl );
    HbAbstractSliderControl::sliderChange( change );
    d->adjustHandle( );
    if ( change == SliderOrientationChange ) {
        //Layout is not mirrored in vertical orientation with absolute ticks
        if(d->orientation ==Qt::Horizontal) { 
            if (!d->userDefinedTooltipAlign) {
                d->toolTipAlignment = ( Qt::AlignTop|Qt::AlignHCenter );
            }
        } else {
            if (!d->userDefinedTooltipAlign) {
                d->toolTipAlignment = ( Qt::AlignTop|Qt::AlignRight );
            }
        }
        repolish( );
    }
}


/*!
    \reimp
*/
QGraphicsItem * HbSliderControl::primitive( HbStyle::Primitive primitive ) const
{
    Q_D( const HbSliderControl );
    switch( primitive ){
        case HbStyle::P_Slider_groove:
            return ( d->groove );
        case HbStyle::P_Slider_thumb:
            return ( d->handle );
        case HbStyle::P_Slider_progressgroove:
            return ( d->progressGroove );
        default:return( NULL );
    }
}

/*!
 reimp

*/
QVariant HbSliderControl::itemChange( GraphicsItemChange change, const QVariant &value )
{
    Q_D( HbSliderControl );
    if ( change == QGraphicsItem::ItemEnabledHasChanged ) {
        updatePrimitives( );
    }
    switch( change ) {
        case ItemVisibleHasChanged:
            if ( !value.toBool( ) ) {
                setSliderDown( false );
                setRepeatAction( SliderNoAction );
                updatePrimitives( );
                if ( d->handle ) {
                    d->handle->updatePrimitives( );
                }
                d->groovePressed = false;
            } else {
                repolish();
            }
        break;
        case ItemSceneHasChanged: {
            updatePrimitives();
        }
        break;
        default:
            break;
    }
    return HbAbstractSliderControl::itemChange( change, value );
}

/*!
    reimp
    consume tooltip event

*/
bool HbSliderControl::sceneEvent ( QEvent * event )
{
    //eating tooltip event
    if ( event->type( ) == QEvent::GraphicsSceneHelp ) {
        event->accept( );
        return true;
    }
    return HbAbstractSliderControl::sceneEvent( event );
}

/*!
    \reimp
*/
void HbSliderControl::updatePrimitives( ) 
{
    Q_D( HbSliderControl );
    HbStyleOptionSlider opt;
    initStyleOption( &opt );
    if ( isEnabled( ) ) {
        opt.state |= QStyle::State_Enabled;
    } else {
        opt.state &= ~QStyle::State_Enabled;
    }
    if ( d->groove ) {
        style( )->updatePrimitive( d->groove, HbStyle::P_Slider_groove, &opt );
    }

    if( d->progressGroove ) {
            style( )->updatePrimitive( d->progressGroove, HbStyle::P_Slider_progressgroove, &opt );
    }

}

/*!
    Sets the current slider position.

    \sa sliderPosition( )
*/
void HbSliderControl::setSliderPosition( int value )
{
    Q_D( HbSliderControl );
    bool b = d->displayCurrValueToolTip;
    d->displayCurrValueToolTip = false;
    HbAbstractSliderControl::setSliderPosition( value );
    d->displayCurrValueToolTip =  b;
}

/*!
  snapp slider values
*/
void HbSliderControl::updateSliderPosToTick( )  
{
    Q_D( HbSliderControl );       
    int nearbyTick = d->getNearbyTick( );
    if ( nearbyTick > sliderPosition( ) ) {
        while ( nearbyTick != sliderPosition( ) ) {
            int curVal = sliderPosition( ) + 1;
            setSliderPosition( curVal );
        }
    } else {
        while ( nearbyTick != sliderPosition( ) ) {
            int curVal = sliderPosition( ) - 1;
            setSliderPosition( curVal );
        }
    }
}

/*!
   if first parameter is true then on mouse press, set this value to slider , 
   instead of increasing or decreasing it
*/
void HbSliderControl::setPreviousValue( bool setDefault, int value )
{
    Q_D( HbSliderControl );
    d->setDefault  = setDefault;
    d->previousValue = value;
}

/*!
   if true is passed then will make track interactive
*/
void HbSliderControl::enableTrackEventHandling( bool enable )
{
    Q_D( HbSliderControl );
    if ( d->trackHandlingEnable == enable ) {
        return;
    }
    if ( enable ) {
        //creating groove touch area
        d->grooveTouchArea =  qgraphicsitem_cast<QGraphicsItem*>( style( )->createPrimitive(
            HbStyle::P_SliderElement_touchgroove, this ) );
        if ( d->grooveTouchArea ) {
            HbStyle::setItemName( d->grooveTouchArea, "groovetoucharea" );
        }
        if(QGraphicsObject *touchArea = d->grooveTouchArea->toGraphicsObject()) {

#ifdef HB_GESTURE_FW
            ungrabGesture(Qt::TapGesture);
            ungrabGesture(Qt::PanGesture);
            touchArea->grabGesture(Qt::TapGesture);
            touchArea->grabGesture(Qt::PanGesture,Qt::ReceivePartialGestures);
#endif 
        }
    }
    d->trackHandlingEnable = enable ;
}

/*!
   returns true if track interactive otherwise return false
*/
bool HbSliderControl::isTrackEventHandlingEnabled ( )
{
    Q_D( HbSliderControl );
    return d->trackHandlingEnable ;
}

/*!
 Gets the size of the handle
 */

QSizeF HbSliderControl::getHandleSize ( ) 
{
    Q_D( HbSliderControl );
    return d->handle->size( ) ;
}


void HbSliderControl::setTrackFilled(bool trackVisible )
{
    
    Q_D( HbSliderControl );
    if ( d->enableProgressTrack != trackVisible ) {
        d->enableProgressTrack = trackVisible;
        if ( d->enableProgressTrack ) {
            if ( !d->progressGroove ) {
                d->progressGroove = d->createProgressGroove();  
            }
            if(d->progressGroove) {
                d->progressGroove->setVisible( true );
                HbStyle::setItemName( d->progressGroove, "progressgroove");
            }
                   
        }else if ( d->progressGroove ) {
            d->progressGroove->setVisible( false );
            HbStyle::setItemName( d->progressGroove, QString());
        }
    }

}


bool HbSliderControl::isTrackFilled() const
{
    Q_D( const HbSliderControl );
    return d->enableProgressTrack;
}

