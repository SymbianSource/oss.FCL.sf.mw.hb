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

#include <hbcheckbox.h>
#include <hbstyleoptioncheckbox_p.h>
#include "hbabstractbutton_p.h"
#include "hbtooltip.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>

#ifdef HB_EFFECTS
#include "hbeffect.h"
#include "hbeffectinternal_p.h"
#endif

#ifdef HB_GESTURE_FW
#include <hbtapgesture.h>
#endif

/*
  private class
*/
class HB_AUTOTEST_EXPORT HbCheckBoxPrivate : public HbAbstractButtonPrivate
{
    Q_DECLARE_PUBLIC( HbCheckBox )

public:
    HbCheckBoxPrivate( );
    virtual ~HbCheckBoxPrivate( );

    void createPrimitives( );

public:
    QString mText;
    QGraphicsItem *mTextItem;
    QGraphicsItem *mIconItem;
    QGraphicsItem *mTouchArea;

    bool mTristate;
    bool mNoChange;
    Qt::CheckState mPublishedState;
};

/*
  private class constructor
*/
HbCheckBoxPrivate::HbCheckBoxPrivate():
    mTextItem( 0 ),
    mIconItem( 0 ), 
    mTouchArea( 0 ),
    mTristate( false ),
    mNoChange( false ),
    mPublishedState( Qt::Unchecked )
{
    //adding effect for checkbox
#ifdef HB_EFFECTS
    HbEffectInternal::add( HB_CHECKBOX_TYPE,"checkbox_selected", "pressed" );
#endif
}

/*
  private class destructor
*/
HbCheckBoxPrivate::~HbCheckBoxPrivate( )
{
}

/*
  create primitive
*/
void HbCheckBoxPrivate::createPrimitives( )
{
    Q_Q( HbCheckBox );
    if( !mTouchArea ) {
        mTouchArea = HbStylePrivate::createPrimitive( HbStylePrivate::P_CheckBox_toucharea, q );
        if( QGraphicsObject *ta = qgraphicsitem_cast<QGraphicsObject*>( mTouchArea ) ) {
            ta->grabGesture( Qt::TapGesture );
        }
    }
    if ( !mTextItem ) {
        mTextItem = HbStylePrivate::createPrimitive( HbStylePrivate::P_CheckBox_text, q );
        }
    if ( !mIconItem ) {
        mIconItem = HbStylePrivate::createPrimitive( HbStylePrivate::P_CheckBox_icon, q );
    }
}

/*!
    @hbwidgets
    \class HbCheckBox
    \brief The HbCheckBox class provides a check box widget with a text label.
    
    A check box is an option button that can be in checked, unchecked, and 
    partially checked states as shown in the pictures below. Check boxes are 
    typically used to represent application features that can be enabled or 
    disabled without affecting other features, but you can also subclass it to 
    implement different behavior.

    \image html checkbox_unchecked.png A check box in unchecked state.

    \image html checkbox_checked.png A check box in checked state.

    \image html checkbox_partiallychecked.png A check box in partially checked state.
    
    When a user checks or unchecks a check box, it emits the stateChanged() 
    signal. Connect a slot to this signal if you want to trigger an action every 
    time the check box state changes. You can use isChecked() to query whether a 
    check box is checked or not.

    In addition to checked and unchecked states, %HbCheckBox provides 
    an optional partially checked state. This state is useful when you handle 
    hierarchical list items, for example. If you need the partially checked 
    state, enable it with setTristate() and query the state of the check box 
    with checkState().

    A check box has default icons for checked, unchecked, and partially checked 
    states. The text can be set in the constructor or with setText(). If the 
    check box text is long, it is shown in multiple lines. By default a check box 
    can have up to three lines of text.

    \section _usecases_hbcheckbox Using the HbCheckBox class
    
    \subsection _uc_hbcheckbox_001 Creating a check box.
    
    The following code snippet creates a check box in the layout.

    \code
    HbCheckBox *checkbox = new HbCheckBox(QString("checkbox_text"));
    layout->addItem(checkbox);
    \endcode

*/


/*!

    \fn void HbCheckBox::stateChanged(int state)

    This signal is emitted when the user checks or unchecks a check box causing a 
    state change of the check box.
    
    \param  state The new state (Qt::CheckState) of the check box.

    \sa Qt

*/


