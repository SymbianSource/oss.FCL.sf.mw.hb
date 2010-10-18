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

#include "hblistviewitem.h"
#include "hblistviewitem_p.h"
#include "hblistview.h"

#include <hbnamespace.h>
#include <hbicon.h>
#include <hbiconitem.h>
#include <hbtextitem.h>
#include <hbrichtextitem.h>
#include <hbstyle.h>
#include <hbstyleprimitivedata.h>
#include <hbstyletextprimitivedata.h>
#include <hbstylerichtextprimitivedata.h>
#include <hbstyleiconprimitivedata.h>

#include <QPersistentModelIndex>
#include <QVariant>
#include <QItemSelectionModel>
#include <QSizePolicy>

/*!
    @beta
    @hbwidgets
    \class HbListViewItem
    \brief The HbListViewItem class represents a single row in a list.

    The HbListViewItem class provides an item that is used by the HbListView class to
    visualize content within single model index. 
    
    HbListViewItem supports following model item types in Hb::ItemTypeRole role of data model
    \li Hb::StandardItem. 
    \li Hb::SeparatorItem. 
    HbAbstractViewItem documentation provides details how these item types distinguish from each other. 

    Following item data roles are supported by HbListViewItem
    \li QString and QStringList stored in Qt::DisplayRole role of the data model
    \li QIcon, HbIcon or list of them in QVariantList stored in Qt::DecoratorRole role. 
    QIcon is supported only for compatibility reasons. If QIcon is used the limitations 
    described in the HbIcon::HbIcon(const QIcon &icon) apply.
    
    Handling Qt::BackgroundRole item data role takes place in base class HbAbstractViewItem.

    This class is provided mainly for customization purposes but it also acts as a default
    item prototype inside HbListView. See HbListView how to set customized class as a item prototype.

    \b Subclassing

    When subclassing HbListViewItem, you must provide implementations of createItem() and updateChildItems() functions.
    
    To support multiple list view items within single list view, you must also provide an implementation of canSetModelIndex().

    If derived list view item has transient state information that is not meaningful to store within model index (child item cursor 
    position selection areas etc.) view item can use list views internal state model to store this information. This feature can 
    be taken into use by implementing state() and setState() functions in derived class.

    \b Layout

    Layout selection in list view item is based on HbListViewItem::GraphicsSize and HbListViewItem::StretchingStyle properties together with list view item content.

    Below is described how different subitems are layouted with default layouts, how content is assigned to different subitems and
    what content is optional and what is mandatory in which layout.

    \note Item sizes and alignment related to each other may differ from real layouts in these pictures. 

    \image html listlayout-1.png Icon layout.

    \image html listlayout-2.png Image layout.

    \image html listlayout-3.png Landscape stretched layout.

    Optional items are indicated with dotted line. In all layouts both text-1 and text-2 item will stretch if required in horizontal direction.

    Image layout is selected when HbListViewItem::GraphicsSize is set as HbListViewItem::Image; otherwise icon layout is selected in portrait. 

    Landscape stretched layout will be used in landscape if HbListViewItem::StretchingStyle is set as HbListViewItem::StretchLandscape; 
    otherwise same layout is used both in portrait and landscape. In stretched layout it is mandatory to have either text-1 or icon-1.

    <table align=center>
         <tr><td> Item name </td>     <td> Description </td>
    </tr><tr><td> selection-icon </td><td> Shown when view selection mode is other than HbAbstractItemView::NoSelection. </td>
    </tr><tr><td> icon-1 </td>        <td> HbIcon from Qt::DecorationRole or if Qt::DecorationRole contains an icon list then first HbIcon from it. </td>
    </tr><tr><td> icon-2 </td>        <td> Second HbIcon from icon list inside Qt::DecorationRole. </td>
    </tr><tr><td> text-1 </td>        <td> QString from Qt::DisplayRole or if Qt::DisplayRole contains a string list then first QString from it. </td>
    </tr><tr><td> text-2 </td>        <td> Second QString from string list inside Qt::DisplayRole. </td>
    </tr><tr><td> text-3 </td>        <td> Third QString from string list inside Qt::DisplayRole. </td></tr>
    </table>        

    Below is an example how the layout properties can be configured.

    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,45}

    \primitives
    \primitive{icon-1} HbIconItem with item name "icon-1" representing icon-1 as described in the table above. 
    \primitive{icon-2} HbIconItem with item name "icon-2" representing the icon-2 as described in the table above. 
    \primitive{text-1} HbTextItem or HbRichTextItem with item name "text-1" representing the text-1 as described in the table above. The type of the return value depends on the textFormat() of the item.
    \primitive{text-2} HbTextItem or HbRichTextItem with item name "text-2" representing the text-2 as described in the table above.
    \primitive{text-3} HbTextItem or HbRichTextItem with item name "text-3" representing the text-3 as described in the table above.

    \sa HbAbstractViewItem
*/

