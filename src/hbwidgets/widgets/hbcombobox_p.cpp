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

#include <hblistview.h>
#include <hbabstractviewitem.h>
#include <hbdeviceprofile.h>
#include <hbtoucharea.h>
#include <hbmainwindow.h>
#include <hbview.h>
#include <hbtextitem.h>
#include <hbstyleoptioncombobox.h>

#include <QSortFilterProxyModel>
#include <QGraphicsScene>
#include <QCompleter>
#include <QItemSelectionModel>

#ifdef HB_EFFECTS
#include <hbeffect.h>
#include "hbeffectinternal_p.h"
#define HB_DROPD0WN_ITEM_TYPE "HB_DROPDOWN"
#endif

HbComboBoxPrivate::HbComboBoxPrivate( ):
    HbWidgetPrivate (  ),
    mLineEdit ( 0 ),
    mTextItem ( 0 ),
    mButton ( 0 ),
    mDropDown ( 0 ),
    mModel ( 0 ),
    mProxyModel ( 0 ),
    mCompleter ( 0 ),
    insertPolicy ( HbComboBox::InsertAtBottom ),
    mBackgroundItem ( 0 ),
    mButtonTouchAreaItem ( 0 ),
    mIsDown ( false ),
    mEditable ( false ),
    mIsDorpdownCreated(false),
    mIsDropwnToSceneAdded(false),
    mHasDownEffect ( false ),
    mHasUpEffect (false ),
    mListItemHeight(-1)
{
}

HbComboBoxPrivate::~HbComboBoxPrivate( )
{
    Q_Q(HbComboBox);
    if( mButtonTouchAreaItem ) {
        static_cast<HbTouchArea*>(mButtonTouchAreaItem)->removeEventFilter( q );
    }
    if (!q->scene() || !q->scene()->property("destructed").isValid()) {
        if( mDropDown ) {
            delete mDropDown;
            mDropDown = 0;
        }
    }
}

void HbComboBoxPrivate::init( )
{
    createPrimitives( );
}

void HbComboBoxPrivate::createPrimitives( )
{
    Q_Q( HbComboBox );

    mTextItem = q->style()->createPrimitive( HbStyle::P_ComboBox_text, q );
    HbStyle::setItemName( mTextItem, "combobox_labelfield" );

    mBackgroundItem = q->style( )->createPrimitive( HbStyle::P_ComboBox_background, q );
    HbStyle::setItemName( mBackgroundItem, "text_background" );

    mButton = q->style( )->createPrimitive( HbStyle::P_ComboBox_button, q );
    HbStyle::setItemName( mButton, "combobox_button" );

    mButtonTouchAreaItem = q->style( )->createPrimitive( 
                                            HbStyle::P_ComboBoxButton_toucharea, q );
    static_cast<HbTouchArea*>(mButtonTouchAreaItem)->installEventFilter( q );
    q->setHandlesChildEvents(true);
}

void HbComboBoxPrivate::touchAreaPressEvent( )
{
    Q_Q( HbComboBox );
    if (q->count() > 0) {
        HbWidgetFeedback::triggered(q, Hb::InstantPressed);
    }
    mIsDown = true;
    q->updatePrimitives( );
    q->setProperty("state", "pressed"); 
}

void HbComboBoxPrivate::touchAreaReleaseEvent(  )
{
    Q_Q( HbComboBox );
    mIsDown = false;
    touchAreaClicked();
    q->updatePrimitives( );
    if (q->count() > 0) {
        HbWidgetFeedback::triggered(q, Hb::InstantReleased);
    }

    q->setProperty("state", "normal"); 
}

void HbComboBoxPrivate::touchAreaClicked( )
{
    Q_Q( HbComboBox );
    if ( mModel && mModel->rowCount( ) ) {
        addDropDownToScene();
        if( !mDropDown->mList ) {
            mDropDown->createList( );
            mDropDown->mList->setModel( mModel );            
            q->connect( mDropDown->mList, SIGNAL( activated( QModelIndex ) ), q,
                        SLOT( _q_textChanged( QModelIndex ) ) );
        }
        if ( mCurrentIndex.isValid( ) ) {
            if( mDropDown->mList->model( ) != mModel ) {
                mDropDown->mList->setModel( mModel );
            }
            mDropDown->mList->scrollTo( mCurrentIndex, HbAbstractItemView::PositionAtTop );
            mDropDown->mList->setCurrentIndex( mCurrentIndex, QItemSelectionModel::Select );
        } else {
            if( mDropDown->mList->model( ) != mModel ) {
                mDropDown->mList->setModel( mModel );
            }
            mDropDown->mList->scrollTo( mModel->index( 0, 0 ) );
            mDropDown->mList->setCurrentIndex(mModel->index( 0, 0 ), QItemSelectionModel::Select);
        }
        #ifdef HB_EFFECTS
               HbEffect::start(mDropDown, HB_DROPD0WN_ITEM_TYPE, "appear");
        #endif
        positionDropDown( );
        mDropDown->setVisible( true );
    }
}