/*!
    
    Constructs a check box with the following default values:
    - \a state is \c Qt::CheckState::Unchecked.
    - \a text is empty.
    - \a tristate is \c false.
    
    \sa Qt
    
*/
HbCheckBox::HbCheckBox( QGraphicsItem *parent )
    : HbAbstractButton( *new HbCheckBoxPrivate, parent )
{
    Q_D( HbCheckBox );
    d->q_ptr = this;
    setCheckable( true );
    d->createPrimitives( );
#ifdef HB_GESTURE_FW
    grabGesture( Qt::TapGesture );
#endif

}

/*!
    
    Constructs a check box with \a text and the following default values:
    - \a state is \c Qt::CheckState::Unchecked.
    - \a tristate is \c false.
    
    \sa Qt

*/
HbCheckBox::HbCheckBox( const QString &text, QGraphicsItem *parent )
    : HbAbstractButton( *new HbCheckBoxPrivate, parent )
{
    Q_D( HbCheckBox );
    d->q_ptr = this;
    d->mText = text;
    setCheckable( true );
    d->createPrimitives( );
}

/*!
    Destructor.
*/
HbCheckBox::~HbCheckBox( )
{
}

/*!

    Sets the \a text shown on a check box.

    \sa text()
*/
void HbCheckBox::setText( const QString &text )
{
    Q_D( HbCheckBox );
    if( d->mText != text ){
        d->mText = text;
        //updatePrimitives();
        if( d->mTextItem ){
            HbStyleOptionCheckBox checkBoxOption;
            initStyleOption( &checkBoxOption );
            HbStylePrivate::updatePrimitive( d->mTextItem, HbStylePrivate::P_CheckBox_text, &checkBoxOption );
        }
    }    
}

/*!
    
    Returns the text of a check box.

    \sa setText()
*/
QString HbCheckBox::text( ) const
{
    Q_D( const HbCheckBox );
    return d->mText;
}

/*!
    
    Sets a check box to:
    - tristate-enabled mode if \a isTristate is \c true.
    - twostate-enabled mode if \a isTristate is \c false (default).
    
    \sa isTristate( )
*/
void HbCheckBox::setTristate( bool isTristate )
{
    Q_D( HbCheckBox );
    d->mTristate = isTristate;
}

/*!
    
    Checks whether a check box can have two or three states.
    
    \return \c true if a check box is tristate-enabled.
    \return \c false if a check box is twostate-enabled.

    \sa setTristate( )
*/
bool HbCheckBox::isTristate( ) const
{
    Q_D( const HbCheckBox );
    return d->mTristate;
}

/*!
    
    Returns the current state (checked, unchecked, or partially checked) of a 
    check box.

*/
Qt::CheckState HbCheckBox::checkState() const
{
    Q_D( const HbCheckBox );
    if ( d->mTristate &&  d->mNoChange ) {
        return Qt::PartiallyChecked;
    }
    return d->checked ? Qt::Checked : Qt::Unchecked;
}

/*!

    \deprecated HbCheckBox::primitive(HbStyle::Primitive)
        is deprecated.

    Returns the pointer for \a primitive passed.
    Will return NULL if \a primitive passed is icon because user cannot
    configure the check and unchecked icons. Style needs to be changed if
    user wants different icons.
*/
QGraphicsItem *HbCheckBox::primitive( HbStyle::Primitive primitive ) const
{
    Q_D( const HbCheckBox );

    switch ( primitive ) {
        case HbStylePrivate::P_CheckBox_text:
            return d->mTextItem;
        case HbStylePrivate::P_CheckBox_toucharea:
            return d->mTouchArea;
        case HbStylePrivate::P_CheckBox_icon:
            return d->mIconItem;
        default:
            return 0;
    }
}

/*!
    
    Sets the state of a check box to \a state and emits the stateChanged() 
    signal. If the check box state changes, updatePrimitives() is called
    
*/
void HbCheckBox::setCheckState( Qt::CheckState state )
{
    Q_D( HbCheckBox );
    if (state == Qt::PartiallyChecked) {
        d->mTristate = true;
        d->mNoChange = true;
    } else {
        d->mNoChange = false;
    }
    d->blockRefresh = true;
    setChecked( state != Qt::Unchecked );
    d->blockRefresh = false;
    d->refresh( );

    if ( state != d->mPublishedState ) {
        d->mPublishedState = state;
        emit stateChanged( state );
    }
}

