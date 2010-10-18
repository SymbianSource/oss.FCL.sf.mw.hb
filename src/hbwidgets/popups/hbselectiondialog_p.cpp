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

#include <QGraphicsLinearLayout>
#include <qglobal.h>
#include "hbabstractviewitem.h"
#include "hbselectiondialog_p.h"
#include <hblabel.h>
#include <hblistwidget.h>
#include <hblistwidgetitem.h>
#include <QtDebug>
#include <hbcheckbox.h>
#include <hbaction.h>
#include <hbstyleoption_p.h>
#include <hbstyleprimitivedata.h>
#include <hbstyleframeprimitivedata.h>
#include <hbstyletextprimitivedata.h>
#include <hbstringutil.h>

HbSelectionDialogMarkWidget::HbSelectionDialogMarkWidget(QGraphicsItem *parent):HbWidget(parent),mBackgroundItem(0),mLbCounter(0){
    chkMark = new HbCheckBox(this);
    HbStyle::setItemName(chkMark,"checkbox");
    createPrimitives();
}

void HbSelectionDialogMarkWidget::createPrimitives()
{
    if (!mBackgroundItem ) {
        mBackgroundItem = style()->createPrimitive( HbStyle::PT_FrameItem ,"background",this );
        mBackgroundItem->setZValue(-5);
    }
    if(!mLbCounter){
        mLbCounter = style()->createPrimitive( HbStyle::PT_TextItem ,"counter",this );
    }
}

void HbSelectionDialogMarkWidget::recreatePrimitives()
{
    delete mBackgroundItem;
    mBackgroundItem = style()->createPrimitive( HbStyle::PT_FrameItem ,"background",this );
    mBackgroundItem->setZValue(-5);

    delete mLbCounter;
    mLbCounter = style()->createPrimitive( HbStyle::PT_TextItem ,"counter",this );

}
void HbSelectionDialogMarkWidget::initPrimitiveData(HbStylePrimitiveData *primitiveData, const QGraphicsObject *primitive)
{
      HbWidgetBase::initPrimitiveData(primitiveData, primitive); 
      QString itemName = HbStyle::itemName(primitive); 
      if (itemName == QLatin1String("background")) { 
          HbStyleFramePrimitiveData* data = hbstyleprimitivedata_cast<HbStyleFramePrimitiveData*>(primitiveData); 
          data->frameGraphicsName = QLatin1String("qtg_fr_groupbox");
          data->frameType = HbFrameDrawer::NinePieces;
      } 
      else if (itemName == QLatin1String("counter")) { 
            HbStyleTextPrimitiveData *data = hbstyleprimitivedata_cast<HbStyleTextPrimitiveData*>(primitiveData);
            data->text = mCounterText;
      } 

}

void HbSelectionDialogMarkWidget::updateCounter(const QString& text)
{
    mCounterText = text;
    HbStyleTextPrimitiveData data;
    initPrimitiveData(&data, mLbCounter);
    style()->updatePrimitive(mLbCounter, &data, this);
}

void HbSelectionDialogMarkWidget::updatePrimitives()
{
      if (mBackgroundItem) { 
          HbStyleFramePrimitiveData frameData; 
          initPrimitiveData(&frameData, mBackgroundItem); 
          style()->updatePrimitive(mBackgroundItem, &frameData, this); 
      }     
}

/*!
    \reimp
 */
QVariant HbSelectionDialogMarkWidget::itemChange( GraphicsItemChange change, const QVariant &value )
{
    switch ( change ) {
        case ItemVisibleHasChanged: {
            updatePrimitives( );
            }
            break;

        case ItemSceneHasChanged: {
            updatePrimitives();
            }
            break;
        default:
            break;
    }
    return HbWidget::itemChange( change, value );
}

/*!
    \reimp
*/
QGraphicsItem *HbSelectionDialogMarkWidget::primitive(const QString &itemName) const
{
    if(!itemName.compare(QString("background"))){
        return mBackgroundItem;
    }

    return HbWidget::primitive(itemName);
}


HbSelectionDialogMarkWidget::~HbSelectionDialogMarkWidget()
{
}

HbSelectionDialogContentWidget::HbSelectionDialogContentWidget(HbSelectionDialogPrivate *priv):HbWidget(),
                        mListView(0),d(priv),markWidget(0)
{
    mSelectionTitle = hbTrId("txt_common_list_mark_all_items");
}

void HbSelectionDialogContentWidget::_q_listWidgetItemSelected(HbListWidgetItem *item)
{
    Q_UNUSED(item)
}


