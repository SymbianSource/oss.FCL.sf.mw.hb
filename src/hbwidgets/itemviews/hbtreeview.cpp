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

#include "hbtreeview.h"
#include "hbtreeview_p.h"
#include "hbtreeitemcontainer_p.h"
#include "hbtreeviewitem.h"
#include "hbtreeitemselectionmodel_p.h"
#include <hbwidgetfeedback.h>
#include "hbtreemodeliterator_p.h"

#include <QItemSelection>
#include <QGraphicsSceneMouseEvent>

/*!
    @alpha
    @hbwidgets
    \class HbTreeView
    \brief HbTreeView represents a hierarchical list

    The HbTreeView class provides a default model/view implementation of a tree view.

    A HbTreeView implements a tree representation of items from a model. This class is used to provide standard hierarchical 
    lists.

    HbTreeView implements the interfaces defined by the HbAbstractItemView class to allow it to display data provided 
    by models derived from the QAbstractItemModel class. 

    It is simple to construct a tree view displaying data from a model. In the following example, the contents of a directory 
    are supplied by a QDirModel and displayed as a tree:

    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,43}

    The model/view architecture ensures that the contents of the tree view are updated as the model changes.

    Items that have children can be in an expanded (children are visible) or collapsed (children are hidden) state. 

    Each tree view item is represented by an instance of HbTreeViewItem. HbTreeView
    uses HbTreeViewItem prototype to instantiate the tree view items. HbTreeViewItem
    can be subclassed for customization purposes.
    
    By default, HbTreeView uses item recycling. This means that only the
    visible tree view items plus a small buffer of items above and below the visible
    list are instantiated at a time. When the view is scrolled the tree view items are
    recycled so that the buffer size above and below the tree view is kept constant.
*/

/*!
    Constructs a tree view with \a parent.
 */
HbTreeView::HbTreeView(QGraphicsItem *parent)
    : HbAbstractItemView(*new HbTreeViewPrivate(), new HbTreeItemContainer(), new HbTreeModelIterator(), parent)
{
    Q_D( HbTreeView );

    d->q_ptr = this;
    d->init();
}

/*!
    \private
*/
HbTreeView::HbTreeView(HbTreeViewPrivate &dd, HbAbstractItemContainer *container, QGraphicsItem *parent) :
    HbAbstractItemView(dd, container, new HbTreeModelIterator(), parent)
{
    Q_D( HbTreeView );

    d->q_ptr = this;
    d->init();
}


/*!
    Destructs the tree view.
 */
HbTreeView::~HbTreeView()
{
}

/*!
    \reimp
*/
void HbTreeView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    Q_D(HbTreeView);

    if (    !index.isValid()
        ||  index.model() != d->mModelIterator->model()) {
        return;
    }

    QModelIndex newIndex = index;
    if (!d->mContainer->itemByIndex(index)) {
        QModelIndex parentIndex = index.parent();
        QModelIndex root = rootIndex();
        if (d->isParentValid(parentIndex) || root == index) {
            while (parentIndex.isValid() && parentIndex != root) {
                if (!isExpanded(parentIndex)) {
                    newIndex = parentIndex;
                }
                parentIndex = parentIndex.parent();
            }
        }
    }

    if ( itemRecycling()) {
        bool itemBuffered = d->mContainer->itemByIndex(newIndex);
        if (! (     itemBuffered
                &&  hint == PositionAtTop)) {
            if ( hint != PositionAtTop ) {
                // Following two variable applies only for hint EnsureVisible.
                // It is position relative to the view
                bool belowBottom = false;
                bool aboveTop = false;
                if (hint == EnsureVisible) {
                    QModelIndex firstVisibleIndex;
                    QModelIndex lastVisibleIndex;
                    d->mContainer->firstAndLastVisibleModelIndex(firstVisibleIndex, lastVisibleIndex);

                    belowBottom = d->treeModelIterator()->lessThan(lastVisibleIndex, newIndex);
                    aboveTop = d->treeModelIterator()->lessThan(newIndex, firstVisibleIndex);
                }

                if (    hint == PositionAtBottom 
                    ||  hint == PositionAtCenter
                    ||  belowBottom ) {
                    // resolve index to be put to top of the recycle buffer
                    int stepCount = 0;
                    if (hint == PositionAtCenter) {
                        stepCount = d->mContainer->items().count() / 2;
                    } else {
                        stepCount = d->mContainer->items().count()-1;
                    }
                    QModelIndex topIndex = d->searchIndexUp(newIndex, stepCount);
                    d->mContainer->setModelIndexes(topIndex);
                } else if (aboveTop) {
                    d->mContainer->setModelIndexes(newIndex);
                }
                // else - hint is EnsureVisible & item is already visible
            } else {
                d->mContainer->setModelIndexes(newIndex);
            }
        }
    }
    HbAbstractItemView::scrollTo(newIndex, hint);
}

