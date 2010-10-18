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

#include "hbdatagroup_p_p.h"
#include "hbdatagroupheadingwidget_p.h"
#include "hbdataform_p.h"
#include "hbstyleoptiondatagroup_p.h"
#include "hbabstractitemcontainer_p_p.h"

#include <hbcombobox.h>
#include <hbdataformmodel.h>

#include <QStringListModel>
#include <QCoreApplication>


HbDataGroupPrivate::HbDataGroupPrivate( HbDataGroup* item ):
    HbDataFormViewItemPrivate( item ),
    mPageCombo( 0 ),
    mGroupHeading( 0 ),
    mPageComboBackgroundItem( 0 )
{
}

HbDataGroupPrivate::HbDataGroupPrivate( const HbDataGroupPrivate &source ):
    HbDataFormViewItemPrivate( source ),
    mPageCombo( source.mPageCombo ),
    mGroupHeading( source.mGroupHeading ),
    mPageComboBackgroundItem( source.mPageComboBackgroundItem )
{
}

HbDataGroupPrivate::~HbDataGroupPrivate( )
{
}

void HbDataGroupPrivate::init( )
{   
}

bool HbDataGroupPrivate::setExpanded( bool expanded )
{
    Q_Q(HbDataGroup);
    HbAbstractItemContainer *container = 0;
    HbDataFormModelItem::DataItemType itemType = 
            static_cast<HbDataFormModelItem::DataItemType>(
            mIndex.data(HbDataFormModelItem::ItemTypeRole).toInt());
    //emit activated signal for newly expanded group page
    if(expanded && (itemType == HbDataFormModelItem::GroupPageItem || 
        itemType == HbDataFormModelItem::FormPageItem)) {
        q->emitActivated(q->modelIndex());
    }
    if(mSharedData->mItemView) {
        container = qobject_cast<HbAbstractItemContainer *>(
            static_cast<QGraphicsWidget *>(mSharedData->mItemView->contentWidget()));
        if(container->itemTransientState(mIndex).value("expanded")  == expanded || !mSharedData->mItemView) {
            return true;
        }    
        // expand function sets correct expansion itemstate for this item and its children
        changeExpansionState(expanded);
        
        // if some one exlicitly calls setExpanded for data group then primitives needs to be updated.
        if(itemType == HbDataFormModelItem::GroupItem){
            if(mPageCombo) {
                if(expanded) {
                    HbStyle::setItemName(mPageCombo,"dataGroup_Combo");
                    HbStyle::setItemName(mPageComboBackgroundItem,"dataGroup_ComboBackground");

                } else {

                    HbStyle::setItemName(mPageCombo,"");
                    HbStyle::setItemName(mPageComboBackgroundItem,"");
                    q->setProperty("groupPage", "");
                    mPageString.clear();
                    delete mPageCombo;
                    mPageCombo = 0;
                    delete mPageComboBackgroundItem;
                    mPageComboBackgroundItem = 0;

                    q->repolish();
                }
            }
        }
        // setModelIndexes will be create or delete items according to the 
        // itemTransient state set using expand function call above
        container->setModelIndexes(mIndex.operator const QModelIndex & ());
    }
    q->updatePrimitives();
    return true;

}

void HbDataGroupPrivate::setEnabled( bool enabled )
{
    Q_Q(HbDataGroup);

    QGraphicsItem::GraphicsItemFlags itemFlags = q->flags( );
    Qt::ItemFlags indexFlags = mIndex.flags( );

    if ( indexFlags & Qt::ItemIsEnabled ) {
        if ( !( itemFlags & QGraphicsItem::ItemIsFocusable ) ) {
            itemFlags |= QGraphicsItem::ItemIsFocusable;
            q->setFocusPolicy( q->prototype( )->focusPolicy( ) );
            q->setProperty( "state", "normal" );
            q->setEnabled( true );
            q->grabGesture( Qt::TapGesture );
        }
    } else {
        if ( itemFlags & QGraphicsItem::ItemIsFocusable ) {
            itemFlags &= ~QGraphicsItem::ItemIsFocusable;
            q->setFocusPolicy( Qt::NoFocus );
            q->setProperty( "state", "disabled" );
            q->setEnabled( false );
            q->ungrabGesture( Qt::TapGesture );
        }
    }

    if( mGroupHeading ) {
        mGroupHeading->setEnabled(enabled);
        mGroupHeading->updatePrimitives();
    }
}

