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

#include "hbdataformmodel_p.h"
#include "hbdataformmodelitem_p.h"

#include <hbdataformmodelitem.h>
#include <hbdataformmodel.h>

/*
    \internal

    adds the dataformviewitem or page or group depending upon /a itemType
    and return pointer to newly created form item.
    \a label : Label for the dataformItem. In case of GroupPage and FormPage,
               label is added to parents combobox as one of the selectors.
    \a parent : is parent of item 

*/
HbDataFormModelItem* HbDataFormModelPrivate::addItem(
    HbDataFormModelItem::DataItemType itemType , const QString& label, HbDataFormModelItem *parent)
{
    if(!parent) {
        parent = mRoot;
    }
    return insertItem( parent->childCount(), itemType , label , parent);
}

/*
    \internal

    insert  data form item or page or group depending upon /a itemType
    and return pointer to newly created form item.
    \a label :  Heading for data group , group tital in case of group page and page tital in case
    item us data page item.
    \a parent : is parent of item 

*/
HbDataFormModelItem* HbDataFormModelPrivate::insertItem(
    int index,HbDataFormModelItem::DataItemType itemType ,const QString& label, HbDataFormModelItem *parent)
{
    if(!parent) {
        parent = mRoot;
    }
    if(itemType == HbDataFormModelItem::GroupPageItem && (parent->type() != HbDataFormModelItem::GroupItem)) {
        return 0;        
    }
    HbDataFormModelItem *item = new HbDataFormModelItem(itemType, label);
    parent->insertChild(index , item);
    return item;
}

/*
  \internal
*/
void HbDataFormModelPrivate::rowsAboutToBeInserted(HbDataFormModelItem *parent,
    int start, int end)
{
    Q_Q(HbDataFormModel);
    QModelIndex index = q->indexFromItem(parent);
    q->beginInsertRows(index, start, end);
}

/*
  \internal
*/
void HbDataFormModelPrivate::rowsAboutToBeRemoved(HbDataFormModelItem *parent,
    int start, int end)
{
    Q_Q(HbDataFormModel);
    QModelIndex index = q->indexFromItem(parent);
    q->beginRemoveRows(index, start, end);
}

/*
  \internal
*/
void HbDataFormModelPrivate::rowsInserted()
{
    Q_Q(HbDataFormModel);
    q->endInsertRows();
}

/*
  \internal
*/
void HbDataFormModelPrivate::rowsRemoved()
{
    Q_Q(HbDataFormModel);
    q->endRemoveRows();
}


/*!
   @beta
   @hbwidgets
   \class HbDataFormModel hbdataformmodel.h
   \brief The HbDataFormModel class provides data model for HbDataForm.
   \ingroup model-view

    HbDataFormModel is derived from QAbstractItemModel. So applications can use,
    QAbstractItemModel API's to create their datamodel. HbDataFormModel also provides
    convenience API's specific to HbDataForm. These convenience API's are useful in creating 
    FormPageItem, GroupItem, GroupPageItem and data item.

    A HbDataForm can be used to display the contents of the model.
    HbDataFormModel also has APIs to return modelindex of the items and vice-versa.
    So applications can individually modify the items data and set it to the HbDataForm.

    The signals emitted by HbDataFormModel
    \li dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight) emitted when the 
    HbDataFormModel is updated \atopLeft and \abottomRight will be same since every node has only one column.
    User can connect to this signal and can fetch the instance of HbDataFormViewItem from HbDataForm 
    using the API dataFormViewItem(const QModelIndex &index) or user can fetch HbDataFormModelItem using API 
    itemFromIndex(const QModelIndex &index) in HbDataFormModel .When user updates model using 
    setContentWidgetData API provided in HbDataFormModelItem class, then DataForm takes care of updating the 
    corresponding item's visualization.
 */

/*!
    Constructs a new data form model with the given \a parent.
    
*/
HbDataFormModel::HbDataFormModel(QObject *parent)
    :QAbstractItemModel(parent),d_ptr(new HbDataFormModelPrivate )
{
    Q_D(HbDataFormModel);
    d->q_ptr = this; 
    HbDataFormModelItemPrivate::d_ptr(d->mRoot)->setModel(this);   
}

/*!
    Destructor.    
*/
HbDataFormModel::~HbDataFormModel()
{
    Q_D(HbDataFormModel);
    removeItem(d->mRoot);
    delete d_ptr;
}

