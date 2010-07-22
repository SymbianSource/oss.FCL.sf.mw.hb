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

#include "hbabstractitemview.h"
#include "hbabstractitemview_p.h"
#include "hbabstractitemcontainer_p_p.h"

#include <hbabstractviewitem.h>
#include <hbevent.h>
#include <hbabstractitemcontainer_p.h>
#include <hbwidgetfeedback.h>
#include <hbinstance.h>
#include <hbscrollbar.h>
#include <hbmodeliterator.h>

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QDebug>

#include <QGesture>
#include <QGestureEvent>
#include <QPanGesture>


/*!
    @alpha
    @hbwidgets
    \class HbAbstractItemView
    \brief HbAbstractItemView provides basic functionality for item View Classes.  
    
    HbAbstractItemView is an abstract class and cannot itself be instantiated.
    HbAbstractItemView class can be used as the base class for every standard view that uses QAbstractItemModel.
    It provides a standard interface for interoperating with models(QAbstractItemModel and QItemSelectionModel)
    through signals and slots mechanism, enabling subclasses to be kept up-to-date with changes to their models.
    This class provides a default selection Model to work with. Application can set its own selection Model.
    There are several functions concerned with selection control like clearSelection(), selectAll(), setSelectionMode().
    HbAbstractItemView provides standard support for mouse navigation and scrolling of items,
    selections.

    \b Subclassing
    HbAbstractItemView can be subclassed for customization purposes. Derived class must provide
    implementation of scrollTo(). 

    Each view item is represented by an instance of HbAbstractViewItem. HbAbstractViewItem
    can be subclassed for customization purposes.

    HbAbstractItemView can use item recycling. This means that only visible items
    plus a small buffer of items above and below the visible area are instantiated at a time. 
    When the view is scrolled view items are recycled so that buffer size above and below the 
    visible area is kept constant.By default this feature is disabled. 
*/

/*!
    \fn void HbAbstractItemView::pressed(const QModelIndex &index)

    This signal is emitted when a touch down event is received within
    view item that is representing \a index.

    See also released() and activated().
*/

/*!
  \fn void HbAbstractItemView::released(const QModelIndex &index)

    This signal is emitted when a release event is received within
    Abstract view item that is representing \a index.

    See also pressed() and activated().
    \sa HbAbstractViewItem::released(const QPointF &position)
*/

/*!
    \fn void HbAbstractItemView::activated(const QModelIndex &index)

    This signal is emitted when the item specified by \a index is activated by the user.
    How to activate items depends on the input method; e.g., with mouse by clicking the item
    or with touch input by tapping the item.

    See also pressed() and released().
*/

/*!
    \fn void HbAbstractItemView::longPressed(HbAbstractViewItem *item, const QPointF &coords)

    This signal is emitted when long press event is received within
    Abstract view item \a viewItem. \a coords is scene position where the long press event happened.
*/

/*!
    \enum HbAbstractItemView::SelectionMode

    selection types supported by HbAbstractItemView.

    This enum describes different selection types available in LibHb.
*/

/*!
    \var HbAbstractItemView::NoSelection

    Items cannot be selected. This is the default value.
*/


/*!
    \var HbAbstractItemView::SingleSelection

    When the user selects an item, any already-selected item becomes unselected, and the user cannot
    unselect the selected item. 
*/

/*!
    \var HbAbstractItemView::MultiSelection

    When the user selects an item in the usual way, the selection state of that item
    is toggled and the other items are left alone. 
*/

/*!
    \enum HbAbstractItemView::ScrollHint

    Enumeration specifies positions on the view, onto which an item can be scrolled. 

    \sa scrollTo(const QModelIndex &index, HbAbstractItemView::ScrollHint hint = EnsureVisible)
*/

/*!
    \var HbAbstractItemView::EnsureVisible

    Item will be fully visible somewhere on the view. 
*/


/*!
    \var HbAbstractItemView::PositionAtTop

    Item will be fully visible as topmost item.
*/


/*!
    \var HbAbstractItemView::PositionAtBottom

    Item will be fully visible as bottommost item.
*/

/*!
    \var HbAbstractItemView::PositionAtCenter

    Item will be centered. 
*/

/*!
    \enum HbAbstractItemView::ItemAnimation

    animations in HbAbstractItemView that can be disabled. By default all animations are enabled.
*/

/*!
    \var HbAbstractItemView::None

    Any animation is not applied. 
*/

/*!
    \var HbAbstractItemView::Appear

    Animation related to item appearance. Disable this animation in cases you expect many model items to appear,
    for example in case like insertion of a new data source, and you do not wish to see animations.
    
    Note that the item appearance animations are not used directly after a setModel call to force non-animated model change. 
*/

/*!
    \var HbAbstractItemView::Disappear

    Animation related to item removal. Disable this animation in cases you expect many model items to disappear,
	for example in cases like filtering or removal of a data source, and you do not wish to see animations.
*/

