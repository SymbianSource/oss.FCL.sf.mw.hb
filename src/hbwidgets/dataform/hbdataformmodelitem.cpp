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

#include <QAbstractItemModel>

#include "hbdataformmodelitem_p.h"
#include "hbdataformmodelitem.h"
#include "hbdataformmodel.h"
#include "hbdataformmodel_p.h"

class QAbstractItemModel;


HbDataFormModelItemPrivate::HbDataFormModelItemPrivate():
    mParentItem(0),
    mModel(0),
    mFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled)
{
    q_ptr = 0;
}

HbDataFormModelItemPrivate::~HbDataFormModelItemPrivate()
{
}

QString HbDataFormModelItemPrivate::dirtyProperty()
{
    return mDirtyProperty;
}


void HbDataFormModelItemPrivate::setModel(const QAbstractItemModel *model)
{
    Q_Q(HbDataFormModelItem);

    mModel = const_cast<QAbstractItemModel*>(model);
    int count = q->childCount();    
    for (int index = 0; index < count ; index++) {
        HbDataFormModelItem* item = mChildItems.at(index);
        HbDataFormModelItemPrivate::d_ptr(item)->setModel(model);         
    }
     
}
void HbDataFormModelItemPrivate::setContentWidgetData(
    const QString& propertyName ,const QVariant &value)
{
    mProperties.remove(propertyName);
    mProperties.insert(propertyName,value);
}
QAbstractItemModel* HbDataFormModelItemPrivate::model() const
{   
    return mModel;
}

/*!
    @beta
    @hbwidgets
    \class HbDataFormModelItem hbdataformmodelitem.cpp
    \brief The HbDataFormModelItem class is for applications to add their data.

    Data is stored in this class and it is added to model.
    HbDataFormModelItem class can be used to store data in hierarchical manner
    For Example if below is the model hierarchy for profiles application
    - general
        - ringtone_general
            - unknowncaller_ringtone_general,

    Then sample code to create above model and data it to data form would be,
    
    \code
    HbDataForm* form = new HbDataForm();
    form->setHeading(QString("Profiles"));
    form->setDescription(QString("With profiles you can define themes used in different profiles"));

    HbDataFormModel *model = new HbDataFormModel();  
    HbDataFormModelItem *general = model->appendDataFormPage(QString("General"));
    HbDataFormModelItem *ringToneGeneral =
        model->appendDataFormGroup(QString("Ring Tones"), general);
    HbDataFormModelItem *unknownCallerGeneral =
        model->appendDataFormGroupPage(QString("Unknown Caller"), ringToneGeneral);

    form->setModel(model);
    \endcode

*/

/*!
    \enum HbDataFormModelItem::Roles

    This enum defines the Roles supported by dataForm. Any data from application 
    can be added through these Roles.
 */

/*!
    \var HbDataFormModelItem::LabelRole
    LabelRole: This Role is used for data item label/heading of HbDataFormViewItem.
    
 */


/*!
    \var HbDataFormModelItem::ItemTypeRole
    ItemTypeRole: This Role is used for data item type of the HbDataFormModelItem
    
 */

/*!
    \var HbDataFormModelItem::PropertyRole
    PropertyRole: This Role is used for data properties of the contentwidget in data item.
    
 */

/*!
    \var HbDataFormModelItem::DescriptionRole
    DescriptionRole: This Role will add a description text in model item visualization. This role
    is valid only for GroupItem and data items ( > GroupPageItem ).
    
 */

/*!
    \enum HbDataFormModelItem::DataItemType

    This enum defines itemtypes supported in dataform.
    Enum also defines itemtype for custom types.
 */

/*!
    \var HbDataFormModelItem::FormPageItem

    FormPageItem: Item Type is FormPageItem. FormPage is usually the top level item supported in 
    HbDataForm.
 */

/*!
    \var HbDataFormModelItem::GroupItem
    GroupItem is used for grouping different group pages or grouping data items under one group
    heading.
 */