/*!
    @beta

    Appends FormPageItem and returns pointer to newly created HbDataFormModelItem.
    The parent of FormPageItem is always model's root item. The DataItemType is set
    as FormPageItem.

    \a label Label for data page. This label will be added in the top level combo box.

    \sa insertDataFormPage

*/
HbDataFormModelItem* HbDataFormModel::appendDataFormPage(const QString &label)
{
    Q_D(HbDataFormModel);
    return d->addItem(HbDataFormModelItem::FormPageItem,label,0);
}

/*!
    @beta

    Appends GroupItem and returns pointer to newly created HbDataFormModelItem.
    The DataItemType is set as GroupItem.

    \a label Label for data group. This label will be set as a group heading.
    \a parent Parent of item. The parent of GroupItem can be either model's root
        index or FormPageItem.

    \sa insertDataFormGroup
    
*/
HbDataFormModelItem* HbDataFormModel::appendDataFormGroup(const QString &label,
    HbDataFormModelItem *parent)
{
    Q_D(HbDataFormModel);
    return d->addItem(HbDataFormModelItem::GroupItem,label,parent);
}

/*!
    @beta

    Appends GroupPageItem and returns pointer to newly created HbDataFormModelItem.
    The parent of GroupPageItem can only be GroupItem. If parent passed is other than
    GroupItem then this item is not appended to model and returns 0. The DataItemType
    is set as GroupPageItem.

    \a label Label for data group page. This label will be added in group combo box.

    \a parent Parent of item which can be only GroupItem.

    \sa insertDataFormGroupPage
    
*/
HbDataFormModelItem* HbDataFormModel::appendDataFormGroupPage(const QString &label,
    HbDataFormModelItem *parent)
{
    Q_D(HbDataFormModel);    
    return d->addItem(HbDataFormModelItem::GroupPageItem,label,parent);
    
}

/*!
    @beta

    Appends data item and returns pointer to newly created HbDataFormModelItem.

    \a itemType Type of data item. It can be anything in 
        HbDataFormModelItem::DataItemType other than FormPageItem, GroupItem and 
        GroupPageItem.
    \a label Label for data item.
    \a parent Parent of data item. Parent can be model's root index, FormPageItem, 
       GroupPageItem or GroupItem.

    \sa insertDataFormItem
    
*/
HbDataFormModelItem* HbDataFormModel::appendDataFormItem(
    HbDataFormModelItem::DataItemType itemType ,const QString &label,
    HbDataFormModelItem *parent)
{
    Q_D(HbDataFormModel);
    return d->addItem(itemType,label,parent);
}

/*!
    @beta

    This is a convenience API. If user wants then he can create HbDataFormModelItem 
    individually and then add that item in model using this API.
    If the \a data is of FormpageItemtype then parent is not considered. FormPage Items are always added
    to rootItem. Also GroupPageItem has to be inserted only in GroupItem.

    \a data Child item to be inserted.
    \a parent Parent of DataFormViewItem

    \sa insertDataFormItem
    
*/
void HbDataFormModel::appendDataFormItem(HbDataFormModelItem *data, HbDataFormModelItem *parent)
{
    if(!data)
        return;
   
    HbDataFormModelItem::DataItemType itemType = data->type();

    if(!parent || itemType == HbDataFormModelItem::FormPageItem) {
        parent = invisibleRootItem();
    }

    
    HbDataFormModelItem::DataItemType parentType = parent->type();

    if(itemType == HbDataFormModelItem::GroupPageItem && parentType != HbDataFormModelItem::GroupItem)
        return;

    
    parent->appendChild(data);
}

/*!
    @beta

    Inserts FormPageItem at the specified index and returns pointer to newly created 
    HbDataFormModelItem. The parent can be only model's root index.

    \a index Index where FormPageItem has to be inserted.
    \a label Label of FormPageItem. This will be added in top level combo

    \sa appendDataFormPage
    
*/
HbDataFormModelItem* HbDataFormModel::insertDataFormPage(int index,const QString &label)
{
    Q_D(HbDataFormModel);
    return d->insertItem(index , HbDataFormModelItem::FormPageItem , label);
}

/*!
    @beta

    Inserts GroupItem at the specified index and returns pointer to newly created 
    HbDataFormModelItem.

    \a index Index where GroupItem has to be inserted.
    \a label Label for GroupItem. This will be group heading.
    \a parent Parent of item 

    \sa appendDataFormGroup
    
*/
HbDataFormModelItem* HbDataFormModel::insertDataFormGroup(int index, const QString &label,
    HbDataFormModelItem *parent)
{
    Q_D(HbDataFormModel);
    return d->insertItem(index , HbDataFormModelItem::GroupItem ,label ,parent);
}

