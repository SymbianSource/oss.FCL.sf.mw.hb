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

#include "hbdataformviewitem.h"
#include "hbdataformviewitem_p.h"
#include "hbabstractitemcontainer_p_p.h"

#include "hbdataformmodelitem_p.h"
#include "hbdataform_p.h"
#include "hbdatagroup_p.h"
#include "hbstyleoptiondataformviewitem_p.h"

#include <hbtapgesture.h>

#ifdef HB_EFFECTS
#include "hbeffect.h"
#include "hbeffectinternal_p.h"
#define HB_DATAFORMVIEWITEM_TYPE "HB_DATAFORMVIEWITEM"
#endif

/*!
    @beta
    @hbwidgets
   
    \class HbDataFormViewItem
    
    \brief The HbDataFormViewItem class is for representing the visual appearence of the data in a data form model item.
  
    An HbDataForm object contains HbDataFormViewItem objects. To add a custom widget to a form create a subclass of HbDataFormViewItem class.
    
    HbDataFormViewItem object's appearance and functionality depends on the value of HbDataFormModelItem::DataItemType as follows:
    - FormPageItem
      - When creating the HbDataFormModelItem object, the text of constructor's QString parameter will be the item's visible text in the combo box which lists available form pages.
      - The user can select a form page from the combo box which lists available form pages.
      - The combo box is the only visible element of FormPageItem.
    - GroupItem
      - Visible element has a group title and an icon which is
          - a plus sign (+) when the user can expand the group, i.e. all the child items are shown.
          - a minus sign (-) when the user can collapse the group, i.e. all the child items are hidden.
      - User can expand and collapse the group by clicking anywhere in the group title bar.
      - When creating the HbDataFormModelItem object, the text of constructor's QString parameter will be the group title.
    - GroupPageItem
      - When creating the HbDataFormModelItem object, the text of constructor's QString parameter will be the item's visible text in the combo box which lists available group pages.
      - The user can select a group page from the group page combo box.
      - The combo box is the only visible element of GroupPageItem.
      - The HbAbstractItemView::activated() signal is emitted when a group page item is clicked.
    - DataItem
      - If the HbDataFormModelItem::DataItemType parameter's value is not FormPageItem, GroupItem or GroupPageItem, it is treated as a data item.
      - Contains
        - a label which you can set with HbDataFormModelItem::LabelRole.
        - a content widget which you can set with HbDataFormModelItem::ItemTypeRole.
        - a description which you can set with HbDataFormModelItem::DescriptionRole.
        - an icon which you can set with HbDataFormModelItem::setIcon().

      - %Data item cannot have a child, i.e. it is always the leaf of the structure.  
    
    \section _usecases_hbdataformviewitem Using the HbDataFormViewItem class
    
    \subsection _uc_hbdataformviewitem_001 Creating a custom data item.

    The code snippet below shows how you can create a custom data item. To create a custom data item object of (in this example) the DataFormCustomItem class, derive it from the HbDataFormViewItem class and set the prototype for HbDataForm with HbAbstractItemView::setItemPrototype() method. When creating data for custom data items pass a value greater than or equal to DataItemType::CustomItemBase. When visual appearance is created and if data item type is custom data item then createCustomWidget() is called. You must override this method and pass the custom widget which you want to show in the data item.

    \code
    //Derive a class from HbDataFormViewItem
    class DataFormCustomItem : public HbDataFormViewItem
    {
        Q_OBJECT

    public:
        DataFormCustomItem(QGraphicsItem *parent);
        ~DataFormCustomItem();

        virtual HbAbstractViewItem* createItem()
        {
            return new DataFormCustomItem(*this);
        }

    protected:
        //Override createCustomWidget API
        virtual HbWidget* createCustomWidget()
        {
            HbDataFormModelItem::DataItemType itemType = static_cast<HbDataFormModelItem::DataItemType>(
            modelIndex().data(HbDataFormModelItem::ItemTypeRole).toInt());
            switch(itemType){
                case HbDataFormModelItem::CustomItemBase:{
                    //Create widget which you want for this data item
                    HbWidget *widget = new HbWidget();
                    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Horizontal);
                    widget->setLayout(layout);
                    HbLineEdit *button1 = new HbLineEdit();
                    layout->addItem(button1);
                    HbLabel *label = new HbLabel(QString("testing label"));
                    layout->addItem(label);
                    //Return the custom widget created
                    return widget;
                }
                //Some other cases can also be added if there are more than one custom data item
            default:
                return 0;
            }
        }

    private:
        Q_DECLARE_PRIVATE_D(d_ptr, DataFormCustomItem)
    };

    //some were is your application
    ContentWidget::ContentWidget(QGraphicsItem *parent)
    {
        HbDataForm *form = new HbDataForm();

        DataFormCustomItem *myPrototype = new DataFormCustomItem();
        form->setItemPrototype(myPrototype);
    }


    \endcode

    The signal emitted by this class is:
    \li itemShown(const QModelIndex&) signal is emitted whenever this item becomes visible.

    See HbDataForm for sample code.

    \sa HbDataForm, HbDataFormModel, HbDataFormModelItem
*/