void HbComboBoxPrivate::vkbOpened( )
{

}

void HbComboBoxPrivate::vkbClosed()
{
    if( mDropDown->isVisible()) {
        positionDropDown();
    }
}

void HbComboBoxPrivate::showPopup( QAbstractItemModel* aModel, QModelIndex aIndex )
{    
    Q_UNUSED( aModel );
    Q_UNUSED( aIndex );
    Q_Q( HbComboBox );
    if ( aModel && aModel->rowCount( ) ) {
        addDropDownToScene();
        if( !mDropDown->mList ) {
            mDropDown->createList();            
            q->connect( mDropDown->mList, SIGNAL( activated( QModelIndex ) ), q,
                        SLOT( _q_textChanged( QModelIndex ) ) );
        }
        mDropDown->mList->setModel( aModel );
        if ( aIndex.isValid( ) ) {
            mDropDown->mList->scrollTo( aIndex, HbAbstractItemView::PositionAtTop );
            mDropDown->mList->setCurrentIndex(mCurrentIndex, QItemSelectionModel::Select);
        } else {
            mDropDown->mList->scrollTo( aModel->index( 0, 0 ) );
        }
        positionDropDown( );
        mDropDown->setVisible( true );
    }
}

void HbComboBoxPrivate::createDropDown()
{    
    if( !mIsDorpdownCreated ) {
        mDropDown = new HbComboDropDown( this );
        mIsDorpdownCreated = true;
        mDropDown->setVisible(false);
    }
}

void HbComboBoxPrivate::calculateListItemHeight()
{
    if( mListItemHeight == -1 ) {
        QAbstractItemModel *model = mDropDown->mList->model( );
        if( mCurrentIndex.isValid( ) && mDropDown->mList->itemByIndex( mCurrentIndex ) ) {
            mListItemHeight = mDropDown->mList->itemByIndex( mCurrentIndex )->geometry( ).height( );
        } else if( model->index( 0, 0 ).isValid() && mDropDown->mList->itemByIndex( model->index( 0, 0 ) ) ) {
            mListItemHeight = mDropDown->mList->itemByIndex( model->index( 0, 0 ) )->geometry( ).height( );
        } else {
            HbListViewItem *proto = mDropDown->mList->listItemPrototype();
            HbListViewItem *temp = static_cast<HbListViewItem*>(proto->createItem());
            mListItemHeight = temp->effectiveSizeHint(Qt::PreferredSize).height();
            delete temp;
            temp = 0;
        }
    }
}

