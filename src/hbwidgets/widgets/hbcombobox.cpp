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

#include "hbcombobox_p.h"

#include <hbcombobox.h>
#include <hblistview.h>
#include <hbtoucharea.h>
#include <hbtextitem.h>
#include <hbstyleoptioncombobox_p.h>
#include <QStandardItemModel>
#include <QDebug>

#include <hbtapgesture.h>


/*!
    @beta
    @hbwidgets
    \class HbComboBox

    \brief The HbComboBox provides a drop-down list that permits selecting an item.

    A HbComboBox provides a means of presenting a list of options to the user
    in a way that takes up the minimum amount of screen space.

    A combobox is a selection widget that displays the currently selected item,
    and provides a drop-down list that permits selecting an item.
    
    A HbComboBox with currently selected item
    \image html noneditablecombobox.png.


    A HbComboBox with drop-down list to select an item from list of options
    \image html comboboxdropdownlist.png.


    HbComboBox are of two types:

    1: Editable:

    If combobox is set to editable, tapping on combobox label field invokes the VKB in
    touch environments or accepts input from an attached keyboard. Tapping on 
    button area of combobox launches dropdown list. Choosing an element from 
    the combobox dropdown list replaces the label with the chosen item. Touch events on
    combobox button behaves same in both editable and non-editable combobox.

    2: Non Editable:
    
    If the combobox is set to non-editable then even tapping on label field will launch 
    drop down unlike editable combobox. Tapping on combobox button area behaves in 
    same way as in editable combobox.

    By default combobox is non-editable. 

    Application is responsible for setting the model in combobox. If no model is set or model
    set is empty then drop down list will not be displayed at all. Combobox owns the model set 
    by application.

    The number of rows to be displayed in HbComboBox drop down can be configured by overriding
    HbComboBox css. By default number of items in drop down is 8 in portrait mode and 5 in
    landscape mode.

    HbComboBox provides four signals:

    \li currentIndexChanged( index ) is emitted when the current selection in the combobox is
    changed either by user interaction or programmatically.

    \li currentIndexChanged( const QString& ) is emitted when the curret selection in the combobox
    is changed by user interaction. If application is setting a differnet index/text
    then this signal is not emmitted.

    \li editTextChanged( const QString& ) is emitted when text is changed in editable combobox.

    \li editingFinished( ) is emitted either because editable combobox lost focus or Return/Enter 
    is pressed.

    The following is an example of how to create a model and adding item in the model.
    HbComboBox takes ownership of model.
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,37}

    An example of how to add strings in HbComboBox and setting currentIndex.
    HbComboBox will creates model internally if model is not created.
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,38}

    An example of how to add stringsList into HbComboBox and setting currentIndex.
    HbComboBox will creates the model internally if model is not created.
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,39}

    An example of how to insert string at some index in HbComboBox.
    HbComboBox will creates the model internally if model is not created.
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,40}

    An example of how to insert StringList at index into HbComboBox.
    HbComboBox will creates the model internally if model is not created.
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,41}

    An example of how to set the items into HbComboBox.
    HbComboBox will replace the existing model.
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,42}

*/


/*!
    Constructs a new HbComboBox with \a parent.
 */
HbComboBox::HbComboBox( QGraphicsItem *parent ):
    HbWidget( *new HbComboBoxPrivate, parent )
{
    Q_D( HbComboBox );
    d->init( );
    updatePrimitives( );
    setProperty( "state", "normal" );
}

/*!
    Destructor.
 */
HbComboBox::~HbComboBox( )
{
}

/*!
    \internal[protected][Access Privilage]
 */
HbComboBox::HbComboBox( HbComboBoxPrivate &dd, QGraphicsItem *parent ) :
    HbWidget( dd, parent )
{
    Q_D( HbComboBox );
    d->init( );
    updatePrimitives( );
    setProperty( "state", "normal" );
}

