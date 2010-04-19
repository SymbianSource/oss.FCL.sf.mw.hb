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
#include <hbstyleoptioncombobox.h>

#include <QGraphicsSceneMouseEvent>
#include <QStandardItemModel>
#include <QDebug>

/*!
    @beta
    @hbwidgets
    \class HbComboBox

    \brief The HbComboBox provides a drop-down list that permits selecting an item.

    A HbComboBox provides a means of presenting a list of options to the user
    in a way that takes up the minimum amount of screen space.

    A combobox is a selection widget that displays the currently selected item,
    and can provides a drop-down list that permits selecting an item.
    
    A HbComboBox with currently Selected item
    \image html noneditablecombobox.png.


    A HbComboBox with drop-down list to select an item from list of options
    \image html comboboxdropdownlist.png.


    HbComboBox are of two types. 

    Editable:

    If the comboBox is set to editable, touching the ComboBox label field invokes the VKB in
    touch environments or accepts input from an attached keyboard.
    choosing an element from the ComboBox dropdown list replaces the label 
    area with the chosen item.
    on touching button area of the comboBox launches the drop-down list to select
    the list of options 
    ( button area in comboBox behaves same in both editable and Non Editable comboBox ).

    Non Editable:
    
    If the comboBox is set to Non Editable widget that displays the currently selected item
    in the label field of ComboBox.
    Touching the comboBox label field or button area of the comboBox will opens
    the drop-down list to select an item from list of options.

    By Default comboBox is Non Editable. 

    Application is responsible for setting the model to the HbComboBox,
    If no model is set, the drop down list cannot be displayed at all.

    HbComboBox owns the model set by the Application.

    HbComboBox provides three signals:

    \li currentIndexChanged( index ) is emitted when the current selection in the combobox is
    changed by user interaction. If application is setting a index/text then this signal is
    not emmitted. In case of an editable combobox on combobox loosing focus if the current
    selection has changed then this signal is emitted with the new model index.

    \li currentIndexChanged( const QString& ) is emitted when the curret selection in the combobox
    is changed by user interaction. If application is setting a differnet index/text
    then this signal is not emmitted. 
    In case of an editable combobox on combobox loosing focus if the current selection
    has changed then this signal is emitted with the new string.

    \li editTextChanged( QString& ) is emitted when combobox looses focus and user has typed a text
    for which there is no matching item found in the model with the text typed by the user.

    The following is an example of how to create a model and adding item to the created model.
    How to set the model on the HbcomboBox.
    Here the model is ownership transfer to the widget.
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,37}

    An example how to add strings into HbComboBox and setting currentIndex.
    HbComboBox will creates the model internally if model is not created.
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,38}

    An example how to add stringsList into HbComboBox and setting currentIndex.
    HbComboBox will creates the model internally if model is not created.
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,39}

    An example how to insert String at index into HbComboBox.
    HbComboBox will creates the model internally if model is not created.
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,40}

    An example how to insert StringList at index into HbComboBox.
    HbComboBox will creates the model internally if model is not created.
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,41}

    An example how to set the items into HbComboBox.
    HbComboBox will replces the existing model.
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
    setProperty("state", "normal");
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
    setProperty("state", "normal");
}

/*!
    @beta
    \property HbComboBox::items
    \brief list of comboBox items
    It replaces the existing list.
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
    if( d->mModel && d->mModel->rowCount( )) {
        QStringList list;
        int rowCount = d->mModel->rowCount();
        for(int i = 0; i < rowCount; i++)
        {
            list<<( d->mModel->data( d->mModel->index( i, 0 ) ) ).toString();
        }
        return list;
    } else {
      return QStringList();
    }
}

/*!
    @alpha
    Sets the \a icon for the item on the given \a index in the combobox.
    this API will not work if applcation sets the model as QStringlistModel.
*/
void HbComboBox::setItemIcon( int index, const HbIcon &icon )
{
    Q_D(const HbComboBox);
    if(d->mModel) {
        QModelIndex item = d->mModel->index( index, 0 );
        if ( item.isValid( ) ) {        
            d->mModel->setData( item, icon.qicon(), Qt::DecorationRole );
        }
    }
}

