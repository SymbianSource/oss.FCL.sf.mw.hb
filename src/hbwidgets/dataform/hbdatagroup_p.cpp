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

#include <hbcombobox.h>
#include <hbdataformmodel.h>

#include "hbdatagroupheadingwidget_p.h"
#include "hbstyleoptiondatagroup_p.h"
#include "hbabstractitemcontainer_p_p.h"
#include "hbdataform_p.h"
#include "hbdatagroup_p_p.h"


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

void HbDataGroupPrivate::expand( bool expanded )
{
    HbAbstractItemContainer *container = qobject_cast<HbAbstractItemContainer *>(
        static_cast<QGraphicsWidget *>( mSharedData->mItemView->contentWidget( ) ) );
    HbDataFormModelItem::DataItemType itemType = static_cast<HbDataFormModelItem::DataItemType>(
            ( mIndex.operator const QModelIndex & ( )).data( HbDataFormModelItem::ItemTypeRole).toInt( ) );

    if(container->itemTransientState(mIndex).value( "expanded" ) == expanded ) {
        return;
    }

    if( !expanded ) {

        //collapsing all the expanded child Types.
        QModelIndex index = mIndex.operator const QModelIndex & ( );
        QModelIndex childIndex = index.child(0,0);
        while(childIndex.isValid()) {

            HbDataFormModelItem::DataItemType childType = static_cast<HbDataFormModelItem::DataItemType>(
            childIndex.data(HbDataFormModelItem::ItemTypeRole).toInt());

            if(childType <= HbDataFormModelItem::GroupPageItem) {
                HbDataGroup* group_Item = 
                static_cast<HbDataGroup*>(mSharedData->mItemView->itemByIndex(
                    childIndex));
                if(group_Item && group_Item->isExpanded()) {
                    HbDataGroupPrivate::d_ptr(group_Item)->expand(false);               
                
                }
            }
            QModelIndex nextChild = index.child(childIndex.row() +1 ,0);
            childIndex = nextChild;
        }

        if( mGroupHeading ) {
            mGroupHeading->mExpanded = false;
        }

    } else { //expand
        
        if(itemType == HbDataFormModelItem::GroupItem){
            HbDataFormModelItem *modelItem = static_cast<HbDataFormModel*>(
                mSharedData->mItemView->model())->itemFromIndex(mIndex.operator const QModelIndex & ());
            QModelIndex childIndex = (mIndex.operator const QModelIndex & ()).child(0,0);
            HbDataFormModelItem::DataItemType childType = static_cast<HbDataFormModelItem::DataItemType>(
                childIndex.data(HbDataFormModelItem::ItemTypeRole).toInt());
            if( childType == HbDataFormModelItem::GroupPageItem ) {
                QVariant pageIndex = modelItem->contentWidgetData(QString("currentPage"));
                int activePage;
                if(!pageIndex.isValid()) {
                    activePage = 0;
                } else {
                    activePage = pageIndex.toInt();
                }
                //get the group page index
                QModelIndex groupPageIndex = mIndex.child(activePage,0);
                if(groupPageIndex.isValid()) {                    
                    container->setItemTransientStateValue(groupPageIndex, "expanded", true);
                }
            }
            if (mGroupHeading )  {
                mGroupHeading->mExpanded = true;
            }
        }
    }

    container->setItemTransientStateValue(mIndex, "expanded", expanded);
}


QString HbDataGroupPrivate::groupPage() const
{   
    return mPageString;
}