/*!
    \enum HbListViewItem::StretchingStyle

    Stretching styles supported by HbListViewItem.

    This enum describes different stretching styles available for list view items in landscape.
*/

/*!
    \var HbListViewItem::NoStretching

    Stretched layout is not used in portrait or landscape resolutions. This is the default value.
*/


/*!
    \var HbListViewItem::StretchLandscape

    Strecthed layout is used in landscape resolution.
*/


/*!
    \enum HbListViewItem::GraphicsSize

    Graphics sizes supported by HbListViewItem.

    This enum describes different graphics sizes available for list view items.
*/

/*!
    \var HbListViewItem::SmallIcon

    Icon layout is used with small icon-1 size.
*/

/*!
    \var HbListViewItem::MediumIcon

    Icon layout is used with medium icon-1 size. This is the default value.
*/

/*!
    \var HbListViewItem::LargeIcon

    Icon layout is used with large icon-1 size.
*/

/*!
    \var HbListViewItem::Thumbnail

    Thumbnail layout is used.
*/

/*!
    \var HbListViewItem::WideThumbnail

    16:9 Thumbnail layout is used.
*/


HbListViewItemPrivate::HbListViewItemPrivate(HbListViewItem *prototype) :
    HbAbstractViewItemPrivate(prototype, new HbListViewItemShared)
{
}

HbListViewItemPrivate::HbListViewItemPrivate(HbListViewItem *prototype, HbListViewItemShared *shared) :
    HbAbstractViewItemPrivate(prototype, shared)
{
}
 
HbListViewItemPrivate::HbListViewItemPrivate(const HbListViewItemPrivate &source) :
    HbAbstractViewItemPrivate(source)
{
}

HbListViewItemPrivate &HbListViewItemPrivate::operator=(const HbListViewItemPrivate &source)
{
    HbAbstractViewItemPrivate::operator=(source);
    
    mItemsChanged = false;

    return *this;
}

HbListViewItemPrivate::~HbListViewItemPrivate()
{
}

void HbListViewItemPrivate::init()
{
    if (isPrototype()) {
        HbEffect::add("listviewitem-focus", "listviewitem_press", "pressed");
        HbEffect::add("listviewitem-focus", "listviewitem_release", "released");
        mSharedData->mItemType = "listviewitem";

        HbEffect::add("viewitem", "viewitem_appear", "appear");
        HbEffect::add("viewitem", "viewitem_disappear", "disappear");
    }
    mContentChangedSupported = true;
    mItemsChanged = false;
}

void HbListViewItemPrivate::setDecorationRole(const QVariant& value,
                                       const int index)
{
    Q_Q( HbListViewItem );

    // create icon item and set it to layout
     HbStyle::PrimitiveType primitive = decorationPrimitive(value);

     if (primitive != HbStyle::PT_None) {
        QGraphicsObject *item = mDecorationRoleItems.value(index);
        if (!item) {
            mItemsChanged = true;
            themingPending = true;

            QString name;
            if (index == 0) {
                name = QLatin1String("icon-1");
            } else if (index == 1) {
                name = QLatin1String("icon-2");
            } else {
                name = QLatin1String("icon-") + QString::number(index + 1);
            }

            item = q->style()->createPrimitive(primitive, name, 0);
            item->setParentItem(q); // To enable asynchronous icon loading.
            item->setZValue(index + 1);

            if (index < mDecorationRoleItems.count()) {
                mDecorationRoleItems.replace(index, item);
            } else {
                mDecorationRoleItems.append(item);
            }
        }
    } else {
        mItemsChanged = true;
        if (index < mDecorationRoleItems.count()) {
            delete mDecorationRoleItems.at(index);
            mDecorationRoleItems.replace(index, 0);
        } else {
            mDecorationRoleItems.insert(index, 0);
        }
    }
}

