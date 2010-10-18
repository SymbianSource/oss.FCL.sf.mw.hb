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

#include "hbgridviewitem_p.h"
#include <hbgridviewitem.h>
#include "hbgridview_p.h"

#include <hbstyletextprimitivedata.h>
#include <hbstylerichtextprimitivedata.h>
#include <hbstyleiconprimitivedata.h>
#include <QDebug>

/*!
 @beta
 @hbwidgets
 \class HbGridViewItem
 \brief HbGridViewItem class represents a single item in a grid.    

 The HbGridViewItem class provides a view item that is used by HbGridView class to visualize content within 
 a single model index. 
 
 HbGridViewItem supports Hb::StandardItem in Hb::ItemTypeRole role of data model.

 Following item data roles are supported by HbGridViewItem
 \li QString in Qt::DisplayRole of the data model
 \li QIcon or HbIcon in Qt::DecoratorRole role. QIcon is supported only for compatibility reasons. 
 If QIcon is used the limitations described in the HbIcon::HbIcon(const QIcon &icon) apply.

 Handling Qt::BackgroundRole item data role takes place in base class HbAbstractViewItem.

 \b Subclassing

 When subclassing HbGridViewItem, you must provide implementations of the createItem() and updateChildItems() functions.

 To support multiple grid view items within a single grid view, you must also provide an implementation of canSetModelIndex().

 If the derived grid view item has transient state information that would have no meaning if stored within the model index (for example child item cursor
 position selection areas etc.) the view item can use the grid view's internal state model to store this information.To use this feature 
 implement the state() and setState() functions in the derived class.
 
 See also HbGridView, HbAbstractItemView, HbAbstractViewItem

 \primitives
 \primitive{icon} HbIconItem with item name "icon" representing the icon in the HbGridViewItem. 
 \primitive{text} HbTextItem with item name "text" representing the text in the HbGridViewItem. 

 \sa HbAbstractViewItem

 */

/*!
 Constructs a grid view item with the given parent.
 */
HbGridViewItem::HbGridViewItem(QGraphicsItem *parent) :
    HbAbstractViewItem(*new HbGridViewItemPrivate(this), parent)
{
    Q_D( HbGridViewItem );
    d->q_ptr = this;

    d->init();
}

/*!
 Constructor
 */
HbGridViewItem::HbGridViewItem(HbGridViewItemPrivate &dd, QGraphicsItem *parent) :
    HbAbstractViewItem(dd, parent)
{
    Q_D( HbGridViewItem );
    d->q_ptr = this;

    d->init();
}

/*!
 Copy constructor
 */
HbGridViewItem::HbGridViewItem(const HbGridViewItem &source) :
    HbAbstractViewItem(*new HbGridViewItemPrivate(*source.d_func()),
            source.parentItem())
{
    Q_D( HbGridViewItem );
    d->q_ptr = this;

    d->init();
}

/*!
 Destroys the grid view item.
 */
HbGridViewItem::~HbGridViewItem()
{
}

/*!
 Creates a new item.
 */
HbAbstractViewItem* HbGridViewItem::createItem()
{
    return new HbGridViewItem(*this);
}

/*!
 Assigns the \a source grid view item to this grid view item and returns a reference to this item.
*/
HbGridViewItem &HbGridViewItem::operator=(const HbGridViewItem &source)
{
    Q_D(HbGridViewItem);
    *d = *source.d_func();
    return *this;
}

/*!
    \reimp
 */
int HbGridViewItem::type() const
{
    return Hb::ItemType_GridViewItem;
}

/*!
 Updates child graphics items to represent the current state and content.
 */
void HbGridViewItem::updateChildItems()
{
    Q_D(HbGridViewItem);

    if (d->mIndex.isValid()) {
        HbGridView *view = qobject_cast<HbGridView*>(itemView());
        if (!view) {
            return;
        }
        HbGridViewPrivate *viewPrivate = HbGridViewPrivate::d_ptr(view);
        d->updateTextItem(*viewPrivate);
        d->updateIconItem(*viewPrivate);
    }
    HbAbstractViewItem::updateChildItems();
}


/*!
  Initializes the HbGridViewItem primitive data. 
  
  This function calls HbWidgetBase::initPrimitiveData().
  \a primitiveData is data object, which is populated with data. \a primitive is the primitive.
*/
void HbGridViewItem::initPrimitiveData(HbStylePrimitiveData     *primitiveData, 
                                       const QGraphicsObject    *primitive)
{
    Q_ASSERT_X(primitive && primitiveData, "HbGridViewItem::initPrimitiveData" , "NULL data not permitted");
    Q_D(HbGridViewItem);

    HbWidgetBase::initPrimitiveData(primitiveData, primitive);
    if (primitiveData->type == HbStylePrimitiveData::SPD_Text) {
        HbStyleTextPrimitiveData *textPrimitiveData = hbstyleprimitivedata_cast<HbStyleTextPrimitiveData*>(primitiveData);
        textPrimitiveData->text = d->mText;

    } else if (primitiveData->type == HbStylePrimitiveData::SPD_Icon) {
        HbStyleIconPrimitiveData *iconPrimitiveData = hbstyleprimitivedata_cast<HbStyleIconPrimitiveData*>(primitiveData);
        iconPrimitiveData->icon = d->mIcon;
    }
}


/*!
 \reimp
 */
void HbGridViewItem::updatePrimitives()
{
    Q_D( HbGridViewItem );
    if (d->mTextItem || d->mIconItem) {
        if (d->mTextItem) {
            HbStyleTextPrimitiveData textPrimitiveData;
            initPrimitiveData(&textPrimitiveData, d->mTextItem);
            style()->updatePrimitive(d->mTextItem, &textPrimitiveData,this);
        }
        if (d->mIconItem) {
            HbStyleIconPrimitiveData iconPrimitiveData;
            initPrimitiveData(&iconPrimitiveData, d->mIconItem);
            style()->updatePrimitive(d->mIconItem, &iconPrimitiveData,this);
        }
    }
    HbAbstractViewItem::updatePrimitives();
}

/*!
 \reimp
 */
bool HbGridViewItem::selectionAreaContains(const QPointF &position, 
                                       SelectionAreaType selectionAreaType) const
{
    if (selectionAreaType == ContiguousSelection ) {
        return false;
    } 
    return HbAbstractViewItem::selectionAreaContains(position, selectionAreaType);
}

/*!
 \reimp
 */
QPainterPath HbGridViewItem::shape() const
{
    // This is called when finding collinding items
    // with QGraphicScene::collidingItems()
    QPainterPath path;
    QRectF rect(boundingRect());
    path.addRect(rect);
    return path;
}

/*!
    \reimp
*/
void HbGridViewItem::polish(HbStyleParameters& params)
{
    Q_D(HbGridViewItem);

    setProperty("icon", (bool)(d->mIconItem ? true : false));
    setProperty("text", (bool)(d->mTextItem ? true : false));

    HbAbstractViewItem::polish(params);
}

#include "moc_hbgridviewitem.cpp"

