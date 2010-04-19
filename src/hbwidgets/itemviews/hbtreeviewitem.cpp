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
#include "hbtreeviewitem_p.h"

#include "hbtreeviewitem.h"
#include "hbtreeview.h"
#include "hbnamespace.h"
#include "hbabstractitemview.h"
#include "hbstyle.h"
#include "hbstyleoptiontreeviewitem.h"
#include "hbabstractitemcontainer.h"

#include <QPersistentModelIndex>
#include <QVariant>
#include <QDebug>

/*!
    @alpha
    @hbwidgets
    \class HbTreeViewItem
    \brief The HbTreeViewItem class represents a single item in a hierarchical list.    

    The HbTreeViewItem class provides an item that is used by the HbTreeView class to
    visualize content within single model index. The item can only be used with HbTreeView
    or with objects derived from HbTreeView. By default the HbTreeViewItem supports
    all the same content that HbListViewItem supports. In addition to this it is able to 
    visualize parent items.

    \b Subclassing

    See HbListViewItem for commmon view item subclassing reference. 

*/

HbTreeViewItemPrivate::HbTreeViewItemPrivate(HbTreeViewItem *prototype) :
    HbListViewItemPrivate(prototype),
    mExpandItem(0),
    mExpanded(false)
{
}

HbTreeViewItemPrivate::HbTreeViewItemPrivate(const HbTreeViewItemPrivate &source) :
    HbListViewItemPrivate(source),
    mExpandItem(0),
    mExpanded(source.mExpanded)
{
}

HbTreeViewItemPrivate &HbTreeViewItemPrivate::operator=(const HbTreeViewItemPrivate &source)
{
    HbListViewItemPrivate::operator=(source);
    mExpandItem = 0;
    mExpanded = source.mExpanded;
    return *this;
}

HbTreeViewItemPrivate::~HbTreeViewItemPrivate()
{
}

int HbTreeViewItemPrivate::modelItemType() const
{
    const QAbstractItemModel *model = mIndex.model();

    if (model && model->hasChildren(mIndex)) {
        return Hb::ParentItem;
    } else {
        return HbListViewItemPrivate::modelItemType();
    }
}

QGraphicsItem *HbTreeViewItemPrivate::updateExpandItem()
{
    Q_Q(HbTreeViewItem);
    QGraphicsItem *item = mExpandItem;

    if (!item) {
        mItemsChanged = true;
        item = q->style()->createPrimitive(HbStyle::P_TreeViewItem_expandicon, q);
    }

    HbStyleOptionTreeViewItem styleOption;
    q->initStyleOption(&styleOption);

    q->style()->updatePrimitive(item, HbStyle::P_TreeViewItem_expandicon, &styleOption);

    return item;
}


/*!
    Constructs an tree view item with the given parent.
*/
HbTreeViewItem::HbTreeViewItem(QGraphicsItem *parent) : 
    HbListViewItem(*new HbTreeViewItemPrivate(this), parent)
{
    Q_D( HbTreeViewItem );
    d->q_ptr = this;
}

/*!
    Creates a separate graphics widget with same tree view item state as \a source.
*/
HbTreeViewItem::HbTreeViewItem(const HbTreeViewItem &source) :
    HbListViewItem(*new HbTreeViewItemPrivate(*source.d_func()), 0)
{
    Q_D( HbTreeViewItem );
    d->q_ptr = this;
}

/*!
    \internal
*/
HbTreeViewItem::HbTreeViewItem(HbTreeViewItemPrivate &dd, QGraphicsItem * parent) :
    HbListViewItem(dd, parent)
{
    Q_D( HbTreeViewItem );
    d->q_ptr = this;
}

/*!
    Destroys the tree view item.
*/
HbTreeViewItem::~HbTreeViewItem()
{
}

/*!
    Assigns the \a source tree view item to this tree view item and returns a reference to this item.
*/
HbTreeViewItem &HbTreeViewItem::operator=(const HbTreeViewItem &source)
{
    Q_D( HbTreeViewItem );
    *d = *source.d_func();
    return *this;
}


/*!
    Creates a new tree view item.
*/
HbAbstractViewItem *HbTreeViewItem::createItem()
{
    return new HbTreeViewItem(*this);
}

/*!
    \reimp
*/
void HbTreeViewItem::updateChildItems()
{
    Q_D(HbTreeViewItem);

    const QAbstractItemModel *model = d->mIndex.model();

    if (model && model->hasChildren(d->mIndex)) {
        d->mExpandItem = d->updateExpandItem();
    } else if (d->mExpandItem) {
        d->mItemsChanged = true;
        delete d->mExpandItem;
        d->mExpandItem = 0;
    }

    HbListViewItem::updateChildItems();
}

/*!
    \reimp
*/
void HbTreeViewItem::updatePrimitives()
{
    Q_D(HbTreeViewItem);
    if(d->mExpandItem) {
        d->updateExpandItem();
    }
    HbListViewItem::updatePrimitives();
}


/*!
    \reimp
*/
int HbTreeViewItem::type() const
{
    return HbTreeViewItem::Type;
}

/*!
    Sets the item to either collapse or expanded, depending on the value of \a expanded.

    \sa isExpanded
*/
void HbTreeViewItem::setExpanded(bool expanded)
{
    Q_D(HbTreeViewItem);
    HB_SD(HbAbstractViewItem);

    if (d->mExpanded != expanded) {
        d->mExpanded = expanded;

		if (sd->mItemView != 0) {
            HbTreeView *treeView = qobject_cast<HbTreeView *>(sd->mItemView);
            if (treeView) {
                treeView->setExpanded(this->modelIndex(), expanded);
            }
        }

        if (d->mExpandItem) {
            d->updateExpandItem();
        }
    }
}

/*!
    Returns true if the item is expanded; otherwise returns false.

    \sa setExpanded
*/
bool HbTreeViewItem::isExpanded() const
{
    Q_D(const HbTreeViewItem);
    return d->mExpanded;
}

/*!
    \reimp
*/
QMap<int,QVariant> HbTreeViewItem::state() const
{
    Q_D(const HbTreeViewItem);
    QMap<int,QVariant> state = HbListViewItem::state();
    
    state.insert(ExpansionKey, d->mExpanded);

    return state;    
}

/*!
    \reimp
*/
void HbTreeViewItem::setState(const QMap<int,QVariant> &state)
{
    Q_D(HbTreeViewItem);

    HbListViewItem::setState(state);

    if (state.contains(ExpansionKey)) {
        d->mExpanded = state.value(ExpansionKey).toBool();
    } else {
        d->mExpanded = false;
    }
}

/*!
    Initialize option with the values from this HbTreeViewItem. 

    This method is useful for subclasses when they need a HbStyleOptionTreeViewItem, 
    but don't want to fill in all the information themselves.
*/
void HbTreeViewItem::initStyleOption(HbStyleOptionTreeViewItem *option) const
{
    Q_D(const HbTreeViewItem);

    HbListViewItem::initStyleOption(option);

    option->expanded = d->mExpanded;
}

/*!
  Provides access to primitives of HbTreeViewItem.
  \param primitive is the type of the requested primitive. The available primitives are 
  \c P_TreeViewItem_expandicon
 */
QGraphicsItem *HbTreeViewItem::primitive(HbStyle::Primitive primitive) const
{
    Q_D(const HbTreeViewItem);
    if (primitive == HbStyle::P_TreeViewItem_expandicon) {
        return d->mExpandItem;
    } else {
        return HbListViewItem::primitive(primitive);
    }
}

#include "moc_hbtreeviewitem.cpp"

