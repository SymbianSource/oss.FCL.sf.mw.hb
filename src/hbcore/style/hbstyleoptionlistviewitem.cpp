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

#include "hbstyleoptionlistviewitem_p.h"

/*
    \class HbStyleOptionListViewItem
    \brief HbStyleOptionListViewItem has the style component for list view item primitives
*/


/*
    \var HbStyleOptionListViewItem::content

    This variable holds content (if any) that is set as item's data. 
    The content is always a single item even if respective role in the model 
    would be a list. 

    Default value is NULL variant.

    \sa HbStyleOptionListViewItem::primaryText
*/

/*
    \var HbStyleOptionListViewItem::role

    Defines role of an model item, from which content is read.
    Default value Qt::DisplayRole.

    \sa HbStyleOptionListViewItem::content
*/

/*
    \var HbStyleOptionListViewItem::index

    This variable holds index of the primitive item required by css/xml layouting, when
    there may be several items with same name prefix e.g. "text-2".

    The itemNameIndex value is incremented by 1 before using it in HbStyle. 
    Thus itemNameIndex value 0 produces item name "text-1".

    When index is 1, style option includes secondary text.

    Default value is 0.
*/

/*
    \var HbStyleOptionListViewItem::minimumLines

    This variable holds minimum count of lines reserved for secondary text (i.e. when index is 1).
    Minimum row count for secondary text can be set using HbListViewItem::setSecondaryTextRowCount(). 
    If minimum and maximum row counts are not set by application, default value is used.

    For other texts default value is always used. When default value is set, minimum count of lines are read from .css file during layouting the view item.
    If multilineSecondaryTextSupported is false and text is secondary text, minimum and maximum line counts are forced to 1. 
    These values cannot be overridden from .css during layouting.

    Default value is -1. 

    \sa HbStyleOptionListViewItem::index
    \sa HbStyleOptionListViewItem::multilineSecondaryTextSupported
    \sa HbListViewItem::setSecondaryTextRowCount()
*/

/*
    \var HbStyleOptionListViewItem::maximumLines

    This variable holds minimum count of lines reserved for secondary text (i.e. when index is 1).
    Maximum row count for secondary text can be set using HbListViewItem::setSecondaryTextRowCount().
    If minimum and maximum row counts are not set by application, default value is used.

    For other texts default value is always used. When default value is set, maximum count of lines are read from .css file during layouting the view item.
    If multilineSecondaryTextSupported is false and text is secondary text, minimum and maximum line counts are forced to 1. 
    Then these values cannot be overridden from .css during layouting.

    Default value is -1. 

    \sa HbStyleOptionListViewItem::index
    \sa HbStyleOptionListViewItem::multilineSecondaryTextSupported
    \sa HbListViewItem::setSecondaryTextRowCount()
*/

/*
    \var HbStyleOptionListViewItem::multilineSecondaryTextSupported

    This variable holds whether multine line secondary text is supported. 

    Default value is true. 

    \sa HbStyleOptionListViewItem::minimumLines
    \sa HbStyleOptionListViewItem::maximumLines
    \sa HbListViewItem::setSecondaryTextRowCount()
*/

HbStyleOptionListViewItem::HbStyleOptionListViewItem() :
    HbStyleOptionAbstractViewItem(),
    role(Qt::DisplayRole),
    index(0),
    minimumLines(-1),
    maximumLines(-1),
    multilineSecondaryTextSupported(true)
{
    type = Type;
    version = Version;
}

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