/*!
    @beta

    Inserts GroupPageItem at the specified index and returns pointer to newly created 
    HbDataFormModelItem. Return 0 if parent passed is other than GroupItem.

    \a index Index where GroupPageItem has to be inserted.
    \a label Label for GroupPageItem. This will be added in group combo box.
    \a parent Parent of item

    \sa appendDataFormGroupPage
    
*/
HbDataFormModelItem* HbDataFormModel::insertDataFormGroupPage(
    int index, const QString &label, HbDataFormModelItem *parent)
{
    Q_D(HbDataFormModel);    
    return d->insertItem(index , HbDataFormModelItem::GroupPageItem, label ,parent);  
}

/*!
    @beta

    Inserts data item at the specified index and returns pointer to newly created 
    HbDataFormModelItem.

    \a index Index where data item has to be inserted.
    \a label Label for data page.
    \a parent Parent of item

    \sa appendDataFormItem
    
*/
HbDataFormModelItem* HbDataFormModel::insertDataFormItem(int index,
    HbDataFormModelItem::DataItemType itemType ,const QString &label,
    HbDataFormModelItem *parent)
{
    Q_D(HbDataFormModel);
    return d->insertItem(index ,itemType , label ,parent);
}

/*!
    @beta

    Inserts given HbDataFormModelItem \a data in the \a parent at the specified \a index.

    \a index Index where data has to be inserted in parent
    \a data HbDataFormModelItem which has to be inserted
    \a parent Parent of item

    \sa appendDataFormItem
    
*/
void HbDataFormModel::insertDataFormItem(int index, HbDataFormModelItem *data,
    HbDataFormModelItem *parent)
{
    if(!parent || (data->data(HbDataFormModelItem::ItemTypeRole) == HbDataFormModelItem::FormPageItem )) {
        parent = invisibleRootItem();
    }
    parent->insertChild(index,data);    
}


/*!
    @beta

    Removes and deletes the model item from the model at the given \a index. The visualization
    corresponding to this \a index is also deleted. Returns true if \a index is removed otherwise
    returns false.

    \sa removeItem
*/
bool HbDataFormModel::removeItem(const QModelIndex &index )
{
    return removeItem(itemFromIndex(index));
}

/*!
    @beta

    Removes and deletes given \a item. The visualization corresponding to this \a item
    is also deleted. If the \a item has childrens, all the children items are removed and deleted.
    \a item is deleted only if \a item belongs to current model. Returns true if \a item is removed 
    otherwise returns false.

    \sa removeItem
*/
bool HbDataFormModel::removeItem(HbDataFormModelItem *item)
{
    if( !item ) {
        return false;
    }
    HbDataFormModelItem* parent = const_cast<HbDataFormModelItem*>(item->parent());
    if ( HbDataFormModelItemPrivate::d_ptr(item)->model() != this ) {
               return false;
    }
    
    if( parent ) {
        int index = parent->indexOf(item);
        parent->removeChild(index);
        return true;
    } else if ( item == invisibleRootItem() ) {
        delete item;
        item = 0;
        return true;
    }

    return false;

}

/*!
    \reimp
*/
bool HbDataFormModel::removeRows(int row, int count, const QModelIndex &index)
{   
    HbDataFormModelItem *item = itemFromIndex(index);
    if ((item == 0) || (count < 1) || (row < 0) || ((row + count) > item->childCount())) {
        return false;
    }

    item->removeChildren(row, count);
    return true;
}

/*!
    \reimp
    Column value should be 0 as DataForm has only one column.If the value is not 0
    function returns invalid index.
    If index is not valid then rootItem's index is considered.
*/
QModelIndex HbDataFormModel::index(int row, int column,
    const QModelIndex &index) const
{
    
    if (!hasIndex(row, column, index) && column > 0) {
         return QModelIndex();
    }
    HbDataFormModelItem *parentItem = 0;

    if (!index.isValid()) {
        parentItem = invisibleRootItem();
    } else {
        parentItem = itemFromIndex(index);
    }

    HbDataFormModelItem *childItem = parentItem->childAt(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    } else {
        return QModelIndex();
    }
}

/*!
  \reimp
*/
QModelIndex HbDataFormModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    HbDataFormModelItem *childItem =itemFromIndex(index);        
    HbDataFormModelItem *parentItem = childItem->parent();

    if (parentItem == invisibleRootItem()) {
        return QModelIndex();
    }

       return indexFromItem(parentItem);    
}

/*!
  \reimp
*/
int HbDataFormModel::rowCount(const QModelIndex &item ) const
{
    HbDataFormModelItem *parentItem;
    if (item.column() > 0) {
        return 0;
    }

    if (!item.isValid()) {
        parentItem = invisibleRootItem();
    } else {
        parentItem = static_cast<HbDataFormModelItem*>(item.internalPointer());
    }
    return parentItem->childCount();
}

