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
#include "hbdataformmodelitem_p.h"
#include "hbdatagroup_p_p.h"

#include <hbslider.h>
#include <hbcheckbox.h>
#include <hblistwidget.h>
#include <hblabel.h>
#include <hblineedit.h>
#include <hbradiobuttonlist.h>
#include <hbcombobox.h>
#include <hbstyleoptiondataformviewitem_p.h>
#include <hbselectiondialog.h>
#include <hbpushbutton.h>
#include <hbaction.h>

#include <QGraphicsLinearLayout>
#include <QCoreApplication>

#ifdef HB_GESTURE_FW
#include <hbtapgesture.h>
#endif
#define MAX_INLINE_ITEM_COUNT 3

HbToggleItem::HbToggleItem( QGraphicsItem* parent ): HbWidget( parent )
{
    // Toggle item uses button as the content widget and toggles the text when item is clicked
    mViewItem = static_cast<HbDataFormViewItem*>( parent );
    mButton = new HbPushButton( );

    QGraphicsLinearLayout* layout = new QGraphicsLinearLayout( Qt::Horizontal );
    layout->setContentsMargins(0, 0, 0, 0);
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
    HbDataFormModelItemPrivate *modelItem_priv = HbDataFormModelItemPrivate::d_ptr(mModelItem); 
    // Dont want to emit datachanged for this property so calling private function
    modelItem_priv->setContentWidgetData( QString("additionalText"), txt );
    // will emit datachanged
    mModelItem->setContentWidgetData( QString("text"), additionalTxt );
    emit valueChanged(mViewItem->modelIndex(), additionalTxt);
}

/*  
    HbToggleItem holds a HbLabel and an HbRadioButtonList internally and toggles these 
    widget as the contentwidget when user clicks the contentwidget area .
 */
HbRadioItem::HbRadioItem( QGraphicsItem* parent ):
    HbWidget( parent ),
    mRadioButtonList( 0 ),
    mButton(0), 
    mDialog(0),
    layout(0),
    mSelected(-1)
{

    mViewItem = static_cast<HbDataFormViewItem*>( parent );
    mModel = static_cast<HbDataFormModel*>(
            HbDataFormViewItemPrivate::d_ptr(mViewItem)->mSharedData->mItemView->model());
    mModelItem = static_cast<HbDataFormModelItem*>(
            mModel->itemFromIndex(mViewItem->modelIndex()));
    QString displayMode = mModelItem->contentWidgetData(QString("displayMode")).toString();
     
    // set displayMode to automatic if not already set by application
    if(displayMode.isEmpty()) {
        mModelItem->setContentWidgetData(QString("displayMode"), "automatic");
    }
    QObject::connect(this,SIGNAL(valueChanged(QPersistentModelIndex, QVariant)),mViewItem, 
        SIGNAL(itemModified(QPersistentModelIndex, QVariant)));
    if(!layout) {
        layout = new QGraphicsLinearLayout( Qt::Vertical );
        layout->setContentsMargins(0, 0, 0, 0);
    }
    setLayout( layout );
}

HbRadioItem::~HbRadioItem()
{
}

HbWidget* HbRadioItem::createRadioButton()
{
    // If not created create and set properties and return the widget
    if(!mRadioButtonList) {
        mRadioButtonList = new HbRadioButtonList();      
        mRadioButtonList->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
        QObject::connect(mRadioButtonList, SIGNAL(itemSelected(int)), this, SIGNAL(itemSelected(int)));
        QObject::connect(mRadioButtonList, SIGNAL(startPreview(int)), this, SIGNAL(startPreview(int)));
        //mRadioButtonList->setClampingStyle(HbScrollArea::StrictClamping);
        mRadioButtonList->setItems( mItems );
        if( mSelected != -1 ) {
            mRadioButtonList->setSelected( mSelected );
        }
        // model need to be updated when selection changes
        // in popup case this connection will be removed since model will be updated 
        // only when dialog closed
        QObject::connect( mRadioButtonList, SIGNAL(itemSelected(int)), 
            this, SLOT(updateModel(int)) );
        
    }
    return mRadioButtonList;
}

