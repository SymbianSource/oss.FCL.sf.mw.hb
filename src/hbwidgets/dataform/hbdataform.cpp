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

#include <hbdataform.h>
#include <hbdataformmodelitem.h>
#include <hbdataformmodel.h>
#include "hbdataform_p.h"
#include "hbdataformviewitem_p.h"
#include "hbdataitemcontainer_p.h"
#include "hbdatagroup_p.h"
#include "hbdatagroup_p_p.h"
#include <hbcombobox.h>
#include "hbdataformheadingwidget_p.h"
#include "hbtreemodeliterator_p.h"

#include <QGraphicsSceneMouseEvent>
#include <QCoreApplication>

/*!
    @beta
    @hbwidgets
    \class HbDataForm
    \brief HbDataForm represents hierarchical dataitems in the form of groups,pages and 
    items.
    The HbDataForm class provides a default view implementation of dataform.
    A HbDataForm implements a hierarchical representation of data items from a model.

    HbDataForm implements the interfaces defined by the HbAbstractItemView class to allow 
    it to display data provided  by models derived from the QAbstractItemModel class.

    It is simple to construct a dataform displaying data from a model. The user has to create
    HbDataFormModel and create the hierarchy of HbDataFormModelItems .The hierarchy is 
    similar to the following.
    
    - HbDataForm
       - HbDataFormPage1
         - HbDataGroup1
           - HbDataGroupPage1
             - HbDataItem
             - HbDataItem
             - HbDataItem
             - HbDataItem
       - HbDataFormPage2
         - HbDataGroup2
           - HbDataGroupPage2
             - HbDataItem
             - HbDataItem
             - HbDataItem
             - HbDataItem
 
    HbDataItem can be the child of HbDataForm, HbDataFormPage,HbDataGroup and 
    HbDataGroupPage. An instance of HbDataForm has to be created and model should be set 
    to the form using setModel(HbDataFormModel) API.
    The properties of each DataItem node can be set using HbDataFormModelItem convenient
    API's. These data are parsed while the visualization instance of each item is created and 
    set on each item.

    The model/view architecture ensures that the contents of the data view are updated as the 
    model changes.

    Items that have children can be in expanded (children are visible) or collapsed 
    (children are hidden) state. DataItems of type HbDataFormPage, HbDataGroup and 
    HbDataGroupPage can be expanded and collapsed. HbDataItem of type FormPageItem, 
    GroupItem, GroupPageItem can only have children. Each item in model is represented by an 
    instance of HbDataFormViewItem. HbDataForm uses HbDataFormViewItem prototype to instantiate 
    the HbDataForm items. HbDataFormViewItem can be subclassed for customization purposes.
    
    The Model hierarchy can be created using the convenient API's provided on model class like
    appendDataFormPage , appendDataFormGroup ,appendDataFormGroupPage and 
    appendDataFormItem. All of these will return HbDataFormModelItem instance correspoding 
    to each type on which user can set item specific data. Otherwise each HbDataFormModelItem can 
    be created individually by passing the corresponding type of item (GroupItem, GroupPageItem, 
    FormPageItem) and create the tree of HbDataFormModelItem using setParent API 
    or by passing the parent HbDataFormModelItem in constructor. Later the top level 
    HbDataFormModelItem can be added inside the model.

    After doing the setModel, the visualization gets created . Only the items inside the expanded 
    group or group page instances are created. When an item's visualization is created , 
    DataForm emits activated(constQModelIndex&) signal. The application can get 
    HbDataFormViewItem and content widget from DataForm using QModelIndex.    
    
    The signal emitted by HbDataForm
    \li activated(const QModelIndex &index) Emitted when the HbDataFormViewItem corresponding to
    \a index is shown. User can connect to this signal and can fetch the instance of 
    HbDataFormViewItem from HbDataForm using the API dataFormViewItem(const QModelIndex &index).
    \li dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight) emitted when the 
    HbDataFormModel is updated \atopLeft and \abottomRight will be same since every node has only one column.
    User can connect to this signal and can fetch the instance of HbDataFormViewItem from HbDataForm 
    using the API dataFormViewItem(const QModelIndex &index) or user can fetch HbDataFormModelItem using API 
    itemFromIndex(const QModelIndex &index) in HbDataFormModel .When user updates model using 
    setContentWidgetData API provided in HbDataFormModelItem class, then DataForm takes care of updating the 
    corresponding item's visualization.

    The user can also provide connection information to correspoding content widget of each HbDataFormModelItem
    using API addConnection(HbDataFormModelItem* item, const char* signal, QObject* receiver, consta char* slot)
    provided in HbDataForm class.The connection will be established when the item visualization is created .
    similar way removeConnection(HbDataFormModelItem *item, const char* signal, QObject *receiver, const char* slot)
    and removeAllConnection() API can be used. Connection can be established or removed even at runtime also.
    
    An example of how to create HbDataForm:
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,31}

    The output generated by the above code looks like:

    \image html hbsettingform.png

    This is how HbDataForm will look like in landscape mode:

    \image html hbsettingform_landscape.png

    \sa HbDataFormViewItem, HbDataFormModel, HbDataFormModelItem

    Creating Custom Item:
    Application developer can create custom DataItem by deriving from HbDataFormViewItem and setting this as 
    prototype(setItemProtoType() API).
    Application has to override virtual API's createCustomWidget() , restore()and save(). 
    createCustomWidget() API should return the corresponding custom HbWidget which can also be a compound widget.
    Signal connection for child widgets inside the compound widget should taken care by the application. 
    restore() API will be called by the framework when the model data is changed.
    So restore() should take care of updating the visual items with correspoding data from model. 
    save() API should update the model.App developer should  connect respective widgets SIGNALs to SLOT save() 
    and update the data to model .

*/