/*!
    \var HbDataFormModelItem::GroupPageItem
    GroupPageItem: This itemType specifies the page for different groups.
    For Ex: In Profiles application, RingTones is group and under RingTones 
    "KnownCallers","UnKnownCallers" and "Friends" can be different grouppages.
    GroupPageItem can be added only inside GroupItem.
 */

/*!
    \var HbDataFormModelItem::SliderItem
    SliderItem:  This itemType is for slider type of data item
    
 */

/*!
    \var HbDataFormModelItem::CheckBoxItem
    CheckBoxItem:  This itemType is for check box type of data item
    
 */

/*!
    \var HbDataFormModelItem::TextItem
    TextItem:  This itemType is for text type of data item
               The TextItem by default has maximum 4 rows. 
               Application can configure this value using HbLineEdit Property maxRows. 
               This Property Value has to be set using setContentWidgetData() API.                
    
 */

/*!
    \var HbDataFormModelItem::ToggleValueItem
    ToggleValueItem:  This itemType is for toggle type of data item
    For data the toggle values the button properties( text and additionalText ) can
    be used. User can connect to dataChanged signal from HbDataFormModel to get 
    notification when toggle data item value is changed. The "text" property of
    HbPushButton holds the current value of toggle data item. Below is the sample code:

    \code
    {
        HbDataForm *form = new HbDataForm();
        model = new HbDataFormModel();
        item = model->appendDataFormItem(HbDataFormModelItem::ToggleValueItem, QString("my toggle"));
        item->setContentWidgetData(QString("text"), QString("yes"));
        item->setContentWidgetData(QString("additionalText"), QString("no"));

        connect(model, SIGNAL(dataChanged(QModelIndex, QModelIndex)), 
            this, SLOT(toggleChange(QModelIndex, QModelIndex)));

        HbDataFormModelItem *toggle2 = model->appendDataFormItem(
            HbDataFormModelItem::ToggleValueItem, QString("my toggle 2"));
        toggle2->setContentWidgetData(QString("text"), QString("1"));
        toggle2->setContentWidgetData(QString("additionalText"), QString("2"));
        form->setModel(model);
        
        mainLayout->addItem(form);

    }

    void MessageBoxView::toggleChange(QModelIndex startIn, QModelIndex endIn)
    {
        HbDataFormModelItem *itm = model->itemFromIndex(startIn);
        if(itm->type() == HbDataFormModelItem::ToggleValueItem) {
            if(itm->data(HbDataFormModelItem::LabelRole).toString() == QString("my toggle")) {
                QVariant data = itm->contentWidgetData(QString("text"));
                qDebug()<<"text"<<data.toString();
                data = itm->contentWidgetData(QString("additionalText"));
                qDebug()<<"additionalText"<<data.toString();
            }
        }
    }

    \endcode
    
 */

/*!
    \var HbDataFormModelItem::RadioButtonListItem
    RadioButtonListItem:  This itemType is for radio button list type of data item
    
    RadioButtonListItem will appear in three display modes

    - Automatic : radioButtonList item appear as embedded( inline) if it contains less than four items and 
    if more than three items then selected items are displayed as text on a PushButton and when pushbutton 
    clicked it lunches popup. Automatic mode is set as the default mode.
    
    - Embedded : Application can set these items as always embedded(inline) by setting the property "displayMode" 
    with value of property as "embedded"
    
    - Popup : Application can set these items as always popup by setting the property "displayMode" 
    with value of property as "popup"
    HbDataFormModelItem *radioItem = model->appendDataItem(HbDataFormModelItem::RadioButtonListItem, 
        QString("Caller Tone"));
    radioItem->setContentWidgetData(QString("displayMode"),QString("embedded"));
    radioItem->setContentWidgetData(QString("displayMode"),QString("automatic"));
    radioItem->setContentWidgetData(QString("displayMode"),QString("popup"));
    
 */

