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

#include "hblistmodel_p.h"
#include "hblistwidgetitem_p.h"

#include <hblistwidgetitem.h>

HbListModel::HbListModel(QObject *parent) : 
    QAbstractListModel(parent)
{
}

HbListModel::~HbListModel()
{
    qDeleteAll(items);
}

void HbListModel::clear()
{
    for (int i = 0; i < items.count(); ++i) {
        if (items.at(i)) {
            items.at(i)->d->id = -1;
            delete items.at(i);
        }
    }
    items.clear();
    reset();
}

HbListWidgetItem *HbListModel::item(int row) const
{
    return items.value(row);
}

void HbListModel::insert(int row, HbListWidgetItem *item)
{
    item->d->mModel = this;
    //Do not accept duplicate inserts.
    //Occurs e.g if user of HbListWidgetitem creates HbListWidgetItem and gives a parent
    //(HbListWidget) but still calls HbListWidget::addItem()
    if (!item || items.contains(item) )
        return;

    if (row < 0)
        row = 0;
    else if (row > items.count())
        row = items.count();
    beginInsertRows(QModelIndex(), row, row);
    items.insert(row, item);
    item->d->id = row;
    endInsertRows();
}

void HbListModel::appendRow(HbListWidgetItem *item)
{
    item->d->mModel = this;
    insert(rowCount(),item);
}

HbListWidgetItem *HbListModel::take(int row)
{
    if (row < 0 || row >= items.count())
        return 0;

    beginRemoveRows(QModelIndex(), row, row);
    items.at(row)->d->id = -1;
    HbListWidgetItem *item = items.takeAt(row);
    endRemoveRows();
    return item;
}

int HbListModel::rowCount(const QModelIndex &parent) const
{
     return parent.isValid() ? 0 : items.count();
}

QModelIndex HbListModel::index(HbListWidgetItem *item) const
{
    if (!item || items.isEmpty())
        return QModelIndex();

    int row;
    const int id = item->d->id;

    if (id >= 0 && id < items.count() && items.at(id) == item)
    {
        row = id;
    }
    else
    {
        // we need to search for the item
        row = items.lastIndexOf(item);  // lastIndexOf is an optimization in favor of indexOf
        if (row == -1) // not found
            return QModelIndex();
        item->d->id = row;
    }
    return createIndex(row, 0, item);
}

QModelIndex HbListModel::index(int row, int column, const QModelIndex &parent) const
{
    if (hasIndex(row, column, parent))
        return createIndex(row, column, items.at(row));

     return QModelIndex();
}

QVariant HbListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= items.count())
        return QVariant();
    return items.at(index.row())->data(role);

}

bool HbListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= items.count())
        return false;
    items.at(index.row())->setData(value,role);
    return true;
}

bool HbListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (count < 1 || row < 0 || (row + count) > rowCount() || parent.isValid())
        return false;

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    HbListWidgetItem *itm = 0;
    for (int r = row; r < row + count; ++r) {
        itm = items.takeAt(row);
        itm->d->id = -1;
        delete itm;
    }
    endRemoveRows();
    return true;
}

void HbListModel::itemChanged(HbListWidgetItem *item)
{
    QModelIndex idx = index(item);
    emit dataChanged(idx, idx);
}

Qt::ItemFlags HbListModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        const HbListWidgetItem *item = items.at(index.row());
        if (item) {
            return item->d->flags;
        } 
    }
    
    return 0;
}

#include "moc_hblistmodel_p.cpp"