/*!
    \fn void HbAbstractItemView::activated(const QModelIndex &index)

    This signal is emitted when HbDataFormViewItem corresponding to \a index is shown.

*/

/*!
    Constructs DataForm with given \a parent.
    \param parent parent .
 */
HbDataForm::HbDataForm(QGraphicsItem *parent)
    : HbAbstractItemView(*new HbDataFormPrivate(), new HbDataItemContainer(),
                         new HbTreeModelIterator(0, QModelIndex(), false), parent)
{
    Q_D( HbDataForm );
    d->q_ptr = this;
    d->init();
    setVerticalScrollBarPolicy(ScrollBarAlwaysOff);
    //d->mHeadingWidget->createPrimitives();
    //static_cast<HbDataItemContainer*>(container())->setFormHeading(d->mHeadingWidget);
}

/*!
    Constructs a data form with a private class object \a dd, 
    \a container and \a parent.
*/
HbDataForm::HbDataForm(HbDataFormPrivate &dd, HbAbstractItemContainer *container,
                       QGraphicsItem * parent)
        : HbAbstractItemView(dd, container, new HbTreeModelIterator(0, QModelIndex(), false), parent)
{
    Q_D( HbDataForm );
    d->q_ptr = this;
    d->init();
}

/*!
    Destructs the data form.
*/
HbDataForm::~HbDataForm()
{
}

/*!
    \reimp

    Scrolls the view so that the item represented by \a index comes at the middle of 
    screen. By default HbDataForm does not scrolls. Application developer is supposed to 
    call this API if he wants this behaviour. User can connect to activated signal and then
    can call this API.
*/
void HbDataForm::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    //Q_D(HbDataForm);
    //d->revealItem(d->mContainer->itemByIndex(index), PositionAtCenter);
    HbAbstractItemView::scrollTo(index, hint);
}

/*!
    \deprecated HbDataForm::indexCount() const
        is deprecated. Use \a HbModelIterator::indexCount() const

    \reimp
    Returns the number of children visible in the hierarchy. Children of collapsed parents are
    not taken into account.
*/
int HbDataForm::indexCount() const
{
    qWarning("HbDataForm::indexCount() is deprecated! Use HbModelIterator::indexCount() const.");

    Q_D(const HbDataForm);
    int bufferSize = 0;
    if (model()) {
        QModelIndex root = rootIndex();
        int rowCount = d->treeModelIterator()->childCount(root);
        for (int row = 0; row < rowCount; ++row) {
            bufferSize += d->childCount(d->treeModelIterator()->child(row, root));
        }
        bufferSize += rowCount;
    }
    return bufferSize;
}

