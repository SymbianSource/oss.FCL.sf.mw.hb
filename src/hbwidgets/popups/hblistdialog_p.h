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

#ifndef HBLISTDIALOGPRIVATE_H
#define HBLISTDIALOGPRIVATE_H

#include <QGraphicsGridLayout>
#include <QGraphicsWidget>

#include <hblistdialog.h>
#include "hbdialog_p.h"
#include <hbabstractitemview.h>

#include <hbglobal.h>

class HbLabel;
class HbAction;
class HbListWidget;
class HbRadioButtonList;
class HbListWidgetItem;
class HbListView;
class HbListDialogPrivate : public HbDialogPrivate
{
	Q_DECLARE_PUBLIC(HbListDialog)
public:
    HbListDialogPrivate();
    ~HbListDialogPrivate();
private:
    void init();
    void setSelectionMode(HbAbstractItemView::SelectionMode mode);
    QList<HbListWidgetItem*> widgetItems() const;
    void setCurrentRow(int row);
    int currentRow();
    void setStringItems(const QStringList &items,int current);
    QStringList stringItems() const;
    void setModel(QAbstractItemModel* model);
    void setWidgetItems(QList<HbListWidgetItem*> &items,bool bTransferOwnership,int current);
    QAbstractItemModel* model() const;
    QItemSelectionModel* selectionModel() const;
    void setSelectedItems(QList<int> items);
    QList<int> selectedItems() const;
    QModelIndexList selectedModelIndexes() const;
public:
    HbAction *mPrimaryAction;
    HbAction *mSecondaryAction;
    bool bOwnItems;
	HbAbstractItemView::SelectionMode mSelectionMode;
    int mSelectedIndex; //for noselection mode
    QModelIndex mSelectedModelIndex;
	void createPrimitives();
    void updatePrimitives();
    void close();
};

class HB_AUTOTEST_EXPORT HbListDialogContentWidget :public HbWidget
{
	Q_OBJECT
public:
    HbListView* mListView;
    HbListDialogPrivate* d;
    HbListDialogContentWidget(HbListDialogPrivate *priv);
    enum { Type = Hb::ItemType_ListDialogContentWidget };
    int type() const { return Type; }
    bool eventFilter(QObject *obj, QEvent *event);    
private slots:    
	void _q_listWidgetItemSelected(HbListWidgetItem *item);
	void _q_listItemSelected(QModelIndex index);
};

#endif //HBLISTDIALOGPRIVATE_H
