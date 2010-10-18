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

#include "hbdataform_p.h"
#include "hbdataformviewitem_p.h"
#include "hbdataitemcontainer_p.h"
#include "hbdatagroup_p.h"
#include "hbdatagroup_p_p.h"
#include "hbdataformheadingwidget_p.h"
#include "hbdataformmodelitem_p.h"
#include "hbtreemodeliterator_p.h"

#include <hbdataform.h>
#include <hbdataformmodelitem.h>
#include <hbdataformmodel.h>
#include <hbcombobox.h>

#include <QGraphicsSceneMouseEvent>
#include <QCoreApplication>
#include <QGraphicsScene>
#include <QGestureRecognizer>

#include <hbgesturerecognizers_p.h>
#include <hbgestures_p.h>

class HbTapDelayGesture:public HbTapGesture
{
public:
    using HbTapGesture::d_ptr;
    
    Q_DECLARE_PRIVATE_D(d_ptr, HbTapGesture)
};
class HbTapDelayGestureRecognizer : public HbTapGestureRecognizer
{
public:
    explicit HbTapDelayGestureRecognizer(){}
    virtual ~HbTapDelayGestureRecognizer()
    {
        int i = 0;
        i++;
    };

    QGestureRecognizer::Result recognize(QGesture *state, QObject *watched, QEvent *event)
    {
        HbTapDelayGesture* gesture = static_cast<HbTapDelayGesture *>(state);
        QMouseEvent* me = toMouseEvent(event);
        Qt::GestureState gestureState = state->state();
        switch(event->type())
        {
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseButtonPress:
            if ( state->state() == Qt::NoGesture && me->button() == Qt::LeftButton){
                
                gesture->setPosition(me->globalPos());
                gesture->setHotSpot(me->globalPos());
                gesture->setStartPos(me->globalPos());
                gesture->setScenePosition(HbGestureUtils::mapToScene(watched, me->globalPos()));
                gesture->setSceneStartPos(HbGestureUtils::mapToScene(watched, me->globalPos()));
                mTapRadius = (int)(HbDefaultTapRadius * HbDeviceProfile::current().ppmValue());        

                HbTapGesturePrivate* d_ptr = gesture->d_func();
                d_ptr->mTapStyleHint = HbTapGesture::Tap;
                if ( d_ptr->mTimerId ) {
                    gesture->killTimer(d_ptr->mTimerId);
                }
                d_ptr->mTimerId = gesture->startTimer(50);
                return QGestureRecognizer::MayBeGesture;
                
            }
            else {
                return QGestureRecognizer::Ignore;
            }
    

        case QEvent::MouseMove:
            if(gestureState != Qt::NoGesture && gestureState != Qt::GestureCanceled) { 
                return handleMouseMove(gestureState, gesture, watched, toMouseEvent(event));
            }
            else{
                if (gesture->d_func()->mTimerId) {
                    gesture->setPosition(me->globalPos());
                    gesture->setScenePosition(HbGestureUtils::mapToScene(watched, me->globalPos()));
                    gesture->setHotSpot(me->globalPos());

                    int tapRadiusSquare(mTapRadius * mTapRadius);
                    if(gesture->property(HbPrivate::TapRadius.latin1()).isValid()) {
                        qWarning("WARNING using widget specific properties in HbTapGestureRecognizer");
                        int tapRadius = gesture->property(HbPrivate::TapRadius.latin1()).toInt();
                        tapRadiusSquare = tapRadius * tapRadius;
                    }
                    QPointF delta = me->globalPos() - gesture->startPos();

                     // cancel long press with radius
                    if((delta.x() * delta.x() + delta.y() * delta.y()) > tapRadiusSquare) {                
                        gesture->killTimer(gesture->d_func()->mTimerId);
                        gesture->d_func()->mTimerId = 0;
                    }

                }
                return QGestureRecognizer::Ignore;
            }

        case QEvent::MouseButtonRelease:
            if(gestureState != Qt::NoGesture) {
                return handleMouseRelease(gestureState, gesture, watched, toMouseEvent(event));
            }
            else{
                
                if (gesture->d_func()->mTimerId) {
                    gesture->killTimer(gesture->d_func()->mTimerId);
                    gesture->d_func()->mTimerId = 0;
                    return QGestureRecognizer::FinishGesture;
                } else {
                    return QGestureRecognizer::Ignore;
                }
            }

        case QEvent::Timer:
            {
            QGestureRecognizer::Result result;
            gesture->killTimer(gesture->d_func()->mTimerId);
            gesture->d_func()->mTimerId = 0;
            if(gestureState == Qt::NoGesture) {
                result = QGestureRecognizer::TriggerGesture;        
                gesture->d_func()->mTimerId = gesture->startTimer(HbTapAndHoldTimeout);
              
            } 
            else {
                result = handleTimerEvent(gesture->state(),gesture,watched);
            }

            return result;
            }
        case QEvent::TouchBegin:
        case QEvent::TouchUpdate:        
            return HbTapGestureRecognizer::recognize(state, watched, event);

        default: break;
        }

        return QGestureRecognizer::Ignore;

    }
};