/*!
    \deprecated HbDataForm::nextIndex(const QModelIndex&) const
        is deprecated. Use \a HbModelIterator::nextIndex(const QModelIndex&) const

    \reimp

    Next index for valid index is determined in following way:

    - If index has children and it is expanded then first child is returned
    - Otherwise if index has next sibling then that is returned
    - Otherwise next valid sibling for parent is returned
    - Otherwise QModelIndex is returned
*/
QModelIndex HbDataForm::nextIndex(const QModelIndex &index) const
{
    qWarning("HbDataForm::nextIndex(const QModelIndex&) const is deprecated! Use HbModelIterator::nextIndex(const QModelIndex&).");

    Q_D(const HbDataForm);
    if (index.isValid() && index.column() == 0) {
        QModelIndex result;
        if (isExpanded(index)) {
            result = index.child(0, 0);
        }

        if (!result.isValid()) {
            result = index.sibling(index.row() + 1, 0);
        }

        if (!result.isValid()) {
            QModelIndex parentIndex(index.parent());
            while (!result.isValid()) {
                if (parentIndex == rootIndex()) {
                    break;
                }
                result = parentIndex.sibling(parentIndex.row() + 1, 0);
                parentIndex = parentIndex.parent();
            }
        }
        return result;
    } else {
        return d->treeModelIterator()->child(0, rootIndex());
    }
}


/*!
    \deprecated HbDataForm::previousIndex(const QModelIndex&) const
        is deprecated. Use \a HbTreeModelIterator::previousIndex(const QModelIndex&) const

    \reimp

    Previous index for valid index is determined in following way:

    - If index has previous sibling last child from it is returned
    - Otherwise previous sibling is returned
    - Otherwise parent index is returned
    - Otherwise QModelIndex is returned
*/
QModelIndex HbDataForm::previousIndex(const QModelIndex &index) const
{
    qWarning("HbDataForm::previousIndex(const QModelIndex&) const is deprecated! Use HbModelIterator::previousIndex(const QModelIndex&) const.");

    Q_D(const HbDataForm);
    if (index.isValid() && index.column() == 0) {
        QModelIndex result(index.sibling(index.row() - 1, 0));
        if (result.isValid()) {
            bool checkChild = true;
            while (checkChild) {
                if (isExpanded(result)) {
                    result = result.child(model()->rowCount(result) - 1, 0);
                } else {
                    checkChild = false;
                }
            }
        }
        if (!result.isValid()) {
            result = index.parent();
            if (result == rootIndex()) {
                result = QModelIndex();
            }
        }
        return result;
    } else {
        QModelIndex result(
                d->treeModelIterator()->child(
                        d->treeModelIterator()->childCount(rootIndex())-1,
                        rootIndex()));
        QModelIndex childIndex;
        bool checkChild = true;
        while (checkChild) {
            if (!isExpanded(result)) {
                checkChild = false;
            } else {
                childIndex = d->treeModelIterator()->child(d->treeModelIterator()->childCount(result) - 1, result);
                if (childIndex.isValid()) {
                    result = childIndex;
                }
            }
        }
        return result;
    }
}

/*!
    @beta

    Sets the item referred to by \a index to either collapse or expanded, 
    depending on the value of \a expanded. If \a expanded is true then child item are 
    supposed to be visible.

    \sa isExpanded
*/
void HbDataForm::setExpanded(const QModelIndex &index, bool expanded)
{
    Q_D(HbDataForm);

    if (isExpanded(index) != expanded) {
        d->treeModelIterator()->itemStateChanged(index, HbDataFormViewItem::ExpansionKey);

        HbDataFormViewItem *item =
            static_cast<HbDataFormViewItem *>(d->mContainer->itemByIndex(index));
        if (item) {
            item->setExpanded(expanded);
        }

        d->mContainer->setItemStateValue(index, HbDataFormViewItem::ExpansionKey, expanded);
        d->mContainer->setModelIndexes();
    }
}

/*!
    @beta

    Returns true if the model item \a index is expanded otherwise returns false.

    \sa setExpanded
*/
bool HbDataForm::isExpanded(const QModelIndex &index) const
{
    Q_D(const HbDataForm);
    QVariant flags = d->mContainer->itemState(index).value(HbDataFormViewItem::ExpansionKey);
    if (flags.isValid() && flags.toBool() == true) {
        return true;
    } else {
        return false;
    }
}

