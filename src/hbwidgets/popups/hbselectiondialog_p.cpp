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

HbSelectionDialogContentWidget::HbSelectionDialogContentWidget(HbSelectionDialogPrivate *priv):HbWidget(),
                        mListView(0),d(priv),chkMark(0),lbCounter(0),markWidgetShown(false)
{
    
}
void HbSelectionDialogContentWidget::_q_listWidgetItemSelected(HbListWidgetItem *item)
{
	Q_UNUSED(item)
}

void HbSelectionDialogContentWidget::_q_listItemSelected(QModelIndex index)
{
    Q_UNUSED(index)
	if(mListView->selectionMode()== HbAbstractItemView::SingleSelection){
       d->close();
    }
    updateCounter();
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

void HbSelectionDialogContentWidget::updateCounter()
{
	if(!mListView) return;
    if(mListView->selectionMode()!= HbAbstractItemView::MultiSelection) return;
    if(chkMark && lbCounter){
        int totalItems = totalItemCount();
        int selectedItems = selectedItemCount();

        lbCounter->setText(QString(QString::number(selectedItems) + "/" + QString::number(totalItems)));
        //update checked state of "MarkAll" checkbox 
        if (totalItems > 0 && (selectedItems == totalItems)){
            chkMark->blockSignals(true); //should not call _q_checkboxclicked()
            chkMark->setChecked(true);
            chkMark->blockSignals(false);
        }
        else{
            chkMark->blockSignals(true); //should not call _q_checkboxclicked()
            chkMark->setChecked(false);
            chkMark->blockSignals(false);
        }
    }
}

void HbSelectionDialogContentWidget::_q_checkboxclicked(int value)
{
    int totalItems = 0;
    int selectedItems = 0;
    QAbstractItemModel* itemModel = mListView->model();
    QModelIndex indexStart,indexEnd;
    if(itemModel){
        indexStart = itemModel->index(0,0);
        indexEnd = itemModel->index(itemModel->rowCount()-1,0);
        totalItems = itemModel->rowCount();
    }

    QItemSelectionModel* selectionModel = mListView->selectionModel();
    if(selectionModel){
        selectedItems = selectionModel->selectedRows().count();
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
		if(!markWidgetShown){
            chkMark = new HbCheckBox(this);
            chkMark->setText("Mark All");
            lbCounter = new HbTextItem(this);
            HbStyle::setItemName(chkMark,"checkbox");
            HbStyle::setItemName(lbCounter,"counter");
            setProperty("multiSelection",true);
            connect(chkMark,SIGNAL(stateChanged ( int )),this,SLOT(_q_checkboxclicked(int)));
            updateCounter();
			markWidgetShown = true;
		}
    }
    else{
        delete chkMark;chkMark=0;
        delete lbCounter;lbCounter=0;
        HbStyle::setItemName(chkMark,"");
        HbStyle::setItemName(lbCounter,"");
        setProperty("multiSelection",false);
		markWidgetShown = false;
    }
}

void HbSelectionDialogContentWidget::connectSlots()
{
    QObject::connect(mListView,SIGNAL(activated(const QModelIndex&)),this,SLOT(_q_listItemSelected(QModelIndex)));
}

void HbSelectionDialogContentWidget::createListWidget()
{
	if(mListView){
		HbListWidget* mView = qobject_cast<HbListWidget*>(mListView);
		if(!mView){
			delete mListView;
			mListView = new HbListWidget(this);
			HbStyle::setItemName(mListView, "list");
			connectSlots();
		}
	}
	else{
			mListView = new HbListWidget(this);
			HbStyle::setItemName(mListView, "list");
			connectSlots();
	}
}

void HbSelectionDialogContentWidget::createListView()
{
	if(mListView){
		HbListView* mView = qobject_cast<HbListView*>(mListView);
		if(!mView){
			delete mListView;
			mListView = new HbListView(this);
			HbStyle::setItemName(mListView, "list");
			connectSlots();
		}
	}
	else{
			mListView = new HbListView(this);
			HbStyle::setItemName(mListView, "list");
			connectSlots();
	}
}

HbSelectionDialogPrivate::HbSelectionDialogPrivate()
    :HbDialogPrivate()
{
    bOwnItems = false;
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

    HbAction *action1=new HbAction(hbTrId("txt_common_button_ok"),q);
    q->addAction(action1);
    q->connect(action1,SIGNAL(triggered()),q,SLOT(accept()));

    HbAction *action2=new HbAction(hbTrId("txt_common_button_cancel"),q);
    q->addAction(action2);
    q->connect(action2,SIGNAL(triggered()),q,SLOT(reject()));


    q->setDismissPolicy(HbPopup::NoDismiss);
    q->setTimeout(HbPopup::NoTimeout);      
    q->setModal(true);
}

void HbSelectionDialogPrivate::setSelectionMode(HbAbstractItemView::SelectionMode mode)
{
    Q_Q(HbSelectionDialog);

    mSelectionMode = mode;
    switch(mSelectionMode)
    {
    case HbAbstractItemView::SingleSelection:
    case HbAbstractItemView::MultiSelection:
    {
        HbSelectionDialogContentWidget* cWidget = qobject_cast<HbSelectionDialogContentWidget*>(q->contentWidget());
        if(cWidget && cWidget->mListView){
            cWidget->mListView->setSelectionMode(mSelectionMode);
			if(mode == HbAbstractItemView::MultiSelection)
				cWidget->showMarkWidget(true);    
			else
				cWidget->showMarkWidget(false);    
		}
    }
    break;
    case HbAbstractItemView::NoSelection:
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
			cWidget->mListView->setModel(0);
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
        cWidget->mListView->setModel(model); 
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
        QModelIndexList indexes = model->selectedIndexes();
        int count = indexes.count();
        QModelIndex index;
        for(int i = 0 ; i < count ; i++){
            index = indexes[i];
            selIndexes.append(QVariant(index.row()));
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
        selIndexes =  model->selectedIndexes();
    }
    return selIndexes;
}

void HbSelectionDialogPrivate::close()
{
    Q_Q(HbSelectionDialog);
	q->accept(); //emit the signal
}

