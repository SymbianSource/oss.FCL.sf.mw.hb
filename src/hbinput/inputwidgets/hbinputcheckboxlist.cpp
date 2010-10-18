/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbInput module of the UI Extensions for Mobile.
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
#include "hbinputcheckboxlist_p.h"

#include <hblistwidget.h>
#include <hblistwidgetitem.h>
#include <hbdataformmodelitem.h>
#include <hbdataformmodel.h>
#include "hbwidget_p.h"

/// @cond

class HbInputCheckBoxListPrivate
{
public:
    HbListWidget *mListWidget;
};

/// @endcond

/*!
Constructs checkbox list
*/
HbInputCheckBoxList::HbInputCheckBoxList(QGraphicsItem *parent)
    : HbDataFormViewItem(parent), d_ptr(new HbInputCheckBoxListPrivate())
{
}

/*!
Destructs checkbox list
*/
HbInputCheckBoxList::~HbInputCheckBoxList()
{
    delete d_ptr;
}

/*!
Returns a new copy of this object
*/
HbAbstractViewItem *HbInputCheckBoxList::createItem()
{
    return new HbInputCheckBoxList(*this);
}

/*!
\reimp
Returns true if \a index is supported, otherwise returns false.
 */
bool HbInputCheckBoxList::canSetModelIndex(const QModelIndex &index) const
{
    HbDataFormModelItem::DataItemType itemType =
        static_cast<HbDataFormModelItem::DataItemType>(index.data(HbDataFormModelItem::ItemTypeRole).toInt());

    if (itemType == HbDataFormModelItem::CustomItemBase) {
        return true;
    } else {
        return false;
    }
}

/*!
Updates the selected items to the model
*/
void HbInputCheckBoxList::itemActivated(const QModelIndex &index)
{
    Q_D(HbInputCheckBoxList);
    QModelIndex itemIndex = modelIndex();
    HbDataFormModelItem *modelItem = static_cast<HbDataFormModelItem *>(
                                         static_cast<HbDataFormModel *>(itemView()->model())->itemFromIndex(itemIndex));

    QList<QVariant> selectedValues = modelItem->contentWidgetData(QString("selectedItems")).toList();
    selectedValues.replace(index.row(), !selectedValues.at(index.row()).toBool());
    modelItem->setContentWidgetData(QString("selectedItems"), selectedValues);

    HbAbstractViewItem *viewItem = d->mListWidget->viewItem(index.row());
    if (selectedValues.at(index.row()).toBool()) {
        d->mListWidget->setCurrentIndex(viewItem->modelIndex(), QItemSelectionModel::Select);
    } else {
        d->mListWidget->setCurrentIndex(viewItem->modelIndex(), QItemSelectionModel::Deselect);
    }
}

/*!
Creates a widget for showing checkbox list
*/
HbWidget *HbInputCheckBoxList::createCustomWidget()
{
    Q_D(HbInputCheckBoxList);

    QModelIndex itemIndex = modelIndex();
    HbDataFormModelItem *modelItem = static_cast<HbDataFormModelItem *>(
                                         static_cast<HbDataFormModel *>(itemView()->model())->itemFromIndex(itemIndex));

    d->mListWidget = new HbListWidget();
    d->mListWidget->setSelectionMode(HbAbstractItemView::MultiSelection);
    d->mListWidget->contentWidget()->setContentsMargins(10, 10, 10, 10);

    QString objectName = modelItem->contentWidgetData(QString("objectName")).toString();
    d->mListWidget->setObjectName(objectName);
    
    // get listwidget's widget private ptr
    HbWidgetPrivate *priv = static_cast<HbWidgetPrivate*>(HbWidgetBasePrivate::d_ptr(d->mListWidget));
    priv->setBackgroundItem(HbStylePrivate::P_DataItem_background);
    d->mListWidget->setScrollDirections(0);

    QStringList items = modelItem->contentWidgetData(QString("items")).toStringList();
    foreach(const QString &itemName, items) {
        HbListWidgetItem *item = new HbListWidgetItem();
        item->setData(QVariant(itemName), Qt::DisplayRole);
        d->mListWidget->addItem(item);
    }

    QList<QVariant> selectedValues = modelItem->contentWidgetData(QString("selectedItems")).toList();
    for (int i = 0; i < d->mListWidget->count(); ++i) {
        HbAbstractViewItem *viewItem = d->mListWidget->viewItem(i);
        viewItem->setObjectName(objectName + "_" + items.at(i));
        if (selectedValues.at(i).toBool()) {
            d->mListWidget->setCurrentIndex(viewItem->modelIndex(), QItemSelectionModel::Select);
        }
    }

    connect(d->mListWidget, SIGNAL(activated(const QModelIndex &)), this, SLOT(itemActivated(const QModelIndex &)));

    return d->mListWidget;
}

/*!
Copy constructor for private use.
*/
HbInputCheckBoxList::HbInputCheckBoxList(const HbInputCheckBoxList &other)
    : HbDataFormViewItem(other), d_ptr(new HbInputCheckBoxListPrivate())
{
}

// End of file
