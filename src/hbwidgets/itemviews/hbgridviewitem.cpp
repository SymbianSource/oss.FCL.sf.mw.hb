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
#include <hbstyleoptiongridviewitem.h>
#include <QDebug>

/*!
 @beta
 @hbwidgets
 \class HbGridViewItem
 \brief HbGridViewItem class represents a single item in a grid.    

 The HbGridViewItem class provides a view item that is used by HbGridView class to visualize content within 
 a single model index. By default HbGridViewItem supports a QString that is stored into the Qt::DisplayRole role 
 and a QIcon or HbIcon that is stored into the Qt::DecoratorRole role within the index. QBrush, HbIcon or HbFrameBackground can be used
 as a background by storing them into the Qt::BackgroundRole role. If Qt::BackgroundRole is empty, the default item background
 is used.

 \b Subclassing

 When subclassing HbGridViewItem, you must provide implementations of the createItem() and updateChildItems() functions.

 To support multiple grid view items within a single grid view, you must also provide an implementation of canSetModelIndex().

 If the derived grid view item has transient state information that would have no meaning if stored within the model index (for example child item cursor
 position selection areas etc.) the view item can use the grid view's internal state model to store this information.To use this feature 
 implement the state() and setState() functions in the derived class.
 
 See also HbGridView, HbAbstractItemView, HbAbstractViewItem, HbStyleOptionAbstractViewItem
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
 \reimp
 */
void HbGridViewItem::updatePrimitives()
{
    Q_D( HbGridViewItem );
    if (d->mTextItem || d->mIconItem) {
        HbStyleOptionGridViewItem styleOption;
        initStyleOption(&styleOption);

        if (d->mTextItem) {
            style()->updatePrimitive(d->mTextItem, HbStyle::P_GridViewItem_text,
                    &styleOption);
        }
        if (d->mIconItem) {
            style()->updatePrimitive(d->mIconItem, HbStyle::P_GridViewItem_icon,
                    &styleOption);
        }
    }
    HbAbstractViewItem::updatePrimitives();
}

/*!
    Populates a style option object for this widget based on its current state, and stores the output in \a option.
*/
void HbGridViewItem::initStyleOption(HbStyleOptionGridViewItem *option) const
{
    Q_D( const HbGridViewItem );

    HbAbstractViewItem::initStyleOption(option);
    option->icon = d->mIcon;
    option->text = d->mText;
}

/*!
  Provides access to primitives of HbGridViewItem. 
  \param primitive is the type of the requested primitive. The available primitives are 
  \c P_GridViewItem_text, \c P_GridViewItem_icon, \c P_ItemViewItem_checkbox and
  \c P_GridViewItem_background.
 */
QGraphicsItem * HbGridViewItem::primitive(HbStyle::Primitive primitive) const
{
    Q_D( const HbGridViewItem );

    if (primitive == HbStyle::P_GridViewItem_text) {
        return d->mTextItem;
    }
    else if (primitive == HbStyle::P_GridViewItem_icon) {
        return d->mIconItem;
    }

    return HbAbstractViewItem::primitive(primitive);
}

/*!
    \deprecated QRectF HbGridViewItem::contiguousSelectionArea() const
        is deprecated.

    \reimp
 */
QRectF HbGridViewItem::contiguousSelectionArea() const
{
    qWarning("QRectF HbGridViewItem::contiguousSelectionArea() const is deprecated!");

    return QRect();
}

/*!
 \reimp
 */
bool HbGridViewItem::selectionAreaContains(const QPointF &scenePosition) const
{
    HB_SDD(const HbAbstractViewItem);
    if (    sd->mItemView
        &&  sd->mItemView->selectionMode() == HbAbstractItemView::ContiguousSelection) {
        return false;
    } 
    return HbAbstractViewItem::selectionAreaContains(scenePosition);
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