void HbListViewItemPrivate::setDisplayRole(const QString& value,
                                           const int index)
{
    Q_Q( HbListViewItem );

    // create text item  and set it to layout
    if (!value.isNull()) {
        QGraphicsObject *textItem = mDisplayRoleTextItems.value(index);

        HbStyle::PrimitiveType primitive = displayPrimitive();
        if (!textItem) {
            mItemsChanged = true;
            themingPending = true;

            QString name;
            if (index == 0) {
                name = QLatin1String("text-1");
            } else if (index == 1) {
                name = QLatin1String("text-2");
            } else if (index == 2) {
                name = QLatin1String("text-3");
            } else {
                name = QLatin1String("text-") + QString::number(index + 1);
            }

            textItem = q->style()->createPrimitive(primitive, name, q);
            HbTextItem *realTextItem = qobject_cast<HbTextItem*>(textItem);
            if (realTextItem) {
                realTextItem->setTextWrapping(Hb::TextNoWrap);
            }
            if (index < mDisplayRoleTextItems.count()) {
                mDisplayRoleTextItems.replace(index, textItem);
            } else {
                mDisplayRoleTextItems.append(textItem);
            }
        }
    } else {
        if (index < mDisplayRoleTextItems.count()) {
            QGraphicsItem *item = mDisplayRoleTextItems.at(index);
            if (item) {
                mItemsChanged = true;
                delete item;
                mDisplayRoleTextItems.replace(index, 0);
            }
        } else {
            mDisplayRoleTextItems.insert(index, 0);
        }
    }
}

/*!
    Constructs a list view item with \a parent.
*/
HbListViewItem::HbListViewItem(QGraphicsItem *parent) : 
    HbAbstractViewItem(*new HbListViewItemPrivate(this), parent)
{
    Q_D( HbListViewItem );
    d->q_ptr = this;

    d->init();
}

/*!
    Creates a separate graphics widget with same list view item state as \a source.
*/
HbListViewItem::HbListViewItem(const HbListViewItem &source) :
    HbAbstractViewItem(*new HbListViewItemPrivate(*source.d_func()), 0)
{
    Q_D( HbListViewItem );
    d->q_ptr = this;

    d->init();
}

/*!
    Constructs a list view item with private view item \a dd object and \a parent.
*/
HbListViewItem::HbListViewItem(HbListViewItemPrivate &dd, QGraphicsItem * parent) :
    HbAbstractViewItem(dd, parent)
{
    Q_D( HbListViewItem );
    d->q_ptr = this;

    d->init();
}

/*!
    Destructor.
*/
HbListViewItem::~HbListViewItem()
{
}

/*!
    Assigns the \a source list view item to this list view item and returns a reference to this item.
*/
HbListViewItem &HbListViewItem::operator=(const HbListViewItem &source)
{
    Q_D( HbListViewItem );
    *d = *source.d_func();
    return *this;
}


/*!
    Creates a new item.
*/
HbAbstractViewItem *HbListViewItem::createItem()
{
    return new HbListViewItem(*this);
}

/*!
    \reimp
*/
int HbListViewItem::type() const
{
    return HbListViewItem::Type;
}