/*  This function is evoked when Dynamic property is set on HbRadioItem , then 
    corresponding property will be set on  HbRadioButtonList
*/
bool HbRadioItem::event( QEvent * e )
{
    switch( e->type() ){
        case QEvent::DynamicPropertyChange: {
                QDynamicPropertyChangeEvent *changeEvent = 
                    static_cast<QDynamicPropertyChangeEvent*>( e );
                QString dynamicPropertyName = changeEvent->propertyName().data();
                if( mRadioButtonList ) {
                    //Set the property on radiobutton list if already created 
                    mRadioButtonList->setProperty( 
                        changeEvent->propertyName().data() ,
                        property( changeEvent->propertyName().data()) );
                }
                if( dynamicPropertyName == "items" || dynamicPropertyName == "displayMode" ) {
                    // store the items locally
                    mItems = property("items").toStringList();
                    // in case of automatic, displayMode (embedded, automatic or popup) will change 
                    // if new items are populated or mode should be changed if mode is set
                    //  explicitly by application at runtime
                    changeMode();
                } else if(dynamicPropertyName == "selected") {
                    // store the new selection 
                    mSelected = property("selected").toInt();
                    // fine tune the new selection if it goes beyond range. This happens when new
                    // items are populated or selection index is changed by application in to the model
                    // which can be out of range
                    resetSelection();
                    selectItem();
                }
                break;
            }
        default:
            break;
    }
    HbWidget::event(e);
    return false;
}

void HbRadioItem::makeEmbedded()
{
    // button need to be deleted when mode is changed from popup to embedded
    if(mButton) {
        layout->removeItem(mButton);
        delete mButton;
        mButton = 0;
    }
    createRadioButton();
    layout->addItem(mRadioButtonList);
    mRadioButtonList->setScrollDirections(0);
}

void HbRadioItem::makePopup()
{
    if(mRadioButtonList) {
        layout->removeItem(mRadioButtonList);
        // RadioButton will not be shown unless button is clicked so delete it
        delete mRadioButtonList;
        mRadioButtonList = 0;
    }
    initilizeButton();
}


void HbRadioItem::resetSelection()
{
    if((mItems.count() > 0 && mSelected == -1) || 
        (mItems.count() > 0 && (mItems.count() <= mSelected ))) {
        // if selection is beyond range then set it to 0. 
        // This happens when new items are populated and 
        // application does not set selection explicitly
        mSelected = 0;
    } else if(mItems.count() == 0) {
        mSelected = -1;
    }
}

/*
changeMode() will change the mode of visualization( embedded, automatic and popup) depending
on the number of items in case of automatic mode or change it to correspoding if not automatic.
In popup the selected item will be displayed as the text of the button otherwise RadioButtonList
is placed inline.
*/
void HbRadioItem::changeMode()
{
    QString displayMode = mModelItem->contentWidgetData(QString("displayMode")).toString();
    
    if(displayMode == "embedded") {
        makeEmbedded();
    } else if(displayMode == "popup") {
        makePopup();
    }
    else if(displayMode == "automatic") {
        if(mItems.count() <= MAX_INLINE_ITEM_COUNT  && mItems.count() > 0  ) {
            makeEmbedded();
        } else if(mItems.count() >= MAX_INLINE_ITEM_COUNT){
            makePopup();
        }
    }
    resetSelection();
    selectItem();
}

void HbRadioItem::selectItem()
{
    //Set Text if button is valid
    if(mSelected != -1) {
        if(mButton ) {
            mButton->setText(mItems.at(mSelected));
        } 
        // change selection of radiobutton list
        if(mRadioButtonList && mRadioButtonList->selected() != mSelected) {
            mRadioButtonList->setSelected(mSelected);
        }
    } else if( mButton ) {
        // clear the text on the button if items are cleared 
        mButton->setText(QString(""));
    }
}

void HbRadioItem::initilizeButton()
{
    if(!mButton) {
        mButton = new HbPushButton();
        mButton->setTextAlignment(Qt::AlignLeft);
        QObject::connect(mButton, SIGNAL(clicked()), this, SLOT(buttonClicked()));
        layout->addItem( mButton ); 
    }
}