void HbDataGroupPrivate::setGroupPage( const QString &page ) 
{
   Q_Q(HbDataGroup);

    if( !mPageCombo ) {
        mPageCombo = new HbComboBox( q );
        mPageString = " ";
        q->setProperty("groupPage", page);
        HbStyle::setItemName(mPageCombo,"dataGroup_Combo");   

        if ( !mPageComboBackgroundItem) {
            mPageComboBackgroundItem = q->style()->createPrimitive(HbStyle::P_DataGroupComboBackground, q);
            HbStyle::setItemName(mPageComboBackgroundItem,"dataGroup_ComboBackground");
        }

        QEvent polishEvent(QEvent::Polish);
        QCoreApplication::sendEvent(q, &polishEvent);
    }

    // disconnecting to avoid pagechanged signal when setting group.
    QObject::disconnect(mPageCombo,SIGNAL(currentIndexChanged(int)),
            q,SLOT(pageChanged(int)));

    QStringList list  = mPageCombo->items();    
    if(!list.contains(page)) {
        mPageCombo->addItem(page);        
        mPageString = page; 
    }   
    mPageCombo->setCurrentIndex(activePage());

    QObject::connect(mPageCombo,SIGNAL(currentIndexChanged(int)),
            q ,SLOT(pageChanged(int)));
}

void HbDataGroupPrivate::removeGroupPage(const QString &page)
{    
    if(mPageCombo) {
        mPageCombo->removeItem(mPageCombo->findText(page));
    }
}

int HbDataGroupPrivate::activePage( )
{
    Q_Q( HbDataGroup );

    HbDataFormModelItem *modelItem = 
        static_cast<HbDataFormModel*>((q->itemView())->model())->itemFromIndex(
            q->modelIndex( ));
    int page = 0;
    if(modelItem){
        page = modelItem->contentWidgetData(QString("currentPage")).toInt();
    }
    return page;
}

void HbDataGroupPrivate::setActivePage(int pageindex)
{  
    Q_Q( HbDataGroup ); 
   
    HbDataFormModelItem *modelItem = 
        static_cast<HbDataFormModel*>((q->itemView())->model())->itemFromIndex(
            q->modelIndex());
    
    QObject::disconnect( mSharedData->mItemView->model(), SIGNAL( dataChanged( QModelIndex,QModelIndex ) ),
        mSharedData->mItemView, SLOT( dataChanged( QModelIndex,QModelIndex ) ) );

    modelItem->setContentWidgetData(QString("currentPage"),pageindex);

    QObject::connect( mSharedData->mItemView->model(), SIGNAL( dataChanged( QModelIndex,QModelIndex ) ),
        mSharedData->mItemView, SLOT( dataChanged( QModelIndex,QModelIndex ) ) );
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
    HB_SD(HbAbstractViewItem);
    HbAbstractItemContainer *container = 0;

    if(d->mSharedData->mItemView) {
        container = qobject_cast<HbAbstractItemContainer *>(
            static_cast<QGraphicsWidget *>(d->mSharedData->mItemView->contentWidget()));
        if(container->itemTransientState(d->mIndex).value("expanded")  == expanded || !sd->mItemView) {
            return true;
        }    
        d->expand(expanded);
       

        //if some one exlicitly calls setExpanded for data group then primitives needs to be
        //updated.
        HbDataFormModelItem::DataItemType itemType = 
            static_cast<HbDataFormModelItem::DataItemType>(
            d->mIndex.data(HbDataFormModelItem::ItemTypeRole).toInt());
        if(itemType == HbDataFormModelItem::GroupItem){
            if(d->mPageCombo) {
                if(expanded) {

                    HbStyle::setItemName(d->mPageCombo,"dataGroup_Combo");
                    HbStyle::setItemName(d->mPageComboBackgroundItem,"dataGroup_ComboBackground");
                    //HbStyle::setItemName(d->mGroupDescriptionItem, "dataGroup_Description");

                } else {

                    HbStyle::setItemName(d->mPageCombo,"");
                    HbStyle::setItemName(d->mPageComboBackgroundItem,"");
                    //HbStyle::setItemName(d->mGroupDescriptionItem, "");
                    setProperty("groupPage", "");
                    d->mPageString.clear();
                    delete d->mPageCombo;
                    d->mPageCombo = 0;
                    delete d->mPageComboBackgroundItem;
                    d->mPageComboBackgroundItem = 0;
                    //delete d->mGroupDescriptionItem;
                    //d->mGroupDescriptionItem = 0;

                    QEvent polishEvent(QEvent::Polish);
                    QCoreApplication::sendEvent(this, &polishEvent);
                }
            }
        }
        container->setModelIndexes(d->mIndex.operator const QModelIndex & ());
    }
    updatePrimitives();
    return true;
}