/*!
    Constructs a data form view item with the given \a parent.
 */
HbDataFormViewItem::HbDataFormViewItem(QGraphicsItem *parent):
    HbAbstractViewItem(*new HbDataFormViewItemPrivate(this), parent)
{
    Q_D(HbDataFormViewItem);
    d->q_ptr = this;
}

/*!
    Destructor.
*/
HbDataFormViewItem::~HbDataFormViewItem()
{
}

/*!
    Creates a data form view item. This method is called to form an HbAbstractItemContainer object when the model is parsed for creating items.

 */
HbAbstractViewItem* HbDataFormViewItem::createItem()
{
    return new HbDataFormViewItem(*this);
}

/*!
    Returns \c true if the given model \a index is supported by the data form view item, otherwise returns \c false. This method is called for every item on the prototype list until an item which can create a list view item for \a index, is found. The method goes through the prototype list from the end to the beginning. 
    
    \sa HbAbstractItemView::setItemPrototype(HbAbstractViewItem *prototype) and HbAbstractItemView::setItemPrototypes(const QList<HbAbstractViewItem *> &prototypes)
 */
bool HbDataFormViewItem::canSetModelIndex(const QModelIndex &index) const
{
    HbDataFormModelItem::DataItemType itemType = 
        static_cast<HbDataFormModelItem::DataItemType>(
        index.data(HbDataFormModelItem::ItemTypeRole).toInt());

    if( ( itemType >= HbDataFormModelItem::SliderItem 
        && itemType < HbDataFormModelItem::CustomItemBase ) ) {
        return true;
    } else {
        return false;
    }

}

/*!
    Updates child graphics items to represent current state and content. In case HbDataFormViewItem represents data item and DataItemType is set to custom item, createCustomWidget is called. You can override createCustomWidget and can pass your own custom widget.

    \sa createCustomWidget

*/
void HbDataFormViewItem::updateChildItems()
{
    Q_D( HbDataFormViewItem );

    d->init();
    //this will create the visualization for all standard data items
    d->createContentWidget( );

    //set the label in data iem
    QString itemLabel = d->mIndex.data( HbDataFormModelItem::LabelRole ).toString( );
    d->setLabel( itemLabel );

    // set the icon in data item
    QString icon = d->mIndex.data( Qt::DecorationRole ).toString( );
    d->setIcon( icon );

    // set the description of data item
    QString itemDescription = d->mIndex.data( HbDataFormModelItem::DescriptionRole ).toString();
    d->setDescription( itemDescription );

    //update visualization based on whether item is enabled or disabled
    HbDataFormModel* data_model = static_cast<HbDataFormModel*>(itemView()->model());
    HbDataFormModelItem *model_item = 
        static_cast<HbDataFormModelItem*>(data_model->itemFromIndex(d->mIndex));
    d->setEnabled(model_item->isEnabled());


#ifdef HB_EFFECTS
    //HbEffectInternal::add( HB_DATAFORMVIEWITEM_TYPE,"dataform_expand", "expanded" );
    //HbEffect::start( settingItem, HB_DATAFORMVIEWITEM_TYPE, "expanded" );  
#endif

    restore( );

    // Establish Signal Connections set in HbDataFormModel to th contentWidget of this item
    HbDataFormPrivate::d_ptr(
        static_cast<HbDataForm*>(d->mSharedData->mItemView))->makeConnection(
        d->mIndex.operator const QModelIndex & (),d->mContentWidget);
    //update only the background primitive
    HbStyleOptionDataFormViewItem options;
    initStyleOption(&options);
    if( d->mBackgroundItem ) {
        HbStylePrivate::updatePrimitive(
            d->mBackgroundItem, HbStylePrivate::P_DataItem_background, &options );
    }

}

/*!
    \protected Constructs a data form view item with the given protected class object \a dd and \a parent.
*/
HbDataFormViewItem::HbDataFormViewItem(HbDataFormViewItemPrivate &dd, QGraphicsItem *parent):
    HbAbstractViewItem(dd, parent)
{
    Q_D(HbDataFormViewItem);
    d->q_ptr = this;
    setProperty( "hasIcon", false );
}