void HbComboBoxPrivate::positionDropDown( )
{
    Q_Q( HbComboBox );
    QRectF popupRect;
    QRectF sceneRect( QPointF( ), HbDeviceProfile::profile( q ).logicalSize( ) );
    QPointF widgetPos = q->scenePos( );
    QAbstractItemModel *model = mDropDown->mList->model( );
    calculateListItemHeight();
    qreal totalHeightRequd = model->rowCount( ) * mListItemHeight;
    qreal maxPopupHeight = 0.0;
    if(q->mainWindow()->orientation() == Qt::Horizontal ) {
        maxPopupHeight = 5 * mListItemHeight;
    } else if(q->mainWindow()->orientation() == Qt::Vertical ) {
        maxPopupHeight = 8 * mListItemHeight;
    }
    if ( totalHeightRequd < maxPopupHeight ) {
        maxPopupHeight = totalHeightRequd;
    }
    QSizeF popupSize = QSizeF( q->rect( ).width( ), maxPopupHeight );
    QPointF popupPos;
    if( !mDropDown->vkbOpened ) {
        //position of drop down in both editable and non-editable combobox depends upon
        //the available space above and below combobox
        if( (widgetPos.y( ) + q->rect( ).height( ) + maxPopupHeight) < sceneRect.height( ) ) {
            popupPos = QPointF( widgetPos.x(), widgetPos.y( )+ q->rect( ).height( ) );
            #ifdef HB_EFFECTS
                if ( !mHasDownEffect ) {
                     mHasDownEffect = true;
                     mHasUpEffect = false;
                     // this is temporary until proper effect theming comes.
                     //this Effect will be shown when there is space in the view bottom.
                     HbEffectInternal::add( mDropDown, "combo_appear_down", "appear" );
                     HbEffectInternal::add( mDropDown, "combo_disappear_downl", "disappear" );
                }
            #endif
        } else if( widgetPos.y() - maxPopupHeight  > 0.0 ) {
            popupPos = QPointF( widgetPos.x(), widgetPos.y()-maxPopupHeight );
            #ifdef HB_EFFECTS
                if ( !mHasUpEffect ) {
                     // this is temporary until proper effect theming comes.
                     //this Effect will be shown when there is no space in the view bottom
                     mHasUpEffect = true;
                     mHasDownEffect = false;
                     HbEffectInternal::add( mDropDown, "combo_appear_up", "appear" );
                     HbEffectInternal::add( mDropDown,  "combo_disappear_up", "disappear" );
                }
            #endif
        } else {
            qreal topScreenHeight = sceneRect.height( ) - maxPopupHeight;
            if( topScreenHeight > sceneRect.height( ) - topScreenHeight ) {
                popupPos = QPointF( widgetPos.x( ), 0.0 );
                #ifdef HB_EFFECTS
                    if ( !mHasDownEffect ) {
                         mHasDownEffect = true;
                         mHasUpEffect = false;
                     // this is temporary until proper effect theming comes.
                     //this Effect will be shown when there is more space in the view bottom.
                         HbEffectInternal::add( mDropDown, "combo_appear_down", "appear" );
                         HbEffectInternal::add( mDropDown, "combo_disappear_down", "disappear" );
                    }
                #endif
            } else {
                popupPos = QPointF( widgetPos.x( ), sceneRect.height( ) - maxPopupHeight );
                #ifdef HB_EFFECTS
                    if ( !mHasUpEffect ) {
                         mHasUpEffect = true;
                         mHasDownEffect = false;
                         // this is temporary until proper effect theming comes.
                         //this Effect will be shown when there is more space in the view bottom.
                         HbEffectInternal::add( mDropDown, "combo_appear_up", "appear" );
                         HbEffectInternal::add( mDropDown, "combo_disappear_up", "disappear" );
                    }
                #endif
            }
        }
    } else {
        // positioning drop down when vkb is positioned
        // drop down will come on top/below of combo based upon which side has more space
        // available 
    
        HbEditorInterface editorInterface(q);
        HbVkbHost *host = editorInterface.vkbHost();
        if ( host ) {
            QSizeF keyBoardArea = host->keyboardArea();
            QSize screenSize = HbDeviceProfile::profile(q).logicalSize();
            
            qreal heightDifference = screenSize.height() - keyBoardArea.height();
            qreal topSpace = widgetPos.y();
            qreal bottomSpace = heightDifference - topSpace - q->boundingRect().height();

            if( topSpace > bottomSpace ) {
                //display drop down at top
                if( widgetPos.y() - maxPopupHeight  > 0.0 ) {
                    popupPos = QPointF( widgetPos.x(), widgetPos.y() - maxPopupHeight );
                } else {
                    popupPos = QPointF( widgetPos.x(), 0.0 );
                    popupSize.setHeight( topSpace );
                }
                #ifdef HB_EFFECTS
                    if ( !mHasUpEffect ) {
                         mHasUpEffect = true;
                         mHasDownEffect = false;
                         // this is temporary until proper effect theming comes.
                         //this Effect will be shown when there is more space in the view bottom.
                         HbEffectInternal::add( mDropDown, "combo_appear_up", "appear" );
                         HbEffectInternal::add( mDropDown, "combo_disappear_up", "disappear" );
                    }
                #endif
                
            } else {
                //display drop down at bottom
                popupPos = QPointF( widgetPos.x(), widgetPos.y( ) + q->rect( ).height( ) );
                if( bottomSpace < maxPopupHeight ) {
                    popupSize.setHeight( bottomSpace );
                }
                #ifdef HB_EFFECTS
                    if ( !mHasDownEffect ) {
                         mHasDownEffect = true;
                         mHasUpEffect = false;
                     // this is temporary until proper effect theming comes.
                     //this Effect will be shown when there is more space in the view bottom.
                         HbEffectInternal::add( mDropDown, "combo_appear_down", "appear" );
                         HbEffectInternal::add( mDropDown, "combo_disappear_down", "disappear" );
                    }
                #endif
            }
        }
    }
    mDropDown->setPreferredSize( popupSize );
    mDropDown->setMinimumSize( popupSize );
    mDropDown->setMaximumSize( popupSize );
    mDropDown->setPos(popupPos);
    QGraphicsWidget* p = q;
    while(p->parentWidget()) {
        p = p->parentWidget();
    }
    mDropDown->setZValue( p->zValue( ) + 1 );
}

