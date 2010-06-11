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
#include "hbdataformmodelitem_p.h"
#include "hbtreemodeliterator_p.h"

#include <QGraphicsSceneMouseEvent>
#include <QCoreApplication>

/*!
    @beta
    @hbwidgets
    \class HbDataForm
    \brief HbDataForm represents hierarchical dataitems in form of form pages, groups, group pages
    and data items.
    HbDataForm implements a hierarchical representation of view items for each model items from 
    HbDataFormModel.

    HbDataForm implements the interfaces defined by the HbAbstractItemView class to allow 
    it to display data provided by models which are derived from QAbstractItemModel class.

    It is simple to construct a dataform displaying data from a model. The user has to create
    HbDataFormModel and create the hierarchy of HbDataFormModelItems.The hierarchy is 
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
       - HbDataGroup3
          - HbDataItem
          - HbDataItem
          - HbDataItem
       - HbDataItem
       - HbDataItem
 
    HbDataItem can be the child of HbDataForm, HbDataFormPage, HbDataGroup and 
    HbDataGroupPage. An instance of HbDataForm has to be created and model should be set 
    to the form using setModel( ) API.
    The properties of each data item node can be set using HbDataFormModelItem convenient
    API's like setContentWidgetData( ). These model data are parsed and set while the visualization 
    instance of each item is created.

    The model/view architecture ensures that the view contents are updated as and when the data in
    model changes.

    Only model items that can have children can be in expanded (childrens are visible) or 
    collapsed (childrens are hidden) state. Model items of type HbDataFormModelItem::FormPageItem,
    HbDataFormModelItem::GroupItem and HbDataFormModelItem::GroupPageItem can be expanded 
    or collapsed. Which in turn means that these types of model item can only have children. 
    Each item in model is represented by either an instance of HbDataFormViewItem or classes which 
    are derived from HbDataFormViewItem. HbDataFormViewItem can be subclassed for
    customization purposes.
    
    The Model hierarchy can be created using the convenient API's provided in model class like
    appendDataFormPage(), appendDataFormGroup(), appendDataFormGroupPage() and 
    appendDataFormItem(). All these API's return HbDataFormModelItem instance corresponding 
    to each HbDataFormModelItem::DataItemType type on which user can set item 
    specific(content widget) data. Otherwise each HbDataFormModelItem can be created individually
    by passing the corresponding type of item (GroupItem, GroupPageItem, FormPageItem) and create
    the tree of HbDataFormModelItem using setParent API or by passing the parent 
    HbDataFormModelItem in constructor. Later the top level HbDataFormModelItem can be added in 
    model.

    After setting model in HbDataForm using setModel(), the visualization gets created.
    Only the items inside the expanded form page, group or group page are created. When an item's
    visualization is created, HbDataForm emits itemShown(constQModelIndex&) signal. The application 
    can connect to this signal and when corresponding slot is called then application can get
    HbDataFormViewItem instance and even content widget instance. Use HbAbstractItemView::itemByIndex()
    to get HbDataFormViewItem instance. Use HbDataFormViewItem::dataItemContentWidget() to get
    content widget instance.
    
    The signals emitted by HbDataForm
    \li itemShown(const QModelIndex &index) Emitted when the HbDataFormViewItem corresponding to
    \a index is shown. User can connect to this signal and can fetch the instance of 
    HbDataFormViewItem from HbDataForm using the API dataFormViewItem(const QModelIndex &index).
    This signal is only emitted for model items of type greater than HbDataFormModelItem::GroupPageItem

    The user can also provide connection information to correspoding content widget of each 
    HbDataFormModelItem using API 
    addConnection(HbDataFormModelItem* item, const char* signal, QObject* receiver, const char* slot)
    provided in HbDataForm. The connection will be established when the item visualization is created.
    Using addConnection() API user can also connect to hbdialog's signals(for ex: aboutToClose) in case 
    of popup items like radio button list item and multi selection list item. Below code snippet demonstrates
    the same:

    \code
    HbDataFormModelItem *days = model->appendDataFormItem(HbDataFormModelItem::MultiselectionItem,
                                    QString("Days"), themeGeneral);
    QStringList multiItems;
    multiItems<<"Sunday"<<"Monday"<<"Tuesday"<<"Wednesday"<<"Thursday"<<"Friday";
    days->setContentWidgetData(QString("items"), multiItems);
    QList<QVariant> selected;
    selected<<2<<3;
    days->setContentWidgetData(QString("selectedItems"), selected);
    days->setContentWidgetData(QString("items"), multiItems);
    form->addConnection(days, SIGNAL(aboutToShow()), this, SLOT(aboutToShow()));
    form->addConnection(days, SIGNAL(aboutToHide()()), this, SLOT(aboutToHide()()));
    form->addConnection(days, SIGNAL(aboutToClose()), this, SLOT(aboutToClose()));
    form->addConnection(days, SIGNAL(finished(HbAction*)), this, SLOT(finished(HbAction*)));

    \endcode

    Similar way 
    removeConnection(HbDataFormModelItem *item, const char* signal, QObject *receiver, const char* slot)
    and removeAllConnection() API can be used. Connection can be established or removed even at runtime.
    An example of how to make connection and setting the content widget property:

    \code
    HbDataForm *form = new HbDataForm();
    model = new HbDataFormModel();

    HbDataFormModelItem *sliderItem = 
        model->appendDataFormItem(HbDataFormModelItem::SliderItem, QString("slider"));
    //Set the content widget properties. In this case its HbSlider.
    sliderItem->setContentWidgetData("maximum", 200);
    sliderItem->setContentWidgetData("minimum", 0);
    sliderItem->setContentWidgetData("value", 100);
    //Make a connection to HbSlider valueChanged signal.
    form->addConnection(sliderItem, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged(int)));

    form->setModel(model);
    setWidget(form);
    \endcode
    
    An example of how to create HbDataForm:
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,31}

    The output generated by the above code looks like:

    \image html hbsettingform.png

    This is how HbDataForm will look like in landscape mode:

    \image html hbsettingform_landscape.png

    \sa HbDataFormViewItem, HbDataFormModel, HbDataFormModelItem

    Creating Custom Item:

    Application developer can create custom DataItem by deriving from HbDataFormViewItem and setting this as 
    prototype using setItemProtoType() API. Application has to override virtual API's createCustomWidget(),
    restore()and save(). createCustomWidget() API should return the corresponding custom HbWidget which
    can also be a compound widget. Signal connection for child widgets inside the compound widget should
    be taken care by the application. restore() API will be called by the framework when the model data 
    is changed. So restore() should take care of updating the visual items with correspoding data from model. 
    save() API should update the model. App developer should connect respective widgets SIGNALs to SLOT save() 
    and update the data to model .

*/

