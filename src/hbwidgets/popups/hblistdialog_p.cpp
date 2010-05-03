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

#include <QGraphicsGridLayout>
#include <qglobal.h>
#include "hbabstractviewitem.h"
#include "hblistdialog_p.h"
#include <hblabel.h>
#include <hbaction.h>
#include <hblistwidget.h>
#include <hblistwidgetitem.h>
#include <QtDebug>

HbListDialogContentWidget::HbListDialogContentWidget(HbListDialogPrivate *priv):HbWidget(),
						mListView(0),d(priv)
{
    mListView = new HbListWidget(this);
    HbStyle::setItemName(mListView, "list");
    HbStyle::setItemName(this, "this");
    HbAbstractItemView *view = qobject_cast<HbListWidget*>(mListView);
    if(view)
        QObject::connect(view,SIGNAL(activated(const QModelIndex&)),this,SLOT(_q_listItemSelected(QModelIndex)));

    HbListWidget* widget = qobject_cast<HbListWidget*>(mListView);
    
    if(widget){
        QObject::connect(widget,SIGNAL(activated(HbListWidgetItem *)),this,SLOT(_q_listWidgetItemSelected(HbListWidgetItem *)));
        widget->installEventFilter(this);
    }
}


void HbListDialogContentWidget::_q_listWidgetItemSelected(HbListWidgetItem *item)
{
	if(item){
		HbListWidget* widget = qobject_cast<HbListWidget*>(mListView);
		if(widget){
			d->mSelectedIndex = widget->row(item);
		}
	}
}


void HbListDialogContentWidget::_q_listItemSelected(QModelIndex index)
{
	if(mListView->selectionMode()== HbAbstractItemView::NoSelection){
		d->mSelectedModelIndex = index;
		d->mSelectedIndex = index.row();
	}
	if(mListView->selectionMode()== HbAbstractItemView::SingleSelection ||
	   mListView->selectionMode()== HbAbstractItemView::NoSelection){
	   d->close();   
	}
}

bool HbListDialogContentWidget::eventFilter(QObject *obj, QEvent *event)
{
    bool accepted = false;
    if (obj == mListView) {
         switch(event->type()){
         case QEvent::LayoutRequest: {
             qreal minHeight = minimumHeight();
             qreal maxHeight = preferredHeight();
             qreal height = 0;
             int count = 0;
             QAbstractItemModel* itemModel = mListView->model();
             if(itemModel)
                 count = itemModel->rowCount();
             for(int i = 0 ; i < count ; i++){
                 HbAbstractViewItem *row = mListView->viewItem(i);
                 if(row)
                     height += row->size().rheight();
             }
             if(height > minHeight && height < maxHeight){
                 setPreferredHeight(height);
                 parentWidget()->resize(parentWidget()->preferredSize());
             }
             break;
         }
         default:
             break;
         }
    }
    return accepted;
}

HbListDialogPrivate::HbListDialogPrivate()
    :HbDialogPrivate()
{
	bOwnItems = false;
	mSelectedIndex = -1;
}


HbListDialogPrivate::~HbListDialogPrivate()
{
	if(!bOwnItems){
			Q_Q(HbListDialog);
			HbListDialogContentWidget* cWidget = qobject_cast<HbListDialogContentWidget*>(q->contentWidget());
			if(cWidget){
				HbListWidget* widget = qobject_cast<HbListWidget*>(cWidget->mListView);
				if(widget){
                    int nRows = 0;
                    QAbstractItemModel* itemModel = widget->model();
                    if(itemModel){
                        nRows = itemModel->rowCount();
                        while(nRows){
                            widget->takeItem(0);
                            nRows = itemModel->rowCount();
                        }
                    }
				}
			}
	}
}
void HbListDialogPrivate::init()
{
    qDebug()<<" Entering init()";
    Q_Q(HbListDialog);

    bOwnItems = false;

    HbListDialogContentWidget* contentWidget = new HbListDialogContentWidget(this);
    q->setContentWidget(contentWidget);

    mPrimaryAction = new HbAction(QString(q->tr("Ok")));
    q->setPrimaryAction(mPrimaryAction);

    mSecondaryAction = new HbAction(QString(q->tr("Cancel")));
    q->setSecondaryAction(mSecondaryAction);
    q->setTimeout(0);
    q->setModal(true);

}


void HbListDialogPrivate::setSelectionMode(HbAbstractItemView::SelectionMode mode)
{
	Q_Q(HbListDialog);

	mSelectionMode = mode;
	switch(mode)
	{
	case HbAbstractItemView::SingleSelection:
	case HbAbstractItemView::MultiSelection:
	case HbAbstractItemView::NoSelection:
	{
		HbListDialogContentWidget* cWidget = qobject_cast<HbListDialogContentWidget*>(q->contentWidget());
		if(cWidget){
			cWidget->mListView->setSelectionMode(mode);
		}
	}
	break;
	case HbAbstractItemView::ContiguousSelection:
		break;
	}
}