/*!
    @beta

    \property HbComboBox::items

    It replaces the existing list with \a texts.
*/
void HbComboBox::setItems( const QStringList &texts )
{  
    if ( texts.isEmpty( ) ) {
        return;
    }
    QStandardItemModel* model = new QStandardItemModel( this );
    int textCount = texts.count( );
    for( int i = 0; i < textCount; i++ )
    {
        QStandardItem* standardItem = new QStandardItem( );
        standardItem->setData( texts.at( i ),Qt::DisplayRole );
        model->appendRow( standardItem );
    }
   setModel( model );
}

/*!
    @beta

    This property holds the list added in the combobox.
    By default, for an empty combo box, this property has a empty StringList. 
 */
QStringList HbComboBox::items( ) const
{
    Q_D( const HbComboBox );
    if( d->mModel && d->mModel->rowCount( ) ) {
        QStringList list;
        int rowCount = d->mModel->rowCount( );
        for(int i = 0; i < rowCount; i++) {
            list<<( d->mModel->data( d->mModel->index( i, 0 ) ) ).toString( );
        }
        return list;
    } else {
      return QStringList( );
    }
}

/*!
    @alpha

    Sets \a icon for the item at the given \a index in the combobox.
    This API will not work if applcation sets the model as QStringListModel.
*/
void HbComboBox::setItemIcon( int index, const HbIcon &icon )
{
    Q_D( const HbComboBox );
    if( d->mModel ) {
        QModelIndex item = d->mModel->index( index, 0 );
        if ( item.isValid( ) ) {
            d->mModel->setData( item, icon.qicon( ), Qt::DecorationRole );
        }
    }
}

/*!
    @alpha

    Returns HbIcon at \a index in the combobox.
*/
HbIcon HbComboBox::itemIcon( int index ) const
{
    Q_D( const HbComboBox );
    if( d->mModel ) {
        QModelIndex mi = d->mModel->index( index, 0 );
        if( mi.isValid ( ) ) {
            return d->itemIcon( mi );
        } 
    }
    return HbIcon( );
}

/*!
    @beta

    Either returns the \a role data at this \a index in HbComboBox, or QVariant::Invalid 
    if there is no data for this role.
*/
QVariant HbComboBox::itemData( int index, int role ) const
{
    Q_D( const HbComboBox );
    if( d->mModel ) {
        QModelIndex mi = d->mModel->index( index, 0 );
        if( mi.isValid ( ) ) {
            return d->mModel->data( mi, role );
        }
    }
    return QVariant( );
}

/*!
    @beta

    Sets the \a role data for the item at the given \a index in HbComboBox
    to the specified \a value.
*/
void HbComboBox::setItemData( int index, const QVariant &value, int role )
{
    Q_D( const HbComboBox );
    if( d->mModel ) {
        QModelIndex item = d->mModel->index( index, 0 );
        if ( item.isValid( ) ) {
            d->mModel->setData( item, value, role );
        }
    }
}

/*!
    @beta

    This API is valid only for editable combobox.
    Either returns the validator that is used to constraint text input in
    combobox or returns NULL if it is invalid.

    \sa editable
*/
const HbValidator *HbComboBox::validator( ) const
{
    Q_D( const HbComboBox );
    if( d->mEditable) {
        return d->mLineEdit->validator( );
    }
    return 0;
}

/*!
    @beta

    This API is only valid for the editable combobox.
    Sets the \a validator to be used in editable combobox.
*/
void HbComboBox::setValidator( HbValidator *validator )
{
    Q_D( HbComboBox );
    if( d->mEditable ) {
        disconnect( d->mLineEdit, SIGNAL( textChanged ( QString ) ),
            this, SLOT( _q_textChanged( QString ) ) );
        d->mLineEdit->setValidator( validator );
        connect( d->mLineEdit, SIGNAL( textChanged ( QString ) ),
            this, SLOT( _q_textChanged( QString ) ) );
    }
}