// This function gets called  for group item or for grouppage item . It returs the corresponding page index .
// The relevance of this function is when some items are inserted before group page inside a group.
// returns -1 if not valid index.
int HbDataGroupPrivate::pageIndex(const QModelIndex &index) const 
{
    const Q_Q(HbDataGroup);
    int pageIndex = -1;
    HbDataFormModelItem::DataItemType indexType = static_cast<HbDataFormModelItem::DataItemType>(
        index.data( HbDataFormModelItem::ItemTypeRole).toInt( ) );

    // Make sure that the child is groupPage type and current item is group.
    if( (indexType == HbDataFormModelItem::GroupPageItem) || (indexType == HbDataFormModelItem::FormPageItem) ) {
        HbDataFormModelItem *modelItem = 
            static_cast<HbDataFormModel*>((q->itemView())->model())->itemFromIndex(
                q->modelIndex( ));
        HbDataFormModelItem *groupModelItem = 0;
        // get the group modelItem pointer depending upon the type
        if( modelItem->type() == HbDataFormModelItem::GroupItem ) {
            groupModelItem = modelItem;
        } else if( modelItem->type() == HbDataFormModelItem::GroupPageItem ) {
            groupModelItem = modelItem->parent();
        } else if (modelItem->type() == HbDataFormModelItem::FormPageItem ) {
            groupModelItem = static_cast<HbDataFormModel*>(q->itemView()->model())->invisibleRootItem();
        }
 
        int childCount = groupModelItem->childCount();
        for( int i = 0; i < childCount; i++) {
            HbDataFormModelItem *child = groupModelItem->childAt(i);
            if( (child->type() == HbDataFormModelItem::GroupPageItem ) || (child->type() == HbDataFormModelItem::FormPageItem)) {
                pageIndex ++;
                // get the index of groupPage
                QModelIndex childIndex = static_cast<HbDataFormModel*>(q->itemView()->model())->indexFromItem(child);
                if(childIndex == index) {
                    break;// we got the page index
                }
            }        
        }
    }
    return pageIndex;
}

QModelIndex HbDataGroupPrivate::pageModelIndex(int index) const 
{
    const Q_Q(HbDataGroup);
    int pageIndex = -1;
    QModelIndex modelIndex;
    
    // Make sure that the child is groupPage type and current item is group.
    if( index >= 0) {

        HbDataFormModelItem *modelItem = 
            static_cast<HbDataFormModel*>((q->itemView())->model())->itemFromIndex(
                q->modelIndex( ));
        HbDataFormModelItem *groupModelItem = 0;
        // get the group modelItem pointer depending upon the type
        if( modelItem->type() == HbDataFormModelItem::GroupItem ) {
            groupModelItem = modelItem;
        } else if( modelItem->type() == HbDataFormModelItem::GroupPageItem ) {
            groupModelItem = modelItem->parent();
        }

        int childCount = groupModelItem->childCount();

        for( int i = 0; i < childCount; i++) {

            HbDataFormModelItem *child = groupModelItem->childAt(i);
            if( child->type() == HbDataFormModelItem::GroupPageItem ) {
                pageIndex ++;
                // get the index of groupPage
                if(pageIndex == index) {
                modelIndex = static_cast<HbDataFormModel*>(q->itemView()->model())->indexFromItem(child);
                break; 
                    
                }
            }        
        }
    }
    return modelIndex;
}