/*!
    \deprecated HbTreeView::indexCount() const
        is deprecated. Use \a HbModelIterator::indexCount() const

    \reimp

    Children of collapsed parents are not taken into account.
*/
int HbTreeView::indexCount() const
{
    qWarning("HbTreeView::indexCount() const is deprecated! Use HbModelIterator::indexCount() const.");

    return modelIterator()->indexCount();
}

/*!
    \deprecated HbTreeView::indexPosition(const QModelIndex&) const
        is deprecated. Use \a HbModelIterator::indexPosition(const QModelIndex&) const

    \reimp
*/
int HbTreeView::indexPosition(const QModelIndex &index) const
{
    qWarning("HbTreeView::indexPosition(const QModelIndex&) const is deprecated! Use HbModelIterator::indexPosition(const QModelIndex&) const.");

    return modelIterator()->indexPosition(index);
}

/*!
    \deprecated HbTreeView::nextIndex(const QModelIndex&) const
        is deprecated. Use \a HbModelIterator::nextIndex(const QModelIndex&) const

    \reimp

    Next index for valid index is determined in following way:

    - If index has children and it is expanded then first child is returned
    - Otherwise if index has next sibling then that is returned
    - Otherwise next valid sibling for parent is returned
    - Otherwise QModelIndex is returned
*/
QModelIndex HbTreeView::nextIndex(const QModelIndex &index) const
{
    qWarning("HbTreeView::nextIndex(const QModelIndex&) const is deprecated! Use HbModelIterator::nextIndex(const QModelIndex&) const.");

    return modelIterator()->nextIndex(index);
}

/*!
    \deprecated HbTreeView::previousIndex(const QModelIndex&) const
        is deprecated. Use \a HbModelIterator::previousIndex(const QModelIndex&) const

    \reimp

    Previous index for valid index is determined in following way:

    - If index has previous sibling last child from it is returned
    - Otherwise previous sibling is returned
    - Otherwise parent index is returned
    - Otherwise QModelIndex is returned
*/
QModelIndex HbTreeView::previousIndex(const QModelIndex &index) const
{
    qWarning("HbTreeView::previousIndex(const QModelIndex&) const is deprecated! Use HbModelIterator::previousIndex(const QModelIndex&) const.");

    return modelIterator()->previousIndex(index);
}

/*!
    \reimp
*/
void HbTreeView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_D(HbTreeView);

    if (d->mModelIterator->model()->columnCount(parent) == 0) {
        return;
    }

    if (d->isParentValid(parent)) {
        if (isExpanded(parent) || parent == d->mModelIterator->rootIndex()) {
            HbAbstractItemView::rowsInserted(parent, start, end);
        }

        HbAbstractViewItem *parentItem = d->mContainer->itemByIndex(parent);
        if (parentItem) {
            parentItem->updateChildItems();
        }            
    }
}