int HbSelectionDialogContentWidget::selectedItemCount() const
{
    int selectedItems = 0;
    if(mListView){
        QItemSelectionModel* selectionModel = mListView->selectionModel();
        if(selectionModel){
            selectedItems = selectionModel->selectedRows().count();
        }
    }
    return selectedItems;
}

int HbSelectionDialogContentWidget::totalItemCount() const
{
    int nCount = 0;
    if(mListView && mListView->model()){
        nCount = mListView->model()->rowCount();
    }
    return nCount;
}

void HbSelectionDialogContentWidget::setModel(QAbstractItemModel* model)
{
    if(mListView){
        HbListWidget* mView = qobject_cast<HbListWidget*>(mListView);
        if(!mView){ //can not set model on HbListWidget
            QObject::disconnect(mListView->selectionModel(),SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),this,SLOT(_q_selectionChanged(const QItemSelection&, const QItemSelection&)));
            mListView->setModel(model);
            if(model)
                QObject::connect(mListView->selectionModel(),SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),this,SLOT(_q_selectionChanged(const QItemSelection&, const QItemSelection&)));
        }
    }
}

QString HbSelectionDialogContentWidget::selectionTitle() const
{
    return mSelectionTitle;
}

void HbSelectionDialogContentWidget::setSelectionTitle(const QString& title)
{
    if(mSelectionTitle != title){
        mSelectionTitle = title;
        if(markWidget)
            markWidget->chkMark->setText(mSelectionTitle);
    }
}

void HbSelectionDialogContentWidget::updateCounter()
{
    if(!mListView) return;
    if(mListView->selectionMode()!= HbAbstractItemView::MultiSelection) return;
    if(markWidget){
        int totalItems = totalItemCount();
        int selectedItems = selectedItemCount();
        markWidget->updatePrimitives();
        markWidget->updateCounter(QString(HbStringUtil::convertDigits(QString::number(selectedItems)) + "/" + HbStringUtil::convertDigits(QString::number(totalItems))));
        //update checked state of "MarkAll" checkbox 
        if (totalItems > 0 && (selectedItems == totalItems)){
            markWidget->chkMark->blockSignals(true); //should not call _q_checkboxclicked()
            markWidget->chkMark->setChecked(true);
            markWidget->chkMark->blockSignals(false);
        }
        else{
            markWidget->chkMark->blockSignals(true); //should not call _q_checkboxclicked()
            markWidget->chkMark->setChecked(false);
            markWidget->chkMark->blockSignals(false);
        }
    }
}

void HbSelectionDialogContentWidget::_q_checkboxclicked(int value)
{
   QAbstractItemModel* itemModel = mListView->model();
    QModelIndex indexStart,indexEnd;
    if(itemModel){
        indexStart = itemModel->index(0,0);
        indexEnd = itemModel->index(itemModel->rowCount()-1,0);
    }

    QItemSelectionModel* selectionModel = mListView->selectionModel();
    if(selectionModel){
        if(value){ //Select All
            selectionModel->select(QItemSelection(indexStart,indexEnd),QItemSelectionModel::Select);
        }
        else{ //Select None
            selectionModel->clear();
        }
    }
    updateCounter();
}

void HbSelectionDialogContentWidget::showMarkWidget(bool bShow)
{
    if(bShow){
        if(!markWidget){
            markWidget = new HbSelectionDialogMarkWidget(this);
            markWidget->chkMark->setText(mSelectionTitle);
        }
        else{
            markWidget->show();
        }
        HbStyle::setItemName(markWidget,"markwidget");
        connect(markWidget->chkMark,SIGNAL(stateChanged ( int )),this,SLOT(_q_checkboxclicked(int)));
        repolish();
        updateCounter();
    }
    else{
        if(markWidget){
            HbStyle::setItemName(markWidget,"");
            markWidget->hide();
            repolish();
        }
    }
}

void HbSelectionDialogContentWidget::connectSlots()
{
    QObject::connect(mListView,SIGNAL(activated(const QModelIndex&)),this,SLOT(_q_listItemSelected(QModelIndex)));
    QObject::connect(mListView->model(),SIGNAL(rowsRemoved(const QModelIndex& ,int,int)),this,SLOT(modelChanged(const QModelIndex&,int,int)));
    QObject::connect(mListView->model(),SIGNAL(rowsInserted(const QModelIndex& ,int,int)),this,SLOT(modelChanged(const QModelIndex&,int,int)));
}

void HbSelectionDialogContentWidget::_q_selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
    updateCounter();
    if(mListView->selectionMode()== HbAbstractItemView::MultiSelection)
        d->_p_SelectionChanged();
}

