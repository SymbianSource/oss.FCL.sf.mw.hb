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
#include <hbabstractviewitem_p.h>
#include <hbevent.h>
#include <hbabstractitemcontainer_p.h>
#include <hbwidgetfeedback.h>
#include <hbinstance.h>
#include <hbscrollbar.h>
#include <hbmodeliterator.h>
#include <hbdataform.h>

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QDebug>

#include <QGesture>
#include <QGestureEvent>
#include <QPanGesture>

#include <QSortFilterProxyModel>

/*!
    @alpha
    @hbwidgets
    \class HbAbstractItemView
    \brief The HbAbstractItemView class defines a set of common properties for the item view classes that derive from it.
    
    The item view classes derived from %HbAbstractItemView are widgets used to display items of related data in a scrollable area. For example,
    derived views are provided to display a sequential list, a tree, a grid and a form. The items of data are typically icons and text. 
    
    This class and its derived classes use Qt's model/view framework, which requires that the data to be displayed is stored first in the model. The only exception to this is the HbListWidget class, which does not require an existing model. The model can be any QAbstractItemModel-based model. This framework provides a standard interface for communicating with models through the signals and slots mechanism, which ensures the views are  updated when the model is changed. The QAbstractItemModel class describes the concepts of the model such as the \e index of the model, model \e items and \e roles.
    
    The individual view items are objects derived from HbAbstractViewItem. %HbAbstractItemView stores a list of HbAbstractViewItems
    and manages these view items for you. By changing this list the view items can be customized. The
     %HbAbstractItemView class explains how this can be done.
    
    %HbAbstractItemView is an abstract class and cannot itself be instantiated. Therefore please refer to the derived classes (HbListView, HbGridView, HbTreeView, HbDataForm) for examples of use.
    
    %HbAbstractItemView provides properties that you can use to customize the following aspects of a view:  

    - <b>Item sizes</b>. You can set the items to all display at the same size using setUniformItemSizes(), which  improves performance for some operations such as scrolling. 
    
    - <b>Item recycling</b>. You can enable item recycling via setItemRecycling() to save memory. When item recycling is enabled, the view does not create a view item for every model item immediately after the view is launched. Instead, it creates view items for the visible area plus a few adjacent extra items. When the view is scrolled the existing view items are updated with new data from the model. By default item recycling is off.
    
    - <b>Pixmap cache</b>. You can enable the pixmap cache via setItemPixmapCacheEnabled() to significantly improve scrolling speed. It 
    is recommended that the cache is enabled except in cases where the cache would potentially cause problems. By default the pixmap cache is off.
    
    - <b>View scrolling</b>. You can scroll the view to ensure a particular item is shown by using scrollTo(). The function takes an optional hint parameter to define which part of the scroll area should contain the item, such as the middle or the top. Typically %scrollTo() is used after the creation of a view or to set a specific position for a newly added item.
    
    - <b>%Selection Mode</b>. You can change the selection mode to either single or multiple using setSelectionMode(). You can also modify the
    existing selection using clearSelection() and selectAll().
    
    - <b>%Selection Model</b>. You can change the selection model using setSelectionModel().

    - <b>Icon load policy</b>. You can change the icon load policy so that icons are loaded synchronously or asynchronously using setIconLoadPolicy(). By default icons are loaded asynchronously to improve performance, but this can result in empty areas being displayed while the icons are still being loaded.
    
    \section Subclassing
    %HbAbstractItemView can be subclassed for customization purposes.
    

    \sa  HbAbstractViewItem
*/

/*!
    \fn void HbAbstractItemView::pressed(const QModelIndex &index)

    This signal is emitted when a touch down event is received within
    the view item that is representing \a index.

    \sa released() activated()
*/

/*!
  \fn void HbAbstractItemView::released(const QModelIndex &index)

    This signal is emitted when a release event is received within
    the view item that is representing \a index.

    \sa pressed(), activated(), HbAbstractViewItem::released(const QPointF &position)
*/