/*!
    \reimp
*/
void HbTreeView::rowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_D(HbTreeView);

    if (d->isParentValid(parent)) {
        if (parent == d->mCurrentIndex.parent() 
            && start <= d->mCurrentIndex.row()
            && d->mCurrentIndex.row() <= end) {
            // new current: 1) next after last deleted (note that
            // start and end index in model prior to deleting)
            // 2) just before first deleted
            QModelIndex newCurrentIndex = d->mModelIterator->nextIndex(d->mCurrentIndex);
            if (!newCurrentIndex.isValid()) {
                newCurrentIndex = d->mModelIterator->previousIndex(d->mCurrentIndex);
            }

            if (d->mSelectionModel) {
                d->mSelectionModel->setCurrentIndex(newCurrentIndex, QItemSelectionModel::NoUpdate);
            } 
        }  

        if (isExpanded(parent) || parent == d->mModelIterator->rootIndex()) {
            for (int i = d->mItemsToBeDeleted.count() - 1; i >= 0; --i) {
                int pos = d->mItemsToBeDeleted.at(i);
                d->mContainer->removeItem(pos);
            }
        }

        HbAbstractViewItem *parentItem = d->mContainer->itemByIndex(parent);
        if (parentItem) {
            parentItem->updateChildItems();
        }
    }

    d->mItemsToBeDeleted.clear();
}

/*!
    \reimp
*/
void HbTreeView::rowsAboutToBeRemoved(const QModelIndex &index, int start, int end)
{
    Q_D(HbTreeView);

    d->mItemsToBeDeleted.clear();
        
    QList <HbAbstractViewItem *> items = d->mContainer->items();
    int itemCount = items.count();

    // Add the view items given as parameters and all their child items to a list for deletion.    
    for (int i = start; i <= end; ++i) {
        QModelIndex parent = model()->index(i, 0, index);
        
        for (int j = 0; j < itemCount; ++j) {
            QModelIndex itemIndex = items.at(j)->modelIndex();
            if (itemIndex == parent || d->isChild(itemIndex, parent)) {
                d->mItemsToBeDeleted.append(j);
            }
        }
    }
}

/*!
    \reimp
*/
void HbTreeView::columnsInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(end)
    Q_UNUSED(start)

    Q_D(HbTreeView);

    if (d->isParentValid(parent)) {
        if (d->mModelIterator->model()->columnCount(parent) == 1){
            rowsInserted(parent, 0, d->mModelIterator->indexCount(parent));
        }
    }
}

/*!
    \reimp
*/
void HbTreeView::columnsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(end)
    Q_UNUSED(start)

    Q_D(HbTreeView);

    if (d->isParentValid(parent)) {
        if (d->mModelIterator->model()->columnCount(parent) == 0){
            rowsRemoved(parent, 0, d->mModelIterator->indexCount(parent));
        }
    }

}

/*!
    \reimp
*/
void HbTreeView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_D(HbTreeView);
    if (d->isParentValid(topLeft.parent())) {
        HbAbstractItemView::dataChanged(topLeft, bottomRight);
    }
}