/*!
    \var HbAbstractItemView::TouchDown

    Animation related to item press and release.
*/

/*!
    \var HbAbstractItemView::Expand

    Animation related to setting item expand.
*/

/*!
    \var HbAbstractItemView::Collapse

    Animation related to setting item collapsed.
*/

/*!
    \var HbAbstractItemView::All

    Every animation is applied. 
*/

/*!
    Here are the main properties of the class:

    \li HbAbstractItemView::itemRecycling: ItemRecycling.
    \li HbAbstractItemView::SelectionMode: Different selection types supported by view.
    \li HbAbstractItemView::bufferSize   : Buffer Size used for item recycling.
    \li HbAbstractItemView::uniformItemSizes : This property holds whether all items in the item view have the same size.
*/


/*!
    \fn void HbAbstractItemView::scrollTo(const QModelIndex &index,
        HbAbstractItemView::ScrollHint hint = EnsureVisible)

    Scrolls the view if necessary to ensure that the item at \a index is visible
    in a position according to \a hint. Default value just guarantees, that item will be fully visible. 

    Scrolling is not animated but movement is immediate. 
*/


/*!
    See also HbAbstractItemManager, HbAbstractViewItem, HbAbstractItemContainer.

*/

/*!
    Constructs a new HbAbstractItemView with \a parent.
*/
HbAbstractItemView::HbAbstractItemView(HbAbstractItemViewPrivate &dd, 
                                       HbAbstractItemContainer *container,
                                       HbModelIterator *modelIterator,
                                       QGraphicsItem *parent)
    : HbScrollArea(dd, parent)
{
    Q_D(HbAbstractItemView);
    Q_ASSERT_X(container, "HbAbstractItemView constructor", "Container is null");

    d->q_ptr = this;
    d->init(container, modelIterator);
}

/*!
    Constructs a new HbAbstractItemView with \a parent.
*/
HbAbstractItemView::HbAbstractItemView(HbAbstractItemContainer *container,
                                       HbModelIterator *modelIterator,
                                       QGraphicsItem *parent)
    : HbScrollArea(*new HbAbstractItemViewPrivate, parent)
{
    Q_D(HbAbstractItemView);
    Q_ASSERT_X(container, "HbAbstractItemView constructor", "Container is null");

    d->q_ptr = this;
    d->init(container, modelIterator);
}

/*!
    Destructs the abstract item view.
*/
HbAbstractItemView::~HbAbstractItemView()
{

}

/*!
    Returns model that view is currently presenting.
*/
QAbstractItemModel *HbAbstractItemView::model() const
{
    Q_D(const HbAbstractItemView);
    return d->mModelIterator->model();
}

/*!
    Sets the model to \a model and replaces current item prototype with \a prototype.
    Ownership of the model is not taken. Ownership of the prototype is taken. 
    If no prototype has been passed, default prototype is used.

    Note! Itemview may create view items asynchronously.
*/
void HbAbstractItemView::setModel(QAbstractItemModel *model, HbAbstractViewItem *prototype)
{
    Q_D(HbAbstractItemView);
    if(prototype) {
        setItemPrototype(prototype);
    }
    d->setModel(model);
}

/*!
    Returns the list of item prototypes.
*/
QList<HbAbstractViewItem *> HbAbstractItemView::itemPrototypes() const
{
    Q_D(const HbAbstractItemView);
    return d->mContainer->itemPrototypes();
}

/*!
    Replaces current item prototypes with \a prototype. Ownership is taken.
    
    Concrete item views provided by library have view specific view item prototype set by default.
    
    Note! This function may cause that view items are recreated. They may be created asynchronously.

*/
void HbAbstractItemView::setItemPrototype(HbAbstractViewItem *prototype)
{
    Q_D(HbAbstractItemView);
    if (prototype && d->mContainer->setItemPrototype(prototype))  {
        d->resetContainer();
    }
}

/*!
    To support multiple Abstractview items within single AbstractItemview.
    Replace current item prototypes with list of \a prototypeList. Ownership is taken.
    Setting more than one prototype will disable item recycling feature.

    When list view item is being created, HbAbstractViewItem::canSetModelIndex()
    is called for every item until item is found, which can create an item for
    a model index. The prototype list is gone through from end to the beginning. 
    
    Thus specialized prototypes should be in the end of the list and 
    'default' prototype first one. The specialized prototypes usually can create
    only certain types of list view items. The default prototype usually return always true,
    meaning that it can create any type of list view item. 

    Concrete item views provided by library have view specific view item prototype set.

    Note! This function may cause that view items are recreated. They may be created asynchronously.

    \sa HbAbstractViewItem::canSetModelIndex()
*/
void HbAbstractItemView::setItemPrototypes(const QList<HbAbstractViewItem *> &prototypes)
{
    Q_D(HbAbstractItemView);
    if(prototypes.count() > 0) {
        if (d->mContainer->setItemPrototypes(prototypes)) {
            d->resetContainer();
        }
    }
    
}

