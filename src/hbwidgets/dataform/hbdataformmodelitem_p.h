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
#include <QAbstractItemModel>

#include "hbdataformmodelitem.h"
#include "hbdataformmodel.h"
#include "hbdataformmodel_p.h"

class QAbstractItemModel;

class HbDataFormModelItemPrivate
{
    Q_DECLARE_PUBLIC(HbDataFormModelItem)

public:
    HbDataFormModelItemPrivate();
    ~HbDataFormModelItemPrivate();
    QString dirtyProperty();

    void setModel(const QAbstractItemModel *model);
    QAbstractItemModel* model() const;
    void setContentWidgetData(const QString& propertyName ,const QVariant &value);

public:

    QList<HbDataFormModelItem*> mChildItems;
    HbDataFormModelItem* mParentItem;    
    QString mLabel;
    QString mIcon;
    QString mDescription;
    HbDataFormModelItem::DataItemType mItemType;
    QHash<QString, QVariant> mProperties;
    HbDataFormModelItem* q_ptr;
    QAbstractItemModel* mModel;
    QString mDirtyProperty;
    QHash<int, QVariant> mItemData;
    Qt::ItemFlags mFlags;
    
public:
    static HbDataFormModelItemPrivate *d_ptr(HbDataFormModelItem *item) {
        Q_ASSERT(item);
        return item->d_func();
    }
};