/*!
    \fn void HbAbstractItemView::activated(const QModelIndex &index)

    This signal is emitted when the item specified by \a index is activated by the user.
    An item is activated for example by clicking the item with a mouse
    or by tapping the item.

    \sa pressed(), released()
*/

/*!
    \fn void HbAbstractItemView::longPressed(HbAbstractViewItem *item, const QPointF &coords)

    This signal is emitted when a long press event is received by a view item.
    
    \param item The view item that received the long press event.
    \param coords The scene position where the long press event happened.
*/

/*!
    \enum HbAbstractItemView::SelectionMode

    Defines the available selection types.
*/

/*!
    \var HbAbstractItemView::NoSelection

    Items cannot be selected. This is the default value.
*/


/*!
    \var HbAbstractItemView::SingleSelection

    When the user selects an item, all other items become deselected. The user cannot
    deselect the selected item.
*/

/*!
    \var HbAbstractItemView::MultiSelection

    When the user selects an item, the selection state of that item
    is toggled and the other items are left unchanged. 
*/

/*!
    \enum HbAbstractItemView::ScrollHint

    Defines the positions on the view that an item can be scrolled to.

    \sa scrollTo()
*/

/*!
    \var HbAbstractItemView::EnsureVisible

    The item will be fully visible in the view. 
*/


/*!
    \var HbAbstractItemView::PositionAtTop

    The item will be fully visible as the topmost item.
*/


/*!
    \var HbAbstractItemView::PositionAtBottom

    The item will be fully visible as the bottommost item.
*/

/*!
    \var HbAbstractItemView::PositionAtCenter

    The item will be centered in the view.
*/

/*!
    \enum HbAbstractItemView::ItemAnimation

    Defines the types of animations in %HbAbstractItemView. The values are designed to be used as flags.
    By default all animations are enabled.
    
    The ItemAnimations type is a typedef for QFlags<ItemAnimation>. It stores an OR combination of the ItemAnimation values.

*/

/*!
    \var HbAbstractItemView::None

    No animations are applied. 
*/

/*!
    \var HbAbstractItemView::Appear

    The animation displayed during item appearance. This animation can be disabled if desired. Disable this animation in cases where you expect many model items to appear,
    such as during the insertion of a new data source. Note that the item appearance animations are not used directly after a call to setModel().
*/

/*!
    \var HbAbstractItemView::Disappear

    The animation displayed during item removal. This animation can be disabled if desired. Disable this animation in cases where you expect many model items to disappear, such as during the filtering or removal of a data source.
*/

/*!
    \var HbAbstractItemView::TouchDown

    The animations displayed during item press and release events. This animation can be disabled if desired.
*/

/*!
    \var HbAbstractItemView::Expand

    The animation displayed when a setting item is expanded.
*/

/*!
    \var HbAbstractItemView::Collapse

    The animation displayed when a setting item is collapsed.
*/

/*!
    \var HbAbstractItemView::All

    A combination of all the flags. Every animation is applied. 
*/



/*!
    \enum HbAbstractItemView::IconLoadPolicy

    Defines the icon load policies that are used to control how icon data is loaded into items.
*/

/*!
    \var HbAbstractItemView::LoadSynchronously

    The icon data for items is always loaded synchronously. In this mode the items will block all other processing while they load their icons.  This can lead to longer item creation time and data update times which may result in poor performance especially while scrolling.
    
    The benefit is that the icon content is guaranteed to be painted immediately so this avoids the possibility of empty areas that might appear if the data is loaded asynchronously.
*/

/*!
    \var HbAbstractItemView::LoadAsynchronouslyWhenScrolling

    The icon data is loaded asynchronously during scrolling but otherwise the data is loaded synchronously. 
*/

/*!
    \var HbAbstractItemView::LoadAsynchronouslyAlways

    The icon data is always loaded asynchronously. In this mode the process of loading the icon data will not block other processing 
    and so will not cause delays in item creation or data update times. This results in better scrolling 
    performance since the item recycling delay is shared between different frames. 

    The disadvantage for this mode is that icon content is not guaranteed to be painted immediately and this may cause empty areas 
    to be painted while asynchronous data is being loaded.

    This is the default mode.
*/