/*!
    @beta

    This property holds the number of items in combobox.
    For an empty combo box, this property is equal to 0.
 */
int HbComboBox::count( ) const
{
    Q_D( const HbComboBox );
    if( d->mModel ) {
        return d->mModel->rowCount( );
    }
    return 0;
}

/*!
    @beta

    Sets the model to \a model 
    Ownership of \a model is taken by HbComboBox.
    It replaces the old model if exists.
    Do not pass 0(NULL) as \a model to clear contents of model instead call clear( ).
    \sa clear
 */
void HbComboBox::setModel( QAbstractItemModel * model )
{
    Q_D( HbComboBox );
    if ( d->mModel != model ) {
        if( model ) {
            d->setModel( model );
        } else {
            clear( );
        }
    }
}

/*!
    @beta

    Returns current model.
 */
QAbstractItemModel* HbComboBox::model( ) const
{
    Q_D( const HbComboBox );
    return d->mModel;
}

/*!
    @beta

    Sets current index to \a index.
    By default no item is selected.
 */
void HbComboBox::setCurrentIndex( int index )
{
    Q_D( HbComboBox );
    if( d->mModel ) {
        QModelIndex mi = d->mModel->index( index, 0 );
        if( mi.isValid( ) ) {
            d->setCurrentIndex( mi );
        }
    }
}

/*!
    @beta

    Either returns index of current item if valid or else returns -1 for invalid current index.
 */
int HbComboBox::currentIndex( ) const
{
    Q_D( const HbComboBox );
    return d->mCurrentIndex.row( );
}

/*!
    @beta

    \fn int HbComboBox::findText(const QString &text, Qt::MatchFlags 
                                      flags = Qt::MatchExactly|Qt::MatchCaseSensitive) const

    Returns the index of the item containing the given \a text; otherwise
    returns -1.

    The \a flags specify how the items in the combobox are searched.
*/

/*!
    @beta

    Returns the index of the item containing the given \a data for the
    given \a role if valid, otherwise returns -1.

    The \a flags specify how the items in the combobox are searched.
*/
int HbComboBox::findData( const QVariant &data, int role, Qt::MatchFlags flags ) const
{
    Q_D( const HbComboBox );
    if( d->mModel ) {
        QModelIndexList result;
        QModelIndex start = d->mModel->index( 0, 0 );
        result = d->mModel->match( start, role, data, 1, flags );
        if ( result.isEmpty( ) ) {
            return -1;
        }
        return result.first( ).row( );
    }
    return -1;
}

/*!
    @beta

    This API is valid for editable combobox.
    Sets the \a text in editable combobox's line edit.
 */
void HbComboBox::setEditText( const QString &text )
{
    Q_D( HbComboBox );
    if( d->mEditable ) {
        disconnect( d->mLineEdit, SIGNAL( textChanged ( QString ) ),
            this, SLOT( _q_textChanged( QString ) ) );
        d->mLineEdit->setText( text );
        connect( d->mLineEdit, SIGNAL( textChanged ( QString ) ),
            this, SLOT( _q_textChanged( QString ) ) );
    }
}

/*!
    @beta

    \property HbComboBox::currentText

    In case of non-editable combobox it returns the text at current index.
    In case of editable combobox it returns the text in line edit.
*/
QString HbComboBox::currentText( ) const
{
    Q_D( const HbComboBox );
    if( d->mEditable ) {
        return d->mLineEdit->text( );
    }else if( d->mCurrentIndex.isValid( ) ) {
        return itemText ( d->mCurrentIndex.row( ) );
    } else {
        return QString( );
    }
}

/*!
    @beta

    Removes the item at the given \a index from HbComboBox.
    If \a index passed is current index then current index will be updated accordingly.
 */