/*!
    \fn void HbAbstractItemView::itemShown(const QModelIndex &index)

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

    Scrolls the view so that the item represented by \a index position is changed as per \a hint
    parameter. By default HbDataForm does not scrolls. Application developer is supposed to 
    call this API if he wants this behaviour. User can connect to itemShown signal and then
    can call this API.
*/
void HbDataForm::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    HbAbstractItemView::scrollTo(index, hint);
}


/*!
    @beta

    Sets the item referred to by \a index to either collapse or expanded state, 
    depending on the value of \a expanded. If \a expanded is true then child item are 
    supposed to be visible and in that case itemShown will be emitted for all the
    new data items which were created.

    \sa isExpanded
*/
void HbDataForm::setExpanded(const QModelIndex &index, bool expanded)
{
    Q_D(HbDataForm);

    if (isExpanded(index) != expanded) {
        d->treeModelIterator()->itemExpansionChanged(index);

        HbDataFormViewItem *item =
            static_cast<HbDataFormViewItem *>(d->mContainer->itemByIndex(index));
        if (item) {
            item->setExpanded(expanded);
        }

        d->mContainer->setItemTransientStateValue(index, "expanded", expanded);
        d->mContainer->setModelIndexes();
    }
}

/*!
    @beta

    Returns true if the model item at \a index is expanded otherwise returns false.

    \sa setExpanded
*/
bool HbDataForm::isExpanded(const QModelIndex &index) const
{
    Q_D(const HbDataForm);
    QVariant flags = d->mContainer->itemTransientState(index).value("expanded");
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
    \sa setDescription
    \sa description
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
        static_cast<HbDataItemContainer*>(d->mContainer)->setFormHeading(d->mHeadingWidget);
    }
    d->mHeadingWidget->callPolish();
}

