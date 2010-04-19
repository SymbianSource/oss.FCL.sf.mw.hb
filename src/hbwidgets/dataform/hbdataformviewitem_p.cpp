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


#include "hbdataformviewitem_p.h"
#include "hbdataformmodel.h"
#include "hbdataform_p.h"
#include "hbdataformheadingwidget_p.h"
#include "hbdatagroup_p.h"
#include "hbdatagroup_p_p.h"

#include <hbslider.h>
#include <hbcheckbox.h>
#include <hblabel.h>
#include <hblineedit.h>
#include <hbradiobuttonlist.h>
#include <hbcombobox.h>
#include <hbstyleoptiondataformviewitem.h>
#include <hblistdialog.h>
#include <hbpushbutton.h>
#include <hbaction.h>

#include <QGraphicsLinearLayout>
#include <QCoreApplication>

HbToggleItem::HbToggleItem( QGraphicsItem* parent ): HbWidget( parent )
{
    // Toggle item uses button as the content widget and toggles the text when item is clicked
    mViewItem = static_cast<HbDataFormViewItem*>( parent );
    mButton = new HbPushButton( );

    QGraphicsLinearLayout* layout = new QGraphicsLinearLayout( Qt::Horizontal );
    layout->addItem( mButton );
    
    setLayout( layout );
    
    QObject::connect( mButton, SIGNAL( clicked( ) ), this, SLOT( toggleValue( ) ) );

    mModel = static_cast<HbDataFormModel*>(
            HbDataFormViewItemPrivate::d_ptr( mViewItem )->mSharedData->mItemView->model( ) );
    mModelItem = static_cast<HbDataFormModelItem*>(
            mModel->itemFromIndex( mViewItem->modelIndex( ) ) );
    QObject::connect(this,SIGNAL(valueChanged(QPersistentModelIndex, QVariant)),mViewItem, 
        SIGNAL(itemModified(QPersistentModelIndex, QVariant)));
}

HbToggleItem::~HbToggleItem()
{
}

/* This function is evoked when Dynamic property is set on HbToggleItem , 
then corresponding property will be set on  HbPushButton */
bool HbToggleItem::event( QEvent * e )
{
    switch( e->type() ){
        case QEvent::DynamicPropertyChange:{
                QDynamicPropertyChangeEvent *changeEvent = 
                    static_cast<QDynamicPropertyChangeEvent*>( e );
                QString name = changeEvent->propertyName().data();
                
                // Do not set AdditionalText on HbPushButton 
                if( name != "additionalText" ) {
                    mButton->setProperty( changeEvent->propertyName().data(), 
                        property(changeEvent->propertyName().data()) );
                }
                break;
            }
        default:
            break;
    }

    HbWidget::event( e );
    return false;
}


HbWidget* HbToggleItem::contentWidget() const
{
    return mButton;
}

void HbToggleItem::toggleValue()
{   
    // Toggle's the text and additionalText in HbDataFormModelItem and set the corresponding text 
    // as the text of HbPushButton
    QString additionalTxt = 
        mModelItem->contentWidgetData( QString("additionalText") ).toString();
    QString txt = mModelItem->contentWidgetData(QString("text")).toString();
    mModelItem->setContentWidgetData( QString("text"), additionalTxt );
    emit valueChanged(mViewItem->modelIndex(), additionalTxt);
    // HbPushButton will not be updated with Additional Text when modelChanged signal is emitted
    mModelItem->setContentWidgetData( QString("additionalText"), txt );
}

/*  
    HbToggleItem holds a HbLabel and an HbRadioButtonList internally and toggles these 
    widget as the contentwidget when user clicks the contentwidget area .
 */
