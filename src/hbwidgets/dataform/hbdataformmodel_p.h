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

#ifndef HBDATAFORMMODEL_P_H
#define HBDATAFORMMODEL_P_H

#include <hbdataformmodel.h>
#include <hbdataformmodelitem.h>


class HbDataFormModelPrivate 
{
    Q_DECLARE_PUBLIC(HbDataFormModel)

public:
    HbDataFormModelPrivate() {
        mRoot = new HbDataFormModelItem();
        q_ptr = 0;
    }

    inline bool indexValid(const QModelIndex &index) const {
         return (index.row() >= 0) && (index.column() >= 0) && (index.model() == q_func());
    }

    virtual ~HbDataFormModelPrivate(){}

    inline HbDataFormModelItem *createItem() const {
        return new HbDataFormModelItem;
    }


    HbDataFormModelItem* addItem(HbDataFormModelItem::DataItemType itemType ,
        const QString &label = QString(), HbDataFormModelItem *parent = 0);

    HbDataFormModelItem* insertItem(
        int index, HbDataFormModelItem::DataItemType itemType ,
        const QString &label = QString(), HbDataFormModelItem *parent = 0);

    
    void rowsAboutToBeInserted(HbDataFormModelItem *parent, int start, int end);
    void rowsAboutToBeRemoved(HbDataFormModelItem *parent, int start, int end);
    void rowsInserted();
    void rowsRemoved();    

    HbDataFormModel *q_ptr;
    HbDataFormModelItem *mRoot;
   
};

#endif // HBDATAFORMMODEL_P_H