void HbComboBox::removeItem( int index )
{
    Q_D( HbComboBox );
    if( d->mModel ) {
        int rowCount = d->mModel->rowCount( );
        if( index >=0 && index < rowCount ) {
            bool currentText = false;
            if ( d->mModel->index( index, 0 ) == d->mCurrentIndex ) {
                currentText = true;
                d->mModel->removeRow( index );
            } else if( d->mCurrentIndex.row( ) > index ) {
                 int row = d->mCurrentIndex.row( );
                 d->mModel->removeRow( index );
                 d->mCurrentIndex = d->mModel->index( --row, 0 );
                 d->currentIndexChanged( d->mCurrentIndex );
            } else {
                d->mModel->removeRow( index );
            }
            if( d->mModel->rowCount( ) == 0 ) {
                if( d->mEditable ) {
                    clearEditText( );
                } else {
                    if( d->mLineEdit ) {
                        d->mLineEdit->setText( QString( ) );
                    } else {
                        d->mText.clear( );
                        HbStyleOptionComboBox comboBoxOption;
                        initStyleOption( &comboBoxOption );
                        style( )->updatePrimitive(
                            d->mTextItem, HbStyle::P_ComboBox_text, &comboBoxOption );
                    }
                }
                d->mCurrentIndex = QModelIndex();
                return;
            }
            if( currentText ) {
                d->mCurrentIndex = d->mModel->index( 0, 0 );
                if( d->mEditable ) {
                    disconnect(d->mLineEdit, SIGNAL( textChanged ( QString ) ),
                        this, SLOT( _q_textChanged( QString ) ) );
                    d->mLineEdit->setText( d->mModel->data( d->mCurrentIndex ).toString( ) );
                    connect(d->mLineEdit, SIGNAL( textChanged ( QString ) ), 
                        this, SLOT( _q_textChanged( QString ) ) );
                } else {
                    if( d->mLineEdit ) {
                        d->mLineEdit->setText( QString( ) );
                    } else {
                        d->mText = d->mModel->data( d->mCurrentIndex ).toString( );
                        HbStyleOptionComboBox comboBoxOption;
                        initStyleOption( &comboBoxOption );
                        style( )->updatePrimitive(
                            d->mTextItem, HbStyle::P_ComboBox_text, &comboBoxOption);
                    }
                }
                d->currentIndexChanged( d->mCurrentIndex );
            }
        }
    }
}

/*!
    \reimp
 */
void HbComboBox::resizeEvent( QGraphicsSceneResizeEvent *event )
{
    Q_UNUSED( event )
    Q_D( HbComboBox );
    if ( d->mDropDown && d->mDropDown->isVisible( ) ) {
        d->positionDropDown( );
    }
}

/*!
    \reimp
*/
void HbComboBox::keyPressEvent( QKeyEvent *event )
{
    Q_D( HbComboBox );
    if ( !event->isAutoRepeat( ) ) {
        if ( event->key( ) == Qt::Key_Enter || event->key( ) == Qt::Key_Return ) {
            d->mIsDown = true;
            updatePrimitives( );
            event->accept( );
        }
    } else {
        event->accept( );
    }
}

/*!
    \reimp
*/
void HbComboBox::keyReleaseEvent( QKeyEvent *event )
{
    Q_D( HbComboBox );
    if ( !event->isAutoRepeat( ) ){
        if ( event->key( ) == Qt::Key_Enter || event->key( ) == Qt::Key_Return ){
            d->mIsDown = false;
            updatePrimitives( );
            event->accept( );
            d->touchAreaClicked( );
            d->mDropDown->mList->setFocus( );
        }
    } else {
        event->accept( );
    }
}

/*!

    \deprecated HbComboBox::primitive(HbStyle::Primitive)
        is deprecated.

    \reimp
 */
QGraphicsItem* HbComboBox::primitive( HbStyle::Primitive primitive ) const
{
    Q_D( const HbComboBox );

    switch( primitive ) {
        case HbStyle::P_ComboBox_text:
            return d->mTextItem;
        case HbStyle::P_ComboBox_background:
            return d->mBackgroundItem;
        case HbStyle::P_ComboBox_button:
            return d->mButton;
        case HbStyle::P_ComboBoxButton_toucharea:
            return d->mButtonTouchAreaItem;
        default:
            return 0;
    }
}