/*!
    @beta

    Sets the heading of HbDataForm with the \a heading provided. Heading is displayed on 
    top of the HbDataForm. Heading is non-focusable.

    \sa heading
*/
void HbDataForm::setHeading(const QString &heading)
{
    Q_D(HbDataForm);

    if(heading.isEmpty() && d->mHeadingWidget) {
        if(!d->mHeadingWidget->mPageCombo && d->mHeadingWidget->mDescription.isEmpty()) {
            // delete the FormheadingWidget
            delete d->mHeadingWidget;
            d->mHeadingWidget = 0;
            // Remove FormheadingWidget from container layout
            HbStyle::setItemName(d->mHeadingWidget,"NULL");
            return;
        }
    }

    if(!d->mHeadingWidget) {
        d->mHeadingWidget = new HbDataFormHeadingWidget();
        HbStyle::setItemName(d->mHeadingWidget,"this");
    }
    d->mHeadingWidget->mHeading = heading;
    d->mHeadingWidget->createPrimitives();

    if(d->mHeadingWidget->mPageCombo || !d->mHeadingWidget->mDescription.isEmpty() || 
        !d->mHeadingWidget->mHeading.isEmpty()) {
        static_cast<HbDataItemContainer*>(container())->setFormHeading(d->mHeadingWidget);
    }
    d->mHeadingWidget->callPolish();
}

/*!
    @beta

    Returns heading of HbDataForm.

    \sa setHeading    
*/
QString HbDataForm::heading() const
{
    Q_D(const HbDataForm);
    if(d->mHeadingWidget) {
        return d->mHeadingWidget->mHeading;
    }

    return QString();
}

/*!
    @beta

    Sets the description of HbDataForm with the \a description. Description is displayed 
    below heading. Description is non-focusable.

    \sa description
*/
void HbDataForm::setDescription(const QString &description)
{
    Q_D(HbDataForm);

    if(description.isEmpty() && d->mHeadingWidget) {
        if(!d->mHeadingWidget->mPageCombo && d->mHeadingWidget->mHeading.isEmpty()) {
            // delete the FormheadingWidget
            delete d->mHeadingWidget;
            d->mHeadingWidget = 0;
            // Remove FormheadingWidget from container layout
            HbStyle::setItemName(d->mHeadingWidget,"NULL");
            return;
        }
    }

    if(!d->mHeadingWidget) {
        d->mHeadingWidget = new HbDataFormHeadingWidget();
        HbStyle::setItemName(d->mHeadingWidget,"this");
    }
    d->mHeadingWidget->mDescription = description;
    d->mHeadingWidget->createPrimitives();

    if(d->mHeadingWidget->mPageCombo || !d->mHeadingWidget->mDescription.isEmpty() || 
        !d->mHeadingWidget->mHeading.isEmpty()) {
        static_cast<HbDataItemContainer*>(container())->setFormHeading(d->mHeadingWidget);
    }
    d->mHeadingWidget->callPolish();
}

/*!
    @beta

    Returns description of HbDataForm.

    \sa setDescription
*/
QString HbDataForm::description() const
{
    Q_D(const HbDataForm);
    if(d->mHeadingWidget) {
        return d->mHeadingWidget->mDescription;
    }
    return QString();
}

/*!
    \reimp

    Returns the style primitive of HbDataForm depending upon the type \a primitive.
    If primitive passed is P_DataForm_background then NULL is returned.
    User cannot customize background of data form.

    \sa primitive
*/
QGraphicsItem* HbDataForm::primitive(HbStyle::Primitive primitive) const
{
    Q_D(const HbDataForm);

    switch (primitive) {
        case HbStyle::P_DataForm_heading:
            return d->mHeadingWidget->mHeadingItem;
        case HbStyle::P_DataForm_description:
            return d->mHeadingWidget->mDescriptionItem;
        default:
            return 0;
    }
}