/*!
    @alpha
    returns the HbIcon for the given \a index in the combobox.
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
    Returns the data for the given \a role in the given \a index in the
    combobox, or QVariant::Invalid if there is no data for this role.
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
    Sets the data \a role for the item on the given \a index in the combobox
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
    @proto
    This case is valid only for Editable comboBox
    Returns the validator that is used to constraint text input to the
    combobox and returns NULL if it is invalid.

    \sa editable
*/
const HbValidator *HbComboBox::validator() const
{
    Q_D( const HbComboBox );
    if( d->mEditable) {
        return d->mLineEdit->validator( );
    }
    return NULL;
}

/*!
    @proto
    This case is only valid for the Editable comboBox.
    Sets the \a validator to use instead of the current validator.
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
    This property holds the number of items in the combobox.
    By default, for an empty combo box, this property has a value of 0.
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
    \deprecated HbComboBox::insertPolicy() const
    is deprecated.
    \sa setInsertPolicy
*/
HbComboBox::InsertPolicy HbComboBox::insertPolicy( ) const
{
    Q_D( const HbComboBox );
    qDebug() << "this is deprecated and will cease to exist in the near future.";
    return d->insertPolicy;
}

/*!
    \deprecated HbComboBox::setInsertPolicy(HbComboBox::InsertPolicy)
    is deprecated.
    \sa insertPolicy
*/
void HbComboBox::setInsertPolicy( InsertPolicy policy )
{
    Q_D( HbComboBox );
    qDebug() << "this is deprecated and will cease to exist in the near future.";
    d->insertPolicy = policy;
}

/*!
    @beta
    Sets the model to \a model 
    comboBox Owns the model set by the Application.
    It replaces the old model if exists.
    please do not pass 0(NULL) as \a model instead call clear() to clear the contens of the model.
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
    Returns model that view is currently presenting.
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
    Returns index of current item and returns -1 for invalid current index.
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
    Returns the index of the item containing the given \a data for the
    given \a role; otherwise returns -1.

    The \a flags specify how the items in the combobox are searched.
*/
int HbComboBox::findData( const QVariant &data, int role, Qt::MatchFlags flags ) const
{
    Q_D( const HbComboBox );
    if(d->mModel) {
        QModelIndexList result;
        QModelIndex start = d->mModel->index( 0, 0 );
        result = d->mModel->match( start, role, data, 1, flags );
        if ( result.isEmpty( ) )
            return -1;
        return result.first( ).row( );
    }
    return -1;
}

/*!
    @beta
    This is specific to the Editable comboBox only.
    Sets the text in the combobox's text edit 
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
    \brief the text of the current item
     combo box with model count \0 returns an empty string.
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
    Removes the item at the given index from the combobox.
    This will update the current index if the index is removed. 
 */