class DelayGestureInstaller
{
public:
    DelayGestureInstaller():count(0){}
    void install()
    {
        if(count <=0){
            QGestureRecognizer::unregisterRecognizer(Qt::TapGesture);
            QGestureRecognizer::registerRecognizer(new HbTapDelayGestureRecognizer);
            
        }
        count++;
    }
    void unInstall()
    {
        count--;
        if(count <1){
            QGestureRecognizer::unregisterRecognizer(Qt::TapGesture);
            QGestureRecognizer::registerRecognizer(new HbTapGestureRecognizer);
        }
    }
    int count;
};
static DelayGestureInstaller delayGestureInstaller;
/*!
    @beta
    @hbwidgets
    \class HbDataForm

    \brief The HbDataForm class is for showing and entering data in 
    hierarchically organized pages and groups of a form.

    A data form contains data items for showing and entering data in an 
    application. Each data item shown in a data form can contain an input widget 
    and an optional text label. Text fields, sliders and check boxes are typical 
    widgets used to show and collect data in an application. If a complex data 
    form contains many data items a user may be required to scroll the data form 
    content. To reduce the need to scroll, the data items can be organised into 
    elements whose hierarchy is the following:
    -  %Data form
      - Form pages
        - Groups
          - Group pages

    The data form uses a model-view architecture. HbDataFormModel represents the 
    data model for the form. You add HbDataFormModelItem objects (i.e. form 
    pages, groups, group pages and data items) to a data form model by creating 
    a HbDataFormModel object and adding HbDataFormModelItem objects to it. You 
    can then create a data form widget to show the data by creating an 
    HbDataForm object and setting its data form model. The model-view 
    architecture ensures that the content of the data form view is updated as 
    the data form model changes.

    The important thing to note is that you do not create data form widgets 
    directly in your data form. The HbDataForm object creates the appropriate UI 
    widget type for each data item in your data form model. You must specify the 
    type of widget that is shown in the data form when you create your data form 
    model.

    HbDataForm implements the interface defined by the HbAbstractItemView class 
    to display the data provided by the data form model. This model is derived 
    from the QAbstractItemModel class. To construct a data form for displaying 
    the data from a data form model, create HbDataFormModel and the hierarchy of 
    HbDataFormModelItem objects. The following rules apply in the hierarchy:
    - A form page can be a child of the data form only.
    - A group can be a child of the data form or a form page.
    - A group page can be a child of a group only.
    - A data item can be the child of data form, form page, group, and group page.

    The hierarchy can be for example the following:
    
    - %Data form
       - Form page 1
         - Group 1
           - Group page 1
             - %Data item 1
             - %Data item 2
             - %Data item 3
       - %Data item 4
       - Form page 2
         - %Data item 5
         - Group 2
           - %Data item 6
           - Group page 2
             - %Data item 7
             - %Data item 8
       - Group 3
         - %Data item 9

    To build the structure create first the HbDataForm object and set the model 
    to the form with the setModel() method. Set properties of each data item 
    with methods of HbDataFormModelItem class. The data is parsed when the 
    visualization instance of each item is created and set on each item.

    Items which have children (i.e. form pages, groups, and group pages) can be 
    in expanded (i.e. children are visible) or collapsed (i.e. children are 
    hidden) state. Each item in data form model is represented by an 
    HbDataFormViewItem object. HbDataForm uses HbDataFormViewItem prototype to 
    instantiate the data form items. HbDataFormViewItem can be subclassed for 
    customization purposes.

    The signals emitted by HbDataForm are the following:
    \li itemShown(const QModelIndex &index) signal is emitted when the 
    HbDataFormViewItem corresponding to \a index is shown. You can connect to 
    this signal and fetch the instance of HbDataFormViewItem from HbDataForm 
    with HbAbstractItemView::itemByIndex().
    \li dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight) 
    signal is emitted when the HbDataFormModel is updated \a topLeft and \a 
    bottomRight will be same since every node has only one column. You can 
    connect to this signal and fetch the instance of
    - HbDataFormViewItem from HbDataForm with HbAbstractItemView::itemByIndex() or
    - HbDataFormModelItem with HbDataFormModel::itemFromIndex(). HbDataForm 
    takes care of updating the corresponding the visualization of item when you 
    update the model with HbDataFormModelItem::setContentWidgetData().

    You can also provide the connection information to the corresponding content 
    widget of each HbDataFormModelItem with the \link HbDataForm::addConnection(HbDataFormModelItem * item, const char* signal, QObject *receiver, const char* slot) HbDataForm::addConnection()\endlink 
    method. The connection is established when the item visualization is 
    created. You can use \link HbDataForm::removeConnection(HbDataFormModelItem *item, const char* signal, QObject *receiver, const char* slot) HbDataForm::removeConnection()\endlink and HbDataForm::removeAllConnection() 
    methods in the same way. You can establish and remove the connection also at 
    runtime.

    \sa HbDataFormViewItem, HbDataFormModel, and HbDataFormModelItem
    
    \section _usecases_hbdataform Using the HbDataForm class
    
    \subsection _uc_hbdataform_001 Creating a data form.
    
    The following example shows how to create a data form. The code
    - creates the data form and data form model
    - adds the data form model items (i.e. groups, group pages, and data items)
    - sets the model to the view
    
    
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,31}
    
    The code creates the following structure:
    - Group 1                              - group
      - %Data Item 1  (check box)              - data item
        - Check box added to group            - text property of the data item
      - %Data Item 2 (text item)              - data item
        - Text Item added to group            - text property of the data item
    - %Data Item 3  (combo box)              - data item
    - Profile                              - group
      - Silent                              - group page
        - Slider                              - data item
      - General                              - group page
      - Meeting                              - group page
        
    The generated data form is the following:

    \image html hbsettingform.png

    The picture below shows the generated data form in the landscape mode.

    \image html hbsettingform_landscape.png

     \subsection _uc_hbdataform_002 Connecting the "sliderReleased" signal to the "volumeChanged" slot.

    In the following example the content widget is a slider whose 
    "sliderReleased"  \a signal is connected to the "volumeChanged" slot which 
    handles the changed volume.
    
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

     \subsection _uc_hbdataform_003 Creating the model hierarchy.

    You can create the model hierarchy with the \link 
    HbDataFormModel::appendDataFormPage() appendDataFormPage()\endlink, \link 
    HbDataFormModel::appendDataFormGroup() appendDataFormGroup()\endlink, \link 
    HbDataFormModel::appendDataFormGroupPage() 
    appendDataFormGroupPage()\endlink, and \link 
    HbDataFormModel::appendDataFormItem() appendDataFormItem()\endlink methods 
    of the HbDataFormModel class. All of these methods will return 
    HbDataFormModelItem object corresponding to each type in which the user can 
    set item specific data. 
    
    After running the setModel method the visualization is created . The items 
    of expanded groups and group pages are created. The data form emits the 
    itemShown(const QModelIndex &index) signal when an the  visualization of 
    item is created. The application can get HbDataFormViewItem and content 
    widget from HbDataForm using QModelIndex.
*/