QList<HbListWidgetItem*> HbListDialogPrivate::widgetItems() const
{
	Q_Q(const HbListDialog);

	QList<HbListWidgetItem*> rows;
	HbListDialogContentWidget* cWidget = qobject_cast<HbListDialogContentWidget*>(q->contentWidget());
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

void HbListDialogPrivate::setCurrentRow(int row)
{
	Q_Q(HbListDialog);

	HbListDialogContentWidget* cWidget = qobject_cast<HbListDialogContentWidget*>(q->contentWidget());
	if(cWidget){
		HbListWidget* widget = qobject_cast<HbListWidget*>(cWidget->mListView);
		if(widget){
            QAbstractItemModel* itemModel = widget->model();
            QModelIndex index;
            if(itemModel){
                index = itemModel->index(row,0);
            }
		    QItemSelectionModel* selectionModel = widget->selectionModel();
		    if(selectionModel){
                selectionModel->select(index,QItemSelectionModel::Select);
		    }
		}
	}
}

void HbListDialogPrivate::setStringItems(const QStringList &items, int current)
{
	Q_Q(HbListDialog);

	HbListDialogContentWidget* cWidget = qobject_cast<HbListDialogContentWidget*>(q->contentWidget());
	if(!cWidget) return;
	
	HbListWidget* widget = qobject_cast<HbListWidget*>(cWidget->mListView);
    int nRows = 0;

	if(widget){
		int count = items.size();
		for (int i = 0; i < count; ++i) {
			HbListWidgetItem* modelItem = new HbListWidgetItem();
			QString str = items.at(i);
			modelItem->setText(str);
			widget->addItem(modelItem);
			
            QAbstractItemModel* itemModel = widget->model();
            if(itemModel)
                nRows = itemModel->rowCount();
		}
		if(nRows > 0){ //if addition of rows was correct.
            QList<int> currentRow;
            currentRow.append(current);
            setSelectedItems(currentRow);
		}
		
	}
}

QStringList HbListDialogPrivate::stringItems() const
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

int HbListDialogPrivate::currentRow()
{
	Q_Q(HbListDialog);

	int nCurrentRow = -1;

	HbListDialogContentWidget* cWidget = qobject_cast<HbListDialogContentWidget*>(q->contentWidget());
	if(!cWidget) return nCurrentRow;

	HbListWidget* widget = qobject_cast<HbListWidget*>(cWidget->mListView);
	if(widget){
            QItemSelectionModel* selectionModel = widget->selectionModel();
            if(selectionModel){
                QModelIndexList selectdList = selectionModel->selectedIndexes();
                if(selectdList.count() > 0)
                    nCurrentRow = selectdList[0].row();
            }
	}
	return nCurrentRow;
}

void HbListDialogPrivate::setModel(QAbstractItemModel* model)
{
	Q_Q(HbListDialog);

	HbListDialogContentWidget* cWidget = qobject_cast<HbListDialogContentWidget*>(q->contentWidget());
	if(cWidget){
		cWidget->mListView->HbListView::setModel(model); //HbListView's implementation of setModel()
	}
}

void HbListDialogPrivate::setWidgetItems(QList<HbListWidgetItem*> &items,bool bTransferOwnership,int current)
{
	Q_Q(HbListDialog);

	HbListDialogContentWidget* cWidget = qobject_cast<HbListDialogContentWidget*>(q->contentWidget());
	if(cWidget){
		HbListWidget* widget = qobject_cast<HbListWidget*>(cWidget->mListView);
		if(widget){
			int count = items.count();
			for(int i = 0; i < count; i++){
				widget->addItem(items[i]);
			}
			widget->setCurrentRow(current);
			
		}
		bOwnItems = bTransferOwnership;
	}
}

QAbstractItemModel* HbListDialogPrivate::model() const
{
	Q_Q(const HbListDialog);
	
	HbListDialogContentWidget* cWidget = qobject_cast<HbListDialogContentWidget*>(q->contentWidget());
	if(cWidget){
		return cWidget->mListView->HbListView::model(); //HbListView's implementation of model()
	}
	return 0;
}

QItemSelectionModel* HbListDialogPrivate::selectionModel() const
{
	Q_Q(const HbListDialog);
	
	HbListDialogContentWidget* cWidget = qobject_cast<HbListDialogContentWidget*>(q->contentWidget());
	if(cWidget){
		return cWidget->mListView->selectionModel();
	}
	return 0;
}

void HbListDialogPrivate::setSelectedItems(QList<int> items)
{
    QItemSelectionModel *model = 0;
    model = selectionModel();
    if(model){
        Q_FOREACH(int i,items) {
                model->select(model->model()->index(i,0),
                    QItemSelectionModel::Select);
        }
    }
}

QList<int> HbListDialogPrivate::selectedItems() const
{
    QItemSelectionModel *model = 0;
    QList<int> selIndexes;
    model = selectionModel();
    if(model){
        QModelIndexList indexes = model->selectedIndexes();
        int count = indexes.count();
        QModelIndex index;
        for(int i = 0 ; i < count ; i++){
            index = indexes[i];
            selIndexes.append(index.row());
        }
    }
    return selIndexes;

}

QModelIndexList HbListDialogPrivate::selectedModelIndexes() const
{
    QItemSelectionModel *model = 0;
    QModelIndexList selIndexes;
    model = selectionModel();
    if(model){
        selIndexes =  model->selectedIndexes();
    }
    return selIndexes;
}

void HbListDialogPrivate::close()
{
	Q_Q(HbListDialog);
	q->close();
}

