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
    \brief HbDataFormViewItem represents an item view in HbDataForm corresponding to model item.
    Each HbDataFormModelItem added inside model is represented using HbDataFormViewItem instance.
  
    HbDataFormViewItem have different visualization based upon the 
    HbDataFormModelItem::DataItemType:

    - FormPageItem: This type does not have any visualization. Whatever QString is passed while
        creating HbDataFormModelItem, is added in a combo box at the top level view. User can 
        switch between different form page using this combo.
    - GroupItem: A visualization is created for this type. Visualization includes +/- icon and 
        group heading. User can expand and collapse the group by clicking any where in 
        HbDataFormViewItem of this type. Whatever QString is passed while creating
        HbDataFormModelItem for this type is set as a group heading.
    - GroupPageItem: This type does not have any visualization. Whatever QString is passed while
        creating HbDataFormModelItem, is added in a group combo box. User can switch between 
        different group page using this combo.
    - DataItems: Any type other then FormPageItem, GroupItem and GroupPageItem is treated as
        a data item. Data item can contain label, description, icon and content widget. Data items
        can not have any children. They are always placed at the leaf. Data item content widget 
        can be set using HbDataFormModelItem::ItemTypeRole, label of data items can be set using 
        HbDataFormModelItem::LabelRole, description of data item can be set using 
        HbDataFormModelItem::DescriptionRole.
    
    If HbDataFormViewItem represents a GroupItem then it can be expanded and collapsed. 
    If group is expanded then all the child items are shown.

    If user wants to create a custom data item then he has to derive from this class and set that
    as a prototype for HbDataForm using HbAbstractItemView::setItemPrototype API. While creating
    data for custom data items user should pass value greater than or equal to 
    DataItemType::CustomItemBase. When visualization is created and if data item type is
    custom item then createCustomWidget() is called. User has to override this API and pass the
    custom widget which he wants to show in data item. Below is the code snippet:

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

    The signals emitted by this class are:
    \li itemShown(const QModelIndex&) This signal is emitted when ever this item becomes visible.
    
    Refer HbDataForm documentation for sample code.

    \sa HbDataForm, HbDataFormModel, HbDataFormModelItem
*/

/*!
    Constructs HbDataFormViewItem with given \a parent.
    \param parent parent .
 */
HbDataFormViewItem::HbDataFormViewItem(QGraphicsItem *parent):
    HbAbstractViewItem(*new HbDataFormViewItemPrivate(this), parent)
{
    Q_D(HbDataFormViewItem);
    d->q_ptr = this;
}

/*!
    Destructs the HbDataFormViewItem.
*/
HbDataFormViewItem::~HbDataFormViewItem()
{
}

/*!
    \reimp
    Creates HbDataFormViewItem. This function is called from HbAbstractItemContainer 
    when model is getting parsed for creating items. 

 */
HbAbstractViewItem* HbDataFormViewItem::createItem()
{
    return new HbDataFormViewItem(*this);
}
/*!
    \reimp
    Returns true if \a model index is supported by HbDataFormViewItem prototype, otherwise returns false.
    This function is called for every item on the prototype list (if several prototypes exist)
    until item is found, which can create view item for \a index.

    \sa HbAbstractItemView::setItemPrototype(HbAbstractViewItem *prototype)
    \sa HbAbstractItemView::setItemPrototype(const QList<HbAbstractViewItem *> &prototypes)
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
    \reimp
    Updates child graphics items to represent current state and content stored in model. In case when 
    HbDataFormViewItem represents data item and DataItemType is set to custom item, then 
    createCustomWidget is called. User can override createCustomWidget and can pass his own
    custom widget.

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
        style()->updatePrimitive(
            d->mBackgroundItem, HbStyle::P_DataItem_background, &options );
    }

}

/*!
    \protected constructor
*/
HbDataFormViewItem::HbDataFormViewItem(HbDataFormViewItemPrivate &dd, QGraphicsItem *parent):
    HbAbstractViewItem(dd, parent)
{
    Q_D(HbDataFormViewItem);
    d->q_ptr = this;
    setProperty( "hasIcon", false );
}

/*!
    \protected constructor
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
    The content widget property for restoring and saving the data need to be initialized when the 
    data item is created. If model item type is custom, then application developer has to override
    this API in order to get notification when data is changed in model.

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

    Saves the current data of the content widget in data item to the model.
    The property for restoring and saving the data need to be initialized when the 
    data item is created. If model item type is custom, then application developer has to override
    this API in order to save the content widget value in model.

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

    This is a virtual function which by default returns NULL. This function must be overridden
    in case user wants to create a data item of type custom item. The user is supposed to pass
    the widget which he wants to display in data item.
    If content widget grabs pan gesture and user wants data form to be scrollable even panning
    is done on disabled content widget then user is supposed to ungrab pan gesture when content 
    widget is disabled. And again grab pan gesture when state of content widget is changed to 
    enabled.
*/
HbWidget* HbDataFormViewItem::createCustomWidget()
{
    return 0;
}

/*!
    \reimp 
    Sets the item to either collapse or expanded state, depending on the value of \a expanded.
    The function calls setModelIndexes which inturn will make the child items visible/invisible 
    accordingly. This API is valid only if HbDataFormViewItem represents a FormPageItem, GroupItem
    or GroupPageItem.

    \sa isExpanded
*/
void HbDataFormViewItem::setExpanded(bool expanded)
{
    Q_D(HbDataFormViewItem);
    // Expansion is valid only for group ,form page and group page
    if( d->mType < HbDataFormModelItem::SliderItem ) {
        static_cast<HbDataGroup*>(this)->setExpanded(expanded);
    } 
}

/*!
    \reimp
    Returns the expanded state of item.

    \sa setExpanded
*/
bool HbDataFormViewItem::isExpanded() const
{
    Q_D( const HbDataFormViewItem);
    // Expansion is valid only for group ,form page and group page
    if( d->mType < HbDataFormModelItem::SliderItem ) {
        HbDataGroup *group = qobject_cast<HbDataGroup *>(const_cast<HbDataFormViewItem*>(this));
        if( group ) {
            return group->isExpanded();
        } else {
            return false;
        }
    } 
    return false;
}

/*!
    @beta

    Returns the content widget of data item. For example if data item is of type SliderItem 
    then this API will return the instance of HbSlider. If user wants to connect to some 
    signals of content widget in data item then this API can be used to fetch the instance
    of the widget. It will return the instance only if data item is visible. User can connect
    to HbDataForm::itemShown() signal and when this item is visible then he can query the 
    content widget using this API.
*/
HbWidget* HbDataFormViewItem::dataItemContentWidget()const
{
    Q_D(const HbDataFormViewItem);
    HbWidget *widget = d->mContentWidget;;

    switch( d->mType ) {
        case HbDataFormModelItem::RadioButtonListItem:
            {
                widget = static_cast<HbRadioItem*>(d->mContentWidget)->createRadioButton();
            }
            break;
        case HbDataFormModelItem::MultiselectionItem:
            {
                widget = NULL;
            }
            break;
        case HbDataFormModelItem::ToggleValueItem:
            {
                widget = static_cast<HbToggleItem*>(d->mContentWidget)->contentWidget();
            }
            break;
        default:
            break;
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
                style()->updatePrimitive(
                d->mBackgroundItem, HbStyle::P_DataItem_background, &options );
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