void HbRadioItem::buttonClicked()
{
    if(mItems.count() > 0) {
        if(!mRadioButtonList) {
            createRadioButton();
        } 
        mRadioButtonList->setScrollDirections(Qt::Vertical);
        QObject::disconnect( mRadioButtonList, SIGNAL(itemSelected(int)), 
            this, SLOT(updateModel(int)) );
        selectItem();
        mDialog = new HbDialog();
        QObject::connect(mDialog, SIGNAL(finished(HbAction*)), this, SIGNAL(finished(HbAction*)));
        QObject::connect(mDialog, SIGNAL(aboutToShow()), this, SIGNAL(aboutToShow()));
        QObject::connect(mDialog, SIGNAL(aboutToHide()), this, SIGNAL(aboutToHide()));
        QObject::connect(mDialog, SIGNAL(aboutToClose()), this, SIGNAL(aboutToClose()));
        mDialog->setTimeout(HbPopup::NoTimeout);    
        mDialog->setAttribute(Qt::WA_DeleteOnClose);
        mDialog->setContentWidget(mRadioButtonList);    
        mDialog->addAction(new HbAction(QString("Ok")));
        mDialog->addAction(new HbAction(QString("Cancel")));
        mDialog->open(this,SLOT(dialogClosed(HbAction*)));  
        mRadioButtonList->setSelected(mSelected);
    }
}


void HbRadioItem::updateModel( int index )
{
    mSelected = index;
    emit valueChanged(mViewItem->modelIndex(), mItems.at(index));
    // Disconnect modelchanged signal since visualization is already updated by user
    // so if not disconnected , this will trigger visualization change again
    disconnect( mModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                  HbDataFormViewItemPrivate::d_ptr(mViewItem)->mSharedData->mItemView, 
                  SLOT( dataChanged(QModelIndex,QModelIndex)) );
    //update the model
    mModelItem->setContentWidgetData("selected", mSelected);
    // establish the connection back
    connect( mModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                  HbDataFormViewItemPrivate::d_ptr(mViewItem)->mSharedData->mItemView, 
                  SLOT( dataChanged(QModelIndex,QModelIndex)) );
    // select the item . The selection is already stored in to the model above
    selectItem();
}


void HbRadioItem::dialogClosed(HbAction* action)
{
    if(( action ) && ( action->text() == "Ok" )) {
        // store the selected item to model
        updateModel(mRadioButtonList->selected());        
    }
    // dont change selection incase of "Cancel" button click .
    mRadioButtonList = 0;
    mDialog = 0;
}

HbMultiSelectionItem::HbMultiSelectionItem( QGraphicsItem* parent ):
    HbWidget(parent),
    mSelectionDialog(0),
    mButton(0),
    mMultiListWidget(0)
{
    // Create label by default . RadioButtonList is created created at runtime 
    // when clicked on the item        
    mViewItem = static_cast<HbDataFormViewItem*>(parent);
    layout = new QGraphicsLinearLayout(Qt::Horizontal);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    
    mModel = static_cast<HbDataFormModel*>(
            HbDataFormViewItemPrivate::d_ptr(mViewItem)->mSharedData->mItemView->model());
    mModelItem = static_cast<HbDataFormModelItem*>(
            mModel->itemFromIndex(mViewItem->modelIndex()));    
    // set displayMode to automatic if not already set by application
    QString displayMode = mModelItem->contentWidgetData(QString("displayMode")).toString();
    if(displayMode.isEmpty()) {
        mModelItem->setContentWidgetData(QString("displayMode"), "automatic");
    }
    QObject::connect(this,SIGNAL(valueChanged(QPersistentModelIndex, QVariant)),
        mViewItem, SIGNAL(itemModified(QPersistentModelIndex, QVariant)));
}

HbMultiSelectionItem::~HbMultiSelectionItem()
{
}


void HbMultiSelectionItem::makeSelection()
{
    QItemSelectionModel *model = 0;
    if(mMultiListWidget) {// embedded case
        // get selection model
        model = mMultiListWidget->selectionModel();
        if(model) {
            // disconnect so that the visualization does not get changed when selction 
            // model changes
            QObject::disconnect(model, 
                SIGNAL(selectionChanged( const QItemSelection , const QItemSelection  )), 
                this, SLOT(updateModel( const QItemSelection , const QItemSelection  )));
            model->clearSelection();
            for( int i = 0; i < mSelectedItems.count() ; i++ ) {
                model->select( model->model()->index(mSelectedItems.at( i ).toInt(),0),
                    QItemSelectionModel::Select);
            }
            QObject::connect(model, 
                SIGNAL(selectionChanged( const QItemSelection , const QItemSelection  )), 
                this, SLOT(updateModel( const QItemSelection , const QItemSelection  )));
        }
    } else if(mButton) { // update text on button
        QString newValue("");
        // create sting to be set on button
        for ( int i = 0; i < mSelectedItems.count() ; i++ ) {
            int selectionindex = mSelectedItems.at( i ).toInt();
            if( selectionindex< mItems.count()) {
                if( i > 0) {// dont add ; in the starting of the string
                    newValue.append( "," );
                }
                newValue.append( mItems.at( mSelectedItems.at( i ).toInt() ) );
            } 
        }
        mButton->setText( newValue );
    }
}

