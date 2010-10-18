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

#ifndef HBSELECTIONDIALOGPRIVATE_H
#define HBSELECTIONDIALOGPRIVATE_H

#include <QGraphicsGridLayout>
#include <QGraphicsWidget>

#include <hbselectiondialog.h>
#include "hbdialog_p.h"
#include <hbabstractitemview.h>
#include <hbtextitem.h>
#include <hbglobal.h>

class HbLabel;
class HbListWidgetItem;
class HbListView;
class HbCheckBox;

class HbSelectionDialogPrivate : public HbDialogPrivate
{
    Q_DECLARE_PUBLIC(HbSelectionDialog)
public:
    HbSelectionDialogPrivate();
    ~HbSelectionDialogPrivate();
    void setSelectionMode(HbAbstractItemView::SelectionMode mode);

private:
    void init();

    void setStringItems(const QStringList &items,int currentIndex);
    QStringList stringItems() const;

    void setWidgetItems(const QList<HbListWidgetItem*> &items,bool transferOwnership,int currentIndex);
    QList<HbListWidgetItem*> widgetItems() const;

    void setSelectedItems(const QList<QVariant> items);
    QList<QVariant> selectedItems() const;

    void setModel(QAbstractItemModel* model);
    QAbstractItemModel* model() const;
    QItemSelectionModel* selectionModel() const;
    QModelIndexList selectedModelIndexes() const;
    void clearItems(bool keepItems);
    void showActions(HbAbstractItemView::SelectionMode selectionMode);
    QString selectionTitle() const;
    void setSelectionTitle(const QString& title);
public:
    bool bOwnItems;
    HbAbstractItemView::SelectionMode mSelectionMode;
    void close();
    void _p_SelectionChanged();
private:
    QPointer<HbAction> action1;
    QPointer<HbAction> action2;
};

class HbSelectionDialogMarkWidget : public HbWidget
{
    Q_OBJECT
public:
    QGraphicsObject *mBackgroundItem;
    HbCheckBox* chkMark;
    QGraphicsObject* mLbCounter;
    HbSelectionDialogMarkWidget(QGraphicsItem *parent = 0);
    ~HbSelectionDialogMarkWidget();
    void createPrimitives();
    void updatePrimitives();
    void recreatePrimitives();
    void initPrimitiveData(HbStylePrimitiveData *primitiveData, const QGraphicsObject *primitive);
    QVariant itemChange( GraphicsItemChange change, const QVariant &value );
    QGraphicsItem *primitive(const QString &itemName) const;
    void updateCounter(const QString& text);
    QString mCounterText;
};

class HB_AUTOTEST_EXPORT HbSelectionDialogContentWidget :public HbWidget
{
    Q_OBJECT
    Q_PROPERTY( bool multiSelection READ multiSelection WRITE setMultiSelection )
private:
    int selectedItemCount() const;
    int totalItemCount() const;
public:
    void createListWidget();
    void createListView();
    void connectSlots();
    HbListView* mListView;
    HbSelectionDialogPrivate* d;
    HbSelectionDialogContentWidget(HbSelectionDialogPrivate *priv);
    enum { Type = Hb::ItemType_SelectionDialogContentWidget };
    int type() const { return Type; }
    void showMarkWidget(bool bShow);
    bool multiSelection(){return bMultiSelection;};
    void setMultiSelection(bool bValue){bMultiSelection = bValue;};
    bool bMultiSelection;
    void updateCounter();
    void setModel(QAbstractItemModel* model);
    QString selectionTitle() const;
    void setSelectionTitle(const QString& title);
private:
    HbSelectionDialogMarkWidget* markWidget;
    QString mSelectionTitle;
private slots:    
    void _q_listWidgetItemSelected(HbListWidgetItem *item);
    void _q_listItemSelected(QModelIndex index);
    void _q_selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void _q_checkboxclicked(int value);
    void modelChanged(const QModelIndex &parent, int start,int end);
};

class HbMarkWidget : public HbWidget
{
public:
    HbCheckBox* chkMark;
    HbTextItem* lbCounter;
};
#endif //HBSELECTIONDIALOGPRIVATE_H