/*!
    \protected Constructs a data form view item with the given protected class object \a source.
*/
HbDataFormViewItem::HbDataFormViewItem(const HbDataFormViewItem &source):
    HbAbstractViewItem( *new HbDataFormViewItemPrivate(*source.d_func()), 0)
{
    Q_D(HbDataFormViewItem);
    d->q_ptr = this;
    setProperty( "hasIcon", false );
}

/*!
    \protected assignment operator
*/
HbDataFormViewItem& HbDataFormViewItem::operator=(const HbDataFormViewItem &source)
{
    Q_D(HbDataFormViewItem);
    *d = *source.d_func();
    setProperty( "hasIcon", false );
    return *this;
}

/*!
    @beta
    Restores the data from the model and assign to the widget.
    The property for restoring and saving the data need to be initialized when the data item is created. If the model item type is custom, then you must override this method to get a notification when the data is changed in the model.

    \sa save
*/
void HbDataFormViewItem::restore()
{
    Q_D( HbDataFormViewItem );    

    if( d->mType < HbDataFormModelItem::CustomItemBase ) {
        if( d->mContentWidget ) {   
            QVariant newValue;
            QModelIndex itemIndex = modelIndex();
            QString currentProperty;

            // fetch all properties set on this model item
            QHash<QString, QVariant> properties = d->mModelItem->contentWidgetData( );
            QList <QString> propertyNames = properties.keys( );
            int upperBound = 1;
            int lowerBound = 0;

            // get the latest modified property
            QString dirtyProperty = 
                HbDataFormModelItemPrivate::d_ptr( d->mModelItem )->dirtyProperty( );

            // First time when DataItem is created load all property, at runtime load only  the modified
            // property. 
            if(  d->mSetAllProperty ) {
                upperBound = propertyNames.count() ;
                d->mSetAllProperty = false;
            } else {
                // Fetch only the last modified property
                lowerBound = propertyNames.indexOf(dirtyProperty);
                upperBound = lowerBound + 1;
            }
            if( lowerBound > -1 ) {
                for( ;lowerBound < upperBound ;lowerBound++) {
                    newValue = properties.value(propertyNames.at(lowerBound));
                    currentProperty = propertyNames.at(lowerBound);
                    if(newValue.isValid()) {                        
                        if(!d->mContentWidget) {
                            return;
                        }
                        d->mContentWidget->setProperty(currentProperty.toAscii().data(), newValue);
                    }
                }
            }
        }
    }
}

/*!
    @beta
    Saves the current data of the content widget in data item to the model .
    The property for restoring and saving the data need to be initialized when the data item is created. If the model item type is custom, then you must override this API in order to save the content widget value into the model.

    \sa restore
*/
void HbDataFormViewItem::save()
{
    Q_D(HbDataFormViewItem);
    if( d->mType < HbDataFormModelItem::CustomItemBase ) {
        if( d->mContentWidget ) {

            disconnect( d->mModel, SIGNAL( dataChanged( QModelIndex,QModelIndex ) ),
                d->mSharedData->mItemView, SLOT( dataChanged( QModelIndex,QModelIndex ) ) );

            d->mModelItem->setContentWidgetData(
                d->mProperty, d->mContentWidget->property(d->mProperty.toAscii( ).data( ) ) );            

            connect( d->mModel, SIGNAL( dataChanged( QModelIndex,QModelIndex ) ),
                d->mSharedData->mItemView, SLOT( dataChanged( QModelIndex,QModelIndex ) ) );
        }
    }
}

/*!
    @beta

    This is a virtual method which returns NULL by default. To create a data item of the custom widget type override this method. Pass the widget which you want to be shown in the data item. If the content widget has requested to receive pan gesture events using QGraphicsObject::grabGesture(), then any scrolling is recognized as a pan gesture and is always sent to the content widget, even when the content widget is disabled. If you want the data form to be scrollable even when the content widget is disabled, you need to call QGraphicsObject::ungrabGesture() while the content widget is disabled, and grab pan gestures again when the content widget is later enabled.
*/
HbWidget* HbDataFormViewItem::createCustomWidget()
{
    return 0;
}