void HbMultiSelectionItem::makeEmbedded()
{
    
    // delete button if displaymode change has happened at runtime 
    if(mButton){
        layout->removeItem(mButton);
        delete mButton;
        mButton = 0;
    }
    // create ListWidget if not yet created 
    if(!mMultiListWidget) {
        mMultiListWidget = new HbListWidget();
    QObject::connect(mMultiListWidget, SIGNAL(activated(HbListWidgetItem *)), this, SIGNAL(activated(HbListWidgetItem *)));
    QObject::connect(mMultiListWidget, SIGNAL(pressed(HbListWidgetItem *)), this, SIGNAL(pressed(HbListWidgetItem *)));
    QObject::connect(mMultiListWidget, SIGNAL(released(HbListWidgetItem *)), this, SIGNAL(released(HbListWidgetItem *)));
    QObject::connect(mMultiListWidget, SIGNAL(longPressed(HbListWidgetItem *,  const QPointF &)), this, SIGNAL(longPressed(HbListWidgetItem *,  const QPointF &)));
        layout->addItem(mMultiListWidget);
    }
    
    mMultiListWidget->setSelectionMode(HbAbstractItemView::MultiSelection);
    
    mMultiListWidget->clear();
    // update the listwidget with new items
    for (int index = 0; index < mItems.count(); ++index) {
        mMultiListWidget->addItem(mItems.at(index));
    }
    mMultiListWidget->setScrollDirections(0);
}

void HbMultiSelectionItem::makePopup()
{
    // Delete ListWidget if mode change happened at runtime from embedded to popup
    if(mMultiListWidget) {
        layout->removeItem(mMultiListWidget);
        delete mMultiListWidget;
        mMultiListWidget = 0;
    }
    // create button since we need to display selected item on button
    // and popup will be launched when button is clicked
    if(!mButton) {
        mButton = new HbPushButton();
        mButton->setTextAlignment(Qt::AlignLeft);
        layout->addItem(mButton);
    }
    QObject::connect(mButton, SIGNAL(clicked()), this, SLOT(launchMultiSelectionList()));
}

void HbMultiSelectionItem::changeMode()
{
    QString displayMode = mModelItem->contentWidgetData(QString("displayMode")).toString();
    if((mItems.count() <= MAX_INLINE_ITEM_COUNT && displayMode == "automatic") || 
        displayMode == "embedded" ) {
        makeEmbedded();
    } else{
        makePopup();
    }
    makeSelection();
}

bool HbMultiSelectionItem::event( QEvent * e )
{
    switch( e->type() ) {
        case QEvent::DynamicPropertyChange: {
                QDynamicPropertyChangeEvent *eve = static_cast<QDynamicPropertyChangeEvent*>( e );
                QString dynamicPropertyName = eve->propertyName( ).data( );                
                if ( dynamicPropertyName == "items" || dynamicPropertyName == "displayMode" ) {
                    int prevCount = mItems.count();
                    mItems.clear();
                    mItems = property("items").toStringList();
                    if ( mItems.count() != prevCount || dynamicPropertyName == "displayMode") {
                        changeMode();
                    }
                    
                } else if ( dynamicPropertyName == "selectedItems" ) {                    
                    mItems = property("items").toStringList();
                    mSelectedItems = property("selectedItems").toList();
                    makeSelection();
                }
                break;
            }
        default:
            break;
    }

    HbWidget::event( e );
    return false;
}


