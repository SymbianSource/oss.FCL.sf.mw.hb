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

#include "hbstyleoptionlistviewitem.h"

/*!
    \class HbStyleOptionListViewItem
    \brief HbStyleOptionListViewItem has the style component for list view item primitives
*/


/*!
    \var HbStyleOptionListViewItem::content

    This variable holds content (if any) that is set as item's data. 
    The content is always a single item even if respective role in the model 
    would be a list. 

    Default value is NULL variant.

    \sa HbStyleOptionListViewItem::primaryText
*/

/*!
    \var HbStyleOptionListViewItem::role

    Defines role of an model item, from which content is read.
    Default value Qt::DisplayRole.

    \sa HbStyleOptionListViewItem::content
*/

/*!
    \var HbStyleOptionListViewItem::index

    This variable holds index of the primitive item required by css/xml layouting, when
    there may be several items with same name prefix e.g. "text-2".

    The itemNameIndex value is incremented by 1 before using it in HbStyle. 
    Thus itemNameIndex value 0 produces item name "text-1".

    Default value is 0.
*/

/*!
    \var HbStyleOptionListViewItem::minimumLines

    This variable holds minimum count of lines reserved for text.
    Minimum row count for secondary text in middle column can be set using HbListViewItem::setSecondaryTextRowCount(). 
    For other texts default value is always used.

    Default value is 1. 

    \sa HbListViewItem::setSecondaryTextRowCount()
*/

/*!
    \var HbStyleOptionListViewItem::maximumLines

    This variable holds maximum count of lines reserved for secondary text.
    Maximum row count for secondary text in middle column can be set using HbListViewItem::setSecondaryTextRowCount().
    For other texts default value is always used.

    Default value is 1.

    \sa HbListViewItem::setSecondaryTextRowCount()
*/



/*!

    \deprecated HbStyleOptionListViewItem::HbStyleOptionListViewItem()
    is deprecated. Styleoptions will not be public.

*/
HbStyleOptionListViewItem::HbStyleOptionListViewItem() :
    HbStyleOptionAbstractViewItem(),
    role(Qt::DisplayRole),
    index(0),
    minimumLines(1),
    maximumLines(1)

{
    type = Type;
    version = Version;
}


/*!

    \deprecated HbStyleOptionListViewItem::HbStyleOptionListViewItem(const HbStyleOptionListViewItem&)
    is deprecated. Styleoptions will not be public.

*/
HbStyleOptionListViewItem::HbStyleOptionListViewItem(const HbStyleOptionListViewItem &other) :
    HbStyleOptionAbstractViewItem(other),
    content(other.content),
    role(Qt::DisplayRole),
    index(other.index),
    minimumLines(other.minimumLines),
    maximumLines(other.maximumLines)
{
    type = Type;
    version = Version;
}

HbStyleOptionListViewItem::~HbStyleOptionListViewItem()
{
}