void HbDataGroupPrivate::changeExpansionState( bool expanded )
{
    Q_Q(HbDataGroup);
    // get container for setting item state
    HbAbstractItemContainer *container = qobject_cast<HbAbstractItemContainer *>(
        static_cast<QGraphicsWidget *>( mSharedData->mItemView->contentWidget( ) ) );
    HbDataFormModelItem::DataItemType itemType = static_cast<HbDataFormModelItem::DataItemType>(
            ( mIndex.operator const QModelIndex & ( )).data( HbDataFormModelItem::ItemTypeRole).toInt( ) );

   if(container->itemTransientState(mIndex).value( "expanded" ) == expanded ) {
        return;
    }

    if( !expanded ) {
        if( mGroupHeading ) {
            mGroupHeading->mExpanded = false;
        }
    } else { //expand
        // here we are checking only for HbDataGroup since for HbDataFormPage expansion will happen through 
        // setModelIndexes. For group we need to expand the current active grouppage . 
        if(itemType == HbDataFormModelItem::GroupItem){
            int page = activePage();
            if(page != -1) {
                QModelIndex groupPageIndex = pageModelIndex(page);
                
                if(groupPageIndex.isValid()) {                    
                    // set transient state for group page so that when it gets created then its  
                    // child items are also created
                    container->setItemTransientStateValue(groupPageIndex, "expanded", true);
                    q->emitActivated(groupPageIndex);                   
                }
            }
            if (mGroupHeading )  {
                mGroupHeading->mExpanded = true;
            }
        }
    }
    // save the item state for this item. All the cild item state are saved above
    container->setItemTransientStateValue(mIndex, "expanded", expanded);
}


/*QString HbDataGroupPrivate::groupPage() const
{   
    return mPageString;
}
*/

void HbDataGroupPrivate::setGroupPage( const QString &page ) 
{
   Q_Q(HbDataGroup);

    if( !mPageCombo ) {
        // This is the first groupPgae getting added so create the combobox to add the page
        mPageCombo = new HbComboBox( q );
        mPageString = ' ';
        q->setProperty("groupPage", page);
        HbStyle::setItemName(mPageCombo,"dataGroup_Combo");   

        if ( !mPageComboBackgroundItem) {
            mPageComboBackgroundItem = HbStylePrivate::createPrimitive(HbStylePrivate::P_DataGroupComboBackground, q);
            HbStyle::setItemName(mPageComboBackgroundItem,"dataGroup_ComboBackground");
        }

        q->repolish();
    }

    // disconnecting to avoid pagechanged signal when setting group.
    QObject::disconnect(mPageCombo,SIGNAL(currentIndexChanged(int)),
            q,SLOT(pageChanged(int)));

    QStringList list  = mPageCombo->items();    
    if(!list.contains(page)) {
        mPageCombo->addItem(page);        
        mPageString = page; 
    }
    int pageIndex = activePage();
    if(pageIndex!= -1) {
        mPageCombo->setCurrentIndex(pageIndex);
    }

    QObject::connect(mPageCombo,SIGNAL(currentIndexChanged(int)),
            q ,SLOT(pageChanged(int)));
}

void HbDataGroupPrivate::removeGroupPage(const QString &page)
{    

    if(mPageCombo) {
        // if we are removing the last page then set current page as 0
        if( mPageCombo->findText(page) + 1 == mPageCombo->count()) {
            mPageCombo->setCurrentIndex(0);
        } else {// set next page as the currrent page
            mPageCombo->setCurrentIndex(mPageCombo->findText(page) + 1);
        }
        // remove the text from ombobox
        mPageCombo->removeItem(mPageCombo->findText(page));
    }
}
/*
int HbDataGroupPrivate::pageChanged(const QModelIndex &modelIndex)
{

}*/

