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

#ifndef HBLISTMODEL_H
#define HBLISTMODEL_H

#include <hblistwidgetitem.h>
#include <hbnamespace.h>
#include <hbglobal.h>

#include <qabstractitemmodel.h>

class HbListWidgetItem;

class HB_AUTOTEST_EXPORT HbListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    
    HbListModel(QObject *parent=0);
    ~HbListModel();

    void clear();
    HbListWidgetItem *item(int row) const;

    void insert(int row, HbListWidgetItem *item);
    void appendRow(HbListWidgetItem *item);

    HbListWidgetItem *take(int row);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    QModelIndex index(HbListWidgetItem *item) const;
    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    bool removeRows(int row, int count = 1, const QModelIndex &parent = QModelIndex());

    void itemChanged(HbListWidgetItem *item);

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

private:
    QList<HbListWidgetItem *> items; 
};

#endif // HBLISTMODEL_H
