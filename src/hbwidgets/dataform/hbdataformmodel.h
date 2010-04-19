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

#ifndef HBDATAFORMMODEL_H
#define HBDATAFORMMODEL_H

#include <QAbstractItemModel>

#include <hbglobal.h>
#include <hbdataformmodelitem.h>

class HbDataFormModelPrivate;

class HB_WIDGETS_EXPORT HbDataFormModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    explicit HbDataFormModel(QObject *parent = 0); 
    virtual ~HbDataFormModel();

    HbDataFormModelItem* appendDataFormPage(const QString &label = QString());
    HbDataFormModelItem* appendDataFormGroup(const QString &label = QString(),
        HbDataFormModelItem *parent = 0);
    HbDataFormModelItem* appendDataFormGroupPage(
        const QString &label = QString(), HbDataFormModelItem *parent = 0);
    HbDataFormModelItem* appendDataFormItem(
        HbDataFormModelItem::DataItemType  itemType,
        const QString &label = QString(), HbDataFormModelItem *parent = 0);
    void appendDataFormItem(HbDataFormModelItem *data, HbDataFormModelItem *parent = 0);

    HbDataFormModelItem* insertDataFormPage(int index, const QString &label = QString());
    HbDataFormModelItem* insertDataFormGroup(int index, const QString &label = QString(),
        HbDataFormModelItem *parent = 0);
    HbDataFormModelItem* insertDataFormGroupPage(int index, const QString &label = QString(), 
        HbDataFormModelItem *parent = 0);
    HbDataFormModelItem* insertDataFormItem(int index, HbDataFormModelItem::DataItemType  itemType,
        const QString &label = QString() , HbDataFormModelItem *parent = 0);
    void insertDataFormItem(int index, HbDataFormModelItem *data,
        HbDataFormModelItem *parent = 0);

    bool removeItem(const QModelIndex &index);
    bool removeItem(HbDataFormModelItem *item);
    
    int childCount(const QModelIndex &parent = QModelIndex()) const;

    HbDataFormModelItem* itemFromIndex(const QModelIndex &index) const;
    QModelIndex indexFromItem(const HbDataFormModelItem *item) const;
    HbDataFormModelItem* item(int row, const QModelIndex &index = QModelIndex()) const;
    HbDataFormModelItem* invisibleRootItem() const;  
    void clear();

    virtual bool insertRows(int row, int count, const QModelIndex &index = QModelIndex());
    virtual bool removeRows(int row, int count, const QModelIndex &index);

    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual QModelIndex index(int row, int column,
        const QModelIndex &index = QModelIndex()) const;

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value,
        int role = Qt::EditRole);

    virtual bool hasChildren(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &index = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &index = QModelIndex()) const;

    virtual bool canFetchMore ( const QModelIndex & parent ) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    friend class HbDataFormModelItem;

protected:
    HbDataFormModelPrivate* const d_ptr;

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbDataFormModel)
};

#endif //HBDATAFORMMODEL_H