/*!
    Updates child graphics items to represent current state and content.
*/
void HbListViewItem::updateChildItems()
{
    Q_D( HbListViewItem );

    // DisplayRoles
    QVariant displayRole = d->mIndex.data(Qt::DisplayRole);
    QStringList stringList;
    if (displayRole.isValid()) {
        if (displayRole.type() == QVariant::String) {
            stringList.append(displayRole.toString());
        } else if (displayRole.type() == QVariant::StringList) {
            stringList = displayRole.toStringList();
        } else if (displayRole.canConvert<QString>()) {
            stringList.append(displayRole.toString());
        } else if (displayRole.canConvert<QStringList>()) {
            stringList = displayRole.toStringList();
        }
    }

    // if internal list is bigger than the new one, remove extra items
    while (d->mDisplayRoleTextItems.count() > stringList.count()) {
        d->mItemsChanged = true;
        delete d->mDisplayRoleTextItems.at(d->mDisplayRoleTextItems.count() - 1);
        d->mDisplayRoleTextItems.removeLast();
    }

    // create/delete primitives
    for (int i = 0; i < stringList.count(); ++i) {
        d->setDisplayRole(stringList.at(i), i);
    }

    d->mStringList = stringList;

    // decorationroles
    QVariant decorationRole = d->mIndex.data(Qt::DecorationRole);
    QVariantList variantList;
    if (decorationRole.isValid()) {
        if (decorationRole.type() == QVariant::Icon 
            || decorationRole.userType() == qMetaTypeId<HbIcon>()) {
            variantList.append(decorationRole);
        } else if (decorationRole.type() == QVariant::List) { 
            variantList = decorationRole.toList();
        } else if (decorationRole.canConvert<QIcon>()
             || decorationRole.canConvert<HbIcon>()) {
            variantList.append(decorationRole);
        } else if (decorationRole.canConvert< QList<QVariant> >()) {
            variantList = decorationRole.toList();
        }
    }

    // if list is too big, remove extra items
    while (d->mDecorationRoleItems.count() > variantList.count()) {
        d->mItemsChanged = true;
        delete d->mDecorationRoleItems.at(d->mDecorationRoleItems.count() - 1);
        d->mDecorationRoleItems.removeLast();
    }

    // create/delete primitives
    for (int i = 0; i < variantList.count(); ++i) {
        d->setDecorationRole(variantList.at(i), i);
    }
    d->mDecorationList = variantList;

    HbAbstractViewItem::updateChildItems();
}

/*!
  Initializes the HbListViewItem primitive data. 
  
  This function calls HbWidgetBase::initPrimitiveData().
  \a primitiveData is data object, which is populated with data. \a primitive is the primitive.
  \a index is used to index data item in the internal store.
*/
void HbListViewItem::initPrimitiveData(HbStylePrimitiveData     *primitiveData, 
                                       const QGraphicsObject    *primitive,
                                       int                      index)
{
    Q_ASSERT_X(primitive && primitiveData, "HbListViewItem::initPrimitiveData" , "NULL data not permitted");
    HB_SDD(HbListViewItem);

    HbWidgetBase::initPrimitiveData(primitiveData, primitive);
    if (primitiveData->type == HbStylePrimitiveData::SPD_Text) {
        HbStyleTextPrimitiveData *textPrimitiveData = hbstyleprimitivedata_cast<HbStyleTextPrimitiveData*>(primitiveData);

         textPrimitiveData->text = d->mStringList.at(index);

        if (index == 1) {
            textPrimitiveData->textWrapping = Hb::TextNoWrap;
            textPrimitiveData->elideMode = Qt::ElideNone;
            if (d->isMultilineSupported()) {
                if (sd->mMinimumSecondaryTextRowCount != -1) {
                    // min & max secondary text row counts set by app
                    if (sd->mMaximumSecondaryTextRowCount != 1) {
                        textPrimitiveData->textWrapping = Hb::TextWordWrap;
                        textPrimitiveData->elideMode = Qt::ElideRight;
                    }
                    textPrimitiveData->minimumLines = sd->mMinimumSecondaryTextRowCount;
                    textPrimitiveData->maximumLines = sd->mMaximumSecondaryTextRowCount;
                }
                else {
                    // min & max secondary text row counts not set by app. Allow setting those from .css
                    // Needed when multilineSecondaryTextSupported changed from FALSE to TRUE and
                    // min & max secondary text row counts has not been set by app
                    HbWidgetBasePrivate *widgetBaseP = HbStylePrivate::widgetBasePrivate(
                        qobject_cast<HbWidgetBase*>(const_cast<QGraphicsObject*>(primitive)));
                    if (widgetBaseP) {
                        widgetBaseP->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextWrapMode, false);
                        widgetBaseP->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMin, false);
                        widgetBaseP->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMax, false);
                    }
                }
            } else {
                // min & max secondary text row counts must always be 1. They cannot be overridden by .css
                textPrimitiveData->minimumLines = 1;
                textPrimitiveData->maximumLines = 1;
            }
        } 
    } else if (primitiveData->type == HbStylePrimitiveData::SPD_RichText) {
        hbstyleprimitivedata_cast<HbStyleRichTextPrimitiveData*>(primitiveData)->text = d->mStringList.at(index);
    } else if (primitiveData->type == HbStylePrimitiveData::SPD_Icon) {
        if (d->mDecorationList.at(index).canConvert<HbIcon>()){
            hbstyleprimitivedata_cast<HbStyleIconPrimitiveData*>(primitiveData)->icon = d->mDecorationList.at(index).value<HbIcon>();
        } else if (d->mDecorationList.at(index).canConvert<QIcon>()){
            hbstyleprimitivedata_cast<HbStyleIconPrimitiveData*>(primitiveData)->icon = HbIcon(d->mDecorationList.at(index).value<QIcon>());
        }
    }
}


