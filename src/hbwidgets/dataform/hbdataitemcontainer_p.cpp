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

#include "hbdataitemcontainer_p.h"
#include <hbabstractitemcontainer_p.h>
#include <hbdataformviewitem.h>
#include <hbdataformmodelitem.h>
#include "hbdatagroup_p.h"
#include <hbdataform.h>
#include "hblistlayout_p.h"

#include <QCoreApplication>
#include <QGraphicsScene>

class HbDataItemContainerPrivate: public HbAbstractItemContainerPrivate
{
    Q_DECLARE_PUBLIC(HbDataItemContainer)

public:
    HbDataItemContainerPrivate();

    virtual ~HbDataItemContainerPrivate();

};

HbDataItemContainerPrivate::HbDataItemContainerPrivate() :
    HbAbstractItemContainerPrivate()
{
}

HbDataItemContainerPrivate::~HbDataItemContainerPrivate()
{
}
HbDataItemContainer::HbDataItemContainer(QGraphicsItem *parent) :
    HbAbstractItemContainer(*new HbDataItemContainerPrivate, parent),
    mLayout(0),
    mHeadingWidget(0)
{
    Q_D(HbDataItemContainer);
    d->q_ptr = this;
    mLayout = new HbListLayout();
    mLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mLayout);
}

/*!
    Destructor.
 */
HbDataItemContainer::~HbDataItemContainer()
{
}

/*!
    Layouts the HeadingWidget.
 */
void HbDataItemContainer::setFormHeading(HbWidget *headingWidget)
{
    headingWidget->setParentItem(this);
    mHeadingWidget = headingWidget;
    if (mLayout) {
        mLayout->insertItem(0, headingWidget, false);
    }
    //HbStyle::setItemName(headingWidget,"dataForm_headingWidget");
    repolish();
}

void HbDataItemContainer::reset()
{
    Q_D(HbAbstractItemContainer);
    removeItems();

    HbDataForm *form = static_cast<HbDataForm*>(itemView());

    //get the index of first child added to the model
    if(!form->model()) {
        return;
    }

    QModelIndex index = form->model()->index(0,0);
    if(index.isValid()) {
        HbDataFormModelItem::DataItemType itemType =
            static_cast<HbDataFormModelItem::DataItemType>(
            index.data(HbDataFormModelItem::ItemTypeRole).toInt());

        //if the first item added is a form page and its not expanded then expand that form page
        if((itemType == HbDataFormModelItem::FormPageItem) && (!form->isExpanded(index))) {
            form->setExpanded(index, true);
        }
    }
    d->updateItemBuffer();
}

/*!
    \Adds the Item in the tree layout of container
*/
void HbDataItemContainer::itemAdded(int index, HbAbstractViewItem *item, bool animate)
{
    Q_UNUSED(animate);
    if(static_cast<HbWidget*>(mLayout->itemAt(0)) == mHeadingWidget) {
            index++;
    }
    mLayout->insertItem(index, item, false);
}



/*!
    \Removes the item from tree layout and hide.
*/
void HbDataItemContainer::itemRemoved( HbAbstractViewItem *item, bool animate )
{
    Q_UNUSED(animate);  
    mLayout->removeItem(item, false);
}


/*!
    \Function is called when container needs to be resized.
*/
void HbDataItemContainer::viewResized(const QSizeF &viewSize)
{
    mLayout->setMinimumWidth(viewSize.width());
    mLayout->activate();
}

/*!
    Assigns new model \a index to the given \a item. Item's current state is saved
    and state for \a index is restored to item.
*/
void HbDataItemContainer::setItemModelIndex(HbAbstractViewItem *item, const QModelIndex &index)
{
    if (item && item->modelIndex() != index) {
        HbAbstractItemContainer::setItemModelIndex(item, index);
        int level = 0;
        if (itemView()) {
            QModelIndex parentIndex = item->modelIndex().parent();
            QModelIndex rootIndex = itemView()->rootIndex();

            while (parentIndex != rootIndex && parentIndex.isValid()) {
                level++;
                parentIndex = parentIndex.parent();
            }
        }
    }
}

void HbDataItemContainer::setModelIndexes(const QModelIndex &startIndex )
{
   Q_D(HbAbstractItemContainer);

   QModelIndex index = startIndex;
    if (!index.isValid() && !d->mItems.isEmpty()) {
        index = d->mItems.first()->modelIndex();
    }
    if (!d->mItemView) {
        return;
    }
    int targetCount = maxItemCount();
    HbAbstractViewItem *startItem = d->mItemView->itemByIndex(index);

    int startItemIndex = d->mItems.indexOf(startItem);
    if (d->mItems.count() < targetCount) {
        // New items needs to be added.
        while (d->mItems.count() < targetCount) {       
            index = d->mItemView->nextIndex(index);
            if (!index.isValid()) {
                break;
            }              
            insertItem(startItemIndex + 1, index);
            startItemIndex++;
        }
        
    } else if(d->mItems.count() > targetCount) {
        while (d->mItems.count() > targetCount) {
            HbAbstractViewItem *nextItem = d->mItems.takeAt(startItemIndex + 1);
            delete nextItem;
            nextItem = 0;            
        }
    }
    if (scene()) {
        QMetaObject::invokeMethod(scene(),"_q_polishItems",Qt::DirectConnection); 
        QEvent event(QEvent::LayoutRequest);
        QCoreApplication::sendEvent(this, &event);
    }
}

/*!
    Inserts item for \a index to \a pos.
*/
void HbDataItemContainer::insertItem(int pos, const QModelIndex &index, bool animate)
{
    Q_D(HbDataItemContainer);
    HbAbstractViewItem *item = d->createItem(index);
    HbDataFormModelItem::DataItemType itemType = 
        static_cast<HbDataFormModelItem::DataItemType>(
        index.data(HbDataFormModelItem::ItemTypeRole).toInt());
    if (item) {
        d->mItems.insert(pos, item);
        itemAdded(pos, item, animate);
        setItemModelIndex(item, index);
        if (itemType == HbDataFormModelItem::GroupPageItem) {
            item->setVisible(false);
        } 
    }
}

HbAbstractViewItem *HbDataItemContainer::createDefaultPrototype() const
{
    return new HbDataFormViewItem();
}