/*!
    Updates the icon and the text primitives of a check box.
*/
void HbCheckBox::updatePrimitives( )
{
    HbWidget::updatePrimitives( );
    Q_D( HbCheckBox );

    HbStyleOptionCheckBox checkBoxOption;
    initStyleOption( &checkBoxOption );

    if ( d->mTextItem ) {
        HbStylePrivate::updatePrimitive( d->mTextItem, HbStylePrivate::P_CheckBox_text, &checkBoxOption );
    }
    if ( d->mIconItem ) {
        HbStylePrivate::updatePrimitive( d->mIconItem, HbStylePrivate::P_CheckBox_icon, &checkBoxOption );
    }
    if (d->mTouchArea) {
        HbStylePrivate::updatePrimitive( d->mTouchArea, HbStylePrivate::P_CheckBox_toucharea, &checkBoxOption );
    }
}

/*!
    Initializes the style of a check box with the \a option style.
*/
void HbCheckBox::initStyleOption(HbStyleOptionCheckBox *option) const
{
    Q_D( const HbCheckBox );

    HbAbstractButton::initStyleOption( option );

    Q_ASSERT( option );
    option->text = d->mText;
    if ( d->mTristate && d->mNoChange ){
        option->state.operator = ( QStyle::State_NoChange );
    } else {
        option->state.operator = ( d->checked ? QStyle::State_On : QStyle::State_Off );
    }
}

/*!

*/
void HbCheckBox::resizeEvent( QGraphicsSceneResizeEvent *event )
{
    HbAbstractButton::resizeEvent( event );
}

/*!

 */
bool HbCheckBox::hitButton( const QPointF &pos ) const
{
    Q_D(const HbCheckBox);
    QRectF compRect = d->mTouchArea->boundingRect( );
    compRect.translate( d->mTouchArea->pos( ) );
    return compRect.contains( pos );
}

/*!

*/
void HbCheckBox::checkStateSet( )
{
    Q_D(HbCheckBox);
    d->mNoChange = false;
    Qt::CheckState state = checkState( );
    if ( state != d->mPublishedState ) {
        #ifdef HB_EFFECTS
            HbEffect::start( d->mIconItem, HB_CHECKBOX_TYPE, "pressed" );
        #endif
        d->mPublishedState = state;
        emit stateChanged( state );
    }
}

/*!

*/
void HbCheckBox::nextCheckState( )
{
    if( checkState( ) == Qt::PartiallyChecked ) {
        HbAbstractButton::nextCheckState( );
    }
    HbAbstractButton::nextCheckState( );
    HbCheckBox::checkStateSet( );
}

#ifndef HB_GESTURE_FW
/*!

*/
void HbCheckBox::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
    HbAbstractButton::mouseReleaseEvent( event );
    updatePrimitives();
}

/*!

*/
void HbCheckBox::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
    Q_UNUSED( event );
    // To show the tooltip on press,move outside and come back to same button.
    // check for hit pos 
    bool hit = hitButton( event->pos( ) );
    if ( hit ) {
        HbToolTip::showText( toolTip(), this );
    }   
}
#endif

#ifdef HB_GESTURE_FW
void HbCheckBox::gestureEvent( QGestureEvent *event )
{
    HbAbstractButton::gestureEvent( event );
}
#endif
/*!

*/
void HbCheckBox::keyPressEvent(QKeyEvent *keyEvent)
{
    switch ( keyEvent->key( ) ) {
    case Qt::Key_Select:
    case Qt::Key_Enter:
    case Qt::Key_Return:
            HbAbstractButton::keyPressEvent( keyEvent );
        break;
    default:
            HbAbstractButton::keyPressEvent( keyEvent );
    }
}

/*!

 */
QVariant HbCheckBox::itemChange( GraphicsItemChange change, const QVariant &value )
{
    switch ( change ) {
        case ItemEnabledHasChanged:
        case ItemVisibleChange: {
                updatePrimitives( );
            }
            break;
        default:
            break;
    }
    return HbAbstractButton::itemChange( change, value );
}
#include "moc_hbcheckbox.cpp"