/*!
    Returns the current selection model.
*/
QItemSelectionModel *HbAbstractItemView::selectionModel() const
{
    Q_D(const HbAbstractItemView);
    return d->mSelectionModel;
}

/*!
    Sets the current selection model to \a selectionModel.
    Note: If setModel() is called after this function, the given selectionModel will be
    replaced by default selection model of the view.
*/
void HbAbstractItemView::setSelectionModel(QItemSelectionModel *selectionModel)
{
    Q_D(HbAbstractItemView);
    if (selectionModel 
        &&  d->mSelectionModel != selectionModel) {
        d->setSelectionModel(selectionModel);
    }
}

/*!
    If \a newMode is not same as current selection mode of view,
    updates selection mode, all viewitems and clears all existing selections.
*/
void HbAbstractItemView::setSelectionMode(SelectionMode newMode)
{
    Q_D(HbAbstractItemView);
    if (d->mSelectionMode != newMode) {
        d->mSelectionMode = newMode;

        clearSelection();

        d->updateItems();
    }
}

/*!
    Selects all items in the view.
*/
void HbAbstractItemView::selectAll()
{
    Q_D(HbAbstractItemView);
    QAbstractItemModel *model = d->mModelIterator->model();
    if (    model
        &&  d->mSelectionModel 
        &&  d->mSelectionMode == MultiSelection) {
        QModelIndex rootIndex = d->mModelIterator->rootIndex();
        QModelIndex firstIndex = model->index(0, 0, rootIndex);
        QModelIndex lastIndex = model->index(model->rowCount(rootIndex)-1, 0, rootIndex);
        d->mSelectionModel->select(QItemSelection(firstIndex, lastIndex), QItemSelectionModel::Select);
    }
}

/*!
    Deselects all selected items. The current index will not be changed.
*/
void HbAbstractItemView::clearSelection()
{
    Q_D(HbAbstractItemView);
    if (d->mSelectionModel) {
        d->mClearingSelection = true;
        d->mSelectionModel->clearSelection();
        d->mClearingSelection = false;
    }
}

/*!
    Returns current selection mode used by view.
*/
HbAbstractItemView::SelectionMode HbAbstractItemView::selectionMode() const
{
    Q_D(const HbAbstractItemView);
    return d->mSelectionMode;
}

/*!
    Returns index of current item.
*/
QModelIndex HbAbstractItemView::currentIndex() const
{
    Q_D(const HbAbstractItemView);
    return d->mCurrentIndex;
}

/*!
   Sets current index to \a index. The item is selected depending on the \a selectionFlag.
   By default item is not selected. If current selection mode is NoSelection,
   item is not selected irrespective of the selection flag.
*/
void HbAbstractItemView::setCurrentIndex(const QModelIndex &index, 
                                         QItemSelectionModel::SelectionFlags selectionFlag)
{
    Q_D(HbAbstractItemView);
    if (d->mSelectionModel) {
        if (d->mSelectionMode == NoSelection) {
            selectionFlag = QItemSelectionModel::NoUpdate;
        } else if (d->mSelectionMode == SingleSelection
            && selectionFlag & QItemSelectionModel::Select) {
            selectionFlag |= QItemSelectionModel::Clear;
        }

        d->mSelectionModel->setCurrentIndex(index, selectionFlag);
    }
}


/*!
    Returns model index of the model's root item. 
    The root item is parent item to view's top level items.
    The root can be invalid.
*/
QModelIndex HbAbstractItemView::rootIndex() const
{
    Q_D(const HbAbstractItemView);
    return d->mModelIterator->rootIndex();
}

/*!
    Sets root index to \a index.
    All view items are deleted and recreated

    Note! View items may be created asynchronously.
*/
void HbAbstractItemView::setRootIndex(const QModelIndex &index)
{
    Q_D(HbAbstractItemView);
    if (d->mModelIterator->rootIndex() != index ) {
        d->mModelIterator->setRootIndex(index);
        d->resetContainer();

        setCurrentIndex(QModelIndex(), QItemSelectionModel::NoUpdate);
    }
}

/*!
    Resets Item view.
*/
void HbAbstractItemView::reset()
{
    Q_D(HbAbstractItemView);    

    d->mModelIterator->setRootIndex(QPersistentModelIndex());
    d->resetContainer();

    setCurrentIndex(QModelIndex(), QItemSelectionModel::NoUpdate);
}