/*!

    Constructs a new %HbAbstractItemView with the given (private data) \a dd, \a container, \a model \a iterator and \a parent.
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
    Constructs a new %HbAbstractItemView with \a container, \a modelIterator and \a parent.
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
    Destructor.
*/
HbAbstractItemView::~HbAbstractItemView()
{

}

/*!
    Returns the model that the view is currently presenting.
    
    \sa setModel()
*/
QAbstractItemModel *HbAbstractItemView::model() const
{
    Q_D(const HbAbstractItemView);
    return d->mModelIterator->model();
}

/*!
    Sets the model to \a model and replaces the current item prototype with \a prototype.
    Ownership of the model is not taken. Ownership of the prototype is taken. 
    If no prototype has been passed, the default prototype for the item view is used.
    
    \note This function may cause existing view items to be recreated. The recreated view items are created asynchronously.
    
    \sa model(), setItemPrototype

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
    Returns the list of item prototypes. How to use item prototypes is described in HbAbstractViewItem.
    
    \sa setItemPrototype(), setItemPrototypes()
*/
QList<HbAbstractViewItem *> HbAbstractItemView::itemPrototypes() const
{
    Q_D(const HbAbstractItemView);
    return d->mContainer->itemPrototypes();
}

/*!
    Replaces the current item prototypes with \a prototype and takes ownership of the prototype.
    
    \note This function may cause existing view items to be recreated. The recreated view items are created asynchronously.
    
    \note Supplied item views such as HbListView have default prototypes which are set when the item view class is created.
    
    \sa itemPrototypes(), setItemPrototypes()

*/
void HbAbstractItemView::setItemPrototype(HbAbstractViewItem *prototype)
{
    Q_D(HbAbstractItemView);
    if (prototype && d->mContainer->setItemPrototype(prototype))  {
        d->resetContainer();
    }
}

/*! 
    Replaces the current item prototypes with the list of \a prototypes and takes ownership of the \a prototypes.
    
    This allows multiple abstract view items within %HbAbstractItemView.
    
    When list view items are being created, the choice of which prototype to use for each item is 
    based on the order of the prototypes and each prototype's response to the view calling HbAbstractViewItem::canSetModelIndex().
    The prototype list is read from the end to the beginning. 
    
    The recommended list order is to place specialized prototypes at the end of the list and a
    default prototype at the list head. The specialized prototypes would create
    only certain types of list view items and so would implement HbAbstractViewItem::canSetModelIndex().
     The default prototype would always return \c true for HbAbstractViewItem::canSetModelIndex() and would be capable of creating
     any type of list view item. The specialized prototypes would be expected to create more complex items from extra data in the
     model that the default prototype would otherwise ignore.

    Supplied item views such as HbListView have default prototypes which are set when the item view class is created.
    
    \note Setting more than one prototype will disable the item recycling feature.

    \note This function may cause existing view items to be recreated. The recreated view items are created asynchronously.

    \sa setItemPrototype(), itemPrototypes()
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
    
    \sa setSelectionModel
*/
QItemSelectionModel *HbAbstractItemView::selectionModel() const
{
    Q_D(const HbAbstractItemView);
    return d->mSelectionModel;
}

/*!
    Sets the current selection model to \a selectionModel.
    
    \note If setModel() is called after this function, the selection model will be
    replaced by the default selection model of the view.
    
    \sa selectionModel
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
    Sets the current selection mode to \a newMode.
    
    If \a newMode is different from the current selection mode for the view,
    this function updates all view items and clears all existing selections.
    
    \sa selectionMode()
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
    Deselects all selected items. The current index is not changed.
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
    Returns the current selection mode used by the view.
    
    \sa setSelectionMode
*/
HbAbstractItemView::SelectionMode HbAbstractItemView::selectionMode() const
{
    Q_D(const HbAbstractItemView);
    return d->mSelectionMode;
}

/*!
    Returns the index of the current item.
*/
QModelIndex HbAbstractItemView::currentIndex() const
{
    Q_D(const HbAbstractItemView);
    return d->mCurrentIndex;
}

