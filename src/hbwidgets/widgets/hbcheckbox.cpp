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
    Spacer class.
*/
class HbCheckBoxSpacer: public HbWidgetBase
{

public:
    HbCheckBoxSpacer( QGraphicsItem *parent = 0 );
    QSizeF sizeHint(  Qt::SizeHint which, const QSizeF &constraint = QSizeF( ) ) const;
};

HbCheckBoxSpacer::HbCheckBoxSpacer( QGraphicsItem *parent )
    : HbWidgetBase( parent )
{
#if QT_VERSION >= 0x040600
    setFlag(QGraphicsItem::ItemHasNoContents, true);
#endif
}

QSizeF HbCheckBoxSpacer::sizeHint( Qt::SizeHint which, const QSizeF &constraint ) const
{
    Q_UNUSED( constraint );

    if ( which == Qt::MaximumSize ) {
        return QSizeF( QWIDGETSIZE_MAX, QWIDGETSIZE_MAX );
    }
    return QSizeF( 0.f, 0.f );
}


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
        mTouchArea = q->style( )->createPrimitive( HbStyle::P_CheckBox_toucharea, q );
        if( QGraphicsObject *ta = qgraphicsitem_cast<QGraphicsObject*>( mTouchArea ) ) {
            ta->grabGesture( Qt::TapGesture );
        }
    }
    if ( !mTextItem ) {
        mTextItem = q->style( )->createPrimitive( HbStyle::P_CheckBox_text, q );
        }
    if ( !mIconItem ) {
        mIconItem = q->style( )->createPrimitive( HbStyle::P_CheckBox_icon, q );
    }
}

/*!
    @beta
    @hbwidgets
    \class HbCheckBox
    \brief The HbCheckBox widget provides a checkbox with a text label.
    
    \image html checkbox_unchecked.png A checkbox with unchecked state.

    \image html checkbox_checked.png A checkbox with checked state.

    \image html checkbox_partiallychecked.png A checkbox with partiallychecked state.

    A HbCheckBox is an option button that can be switched on (checked)
    or off (unchecked). Checkboxes are typically used to represent
    features in an application that can be enabled or disabled without
    affecting others, but different types of behavior can be
    implemented.

    Whenever a checkbox is checked or cleared it emits the signal
    stateChanged(). Connect to this signal if you want to trigger an
    action each time the checkbox changes state. You can use
    isChecked() to query whether or not a checkbox is checked.

    In addition to checked and unchecked states, HbCheckBox
    optionally provides a third state which is PartiallyChecked. This
    is useful for Hierarchical list item. If you need this third state,
    enable it with setTristate(), and use checkState() to query the current
    toggle state.

    Checkbox has default icon for checked,unchecked and partiallyChecked states.
    The text can be set in the constructor or with setText().

    Example usage:
    \code
    HbCheckBox *checkbox = new HbCheckBox(QString("checkbox_text"));
    layout->addItem(checkbox);
    \endcode

*/


/*!
    \fn void HbCheckBox::stateChanged(int state)
    This signal is emitted whenever the check box's state changes,
    i.e. whenever the user checks or unchecks it.
    \a state contains the check box's new Qt::CheckState.
*/



/*!
    @beta
    Constructor an object of HbCheckBox with default parameters.
    By default the state of check box will be Unchecked, text will
    be empty and tristate will be false.
*/
HbCheckBox::HbCheckBox( QGraphicsItem *parent )
    : HbAbstractButton( *new HbCheckBoxPrivate, parent )
{
    Q_D( HbCheckBox );
    d->q_ptr = this;
    setCheckable( true );
    d->createPrimitives( );
    // creattion of top and bottom spacer.
    HbStyle::setItemName( new HbCheckBoxSpacer( this ), "topSpacer" );
    HbStyle::setItemName( new HbCheckBoxSpacer( this ), "bottomSpacer" );
#ifdef HB_GESTURE_FW
    grabGesture( Qt::TapGesture );
#endif

}