int HbDataGroupPrivate::activePage( )
{
    Q_Q( HbDataGroup );
    // Here we need to find which page has the itemTransientState  true, 
    // which in turn will be the active page. If no page is set as true , then
    // make the 0th page as expanded ie itemTransientState as true

    // This function can be called from Group or GroupPage items 
    QModelIndex groupIndex ;    
    HbDataFormModelItem *modelItem = 
        static_cast<HbDataFormModel*>((q->itemView())->model())->itemFromIndex(
            q->modelIndex( ));
    HbDataFormModelItem *groupModelItem = 0;
    // get the group modelItem pointer depending upon the type
    if( modelItem->type() == HbDataFormModelItem::GroupItem ) {
        groupModelItem = modelItem;
    } else if( modelItem->type() == HbDataFormModelItem::GroupPageItem ) {
        groupModelItem = modelItem->parent();
    }
    int childCount = groupModelItem->childCount();
    //int activePage = 0;
    // get container for setting item state
    HbAbstractItemContainer *container = qobject_cast<HbAbstractItemContainer *>(
        static_cast<QGraphicsWidget *>( mSharedData->mItemView->contentWidget( ) ) );
    int currentPage = -1; 
    bool somePageExpanded = false;
    for( int i = 0; i < childCount; i++) {
        // Make sure that the child is groupPage type.
        HbDataFormModelItem *child = groupModelItem->childAt(i);
        if( child->type() == HbDataFormModelItem::GroupPageItem ) {
            // Here we are not using i as the page index since there can other types of children in the same level
            // so need a seperate variable for this 
            currentPage ++;
            
            // get the index of groupPage
            QModelIndex pageIndex = static_cast<HbDataFormModel*>(q->itemView()->model())->indexFromItem(child);
            if(container->itemTransientState(pageIndex).value("expanded").toBool()) {
                somePageExpanded = true;
                break;
            }
            
        }        
    }
    if(!somePageExpanded && currentPage != -1) {// This means pages are present and no page is expanded so expand first page
        currentPage = 0;
    }
    return currentPage;
}

void HbDataGroupPrivate::setHeading( const QString &heading )
{ 
    Q_Q( HbDataGroup );
    mGroupHeading->mHeading = heading;
    mGroupHeading->createPrimitives( );    
    q->updatePrimitives( );
}


QString HbDataGroupPrivate::heading() const
{    
    return mGroupHeading->mHeading;
}

HbDataGroup::HbDataGroup(QGraphicsItem *parent)
    :HbDataFormViewItem(* new HbDataGroupPrivate( this ), parent)
{    
}

HbDataGroup::HbDataGroup(const HbDataGroup &source):
    HbDataFormViewItem( *new HbDataGroupPrivate(*source.d_func()), 0)
{
    Q_D(HbDataGroup);
    d->q_ptr = this;
    d->init( );   
}

HbDataGroup::~HbDataGroup()
{
}

void HbDataGroup::initStyleOption(HbStyleOptionDataGroup *option)
{
    //Q_D(HbDataGroup);
    HbWidget::initStyleOption(option);
}

void HbDataGroup::setDescription( const QString &description )
{
     
    Q_D( HbDataGroup );
    d->mGroupHeading->mDescription = description;
    if(d->mGroupHeading->mDescription.isEmpty()) {
        setProperty("hasDescription",false);
    } else {
        setProperty("hasDescription",true);
    }
    d->mGroupHeading->createPrimitives( );    
    d->mGroupHeading->updatePrimitives( );
}


QString HbDataGroup::description() const
{
    Q_D(const HbDataGroup);
    return d->mGroupHeading->mDescription;
}

bool HbDataGroup::setExpanded( bool expanded )
{
    Q_D(HbDataGroup);
    if(d->mSharedData->mItemView){
        HbDataFormModelItem *modelItem = static_cast<HbDataFormModel*>(
                    d->mSharedData->mItemView->model())->itemFromIndex(modelIndex());
        HbDataFormModelItem::DataItemType contentWidgetType =
            static_cast<HbDataFormModelItem::DataItemType>(
            (d->mIndex.data(HbDataFormModelItem::ItemTypeRole)).toInt());
        if( contentWidgetType == HbDataFormModelItem::GroupItem ) {
            d->setExpanded(expanded);
        } else if (contentWidgetType == HbDataFormModelItem::GroupPageItem) {
            //We need to change even the combobox state also so call pageChanged fuction
            if(modelItem) {
                int page = d->pageIndex(d->mIndex);
                if(page != -1) {
                    if(!expanded) {
                        QModelIndex parentIndex = d->mIndex.parent();
                        HbDataGroup *parentGroup =
                        static_cast<HbDataGroup *>(d->mSharedData->mItemView->itemByIndex(parentIndex));
                        if(parentGroup && HbDataGroupPrivate::d_ptr(parentGroup)->mPageCombo) {
                            if(HbDataGroupPrivate::d_ptr(parentGroup)->mPageCombo->count() > (page + 1) ) {
                                HbDataGroupPrivate::d_ptr(parentGroup)->mPageCombo->setCurrentIndex(page + 1);
                                //parentGroup->pageChanged(0);
                            } else {
                                HbDataGroupPrivate::d_ptr(parentGroup)->mPageCombo->setCurrentIndex(0);
                                //parentGroup->pageChanged(1);
                            }
                        }
                    } else {
                        pageChanged(page);
                    }
                }
            }
        } else if(contentWidgetType == HbDataFormModelItem::FormPageItem) {
            if(modelItem) {
                int formPageIndex = d->pageIndex(d->mIndex);
                if( formPageIndex!= -1) {
                    HbDataFormPrivate::d_ptr(
                    static_cast<HbDataForm*>(d->mSharedData->mItemView))->_q_page_changed(formPageIndex);
                }
            }
        }
        return true;
    } else {
        return false;
    }   
}