/*!
    \fn void HbDataForm::itemShown(const QModelIndex &index)

    This signal is emitted when HbDataFormViewItem corresponding to \a index is 
    shown.

*/

/*!
    Constructs a data form with the given \a parent.
 */
HbDataForm::HbDataForm(QGraphicsItem *parent)
    : HbAbstractItemView(*new HbDataFormPrivate(), new HbDataItemContainer(),
                         new HbTreeModelIterator(0, QModelIndex(), false), parent)
{
    Q_D( HbDataForm );
    d->q_ptr = this;
    d->init();
    //setVerticalScrollBarPolicy(ScrollBarAlwaysOff);
}

/*!
    Constructs a data form with the given private class object \a dd, 
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
    Destructor.
*/
HbDataForm::~HbDataForm()
{
}

/*! Scrolls the view so that the data form item of given \a index is shown at 
the given \a hint position of the screen. By default the data form does not 
scroll, so to make it scroll connect to the activated signal first and then call 
this method.
    
    \param index - item of the data form to be scrolled to the \a hint position. 
    \param hint - position the item is scrolled to on the view, for example to 
    the top or center. The values of \a hint are the following:

    - EnsureVisible (default)
    - PositionAtTop
    - PositionAtBottom
    - PositionAtCenter

*/
void HbDataForm::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    HbAbstractItemView::scrollTo(index, hint);
}