bool HbDataGroup::isExpanded() const
{
    Q_D(const HbDataGroup);
    HbDataFormModelItem::DataItemType contentWidgetType =
        static_cast<HbDataFormModelItem::DataItemType>(
        (d->mIndex.data(HbDataFormModelItem::ItemTypeRole)).toInt());
    if( contentWidgetType == HbDataFormModelItem::GroupItem ) {
        if(d->mGroupHeading) {
            return d->mGroupHeading->mExpanded;
        }
    } else if ( contentWidgetType == HbDataFormModelItem::GroupPageItem ) {
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
                style()->updatePrimitive(
                    d->mPageComboBackgroundItem, HbStyle::P_DataGroupComboBackground, &opt);
            }

            //update the data group description
            /*if(d->mGroupDescriptionItem) {
                style()->updatePrimitive( 
                    d->mGroupDescriptionItem, HbStyle::P_DataGroup_description, &opt);
            }*/
        }
    }
}


void HbDataGroup::pageChanged(int index)
{
    Q_D(HbDataGroup);
    QStringListModel *model = (QStringListModel*)d->mPageCombo->model();
    QModelIndex changedIndex = model->index(index, 0);
    if(!itemView()) {
        return;
    }
    if(changedIndex.isValid()) {
        // Get Previous Active Group Page
        QModelIndex previousPageIndex = modelIndex().child(d->activePage(),0);
        if(changedIndex.row() != d->activePage()) {// If the page is different
            // Collapse previous group page
            HbDataGroup* previousPage = static_cast<HbDataGroup*>(itemView()->itemByIndex(previousPageIndex));
            d->setActivePage(changedIndex.row());
            if(previousPage) {
                previousPage->setExpanded(false);
            }
            // Expand current selected page set as active page
            QModelIndex currentPageIndex = modelIndex().child(changedIndex.row(),0);
            if(currentPageIndex.isValid()) {
                HbDataGroup* currentPage = static_cast<HbDataGroup*>(
                    (itemView())->itemByIndex(currentPageIndex));
                if(currentPage) {
                    currentPage->setExpanded(true);
                }
            }
        } else {//If selected page is same then expand it if it is not expanded already
            HbDataGroup* currentPage = static_cast<HbDataGroup*>(
                (itemView())->itemByIndex(previousPageIndex));
            if(currentPage && !currentPage->isExpanded()) {
                currentPage->setExpanded(true);
            }
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

    HbDataFormModelItem::DataItemType contentWidgetType = 
            static_cast<HbDataFormModelItem::DataItemType>(
            (d->mIndex.data(HbDataFormModelItem::ItemTypeRole)).toInt());

    if ( contentWidgetType == HbDataFormModelItem::GroupItem ) {
        d->mGroupHeading = new HbDataGroupHeadingWidget();
        HbStyle::setItemName(d->mGroupHeading,"dataGroup_HeadingWidget");
        d->mGroupHeading->setParentItem(this);
        d->mGroupHeading->mParent = this;
        d->mGroupHeading->createPrimitives();
        QEvent polishEvent(QEvent::Polish);
        QCoreApplication::sendEvent(d->mGroupHeading, &polishEvent);

        //set the heading of data group
        QString groupHeading = d->mIndex.data(
                    HbDataFormModelItem::LabelRole).toString();
        d->setHeading(groupHeading);

        //set the heading of data group
        QString groupDescription = d->mIndex.data(
            HbDataFormModelItem::DescriptionRole).toString();
        if(!groupDescription.isEmpty()) {
            setDescription(groupDescription);
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