bool HbDataGroup::isExpanded() const
{
    Q_D(const HbDataGroup);
    HbDataFormModelItem::DataItemType contentWidgetType =
        static_cast<HbDataFormModelItem::DataItemType>(
        (d->mIndex.data(HbDataFormModelItem::ItemTypeRole)).toInt());
    if( contentWidgetType < HbDataFormModelItem::SliderItem ) {
        HbAbstractItemContainer *container = qobject_cast<HbAbstractItemContainer *>(
            static_cast<QGraphicsWidget *>(d->mSharedData->mItemView->contentWidget()));
        if(container) {
            return container->itemTransientState(d->mIndex).value("expanded").toBool();
        }
    }
    return false;
}

void HbDataGroup::updateGroupPageName(int index , const QString &page)
{
    Q_D(HbDataGroup);
    if(index >= 0 && d->mPageCombo) {       
        if( d->mPageCombo->itemText(index) != page)  {          
            d->mPageCombo->setItemText(index,page);           
        }       
    }
}

void HbDataGroup::updatePrimitives()
{
    Q_D(HbDataGroup);

    //update data group heading primitives
    if(d->mGroupHeading) {
        d->mGroupHeading->updatePrimitives();        

        if( d->mGroupHeading->mExpanded ) {
            //update data group primitives
            HbStyleOptionDataGroup opt;
            initStyleOption(&opt);

            //update the combo background
            if ( d->mPageComboBackgroundItem ) {
                HbStylePrivate::updatePrimitive(
                    d->mPageComboBackgroundItem, HbStylePrivate::P_DataGroupComboBackground, &opt);
            }

            //update the data group description
            /*if(d->mGroupDescriptionItem) {
                HbStylePrivate::updatePrimitive( 
                    d->mGroupDescriptionItem, HbStylePrivate::P_DataGroup_description, &opt);
            }*/
        }
    }
}

/*!
    \reimp
 */
QVariant HbDataGroup::itemChange( GraphicsItemChange change, const QVariant &value )
{
    switch ( static_cast<HbPrivate::HbItemChangeValues>( change ) ) {
        case QGraphicsItem::ItemEnabledHasChanged: {
            updatePrimitives( );
            //We are skipping call to abstractviewitem::itemChange here because updateChildItems is 
            //called in that function which will again create data group primitives.
            return HbWidget::itemChange( change, value );
       }
        default:
            break;
    }
    return HbDataFormViewItem::itemChange( change, value );
}

