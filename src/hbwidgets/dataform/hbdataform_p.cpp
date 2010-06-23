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

#include "hbdataform_p.h"
#include <hbdataformviewitem.h>
#include "hbdataformviewitem_p.h"
#include "hbdataitemcontainer_p.h"
#include "hbdatagroup_p_p.h"
#include <hbcombobox.h>
#include <hbapplication.h>
#include <hbdataformmodel.h>
#include "hbdataformheadingwidget_p.h"
#include "hbtreemodeliterator_p.h"
#include <hbdatagroup_p.h>

#include <QStringListModel>
#include <QHash>

HbDataFormPrivate::HbDataFormPrivate() :
    HbAbstractItemViewPrivate(),
    mHeadingWidget(0)
{
}

HbDataFormPrivate::~HbDataFormPrivate()
{
}

int HbDataFormPrivate::childCount(const QModelIndex &index) const
{
    Q_Q(const HbDataForm);

    int itemCount = 0;
    int rowCount = 0;
    if(!index.isValid()) {
        return 0;
    }
    if (q->isExpanded(index)) {
        rowCount = index.model()->rowCount(index);
    }
    itemCount += rowCount;
    for (int row = 0; row < rowCount; ++row) {
        itemCount += childCount(index.child(row, 0));
    }
    return itemCount;
}

void HbDataFormPrivate::init()
{
    Q_Q(HbDataForm);

    QList<HbAbstractViewItem*> protos;
    protos.append(new HbDataFormViewItem());
    protos.append(new HbDataGroup());
    q->setItemPrototypes(protos);
    q->setClampingStyle(HbScrollArea::BounceBackClamping);
    q->setItemRecycling(false);
    treeModelIterator()->setItemContainer(mContainer);
}

void  HbDataFormPrivate::_q_page_changed(int index)
{
    Q_Q(const HbDataForm);
    QModelIndex childIndex = q->model()->index(index,0);
    HbDataFormModelItem::DataItemType itemType =
        static_cast<HbDataFormModelItem::DataItemType>(
        (childIndex.data(HbDataFormModelItem::ItemTypeRole)).toInt());

    if(mHeadingWidget->mPageCombo) {
        if(mHeadingWidget->mPageCombo->currentIndex() != index) {
            QObject::disconnect(mHeadingWidget->mPageCombo,SIGNAL(currentIndexChanged(int)),
                q,SLOT(_q_page_changed(int)));
            mHeadingWidget->mPageCombo->setCurrentIndex(index);
            QObject::connect(mHeadingWidget->mPageCombo,SIGNAL(currentIndexChanged(int)),
                q,SLOT(_q_page_changed(int)));
        }
    }

   /* QStringListModel *model = static_cast<QStringListModel*>(
        mHeadingWidget->mPageCombo->model());
    QModelIndex changedIndex = model->index(index, 0);*/
    if(itemType == HbDataFormModelItem::FormPageItem) {
        if(index != mHeadingWidget->mActivePage) {
            QModelIndex prevPageIndex = q->model()->index(mHeadingWidget->mActivePage,0);
            QModelIndex newPageIndex = q->model()->index(index,0);
            if(prevPageIndex.isValid()) {
                HbDataGroup *prevPage = static_cast<HbDataGroup *>(
                                               q->itemByIndex(prevPageIndex));
                if(prevPage) {
                    HbDataGroupPrivate::d_ptr(prevPage)->setExpanded(false);
                }
            }
            if(newPageIndex.isValid()) {
                HbDataGroup *newPage = static_cast<HbDataGroup *>(
                                               q->itemByIndex(newPageIndex));
                if(newPage) {
                    HbDataGroupPrivate::d_ptr(newPage)->setExpanded(true);
                }
            }
            mHeadingWidget->mActivePage = index;
        }
    }
}