HbRadioItem::HbRadioItem( QGraphicsItem* parent ):
    HbWidget( parent ),
    mRadioButton( 0 )
{
    // Create label by default . RadioButtonList is created created at runtime 
    // when clicked on the item
    mViewItem = static_cast<HbDataFormViewItem*>( parent );

    mButton = new HbPushButton();
    QObject::connect(mButton, SIGNAL(released()), this, SLOT(buttonClicked()));
    QGraphicsLinearLayout* layout = new QGraphicsLinearLayout( Qt::Vertical );
    layout->addItem( mButton );
    setLayout( layout );
    
    mButtonVisible = true;
    mSelected = 0;
    mModel = static_cast<HbDataFormModel*>(
            HbDataFormViewItemPrivate::d_ptr(mViewItem)->mSharedData->mItemView->model());
    mModelItem = static_cast<HbDataFormModelItem*>(
            mModel->itemFromIndex(mViewItem->modelIndex()));
    QObject::connect(this,SIGNAL(valueChanged(QPersistentModelIndex, QVariant)),mViewItem, 
        SIGNAL(itemModified(QPersistentModelIndex, QVariant)));
}

HbRadioItem::~HbRadioItem()
{
    // delete the widget which is not currently visible. The visible will be deleted by layout hierarchy
    if( mButtonVisible ) {
        if( mRadioButton ) {
            delete mRadioButton;
        }
    } else {
        delete mButton;
    }
}

HbWidget* HbRadioItem::contentWidget()
{
    // If not created create and set properties and return the widget
    if(!mRadioButton) {
        mRadioButton = new HbRadioButtonList();        
        mRadioButton->setItems( mItems );        
        mRadioButton->setVisible(false);
        mRadioButton->setSelected(mSelected);

        QObject::connect( mRadioButton, SIGNAL(itemSelected(int)), this, SLOT(itemSelected(int)) );
        
    }
    return mRadioButton;
}

/*  This function is evoked when Dynamic property is set on HbRadioItem , then 
    corresponding property will be set on  HbRadioButtonList
*/
bool HbRadioItem::event( QEvent * e )
{
    switch( e->type() ){
        case QEvent::DynamicPropertyChange: {
                QDynamicPropertyChangeEvent *changeEvent = static_cast<QDynamicPropertyChangeEvent*>( e );
                QString name = changeEvent->propertyName().data();
                if( mRadioButton ) {
                    mRadioButton->setProperty( 
                        changeEvent->propertyName().data() ,property( changeEvent->propertyName().data()) );
                }
                if( name == "items" ) {
                    mItems = property("items").toStringList();
                } else if( name == "selected" ) {
                    mItems = property("items").toStringList();
                    mSelected = property("selected").toInt();
                    mButton->setText(mItems.at(mSelected));
                    mButton->setTextAlignment(Qt::AlignLeft);
                }
                break;
            }
        default:
            break;
    }

    HbWidget::event(e);
    return false;
}


void HbRadioItem::buttonClicked()
{
    // launch popup only if number of items are more than 5
    if(!mRadioButton) {
            contentWidget();
        } 
    
    
        
        mButton->setVisible( false );  
        static_cast<QGraphicsLinearLayout*>( layout() )->addItem( mRadioButton );   
        mRadioButton->setVisible( true );
        mButtonVisible = false;
        layout()->removeAt( 0 );
        

}


void HbRadioItem::itemSelected( int index )
{
    //update the label with the selected index text
    //mLabel->clear( );
    mButton->setText( mRadioButton->items().at(index) );
    emit valueChanged(mViewItem->modelIndex(), mRadioButton->items().at(index));
    
    
    static_cast<QGraphicsLinearLayout*>( layout() )->addItem( mButton );

    mRadioButton->setVisible( false );
    mButton->setVisible( true );
    mButtonVisible = true;
    layout()->removeAt( 0 );
    
    //update the model
    disconnect( mModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                  HbDataFormViewItemPrivate::d_ptr(mViewItem)->mSharedData->mItemView, 
                  SLOT( dataChanged(QModelIndex,QModelIndex)) );
    mModelItem->setContentWidgetData("selected", mRadioButton->property("selected"));
    connect( mModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                  HbDataFormViewItemPrivate::d_ptr(mViewItem)->mSharedData->mItemView, 
                  SLOT( dataChanged(QModelIndex,QModelIndex)) );
}