void HbSelectionDialogContentWidget::_q_listItemSelected(QModelIndex index)
{
    Q_UNUSED(index)
    if(mListView->selectionMode()== HbAbstractItemView::SingleSelection || mListView->selectionMode()== HbAbstractItemView::NoSelection){
       d->close();
    }
    updateCounter();
}


void HbSelectionDialogContentWidget::modelChanged(const QModelIndex &parent, int start,int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    updateCounter();
}

void HbSelectionDialogContentWidget::createListWidget()
{
    if(mListView){
        QObject::disconnect(mListView->selectionModel(),SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),this,SLOT(_q_selectionChanged(const QItemSelection&, const QItemSelection&)));
        HbListWidget* mView = qobject_cast<HbListWidget*>(mListView);
        if(!mView){
            delete mListView;
            mListView = new HbListWidget(this);
            HbStyle::setItemName(mListView, "list");
            connectSlots();
            repolish();
        }
    }
    else{
            mListView = new HbListWidget(this);
            HbStyle::setItemName(mListView, "list");
            connectSlots();
            repolish();
    }
    QObject::connect(mListView->selectionModel(),SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),this,SLOT(_q_selectionChanged(const QItemSelection&, const QItemSelection&)));
}

void HbSelectionDialogContentWidget::createListView()
{
    if(mListView){
        QObject::disconnect(mListView->selectionModel(),SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),this,SLOT(_q_selectionChanged(const QItemSelection&, const QItemSelection&)));
        HbListWidget* mView = qobject_cast<HbListWidget*>(mListView);
        if(mView){
            delete mListView;
            mListView = new HbListView(this);
            HbStyle::setItemName(mListView, "list");
            connectSlots();
            repolish();
        }
    }
    else{
            mListView = new HbListView(this);
            HbStyle::setItemName(mListView, "list");
            connectSlots();
            repolish();
    }
    QObject::connect(mListView->selectionModel(),SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),this,SLOT(_q_selectionChanged(const QItemSelection&, const QItemSelection&)));
}

HbSelectionDialogPrivate::HbSelectionDialogPrivate()
    :HbDialogPrivate()
{
    bOwnItems = false;
//    action1 = action2 = 0;
}

HbSelectionDialogPrivate::~HbSelectionDialogPrivate()
{
    clearItems(bOwnItems);    
}

void HbSelectionDialogPrivate::init()
{
    qDebug()<<" Entering init()";
    Q_Q(HbSelectionDialog);

    bOwnItems = false;
    mSelectionMode = HbAbstractItemView::SingleSelection;
    HbSelectionDialogContentWidget* contentWidget = new HbSelectionDialogContentWidget(this);
    q->setContentWidget(contentWidget);

    q->setDismissPolicy(HbPopup::NoDismiss);
    q->setTimeout(HbPopup::NoTimeout);      
    q->setModal(true);
    showActions(mSelectionMode);
}

void HbSelectionDialogPrivate::_p_SelectionChanged()
{
    Q_Q(HbSelectionDialog);
    if(mSelectionMode == HbAbstractItemView::MultiSelection)
    {
        if(action1){
            QItemSelectionModel *model = 0;
            model = selectionModel();
            if(model->selectedRows().count()==0)
                action1->setEnabled(false);
            else
                action1->setEnabled(true);
        }
        emit q->selectionChanged();
    }
}

void HbSelectionDialogPrivate::showActions(HbAbstractItemView::SelectionMode selectionMode)
{
    Q_Q(HbSelectionDialog);
    if(selectionMode == HbAbstractItemView::SingleSelection || selectionMode == HbAbstractItemView::NoSelection){
        if(action1) {
            q->disconnect(action1,SIGNAL(triggered()),q,SLOT(accept()));
            q->removeAction(action1);
        }
        if(action2 == NULL){
            action2 =new HbAction(hbTrId("txt_common_button_cancel"),q);
        }
        if(q->actions().contains(action2) == false){
            q->connect(action2,SIGNAL(triggered()),q,SLOT(reject()));
            q->insertAction(0,action2);
        }
    }
    else{

        if(action2 == NULL){
            action2 =new HbAction(hbTrId("txt_common_button_cancel"),q);
        }    
        if(q->actions().contains(action2) == false){
            q->connect(action2,SIGNAL(triggered()),q,SLOT(reject()));
            q->insertAction(0,action2);
        }
        if(action1 == NULL){
            action1 =new HbAction(hbTrId("txt_common_button_ok"),q);
        }    
        if(q->actions().contains(action1) == false)
        {
            q->connect(action1,SIGNAL(triggered()),q,SLOT(accept()));
            q->insertAction(q->actions().first(),action1);
        }
    }
    
}