void HbMultiSelectionItem::updateModel( const QItemSelection &selected, 
    const QItemSelection  &deselected)
{

    // This function gets called when selection changes in HbListWidget model
    disconnect( mModel, SIGNAL( dataChanged( QModelIndex, QModelIndex ) ),
        HbDataFormViewItemPrivate::d_ptr(mViewItem)->mSharedData->mItemView, 
        SLOT( dataChanged(QModelIndex,QModelIndex)));
    QModelIndexList selectList = selected.indexes();    
    QModelIndexList deselectList = deselected.indexes();
    // Add newly selected item to selected list
    for(int i = 0;i<selectList.count(); i++) {
       if(!mSelectedItems.contains(selectList.at(i).row())) {
            mSelectedItems.append(selectList.at(i).row());
       }
    }
    // remove selection from selection list
    for(int i = 0;i<deselectList.count(); i++) {
        mSelectedItems.removeOne(deselectList.at(i).row());
    }
    //update the model with the selected items
    
    mModelItem->setContentWidgetData("selectedItems", mSelectedItems);

    connect(mModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                  HbDataFormViewItemPrivate::d_ptr(mViewItem)->mSharedData->mItemView, 
                  SLOT( dataChanged(QModelIndex,QModelIndex)));
}

void HbMultiSelectionItem::launchMultiSelectionList()
{
    if(!mSelectionDialog ) {
        mSelectionDialog = new HbSelectionDialog();
        QObject::connect(mSelectionDialog, SIGNAL(aboutToShow()),this ,SIGNAL(aboutToShow()));
        QObject::connect(mSelectionDialog, SIGNAL(aboutToHide()),this ,SIGNAL(aboutToHide()));
        QObject::connect(mSelectionDialog, SIGNAL(aboutToClose()),this ,SIGNAL(aboutToClose()));
        QObject::connect(mSelectionDialog, SIGNAL(finished(HbAction*)),this ,SIGNAL(finished(HbAction*)));
        mSelectionDialog->setSelectionMode( HbAbstractItemView::MultiSelection );
        mSelectionDialog->setStringItems( mItems, -1 ); 
        mSelectionDialog->setSelectedItems( mSelectedItems );
        mSelectionDialog->setAttribute(Qt::WA_DeleteOnClose);
        mSelectionDialog->open(this,SLOT(dialogClosed(HbAction*)));   
    }
}

void HbMultiSelectionItem::dialogClosed(HbAction* action)
{
    if(( action ) && ( action->text() == "Ok" )) {
        //fetch the selected items
        mSelectedItems = mSelectionDialog->selectedItems();
        QString newValue("");
        QList<int> selection;
        for( int i = 0; i < mSelectedItems.count(); i++ ) {
            selection.append(mSelectedItems.at(i).toInt());
        }
        qSort( selection.begin(), selection.end( ) );
        mSelectedItems.clear();
        for( int i = 0; i < selection.count(); i++ ) {
            mSelectedItems.append(selection.at(i));
            newValue.append(mSelectionDialog->stringItems().at(selection.at(i)));
            if( i != selection.count() - 1 ) {
                newValue.append( "," );
            }
        }        
        mButton->setText( newValue );
        emit valueChanged(mViewItem->modelIndex(), newValue);

        disconnect( mModel, SIGNAL( dataChanged( QModelIndex, QModelIndex ) ),
                      HbDataFormViewItemPrivate::d_ptr(mViewItem)->mSharedData->mItemView, 
                      SLOT( dataChanged(QModelIndex,QModelIndex)));
        if(mModelItem) {
            mModelItem->setContentWidgetData(QString("text"), newValue);
        }
        
        mModelItem->setContentWidgetData( "items", mItems );

        //update the model with the selected items
        mModelItem->setContentWidgetData("selectedItems", mSelectedItems);

        connect(mModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                      HbDataFormViewItemPrivate::d_ptr(mViewItem)->mSharedData->mItemView, 
                      SLOT( dataChanged(QModelIndex,QModelIndex)));
    }
    mSelectionDialog = 0;
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
    } else {
        if( mLabelItem ) {
            HbStyle::setItemName( mLabelItem, NULL );
            delete mLabelItem;
            mLabelItem = 0;

        }
    }

    if( !mIcon.isEmpty() ) {
        q->setProperty( "hasIcon", true );
        if(!mIconItem) {
            mIconItem = q->style()->createPrimitive( HbStyle::P_DataItem_icon, q );
        }
    } else {
        q->setProperty( "hasIcon", false );
        if( mIconItem ) {
            HbStyle::setItemName( mIconItem, NULL );
            delete mIconItem;
            mIconItem = 0;
        }
    }

    if(!mDescription.isEmpty()) {
        if(!mDescriptionItem) {
            mDescriptionItem = q->style()->createPrimitive(HbStyle::P_DataItem_description, q);
        }
    } else {
        if( mDescriptionItem ) {
            HbStyle::setItemName( mDescriptionItem, NULL );
            delete mDescriptionItem;
            mDescriptionItem = 0;

        }
    }
}