/*!
  \reimp
*/
int HbDataFormModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 1;
}

/*!
    \reimp

    Returns true if given /a index has children, other wise false
*/
bool HbDataFormModel::hasChildren(const QModelIndex &index) const
{
    HbDataFormModelItem* item = 0;
    if(index.isValid()) {
        item = static_cast<HbDataFormModelItem*>(index.internalPointer());
        if(item && item->childCount()) {
            return true;
        }
    }
    return false;
}

/*!
    @beta

    Returns child count for given parent index /a parent
*/
int HbDataFormModel::childCount(const QModelIndex &index) const
{
    return rowCount(index);
}

/*!
  \reimp
*/
QVariant HbDataFormModel::data(const QModelIndex &index, int role) const
{
    HbDataFormModelItem *item = static_cast<HbDataFormModelItem*>(index.internalPointer());
    return item ? item->data(role) : QVariant();
}

/*!
  \reimp
*/
bool HbDataFormModel::setData(const QModelIndex &index, const QVariant &value,
    int role)
{
    if (!index.isValid()) {
        return false;
    }
    HbDataFormModelItem *item = itemFromIndex(index);
    if (item == 0) {
        return false;
    }
    item->setData(role , value);   
    return true;
}

/*!
    @beta

    Removes all items  from the model and sets the
    number of rows and columns to zero.

    \sa removeColumns(), removeRows()
*/
void HbDataFormModel::clear()
{
    Q_D(HbDataFormModel);
    removeItem(invisibleRootItem());
    d->mRoot = new HbDataFormModelItem();
    HbDataFormModelItemPrivate::d_ptr(d->mRoot)->setModel(this);
    
    reset();
}

/*!
    \reimp

    Returns a pointer to the HbDataFormModelItem associated with the given \a index.

    Calling this function is typically the initial step when processing
    QModelIndex-based signals from a view, such as
    HbAbstractItemView::activated(). In your slot, you call itemFromIndex(),
    with the QModelIndex carried by the signal as argument, to obtain a
    pointer to the corresponding HbDataFormModelItem.

    NOTE: index passed should be the one returned from HbDataFromModel Only.

    If \a index is an invalid index, this function returns 0.

    \sa indexFromItem()
*/
HbDataFormModelItem* HbDataFormModel::itemFromIndex(const QModelIndex &index) const
{
    if ((index.row() < 0) || (index.column() < 0) || (index.model() != this)) {
        return 0;
    }
         
    HbDataFormModelItem *item = static_cast<HbDataFormModelItem*>(index.internalPointer());
    if (item) {
        return item;
    }   
    return 0;
}

/*!
    \reimp

    Returns the QModelIndex associated with the given \a item.
    Use this function when you want to perform an operation that requires the
    QModelIndex of the item, such as HbAbstractItemView::scrollTo(). 
   
    \sa itemFromIndex()
*/
QModelIndex HbDataFormModel::indexFromItem(const HbDataFormModelItem* item) const
{
    if(item) {          
        HbDataFormModelItem* parentItem = item->parent();
        if (parentItem) {
            return createIndex(parentItem->indexOf(item), 0, (void*)item);
        }
    }
    return QModelIndex();
}

/*!
    @beta

    Returns the HbDataFormModelItem at given \a row and with given parent /a index.
*/
HbDataFormModelItem* HbDataFormModel::item(int row, const QModelIndex &index) const
{
    if(index.isValid()) {
        return itemFromIndex(index)->childAt(row);
    } else {
        return invisibleRootItem()->childAt(row);
    }
}

/*!
    \reimp 
   
    Returns the model's invisible root item.

    The invisible root item provides access to the model's top-level items
    through the HbDataFormModelItem API, making it possible to write functions that
    can treat top-level items and their children in a uniform way.
    for example,recursive functions involving a tree model.

*/
HbDataFormModelItem* HbDataFormModel::invisibleRootItem() const
{
    Q_D(const HbDataFormModel);
    return d->mRoot;
}

/*
    \reimp
*/
bool HbDataFormModel::canFetchMore ( const QModelIndex & parent ) const
{
    if(parent.isValid()) {
        if((hasChildren(parent)) || (sibling(parent.row() + 1,0,parent)).isValid()) {
            return true;
        }
    }

    return false;
}

/*
    \reimp
*/
Qt::ItemFlags HbDataFormModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        const HbDataFormModelItem *item = itemFromIndex(index);
        if (item) {
            return item->flags();
        } 
    }
    return 0;
}