/*!
    
    \deprecated HbDataForm::dataFormViewItem(const QModelIndex &index) const
        is deprecated. Please use HbAbstractItemView::itemByIndex instead.

    Returns HbDataFormViewItem for the correspoding \a index passed. Returns
    NULL is index passed is invalid. If \a index passed is not visible then NULL is returned.
    Ideally user should call this API when activate is called for \a index.    

*/
HbDataFormViewItem* HbDataForm::dataFormViewItem(const QModelIndex &index) const
{
    Q_D(const HbDataForm);
    if(index.isValid()) {
        return static_cast<HbDataFormViewItem*>(d->mContainer->itemByIndex(index));
    } else {
        return 0;
    }
}


/*!
    \reimp

    If \a model passed is NULL then all values of data form are reset. Calls the
    setModel of base class. This API does not clears the heading and description set
    for HbDataForm. If with new \a model user does not wants heading and description
    then he should call setHeading and setDescription with empty string.

    \sa setHeading, setDescription
*/
void HbDataForm::setModel(QAbstractItemModel *model, HbAbstractViewItem *prototype)
{
    Q_D(HbDataForm);
    if(d->mHeadingWidget) {
        if(model) {
            d->mHeadingWidget->mActivePage = -1;
        }
        if(d->mHeadingWidget->mPageCombo) {
            delete d->mHeadingWidget->mPageCombo;
            d->mHeadingWidget->mPageCombo = 0;
        }
    }
    HbAbstractItemView::setModel(model, prototype);
}

/*!
    \deprecated HbDataForm::loadSettings() 
        is deprecated. Please use HbDataFormViewItem::restore API instead..
                
    Updates the values on each HbDataItem's stored in central repository.
    This function will be invoked when DataForm is shown .

    \sa storeSettings 
*/
void HbDataForm::loadSettings()
{
    //Q_D(HbDataForm);

    /*QList<HbAbstractViewItem *> items = d->mContainer->items();
    foreach(HbAbstractViewItem* item, items) {
        static_cast<HbDataFormViewItem*>(item)->load();
    }*/
}

/*!
     \deprecated HbDataForm::storeSettings() 
        is deprecated. Please use HbDataFormViewItem::save instead.

    Stores the values of each HbDataItem's in to the repository. This function invoked 
    when DataForm is exited.

    \sa loadSettings 
*/
void HbDataForm::storeSettings()
{
    Q_D(HbDataForm);

    QList<HbAbstractViewItem *> items = d->mContainer->items();
    foreach(HbAbstractViewItem* item, items) {
        static_cast<HbDataFormViewItem*>(item)->store();
    }

}
/*!
    \reimp
*/

void HbDataForm::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_UNUSED(bottomRight);
    if(topLeft.isValid()) {       
            HbDataFormViewItem* item = static_cast<HbDataFormViewItem*>(dataFormViewItem(topLeft));
            if(item){
                item->load();
                HbDataFormModelItem *modelItem = 
                        static_cast<HbDataFormModel *>(model())->itemFromIndex(topLeft);
                HbDataFormViewItemPrivate::d_ptr(item)->setEnabled( modelItem->isEnabled() );          
            }
    }
}
/*!
    \reimp

    Initializes \a option with the values from HbDataForm.
*/
void HbDataForm::initStyleOption(HbStyleOptionDataForm *option)
{
    Q_D(HbDataForm);
    d->mHeadingWidget->initStyleOption(option);
}

/*!
    \reimp
*/
void HbDataForm::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(HbDataForm);
    HbDataFormViewItem *hitItem = qobject_cast<HbDataFormViewItem*>(d->itemAt(event->scenePos()));

    if ( d->mHitItem
        && d->mHitItem == hitItem 
        && !d->mWasScrolling ) {
            hitItem->setExpanded(!hitItem->isExpanded());
            d->mInstantClickedModifiers |= Hb::ModifierExpandedItem;
    }
    HbAbstractItemView::mouseReleaseEvent( event );
}