void HbComboBox::removeItem( int index )
{
    Q_D( HbComboBox );
    if( d->mModel ) {
        int rowCount = d->mModel->rowCount( );
        if( index >=0 && index < rowCount ) {
            bool currentText = false;
            if ( d->mModel->index(index, 0) == d->mCurrentIndex ) {
                currentText = true;
                d->mModel->removeRow( index );
            } else if( d->mCurrentIndex.row() > index ) {
                 int row = d->mCurrentIndex.row();
                 d->mModel->removeRow( index );
                 d->mCurrentIndex = d->mModel->index( --row, 0 );
                 d->currentIndexChanged( d->mCurrentIndex );
            } else {
                d->mModel->removeRow( index );
            }
            if( d->mModel->rowCount( ) == 0 ) {
                if( d->mEditable ) {
                    clearEditText();
                } else {
                    if( d->mLineEdit ) {
                        d->mLineEdit->setText( QString() );
                    } else {
                        d->mText = QString( );
                        HbStyleOptionComboBox comboBoxOption;
                        initStyleOption(&comboBoxOption);
                        style()->updatePrimitive( d->mTextItem, HbStyle::P_ComboBox_text, &comboBoxOption);
                    }
                }
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
                        d->mLineEdit->setText( QString() );
                    } else {
                        d->mText = d->mModel->data( d->mCurrentIndex ).toString( );
                        HbStyleOptionComboBox comboBoxOption;
                        initStyleOption(&comboBoxOption);
                        style()->updatePrimitive( d->mTextItem, HbStyle::P_ComboBox_text, &comboBoxOption);
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
    if ( d->mDropDown && d->mDropDown->isVisible() ){
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
    \reimp
 */
QGraphicsItem* HbComboBox::primitive( HbStyle::Primitive primitive ) const
{
    Q_D( const HbComboBox );

    switch( primitive ){
        case HbStyle::P_ComboBox_background:
            return d->mBackgroundItem;
        case HbStyle::P_ComboBoxButton_toucharea:
            return d->mButtonTouchAreaItem;
        default:
            return 0;
    }
}

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
    Clears the combobox, removes all items from model. 
 */
void HbComboBox::clear( )
{
    Q_D( HbComboBox );
    if( d->mModel ) {
        d->mModel->removeRows(0, d->mModel->rowCount());
        d->mCurrentIndex = QModelIndex( );
        if( d->mEditable ) {
            clearEditText();
        } else {
            if( d->mLineEdit ) {
                d->mLineEdit->setText( QString() );
            } else {
                d->mText = QString( );
                HbStyleOptionComboBox comboBoxOption;
                initStyleOption(&comboBoxOption);
                style()->updatePrimitive( d->mTextItem, HbStyle::P_ComboBox_text, &comboBoxOption);
            }
        }
    }
}

/*!
    @beta
    Clears the contents of the line edit used for editing in the combobox.
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
    \brief Set editable the property of the combobox.
    True, Editable, user can type in the combobox to search for items from 
    the list of items. Shows the prediction. If user types a text which does not match
    to any items in the model then slection is not changed.
    False, Non editable user canot enter text using keyboard.
 */
void HbComboBox::setEditable( bool editable )
{
    Q_D( HbComboBox ); 
    d->setEditable( editable );
    setProperty("state", "normal");
}

/*!
    @beta
    Returns whether or not the combobox is editable
    True if editable, else false
 */
bool HbComboBox::isEditable( ) const
{
    Q_D( const HbComboBox );
    return d->mEditable;
}

/*!
    @beta
    Adds an item to the combobox with the given text,
    and containing the specified userData (stored in the Qt::UserRole).
    The item is appended to the list of existing items.
 */
void HbComboBox::addItem( const QString &text, const QVariant &userData)
{
    insertItem( count( ), text, userData );
}

/*!
    @beta
    Adds an item to the combobox with the given icon and text,
    and containing the specified userData (stored in the Qt::UserRole). 
    The item is appended to the list of existing items.
 */
void HbComboBox::addItem( const HbIcon &icon, const QString &text, const QVariant &userData )
{
    insertItem( count( ), icon, text, userData );
}

/*!
    @beta
    Adds each of the strings in the given texts to the combobox.
    Each item is appended to the list of existing items in turn.
 */
void HbComboBox::addItems( const QStringList &texts )
{
    insertItems( count(), texts );
}


/*!
    @beta
    Inserts the text into the combobox at the given index.
    If the index is equal to or higher than the total number of items, 
    the new item is appended to the list of existing items.
    If the index is zero or negative, the new item is prepended to the list of existing items.
 */
void HbComboBox::insertItem( int index, const QString &text, const QVariant &userData )
{
    Q_D( HbComboBox );
    if( text.isNull( ) ){
        return;
    }
    if( !d->mModel ) {
        QStandardItemModel* model = new QStandardItemModel(this );
        setModel( model );
    }
    if (  !d->mModel->rowCount( ) ) {
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
            d->mModel->setData( d->mModel->index(0,0), text, Qt::DisplayRole );
            if( userData.isValid( ) ) {
                d->mModel->setData( d->mModel->index( 0, 0 ), userData, Qt::UserRole );
            }
            if( d->mCurrentIndex.row() >= 0 ) {
                d->mCurrentIndex = d->mModel->index(d->mCurrentIndex.row()+1, 0);
                d->currentIndexChanged(d->mCurrentIndex);
            }
        } else {
           d->mModel->insertRow( index );
           d->mModel->setData( d->mModel->index( index, 0 ), text, Qt::DisplayRole );
           if( userData.isValid( ) ) {
                d->mModel->setData( d->mModel->index( index, 0 ), userData, Qt::UserRole );                
            }
           if( d->mCurrentIndex.row() <= index ) {
                d->mCurrentIndex = d->mModel->index(d->mCurrentIndex.row()+1, 0);
                d->currentIndexChanged(d->mCurrentIndex);
            }
        }
    }
}

/*!
    @beta
    Inserts the text and icon into the combobox at the given index.
    If the index is equal to or higher than the total number of items, 
    the new item is appended to the list of existing items.
    If the index is zero or negative, the new item is prepended to the list of existing items.
 */
void HbComboBox::insertItem( int index, const HbIcon &icon, const QString &text, const QVariant &userData)
{
    Q_D( HbComboBox );
    if( text.isEmpty( ) ){
        return;
    }

    if( !d->mModel ) {
        QStandardItemModel* model = new QStandardItemModel( this );
        setModel( model );
    }
    if (  !d->mModel->rowCount( ) ) {
        d->mModel->insertRow( 0 );
        d->mModel->insertColumn( 0 );
        if( d->mModel->index( 0, 0 ).isValid( ) ) {
            d->mModel->setData( d->mModel->index(0,0), text, Qt::DisplayRole );
            setCurrentIndex( 0 );
        }
        if(!icon.isNull() && d->mModel->index( 0, 0 ).isValid( ) ) {
            d->mModel->setData(d->mModel->index( 0, 0 ), icon.qicon( ), Qt::DecorationRole);
        }
        if( userData.isValid( ) && d->mModel->index( 0, 0 ).isValid( ) ) {
            d->mModel->setData( d->mModel->index( 0, 0 ), userData, Qt::UserRole );                
        }
    } else {
        int rowCount = d->mModel->rowCount( );
        if( index >= rowCount ) {
            d->mModel->insertRow( rowCount );
            d->mModel->setData( d->mModel->index( rowCount, 0 ), text, Qt::DisplayRole );
            if(!icon.isNull()) {
                d->mModel->setData( d->mModel->index( rowCount, 0 ), icon, Qt::DecorationRole );
            }
            if( userData.isValid( ) ) {
                d->mModel->setData( d->mModel->index( rowCount, 0 ), userData, Qt::UserRole );
            }
            if( d->mCurrentIndex.row() == index ) {
                d->mCurrentIndex = d->mModel->index( d->mCurrentIndex.row( ) + 1, 0 );
                d->currentIndexChanged( d->mCurrentIndex );
            }
        } else if(index <= 0) {
            d->mModel->insertRow( 0 );
            d->mModel->setData( d->mModel->index( 0, 0 ), text, Qt::DisplayRole );
            if(!icon.isNull()) {
                d->mModel->setData(d->mModel->index( 0, 0 ), icon, Qt::DecorationRole);
            }
            if( userData.isValid( ) ) {
                d->mModel->setData( d->mModel->index( 0, 0 ), userData, Qt::UserRole );
            }
            if( d->mCurrentIndex.row() >= 0 ) {
                d->mCurrentIndex = d->mModel->index( d->mCurrentIndex.row( ) + 1, 0);
                d->currentIndexChanged( d->mCurrentIndex );
            }
        } else {
            d->mModel->insertRow( index );
            d->mModel->setData( d->mModel->index( index, 0 ), text, Qt::DisplayRole );
            if(!icon.isNull( ) ) {
                d->mModel->setData( d->mModel->index( index, 0 ), icon, Qt::DecorationRole );
            }
            if( userData.isValid( ) ) {
                d->mModel->setData( d->mModel->index( index, 0 ), userData, Qt::UserRole );
            }
            if( d->mCurrentIndex.row() <= index ) {
                d->mCurrentIndex = d->mModel->index( d->mCurrentIndex.row( ) + 1, 0);
                d->currentIndexChanged( d->mCurrentIndex );
            }
        }
    }
}

/*!
    @beta
    Inserts the strings from the list into the combobox as separate items, starting at the index.
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
        for( int i = 0; i < textCount; i++)
        {
            d->mModel->insertRow( i );
            if( i == 0)
                d->mModel->insertColumn( 0 );
            if( d->mModel->index( i, 0 ).isValid( ) ) {
                d->mModel->setData( d->mModel->index( i, 0 ),  texts.at( i ), Qt::DisplayRole );
                if( i == 0) {
                    setCurrentIndex( 0 );
                }
            }
        }
    }  else {
        int rowCount = -1;
        rowCount = d->mModel->rowCount( );
        int textCount = texts.count( );
        if ( index >= rowCount ) {
            d->mModel->insertRows( rowCount, textCount );
            int temp = 0;
            for ( int i = rowCount; i < ( rowCount + textCount ); i++ ) {
                d->mModel->setData ( d->mModel->index( i, 0 ), texts.at( temp++ ) );
            }
            if( d->mCurrentIndex.row() == index ) {
                d->mCurrentIndex = d->mModel->index( d->mCurrentIndex.row( ) + textCount, 0);
                d->currentIndexChanged( d->mCurrentIndex );
            }
        } else if( index <= 0 ) {
            d->mModel->insertRows( 0, textCount );
            for ( int i = 0; i < textCount; i++ ) {
                d->mModel->setData( d->mModel->index( i, 0 ), texts.at( i ) );
            }
            if( d->mCurrentIndex.row() >= 0 ) {
                d->mCurrentIndex = d->mModel->index(d->mCurrentIndex.row() + textCount, 0);
                d->currentIndexChanged( d->mCurrentIndex );
            }
        } else {
            d->mModel->insertRows( index, texts.count( ) );
            int temp = 0;
            for ( int i = index; i < ( textCount + index ); i++ ) {
                d->mModel->setData( d->mModel->index( i, 0 ), texts.at( temp++ ) );
            }
            if( d->mCurrentIndex.row() <= index ) {
                d->mCurrentIndex = d->mModel->index(d->mCurrentIndex.row() + textCount, 0);
                d->currentIndexChanged( d->mCurrentIndex );
            }
        }
    }
}

/*!
    @beta
    Returns the text for the given index in the combobox.
 */
QString HbComboBox::itemText( int index ) const
{
    Q_D( const HbComboBox );
    if(d->mModel) {
        QModelIndex mi = d->mModel->index( index, 0 );
        if( mi.isValid() ) {
            return d->itemText( mi );
        }
     }
     return QString( );
}

/*!
    @beta
    Sets the text for the item on the given index in the combobox.
 */
void HbComboBox::setItemText( int index, const QString &text )
{
    Q_D( HbComboBox );
    if (d->mModel) {
        QModelIndex item = d->mModel->index( index, 0 );
        if ( item.isValid( ) ) {
            if(d->mModel->setData( item, text, Qt::EditRole )) {
                if(d->mCurrentIndex.row() == index) {
                    if( d->mLineEdit ) {
                        d->mLineEdit->setText( text );
                    } else {                    
                        d->mText = text ;
                        HbStyleOptionComboBox comboBoxOption;
                        initStyleOption(&comboBoxOption);
                        style()->updatePrimitive( d->mTextItem, HbStyle::P_ComboBox_text, &comboBoxOption);
                    }
                }
            }
            
        }
    }
}

/*!
    reimplementation. 
*/
bool HbComboBox::eventFilter( QObject *obj, QEvent *event )
{
    Q_D( HbComboBox );
    bool accepted = false;  
    if(obj == static_cast<HbTouchArea*>(d->mButtonTouchAreaItem)) {
        if( (event->type() == QEvent::GraphicsSceneMousePress ) || 
                (event->type() == QEvent::GraphicsSceneMouseDoubleClick ) ) {
            if( static_cast<HbTouchArea*>(d->mButtonTouchAreaItem)->rect( ).contains( 
                        static_cast<QGraphicsSceneMouseEvent *>( event )->pos( ) ) ){
                d->touchAreaPressEvent( );
                accepted = true;
            }
        }else if( event->type() == QEvent::GraphicsSceneMouseRelease ) {
             if( static_cast<HbTouchArea*>(d->mButtonTouchAreaItem)->rect( ).contains(
                        static_cast<QGraphicsSceneMouseEvent *>( event )->pos( ) ) ){
                    d->touchAreaReleaseEvent( );
                    accepted = true;
             } else if (d->mIsDown) {
                 d->mIsDown = false;
                 updatePrimitives( );
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
    switch ( event->type( ) ){
        case QEvent::EnabledChange:
            updatePrimitives( );
            break;
        default:
            break;
    }
    HbWidget::changeEvent( event );
}

// End of file


