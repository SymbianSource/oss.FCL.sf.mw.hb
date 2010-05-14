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
    
#include "hbtreeitemselectionmodel_p.h"

#include "hbabstractitemview_p.h"
#include "hbtreeviewitem.h"
#include "hbabstractitemcontainer_p.h"
#include "hbmodeliterator.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QItemSelectionRange>

#include <QDebug>

HbTreeItemSelectionModel::HbTreeItemSelectionModel( QAbstractItemModel          *model, 
                                                    HbAbstractItemViewPrivate   *viewPrivate,
                                                    QObject                     *parent) :
    QItemSelectionModel(model, parent),
    mViewPrivate(viewPrivate)
{
    Q_ASSERT_X(mViewPrivate, "HbTreeItemSelectionModel::HbTreeItemSelectionModel", "HbAbstractItemView is 0");
}

HbTreeItemSelectionModel::~HbTreeItemSelectionModel()
{
}

void HbTreeItemSelectionModel::addChildToSelection(QItemSelection &selection, const QModelIndex &index)
{
    int childCount = model()->rowCount(index);
    for (int current = 0; current < childCount; ++current) {
        QModelIndex childIndex = model()->index(current, 0, index);
        addChildToSelection(selection, childIndex);
    }
    if (childCount > 0) {
        selection.select(model()->index(0, 0, index),
                         model()->index(childCount-1, 0, index));
    }
}

/*
    This function and HbTreeView::currentSelectionChanged() are involded in selection in treeview.
    This function selects indexed item and every it children.
    HbTreeView::currentSelectionChanged() handles unselection of deleselected items and partial selection
    of parents.
    There is one exception case to partial selection of parents. This exception case is implemented in this function.
    The exception case is:
      Case is 1) node is single selected, which has one child 2) then its child is single selected
      2) causes HbTreeView::currentSelectionChanged() to be called by with
      no selected items and the node as deselected item. The slot does not have no clue
      which item is selected (except by asking from model directly)
*/
void HbTreeItemSelectionModel::select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command )
{
    if (command == NoUpdate) {
        return;
    }

    QModelIndexList indexes = selection.indexes();
    int count = indexes.count();
    for (int i = 0; i < count; ++i) {
        QModelIndex index = indexes.at(0);
        if (!index.isValid()) {
            return;
        }

        QItemSelection selectedItems;
        selectedItems.append(QItemSelectionRange(index, index));
        addChildToSelection(selectedItems, index);

        // Toggle is not allowed. All of the items are either selected or deselected.
        // First item determines, which one is selected
        bool toggling = command & QItemSelectionModel::Toggle;
        bool selected = isSelected(index);
        if (toggling) {
            command &= ~QItemSelectionModel::Toggle;
            if (selected) {
                command |= QItemSelectionModel::Deselect;
            } else {
                command |= QItemSelectionModel::Select;
            }
        }

        QItemSelectionModel::select(selectedItems, command);

        // Partial selection exception (see function comment):
        if ( mViewPrivate->mSelectionMode == HbAbstractItemView::SingleSelection ) {
            QModelIndex parentIndex = index.parent();
            if (model()->rowCount(parentIndex) == 1) {
                while (parentIndex != mViewPrivate->mModelIterator->rootIndex()) {
                    if (!isSelected(parentIndex)) {
                        HbAbstractViewItem *item = mViewPrivate->mContainer->itemByIndex(parentIndex);
                        if (item) {
                            item->setCheckState(Qt::PartiallyChecked);
                        } 
                        mViewPrivate->mContainer->setItemTransientStateValue(parentIndex, "checkState", Qt::PartiallyChecked);
                    }
                    parentIndex = parentIndex.parent();
                }
            }
        }
    }
}