/*!
   Sets the current index to \a index. The item is selected depending on the \a selectionFlag.
   By default the item is not selected. If the current selection mode (HbAbstractItemView::SelectionMode) is NoSelection,
   the item is not selected irrespective of the selection flag.
   
   \sa setSelectionMode()
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
    Returns the model index of the model's root item. 
    The root item is the parent item to the view's top level items.
    The root can be invalid.
    
    \sa setRootIndex
*/
QModelIndex HbAbstractItemView::rootIndex() const
{
    Q_D(const HbAbstractItemView);
    return d->mModelIterator->rootIndex();
}

/*!
    Sets the root index to \a index.
    All view items are deleted and recreated.

    \note View items may be created asynchronously.
    
    \sa rootIndex
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
    Resets the item view. This resets all animations, scrolls the view to the original position, but preserves any existing selections.
*/
void HbAbstractItemView::reset()
{
    Q_D(HbAbstractItemView);    

    d->stopAnimating();
    d->mModelIterator->setRootIndex(QPersistentModelIndex());
    d->resetContainer();

    setCurrentIndex(QModelIndex(), QItemSelectionModel::NoUpdate);
}

/*!
    
    If overridden, the child class should always call this function to ensure the events related to the abstract item view
    and scroll area are handled.
*/
bool HbAbstractItemView::event(QEvent *e)
{
    Q_D(HbAbstractItemView);

    bool result = HbScrollArea::event(e);

    // The two above statements have to be executed before these
    if (e->type() == QEvent::LayoutRequest) {
        d->mContainer->resizeContainer();
        if (d->mPostponedScrollIndex.isValid()) { 
           d->scrollTo(d->mPostponedScrollIndex, d->mPostponedScrollHint);
           if (scrollDirections() | Qt::Vertical) {
               d->updateScrollBar(Qt::Vertical);
           }
           if (scrollDirections() | Qt::Horizontal) {
               d->updateScrollBar(Qt::Horizontal);
           }
        } 
        result = true;
    }

    return result;
}

/*!
    

    This slot is called when the orientation is changed.
    \param newOrientation The current orientation mode.
    
    \sa Qt:Orientation
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
    This slot is called just before the orientation is changed.
    
    \sa orientationChanged()
*/
void HbAbstractItemView::orientationAboutToBeChanged()
{
    Q_D(HbAbstractItemView);
    d->saveIndexMadeVisibleAfterMetricsChange();
}

/*!
    Sets item recycling to \a enabled.
    By default recycling is off.
    
    \sa itemRecycling
*/
void HbAbstractItemView::setItemRecycling(bool enabled)
{
    Q_D(HbAbstractItemView);
    d->mContainer->setItemRecycling(enabled);
}

/*!
    Returns whether the item recycling feature is in use.
    
    \sa setItemRecycling
*/
bool HbAbstractItemView::itemRecycling() const
{
    Q_D(const HbAbstractItemView); 
    return d->mContainer->itemRecycling();
}

/*!
    Returns the view item representing the current model index. This can be \c NULL if
    the index has no view item representing it.
*/
HbAbstractViewItem *HbAbstractItemView::currentViewItem() const
{
    Q_D(const HbAbstractItemView);
    return d->currentItem();
}


/*!
    Returns the view item representing the given  \a index in the model. This can be \c NULL if
    the index has no view item representing it.
*/
HbAbstractViewItem *HbAbstractItemView::itemByIndex(const QModelIndex &index) const
{
    Q_D(const HbAbstractItemView);
    return d->mContainer->itemByIndex(index);
}

/*!
   Returns \c true if the item in the model at \a index is fully or partially visible in the view.
*/
bool HbAbstractItemView::isVisible(const QModelIndex & index) const
{
    Q_D( const HbAbstractItemView );
    return d->visible(d->mContainer->itemByIndex(index), false);
}