/*!
    \reimp
*/
void HbListViewItem::updatePrimitives()
{
    Q_D( HbListViewItem );

    int count = d->mStringList.count();
    for (int i = 0; i < count; ++i) {
        QGraphicsObject *item = d->mDisplayRoleTextItems.value(i);
        if (item) {
            if (d->displayPrimitive() == HbStyle::PT_TextItem) {
                HbStyleTextPrimitiveData textPrimitiveData;
                initPrimitiveData(&textPrimitiveData, item, i);
                style()->updatePrimitive(item,&textPrimitiveData,this);
            } else {
                HbStyleRichTextPrimitiveData richTextPrimitiveData;
                initPrimitiveData(&richTextPrimitiveData, item, i);
                style()->updatePrimitive(item,&richTextPrimitiveData,this);
            }
        }
    }

    count = d->mDecorationList.count();
    for (int i = 0; i < count; ++i) {
        QGraphicsObject *item = d->mDecorationRoleItems.value(i);
        if (item) {
            HbStyleIconPrimitiveData iconPrimitiveData;
            initPrimitiveData(&iconPrimitiveData, item, i);
            style()->updatePrimitive(item,&iconPrimitiveData,this);
        }
    }
    HbAbstractViewItem::updatePrimitives();
}

/*!
    Returns the current text format.

    The default text format is Qt::PlainText.

    \sa setTextFormat
*/
Qt::TextFormat HbListViewItem::textFormat() const
{
    HB_SDD(const HbListViewItem);

    return sd->mTextFormat;
}

/*!
    Sets the text format as a \a textFormat.  
    
    This method will change the used text format for all view items.

    \sa textFormat
*/
void HbListViewItem::setTextFormat(Qt::TextFormat textFormat)
{
    HB_SDD(HbListViewItem);

    if (textFormat != sd->mTextFormat) {
        sd->mTextFormat = textFormat;

        int count(sd->mCloneItems.count());
        for (int i = 0; i < count; ++i) {
            HbListViewItem *item = static_cast<HbListViewItem *>(sd->mCloneItems.at(i));
            HbListViewItemPrivate *itemPrivate = item->d_func();

            qDeleteAll(itemPrivate->mDisplayRoleTextItems);
            itemPrivate->mDisplayRoleTextItems.clear();

            item->d_func()->mItemsChanged = true;
            item->updateChildItems();
        }
    }
}

/*!
    Returns the current stretching style.

    The default stretching style is HbListViewItem::NoStretching.

    \sa setStretchingStyle
*/
HbListViewItem::StretchingStyle HbListViewItem::stretchingStyle() const
{
    HB_SDD(const HbListViewItem);
    return sd->mStretchingStyle;
}

/*!
    Sets the stretching style as a \a style.  
    
    This method will change the used stretching style for all view items in landscape mode.
    If view is in portrait mode, behaviour defined by \a style becomes effective only when view is changed to landscape mode.

    \sa stretchingStyle
*/
void HbListViewItem::setStretchingStyle(StretchingStyle style)
{
    HB_SDD(HbListViewItem);
    if (style != sd->mStretchingStyle) {
        sd->mStretchingStyle = style;
        if (d->isLandscape()) { 
            // secondary text multiline change!
            d->updateCloneItems(false);
            d->repolishCloneItems();
        }
    }
}