/*!
    \reimp
    This function is involded in partial selecting and deselecting parent items.
    Selecting items are implemented by internal selection model class.
*/
void HbTreeView::currentSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_D(HbTreeView);
    bool selectionRoot = false;
    if (!d->mSelectionStarted) {
        d->mSelectionStarted = true;
        selectionRoot = true;
    }

    HbAbstractItemView::currentSelectionChanged(selected, deselected);

    if (selectionRoot) {
        QModelIndexList selectedIndexes(selected.indexes());
        QMap<QModelIndex, Qt::CheckState> parentItems;

        int count = selectedIndexes.count();
        for (int i = 0; i < count; ++i) {
            QModelIndex index = selectedIndexes.at(i);

            // Parents are always partially selected when one of the
            // children is selected.
            QModelIndex parentIndex = index.parent();
            if (!parentItems.contains(parentIndex)) {
                while (parentIndex != d->mModelIterator->rootIndex()) {
                    if (!d->mSelectionModel->isSelected(parentIndex)) {
                        parentItems.insert(parentIndex, Qt::PartiallyChecked);
                    } 
                    // else - occurs when node is selected. Node
                    // may come first in the selection list and only after that its children
                    parentIndex = parentIndex.parent();
                }
            }
        }

        QModelIndexList deselectedIndexes(deselected.indexes());
        QItemSelection deselectedChildren;

        count = deselectedIndexes.count();
        for (int i = 0; i < count; ++i) {
            QModelIndex index = deselectedIndexes.at(i);
            bool solveParentCheckState = true;

            // Solves the partial selection/non-selection state for the parents
            // by checking status of siblings. Parent will PartiallyChecked, if
            // sibling is either checked or partially checked.

            // There is one exception, which this slot cannot handle. It is handled by HbTreeItemSelectionModel::select().
            // See its documentation
            QModelIndex parentIndex = index.parent();
            while (parentIndex != d->mModelIterator->rootIndex()) {
                Qt::CheckState parentCheckState = Qt::PartiallyChecked;
                if (solveParentCheckState) {
                    int childCount = model()->rowCount(parentIndex);
                    for (int current = 0; current < childCount; ++current) {
                        QModelIndex childIndex = parentIndex.child(current, parentIndex.column());

                        // Resolves child check state.
                        Qt::CheckState childCheckState = Qt::Unchecked;
                        if (d->mSelectionModel->isSelected(childIndex)) {
                            childCheckState = Qt::PartiallyChecked;
                        } else if (parentItems.contains(childIndex)) {
                            // handles use case: node1, which has child node2, which has child leaf1.
                            // Initially node1 is selected, then leaf1 is selected.
                            // Node2 may have been set Unchecked into map before node1.
                            // When node1 will be handled, node2 is unchecked in the map but not yet in the view item
                            childCheckState = parentItems.value(childIndex);
                        } else {
                            QVariant value = d->mContainer->itemState(childIndex).value(HbTreeViewItem::CheckStateKey);
                            if (value.isValid()) {
                                childCheckState = (Qt::CheckState)value.toInt();
                            }
                        }
                        if (childCheckState != Qt::Unchecked) {
                            // No need to solve further parents check state as
                            // one child item is selected -> all parents are partially
                            // selected.
                            solveParentCheckState = false;
                            break;
                        }

                        if (current == childCount - 1) {
                            parentCheckState = Qt::Unchecked;
                        }
                    }   
                }
                if (!parentItems.contains(parentIndex)) {
                    // handling use case: parent initially multi selected, then a leaf is multi deselected
                    QItemSelection selection(parentIndex, parentIndex);
                    // Using HbTreeItemSelectionModel must be bypassed, because it would deselect every children, too
                    d->mSelectionModel->QItemSelectionModel::select(selection, QItemSelectionModel::Deselect);
                }
                parentItems.insert(parentIndex, parentCheckState);
                parentIndex = parentIndex.parent();
            }
        }

        QMap<QModelIndex, Qt::CheckState>::iterator iterator = parentItems.begin();
        while (iterator != parentItems.end()) {
            HbAbstractViewItem *item = d->mContainer->itemByIndex(iterator.key());
            if (item) {
                item->setCheckState(iterator.value());
            } 

            d->mContainer->setItemStateValue(iterator.key(), HbAbstractViewItem::CheckStateKey, iterator.value());
            iterator++;
        }

        d->mSelectionStarted = false;
    }
}