/*!
    Scrolls the view to ensure that the item at \a index is visible.
    
    The optional \a hint parameter defines which part of the scroll area should contain the item. 
    The view is only scrolled if necessary, and the scrolling is not animated. Any child class implementations
    must be called if they override this function.
    
    The base class implementation is able to scroll an indexed item to make it visible if a view item has already been populated with the indexed item from the model. If recycling is enabled, derived classes must perform any recycling required. 
    
    \note If item recycling is enabled, the view item may not have reached its final 
    position when this function returns. Item positions can still be changed while the items are in the process of being layed out,
     which takes place asynchronously. 

    \sa itemRecycling(), QEvent
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
    Returns the list of fully or partially visible view items.
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
    Called when the selection is being updated as a result of user input. This might be the start of a selection or a change to the selection. Returns the selection flags to be used when updating the selection of an item.
    Subclasses should override this function to define their own selection behavior.

    \param item The view item that emitted the event.
    \param event The event such as a mouse or keyboard event. Touch events are received as mouse events. The event parameter does not always have all its properties set. For mouse events the event parameter always has the event type and position set.
    
    \sa HbAbstractViewItem::selectionAreaContains(), HbAbstractViewItem
*/
QItemSelectionModel::SelectionFlags HbAbstractItemView::selectionCommand(const HbAbstractViewItem *item,
                                                                         const QEvent *event)
{ 
    Q_D(HbAbstractItemView);
    // When the parameter contiguousArea is removed, copy paste 'd ->selectionFlags()' into here.
    return d->selectionFlags(item, event);
}

/*!
    This slot is called when items are changed in the model. 
    The changed items are those from \a topLeft to \a bottomRight inclusive.
    If only one item is changed \a topLeft will equal \a bottomRight.
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
    This slot is called when the current index is changed in the selection model.
    Changing the current index does not affect which items are selected.

    \param current New current index.
    \param previous Previous current index.
*/
void HbAbstractItemView::currentIndexChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    Q_D(HbAbstractItemView);


    if (current != d->mCurrentIndex) {
        d->mCurrentIndex = current;
    }
}

/*!
    This slot is called whenever the selection changes. The change in the selection is represented as an item selection
    of \a selected items and an item selection of \a deselected items.
    Note that the current index changes independently from the selection. Also note that this signal will not be emitted when the item model is reset.
    
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
            if (d->mContSelectionAction) {
                HbWidgetFeedback::triggered(item, Hb::InstantSelectionChanged, Hb::ModifierScrolling);
            }
            else {
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

            if (d->mContSelectionAction) {
                HbWidgetFeedback::triggered(item, Hb::InstantSelectionChanged, Hb::ModifierScrolling);
            }
            else {
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
    The new rows are those between \a start and \a end inclusive, under the given \a parent item.
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
    The removed items are those between \a start and \a end inclusive, under the given \a parent item.
*/
void HbAbstractItemView::rowsRemoved(const QModelIndex &parent,int start,int end)
{
    Q_D(HbAbstractItemView);
    d->rowsRemoved(parent, start, end);
}

/*!
    This slot is called just before rows are removed from the model.
    The items that will be removed are those between \a start and \a end inclusive, under the given \a index item.
    The base implementation is empty.
*/
void HbAbstractItemView::rowsAboutToBeRemoved(const QModelIndex &index, int start, int end)
{
    Q_UNUSED(index);
    Q_UNUSED(start);
    Q_UNUSED(end);
}

/*!
    This slot is called just before rows are inserted into the model.
    The items that will be inserted are those between \a start and \a end inclusive, under the given \a index item.
    The base implementation is empty.
*/
void HbAbstractItemView::rowsAboutToBeInserted(const QModelIndex &index, int start, int end)
{
    Q_UNUSED(index);
    Q_UNUSED(start);
    Q_UNUSED(end);
}

/*!
    This slot is called when columns are inserted. 
    The new columns are those under the given \a parent from \a start to \a end inclusive.
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
    The removed items are those between \a start and \a end inclusive, under the given \a parent item.
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
    The items that will be removed are those between \a start and \a end inclusive, under the given \a index item.
    The base implementation is empty.
*/
void HbAbstractItemView::columnsAboutToBeRemoved(const QModelIndex &index, int start, int end)
{
    Q_UNUSED(index);
    Q_UNUSED(start);
    Q_UNUSED(end);
}

