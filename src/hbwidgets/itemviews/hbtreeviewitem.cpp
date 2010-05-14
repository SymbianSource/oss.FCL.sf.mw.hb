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
#include "hbabstractitemview.h"
#include "hbabstractitemcontainer_p.h"

#include <hbnamespace.h>
#include <hbstyle.h>
#include <hbstyleoptiontreeviewitem_p.h>
#include <hbwidgetfeedback.h>
#include <hbtapgesture.h>
#include <hbeffect.h>

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
    HbListViewItemPrivate(prototype, new HbTreeViewItemShared),
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

void HbTreeViewItemPrivate::init()
{
    if (isPrototype()) {
        HbEffect::add("treeviewitem", "treeviewitem_expand", "expand");
        HbEffect::add("treeviewitem", "treeviewitem_collapse", "collapse");
    }
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

void HbTreeViewItemPrivate::updateExpandItem()
{
    Q_Q(HbTreeViewItem);

    HbStyleOptionTreeViewItem styleOption;
    q->initStyleOption(&styleOption);

    q->style()->updatePrimitive(mExpandItem, HbStyle::P_TreeViewItem_expandicon, &styleOption);
}

void HbTreeViewItemPrivate::tapTriggered(QGestureEvent *event)
{
    Q_Q(HbTreeViewItem);

    HbTapGesture *gesture = static_cast<HbTapGesture *>(event->gesture(Qt::TapGesture));

    if (gesture->state() == Qt::GestureFinished 
        && gesture->tapStyleHint() == HbTapGesture::Tap) {

        QPointF position = event->mapToGraphicsScene(gesture->hotSpot());
        position = q->mapFromScene(position);

        bool inSelectionArea = false;
        if (mSharedData->mItemView->selectionMode() == HbAbstractItemView::SingleSelection) {
            inSelectionArea = q->selectionAreaContains(position, HbAbstractViewItem::SingleSelection);
        } else if (mSharedData->mItemView->selectionMode() == HbAbstractItemView::MultiSelection) {
            inSelectionArea = q->selectionAreaContains(position, HbAbstractViewItem::MultiSelection);
        }

        Hb::InteractionModifiers modifiers = 0;
        if (mExpandItem 
            && mSharedData->mItemView
            && (mSharedData->mItemView->selectionMode() == HbAbstractItemView::SingleSelection
                || !inSelectionArea)) {
            if (q->isExpanded()) {
                modifiers |= Hb::ModifierExpandedItem;
                q->setExpanded(false);
            } else {
                modifiers |= Hb::ModifierCollapsedItem;
                q->setExpanded(true);
            }
        }

        HbWidgetFeedback::triggered(q, Hb::InstantClicked, modifiers);
        setPressed(false, true);

        emit q->activated(position);
        emit q->released(position);
        revealItem();

        event->accept();
    } else {
        HbListViewItemPrivate::tapTriggered(event);
    }

}

/*!
    Constructs an tree view item with the given parent.
*/
HbTreeViewItem::HbTreeViewItem(QGraphicsItem *parent) : 
    HbListViewItem(*new HbTreeViewItemPrivate(this), parent)
{
    Q_D( HbTreeViewItem );
    d->q_ptr = this;

    d->init();
}

/*!
    Creates a separate graphics widget with same tree view item state as \a source.
*/
HbTreeViewItem::HbTreeViewItem(const HbTreeViewItem &source) :
    HbListViewItem(*new HbTreeViewItemPrivate(*source.d_func()), 0)
{
    Q_D( HbTreeViewItem );
    d->q_ptr = this;

    d->init();
}

/*!
    \internal
*/
HbTreeViewItem::HbTreeViewItem(HbTreeViewItemPrivate &dd, QGraphicsItem * parent) :
    HbListViewItem(dd, parent)
{
    Q_D( HbTreeViewItem );
    d->q_ptr = this;

    d->init();
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
    HB_SDD(HbTreeViewItem);

    const QAbstractItemModel *model = d->mIndex.model();

    if (model && model->hasChildren(d->mIndex) && sd->mUserExpandable) {
        if (!d->mExpandItem) {
            d->mExpandItem = style()->createPrimitive(HbStyle::P_TreeViewItem_expandicon, this);
            d->mItemsChanged = true;
        }
    } else {
        if (d->mExpandItem) {
            d->mItemsChanged = true;
            delete d->mExpandItem;
            d->mExpandItem = 0;
        }
    }

    HbListViewItem::updateChildItems();
}

/*!
    \reimp
*/
void HbTreeViewItem::updatePrimitives()
{
    Q_D(HbTreeViewItem);
    if (d->mExpandItem) {
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
QHash<QString, QVariant> HbTreeViewItem::transientState() const
{
    Q_D(const HbTreeViewItem);
    QHash<QString, QVariant> state = HbListViewItem::transientState();
    if (d->mExpanded) {
        state.insert("expanded", d->mExpanded);
    }
    return state;
}


/*!
    \reimp
*/
void HbTreeViewItem::setTransientState(const QHash<QString, QVariant> &state)
{
    Q_D(HbTreeViewItem);

    HbListViewItem::setTransientState(state);
    d->mExpanded = state.value("expanded").toBool();
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
    Sets items either expandable or non-expandable by the user, depending on the value of \a expandable.

    This method will change the user expandable value for all view items.

    \sa isUserExpandable
*/
void HbTreeViewItem::setUserExpandable(bool expandable)
{
    HB_SDD(HbTreeViewItem);
    if (sd->mUserExpandable != expandable) {
        sd->mUserExpandable = expandable;
        d->updateCloneItems(true);
    }
}

/*!
    Returns true if the items are expandable by the user; otherwise returns false.

    \sa setUserExpandable
*/
bool HbTreeViewItem::isUserExpandable() const
{
    HB_SDD(const HbTreeViewItem);
    return sd->mUserExpandable;
}

/*!
    \reimp
    
    In the base class the multiselection mode selection area is the whole item. In HbTreeView this is not
    possible because of the expansion icon. For the HbTreeView the selection area in multiselection mode is 
    defined by the primitive HbStyle::P_ItemViewItem_touchmultiselection
*/
bool HbTreeViewItem::selectionAreaContains(const QPointF &position, SelectionAreaType selectionAreaType) const
{
    Q_D(const HbTreeViewItem);
    if (   selectionAreaType == HbAbstractViewItem::MultiSelection 
        || selectionAreaType == HbAbstractViewItem::ContiguousSelection) {

        if(     d->mMultiSelectionTouchArea 
            &&  !d->mMultiSelectionTouchArea->boundingRect().isEmpty()) {
            return d->mMultiSelectionTouchArea->boundingRect().contains(mapToItem(d->mMultiSelectionTouchArea,position));
        } else if (d->mSelectionItem) {
            return d->mSelectionItem->boundingRect().contains(mapToItem(d->mSelectionItem, position));
        } else {
            return false;
        }
    }
    return HbAbstractViewItem::selectionAreaContains(position, selectionAreaType);
}

#include "moc_hbtreeviewitem.cpp"