/*!
    \reimp
    It should be always called by child class if overridden.
*/
bool HbAbstractItemView::event(QEvent *e)
{
    Q_D(HbAbstractItemView);

    bool result = HbScrollArea::event(e);

    // The two above statements have to be executed before these
    if (e->type() == HbEvent::ChildFocusIn) {
        HbAbstractViewItem *item = 0;
        QList<HbAbstractViewItem *> items = d->mContainer->items();

        for (QGraphicsItem *currentItem = scene()->focusItem(); currentItem != 0; currentItem = currentItem->parentItem()) {
            item = d->viewItem(currentItem);
            if (item) {
                if (items.indexOf(item) == -1) {
                    item = 0;
                } else {
                    break;
                }
            }
        }
        if (item && item->modelIndex() != d->mCurrentIndex) {
            setCurrentIndex(item->modelIndex());
        }
        result = true;
    } else if (e->type() == QEvent::LayoutRequest) {
        d->mContainer->resizeContainer();
        if (d->mPostponedScrollIndex.isValid()) { 
           d->scrollTo(d->mPostponedScrollIndex, d->mPostponedScrollHint);
        } 
        result = true;
    }

    return result;
}

/*!
    \reimp

    This slot is called when orientation is changed.
    \a newOrientation has the currentOrientation mode.
    Note: Currently platform dependent orientation support is not available
*/
void HbAbstractItemView::orientationChanged(Qt::Orientation newOrientation)
{
    Q_UNUSED(newOrientation);

    Q_D(HbAbstractItemView);


    //Setting the uniform ites sizes to container again resets size caches.
    d->mContainer->setUniformItemSizes(d->mContainer->uniformItemSizes());
    d->mContainer->setPos(0,0);
    d->mContainer->resizeContainer();

    d->updateScrollMetrics();

    d->stopAnimating();
    scrollTo(d->mVisibleIndex, HbAbstractItemView::PositionAtTop);

    d->mVisibleIndex = QModelIndex();
}

/*!
    This slot is called just before orientation is to be changed.
    Note: Currently platform dependent orientation support is not available
*/
void HbAbstractItemView::orientationAboutToBeChanged()
{
    Q_D(HbAbstractItemView);
    d->saveIndexMadeVisibleAfterMetricsChange();
}

/*!
    Sets item recycling to \a enabled.
    By default recycling is off.
*/
void HbAbstractItemView::setItemRecycling(bool enabled)
{
    Q_D(HbAbstractItemView);
    d->mContainer->setItemRecycling(enabled);
}

/*!
    Returns whether item recycling feature is in use.
*/
bool HbAbstractItemView::itemRecycling() const
{
    Q_D(const HbAbstractItemView);
    return d->mContainer->itemRecycling();
}

/*!
    Returns view item representing current model index. This can be NULL if
    index has no view item representing it.
*/
HbAbstractViewItem *HbAbstractItemView::currentViewItem() const
{
    Q_D(const HbAbstractItemView);
    return d->currentItem();
}


/*!
    Returns view item representing given model \a index. This can be NULL if
    index has no view item representing it.
*/
HbAbstractViewItem *HbAbstractItemView::itemByIndex(const QModelIndex &index) const
{
    Q_D(const HbAbstractItemView);
    return d->mContainer->itemByIndex(index);
}

/*!
   Returns true if item with model index is fully or partially visible in view.
*/
bool HbAbstractItemView::isVisible(const QModelIndex & index) const
{
    Q_D( const HbAbstractItemView );
    return d->visible(d->mContainer->itemByIndex(index), false);
}

/*!
    Base class implemmentation. Take care about scrollTo when it was
    called before view was visible (first scrollTo can be done after
    first layoutRequest).
    It should be always called by child class if overridden.

    Note! If item recycling is enabled, view item may not have reached its final 
    position, when this function returns. Then its position is fine tuned asynchronously. 

    \sa HbAbstractItemView::itemRecycling()
*/
void HbAbstractItemView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    Q_D(HbAbstractItemView);
    if (    index.isValid()
        &&  d->mModelIterator->model() == index.model()) {
        d->mPostponedScrollIndex = QPersistentModelIndex();
        d->scrollTo(index, hint);
        d->mPostponedScrollIndex = index;
        d->mPostponedScrollHint = hint;
        if (    d->mContainer->itemRecycling()
            &&  !d->mContainer->items().count()) {
            // settings index from which loading viewitems start when itemview is 
            // empty or reset by setModel()
            d->mContainer->d_func()->mFirstItemIndex = index;
        }
        QCoreApplication::postEvent(this, new QEvent(QEvent::LayoutRequest));
    }
}

/*!
    Returns list of currently visible view items.
*/
QList<HbAbstractViewItem *> HbAbstractItemView::visibleItems() const
{
    Q_D(const HbAbstractItemView);
    QList<HbAbstractViewItem *> visibleItems;

    QList<HbAbstractViewItem *> containerItems = d->mContainer->items();
    const int count(containerItems.count());
    for (int i = 0; i < count; ++i) {
        if(d->visible(containerItems.at(i), false))
            visibleItems.append(containerItems.at(i));
    }
    return visibleItems;
}