/*!
    @beta
    Constructor an object of HbCheckBox with \a text passed.
    By default the state of check box will be Unchecked.
    Tristate will be false by default.
*/
HbCheckBox::HbCheckBox( const QString &text, QGraphicsItem *parent )
    : HbAbstractButton( *new HbCheckBoxPrivate, parent )
{
    Q_D( HbCheckBox );
    d->q_ptr = this;
    d->mText = text;
    setCheckable( true );
    d->createPrimitives( );
    // creattion of top and bottom spacer.
    HbStyle::setItemName( new HbCheckBoxSpacer(this), "topSpacer");
    HbStyle::setItemName( new HbCheckBoxSpacer(this), "bottomSpacer");

}

/*!
    Destructor.
*/
HbCheckBox::~HbCheckBox( )
{
}

/*!
    @beta
    Sets the text of checkbox.
    UpdatePrimitive is called by this functions once the text is changed.
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
            style( )->updatePrimitive( d->mTextItem, HbStyle::P_CheckBox_text, &checkBoxOption );
        }
    }    
}

/*!
    @beta
    Returns the text of checkbox.
    \sa setText()
*/
QString HbCheckBox::text( ) const
{
    Q_D( const HbCheckBox );
    return d->mText;
}

/*!
    @beta
    Sets tristate support for checkbox based upon \a flag.
    By default tristate is false and checkbox has got only two states.
    \sa isTristate( )
*/
void HbCheckBox::setTristate( bool isTristate )
{
    Q_D( HbCheckBox );
    d->mTristate = isTristate;
}

/*!
    @beta
    Returns whether tristate is enabled for checkbox or not.
    \sa setTristate( )
*/
bool HbCheckBox::isTristate( ) const
{
    Q_D( const HbCheckBox );
    return d->mTristate;
}

/*!
    @beta
    Returns the current state of HbCheckBox.
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
        case HbStyle::P_CheckBox_text:
            return d->mTextItem;
        case HbStyle::P_CheckBox_toucharea:
            return d->mTouchArea;
        case HbStyle::P_CheckBox_icon:
            return d->mIconItem;
        default:
            return 0;
    }
}

/*!
    @beta
    Sets the state of HbCheckBox to the \a state passed.
    Calls updatePrimitive when state is changed.
    Emits signal stateChanged(state).
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
    Updates the icon and text primitives.
*/
void HbCheckBox::updatePrimitives( )
{
    HbWidget::updatePrimitives( );
    Q_D( HbCheckBox );

    HbStyleOptionCheckBox checkBoxOption;
    initStyleOption( &checkBoxOption );

    if ( d->mTextItem ) {
        style( )->updatePrimitive( d->mTextItem, HbStyle::P_CheckBox_text, &checkBoxOption );
    }
    if ( d->mIconItem ) {
        style( )->updatePrimitive( d->mIconItem, HbStyle::P_CheckBox_icon, &checkBoxOption );
    }
    if (d->mTouchArea) {
        style( )->updatePrimitive( d->mTouchArea, HbStyle::P_CheckBox_toucharea, &checkBoxOption );
    }
}

/*!
    Initializes the style for check box with the \a option passed.
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
    Updates the primitive as per new size.
*/
void HbCheckBox::resizeEvent( QGraphicsSceneResizeEvent *event )
{
    HbAbstractButton::resizeEvent( event );
}

/*!
  Overloaded hit detection to include touch area
 */
bool HbCheckBox::hitButton( const QPointF &pos ) const
{
    Q_D(const HbCheckBox);
    QRectF compRect = d->mTouchArea->boundingRect( );
    compRect.translate( d->mTouchArea->pos( ) );
    return compRect.contains( pos );
}

/*!
    \reimp.
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
    \reimp.
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
    \reimp.
*/
void HbCheckBox::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
    HbAbstractButton::mouseReleaseEvent( event );
    updatePrimitives();
}

/*!
    \reimp.
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
    \reimp.
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
    \reimp
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