/*!
    \reimp
 */
void HbComboBox::initStyleOption( HbStyleOptionComboBox *option )const
{
    Q_D( const HbComboBox );
    option->text = d->mText;
    HbWidget::initStyleOption( option );
}

/*!
    \reimp
 */
void HbComboBox::updatePrimitives( )
{
    Q_D( HbComboBox );
    HbStyleOption styleOption;
    HbWidget::initStyleOption( &styleOption );
    if ( d->mIsDown ) {
        styleOption.state |= QStyle::State_Sunken;
    }
    if ( d->mBackgroundItem ) {
        style( )->updatePrimitive( 
            d->mBackgroundItem, HbStyle::P_ComboBox_background, &styleOption );
        style( )->updatePrimitive( 
            d->mButton, HbStyle::P_ComboBox_button, &styleOption );
    }
}

/*!
    @beta

    Removes all items from model. 
 */
void HbComboBox::clear( )
{
    Q_D( HbComboBox );
    if( d->mModel ) {
        d->mModel->removeRows( 0, d->mModel->rowCount( ) );
        d->mCurrentIndex = QModelIndex( );
        if( d->mEditable ) {
            clearEditText( );
        } else {
            if( d->mLineEdit ) {
                d->mLineEdit->setText( QString( ) );
            } else {
                d->mText.clear( );
                HbStyleOptionComboBox comboBoxOption;
                initStyleOption( &comboBoxOption );
                style( )->updatePrimitive( d->mTextItem, HbStyle::P_ComboBox_text, &comboBoxOption );
            }
        }
    }
}

/*!
    @beta

    This API is valid only for editable combobox.
    Clears the contents of line edit in editable combobox.
 */
void HbComboBox::clearEditText( )
{
    Q_D( HbComboBox );
    if( d->mEditable ) {
        disconnect( d->mLineEdit, SIGNAL( textChanged ( QString ) ),
            this, SLOT( _q_textChanged( QString ) ) );
        d->mLineEdit->setText( QString( ) );
        connect( d->mLineEdit, SIGNAL( textChanged ( QString ) ),
            this, SLOT( _q_textChanged( QString ) ) );
    }
}

/*!
    @beta

    \property HbComboBox::editable
    
    If \a editable is true then combobox will be editable else it will be
    non-editable.

    \sa isEditable
 */
void HbComboBox::setEditable( bool editable )
{
    Q_D( HbComboBox ); 
    d->setEditable( editable );
    setProperty( "state", "normal" );
}

/*!
    @beta

    Returns combobox is editable or not. If editable then returns true else returns
    false.

    \sa setEditable
 */
bool HbComboBox::isEditable( ) const
{
    Q_D( const HbComboBox );
    return d->mEditable;
}

/*!
    @beta

    Adds an item in combobox with the given \a text, containing the specified \a userData 
    (stored in the Qt::UserRole). The item is appended to the list of existing items.

    \sa insertItem
 */
void HbComboBox::addItem( const QString &text, const QVariant &userData )
{
    insertItem( count( ), text, userData );
}

/*!
    @beta

    Adds an item in combobox with the given \a icon and \a text,
    and containing the specified \a userData (stored in the Qt::UserRole). 
    The item is appended to the list of existing items.

    \sa insertItem
 */
void HbComboBox::addItem( const HbIcon &icon, const QString &text, const QVariant &userData )
{
    insertItem( count( ), icon, text, userData );
}

/*!
    @beta

    Adds each of the strings in the given \a texts to combobox.
    Each item is appended to the list of existing items in turn.

    \sa insertItem
 */
void HbComboBox::addItems( const QStringList &texts )
{
    insertItems( count( ), texts );
}