/*!
    Returns SelectionFlags to be used when updating selection of a item.
    The event is a user input event, such as a mouse or keyboard event.
    contiguousArea is true, if selectiontype is not single or no selection and
    user has pressed on contiguousArea of viewItem i.e CheckBox.
    By default this is false.
    Subclasses should overide this function to define their own selection behavior.

    Note: The \a event parameter is not necessary a full event. For mouse events
    it is quaranteed to contain the event type and position.

    \sa HbAbstractViewItem::selectionAreaContains(const QPointF &scenePosition) const
*/
QItemSelectionModel::SelectionFlags HbAbstractItemView::selectionCommand(const HbAbstractViewItem *item,
                                                                         const QEvent *event)
{ 
    Q_D(HbAbstractItemView);
    // When the parameter contiguousArea is removed, copy paste 'd ->selectionFlags()' into here.
    return d->selectionFlags(item, event);
}

/*!
    This slot is called when items are changed in model. 
    The changed items are those from \a topLeft to \a bottomRight inclusive.
    If just one item is changed topLeft == bottomRight.
*/
void  HbAbstractItemView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_D(HbAbstractItemView);

    QList<HbAbstractViewItem *> items = d->mContainer->items();
    int itemCount = items.count();

    if (itemCount != 0
        && topLeft.isValid() 
        && bottomRight.isValid()
        && topLeft.parent() == bottomRight.parent()) {
        HbAbstractViewItem *startItem = itemByIndex(topLeft);
        if (topLeft != bottomRight) {
            // Multiple indexes have changed.
            int start = 0;
            if (startItem) {
                start = items.indexOf(startItem);
            }

            int end = itemCount-1;
            
            HbAbstractViewItem *endItem = itemByIndex(bottomRight);
            if (endItem) {
                end = items.indexOf(endItem, start+1);
            }

            for (int current = start; current <= end; current++) {
                HbAbstractViewItem *item = items.at(current);
                QModelIndex index = item->modelIndex();
                int currentRow = index.row();
                int currentColumn = index.column();

                if (index.parent() == topLeft.parent()
                    && currentRow >= topLeft.row()
                    && currentRow <= bottomRight.row()
                    && currentColumn >= topLeft.column()
                    && currentColumn <= bottomRight.column()) {
                    item->updateChildItems();
                }
            }
        } else {
            // Single index has changed.
            if (startItem) {
                startItem->updateChildItems();
            }
        }
    }
}

/*!
    This slot is called when current index is changed in selection model.
    current is the changed or current index and previous in the old current index.
    current index may not be selected.
*/
void HbAbstractItemView::currentIndexChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_D(HbAbstractItemView);

    if (current != d->mCurrentIndex) {
        d->mCurrentIndex = current;
               
        if (previous.isValid()) {
           d->mContainer->setItemTransientStateValue(previous, "focused", false);
        }
        
        if (d->mCurrentIndex.isValid()) {
            d->mContainer->setItemTransientStateValue(d->mCurrentIndex, "focused", true);
        }

    }
}

/*!
    This slot is called when selection of items has changed.
    selected contains all selected items, deselected contains
    all deselected items.
*/
void HbAbstractItemView::currentSelectionChanged(const QItemSelection &selected, 
                                                 const QItemSelection &deselected)
{
    Q_D(HbAbstractItemView);
    QModelIndexList selectedIndexes(selected.indexes());
    int count = selectedIndexes.count();
    for (int i = 0; i < count; ++i) {
        HbAbstractViewItem *item = d->mContainer->itemByIndex(selectedIndexes.at(i));
        if (item) {
            item->setCheckState(Qt::Checked);
            if (!d->mClearingSelection && !d->mDoingContiguousSelection) {
                HbWidgetFeedback::triggered(item, Hb::InstantSelectionChanged); 
            }
        } 
        d->mContainer->setItemTransientStateValue(selectedIndexes.at(i),
                                         "checkState",
                                         Qt::Checked);
    }

    QModelIndexList deselectedIndexes(deselected.indexes());
    count = deselectedIndexes.count();
    for (int i = 0; i < count; ++i) {
        HbAbstractViewItem *item = d->mContainer->itemByIndex(deselectedIndexes.at(i));
        if (item) {
            item->setCheckState(Qt::Unchecked);
            if (!d->mClearingSelection && !d->mDoingContiguousSelection) {
                HbWidgetFeedback::triggered(item, Hb::InstantSelectionChanged);
            }
        } 
        d->mContainer->setItemTransientStateValue(deselectedIndexes.at(i),
                                         "checkState",
                                         Qt::Unchecked);
    }
}