HbMultiSelectionItem::HbMultiSelectionItem( QGraphicsItem* parent ):
    HbWidget(parent),
    mQuery(0)
{
    // Create label by default . RadioButtonList is created created at runtime 
    // when clicked on the item        
    mViewItem = static_cast<HbDataFormViewItem*>(parent);

    mButton = new HbPushButton();
    QObject::connect(mButton, SIGNAL(released()), this, SLOT(launchMultiSelectionList()));
    QGraphicsLinearLayout* layout = new QGraphicsLinearLayout(Qt::Horizontal);
    layout->addItem(mButton);
    setLayout(layout);
    
    mModel = static_cast<HbDataFormModel*>(
            HbDataFormViewItemPrivate::d_ptr(mViewItem)->mSharedData->mItemView->model());
    mModelItem = static_cast<HbDataFormModelItem*>(
            mModel->itemFromIndex(mViewItem->modelIndex()));
    QObject::connect(this,SIGNAL(valueChanged(QPersistentModelIndex, QVariant)),mViewItem, SIGNAL(itemModified(QPersistentModelIndex, QVariant)));
}

HbMultiSelectionItem::~HbMultiSelectionItem()
{
}

HbWidget* HbMultiSelectionItem::contentWidget() const
{
    return mButton;
}

bool HbMultiSelectionItem::event( QEvent * e )
{
    switch( e->type() ) {
        case QEvent::DynamicPropertyChange: {
                QDynamicPropertyChangeEvent *eve = static_cast<QDynamicPropertyChangeEvent*>( e );
                QString name = eve->propertyName( ).data( );
                
                if ( name == "text" ) {
                    mButton->setProperty(
                        eve->propertyName().data(), property(eve->propertyName().data()) );
                }
                if ( name == "items" ) {
                    mItems = property("items").toStringList();
                } else if ( name == "selectedItems" ) {
                     mItems = property("items").toStringList();
                     QList<QVariant> selected = property("selectedItems").toList();
                     
                     for( int i = 0; i < selected.count() ; i++ ) {
                         if ( !mSelectedItems.contains( selected.at( i ).toInt( ) ) ) {
                            mSelectedItems.append( selected.at( i ).toInt( ) );
                         }
                     }
                }
                if ( mSelectedItems.count() > 0 && mItems.count() > 0 ) {
                    QString newValue("");
                    for ( int i = 0; i < mSelectedItems.count() ; i++ ) {
                        newValue.append( mItems.at( mSelectedItems.at( i ) ) );
                        newValue.append( ";" );
                    }
                    //mButton->clear( );
                    mButton->setText( newValue );                    
                    mButton->setTextAlignment(Qt::AlignLeft);
                }
                break;
            }
        default:
            break;
    }

    HbWidget::event( e );
    return false;
}


void HbMultiSelectionItem::launchMultiSelectionList()
{
    mQuery = 0;
    mQuery = new HbListDialog();
    mQuery->setSelectionMode( HbAbstractItemView::MultiSelection );
    mQuery->setStringItems( mItems, mItems.count() + 1 );    
    mQuery->setSelectedItems( mSelectedItems );
    mQuery->setAttribute(Qt::WA_DeleteOnClose);

    mQuery->open(this,SLOT(dialogClosed(HbAction*)));   
}

