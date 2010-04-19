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
#include "hblistwidgetitem_p.h"
#include "hblistmodel_p.h"

#include <hblistwidgetitem.h>

#include <QVariant>


/*
    HbListWidgetItemPrivate in a internal class, which implements data container 
    for HblistWidgetItems and HblistWidget. 
*/
HbListWidgetItemPrivate::HbListWidgetItemPrivate(
        HbListWidgetItem *itemWidget) :
    mItem(itemWidget),
    mModel(0),
    id(0),
    flags(Qt::ItemIsSelectable|Qt::ItemIsEnabled)
{
}

HbListWidgetItemPrivate::~HbListWidgetItemPrivate()
{
}

void HbListWidgetItemPrivate::setData(const QVariant &value, int role, int index)
{
    QVector<HbWidgetItemData>::iterator it;
    for (it = mValues.begin(); it != mValues.end(); ++it) {
        if ((*it).role == role) {
            QVariantList list;
            if ((*it).value.canConvert<QVariantList>()) {
                list = (*it).value.toList();
            } else {
                list.append((*it).value);
            }
             
            if (index < list.count()) {
                // Existing value -> replace if different
                if (list.at(index) == value)  
                    return;
                list.replace(index, value);
            } else {
                // New value -> grow the list if needed
                while (list.count() < index) {
                    list.append(QVariant());
                }

                list.append(value);
            }

            (*it).value = list;
            return;
        }
    }

    // role is not present yet
    QVariantList list;
    for (int i = 0; i < index; i++) {
        list.append(QVariant());
    }
    list.append(value);
    mValues.append(HbWidgetItemData(role, list));
}

void HbListWidgetItemPrivate::setData(const QVariant &value, int role)
{
    //Note: no more call to itemChanged, caller is responsible for calling it
    role = (role == Qt::EditRole) ? Qt::DisplayRole : role;

    QVector<HbWidgetItemData>::iterator it;
    for (it = mValues.begin(); it != mValues.end(); ++it) {
        if ((*it).role == role) {
            (*it).value = value;
            return;
        }
    }

    mValues.append(HbWidgetItemData(role, value));
}


QVariant HbListWidgetItemPrivate::data(int role, int index) const
{
    QVariant value = data(role);
    if (value.canConvert<QVariantList>()) {
        QVariantList list = value.toList();
        return list.value(index);
    } else if (index == 0) {
        return value;
    } else {
        return QVariant();
    }
}

/*
    Returns the item's data for the given \a role, or an invalid
    QVariant if there is no data for the role.
    
    \note The default implementation treats Qt::EditRole and Qt::DisplayRole
    as referring to the same data.  
*/
QVariant HbListWidgetItemPrivate::data(int role) const
{
    role = (role == Qt::EditRole) ? Qt::DisplayRole : role;

    QVector<HbWidgetItemData>::const_iterator it;
    for (it = mValues.begin(); it != mValues.end(); ++it) {
        if ((*it).role == role) {
            return (*it).value;
        }
    }

    if (role == Hb::ItemTypeRole) {
        return Hb::StandardItem;
    }
    return QVariant();
}