/*!
    This slot is called when rows are inserted. 
    The new rows are those under the given parent from start to end inclusive
*/
void HbAbstractItemView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_D(HbAbstractItemView);

    if (d->mModelIterator->model()->columnCount(parent) == 0) {
        return;
    }

    for (int current = start; current <= end; current++) {
        QModelIndex index = model()->index(current, 0, parent);
        d->mContainer->addItem(index, d->animationEnabled(true));
    }
    

    if (!d->mCurrentIndex.isValid() && d->mSelectionModel) {
        d->mSelectionModel->setCurrentIndex(d->mModelIterator->nextIndex(QModelIndex()), QItemSelectionModel::NoUpdate);
    }
}

/*!
    This slot is called after rows have been removed from the model.
    The removed items are those between start and end inclusive, under the given parent item.
*/
void HbAbstractItemView::rowsRemoved(const QModelIndex &parent,int start,int end)
{
    Q_D(HbAbstractItemView);
    d->rowsRemoved(parent, start, end);
}

/*!
    This slot is called just before rows are removed from the model.
    The items that will be removed are those between \a start and \a end inclusive, under the given \a parent item.
    Default implementation is empty
*/
void HbAbstractItemView::rowsAboutToBeRemoved(const QModelIndex &index, int start, int end)
{
    Q_UNUSED(index);
    Q_UNUSED(start);
    Q_UNUSED(end);
}

/*!
    This slot is called just before rows are inserted into the model.
    The items that will be inserted are those between \a start and \a end inclusive, under the given \a parent item.
    Default implementation is empty.
*/
void HbAbstractItemView::rowsAboutToBeInserted(const QModelIndex &index, int start, int end)
{
    Q_UNUSED(index);
    Q_UNUSED(start);
    Q_UNUSED(end);
}

/*!
    This slot is called when columns are inserted. 
    The new rows are those under the given parent from start to end inclusive
*/
void HbAbstractItemView::columnsInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(end)
    Q_UNUSED(start)

    Q_D(HbAbstractItemView);

    if (parent == d->mModelIterator->rootIndex()) {
        // ??????? why columns
        if (d->mModelIterator->model()->columnCount(parent) == 1) {
            rowsInserted(parent, 0, d->mModelIterator->model()->rowCount(parent));
        }
    }
}

/*!
    This slot is called after columns have been removed from the model.
    The removed items are those between start and end inclusive, under the given parent item.
*/
void HbAbstractItemView::columnsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(end)
    Q_UNUSED(start)

    Q_D(HbAbstractItemView);

    if (parent == d->mModelIterator->rootIndex()) {
        // ??????? why columns
        if (d->mModelIterator->model()->columnCount(parent) == 0){
            rowsRemoved(parent, 0, d->mModelIterator->model()->rowCount(parent));
        }
    }

}

/*!
    This slot is called just before columns are removed from the model.
    The items that will be removed are those between \a start and \a end inclusive, under the given \a parent item.
    Default implementation is empty
*/
void HbAbstractItemView::columnsAboutToBeRemoved(const QModelIndex &index, int start, int end)
{
    Q_UNUSED(index);
    Q_UNUSED(start);
    Q_UNUSED(end);
}

/*!
    This slot is called just before columns are inserted into the model.
    The items that will be inserted are those between \a start and \a end inclusive, under the given \a parent item.
    Default implementation is empty.
*/
void HbAbstractItemView::columnsAboutToBeInserted(const QModelIndex &index, int start, int end)
{
    Q_UNUSED(index);
    Q_UNUSED(start);
    Q_UNUSED(end);
}

/*!
    Emits the activated signal.
*/
void HbAbstractItemView::emitActivated(const QModelIndex &modelIndex)
{
    emit activated(modelIndex);
}

/*!
    Emits the pressed signal.
*/
void HbAbstractItemView::emitPressed(const QModelIndex &modelIndex)
{
    emit pressed(modelIndex);
}

/*!
    Emits the released signal.
*/
void HbAbstractItemView::emitReleased(const QModelIndex &modelIndex)
{
    emit released(modelIndex);
}

/*!
    \reimp
*/
bool HbAbstractItemView::scrollByAmount(const QPointF &delta)
{
    Q_D(HbAbstractItemView);

    QPointF newDelta(delta);

    if (d->mContainer->itemRecycling()
        && !d->mContainer->items().isEmpty()) {
        newDelta = d->mContainer->recycleItems(delta);
        d->mAnimationInitialPosition = d->mAnimationInitialPosition + newDelta - delta; 
        d->mContainer->resizeContainer();
    }

    return HbScrollArea::scrollByAmount(newDelta);
}

