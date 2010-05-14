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
#include <hblistwidgetitem.h>
#include "hblistwidgetitem_p.h"

#include "hblistmodel_p.h"
#include <hblistwidget.h>
#include <hbicon.h>

#include <QDebug>

/*!
    @beta
    @hbwidgets
    \class HbListWidgetItem
    \brief HbListWidgetItem represents a list item. It is part of convenience list API together with HbListWidget. 

    As this is a convenience API it is supposed to be easy to use. For that reason complicated list items are not possible
    to be created. HbListWidgetItem supports at maximum two text items and two icon items. 
    Either primary text or primary icon must always be provided. Icons are shown only in default size.

    A view item created from this item consists of three columns at maximum.
    - First column contains primary icon
    - Middle column has one or two rows to show text(s)
    - Last column contains secondary icon
    - Any column can be empty 

    The following code snippet presents how to create an list item with icons in the left 
    and right columns and two rows of text in the middle column.

    \snippet{unittest_hblistwidgetitem.cpp,1}

    \sa HbListWidget

*/

/*!
    \fn QBrush HbListWidgetItem::background() const

    Returns item's background.

    \sa setBackground
*/

/*!
    \fn void HbListWidgetItem::setBackground(const QVariant &background)

    Sets the item's background to the specified \a background. Supported
    background types are HbIcon and QBrush types plus any type that can be
    converted to those types.

    \sa background
*/

/*!
    Constructs an empty list widget item with the given \a type. 
    
    Default type for item is Hb::StandardItem.
*/
HbListWidgetItem::HbListWidgetItem(int type) :
    d(new HbListWidgetItemPrivate(this))
{
    if (type != Hb::StandardItem) {
        setData(type, Hb::ItemTypeRole); 

        if (type == Hb::SeparatorItem) {
            setEnabled(false);
        }
    }

    setText(QString(""));
}

/*!
    Destructor
 */
HbListWidgetItem::~HbListWidgetItem()
{
    delete d;
}


/*!
    Returns the item's application-specific data for the given role, 
    or an invalid QVariant if there is no data for the role.
 */
QVariant HbListWidgetItem::data(int role) const
{
    return d->data(role);
}

/*!
    Sets the item's data for the given role to the specified value.
    Can be used to store application-specific data in an item. 

    Specialised functions in section 'see also' are recommended setting user data. 

    \sa setText()
    \sa setSecondaryText()
    \sa setIcon()
    \sa setSecondaryIcon()
*/
void HbListWidgetItem::setData(const QVariant &value, int role)
{
    d->setData(value,role);
    if (d->mModel)
        d->mModel->itemChanged(this);
}

/*!
    Returns item's primary text.

    \sa setText
*/
QString HbListWidgetItem::text() const
{
    return d->data(Qt::DisplayRole, 0).toString();
}


/*!
    Sets the item's primary text to the specified \a text. 

    \sa text
*/
void HbListWidgetItem::setText(const QString &text)
{
    d->setData(text, Qt::DisplayRole, 0);
    if (d->mModel)
        d->mModel->itemChanged(this);
}

/*!
    Returns item's secondary text. 

    \sa setSecondaryText
*/
QString HbListWidgetItem::secondaryText() const
{
    return d->data(Qt::DisplayRole, 1).toString();
}

/*!
    Sets the item's secondary text to the specified \a text.

    \sa secondaryText
*/
void HbListWidgetItem::setSecondaryText(const QString &text)
{
    d->setData(text, Qt::DisplayRole, 1);
    if (d->mModel)
        d->mModel->itemChanged(this);
}

/*!
    Returns item's icon. 

    \sa setIcon
*/
HbIcon HbListWidgetItem::icon() const
{
    return d->data(Qt::DecorationRole, 0).value<HbIcon>();
}

/*!
    Sets the item's icon to the specified \a icon.

    \sa icon
*/
void HbListWidgetItem::setIcon(const HbIcon &icon)
{
    d->setData(icon, Qt::DecorationRole, 0);
    if (d->mModel)
        d->mModel->itemChanged(this);
}

/*!
    Returns item's secondary icon. 

    \sa setSecondaryIcon
*/
HbIcon HbListWidgetItem::secondaryIcon() const
{
    return d->data(Qt::DecorationRole, 1).value<HbIcon>();
}

/*!
    Sets the item's secondary icon to the specified \a icon.

    \sa secondaryIcon
*/
void HbListWidgetItem::setSecondaryIcon(const HbIcon &icon)
{
    d->setData(icon, Qt::DecorationRole, 1);
    if (d->mModel)
        d->mModel->itemChanged(this);
}

/*!
    Sets whether the item is enabled. 
    
    If enabled is true, the item is \a enabled, meaning that the user can interact with the item; if \a enabled is false, 
    the user cannot interact with the item.
*/
void HbListWidgetItem::setEnabled(bool enabled)
{
    if (enabled != (bool)(d->flags & Qt::ItemIsEnabled)) {
        if (enabled) {
            d->flags |= Qt::ItemIsEnabled;
        } else {
            d->flags &= ~Qt::ItemIsEnabled;
        }
        
        if (d->mModel)
            d->mModel->itemChanged(this);
    }
}

/*!
    Returns true if the item is enabled; otherwise returns false.
*/
bool HbListWidgetItem::isEnabled() const
{
    return d->flags & Qt::ItemIsEnabled;
}

/*!
    Returns the type of item.
*/
int HbListWidgetItem::type() const
{
    return data(Hb::ItemTypeRole).toInt(); 
}