/*!
    @beta

    Inserts the \a text in combobox at the given \a index.
    If the index is equal to or higher than the total number of items, 
    the new item is appended to the list of existing items.
    If the index is zero or negative, the new item is prepended to the list of existing items.

    \sa addItems addItem
 */
void HbComboBox::insertItem( int index, const QString &text, const QVariant &userData )
{
    Q_D( HbComboBox );
    if( text.isNull( ) ) {
        return;
    }
    if( !d->mModel ) {
        QStandardItemModel* model = new QStandardItemModel( this );
        setModel( model );
    }
    if ( !d->mModel->rowCount( ) ) {
        d->mModel->insertRow( 0 );
        d->mModel->insertColumn( 0 );
        if( d->mModel->index( 0, 0 ).isValid( ) ) {
            d->mModel->setData( d->mModel->index( 0, 0 ), text, Qt::DisplayRole );
            if( userData.isValid( ) ) {
                d->mModel->setData( d->mModel->index( 0, 0 ), userData, Qt::UserRole );
            }
            setCurrentIndex( 0 );
        }
    } else {
        int rowCount = d->mModel->rowCount( );
        if( index >= rowCount ) {
            d->mModel->insertRow( rowCount );
            d->mModel->setData( d->mModel->index( rowCount, 0 ), text, Qt::DisplayRole );
            if( userData.isValid( ) ) {
                d->mModel->setData( d->mModel->index( rowCount, 0 ), userData, Qt::UserRole );
            }
        } else if(index <= 0) {
            d->mModel->insertRow( 0 );
            d->mModel->setData( d->mModel->index( 0, 0 ), text, Qt::DisplayRole );
            if( userData.isValid( ) ) {
                d->mModel->setData( d->mModel->index( 0, 0 ), userData, Qt::UserRole );
            }
            if( d->mCurrentIndex.row( ) >= 0 ) {
                d->mCurrentIndex = d->mModel->index( d->mCurrentIndex.row( ) + 1, 0);
                d->currentIndexChanged( d->mCurrentIndex );
            }
        } else {
           d->mModel->insertRow( index );
           d->mModel->setData( d->mModel->index( index, 0 ), text, Qt::DisplayRole );
           if( userData.isValid( ) ) {
                d->mModel->setData( d->mModel->index( index, 0 ), userData, Qt::UserRole );
            }
           if( d->mCurrentIndex.row( ) >= index ) {
                d->mCurrentIndex = d->mModel->index( d->mCurrentIndex.row( ) + 1, 0);
                d->currentIndexChanged( d->mCurrentIndex );
            }
        }
    }
}

/*!
    @beta

    Inserts the \a text and \a icon into the combobox at the given \a index.
    If the index is equal to or higher than the total number of items, 
    the new item is appended to the list of existing items.
    If the index is zero or negative, the new item is prepended to the list of existing items.

    \sa insertItem
 */