/*!
    Sets the item to either collapse or expanded, depending on the value of \a expanded.
    The function calls setModelIndexes which in turn will make the child items visible or invisible accordingly. This method is valid only if HbDataFormViewItem represents FormPageItem, GroupItem or GroupPageItem.

    \sa isExpanded
*/
void HbDataFormViewItem::setExpanded(bool expanded)
{
    Q_D(HbDataFormViewItem);
    // Expansion is valid only for group ,form page and group page
    if( d->mType < HbDataFormModelItem::SliderItem ) {
        static_cast<HbDataGroup*>(this)->setExpanded(expanded);
    }  else {
        HbAbstractItemContainer *container = qobject_cast<HbAbstractItemContainer *>(
        static_cast<QGraphicsWidget *>( d->mSharedData->mItemView->contentWidget( ) ) );
        container->setItemTransientStateValue(d->mIndex, "expanded", expanded);
    }
}

/*!
    Returns the expanded state of item.

    \sa setExpanded
*/
bool HbDataFormViewItem::isExpanded() const
{
    Q_D( const HbDataFormViewItem);
    // Expansion is valid only for group ,form page and group page
    if( d->mType < HbDataFormModelItem::SliderItem ) {
        HbDataGroup *group = qobject_cast< HbDataGroup *>(const_cast<HbDataFormViewItem*>(this));
        if( group ) {
            return group->isExpanded();
        }
    } 
    if(d->mSharedData->mItemView) {
        HbAbstractItemContainer *container = qobject_cast<HbAbstractItemContainer *>(
            static_cast<QGraphicsWidget *>( d->mSharedData->mItemView->contentWidget( ) ) );
        if(container) {
            return container->itemTransientState(d->mIndex).value("expanded").toBool();
        }
    }
    return false;
}


/*!
    This method is valid only if the data form view item represents a data item. The method returns the content widget of data item. For example, if the type of data item is SliderItem then this method will return the HbSlider content widget object. 
    
    You can use this method to retrieve the widget object of a data item for connecting its signals to appropriate slot. It returns the object only if the data item is visible. You can query the content widget with this method when the item is visible and connect the HbDataForm::activated() signal to an appropriate slot.
*/
HbWidget* HbDataFormViewItem::dataItemContentWidget()const
{
    Q_D(const HbDataFormViewItem);
    HbWidget *widget = d->mContentWidget;

    if(d->mContentWidget) {
        switch( d->mType ) {
            case HbDataFormModelItem::RadioButtonListItem:{
                widget = static_cast<HbRadioItem*>(d->mContentWidget)->createRadioButton();
            }
            break;
            case HbDataFormModelItem::MultiselectionItem:{
                widget = NULL;
            }
            break;
            case HbDataFormModelItem::ToggleValueItem:{
                widget = static_cast<HbToggleItem*>(d->mContentWidget)->contentWidget();
            }
            break;
            default:
            break;
        }
    }
    return widget;
}

/*!
    \reimp
*/
void HbDataFormViewItem::pressStateChanged(bool value, bool animate)
{
    //Since there are no effects defined for mousePressed and mouseReleased for 
    //HbDataFormViewItem we are overriding this function so that redundant effects functions are
    //not called in HbAbstractViewItem::pressStateChanged.
    Q_UNUSED(value);
    Q_UNUSED(animate);
}

/*!
    \reimp
*/
void HbDataFormViewItem::initStyleOption(HbStyleOptionDataFormViewItem *option) const
{
    Q_D( const HbDataFormViewItem );

    HbWidget::initStyleOption(option);
    option->label = d->mLabel;
    option->icon = d->mIcon;
    option->description = d->mDescription;
}

/*!
    \reimp
*/
void HbDataFormViewItem::showEvent(QShowEvent * event)
{
    Q_D( const HbDataFormViewItem );

    HbWidget::showEvent( event );
    if( d->mIndex.isValid( ) ) {
        emit itemShown( d->mIndex.operator const QModelIndex & ( ) );
    }
}

/*!
    \reimp
 */
QVariant HbDataFormViewItem::itemChange( GraphicsItemChange change, const QVariant &value )
{
    Q_D( HbDataFormViewItem );
    switch ( static_cast<HbPrivate::HbItemChangeValues>( change ) ) {
    case QGraphicsItem::ItemEnabledHasChanged: {
            HbStyleOptionDataFormViewItem options;
            initStyleOption(&options);
            if( d->mBackgroundItem ) {
                HbStylePrivate::updatePrimitive(
                d->mBackgroundItem, HbStylePrivate::P_DataItem_background, &options );
            }
            //We are skipping call to abstractviewitem::itemChange here because updateChildItems is 
            //called in that function which will again create data view item primitives.
            return HbWidget::itemChange( change, value );
        }
        default:
            break;
  }
    return HbAbstractViewItem::itemChange( change, value );
}

#include "moc_hbdataformviewitem.cpp"