void HbSelectionDialogPrivate::setSelectionMode(HbAbstractItemView::SelectionMode mode)
{
    Q_Q(HbSelectionDialog);

    mSelectionMode = mode;
    switch(mSelectionMode)
    {
    case HbAbstractItemView::NoSelection:
    case HbAbstractItemView::SingleSelection:
    case HbAbstractItemView::MultiSelection:
    {
        HbSelectionDialogContentWidget* cWidget = qobject_cast<HbSelectionDialogContentWidget*>(q->contentWidget());
        if(cWidget && cWidget->mListView){
            cWidget->mListView->setSelectionMode(mSelectionMode);
            if(mode == HbAbstractItemView::MultiSelection){
                cWidget->showMarkWidget(true);    
                q->setFullScreen(true);
            }
            else{
                cWidget->showMarkWidget(false);    
                q->setFullScreen(false);
            }
        }
        showActions(mSelectionMode);
    }
    break;
    default:
    break;
    }
}


void HbSelectionDialogPrivate::clearItems(bool keepItems)
{
    Q_Q(HbSelectionDialog);
    HbSelectionDialogContentWidget* cWidget = qobject_cast<HbSelectionDialogContentWidget*>(q->contentWidget());
    if(cWidget){
        HbListWidget* mWidget = qobject_cast<HbListWidget*>(cWidget->mListView);
        if(mWidget){
                if(keepItems){
                int nRows = 0;
                QAbstractItemModel* itemModel = mWidget->model();
                if(itemModel){
                    nRows = itemModel->rowCount();
                    while(nRows){
                        mWidget->takeItem(0);
                        nRows = itemModel->rowCount();
                    }
                }
            }
            else{
                mWidget->clear();
            }
            bOwnItems = false;
            return;
        }
        HbListView* mView = qobject_cast<HbListView*>(cWidget->mListView);
        if(mView){
            cWidget->setModel(0);
        }
    }
}

QList<HbListWidgetItem*> HbSelectionDialogPrivate::widgetItems() const
{
    Q_Q(const HbSelectionDialog);

    QList<HbListWidgetItem*> rows;
    HbSelectionDialogContentWidget* cWidget = qobject_cast<HbSelectionDialogContentWidget*>(q->contentWidget());
    if(cWidget){
        HbListWidget* widget = qobject_cast<HbListWidget*>(cWidget->mListView);
        if(widget){
            int count = 0;
            QAbstractItemModel* itemModel = widget->model();
            if(itemModel)
                count = itemModel->rowCount();
            for(int i = 0; i < count; i++){
                rows.append(widget->item(i));
            }
        }
    }
    return rows;
}

void HbSelectionDialogPrivate::setStringItems(const QStringList &items, int currentIndex)
{
    Q_Q(HbSelectionDialog);

    HbSelectionDialogContentWidget* cWidget = qobject_cast<HbSelectionDialogContentWidget*>(q->contentWidget());
    if(!cWidget) return;
    
    int nRows = 0;
    cWidget->createListWidget();
    setSelectionMode(mSelectionMode);
    if(cWidget->mListView){
        int count = items.size();
        if(count > 0) clearItems(bOwnItems); //Clear the existing items first
        for (int i = 0; i < count; ++i) {
            HbListWidgetItem* modelItem = new HbListWidgetItem();
            QString str = items.at(i);
            modelItem->setText(str);
            HbListWidget* widget = (HbListWidget*)cWidget->mListView;
            widget->addItem(modelItem);
            
            QAbstractItemModel* itemModel = cWidget->mListView->model();
            if(itemModel)
                nRows = itemModel->rowCount();
        }
        if(nRows > 0){ //if addition of rows was correct.
            QList<QVariant> currentRow;
            currentRow.append(QVariant(currentIndex));
            setSelectedItems(currentRow);
        }
        
    }
}

QStringList HbSelectionDialogPrivate::stringItems() const
{
    QStringList list;
    QList<HbListWidgetItem*> items = widgetItems();
    int count = items.count();
    for(int i = 0; i < count; i++){
                QString text = items[i]->text();
                if(!text.isEmpty()){
            list += text;
        }
    }
    return list;
}