void HbMultiSelectionItem::dialogClosed(HbAction* action)
{
    if( action == mQuery->primaryAction( )) {
        //fetch the selected items
        mSelectedItems = mQuery->selectedItems();
        QString newValue("");

        qSort( mSelectedItems.begin(), mSelectedItems.end( ) );
        for( int i = 0; i < mSelectedItems.count(); i++ ) {
            newValue.append(mQuery->stringItems().at(mSelectedItems.at(i)));
            if( i != mSelectedItems.count() - 1 ) {
                newValue.append( ";" );
            }
        }
        
        mButton->setText( newValue );
        emit valueChanged(mViewItem->modelIndex(), newValue);

        disconnect( mModel, SIGNAL( dataChanged( QModelIndex, QModelIndex ) ),
                      HbDataFormViewItemPrivate::d_ptr(mViewItem)->mSharedData->mItemView, 
                      SLOT( dataChanged(QModelIndex,QModelIndex)));
        
        mModelItem->setContentWidgetData( "items", mItems );

        //update the model with the selected items
        QList<QVariant> items;
        for( int i = 0; i < mSelectedItems.count(); i++ ) {
            items.append( mSelectedItems.at( i ) );
        }
        mModelItem->setContentWidgetData("selectedItems", items);

        connect(mModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                      HbDataFormViewItemPrivate::d_ptr(mViewItem)->mSharedData->mItemView, 
                      SLOT( dataChanged(QModelIndex,QModelIndex)));
    }
}


HbDataFormViewItemPrivate::HbDataFormViewItemPrivate( HbDataFormViewItem *prototype ):
    HbAbstractViewItemPrivate( prototype ),
    mContentWidget( 0 ),
    mBackgroundItem( 0 ),
    mLabelItem( 0 ),
    mIconItem( 0 ),
    mDescriptionItem(0),
    mSetAllProperty( true )
{
}

HbDataFormViewItemPrivate::HbDataFormViewItemPrivate( const HbDataFormViewItemPrivate &source ):
    HbAbstractViewItemPrivate( source ),
    mContentWidget( source.mContentWidget ),
    mBackgroundItem( source.mBackgroundItem ),
    mLabelItem( source.mLabelItem ),
    mIconItem( source.mIconItem ),
    mDescriptionItem(source.mDescriptionItem),
    mSetAllProperty( source.mSetAllProperty )
{
}

HbDataFormViewItemPrivate& HbDataFormViewItemPrivate::operator= (
    const HbDataFormViewItemPrivate &source )
{
    HbAbstractViewItemPrivate::operator =( source );
    return *this;
}

HbDataFormViewItemPrivate::~HbDataFormViewItemPrivate()
{
}

void HbDataFormViewItemPrivate::init()
{
    Q_Q(HbDataFormViewItem);

    if( mIndex.isValid( ) ) {
        mType = static_cast<HbDataFormModelItem::DataItemType>(
            mIndex.data(HbDataFormModelItem::ItemTypeRole).toInt( ) );
    }

    if( mSharedData && mSharedData->mItemView ) {
        mModel = static_cast<HbDataFormModel*>( mSharedData->mItemView->model( ) );
        mModelItem = mModel->itemFromIndex( q->modelIndex( ) );
    }
}


void HbDataFormViewItemPrivate::createPrimitives()
{
    Q_Q( HbDataFormViewItem );

    if( !mBackgroundItem ) {
        mBackgroundItem = q->style()->createPrimitive( HbStyle::P_DataItem_background, q );
    }
    

    if( !mLabel.isEmpty() ) {
        if( !mLabelItem ) {
            mLabelItem = q->style()->createPrimitive( HbStyle::P_DataItem_label, q );
        }
    }

    if( !mIcon.isEmpty() ) {
        q->setProperty( "hasIcon", true );
        if(!mIconItem) {
            mIconItem = q->style()->createPrimitive( HbStyle::P_DataItem_icon, q );
        }
    } else {
        q->setProperty( "hasIcon", false );
    }

    if(!mDescription.isEmpty()) {
        if(!mDescriptionItem) {
            mDescriptionItem = q->style()->createPrimitive(HbStyle::P_DataItem_description, q);
        }
    }
}

/*
    Sets the description for the data item.
*/
void HbDataFormViewItemPrivate::setDescription( const QString& description )
{
    mDescription = description; 
    createPrimitives();
    updatePrimitives();
}

/*
    Returns the description of data item.
*/
QString HbDataFormViewItemPrivate::description() const
{
    return mDescription;
}