/*!
    \reimp
*/
QVariant HbAbstractItemView::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSceneHasChanged) {
        HbMainWindow *window = mainWindow();
        if (window) { // added to scene
            QObject::connect(window, SIGNAL(aboutToChangeOrientation()),
                this, SLOT(orientationAboutToBeChanged()));
            QObject::connect(window, SIGNAL(orientationChanged(Qt::Orientation)), 
                this, SLOT(orientationChanged(Qt::Orientation)));

            if (verticalScrollBar()) {
                verticalScrollBar()->installSceneEventFilter(this);
            }
            if (horizontalScrollBar()) {
                horizontalScrollBar()->installSceneEventFilter(this);
            }
        }
        else { // removed from scene
            QObject::disconnect(this, SLOT(orientationAboutToBeChanged()));
            QObject::disconnect(this, SLOT(orientationChanged(Qt::Orientation)));

            if (verticalScrollBar()) {
                verticalScrollBar()->removeSceneEventFilter(this);
            }
            if (horizontalScrollBar()) {
                horizontalScrollBar()->removeSceneEventFilter(this);
            }

        }

    }
    return HbScrollArea::itemChange(change, value);
}

/*!
    Returns the current name of layout definition of view items of this view.

    \sa setLayoutName()
*/
QString HbAbstractItemView::layoutName() const
{
    Q_D(const HbAbstractItemView);
    
    return d->mLayoutOptionName;
}

/*!
    Sets the name of layout definition \a layoutName for selecting 
    the layout of view items of this view from css/xml files.

    This layoutName is accessible from css file as layoutName property
    of the view item.

    This can be used for customization purposes. By default the layout name
    is "default".

    \sa layoutOption()
*/
void HbAbstractItemView::setLayoutName(const QString &layoutName)
{
    Q_D(HbAbstractItemView);
    
    d->mLayoutOptionName = layoutName;
    
    QList<HbAbstractViewItem *> items = d->mContainer->items();
    foreach (HbAbstractViewItem *item, items) {
        QEvent* polishEvent = new QEvent( QEvent::Polish );
        QCoreApplication::postEvent(item, polishEvent);
    }
} 

/*!
    Sets the property informing whether all items in the item view have the same size.
    
    This property should only be set to true if it is guaranteed that all items in the view have 
    the same size. This enables the view to do some optimizations for performance purposes.

    By default, this property is false.

    \sa uniformItemSizes
*/
void HbAbstractItemView::setUniformItemSizes(bool enable)
{
    Q_D(HbAbstractItemView);
    d->mContainer->setUniformItemSizes(enable);
}

/*!
    Returns the current value of the uniformItemsSizes property.

    By default, this property is false.
*/
bool HbAbstractItemView::uniformItemSizes() const
{
    Q_D(const HbAbstractItemView);
    return d->mContainer->uniformItemSizes();
}

/*!
    Returns pointer to HbModelIterator. It provides functions to work with QModelIndex.
*/
HbModelIterator *HbAbstractItemView::modelIterator() const
{
    Q_D(const HbAbstractItemView);
    return d->mModelIterator;
}

/*!
    Sets the bitmask controlling the item animations.
*/
void HbAbstractItemView::setEnabledAnimations(HbAbstractItemView::ItemAnimations flags)
{
    Q_D(HbAbstractItemView);
    d->mEnabledAnimations = flags;
}

/*!
    Returns the mask controlling the item animations.
*/
HbAbstractItemView::ItemAnimations HbAbstractItemView::enabledAnimations() const
{
    Q_D(const HbAbstractItemView);
    return d->mEnabledAnimations;
}

/*!
    \reimp
*/
void HbAbstractItemView::gestureEvent(QGestureEvent *event)
{
    if (event->gesture(Qt::PanGesture)) {
        Q_D(HbAbstractItemView);
        if (d->panTriggered(event)) {
            event->accept();
        } else {
            HbScrollArea::gestureEvent(event);
        }
    } else {
        HbScrollArea::gestureEvent(event);
    }
}

/*! 
    This slot is called when touch down event occurs. Default implementation handles selection 
    and calls emitPressed().

    \sa HbAbstractViewItem::pressed(const QPointF &position)
    \sa emitPressed()
*/
void HbAbstractItemView::itemPressed(const QPointF &pos)
{
    Q_D(HbAbstractItemView);

    d->mPostponedScrollIndex = QPersistentModelIndex();
    d->mPreviousSelectedIndex = QModelIndex();
    d->mPreviousSelectedCommand = QItemSelectionModel::NoUpdate;
    d->mSelectionSettings &= ~HbAbstractItemViewPrivate::Selection;
    d->mContSelectionAction = QItemSelectionModel::NoUpdate;

    HbAbstractViewItem *item = qobject_cast<HbAbstractViewItem *>(sender()); 
    QModelIndex index = item->modelIndex();

    if (d->mSelectionMode != HbAbstractItemView::NoSelection) {
        QGraphicsSceneMouseEvent mousePressEvent(QEvent::GraphicsSceneMousePress);
        mousePressEvent.setPos(pos);
        d->mSelectionModel->select(index, selectionCommand(item, &mousePressEvent));
    }

    emitPressed(item->modelIndex());
}