/*!
    \var HbDataFormModelItem::MultiselectionItem
    MultiselectionItem:  This itemType is for multi selection type of data item.
    
    MultiSelectionListItem will appear in three display modes

    - Automatic : radioButtonList item appear as embedded( inline) if it contains less than four items and 
    if more than three items then selected items are displayed as text on a PushButton and when pushbutton 
    clicked it lunches popup. Automatic mode is set as the default mode.
    
    - Embedded : Application can set these items as always embedded(inline) by setting the property "displayMode" 
    with value of property as "embedded"
    
    - Popup : Application can set these items as always popup by setting the property "displayMode" 
    with value of property as "popup"
    HbDataFormModelItem *radioItem = model->appendDataItem(HbDataFormModelItem::MultiSelectionListItem, 
        QString("Caller Tone"));
    radioItem->setContentWidgetData(QString("displayMode"),QString("embedded"));
    radioItem->setContentWidgetData(QString("displayMode"),QString("automatic"));
    radioItem->setContentWidgetData(QString("displayMode"),QString("popup"));
 */

/*!
    \var HbDataFormModelItem::ComboBoxItem
    ComboBoxItem:  This itemType is for combo box type of data item
    
 */

/*!
    \var HbDataFormModelItem::CustomItemBase
    CustomItemBase:  This itemType is for CustomItemType of data item.
    Applications can use this as base value for customItems.
    
 */

/*!
    Constructs a new HbDataFormModelItem data class  with the given \a type , given \a label of the item
    and given \a parent.

    Value of \a type should be from
    
    enum DataItemType
    {
        FormPageItem,
        GroupItem,
        GroupPageItem,
        SliderItem,
        VolumeSliderItem,
        CheckBoxItem,
        TextItem,
        ToggleValueItem,
        RadioButtonListItem,
        MultiselectionItem,
        ComboBoxItem,
        CustomItemBase = FormPageItem + 100
    };
    
*/

HbDataFormModelItem::HbDataFormModelItem(
    HbDataFormModelItem::DataItemType type,const QString &label,
    const HbDataFormModelItem* parent):
    d_ptr(new HbDataFormModelItemPrivate())
{
    Q_D(HbDataFormModelItem);
    d->q_ptr = this ;
    d->mParentItem = const_cast<HbDataFormModelItem*>(parent);
    setData(ItemTypeRole, type);
    setData(LabelRole, label);
}

/*!
    Constructs a new HbDataFormModelItem class  with the given \a parent.
*/
HbDataFormModelItem::HbDataFormModelItem(const HbDataFormModelItem* parent):
    d_ptr(new HbDataFormModelItemPrivate())
{
    Q_D(HbDataFormModelItem);
    d->q_ptr = this ;
    d->mParentItem = const_cast<HbDataFormModelItem*>(parent);
}

/*!
    Destructor
*/
HbDataFormModelItem::~HbDataFormModelItem()
{
    Q_D(HbDataFormModelItem);
    int childcount = childCount();
    for (int index = 0; index < childcount; index++) {
        HbDataFormModelItem* item = childAt(0);
        d->mChildItems.removeAt(0);
        delete item;
    }
    d->mProperties.clear();
    d->mItemData.clear();
    delete d_ptr;
}

/*!
    @beta

    Adds the given \a child to the children list of current item.

    \sa insertChild, insertChildren
*/
void HbDataFormModelItem::appendChild(HbDataFormModelItem *child)
{
    Q_D(HbDataFormModelItem);    

    if(child){
        child->setParent(this);
        if(d->mModel) {
            HbDataFormModelItemPrivate::d_ptr(child)->setModel(d->mModel);
            HbDataFormModel* model = static_cast<HbDataFormModel*>(d->mModel);
            model->d_func()->rowsAboutToBeInserted(this, d->mChildItems.count(), d->mChildItems.count());
            d->mChildItems.append(child);
            model->d_func()->rowsInserted();
        }
        else {
            d->mChildItems.append(child);
        }
    }
}