void HbDataFormViewItemPrivate::updatePrimitives()
{
    Q_Q( HbDataFormViewItem );

    HbStyleOptionDataFormViewItem options;
    q->initStyleOption(&options);

    if( mBackgroundItem ) {
        q->style()->updatePrimitive(
            mBackgroundItem, HbStyle::P_DataItem_background, &options );
    }

    if( mLabelItem ) {
        q->style()->updatePrimitive( mLabelItem, HbStyle::P_DataItem_label, &options );
    }

    if( mIconItem ) {
        q->style()->updatePrimitive(
            mIconItem, HbStyle::P_DataItem_icon, &options );
    }
    
    if(mDescriptionItem) {
        q->style()->updatePrimitive(mDescriptionItem, HbStyle::P_DataItem_description, &options);
    }
}

/*
    Sets the label/ heading for the setting item . If no label set the label widget 
    will not be created .
    
*/
void HbDataFormViewItemPrivate::setLabel( const QString& label )
{
    mLabel = label; 
    createPrimitives();
    updatePrimitives();
}
void HbDataFormViewItemPrivate::updateLabel(const QString& label)
{
    Q_Q(HbDataFormViewItem);
    
    HbDataFormModelItem::DataItemType type = static_cast< HbDataFormModelItem::DataItemType>(
                q->modelIndex().data(HbDataFormModelItem::ItemTypeRole).toInt());
    HbDataFormModel* data_model = static_cast<HbDataFormModel*>(q->itemView()->model());
    HbDataFormModelItem *model_item = static_cast<HbDataFormModelItem*>(data_model->itemFromIndex(mIndex));   
    
    
    if(type == HbDataFormModelItem::FormPageItem) {

        int index = data_model->invisibleRootItem()->indexOf(model_item);
        HbDataFormPrivate* form_priv = HbDataFormPrivate::d_ptr(
                                    static_cast<HbDataForm*>(q->itemView()));        
        if(index >= 0) {
            form_priv->mHeadingWidget->updatePageName(index ,label);
        }

    } else if(type == HbDataFormModelItem::GroupItem) {
        
        
        HbDataGroupPrivate::d_ptr(static_cast<HbDataGroup*>(q))->setHeading(label);            

    } else if(type == HbDataFormModelItem::GroupPageItem) {
       
        QModelIndex groupIndex = data_model->parent(mIndex);
        int index = (data_model->itemFromIndex(groupIndex))->indexOf(model_item);       
        HbDataGroup* groupItem = static_cast<HbDataGroup*>(
                        q->itemView()->itemByIndex(groupIndex));
        groupItem->updateGroupPageName(index,label);

    } else if (type > HbDataFormModelItem::GroupPageItem ) {
        setLabel(label);        
    }
    
}

void HbDataFormViewItemPrivate::setEnabled(bool enabled)
{
    Q_Q(HbDataFormViewItem);


    QGraphicsItem::GraphicsItemFlags itemFlags = q->flags();
    Qt::ItemFlags indexFlags = mIndex.flags();

    if (indexFlags & Qt::ItemIsEnabled) {
        if (!(itemFlags & QGraphicsItem::ItemIsFocusable)) {
            itemFlags |= QGraphicsItem::ItemIsFocusable;
            q->setFocusPolicy(q->prototype()->focusPolicy());
            q->setProperty("state", "normal");
        }
    } else {
        if (itemFlags & QGraphicsItem::ItemIsFocusable) {
            itemFlags &= ~QGraphicsItem::ItemIsFocusable;
            q->setFocusPolicy(Qt::NoFocus);
            q->setProperty("state", "disabled");
        }
    }

    if( mContentWidget ) {
        mContentWidget->setEnabled(enabled);
    }
}

/*
    Returns the heading / label of the setting item.
*/
QString HbDataFormViewItemPrivate::label() const
{
    return mLabel;
}

