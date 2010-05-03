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

#ifndef HBSELECTIONDIALOG_H
#define HBSELECTIONDIALOG_H

#include <QAbstractItemView>
#include <hbdialog.h>
#include <hbabstractitemview.h>

class HbSelectionDialogPrivate;
class HbListWidgetItem;

QT_BEGIN_NAMESPACE

class QAbstractItemModel;

QT_END_NAMESPACE

class HB_WIDGETS_EXPORT HbSelectionDialog : public HbDialog
{
	Q_OBJECT
    Q_PROPERTY(HbAbstractItemView::SelectionMode selectionMode READ selectionMode WRITE setSelectionMode)
    Q_PROPERTY(QStringList items READ stringItems WRITE setStringItems)
    Q_PROPERTY(QList<QVariant> selectedItems READ selectedItems WRITE setSelectedItems)
    
public:
	enum { Type = Hb::ItemType_SelectionDialog };
	int type() const { return Type; }
	
	explicit HbSelectionDialog(QGraphicsItem* parent=0);
    ~HbSelectionDialog();
        
    void setSelectionMode(HbAbstractItemView::SelectionMode mode);
    HbAbstractItemView::SelectionMode selectionMode() const;
    
	void setStringItems(const QStringList &items,int currentIndex = 0);
    QStringList stringItems() const;
	
    void setWidgetItems(const QList<HbListWidgetItem*> &items,bool transferOwnership = false,int currentIndex = 0);
	QList<HbListWidgetItem*> widgetItems() const;
    
	void setModel(QAbstractItemModel* model);
	QAbstractItemModel* model() const;
	
    QList<QVariant> selectedItems() const;
    void setSelectedItems(const QList<QVariant> items);

    QModelIndexList selectedModelIndexes() const;

protected:
    void showEvent(QShowEvent *event);

private:
    Q_DISABLE_COPY(HbSelectionDialog)
    Q_DECLARE_PRIVATE_D(d_ptr, HbSelectionDialog)
};

#endif //HBSELECTIONDIALOG_H
