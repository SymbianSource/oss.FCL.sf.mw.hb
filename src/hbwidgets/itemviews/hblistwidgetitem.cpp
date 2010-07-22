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

#include "hblistwidgetitem.h"
#include "hblistwidgetitem_p.h"

#include "hblistmodel_p.h"
#include <hblistwidget.h>
#include <hbicon.h>

#include <QDebug>

/*!
    @beta
    @hbwidgets
    \class HbListWidgetItem
    \brief The HbListWidgetItem class provides an item for use with the HbListWidget class.

    As HbListWidget by itself supports the use of very basic items in simple lists, HbListWidgetItem provides together with HbListWidget a convenience API for adding, modifying, and removing more versatile items in simple lists. So, if you want to have list items consisting of more than an icon and a text, create HbListWidgetItem objects and add them to an HbListWidget object. The list item of HbListWidgetItem class consists of three columns and its features are as follows:
    - The left and right columns have one row and they can contain an icon.
    - The middle column can have two rows and both rows can contain text.
    - At least the primary text is required.
    
    \section _usecases_hblistwidgetitem Using the HbListWidgetItem class
    
    \subsection _uc_hblistwidgetitem_001 Creating a list item and adding two icons and two rows of text into it.

    The following code snippet creates a list item with icons in the left and right columns and two rows of text in the middle column.

    \snippet{unittest_hblistwidgetitem.cpp,1}

    \sa HbListWidget
*/

/*!
    \fn QBrush HbListWidgetItem::background() const

    Returns the background of the item.

    \sa setBackground
*/

/*!
    \fn void HbListWidgetItem::setBackground(const QVariant &background)

    Sets the background of the item to the given \a background. Supported types are as follows:
    - HbIcon
    - QBrush
    - any type that can be converted to HbIcon or QBrush

    \sa background
*/

/*!
    Constructs an empty list widget item of the given \a type. If no type is specified then Hb::StandardItem is used.
    
    
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
    Returns
    - the item's application-specific data of the given \a role if the data exists.
    - an invalid QVariant if there is no data of the given \a role for the item.
 */
QVariant HbListWidgetItem::data(int role) const
{
    return d->data(role);
}

/*!
    Sets the item's data for the given \a role to the given \a value. It can be used to store application-specific data into an item.

    Specialized functions in section 'see also' are recommended setting user data. 
    \param role - Role for the data of the item.
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
    Sets the item's primary text to the given \a text. 

    \sa text
*/
void HbListWidgetItem::setText(const QString &text)
{
    d->setData(text, Qt::DisplayRole, 0);
    if (d->mModel)
        d->mModel->itemChanged(this);
}

/*!
    Returns the secondary text of the item.

    \sa setSecondaryText
*/
QString HbListWidgetItem::secondaryText() const
{
    return d->data(Qt::DisplayRole, 1).toString();
}

/*!
    Sets the item's secondary text to the given \a text.

    \sa secondaryText
*/
void HbListWidgetItem::setSecondaryText(const QString &text)
{
    d->setData(text, Qt::DisplayRole, 1);
    if (d->mModel)
        d->mModel->itemChanged(this);
}

/*!
    Returns the item's icon. 

    \sa setIcon
*/
HbIcon HbListWidgetItem::icon() const
{
    return d->data(Qt::DecorationRole, 0).value<HbIcon>();
}

/*!
    Sets the item's icon to the given \a icon.

    \sa icon
*/
void HbListWidgetItem::setIcon(const HbIcon &icon)
{
    d->setData(icon, Qt::DecorationRole, 0);
    if (d->mModel)
        d->mModel->itemChanged(this);
}

/*!
    Returns the item's secondary icon. 

    \sa setSecondaryIcon
*/
HbIcon HbListWidgetItem::secondaryIcon() const
{
    return d->data(Qt::DecorationRole, 1).value<HbIcon>();
}

/*!
    Sets the item's secondary icon to the given \a icon.

    \sa secondaryIcon
*/
void HbListWidgetItem::setSecondaryIcon(const HbIcon &icon)
{
    d->setData(icon, Qt::DecorationRole, 1);
    if (d->mModel)
        d->mModel->itemChanged(this);
}

/*!
    Enables the user interaction with item. 
    
    \param enabled - If the value is \c true, the user can interact with the item. If the value is \c false, the user cannot interact with the item.
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