void HbSelectionDialogPrivate::setModel(QAbstractItemModel* model)
{
    Q_Q(HbSelectionDialog);

    HbSelectionDialogContentWidget* cWidget = qobject_cast<HbSelectionDialogContentWidget*>(q->contentWidget());
    if(cWidget){
        cWidget->createListView();
        setSelectionMode(mSelectionMode);
        cWidget->setModel(model); 
        cWidget->updateCounter();
    }
}

void HbSelectionDialogPrivate::setWidgetItems(const QList<HbListWidgetItem*> &items,bool transferOwnership,int currentIndex)
{
    Q_Q(HbSelectionDialog);

    HbSelectionDialogContentWidget* cWidget = qobject_cast<HbSelectionDialogContentWidget*>(q->contentWidget());
    if(cWidget){
        cWidget->createListWidget();
        setSelectionMode(mSelectionMode);
        if(cWidget->mListView){
            HbListWidget* widget = (HbListWidget*)cWidget->mListView;
            int count = items.count();
            if(count > 0) clearItems(bOwnItems); //Clear the existing items first
            for(int i = 0; i < count; i++){
                widget->addItem(items[i]);
            }
            widget->setCurrentRow(currentIndex);
            
        }
        bOwnItems = transferOwnership;
        cWidget->updateCounter();
    }
}

QAbstractItemModel* HbSelectionDialogPrivate::model() const
{
    Q_Q(const HbSelectionDialog);
    
    HbSelectionDialogContentWidget* cWidget = qobject_cast<HbSelectionDialogContentWidget*>(q->contentWidget());
    if(cWidget && cWidget->mListView){
        return cWidget->mListView->model();
    }
    return 0;
}

QItemSelectionModel* HbSelectionDialogPrivate::selectionModel() const
{
    Q_Q(const HbSelectionDialog);
    
    HbSelectionDialogContentWidget* cWidget = qobject_cast<HbSelectionDialogContentWidget*>(q->contentWidget());
    if(cWidget && cWidget->mListView){
        return cWidget->mListView->selectionModel();
    }
    return 0;
}

void HbSelectionDialogPrivate::setSelectedItems(const QList<QVariant> items)
{
    Q_Q(const HbSelectionDialog);
    QItemSelectionModel *model = 0;
    model = selectionModel();
    if(model){
        model->clearSelection();
        _p_SelectionChanged();
        Q_FOREACH(QVariant i,items) {
                model->select(model->model()->index(i.toInt(),0),
                    QItemSelectionModel::Select);
        }
        HbSelectionDialogContentWidget* cWidget = qobject_cast<HbSelectionDialogContentWidget*>(q->contentWidget());
        if(cWidget){
            cWidget->updateCounter();
        }
    }
}

QList<QVariant> HbSelectionDialogPrivate::selectedItems() const
{
    QItemSelectionModel *model = 0;
    QList<QVariant> selIndexes;
    model = selectionModel();
    if(model){
        if(mSelectionMode == HbAbstractItemView::NoSelection){
            QModelIndex index = model->currentIndex();
            selIndexes.append(QVariant(index.row()));
        }
        else{
            QModelIndexList indexes = model->selectedIndexes();
            int count = indexes.count();
            QModelIndex index;
            for(int i = 0 ; i < count ; i++){
                index = indexes[i];
                selIndexes.append(QVariant(index.row()));
            }
        }
    }
    return selIndexes;
}

QModelIndexList HbSelectionDialogPrivate::selectedModelIndexes() const
{
    QItemSelectionModel *model = 0;
    QModelIndexList selIndexes;
    model = selectionModel();
    if(model){
        if(mSelectionMode == HbAbstractItemView::NoSelection){
            QModelIndex index = model->currentIndex();
            selIndexes.append(index);
        }
        else{
            selIndexes =  model->selectedIndexes();
        }
    }
    return selIndexes;
}

void HbSelectionDialogPrivate::close()
{
    Q_Q(HbSelectionDialog);
    q->accept(); //emit the signal
}

QString HbSelectionDialogPrivate::selectionTitle() const
{
    Q_Q(const HbSelectionDialog);
    QString str;
    HbSelectionDialogContentWidget* cWidget = qobject_cast<HbSelectionDialogContentWidget*>(q->contentWidget());
    if(cWidget){
        str = cWidget->selectionTitle();
    }
    return str;
}

void HbSelectionDialogPrivate::setSelectionTitle(const QString& title)
{
    Q_Q(HbSelectionDialog);
    HbSelectionDialogContentWidget* cWidget = qobject_cast<HbSelectionDialogContentWidget*>(q->contentWidget());
    if(cWidget){
        cWidget->setSelectionTitle(title);
    }
}