/*!
    Returns the current graphics size.

    The default graphics size is HbListViewItem::MediumIcon.

    \sa setGraphicsSize
*/
HbListViewItem::GraphicsSize HbListViewItem::graphicsSize() const
{
    HB_SDD(const HbListViewItem);
    return sd->mGraphicsSize;
}

/*!
    Sets the graphics size as a \a size.  
    
    This method will change the used graphics size for all view items.
    \note When graphics size HbListViewItem::Thumbnail or HbListViewItem::WideThumbnail is set,
    multiline secondary texts are not supported.

    \sa graphicsSize
    \sa setSecondaryTextRowCount
*/
void HbListViewItem::setGraphicsSize(GraphicsSize size)
{
    HB_SDD(HbListViewItem);
    if (size != sd->mGraphicsSize) {
        bool thumbnailChange = (size == Thumbnail) | (sd->mGraphicsSize == Thumbnail) 
                                | (size == WideThumbnail) | (sd->mGraphicsSize == WideThumbnail);
        sd->mGraphicsSize = size;
        if (   thumbnailChange
            && !d->isStretching()) {
            // secondary text multiline change!
            d->updateCloneItems(false);
        }
        d->repolishCloneItems();
    }
}

/*!
    Returns secondary text item maximum and minimum row counts. 
    The default maximum row count is 1 and minimum row count is 1.

    \note This function does not return values from .css although they were effective. 

    \sa setSecondaryTextRowCount
*/
void HbListViewItem::getSecondaryTextRowCount(int &minimum, int &maximum) const
{
    HB_SDD(const HbListViewItem);
    minimum = 1;
    maximum = 1;
    if (sd->mMinimumSecondaryTextRowCount != -1) {
        minimum = sd->mMinimumSecondaryTextRowCount;
        maximum = sd->mMaximumSecondaryTextRowCount;
    }
}

/*!
    Sets the secondary text item \a maximum row count and \a minimum row count.
    This method will change these values for all view items.

    Default value is 1 for both minimum and maximum line counts. 
    If default values are used wrapping mode, minimum and maximum line counts are read from .css file. 
    When non default value are set by this function, wrapping mode of text item is 
    automatically set to Hb::TextWordWrap.

    \note \a minimum and \a maximum are applied only when graphics size is not HbListViewItem::Thumbnail or 
    HbListViewItem::WideThumbnail and
    stretching style is not HbListViewItem::StretchLandscape in landscape mode. Otherwise only one row secondary text is used.

    \note If Qt::TextFormat is Qt::RichText, multiline secondary texts are not supported.

    \sa setTextFormat
    \sa secondaryTextRowCount
    \sa setStretchingStyle
    }sa setGraphicsSize
*/
void HbListViewItem::setSecondaryTextRowCount(int minimum, int maximum)
{
    HB_SDD(HbListViewItem);
    
    bool update = false;

    maximum = qMax(1, maximum);
    minimum = qBound(1, minimum, maximum);

    if (minimum != sd->mMinimumSecondaryTextRowCount) {
        sd->mMinimumSecondaryTextRowCount = minimum;
        update = true;
    }

    if (maximum != sd->mMaximumSecondaryTextRowCount) {
        sd->mMaximumSecondaryTextRowCount = maximum;
        update = true;
    }

    if (    update
        &&  d->isMultilineSupported()) {
        d->updateCloneItems(false);
    }
}

/*!
    \reimp
*/
void HbListViewItem::polish(HbStyleParameters& params)
{
    Q_D( HbListViewItem );
    setProperty("icon-1", (bool)d->mDecorationRoleItems.value(0));
    setProperty("icon-2", (bool)d->mDecorationRoleItems.value(1));

    setProperty("text-1", (bool)d->mDisplayRoleTextItems.value(0));
    setProperty("text-2", (bool)d->mDisplayRoleTextItems.value(1));
    setProperty("text-3", (bool)d->mDisplayRoleTextItems.value(2));

    if (itemView() && itemView()->selectionMode() != HbListView::NoSelection) {
        setProperty("selectionMode", true);
    } else {
        setProperty("selectionMode", false);
    }
    HbAbstractViewItem::polish(params);
}

#include "moc_hblistviewitem.cpp"

