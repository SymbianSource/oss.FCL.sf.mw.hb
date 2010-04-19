/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbCore module of the UI Extensions for Mobile.
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

#include "hbstyleoptionabstractviewitem.h"

#include <hbnamespace.h>

#include <QDebug>

/*!
    \class HbStyleOptionAbstractViewItem
    \brief HbStyleOptionAbstractViewItem has the style component for abstract view item primitives
*/

/*!
    \var HbStyleOptionAbstractViewItem::checkState

    This variable holds what is the checkstate of the item.
*/

/*!
    \var HbStyleOptionAbstractViewItem::background

    This variable holds content (if any) that is set as item's background.

    Default value is NULL variant.
*/

/*!
    \var HbStyleOptionAbstractViewItem::itemName
    @deprecated This variable will cease to exist in near future. HbStyleOptionListViewItem::itemNameIndex replaces this variable.

    This variable holds content item name of the primitive item required by css/xml layouting.
    
    \sa HbStyleOptionListViewItem::itemNameIndex
*/


/*!
    \var HbStyleOptionAbstractViewItem::modelItemType

    This variable holds what is the type of the model item that view item is representing.

    Default value is Hb::StandardItem.
*/

/*!
    \var HbStyleOptionAbstractViewItem::viewItemType

    This variable holds what is the type of the view item. This is value returned by QGraphicsItem::type().

    Default is Hb::ItemType_Last+1
*/

/*!
    \var HbStyleOptionAbstractViewItem::index

    The model index that the view item represents.
*/

/*!
    \var HbStyleOptionAbstractViewItem::singleSelectionMode

    This variable holds information whether HbAbstractItemView::SelectionMode mode is used. If it is used, set this variable On.

    Default value is false, which is fine when either any selection mode is not selected or multi or contiguous selection
    mode is in use.
*/

/*! \var HbStyleOptionAbstractViewItem::insidePopup
    Indicates whether widget and its children (classes derived from HbWidgetBase) are inside popup.
*/


HbStyleOptionAbstractViewItem::HbStyleOptionAbstractViewItem() :
    HbStyleOption(),
    modelItemType(Hb::StandardItem),
    viewItemType(Hb::ItemType_Last+1),
    singleSelectionMode(false),
    insidePopup(false)
{
    type = Type;
    version = Version;
}

HbStyleOptionAbstractViewItem::HbStyleOptionAbstractViewItem(const HbStyleOptionAbstractViewItem &other) :
    HbStyleOption(other),
    checkState(other.checkState),
    background(other.background),
    modelItemType(other.modelItemType),
    viewItemType(other.viewItemType),
    singleSelectionMode(other.singleSelectionMode),
    insidePopup(other.insidePopup)
{
    type = Type;
    version = Version;
}

HbStyleOptionAbstractViewItem::~HbStyleOptionAbstractViewItem()
{
}