/*!
    @beta
    Adds the given \a child to the children list of current item at the given \a index.

    \sa insertChildren, appendChild
*/
void HbDataFormModelItem::insertChild(int index, HbDataFormModelItem *child)
{
    Q_D(HbDataFormModelItem);

    if(child) {
        child->setParent(this);
        if(d->mModel) {            
            HbDataFormModelItemPrivate::d_ptr(child)->setModel(d->mModel);
            HbDataFormModel* model = static_cast<HbDataFormModel*>(d->mModel);
            model->d_func()->rowsAboutToBeInserted(this, index, index);
            d->mChildItems.insert(index,child);
            model->d_func()->rowsInserted();
        } else {
            d->mChildItems.insert(index,child);
        }
    }
}

/*!
    @beta

    Inserts the given list of \a items starting from the given \a row.

    \sa insertChild, appendChild
*/
void HbDataFormModelItem::insertChildren(int row , int count ,
        QList<HbDataFormModelItem*> items)
{
    Q_D(HbDataFormModelItem);
    HbDataFormModel* model = static_cast<HbDataFormModel*>(d->mModel);

    if(model) {
        model->d_func()->rowsAboutToBeInserted(this, row, row + count - 1);
        for(int index = 0; index < count; index++) {
        d->mChildItems.insert(row + index, items.at(index));
        }
        model->d_func()->rowsInserted();
    }
    else {
        for(int index = 0; index < count; index++) {
        d->mChildItems.insert(row + index, items.at(index));
        }
    }

}

/*!
    @beta

    Removes the child item at the given \a index. The item at \a index is
    deleted.

    \sa removeChildren
*/
void HbDataFormModelItem::removeChild(int index)
{
    if( ( index < 0 ) || ( index >= childCount() ) ) {
        return;
    }
    Q_D(HbDataFormModelItem);
    HbDataFormModel* model = static_cast<HbDataFormModel*>(d->mModel);

    if(model)
        model->d_func()->rowsAboutToBeRemoved(this, index, index);

        HbDataFormModelItem *item = d->mChildItems.at(index);
        if( item ) {
            int childCount = item->childCount();
            for ( int childIndex = 0; childIndex <= childCount ;childIndex++) {
                item->removeChild(0); 
            }

            HbDataFormModelItem *item = d->mChildItems.takeAt(index);
            delete item;
            item = 0;
        }        
        
    if(model)
        model->d_func()->rowsRemoved();
     
}

/*!
    @beta

    Removes the given no of \a count of childitems from the given \a startindex. The
    items are deleted.

    \sa removeChild
*/
void HbDataFormModelItem::removeChildren(int startIndex, int count)
{
    if( ( startIndex < 0 ) || ( startIndex > childCount() ) || ( count <= 0 )) {
        return;
    }
    
    if( startIndex + count > childCount() ) {
        return;
    }

    Q_D(HbDataFormModelItem);

    HbDataFormModel* model = static_cast<HbDataFormModel*>(d->mModel);
    if( model ) {
        model->d_func()->rowsAboutToBeRemoved(this, startIndex, startIndex + count -1); 
    }

    for(int index = 0; index < count ;index++) {
        HbDataFormModelItem *item = d->mChildItems.takeAt(0);
        if ( item ) {
            delete item;
            item = 0;
        }
    }

    if( model ) {
        model->d_func()->rowsRemoved();
    }
}

/*!
    @beta

    Returns the child item at the given \a index. 
    Returns 0 if \a index passed in greater than count or less than 0.

    \sa indexOf
*/
HbDataFormModelItem* HbDataFormModelItem::childAt(int index) const
{
    Q_D( const HbDataFormModelItem);
    if(d->mChildItems.count() > index && index > -1) {
        return d->mChildItems.at(index);
    }
    return 0;
}