/*!
    This slot is called just before columns are inserted into the model.
    The items that will be inserted are those between \a start and \a end inclusive, under the given \a index item.
    The base implementation is empty.
*/
void HbAbstractItemView::columnsAboutToBeInserted(const QModelIndex &index, int start, int end)
{
    Q_UNUSED(index);
    Q_UNUSED(start);
    Q_UNUSED(end);
}

/*!
    Emits the activated() signal.

*/
void HbAbstractItemView::emitActivated(const QModelIndex &modelIndex)
{
    emit activated(modelIndex);
}

/*!
    Emits the pressed() signal.

*/
void HbAbstractItemView::emitPressed(const QModelIndex &modelIndex)
{
    emit pressed(modelIndex);
}

/*!
    Emits the released() signal.

*/
void HbAbstractItemView::emitReleased(const QModelIndex &modelIndex)
{
    emit released(modelIndex);
}

/*!
    The abstract item view implementation handles item recycling.
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
    The abstract item view responds to QGraphicsItem::ItemSceneHasChanged to check if this item view has been added
    or removed from the window. This is to ensure scene filters and signals are connected only when necessary.
    
    \sa QGraphicsItem
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
    Returns the current name of the layout definition for the view items of this view.

    \sa setLayoutName()
*/
QString HbAbstractItemView::layoutName() const
{
    Q_D(const HbAbstractItemView);
    
    return d->mLayoutOptionName;
}

/*!
    Sets the name of the layout definition for this view to \a layoutName. This specifies that 
    the layout of the view items comes from the css/xml files.

    This layoutName property is accessible from the css file as the \c layoutName property
    of the view item.

    This can be used for customization purposes. By default the layout name
    is set to "default".

    \sa layoutName()
*/
void HbAbstractItemView::setLayoutName(const QString &layoutName)
{
    Q_D(HbAbstractItemView);
    
    d->mLayoutOptionName = layoutName;
    
    QList<HbAbstractViewItem *> items = d->mContainer->items();
    foreach (HbAbstractViewItem *item, items) {
        HbAbstractViewItemPrivate::d_ptr(item)->repolishItem();
    }
} 

/*!
    Sets the items to all display at the same size if \a enabled is \c true.
    
    This property should only be set to \c true if it is guaranteed that all items in the view have 
    the same size. This enables the view to do some optimizations for performance purposes.

    By default, this property is \c false.

    \sa uniformItemSizes
*/
void HbAbstractItemView::setUniformItemSizes(bool enable)
{
    Q_D(HbAbstractItemView);
    d->mContainer->setUniformItemSizes(enable);
}

/*!
    Returns whether the uniform item sizes feature is in use.

    The default value is \c false.
    
    \sa setUniformItemSizes
*/
bool HbAbstractItemView::uniformItemSizes() const
{
    Q_D(const HbAbstractItemView);
    return d->mContainer->uniformItemSizes();
}

/*!
    Returns the model iterator used by this item view.
*/
HbModelIterator *HbAbstractItemView::modelIterator() const
{
    Q_D(const HbAbstractItemView);
    return d->mModelIterator;
}

/*!
    Sets the bit mask controlling the item animations.
    
    \sa enabledAnimations()
*/
void HbAbstractItemView::setEnabledAnimations(ItemAnimations flags)
{
    Q_D(HbAbstractItemView);
    d->mEnabledAnimations = flags;
}

/*!
    Returns the mask controlling the item animations.
    
    \sa ItemAnimation, setEnabledAnimations()
*/
HbAbstractItemView::ItemAnimations HbAbstractItemView::enabledAnimations() const
{
    Q_D(const HbAbstractItemView);
    return d->mEnabledAnimations;
}