void HbComboBox::insertItem( int index, const HbIcon &icon,
    const QString &text, const QVariant &userData )
{
    Q_D( HbComboBox );
    if( text.isEmpty( ) ) {
        return;
    }

    if( !d->mModel ) {
        QStandardItemModel* model = new QStandardItemModel( this );
        setModel( model );
    }
    if ( !d->mModel->rowCount( ) ) {
        d->mModel->insertRow( 0 );
        d->mModel->insertColumn( 0 );
        if( d->mModel->index( 0, 0 ).isValid( ) ) {
            d->mModel->setData( d->mModel->index( 0, 0 ), text, Qt::DisplayRole );
            setCurrentIndex( 0 );
        }
        if( !icon.isNull( ) && d->mModel->index( 0, 0 ).isValid( ) ) {
            d->mModel->setData( d->mModel->index( 0, 0 ), icon.qicon( ), Qt::DecorationRole );
        }
        if( userData.isValid( ) && d->mModel->index( 0, 0 ).isValid( ) ) {
            d->mModel->setData( d->mModel->index( 0, 0 ), userData, Qt::UserRole );
        }
    } else {
        int rowCount = d->mModel->rowCount( );
        if( index >= rowCount ) {
            d->mModel->insertRow( rowCount );
            d->mModel->setData( d->mModel->index( rowCount, 0 ), text, Qt::DisplayRole );
            if( !icon.isNull( ) ) {
                d->mModel->setData( d->mModel->index( rowCount, 0 ), icon, Qt::DecorationRole );
            }
            if( userData.isValid( ) ) {
                d->mModel->setData( d->mModel->index( rowCount, 0 ), userData, Qt::UserRole );
            }
            if( d->mCurrentIndex.row( ) == index ) {
                d->mCurrentIndex = d->mModel->index( d->mCurrentIndex.row( ) + 1, 0 );
                d->currentIndexChanged( d->mCurrentIndex );
            }
        } else if( index <= 0 ) {
            d->mModel->insertRow( 0 );
            d->mModel->setData( d->mModel->index( 0, 0 ), text, Qt::DisplayRole );
            if( !icon.isNull( ) ) {
                d->mModel->setData( d->mModel->index( 0, 0 ), icon, Qt::DecorationRole );
            }
            if( userData.isValid( ) ) {
                d->mModel->setData( d->mModel->index( 0, 0 ), userData, Qt::UserRole );
            }
            if( d->mCurrentIndex.row( ) >= 0 ) {
                d->mCurrentIndex = d->mModel->index( d->mCurrentIndex.row( ) + 1, 0 );
                d->currentIndexChanged( d->mCurrentIndex );
            }
        } else {
            d->mModel->insertRow( index );
            d->mModel->setData( d->mModel->index( index, 0 ), text, Qt::DisplayRole );
            if( !icon.isNull( ) ) {
                d->mModel->setData( d->mModel->index( index, 0 ), icon, Qt::DecorationRole );
            }
            if( userData.isValid( ) ) {
                d->mModel->setData( d->mModel->index( index, 0 ), userData, Qt::UserRole );
            }
            if( d->mCurrentIndex.row( ) <= index ) {
                d->mCurrentIndex = d->mModel->index( d->mCurrentIndex.row( ) + 1, 0 );
                d->currentIndexChanged( d->mCurrentIndex );
            }
        }
    }
}

/*!
    @beta

    Inserts the strings in \a texts into combobox as separate items, starting at the given \a index.
    If the index is equal to or higher than the total number of items, 
    the new item is appended to the list of existing items.
    If the index is zero or negative, the new item is prepended to the list of existing items.
 */
void HbComboBox::insertItems( int index, const QStringList &texts )
{
    Q_D( HbComboBox );
    if( !d->mModel ) {
        QStandardItemModel* model = new QStandardItemModel( this );
        setModel( model );
    }
    if ( !d->mModel->rowCount( ) ) {
        int textCount = texts.count( );
        for( int i = 0; i < textCount; i++) {
            d->mModel->insertRow( i );
            if( i == 0) {
                d->mModel->insertColumn( 0 );
            }
            if( d->mModel->index( i, 0 ).isValid( ) ) {
                d->mModel->setData( d->mModel->index( i, 0 ),  texts.at( i ), Qt::DisplayRole );
                if( i == 0) {
                    setCurrentIndex( 0 );
                }
            }
        }
    } else {
        int rowCount = -1;
        rowCount = d->mModel->rowCount( );
        int textCount = texts.count( );
        if ( index >= rowCount ) {
            d->mModel->insertRows( rowCount, textCount );
            int temp = 0;
            for ( int i = rowCount; i < ( rowCount + textCount ); i++ ) {
                d->mModel->setData ( d->mModel->index( i, 0 ), texts.at( temp++ ) );
            }
            if( d->mCurrentIndex.row( ) == index ) {
                d->mCurrentIndex = d->mModel->index( d->mCurrentIndex.row( ) + textCount, 0 );
                d->currentIndexChanged( d->mCurrentIndex );
            }
        } else if( index <= 0 ) {
            d->mModel->insertRows( 0, textCount );
            for ( int i = 0; i < textCount; i++ ) {
                d->mModel->setData( d->mModel->index( i, 0 ), texts.at( i ) );
            }
            if( d->mCurrentIndex.row( ) >= 0 ) {
                d->mCurrentIndex = d->mModel->index( d->mCurrentIndex.row( ) + textCount, 0 );
                d->currentIndexChanged( d->mCurrentIndex );
            }
        } else {
            d->mModel->insertRows( index, texts.count( ) );
            int temp = 0;
            for ( int i = index; i < ( textCount + index ); i++ ) {
                d->mModel->setData( d->mModel->index( i, 0 ), texts.at( temp++ ) );
            }
            if( d->mCurrentIndex.row( ) <= index ) {
                d->mCurrentIndex = d->mModel->index( d->mCurrentIndex.row( ) + textCount, 0 );
                d->currentIndexChanged( d->mCurrentIndex );
            }
        }
    }
}