/*
    Sets the description for the data item.
*/
void HbDataFormViewItemPrivate::setDescription( const QString& description )
{
    Q_Q( HbDataFormViewItem );

    if ( mDescription == description ) {
        return;
    }
    
    bool doRepolish = false;

    if ( ( mDescription.isEmpty() && !description.isEmpty() ) || 
        ( !mDescription.isEmpty() && description.isEmpty() )) {
            doRepolish = true;
    }

    mDescription = description;
    createPrimitives();
    if ( doRepolish ) {
        q->repolish();
    }
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
    Q_Q( HbDataFormViewItem );

    if (  mLabel == label ) {
        return;
    }

    bool doRepolish = false;

    if ( ( mLabel.isEmpty() && !label.isEmpty() ) || 
        ( !mLabel.isEmpty() && label.isEmpty() )) {
            doRepolish = true;
    }

    mLabel = label; 
    createPrimitives();
    if ( doRepolish ) {
        q->repolish();
    }
    updatePrimitives();
}

void HbDataFormViewItemPrivate::updateData()
{
    Q_Q(HbDataFormViewItem);
    
    HbDataFormModelItem::DataItemType type = static_cast< HbDataFormModelItem::DataItemType>(
                q->modelIndex().data(HbDataFormModelItem::ItemTypeRole).toInt());
    HbDataFormModel* data_model = static_cast<HbDataFormModel*>(q->itemView()->model());
    HbDataFormModelItem *model_item = static_cast<HbDataFormModelItem*>(data_model->itemFromIndex(mIndex));
    HbDataFormModelItemPrivate *modelItem_priv = HbDataFormModelItemPrivate::d_ptr(model_item);    

    //update label
    if( modelItem_priv->dirtyProperty() == "LabelRole" ) {
        QString label = model_item->label();
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
    } else if ( modelItem_priv->dirtyProperty() == "DescriptionRole" ) {
        //update description of either data item or data group
        QString description = model_item->description();
        if( type == HbDataFormModelItem::GroupItem ) {
            HbDataGroupPrivate::d_ptr(static_cast<HbDataGroup*>(q))->setDescription(description);
        } else if ( type > HbDataFormModelItem::GroupPageItem ) {
            setDescription(description);
        }
    } else if ( modelItem_priv->dirtyProperty() == "DecorationRole" ){
        //update data item icon
        if ( type > HbDataFormModelItem::GroupPageItem ) {
            setIcon(model_item->icon());
        }
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
            q->grabGesture(Qt::TapGesture);
        }
    } else {
        if (itemFlags & QGraphicsItem::ItemIsFocusable) {
            itemFlags &= ~QGraphicsItem::ItemIsFocusable;
            q->setFocusPolicy(Qt::NoFocus);
            q->setProperty("state", "disabled");
            q->ungrabGesture(Qt::TapGesture);
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
    Q_Q(HbDataFormViewItem);
    if ( mIcon == icon ) {
        return;
    }

    bool doRepolish = false;

    if ( ( mIcon.isEmpty() && !icon.isEmpty() ) || 
        ( !mIcon.isEmpty() && icon.isEmpty() )) {
            doRepolish = true;
    }

    mIcon = icon;
    createPrimitives();
    if ( doRepolish ) {
        q->repolish();
    }

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
                mSharedData->mItemView, SIGNAL(itemShown(const QModelIndex&)));   

    switch( mType ) {
        // following are standard data item
        case HbDataFormModelItem::SliderItem:
        case HbDataFormModelItem::VolumeSliderItem: {
                mContentWidget = new HbSlider( Qt::Horizontal, q );
                mProperty.append( "value" );            
                QObject::connect( mContentWidget, SIGNAL(valueChanged(int)), q,SLOT(save()) );
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
    //background primitive should get created because if none of the other primitives are created
    //then createPrimitive and updatePrimitive will never get called and hence background
    //will not be visible
    createPrimitives();

    //update only the background primitive
    HbStyleOptionDataFormViewItem options;
    q->initStyleOption(&options);
    if( mBackgroundItem ) {
        q->style()->updatePrimitive(
            mBackgroundItem, HbStyle::P_DataItem_background, &options );
    }

    if ( mContentWidget ) {
        QEvent polishEvent( QEvent::Polish );
        QCoreApplication::sendEvent( mContentWidget, &polishEvent );
    }
}