void HbComboBoxPrivate::_q_textChanged( const QModelIndex & aIndex )
{
    Q_Q( HbComboBox );
    QVariant data = mDropDown->mList->model( )->data( aIndex );
    mText = data.toString( );
    if( !mEditable ) {        
        if( mLineEdit ) {
            mLineEdit->setText( mText );
        } else {
            HbStyleOptionComboBox comboBoxOption;
            q->initStyleOption(&comboBoxOption);
            q->style()->updatePrimitive( mTextItem, HbStyle::P_ComboBox_text, &comboBoxOption);
        }
        mCurrentIndex = aIndex;
    } else {
       q->disconnect( mLineEdit, SIGNAL( textChanged ( QString ) ), q,
            SLOT( _q_textChanged( QString ) ) );       
       mLineEdit->setText( mText );
       mCurrentIndex = findData( mText );
       q->connect( mLineEdit, SIGNAL( textChanged ( QString ) ), q, 
            SLOT( _q_textChanged( QString ) ) );
    }
    if ( mDropDown->isVisible( ) ) {
        mDropDown->setVisible( false );
    }
    currentIndexChanged( mCurrentIndex );
}

void HbComboBoxPrivate::_q_textCompleted( const QModelIndex & aIndex )
{    
    if( aIndex.isValid( ) ) {
        showPopup( mCompleter->completionModel( ) );
    }
}

void HbComboBoxPrivate::_q_textChanged( const QString & aString )
{
    Q_Q(HbComboBox);

    if( !aString.isEmpty( ) ) {
        if ( mCompleter ) {
            mCompleter->setCompletionPrefix( aString );
            mCompleter->complete( );
            if( mCompleter->currentRow() == -1 )
            {
                if (( mDropDown ) && ( mDropDown->isVisible() )) {
                    mDropDown->setVisible(false);
                }
            }
        }
    } else {
        showPopup( mModel, mCurrentIndex);
    }
    emit q->editTextChanged( aString );
}

void HbComboBoxPrivate::setModel( QAbstractItemModel * model )
{
    Q_Q ( HbComboBox );
    createDropDown( );    
    if ( mDropDown->isVisible( ) ) {
        mDropDown->setVisible( false );
    }
    q->clear( );
    delete mModel;
    mModel = model;
    mModel->setParent( q );
    setCompletion( mEditable );
    if( mModel->rowCount( ) ) {
        q->setCurrentIndex( 0 );
    }
}

void HbComboBoxPrivate::setCompletion( bool completion )
{
    Q_Q( HbComboBox );
    if ( completion ) {
        if ( mCompleter ) {
            mProxyModel->setSourceModel( mModel );
            mProxyModel->sort( Qt::AscendingOrder );
            mCompleter->setModel( mProxyModel );
        } else {
            mProxyModel = new QSortFilterProxyModel( q );
            mProxyModel->setDynamicSortFilter( true );
            mProxyModel->setSortCaseSensitivity( Qt::CaseInsensitive );
            mProxyModel->setSourceModel( mModel );
            mProxyModel->sort( Qt::AscendingOrder );
            mCompleter = new QCompleter( mProxyModel, q );
            mCompleter->setCaseSensitivity( Qt::CaseInsensitive );
            mCompleter->setCompletionRole( Qt::DisplayRole );
            mCompleter->setCompletionMode( QCompleter::InlineCompletion );
            q->connect( mCompleter, SIGNAL( highlighted( QModelIndex ) ), q, 
                SLOT( _q_textCompleted( QModelIndex ) ) );
        }
    } else {
        if ( mCompleter ) {
            delete mCompleter;
            mCompleter = 0;
        }
        if ( mProxyModel ) {
            delete mProxyModel;
            mProxyModel = 0;
        }
    }
}