/*!
    @beta

    Returns heading of HbDataForm.

    \sa setHeading
    \sa setDescription
    \sa description
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
    \sa setHeading
    \sa heading
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
            static_cast<HbDataItemContainer*>(d->mContainer)->setFormHeading(d->mHeadingWidget);
    }
    d->mHeadingWidget->callPolish();
}

/*!
    @beta

    Returns description of HbDataForm.

    \sa setDescription
    \sa setHeading
    \sa heading
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

    \deprecated HbDataForm::primitive(HbStyle::Primitive)
         is deprecated.

    \reimp

    Returns the style primitive of HbDataForm depending upon the type \a primitive.
    \sa primitive
*/
QGraphicsItem* HbDataForm::primitive(HbStyle::Primitive primitive) const
{
    Q_D(const HbDataForm);

    switch (primitive) {
        case HbStyle::P_DataForm_heading_background:
            return d->mHeadingWidget->mBackgroundItem;
        case HbStyle::P_DataForm_heading:
            return d->mHeadingWidget->mHeadingItem;
        case HbStyle::P_DataForm_description:
            return d->mHeadingWidget->mDescriptionItem;
        default:
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
    \reimp
*/
void HbDataForm::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_UNUSED(bottomRight);
    if(topLeft.isValid()) {

            HbDataFormViewItem* item = static_cast<HbDataFormViewItem*>(itemByIndex(topLeft));
            HbDataFormModelItem *modelItem = 
                        static_cast<HbDataFormModel *>(model())->itemFromIndex(topLeft);           
            HbDataFormModelItemPrivate *modelItem_priv = HbDataFormModelItemPrivate::d_ptr(modelItem);

            if(item){
                HbDataFormViewItemPrivate::d_ptr(item)->setEnabled( modelItem->isEnabled() );
                if( modelItem_priv->dirtyProperty() == "LabelRole"      ||
                    modelItem_priv->dirtyProperty() == "DecorationRole" || 
                    modelItem_priv->dirtyProperty() == "DescriptionRole" ) {

                     HbDataFormViewItemPrivate::d_ptr(item)->updateData();
                     return;
                }
                item->restore();
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
void HbDataForm::rowsInserted(const QModelIndex &parent, int start, int end)
{
    HbAbstractItemView::rowsInserted(parent, start, end);
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
    @beta 

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
    \sa removeAllConnection
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
    @beta

    This API can be used to remove the signal connection which was established using the
    addConnection API.

    \sa addConnection
    \sa removeAllConnection
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
    @beta

    Removes the connection of all the contentwidget of all the items which has been established.
    The connection information stored inside data form is also cleared.

    \sa removeConnection
    \sa addConnection
*/
void HbDataForm::removeAllConnection()
{   
    Q_D(HbDataForm);
    d->removeAllConnection();
}

/*!
    @beta

    Removes all connections to the contentwidget of HbDataFormModelItem's corresponding 
    visual Item ( HbdataFormViewItem ).The connection information of correspoding 
    HbDataFormModelItem stored inside data form also cleared.

    \sa removeAllConnection 
*/
void HbDataForm::removeAllConnection(HbDataFormModelItem *item)
{   
    Q_D(HbDataForm);
    d->removeAllConnection(item);
}

#include "moc_hbdataform.cpp"