/*!
    Creates a DataForm Page \a page in DataForm .
    DataForm Page is an invisible DataItem which can be changed/selected using combo box.
    \sa addFormPage
*/
void HbDataFormPrivate::addFormPage(const QString& page)
{
    Q_Q(HbDataForm);

    // Create combobox if not created 
    if(!mHeadingWidget) {
        mHeadingWidget = new HbDataFormHeadingWidget();
        static_cast<HbDataItemContainer*>(mContainer)->setFormHeading(mHeadingWidget);        
        QEvent polishEvent(QEvent::Polish);
        QCoreApplication::sendEvent(mHeadingWidget, &polishEvent);
    }

    if(!mHeadingWidget->mPageCombo) {
        mHeadingWidget->createPrimitives();
        mHeadingWidget->mPageCombo = new HbComboBox(mHeadingWidget);
		HbStyle::setItemName(mHeadingWidget->mPageCombo,"dataForm_Combo");
        QEvent polishEvent(QEvent::Polish);
        QCoreApplication::sendEvent(mHeadingWidget->mPageCombo, &polishEvent);
        // setFormHeading to the layout
        if(mHeadingWidget->mPageCombo || !mHeadingWidget->mDescription.isEmpty() || 
            !mHeadingWidget->mHeading.isEmpty()) {
            static_cast<HbDataItemContainer*>(mContainer)->setFormHeading(mHeadingWidget);
        }
        QObject::connect(mHeadingWidget->mPageCombo,SIGNAL(currentIndexChanged(int)),
            q,SLOT(_q_page_changed(int)));
    }

    // Get the model and add the page string to the model of combobox
    QStringListModel *model = static_cast<QStringListModel*>(mHeadingWidget->mPageCombo->model());
    if(!model) {
        QStringList list;
        model = new QStringListModel(list);
    }

    QStringList list = model->stringList();
    if(!list.contains(page)){
        list.append(page);
        model->setStringList(list);
        mHeadingWidget->mPageCombo->setModel(model);
        //mHeadingWidget->mPageCombo->setCurrentIndex(model->index(0,0));
        mHeadingWidget->mPageCombo->setCurrentIndex(0);
        _q_page_changed((mHeadingWidget->mPageCombo)->currentIndex());
    }
    mHeadingWidget->updatePrimitives();
}

void HbDataFormPrivate::removeFormPage(const QString& page) 
{
    Q_Q(HbDataForm);    

    if(mHeadingWidget && mHeadingWidget->mPageCombo) {
        if(mHeadingWidget->mPageCombo->currentText() == page){
            mHeadingWidget->mPageCombo->setCurrentIndex(mHeadingWidget->mActivePage+1);
        }
    }
        QObject::disconnect(mHeadingWidget->mPageCombo,SIGNAL(currentIndexChanged(int)),
            q,SLOT(_q_page_changed(int)));

        mHeadingWidget->mPageCombo->removeItem(mHeadingWidget->mPageCombo->findText(page));
        mHeadingWidget->mActivePage = mHeadingWidget->mPageCombo->currentIndex();
        
         QObject::connect(mHeadingWidget->mPageCombo,SIGNAL(currentIndexChanged(int)),
            q,SLOT(_q_page_changed(int)));

   mHeadingWidget->callPolish();    
}

void HbDataFormPrivate::_q_item_displayed(const QModelIndex &index)
{
    Q_Q( HbDataForm);
    emit q->itemShown(index);
    emit q->activated(index);
    qWarning("activated signal will not be emitted when items are created ," 
        "instead itemShown SIGNAL should be used");
}

void HbDataFormPrivate::makeConnection(QModelIndex index, HbWidget* widget)
{
    Q_Q( HbDataForm);
    if(!index.isValid()){
        return;
    }
    if(q->model()) {
        HbDataFormModelItem *modelItem = static_cast<HbDataFormModel *>(q->model())->itemFromIndex(index);
        if(modelItem){
            QList<ItemSignal> signalList = mConnectionList.values(modelItem);
            if(signalList.count() > 0){
                if(widget){
                    foreach(const ItemSignal& signal, signalList) {
                        QObject *objct = signal.receiver;
                        QString signalName = signal.signal;
                        QString slot = signal.slot;
                        // Make connection
                        if(objct) { 
                            QObject::connect(widget, signalName.toAscii().data(), 
                                objct,slot.toAscii().data());
                        }
                        
                    }
                }
            }
        }
    }
}