void HbComboBoxPrivate::setEditable(  bool editable )
{
    Q_Q(HbComboBox);
    if( mEditable == editable ) {
        return;
    }
    mEditable = editable;
    if( editable )
    {        
        if( mTextItem ) {
            HbStyle::setItemName( mTextItem, "" );
            delete mTextItem;
            mTextItem = 0;
            mLineEdit = new HbCustomLineEdit( q, this );
            HbStyle::setItemName( mLineEdit, "combobox_labelfield" );
            mLineEdit->backgroundItem()->setVisible(false);
        }
        q->setHandlesChildEvents( false );
        mLineEdit->setReadOnly( false );
        mLineEdit->setCursorVisibility( Hb::TextCursorVisible );
        mLineEdit->setLongPressEnabled( );
        q->repolish( );
        q->connect( mLineEdit, SIGNAL( textChanged ( QString ) ),
            q, SLOT( _q_textChanged( QString ) ) );
        setCompletion( true );
    } else {
        q->disconnect( mLineEdit, SIGNAL( textChanged ( QString ) ),
            q, SLOT( _q_textChanged( QString ) ) );
        q->setHandlesChildEvents( true );
        mLineEdit->setReadOnly( true );
        mLineEdit->setLongPressEnabled( false );
        setCompletion( false );
        mLineEdit->setCursorVisibility( Hb::TextCursorHidden );
        if( mModel && mModel->rowCount( ) ) {
            QModelIndex mi = mModel->index( 0, 0 );
            if( mi.isValid( ) ) {
                mCurrentIndex = QModelIndex( mi );
                mText = q->itemText( mCurrentIndex.row( ) );
                mLineEdit->setText( mText );
            }
        }
        q->repolish( );
    }
}

QString HbComboBoxPrivate::itemText( const QModelIndex &index ) const
{
    return mModel->data( index, itemRole( ) ).toString( );
}

QIcon HbComboBoxPrivate::itemIcon( const QModelIndex &index ) const
{
    QVariant decoration = mModel->data( index, Qt::DecorationRole );
    if ( decoration.type() == QVariant::Icon ) {
        return QIcon( qvariant_cast<QIcon>( decoration ) );
    }
    return qvariant_cast<QIcon>( decoration );
}

int HbComboBoxPrivate::itemRole( ) const
{
    return q_func( )->isEditable( ) ? Qt::EditRole : Qt::DisplayRole;
}

void HbComboBoxPrivate::addDropDownToScene( )
{    
    Q_Q( HbComboBox );
    if( !mIsDropwnToSceneAdded ) {
        if ( q->scene( ) ) {
            q->scene( )->addItem( mDropDown );
        }
        QGraphicsScene *scene1 = mDropDown->scene( );
        if( scene1 )
        {
            scene1->installEventFilter( mDropDown );
        }
        mIsDropwnToSceneAdded = true;
    }
}
void HbComboBoxPrivate::setCurrentIndex( const QModelIndex &mi )
{
    Q_Q( HbComboBox );
    bool indexChanged = ( mi != mCurrentIndex );
    if ( indexChanged ) {
        mCurrentIndex = QModelIndex( mi );
        mText = q->itemText( mCurrentIndex.row( ) );         
        if( mEditable ) {
            q->disconnect( mLineEdit, SIGNAL( textChanged ( QString ) ), q,
                SLOT( _q_textChanged( QString ) ) );
             mLineEdit->setText( mText );
             q->connect( mLineEdit, SIGNAL( textChanged ( QString ) ), 
                          q, SLOT( _q_textChanged( QString ) ) );
        } else {            
            if( mLineEdit ) {
                mLineEdit->setText( mText );
            } else {                
                HbStyleOptionComboBox comboBoxOption;
                q->initStyleOption(&comboBoxOption);
                q->style()->updatePrimitive( mTextItem, HbStyle::P_ComboBox_text, &comboBoxOption);
            }
        }
        currentIndexChanged( mCurrentIndex );
    }
}

void HbComboBoxPrivate::currentIndexChanged( const QModelIndex &index )
{
    Q_Q( HbComboBox );
    emit q->currentIndexChanged( index.row( ) );    
    emit q->currentIndexChanged( q->itemText ( mCurrentIndex.row( ) ) );
}

/*
 Returns the index of the item containing the given \a data for the
 given \a role; otherwise returns QModelIndex.

 The \a flags specify how the items in the combobox are searched.
 */
QModelIndex HbComboBoxPrivate::findData( const QVariant &data ) const
{
    QModelIndexList result;
    if ( mModel ) {
        QModelIndex start = mModel->index( 0, 0 );
        result = mModel->match(
            start, Qt::DisplayRole, data, 1, Qt::MatchExactly|Qt::MatchCaseSensitive );
    }
    if ( result.isEmpty( ) ) {
        return QModelIndex( );
    } else {
        return result.first( );
    }
}

#include "moc_hbcombobox.cpp"