void HbDataGroup::pageChanged(int index)
{
    Q_D(HbDataGroup);

    HbDataFormModelItem::DataItemType itemType =
        static_cast<HbDataFormModelItem::DataItemType>(
        (d->mIndex.data(HbDataFormModelItem::ItemTypeRole)).toInt());
    

    if(!itemView()) {
        return;
    }

    HbAbstractItemContainer *container = qobject_cast<HbAbstractItemContainer *>(
                static_cast<QGraphicsWidget *>(d->mSharedData->mItemView->contentWidget()));

    // Get Previous Active Group Page
    QModelIndex previousPageIndex;
    QModelIndex currentPageIndex;
    HbDataGroup* group = this;
    // this function can get called for both group and grouppage
    // active grouppage is always stored in group modelitem so get the next index and 
    // activepage according to group

    int previous_page = d->activePage();
    if(previous_page != -1) {
        previousPageIndex = d->pageModelIndex(previous_page);  
    }
    currentPageIndex =  d->pageModelIndex(index); 

    if(itemType == HbDataFormModelItem::GroupPageItem) {
        // need to fetch group (parent) for getting previus page and active page
        group = static_cast<HbDataGroup*>(itemView()->itemByIndex(modelIndex().parent()));
        if(HbDataGroupPrivate::d_ptr(group)->mPageCombo) {
            if(HbDataGroupPrivate::d_ptr(group)->mPageCombo->currentIndex() != index) {

                // combobox has to be changed explicitly here since the call happened on GroupPage item
                QObject::disconnect(HbDataGroupPrivate::d_ptr(group)->mPageCombo,SIGNAL(currentIndexChanged(int)),
                        group ,SLOT(pageChanged(int)));
                HbDataGroupPrivate::d_ptr(group)->mPageCombo->setCurrentIndex(index);
                QObject::connect(d->mPageCombo,SIGNAL(currentIndexChanged(int)),
                        group ,SLOT(pageChanged(int)));
            }
        }
    }
    
    if(index != HbDataGroupPrivate::d_ptr(group)->activePage()) {// If the page is different

        // Collapse previous group page
        if(previousPageIndex.isValid()) {
            HbDataGroup* previousPage = static_cast<HbDataGroup*>(itemView()->itemByIndex(previousPageIndex));
            //HbDataGroupPrivate::d_ptr(group)->setActivePage(index);
            
            if(previousPage) {
                HbDataGroupPrivate::d_ptr(previousPage)->setExpanded(false);
            }else {
                container->setItemTransientStateValue(previousPageIndex, "expanded", false);
            }
        }

        // Expand current selected page set as active page        
        if(currentPageIndex.isValid()) {
            HbDataGroup* currentPage = static_cast<HbDataGroup*>(
                (itemView())->itemByIndex(currentPageIndex));
            if(currentPage) {
                HbDataGroupPrivate::d_ptr(currentPage)->setExpanded(true);
            } else {
                container->setItemTransientStateValue(currentPageIndex, "expanded", true);
            }
        }
    } else {//If selected page is same then expand it if it is not expanded already
        HbDataGroup* currentPage = static_cast<HbDataGroup*>(
            (itemView())->itemByIndex(previousPageIndex));
        if(currentPage && !currentPage->isExpanded()) {
            HbDataGroupPrivate::d_ptr(currentPage)->setExpanded(true);
        } else {
            container->setItemTransientStateValue(currentPageIndex, "expanded", true);
        } 
    }
}


HbDataFormViewItem* HbDataGroup::createItem()
{
    return new HbDataGroup(*this);
}