/*!
    @beta

    Expands and collapses the given item specified by \a index, depending on the 
    given \a expanded value. If it is \c true, the item is expanded. If it is \c 
    false, the item is collapsed. When the item is
    
    - expanded, its child items are shown.
    - collapsed, its child items are not shown.

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
        // If view item is not yet created then set the ItemTransientState so that 
        // when ever it gets created expansion state will be considered . This is valid for formPage group 
        // and group page . Itemstate for the leaf items also will be set but does not have any
        // significance since these items cannot expand( do not have children )
        
        else {
            HbDataFormModelItem *modelItem = static_cast<HbDataFormModel *>(model())->itemFromIndex(index);
            if(modelItem->type() == HbDataFormModelItem::GroupPageItem ) {
                d->collapseAllGroupPages(index.parent());
            }
            d->mContainer->setItemTransientStateValue(index, "expanded", expanded);
        } 
    }
}

/*!
    @beta

    Returns \c true if the given model item is expanded (i.e. children are 
    visible), otherwise returns \c false.
    
    \param index - the model item

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

    Sets the data form's heading to be the given \a heading. The heading is 
    displayed on the top of form and it is non-focusable.

    \sa heading, setDescription and description
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

    Returns the heading of the data form.

    \sa setHeading, setDescription and description
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

    Sets the data form's description to be the given \a description. The 
    description is displayed below the heading and it is non-focusable.

    \sa description, setHeading and heading
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

    Returns the description of the data form.

    \sa setDescription, setHeading and heading
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

    Returns the style primitive of HbDataForm depending on the type \a primitive.
    \sa primitive
*/
QGraphicsItem* HbDataForm::primitive(HbStyle::Primitive primitive) const
{
    Q_D(const HbDataForm);

    switch (primitive) {
        case HbStylePrivate::P_DataForm_heading_background:
            return d->mHeadingWidget->mBackgroundItem;
        case HbStylePrivate::P_DataForm_heading:
            return d->mHeadingWidget->mHeadingItem;
        case HbStylePrivate::P_DataForm_description:
            return d->mHeadingWidget->mDescriptionItem;
        default:
            return 0;
    }
}



/*!
    Sets the data form model to the given \a model. If the given \a model is 
    NULL then all values of the data form are reset. This method does not clear 
    the heading and description of the data form. If you want the heading and 
    description of the data model to be empty, call HbDataForm::setHeading() and 
    HbDataForm::setDescription() with an empty string as a parameter.
    
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
                if( modelItem_priv->dirtyProperty() == "LabelRole"      ||
                    modelItem_priv->dirtyProperty() == "DecorationRole" || 
                    modelItem_priv->dirtyProperty() == "DescriptionRole" ) {

                     HbDataFormViewItemPrivate::d_ptr(item)->updateData();
                     return;
                } else if(modelItem_priv->dirtyProperty() == "enabled") {
                    HbDataFormViewItemPrivate::d_ptr(item)->setEnabled( modelItem->isEnabled() );
                }
                item->restore();
            }
    }
}
/*!
    Initializes the style option data form defined by \a option with the data 
    form values.
    
    \param option - Style option data form to be initialized.
*/
void HbDataForm::initStyleOption(HbStyleOptionDataForm *option)
{
    Q_D(HbDataForm);
    d->mHeadingWidget->initStyleOption(option);
}