void HbDataFormPrivate::removeConnection(HbDataFormModelItem * modelItem, 
                                  QString signal, 
                                  QObject *receiver, 
                                  QString slot)
{
    Q_Q( HbDataForm);
    if(q->model()) {
        if(modelItem){
            QList<ItemSignal> signalList = mConnectionList.values(modelItem);
            mConnectionList.remove(modelItem);
            if(signalList.count() > 0){
            QModelIndex index = 
                static_cast<HbDataFormModel*>(q->model())->indexFromItem(modelItem);
                HbDataFormViewItem *viewItem = static_cast<HbDataFormViewItem*>(q->itemByIndex(index));
                if(viewItem){
                    HbWidget *contentWidget = HbDataFormViewItemPrivate::d_ptr(viewItem)->mContentWidget;
                    if(contentWidget){
                        //foreach(ItemSignal signalItem, signalList) {
                        for(int i = 0; i < signalList.count() ;i++){
                            ItemSignal signalItem = signalList.at(i);
                            if(receiver == signalItem.receiver &&
                            signal == signalItem.signal &&
                            slot == signalItem.slot){
                            // disconnect
                                QObject::disconnect(contentWidget, signal.toAscii().data(), 
                                    receiver,slot.toAscii().data());
                                signalList.removeAt(i);
                                for(int j = 0; j < signalList.count(); j++){
                                    mConnectionList.insertMulti(modelItem, signalList.at(j));
                                }
                                break;
                            }                            
                        }
                    }
                }
            }
        }
    }
}

void HbDataFormPrivate::connectNow(HbDataFormModelItem * modelItem, 
                                   QString signal, 
                                   QObject *receiver, 
                                   QString slot)
{
    Q_Q( HbDataForm);    
    
    if(q->model()) {
        QModelIndex index = static_cast<HbDataFormModel*>(q->model())->indexFromItem(modelItem);
        if(modelItem){
            HbDataFormViewItem *viewItem =static_cast<HbDataFormViewItem*>( q->itemByIndex(index) );
            if(viewItem){
                if(HbDataFormViewItemPrivate::d_ptr(viewItem)->mContentWidget) {
                    // Make connection
                        QObject::connect(HbDataFormViewItemPrivate::d_ptr(viewItem)->mContentWidget, signal.toAscii().data(), 
                            receiver,slot.toAscii().data());
                }
            }
        }
    }
}

void HbDataFormPrivate::removeAllConnection()
{
    Q_Q( HbDataForm);
    if(q->model()) {
        QList<HbDataFormModelItem*> keys = mConnectionList.uniqueKeys();

        foreach(HbDataFormModelItem* item ,keys) {
            QList<ItemSignal> signalList = mConnectionList.values(item);
            mConnectionList.remove(item);
            if(signalList.count() > 0){
                QModelIndex index = static_cast<HbDataFormModel*>(q->model())->indexFromItem(item);
                HbDataFormViewItem *viewItem = static_cast<HbDataFormViewItem*> (q->itemByIndex(index));
                if(viewItem){
                    HbWidget *contentWidget = HbDataFormViewItemPrivate::d_ptr(viewItem)->mContentWidget;
                    // disconnect signal and remove signal from list
                    for(int i = 0;i<signalList.count();) {
                        ItemSignal signalItem = signalList.takeAt(i);
                            // Make connection
                        QObject::disconnect(contentWidget, signalItem.signal.toAscii().data(), 
                                signalItem.receiver,signalItem.slot.toAscii().data());
                            
                        
                    }
                }
            }
        }
    }
}

void HbDataFormPrivate::removeAllConnection(HbDataFormModelItem *modelItem)
{
    Q_Q( HbDataForm);
    if(q->model()) {
        if(modelItem){
            QList<ItemSignal> signalList = mConnectionList.values(modelItem);
            mConnectionList.remove(modelItem);
            if(signalList.count() > 0){
                QModelIndex index = static_cast<HbDataFormModel*>(q->model())->indexFromItem(modelItem);
                HbDataFormViewItem *viewItem =static_cast<HbDataFormViewItem*>( q->itemByIndex(index));
                if(viewItem){
                    HbWidget *contentWidget = HbDataFormViewItemPrivate::d_ptr(viewItem)->mContentWidget;
                    // disconnect signal and remove signal from list
                    for(int i = 0;i<signalList.count(); ) {
                        ItemSignal signalItem = signalList.takeAt(i);
                            // Make connection
                        QObject::disconnect(contentWidget, signalItem.signal.toAscii().data(), 
                                signalItem.receiver,signalItem.slot.toAscii().data());
                    }
                }
            }
        }
    }
}

void HbDataFormPrivate::emitActivated(const QModelIndex &modelIndex)
{
    Q_Q( HbDataForm);
    emit q->activated(modelIndex);
}