void HbDataGroup::updateChildItems()
{
    Q_D(HbDataGroup);
    HB_SD(HbAbstractViewItem);
     if( d->mIndex.isValid( ) ) {
        d->mType = static_cast<HbDataFormModelItem::DataItemType>(
            d->mIndex.data(HbDataFormModelItem::ItemTypeRole).toInt( ) );
    }

    if( d->mSharedData && d->mSharedData->mItemView ) {
        d->mModel = static_cast<HbDataFormModel*>( d->mSharedData->mItemView->model( ) );
        d->mModelItem = d->mModel->itemFromIndex( modelIndex( ) );
    }
    HbAbstractItemContainer *container = qobject_cast<HbAbstractItemContainer *>(
            static_cast<QGraphicsWidget *>(d->mSharedData->mItemView->contentWidget()));

    HbDataFormModelItem::DataItemType contentWidgetType = 
            static_cast<HbDataFormModelItem::DataItemType>(
            (d->mIndex.data(HbDataFormModelItem::ItemTypeRole)).toInt());
    connect(this, SIGNAL(itemShown(const QModelIndex&)), 
                d->mSharedData->mItemView, SIGNAL(itemShown(const QModelIndex&)));   

    if ( contentWidgetType == HbDataFormModelItem::GroupItem ) {
        d->mGroupHeading = new HbDataGroupHeadingWidget();
        HbStyle::setItemName(d->mGroupHeading,"dataGroup_HeadingWidget");
        d->mGroupHeading->setParentItem(this);
        d->mGroupHeading->mParent = this;
        d->mGroupHeading->createPrimitives();
        //QEvent polishEvent(QEvent::Polish);
        //QCoreApplication::sendEvent(d->mGroupHeading, &polishEvent);

        //set the heading of data group
        QString groupHeading = d->mIndex.data(
                    HbDataFormModelItem::LabelRole).toString();
        d->setHeading(groupHeading);

        //set the heading of data group
        QString groupDescription = d->mIndex.data(
            HbDataFormModelItem::DescriptionRole).toString();
        if(!groupDescription.isEmpty()) {
            setDescription(groupDescription);
        } else {
            setProperty("hasDescription",false);
        }

        //update visualization based on whether item is enabled or disabled
        HbDataFormModel* data_model = static_cast<HbDataFormModel*>( itemView( )->model( ) );
        HbDataFormModelItem *model_item = 
            static_cast<HbDataFormModelItem*>( data_model->itemFromIndex( d->mIndex ) );
        d->setEnabled( model_item->isEnabled( ) );

        if(container) {
            if(container->itemTransientState(d->mIndex).value("expanded").toBool()) {
                container->setItemTransientStateValue(d->mIndex, "expanded", false);
                setExpanded(true);
            }
        }
    } 
    else if( contentWidgetType == HbDataFormModelItem::GroupPageItem){
            QModelIndex parentIndex = d->mIndex.parent();
            HbDataGroup *parentGroup =
                static_cast<HbDataGroup *>(sd->mItemView->itemByIndex(parentIndex));
            QString groupHeading = d->mIndex.data(
                HbDataFormModelItem::LabelRole).toString();            
            HbDataGroupPrivate::d_ptr(parentGroup)->setGroupPage(groupHeading);

    } else if( contentWidgetType == HbDataFormModelItem::FormPageItem) {
        QString formPageName = d->mIndex.data(
                HbDataFormModelItem::LabelRole).toString();
            HbDataFormPrivate::d_ptr(static_cast<HbDataForm*>(sd->mItemView))->addFormPage(
                formPageName);

    }
    HbAbstractViewItem::updateChildItems();

}

bool HbDataGroup::canSetModelIndex(const QModelIndex &index) const
{
    HbDataFormModelItem::DataItemType itemType = 
        static_cast<HbDataFormModelItem::DataItemType>(
        index.data(HbDataFormModelItem::ItemTypeRole).toInt());

    if( itemType < HbDataFormModelItem::SliderItem ){
        return true;
    }
    return false;
}

void HbDataGroup::polish(HbStyleParameters& params)
{
    Q_D(HbDataGroup);
    HbDataFormModelItem::DataItemType itemType = 
        static_cast<HbDataFormModelItem::DataItemType>(
        d->mIndex.data(HbDataFormModelItem::ItemTypeRole).toInt());
    if( itemType == HbDataFormModelItem::GroupItem ){
        HbDataFormViewItem::polish(params);
    }
}

/*!
    \reimp
*/
// TODO: remove this, temporary workaround to return size zero incase of no contentwidget
QSizeF HbDataGroup::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    //TODO: remove this
    QSizeF size;
    HbDataFormModelItem::DataItemType itemType = 
        static_cast<HbDataFormModelItem::DataItemType>(
        modelIndex().data(HbDataFormModelItem::ItemTypeRole).toInt());

    if(( itemType == HbDataFormModelItem::GroupPageItem )
        ||( itemType == HbDataFormModelItem::FormPageItem )) {
        size.setHeight(0);
    } else {
        size = HbDataFormViewItem::sizeHint(which,constraint );
    }
    return size;       
}

/*!
    \reimp
*/
void HbDataGroup::pressStateChanged(bool value, bool animate)
{
    //Since there are no effects defined for mousePressed and mouseReleased for 
    //HbDataFormViewItem we are overriding this function so that redundant effects functions are
    //not called in HbDataFormViewItem::pressStateChanged.
    Q_UNUSED(value);
    Q_UNUSED(animate);
}

void HbDataGroup::emitActivated(const QModelIndex &index )const
{
    HbDataForm  *form = static_cast<HbDataForm*>(itemView());
    if(form) {
        HbDataFormPrivate::d_ptr(form)->emitActivated(index);
    }
}