/*!

*/
void HbDataForm::rowsInserted(const QModelIndex &parent, int start, int end)
{
    HbAbstractItemView::rowsInserted(parent, start, end);
}

/*!

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

    Connects the \a signal of content widget \a item to the \a slot of \a 
    receiver object.
    
    \param item - %Data form model item.
    \param signal - Signal of the content widget. 
    \param receiver - Object whose slot is called. 
    \param slot - Slot of \a receiver object which is called when \a signal is emitted.
 
    \sa removeConnection 
*/
void HbDataForm::addConnection(HbDataFormModelItem * item, 
                               const char* signal, 
                               QObject *receiver, 
                               const char* slot)
{   
    Q_D(HbDataForm);
    ItemSignal itemSignal;
    itemSignal.receiver = receiver;
    itemSignal.signal = signal;
    itemSignal.slot = slot;
    d->mConnectionList.insertMulti(item, itemSignal);
    d->connectNow(item, signal, receiver, slot);
}

/*!
    @beta

    Removes the connection between the signal of content widget object and the 
    slot of receiver object.

    \param item - %Data form model item.
    \param signal - The signal of content widget object.
    \param receiver - The object whose slot is called.
    \param slot - The slot of \a receiver object which is called when \a signal is emitted.

    \sa addConnection and removeAllConnection
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

    Removes all the connections between signals and slots of all content widgets 
    of all items. The connection information stored in the underlying data form 
    is also cleared.

    \sa removeConnection and addConnection
*/
void HbDataForm::removeAllConnection()
{   
    Q_D(HbDataForm);
    d->removeAllConnection();
}

/*!
    @beta

    Removes all the connections of the content widget \a item. The connection 
    information stored in the underlying data form is also cleared.

    \sa removeAllConnection 
*/
void HbDataForm::removeAllConnection(HbDataFormModelItem *item)
{   
    Q_D(HbDataForm);
    d->removeAllConnection(item);
}


/*!
    \reimp

    This slot is called when orientation is changed.
    \a newOrientation has the currentOrientation mode.
    Note: Currently platform dependent orientation support is not available
*/
void HbDataForm::orientationChanged(Qt::Orientation newOrientation)
{
    Q_UNUSED(newOrientation);
    Q_D(HbDataForm);

    //Setting the uniform ites sizes to container again resets size caches.
    d->mContainer->setUniformItemSizes(d->mContainer->uniformItemSizes());
    d->mContainer->setPos(0,0);
    d->mContainer->resizeContainer();

    d->updateScrollMetrics();

    d->stopAnimating();
    scrollTo(d->mVisibleIndex, HbAbstractItemView::PositionAtCenter);
    d->mVisibleIndex = QModelIndex();
}

void HbDataForm::orientationAboutToBeChanged()
{
    Q_D(HbDataForm);
    QRectF rect = mapToScene(boundingRect()).boundingRect();
    HbAbstractViewItem * item = d->itemAt((rect.center()));
    if(item){
        d->mVisibleIndex = item->modelIndex();
    } else{
        HbAbstractItemView::orientationAboutToBeChanged();
    }
}

/*!
    \reimp
*/

void HbDataForm::showEvent(QShowEvent * event)
{
    //if(!isVisible() ){
    //    delayGestureInstaller.install();       
    //}    
    HbAbstractItemView::showEvent( event );
}

/*!
    \reimp
*/
void HbDataForm::hideEvent ( QHideEvent * event )  
{
    //delayGestureInstaller.unInstall();
    HbAbstractItemView::hideEvent( event );
    
}


#include "moc_hbdataform.cpp"