/*!
    @beta

    Returns text at given \a index in combobox.

    \sa setItemText
 */
QString HbComboBox::itemText( int index ) const
{
    Q_D( const HbComboBox );
    if( d->mModel ) {
        QModelIndex mi = d->mModel->index( index, 0 );
        if( mi.isValid( ) ) {
            return d->itemText( mi );
        }
     }
     return QString( );
}

/*!
    @beta

    Sets the \a text for item at given \a index in combobox.

    \sa itemText
 */
void HbComboBox::setItemText( int index, const QString &text )
{
    Q_D( HbComboBox );
    if ( d->mModel ) {
        QModelIndex item = d->mModel->index( index, 0 );
        if ( item.isValid( ) ) {
            if( d->mModel->setData( item, text, Qt::EditRole ) ) {
                if( d->mCurrentIndex.row( ) == index ) {
                    if( d->mLineEdit ) {
                        d->mLineEdit->setText( text );
                    } else {
                        d->mText = text ;
                        HbStyleOptionComboBox comboBoxOption;
                        initStyleOption( &comboBoxOption );
                        style( )->updatePrimitive(
                            d->mTextItem, HbStyle::P_ComboBox_text, &comboBoxOption );
                    }
                }
            }
        }
    }
}

/*!
    reimplementation. 
*/
bool HbComboBox::eventFilter( QObject* obj, QEvent* event )
{
    Q_D( HbComboBox );
    bool accepted = false;
    if ( !isEnabled( ) ) {
        return false ;
    }
    if( obj == static_cast<HbTouchArea*>( d->mButtonTouchAreaItem ) ) {
        if( event->type( ) == QEvent::Gesture ) {
            QGestureEvent *gestureEvent = static_cast<QGestureEvent *>( event );
            if( gestureEvent->gesture( Qt::TapGesture ) ) {
                HbTapGesture *tap =
                    static_cast<HbTapGesture *>( gestureEvent->gesture( Qt::TapGesture ) );
                switch( tap->state( ) ) {
                case Qt::GestureStarted:
                    {
                        d->touchAreaPressEvent( );
                        accepted = true;
                        break;
                    }
                case Qt::GestureCanceled:
                    {
                        d->mIsDown = false;
                        updatePrimitives( );
                        setProperty( "state", "normal" );
                        accepted = true;
                        break;
                    }
                case Qt::GestureFinished:
                    {
                        d->touchAreaReleaseEvent( );
                        accepted = true;
                        break;
                        //TODO :: move else part here
                    }
                default:
                    break;
                }
            }
        }
    }
    return accepted;
}

/*!
    \reimp
 */
void HbComboBox::changeEvent( QEvent *event )
{
    switch ( event->type( ) ) {
        case QEvent::EnabledChange:
            updatePrimitives( );
            break;
        default:
            break;
    }
    HbWidget::changeEvent( event );
}

// End of file