/*!
    @beta

    Returns index of the given \a child.

    \sa childAt
*/
int HbDataFormModelItem::indexOf(const HbDataFormModelItem* child) const 
{
    Q_D(const HbDataFormModelItem);
    return d->mChildItems.indexOf(const_cast<HbDataFormModelItem*>(child));
}

/*!
    @beta

    Returns the number of child items.
*/
int HbDataFormModelItem::childCount() const
{
    Q_D(const HbDataFormModelItem);
    return d->mChildItems.count();
}

/*!
    @beta

    Returns the data for the given \a role. Returns empty string if DescriptionRole is queried for
    items other then GroupItem and data item.
*/
QVariant HbDataFormModelItem::data(int role ) const
{
    Q_D(const HbDataFormModelItem);
    if (role == LabelRole) {
        return d->mLabel;
    } else if (role == ItemTypeRole) {
        return d->mItemType;
    } else if (role == Qt::DecorationRole) {
        return d->mIcon;
    } else if (role == PropertyRole) {
        return d->mProperties;
    } else if (role == DescriptionRole) {
        //Description role is valid only for GroupItem and data items
        if (( d->mItemType == HbDataFormModelItem::GroupItem ) 
            || ( d->mItemType > HbDataFormModelItem::GroupPageItem )) {
            return d->mDescription;
        } else {
            return QString();
        }
    } else {
         return d->mItemData.value(role);
    }
}

/*!
    @beta

    Sets the given \a value of variant to the given \a role.
*/
 void HbDataFormModelItem::setData(int role ,const QVariant &value)
{
    Q_D(HbDataFormModelItem);
   if (role == LabelRole) {
        d->mLabel = value.toString();
        d->mDirtyProperty = "LabelRole";
    } else if (role == ItemTypeRole) {
        d->mItemType = (DataItemType)value.toInt();   
        d->mDirtyProperty = "ItemTypeRole";
    } else if (role == Qt::DecorationRole) {
        d->mIcon = value.toString();
        d->mDirtyProperty = "DecorationRole";
    } else if(role == PropertyRole){
        d->mProperties = value.toHash();
        d->mDirtyProperty = "PropertyRole";
    } else if( role == DescriptionRole ) {
        //Description role is valid only for GroupItem and data items
        if (( d->mItemType == HbDataFormModelItem::GroupItem ) 
            || ( d->mItemType > HbDataFormModelItem::GroupPageItem )) {
            d->mDescription = value.toString();
            d->mDirtyProperty = "DescriptionRole";
        }
    } else {
        d->mItemData.insert(role,value);       
    }

    HbDataFormModel* data_model = static_cast<HbDataFormModel*>(d->mModel);
    if(data_model){
        QModelIndex index = data_model->indexFromItem(this);
        emit data_model->dataChanged(index, index);
    }
}

/*!
    @beta

    Sets the content widget property values.
    Sets the given \a value to given \a propertyName. Below is the widget whose property 
    should be used depending upon the type of data item.

    - SliderItem: HbSlider porperty should be used
    - VolumeSliderItem: HbVolumeSlider property should be used
    - CheckBoxItem: HbCheckBox property should be used
    - TextItem: HbLineEdit property should be used
    - ToggleValueItem: HbPushButton(text and additionalText) property should be used
    - RadioButtonListItem: HbRadioButtonList property should be used
    - MultiselectionItem: HbListDialog property should be used
    - ComboBoxItem: HbComboBox property should be used
*/
void HbDataFormModelItem::setContentWidgetData(
    const QString& propertyName ,const QVariant &value)
{
    Q_D(HbDataFormModelItem);
    d->setContentWidgetData(propertyName, value);

    d->mDirtyProperty = propertyName;
    HbDataFormModel *data_model = static_cast<HbDataFormModel*>(d->mModel);
    if(data_model) {
        QModelIndex index = data_model->indexFromItem(this);
        emit data_model->dataChanged(index, index);
    }
}

/*!
   @beta

   Returns the property  \a value for the given \a propertyName.
*/
QVariant HbDataFormModelItem::contentWidgetData(const QString& propertyName ) const
{
    Q_D(const HbDataFormModelItem);
    return d->mProperties.value(propertyName);
}