/*! 
    This slot is called when release event occurs for view item.
    Default implementation calls emitReleased().

    \sa HbAbstractViewItem::released(const QPointF &position)
    \sa emitReleased()
*/
void HbAbstractItemView::itemReleased(const QPointF &pos)
{
    Q_UNUSED(pos);

    HbAbstractViewItem *item = qobject_cast<HbAbstractViewItem *>(sender()); 
    emitReleased(item->modelIndex());
}

/*! 
    This slot is called when view item is activated.
    Default implementation handles selection and calls emitActivated().

    \sa HbAbstractViewItem::itemActivated(const QPointF &position)
    \sa emitActivated()
*/
void HbAbstractItemView::itemActivated(const QPointF &pos)
{
    Q_D(HbAbstractItemView);

    HbAbstractViewItem *item = qobject_cast<HbAbstractViewItem *>(sender()); 
    QModelIndex index = item->modelIndex();

    d->mSelectionModel->setCurrentIndex(index, QItemSelectionModel::NoUpdate);

    if (d->mSelectionMode != HbAbstractItemView::NoSelection) {
        QGraphicsSceneMouseEvent mouseReleaseEvent(QEvent::GraphicsSceneMouseRelease);
        mouseReleaseEvent.setPos(pos);
        d->mSelectionModel->select(index, selectionCommand(item, &mouseReleaseEvent));
    }

    emitActivated(index);
}

/*! 
    This slot is called when view item is long pressed and long press is enabled in itemview.
    Default implementation calls longPressed().

    \sa HbAbstractViewItem::longPressed(const QPointF &position)
    \sa longPressEnabled()
    \sa longPressed()
*/
void HbAbstractItemView::itemLongPressed(const QPointF &pos)
{
    Q_D(HbAbstractItemView);

    HbAbstractViewItem *item = qobject_cast<HbAbstractViewItem *>(sender()); 
    QModelIndex index = item->modelIndex();

    d->mSelectionModel->setCurrentIndex(index, QItemSelectionModel::NoUpdate);

    emit longPressed(item, item->mapToScene(pos));
}

/*! 
    This slot is called when concrete view item has been created. Default implementation connects
    touch event related signals of HbAbstractViewItem to respective slots in this class.

    \sa HbAbstractViewItem::pressed(const QPointF &position)
    \sa HbAbstractViewItem::released(const QPointF &position)
    \sa HbAbstractViewItem::itemActivated(const QPointF &position)
    \sa HbAbstractViewItem::longPressed(const QPointF &position)
    \sa itemPressed()
    \sa itemReleased()
    \sa itemActivated()
    \sa itemLongPressed()
*/
void HbAbstractItemView::itemCreated(HbAbstractViewItem *item)
{
    QObject::connect(item, SIGNAL(pressed(QPointF)), this, SLOT(itemPressed(QPointF)));
    QObject::connect(item, SIGNAL(released(QPointF)), this, SLOT(itemReleased(QPointF)));
    QObject::connect(item, SIGNAL(activated(QPointF)), this, SLOT(itemActivated(QPointF)));
    QObject::connect(item, SIGNAL(longPressed(QPointF)), this, SLOT(itemLongPressed(QPointF))); 
}

/*!
    \reimp
*/
bool HbAbstractItemView::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    Q_D(HbAbstractItemView);
    if (    d->mPostponedScrollIndex.isValid()
        &&  event->type() == QEvent::GraphicsSceneMousePress
        &&  (   watched == verticalScrollBar()
            ||  watched == horizontalScrollBar())) {
        d->mPostponedScrollIndex = QPersistentModelIndex();
    }
    return false;
}

/*!
    Sets the value of the longPressEnabled property.  This value is set
    to true if the widget is to respond to long press gestures, false otherwise.

    The default value is true.

    \sa HbAbstractItemView::longPressEnabled()
*/
void HbAbstractItemView::setLongPressEnabled(bool enabled)
{
    Q_D(HbAbstractItemView);
    d->mLongPressEnabled = enabled;
}

/*!
    Returns true if the item view handles long press gestures, false otherwise.

    \sa HbAbstractItemView::setLongPressEnabled()
*/
bool HbAbstractItemView::longPressEnabled() const
{
    Q_D(const HbAbstractItemView);
    return d->mLongPressEnabled;
}

/*!
    Slot handles QAbstractItemModel::layoutChanged() signal. 
    Default implementation sets first model item visible as first view item. 
*/
void HbAbstractItemView::modelLayoutChanged()
{
    Q_D(HbAbstractItemView);
    d->mContainer->d_func()->updateItemBuffer();
    d->mContainer->setModelIndexes(d->mModelIterator->nextIndex(QModelIndex()));
    if (d->mContainer->items().count() > 0) {
        scrollTo(d->mContainer->items().at(0)->modelIndex(), PositionAtTop);
    }
}

#include "moc_hbabstractitemview.cpp"