/*!
   The abstract item view implementation accepts panning events.
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
    This slot is called when a touch down event occurs.
    
    The base implementation handles selection      and calls emitPressed().

    \sa HbAbstractViewItem::pressed(const QPointF &position),  emitPressed()
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
    if (item) {
        QModelIndex index = item->modelIndex();

        if (d->mSelectionMode != HbAbstractItemView::NoSelection) {
            QGraphicsSceneMouseEvent mousePressEvent(QEvent::GraphicsSceneMousePress);
            mousePressEvent.setPos(pos);
            d->mSelectionModel->select(index, selectionCommand(item, &mousePressEvent));
        }
        emitPressed(item->modelIndex());
    }
}

/*! 
    This slot is called by a view item when a release event occurs for that item.
    
    The base implementation calls emitReleased().

    \sa HbAbstractViewItem::released(const QPointF &position),   emitReleased(),  itemPressed()
*/
void HbAbstractItemView::itemReleased(const QPointF &pos)
{
    Q_UNUSED(pos);

    HbAbstractViewItem *item = qobject_cast<HbAbstractViewItem *>(sender()); 
    emitReleased(item->modelIndex());
}

/*! 
    This slot is called when a view item is activated.
    
    The base implementation handles selection and calls emitActivated().

    \sa HbAbstractViewItem::activated(const QPointF &position), emitActivated()
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

    setCurrentIndex(index);
}

/*! 
    This slot is called when the view item is long pressed and long press events are allowed for this item view.
    
    The base implementation calls longPressed().

    \sa HbAbstractViewItem::longPressed(const QPointF &position),  longPressEnabled(),  longPressed()
*/
void HbAbstractItemView::itemLongPressed(const QPointF &pos)
{
    Q_D(HbAbstractItemView);

    HbAbstractViewItem *item = qobject_cast<HbAbstractViewItem *>(sender()); 
    QModelIndex index = item->modelIndex();

    d->mSelectionModel->setCurrentIndex(index, QItemSelectionModel::NoUpdate);

    emit longPressed(item, item->mapToScene(pos));

    setCurrentIndex(index);
}

/*! 
    This slot is called when \a item has been created.
    
    The base implementation connects
    the touch event signals of HbAbstractViewItem to the respective slots in this class.

    \sa HbAbstractViewItem, itemPressed(), itemReleased(), itemActivated(), itemLongPressed()
*/
void HbAbstractItemView::itemCreated(HbAbstractViewItem *item)
{
    QObject::connect(item, SIGNAL(pressed(QPointF)), this, SLOT(itemPressed(QPointF)));
    QObject::connect(item, SIGNAL(released(QPointF)), this, SLOT(itemReleased(QPointF)));
    QObject::connect(item, SIGNAL(activated(QPointF)), this, SLOT(itemActivated(QPointF)));
    QObject::connect(item, SIGNAL(longPressed(QPointF)), this, SLOT(itemLongPressed(QPointF))); 


    Q_D(HbAbstractItemView);
    if (d->mContainer->items().count() == 0) {
        d->mEmptyView->setVisible(false);
    }
}

/*!
   \sa QGraphicsItem::sceneEventFilter()
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
    Sets the long press gestures feature to \a enabled.  Set this value 
    to \c true if the widget is to respond to long press gestures, or to \c false otherwise.

    The default value is \c true.

    \sa longPressEnabled()
*/
void HbAbstractItemView::setLongPressEnabled(bool enabled)
{
    Q_D(HbAbstractItemView);
    d->mLongPressEnabled = enabled;
}

/*!
    Returns whether the long press gestures feature is enabled.

    \sa setLongPressEnabled()
*/
bool HbAbstractItemView::longPressEnabled() const
{
    Q_D(const HbAbstractItemView);
    return d->mLongPressEnabled;
}

/*!
    Sets the pixmap cache feature to \a enabled. When this is set to \c true, it enables the item view's pixmap cache.
    
    Enabling the item view's pixmap cache will significantly improve the item view scrolling speed.
    It is recommended that the cache is enabled except 
    in cases where the cache would potentially cause problems. Examples of situations where the cache should not be used are when a custom view item prototype cannot update the pixmap properly, or an effect is applied to a widget within a view item.
    
    By default the item pixmap cache is disabled. 

    \note The item pixmap cache is not supported by HbDataForm.

    \sa itemPixmapCacheEnabled(), HbAbstractViewItem::updatePixmapCache()
*/
void HbAbstractItemView::setItemPixmapCacheEnabled(bool enabled)
{
    Q_D(HbAbstractItemView);
    
    if (qgraphicsitem_cast<HbDataForm*>(this) == 0) {
        d->mItemPixmapCacheEnabled = enabled;
    }
}