/*!
    Sets the item referred to by \a index to either collapse or expanded, depending on the value of \a expanded.

    \sa isExpanded
*/
void HbTreeView::setExpanded(const QModelIndex &index, bool expanded)
{
    Q_D(HbTreeView);

    if (isExpanded(index) != expanded) {
        d->treeModelIterator()->itemStateChanged(index, HbTreeViewItem::ExpansionKey);
        HbTreeItemContainer *container = qobject_cast<HbTreeItemContainer *>(d->mContainer);
        container->setExpanded(index, expanded);
    }
}

/*!
    Returns true if the model item \a index is expanded; otherwise returns false.

    \sa setExpanded
*/
bool HbTreeView::isExpanded(const QModelIndex &index) const
{
    Q_D(const HbTreeView);

    HbTreeItemContainer *container = qobject_cast<HbTreeItemContainer *>(d->mContainer);
    return container->isExpanded(index);
}
/*!
    Overrides default indentation of tree view items defined in style sheet.
    Items, which are not immediate children of root item are indented by
    indentation * level pixels. 
    Level is count of parent items between item itself and root item.

    If negative value is set, then indentation from style sheet is used.

    \sa indentation
*/
void HbTreeView::setIndentation(qreal indentation)
{
    Q_ASSERT_X(qobject_cast<HbTreeItemContainer*>(container()), "HbTreeView::setIndentation", "wrong container type");
    qobject_cast<HbTreeItemContainer*>(container())->setIndentation(indentation);
}

/*!
    Returns indentation of tree view items. 
    The returned value is either default value or set by setIndentation().

    Default value is -1. In this case indentation from style sheet is used.

    \sa setIndentation
*/
qreal HbTreeView::indentation() const
{
    Q_ASSERT_X(qobject_cast<HbTreeItemContainer*>(container()), "HbTreeView::indentation", "wrong container type");
    return qobject_cast<HbTreeItemContainer*>(container())->indentation();
}

/*!
    \reimp
*/
void HbTreeView::reset()
{
    HbAbstractItemView::reset();
}


/*!
    \reimp
*/
void HbTreeView::setRootIndex(const QModelIndex &index)
{
    //Q_D(HbTreeView);
    HbAbstractItemView::setRootIndex(index);
    setExpanded(index, true); 
}

/*!
    This function handles expanding and collapsing parent items in tree view. 
    Parent item expands and collapses, when pressed down is released
    exluding following use cases:
    \li HbTreeViewItem::selectionAreaContains() has returned true for pressed down
    \li view is panned or scrolled 

    \sa HbTreeViewItem::selectionAreaContains() 
*/
void HbTreeView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(HbTreeView);

    HbTreeViewItem* hitItem = qobject_cast<HbTreeViewItem*>(d->itemAt(event->scenePos()));

    // check whether expansion collapsion can be done
    if (    d->mHitItem
        &&  d->mHitItem == hitItem
        &&  hitItem->primitive(HbStyle::P_TreeViewItem_expandicon)
        &&  hitItem->flags().testFlag(QGraphicsItem::ItemIsFocusable)
        &&  !d->mWasScrolling
        &&  (   d->mSelectionMode == SingleSelection
            || !d->mSelectionSettings.testFlag(HbAbstractItemViewPrivate::Selection))) {
        Hb::InteractionModifiers modifiers = 0;
        if (hitItem->isExpanded()) {
            d->mInstantClickedModifiers |= Hb::ModifierExpandedItem;
        } else {
            d->mInstantClickedModifiers |= Hb::ModifierCollapsedItem;
        }
        hitItem->setExpanded(!hitItem->isExpanded());
    }

    HbAbstractItemView::mouseReleaseEvent(event);
}

/*!
    \reimp
    Tree view has its internal selection model. Implementation of this virtual function is needed
    to take it into use with new model..
*/
void HbTreeView::setModel(QAbstractItemModel *model, HbAbstractViewItem *prototype)
{
    Q_D(HbTreeView);
    HbAbstractItemView::setModel(model, prototype);
    setSelectionModel(new HbTreeItemSelectionModel(model, d, this));
}

#include "moc_hbtreeview.cpp"