/*!
   @beta

   Returns all properties with values which was set in HbDataFormModelItem.
*/
QHash<QString, QVariant> HbDataFormModelItem::contentWidgetData() const
{
    Q_D(const HbDataFormModelItem);
    return d->mProperties;
}

/*!
   @beta

   Sets \a parent as a parent to this item.
   It only sets the parent pointer. It doesnt put the item in the 
   hierarchy.
*/
void HbDataFormModelItem::setParent(HbDataFormModelItem* parent)
{
    Q_D(HbDataFormModelItem);
    d->mParentItem=parent;
}

/*!
   @beta

   Returns the parent of the this data item.
*/
HbDataFormModelItem* HbDataFormModelItem::parent() const
{
    Q_D(const HbDataFormModelItem);
    return d->mParentItem;
}

/*!
    @beta

    Sets \a type as a DataItemType for this data item.
*/
void HbDataFormModelItem::setType(HbDataFormModelItem::DataItemType type)
{
    setData(ItemTypeRole,type);
}

/*!
    @beta

    Returns the DataItemType of the this item.
*/
HbDataFormModelItem::DataItemType HbDataFormModelItem::type() const
{
    return static_cast<HbDataFormModelItem::DataItemType>(data(ItemTypeRole).toInt());
}

/*!
    @beta

    Sets the \a label to the item. This is valid only if the type is other than FormPageItem,
    GroupItem and GroupPageItem.
*/
void HbDataFormModelItem::setLabel(const QString& label)
{
    setData(LabelRole,label);
}

/*!
    @beta

    Returns the label of the item.
*/
QString HbDataFormModelItem::label() const
{
    return data(LabelRole).toString();
}

/*!
    @beta

    Sets the \a icon to the item. This is valid only if the type is other than FormPageItem,
    GroupItem and GroupPageItem.
*/
void HbDataFormModelItem::setIcon(const QString& icon)
{
    setData(Qt::DecorationRole,icon);
}

/*!
    @beta

    Returns the icon of the item.
*/
QString HbDataFormModelItem::icon() const
{
    return data(Qt::DecorationRole).toString();
}

/*!
    @beta

    Sets whether the item is enabled. 
    
    If enabled is true, the item is \a enabled, meaning that the user can interact with the item
    if \a enabled is false, the user cannot interact with the item.
*/
void HbDataFormModelItem::setEnabled(bool enabled)
{
    Q_D(HbDataFormModelItem);

    if (enabled != (bool)(d->mFlags & Qt::ItemIsEnabled)) {
        if (enabled) {
            d->mFlags |= Qt::ItemIsEnabled;
        } else {
            d->mFlags &= ~Qt::ItemIsEnabled;
        }
        HbDataFormModel* data_model = static_cast<HbDataFormModel*>(d->mModel);
        if(data_model){
            QModelIndex index = data_model->indexFromItem(this);
            emit data_model->dataChanged(index, index);
        }
    }
}

/*!
    @beta
    Returns true if the item is enabled otherwise returns false.
*/
bool HbDataFormModelItem::isEnabled() const
{
    Q_D(const HbDataFormModelItem);
    return d->mFlags & Qt::ItemIsEnabled;
}

/*!
    @beta
    Returns item flags for this item.
*/
Qt::ItemFlags HbDataFormModelItem::flags() const
{
    Q_D(const HbDataFormModelItem);
    return d->mFlags;
}

/*!
    @beta
    Sets the \a description to the item. This is valid only if the type is GroupItem or 
        DataItem. Its not valid for GroupPageItem and FormPageItem.
*/
void HbDataFormModelItem::setDescription(const QString& description)
{
    setData(DescriptionRole , description);
}

/*!
    @beta
    Returns the description of the item.
*/
QString HbDataFormModelItem::description() const
{
    return data(DescriptionRole).toString();
}