/*!
    Returns whether the pixmap cache is in use.

    \sa setItemPixmapCacheEnabled()
*/
bool HbAbstractItemView::itemPixmapCacheEnabled() const
{
    Q_D(const HbAbstractItemView);
    return d->mItemPixmapCacheEnabled;
}

/*!
    Sets the icon load policy to \a policy.

    The default value is HbAbstractItemView::LoadAsynchronouslyAlways.

    \sa iconLoadPolicy()
*/
void HbAbstractItemView::setIconLoadPolicy(IconLoadPolicy policy)
{
    Q_D(HbAbstractItemView);
    if (d->mIconLoadPolicy != policy) {
        d->mIconLoadPolicy = policy;
        
        foreach (HbAbstractViewItem *prototype, itemPrototypes()) {
            HbAbstractViewItemPrivate::d_ptr(prototype)->mSharedData->updateIconItemsAsyncMode();
        }
    }
}

/*!
    Returns the current icon load policy used by the view.

    \sa setIconLoadPolicy()
*/
HbAbstractItemView::IconLoadPolicy HbAbstractItemView::iconLoadPolicy() const
{
    Q_D(const HbAbstractItemView);
    return d->mIconLoadPolicy;
}


/*!
    Returns the current text displayed in the item view when there are no items.

    \sa HbAbstractItemView::setEmptyText()
*/
QString HbAbstractItemView::emptyText() const
{
    Q_D(const HbAbstractItemView);
    return d->mEmptyView->emptyViewText();
}

/*!
    Sets the text displayed in the item view when there are no items. The default value is empty.

    \sa HbAbstractItemView::emptyText()
*/
void HbAbstractItemView::setEmptyText(const QString &emptyText)
{
    Q_D(HbAbstractItemView);
    d->mEmptyView->setEmptyViewText(emptyText);
}

/*!
    This slot handles the QAbstractItemModel::layoutChanged() signal. 
    
    The base implementation checks if the buffer items need to be updated and if so after the update sets the first model item visible 
    as the first view item.
    
    
    QSortFilterProxyModel sends the layoutChanged 
    signal whenever there is a data change in the model but in that case the view should
    not be scrolled. 
*/
void HbAbstractItemView::modelLayoutChanged()
{
    Q_D(HbAbstractItemView);
    QModelIndex firstItemIndex;

    QSortFilterProxyModel *proxyModel = qobject_cast<QSortFilterProxyModel *>(d->mModelIterator->model());
    if (proxyModel && proxyModel->dynamicSortFilter()) {
        QList<HbAbstractViewItem *> items = d->mContainer->items();

        if (!items.isEmpty() && !size().isEmpty()) {
            // Try maintaining first visible item as first visible item.
            int itemCount = items.count();

            for (int currentIndex = 0; currentIndex < itemCount; ++currentIndex) {
                HbAbstractViewItem *item = items.at(currentIndex);
                if (d->visible(item, true)) {
                    // Resolve new first item index when first visible item is known.
                    firstItemIndex = d->mModelIterator->index(d->mModelIterator->indexPosition(item->modelIndex()) - currentIndex);
                    break;
                }
            }
        }
    }

    if (!firstItemIndex.isValid()) {
        // New first item index not fount (e.g. when items are not visible) -> take first index.
        firstItemIndex = d->mModelIterator->nextIndex(firstItemIndex);
    }

    d->mContainer->d_func()->updateItemBuffer();

    if (!d->mContainer->items().isEmpty()) {
        if (d->mContainer->items().first()->modelIndex() != firstItemIndex) {
            d->mContainer->setModelIndexes(firstItemIndex);
            scrollTo(firstItemIndex, PositionAtTop);
        } else {
            d->mContainer->setModelIndexes(firstItemIndex);
        }
    }
}

#include "moc_hbabstractitemview.cpp"