/*!
    \reimp
*/
void HbDataForm::rowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_D(HbDataForm);
    HbAbstractItemView::rowsInserted(parent, start, end);
    HbDataFormModelItem::DataItemType itemType = static_cast<HbDataFormModelItem::DataItemType>(
        parent.data(HbDataFormModelItem::ItemTypeRole).toInt());
    if(itemType == HbDataFormModelItem::GroupItem ||
        itemType == HbDataFormModelItem::GroupPageItem ||
        itemType == HbDataFormModelItem::FormPageItem ) {
        HbDataGroup *item = static_cast<HbDataGroup*>(itemByIndex(parent));
        if((item && item->isExpanded()) || parent == d->mModelIterator->rootIndex()) {
            container()->setModelIndexes(parent);
        }
        
    }else {
        container()->setModelIndexes();
    }
}

/*!
    \reimp
*/
void HbDataForm::rowsAboutToBeRemoved(const QModelIndex &index, int start, int end)
{
   Q_D(HbDataForm);

     for(int i = start; i <= end; i++) {

        QModelIndex childIndex = model()->index(i,0,index);
        // HbDataFormViewItem* view_Item =  static_cast<HbDataFormViewItem*>(itemByIndex(childIndex));
        QModelIndex siblingIndex = model()->index(i+1,0,index);
        HbDataFormModelItem::DataItemType itemType = static_cast<HbDataFormModelItem::DataItemType>(
        childIndex.data(HbDataFormModelItem::ItemTypeRole).toInt());
        QString label = childIndex.data(HbDataFormModelItem::LabelRole).toString();        

        if(itemType == HbDataFormModelItem::FormPageItem) {
            d->removeFormPage(label);            
        }

        if(itemType == HbDataFormModelItem::GroupPageItem) {
            QModelIndex group = childIndex.parent();
            HbDataGroup* groupItem = static_cast<HbDataGroup*>(itemByIndex(group));
            if(groupItem) {
                HbDataGroupPrivate::d_ptr(groupItem)->removeGroupPage(label);                
            }           
        }        
     }
}

/*!
    @alpha 

    This API can be used to connect with the signal of HbDataFormViewItem's content widget.
    For example: If HbDataFormModelItem is of type DataItemType::SliderItem then user
    can connect to the signals of slider using this API.
    Example Usage:
    \code
    HbDataForm *form = new HbDataForm();
    HbDataFormModel *model = new HbDataFormModel();
    HbDataFormModelItem *sliderItem = model->appendDataFormItem(HbDataFormModelItem::SliderItem);
    form->addConnection(sliderItem, SIGNAL(sliderReleased()), 
        this, SLOT(volumeChanged()));
    \endcode

    \param item Instance of model item 
    \param signal Signal of content widget. 
    \param receiver Instance of object whose slot will be called 
    \param slot Slot of \a receiver which will get called when signal is emitted
 
    \sa removeConnection 
*/
void HbDataForm::addConnection(HbDataFormModelItem * item, 
                               const char* signal, 
                               QObject *receiver, 
                               const char* slot)
{   
    Q_D(HbDataForm);
    ItemSignal itemSignal;
    itemSignal.reciever = receiver;
    itemSignal.signal = signal;
    itemSignal.slot = slot;
    d->mConnectionList.insertMulti(item, itemSignal);
    d->connectNow(item, signal, receiver, slot);
}

/*!
    @alpha

    This API can be used to remove the signal connection which was established using the
    addConnection API.

    \sa addConnection 
*/
void HbDataForm::removeConnection(HbDataFormModelItem * item, 
                                  const char* signal, 
                                  QObject *receiver, 
                                  const char* slot)
{   
    Q_D(HbDataForm);
    d->removeConnection(item, signal, receiver, slot);
}

/*!
    @alpha

    Removes the connection of all the contentwidget of all the items which has been established .
    The connection information stored inside DataForm also cleared.

    \sa removeAllConnection 
*/
void HbDataForm::removeAllConnection()
{   
    Q_D(HbDataForm);
    d->removeAllConnection();
}

/*!
    @alpha

    Removes the all connections of the contentwidget of HbDataFormModelItem's corresponding 
    visual Item ( HbdataFormViewItem).The connection information of correspoding 
    HbDataFormModelItem stored inside DataForm also cleared.

    \sa removeAllConnection 
*/
void HbDataForm::removeAllConnection(HbDataFormModelItem *item)
{   
    Q_D(HbDataForm);
    d->removeAllConnection(item);
}

#include "moc_hbdataform.cpp"