/*
    Sets the icon for the setting item . If no icon is set icon will not be created.
    
*/
void HbDataFormViewItemPrivate::setIcon( const QString& icon )
{
    mIcon = icon;
    createPrimitives();
    updatePrimitives();
}

/*
    Returns the heading / label of the setting item.
*/
QString HbDataFormViewItemPrivate::icon() const
{
    return mIcon;
}

void HbDataFormViewItemPrivate::createContentWidget()
{
    Q_Q(HbDataFormViewItem);
    
    QObject::connect(q, SIGNAL(itemShown(const QModelIndex&)), 
                mSharedData->mItemView, SIGNAL(activated(const QModelIndex&)));
    QObject::connect(q, SIGNAL(itemShown(const QModelIndex&)), 
                mSharedData->mItemView, SIGNAL(itemShown(const QModelIndex&)));
    switch( mType ) {
        // following are standard data item
        case HbDataFormModelItem::SliderItem:
        case HbDataFormModelItem::VolumeSliderItem: {
                mContentWidget = new HbSlider( Qt::Horizontal, q );
                mProperty.append( "sliderPosition" );            
                QObject::connect( mContentWidget, SIGNAL(sliderReleased()), q,SLOT(save()) );
                HbStyle::setItemName( mContentWidget, "dataItem_ContentWidget" );                
            }
            break;
        case HbDataFormModelItem::CheckBoxItem: {
                mContentWidget = new HbCheckBox(q);
                mProperty.append("checkState");            
                QObject::connect(mContentWidget, SIGNAL(stateChanged(int)), q, SLOT(save()));
                HbStyle::setItemName(mContentWidget, "dataItem_ContentWidget");
            }
            break;
        case HbDataFormModelItem::TextItem: {
                mContentWidget = new HbLineEdit(q);
                static_cast<HbLineEdit *>(mContentWidget)->setMaxRows( 4 );
                mProperty.append("text");            
                QObject::connect(mContentWidget, SIGNAL(editingFinished()), q, SLOT(save()));
                HbStyle::setItemName(mContentWidget, "dataItem_ContentWidget");
            }
            break;
        case HbDataFormModelItem::ToggleValueItem: {
                mContentWidget = new HbToggleItem(q);  
                mProperty.append("text");
                //QObject::connect(mContentWidget, SIGNAL(valueChanged()), q, SLOT(save()));
                HbStyle::setItemName(mContentWidget, "dataItem_ContentWidget");
            }
            break;
        case HbDataFormModelItem::RadioButtonListItem:{
                mContentWidget = new HbRadioItem(q);
                mProperty.append("selected"); 
                //QObject::connect(mContentWidget, SIGNAL(valueChanged()), q, SLOT(save()));
                HbStyle::setItemName(mContentWidget, "dataItem_ContentWidget");
            }
            break;
        case HbDataFormModelItem::MultiselectionItem:{
                mContentWidget = new HbMultiSelectionItem( q);                
                mProperty.append("text");          
                //QObject::connect(mContentWidget, SIGNAL(valueChanged()), q, SLOT(save()));
                HbStyle::setItemName(mContentWidget, "dataItem_ContentWidget");
            }
            break;
        case HbDataFormModelItem::ComboBoxItem:{
                mContentWidget = new HbComboBox(q);
                mProperty.append("currentIndex");
                QObject::connect(static_cast<HbComboBox *>(mContentWidget), 
                    SIGNAL(currentIndexChanged(int)), 
                    q,SLOT(save()));
                HbStyle::setItemName(mContentWidget, "dataItem_ContentWidget");
            }
            break;
        default:{
                HbWidget* custom = q->createCustomWidget();
                if( custom != 0 ) {
                    mContentWidget = custom;
                    mContentWidget->setParentItem(q);
                    HbStyle::setItemName(mContentWidget, "dataItem_ContentWidget");
                }
            }
            break;
    }
    if ( mContentWidget ) {
        QEvent polishEvent( QEvent::Polish );
        QCoreApplication::sendEvent( mContentWidget, &polishEvent );
    }
}



