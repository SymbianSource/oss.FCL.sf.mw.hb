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

#include "hbabstractitemview_p.h"
#include "hbabstractitemcontainer_p.h"

#include <hbabstractitemview.h>
#include <hbabstractviewitem.h>
#include <hbgesturefilter.h>
#include <hbevent.h>
#include <hbabstractitemcontainer.h>
#include <hbwidgetfeedback.h>
#include <hbinstance.h>
#include <hbscrollbar.h>
#include <hbmodeliterator.h>

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QDebug>

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
    Abstract view item that is representing \a index.

    See also released() and activated().
*/

/*!
  \fn void HbAbstractItemView::released(const QModelIndex &index)

    This signal is emitted when a touch release event is received within
    Abstract view item that is representing \a index.

    See also pressed() and activated().
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
    \var HbAbstractItemView::ContiguousSelection
    \deprecated HbAbstractItemView::ContiguousSelection
       is deprecated.

     When the user selects an item in the usual way, the selection is cleared and the new item selected.
     However, if the user presses an item, moves the selection and releases it, all the items between
     are selected or unselected, depending on the state of the first item.
*/

/*!
    \enum HbAbstractItemView::ItemAnimation

    animations in HbAbstractItemView that can be disabled. By default all animations are enabled.
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
    according to hint. Default value just guarantees, that item will be fully visible. 
*/


/*!
    See also HbAbstractItemManager, HbAbstractViewItem, HbAbstractItemContainer.

*/

/*!
    \deprecated HbAbstractItemView::HbAbstractItemView(HbAbstractItemViewPrivate &, HbAbstractItemContainer *, QGraphicsItem *)
        is deprecated. Use \a HbAbstractItemView::HbAbstractItemView(HbAbstractItemViewPrivate &, HbAbstractItemContainer *, HbModelIterator *, QGraphicsItem *).

    Constructs a new HbAbstractItemView with \a parent.
*/
HbAbstractItemView::HbAbstractItemView(HbAbstractItemViewPrivate &dd,
                                       HbAbstractItemContainer *container,
                                       QGraphicsItem *parent)
    : HbScrollArea(dd, parent)
{
    qWarning("HbAbstractItemView::HbAbstractItemView(HbAbstractItemViewPrivate &, HbAbstractItemContainer *, QGraphicsItem *) is deprecated! Use HbAbstractItemView::HbAbstractItemView(HbAbstractItemViewPrivate &, HbAbstractItemContainer *, HbModelIterator *, QGraphicsItem *).");
    Q_D(HbAbstractItemView);
    Q_ASSERT_X(container, "HbAbstractItemView constructor", "Container is null");

    d->q_ptr = this;
    d->init(container, new HbModelIterator());
}

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
    \deprecated HbAbstractItemView::setSelectionMode(HbAbstractItemView::SelectionMode, bool)
        is deprecated. Use \a HbAbstractItemView::setSelectionMode(HbAbstractItemView::SelectionMode newMode)

    If \a newMode is not same as current selection mode of view,
    updates selection mode and all viewitems.
    If \a resetSelection is true, it clears all existing selections. By default this value is true.
 */
void HbAbstractItemView::setSelectionMode(SelectionMode newMode, bool resetSelection)
{
    qWarning("HbAbstractItemView::setSelectionMode(HbAbstractItemView::SelectionMode, bool) is deprecated! Use HbAbstractItemView::setSelectionMode(HbAbstractItemView::SelectionMode).");

    Q_D(HbAbstractItemView);
    if (d->mSelectionMode != newMode) {
        d->mSelectionMode = newMode;

        if (resetSelection) {
            clearSelection();
        }

        d->updateItems();
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
    if (d->mModelIterator->model()
        && d->mSelectionModel 
        && (d->mSelectionMode == MultiSelection 
        ||  d->mSelectionMode == ContiguousSelection)) {
        QModelIndex firstIndex = d->mModelIterator->nextIndex(QModelIndex());
        QModelIndex lastIndex = d->mModelIterator->previousIndex(QModelIndex());
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
   Sets Currentindex to \a index. The item is selected depending on the \a selectionFlag.
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
    It should be allways called by child class if overriden.
*/
bool HbAbstractItemView::event(QEvent *e)
{
    Q_D(HbAbstractItemView);

    bool result = false;

    if (e->type() == HbEvent::ChildFocusOut) {
        d->mWasScrolling = isScrolling();
        result = true;
    }

    result |= HbScrollArea::event(e);

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
        d->refreshContainerGeometry();
        if (d->mPostponedScrollIndex.isValid()) { 
           d->scrollTo(d->mPostponedScrollIndex, d->mPostponedScrollHint);
        } 
        result = true;
    }

    return result;
}

/*!
    \reimp
*/
void HbAbstractItemView::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(HbAbstractItemView);
    d->mPostponedScrollIndex = QPersistentModelIndex();
    d->mPreviousSelectedIndex = QModelIndex();
    d->mPreviousSelectedCommand = QItemSelectionModel::NoUpdate;
    d->mInstantClickedModifiers = 0;

    d->mHitItem = d->itemAt(event->scenePos());

    if (d->mHitItem && d->mHitItem->modelIndex().isValid()) {
        QGraphicsItem::GraphicsItemFlags flags = d->mHitItem->flags();
        if (!flags.testFlag(QGraphicsItem::ItemIsFocusable)){
            d->mHitItem = 0;
        }
    }

    if (d->mHitItem) {
        if (d->mHitItem->modelIndex().isValid()) {
            Hb::InteractionModifiers modifiers = 0;
            if (d->mWasScrolling) {
                modifiers |= Hb::ModifierScrolling;
            }
            HbWidgetFeedback::triggered(d->mHitItem, Hb::InstantPressed, modifiers);

            if (!d->mWasScrolling) {
                d->mHitItem->setPressed(true);
            }
        }
    }

    if (isScrolling()) {
        // Needed when focus does not change. Otherwise mWasScrolling updating is done on 
        // focusOutEvent or event function.
        d->mWasScrolling = true;
    }

    HbScrollArea::mousePressEvent(event);
    
    if (d->mHitItem) {
        emitPressed(d->mHitItem->modelIndex());
        if (d->mSelectionModel) {
            QItemSelectionModel::SelectionFlags flags = selectionCommand(d->mHitItem, event);
            d->mSelectionModel->select(d->mHitItem->modelIndex(), flags);
        }
    } else if (d->mGestureFilter) {
        d->mGestureFilter->setLongpressAnimation(false);
    }

    event->accept();
}

/*!
    \reimp
*/
void HbAbstractItemView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(HbAbstractItemView);

    HbScrollArea::mouseReleaseEvent(event);

    if (d->mGestureFilter) {
        d->mGestureFilter->setLongpressAnimation(true);
    }

    HbAbstractViewItem* hitItem = d->itemAt(event->scenePos());
    if (hitItem) {
        QGraphicsItem::GraphicsItemFlags flags = hitItem->flags();
        if (!flags.testFlag(QGraphicsItem::ItemIsFocusable)){
            hitItem = 0;
        }
    }

    if (d->mHitItem) {
        d->mHitItem->setPressed(false);
    }

    if (hitItem) {
        if (hitItem->modelIndex().isValid()) {
            Hb::InteractionModifiers modifiers = 0;
            if (d->mWasScrolling) {
                modifiers |= Hb::ModifierScrolling;
            }
            HbWidgetFeedback::triggered(hitItem, Hb::InstantReleased, modifiers);
        }
        emitReleased(hitItem->modelIndex());
    }

    if (d->mWasScrolling || d->mOptions.testFlag(HbAbstractItemViewPrivate::PanningActive)) {
        d->mOptions &= ~HbAbstractItemViewPrivate::PanningActive;
        if (d->mSelectionSettings.testFlag(HbAbstractItemViewPrivate::Selection)) {
            d->mSelectionSettings &= ~HbAbstractItemViewPrivate::Selection;
            d->mContSelectionAction = QItemSelectionModel::NoUpdate;
        }
    } else if (hitItem) {
        if (d->mSelectionModel) {
            d->mSelectionModel->setCurrentIndex(hitItem->modelIndex(), QItemSelectionModel::NoUpdate);

            QItemSelectionModel::SelectionFlags flags = selectionCommand(hitItem, event);
            d->mSelectionModel->select(hitItem->modelIndex(), flags);
        }

        if (    d->mHitItem == hitItem
            &&  hitItem->modelIndex().isValid()){
            HbWidgetFeedback::triggered(hitItem, Hb::InstantClicked, d->mInstantClickedModifiers);
            emitActivated(hitItem->modelIndex());
        }

        HbAbstractViewItem *item = d->currentItem();
        if (item) {
            d->revealItem(item, EnsureVisible);
        }
    } 
    d->mWasScrolling = false;
    event->accept(); 
}

/*!
    \reimp
*/
void HbAbstractItemView::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    HbScrollArea::mouseMoveEvent( event );

    Q_D(HbAbstractItemView);

    // contiguous selection handling.
    if (d->mSelectionSettings.testFlag(HbAbstractItemViewPrivate::Selection)
        && d->mSelectionModel
        && d->mSelectionMode == HbAbstractItemView::ContiguousSelection
        && geometry().contains(event->pos())) {

        QModelIndex firstIndex;
        QModelIndex lastIndex;
        d->mContainer->firstAndLastVisibleModelIndex(firstIndex, lastIndex);
        qreal scenePosY = event->scenePos().y();
        qreal lastScenePosY = event->lastScenePos().y();
        QPolygonF polygon;
        polygon << event->lastScenePos() << event->scenePos();

        QList<QGraphicsItem *> items = scene()->items(polygon);
        int itemCount = items.count();

        // loop through the items in the scene
        for (int current = 0; current < itemCount ; ++current) {
            HbAbstractViewItem *item = d->viewItem(items.at(current));
            if (item) {
                if (d->mHitItem && item != d->mHitItem) {
                    d->mHitItem->setPressed(false);
                }

                QModelIndex itemIndex(item->modelIndex());
                QItemSelectionModel::SelectionFlags command = selectionCommand(item, event);
                if (    itemIndex != d->mPreviousSelectedIndex
                    ||  command != d->mPreviousSelectedCommand) {
                    d->mPreviousSelectedIndex = itemIndex;
                    d->mPreviousSelectedCommand = command;
                    d->mSelectionModel->select(itemIndex, command);

                    // Scroll up/down when needed
                    HbAbstractViewItem *scrollItem = 0;
                    if (itemIndex == firstIndex
                        && lastScenePosY > scenePosY) {
                        scrollItem = d->mContainer->itemByIndex(d->mModelIterator->previousIndex(itemIndex));
                    } else if (itemIndex == lastIndex
                        && lastScenePosY < scenePosY) {
                        scrollItem = d->mContainer->itemByIndex(d->mModelIterator->nextIndex(itemIndex));
                    }

                    if (scrollItem) {
                        QPointF delta = d->pixelsToScroll(scrollItem, EnsureVisible);
                        d->scrollByAmount(delta);
                    }
                    break;
                }
            }
        }
    }

    event->accept();
}

/*!
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
    d->refreshContainerGeometry();

    d->updateScrollMetrics();

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
    \deprecated HbAbstractItemView::isVisible(HbAbstractViewItem*) const
        is deprecated. Use isVisible \a HbAbstractItemView::isVisible(const QModelIndex&) const.

    This is an overloaded member function, provided for convenience.
    Returns true if item is fully or partially visible in view.
 */
bool HbAbstractItemView::isVisible(HbAbstractViewItem *item) const
{
    qWarning("HbAbstractItemView::isVisible(HbAbstractViewItem*) const is deprecated! Use isVisible \a HbAbstractItemView::isVisible(const QModelIndex&) const");

    Q_D(const HbAbstractItemView);
    return d->visible(item, false);
}

/*!
    Base class implemmentation. Take care about scrollTo when it was
    called before view was visible (first scrollTo can be done after
    first layoutRequest).
    It should be always called by child class if overriden.

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

    const int count(d->mContainer->items().count());
    for (int i = 0; i < count; ++i) {
        if(d->visible(d->mContainer->items().at(i), false))
            visibleItems.append(d->mContainer->items().at(i));
    }
    return visibleItems;
}

/*!
    \deprecated HbAbstractItemView::itemAtPosition(const QPointF&) const
        is deprecated.

    Returns a pointer to item at the coordinates.
 */
HbAbstractViewItem *HbAbstractItemView::itemAtPosition(const QPointF& position) const
{
    qWarning("HbAbstractItemView::itemAtPosition(const QPointF&) const is deprecated!");

    Q_D(const HbAbstractItemView);
    return d->itemAt(position);
}

/*!
    Returns SelectionFlags to be used when updating selection of a item.
    The event is a user input event, such as a mouse or keyboard event.
    contiguousArea is true, if selectiontype is not single or no selection and
    user has pressed on contiguousArea of viewItemi.e CheckBox.
    By default this is false.
    Subclasses should overide this function to define their own selection behavior.

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

        HbAbstractViewItem* oldItem = d->mContainer->itemByIndex(previous);
        HbAbstractViewItem* newItem = d->mContainer->itemByIndex(current);
       
        if (oldItem) {
            oldItem->lostFocus();
        } 
        
        if (previous.isValid()) {
            d->mContainer->setItemStateValue(previous, HbAbstractViewItem::FocusKey, false);
        }

        if (newItem) {
            newItem->receivedFocus();
        } 
        
        if (d->mCurrentIndex.isValid()) {
            d->mContainer->setItemStateValue(d->mCurrentIndex, HbAbstractViewItem::FocusKey, true);
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
            if (!d->mClearingSelection) {
                HbWidgetFeedback::triggered(item, Hb::InstantSelectionChanged);
            }
        } 
        d->mContainer->setItemStateValue(selectedIndexes.at(i),
                                         HbAbstractViewItem::CheckStateKey,
                                         Qt::Checked);
    }

    QModelIndexList deselectedIndexes(deselected.indexes());
    count = deselectedIndexes.count();
    for (int i = 0; i < count; ++i) {
        HbAbstractViewItem *item = d->mContainer->itemByIndex(deselectedIndexes.at(i));
        if (item) {
            item->setCheckState(Qt::Unchecked);
            if (!d->mClearingSelection) {
                HbWidgetFeedback::triggered(item, Hb::InstantSelectionChanged);
            }
        } 
        d->mContainer->setItemStateValue(deselectedIndexes.at(i),
                                         HbAbstractViewItem::CheckStateKey,
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
        bool animate = d->mEnabledAnimations & HbAbstractItemView::Appear ? d->mAnimateItems : false;
        d->mContainer->addItem(index, animate);
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
    \reimp
*/
void HbAbstractItemView::panGesture(const QPointF &point)
{
    Q_D( HbAbstractItemView );
    d->mPostponedScrollIndex = QPersistentModelIndex();
    d->mOptions |= d->PanningActive;
    
    if (d->mHitItem) {
        d->mHitItem->setPressed(false, false);
    }

    HbScrollArea::panGesture(point);
    
}

/*!
    \reimp
*/
void HbAbstractItemView::longPressGesture(const QPointF &point)
{
    Q_D( HbAbstractItemView );
    d->mPostponedScrollIndex = QPersistentModelIndex();
    HbScrollArea::longPressGesture(point);

    if (d->mHitItem) {
        if (d->mHitItem->modelIndex().isValid()) {
            HbWidgetFeedback::triggered(d->mHitItem, Hb::InstantLongPressed);
        }
        if (d->mHitItem) {
            d->mHitItem->setPressed(false);
        }
        emit longPressed(d->mHitItem, point);
    }
}

/*!
    \reimp
*/
void HbAbstractItemView::focusOutEvent(QFocusEvent *event)
{
    HbScrollArea::focusOutEvent(event);

    Q_D( HbAbstractItemView );

    d->mWasScrolling = isScrolling();
    d->stopScrolling();
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
    \deprecated HbAbstractItemView::indexCount() const
        is deprecated. Use \a HbModelIterator::indexCount()

    Returns the total model index count that can be traversed with nextIndex and previousIndex functions.
*/
int HbAbstractItemView::indexCount() const
{
    qWarning("HbAbstractItemView::indexCount() is deprecated! Use HbModelIterator::indexCount().");

    Q_D(const HbAbstractItemView);
    return d->mModelIterator->indexCount();
}

/*!
    \deprecated HbAbstractItemView::indexPosition(const QModelIndex &) const
        is deprecated. Use \a HbModelIterator::indexPosition(const QModelIndex &) const

    Returns the position of \a index from the first index.
*/
int HbAbstractItemView::indexPosition(const QModelIndex &index) const
{
    qWarning("HbAbstractItemView::indexPosition(const QModelIndex &) is deprecated! Use HbModelIterator::indexPosition(const QModelIndex &).");

    Q_D(const HbAbstractItemView);
    return d->mModelIterator->indexPosition(index);
}

/*!
    \deprecated HbAbstractItemView::nextIndex(const QModelIndex&) const
        is deprecated. Use \a HbModelIterator::nextIndex(const QModelIndex&) const

    Returns the next model index from \a index. If QModelIndex() is given as a parameter
    this should return the first model index.

    \note Returned QModelIndex() is interpret so that there is no next index from given one.
*/
QModelIndex HbAbstractItemView::nextIndex(const QModelIndex &index) const
{
    qWarning("HbAbstractItemView::nextIndex(const QModelIndex&) is deprecated! Use HbModelIterator::nextIndex(const QModelIndex&).");

    Q_D(const HbAbstractItemView);
    return d->mModelIterator->nextIndex(index);
}

/*!
    \deprecated HbAbstractItemView::previousIndex(const QModelIndex&) const
        is deprecated. Use \a HbModelIterator::previousIndex(const QModelIndex&) const

    Returns the previous model index from \a index. If QModelIndex() is given as a parameter
    this should return the last model index.

    \note Returned QModelIndex() is interpret so that there is no previous index from given one.
*/
QModelIndex HbAbstractItemView::previousIndex(const QModelIndex &index) const
{
    qWarning("HbAbstractItemView::previousIndex(const QModelIndex&) is deprecated! Use HbModelIterator::previousIndex(const QModelIndex&).");

    Q_D(const HbAbstractItemView);
    return d->mModelIterator->previousIndex(index);
}

/*!
    \reimp
*/
bool HbAbstractItemView::scrollByAmount(const QPointF &delta)
{
    Q_D(HbAbstractItemView);

    QPointF newDelta(delta);

    if (d->mContainer->itemRecycling()) {
        newDelta = d->mContainer->recycleItems(delta);
        d->mAnimationInitialPosition = d->mAnimationInitialPosition + newDelta - delta; 
        d->refreshContainerGeometry();
    }

    return HbScrollArea::scrollByAmount(newDelta);
}

/*!
    Returns the container widget.
*/
HbAbstractItemContainer *HbAbstractItemView::container() const
{
    Q_D(const HbAbstractItemView);
    return d->mContainer;
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
    \deprecated HbAbstractItemView::type() const
        is deprecated.

    \reimp
*/
int HbAbstractItemView::type() const
{
    qWarning("HbAbstractItemView::type() const is deprecated!");

    return Type;
}

/*!
    Returns the current name of layout definition of view items of this view

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
*/
void HbAbstractItemView::setUniformItemSizes(bool enable)
{
    Q_D(HbAbstractItemView);
    d->mContainer->setUniformItemSizes(enable);
}

/*!
    Returns the current value of the uniformItemsSizes property
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

    Sets the pressed item non-pressed.
*/
void HbAbstractItemView::upGesture(int value)
{
    Q_D(HbAbstractItemView);

    d->mPostponedScrollIndex = QPersistentModelIndex();

    if (d->mHitItem) {
        d->mHitItem->setPressed(false, false);
    }

    HbScrollArea::upGesture(value);
}

/*!
    \reimp

    Sets the pressed item non-pressed.
*/
void HbAbstractItemView::downGesture(int value)
{
    Q_D(HbAbstractItemView);

    d->mPostponedScrollIndex = QPersistentModelIndex();

    if (d->mHitItem) {
        d->mHitItem->setPressed(false, false);
    }

    HbScrollArea::downGesture(value);
}

/*!
    \reimp

    Sets the pressed item non-pressed.
*/
void HbAbstractItemView::leftGesture(int value)
{
    Q_D(HbAbstractItemView);

    d->mPostponedScrollIndex = QPersistentModelIndex();

    if (d->mHitItem) {
        d->mHitItem->setPressed(false, false);
    }

    HbScrollArea::leftGesture(value);
}

/*!
    \reimp

    Sets the pressed item non-pressed.
*/
void HbAbstractItemView::rightGesture(int value)
{
    Q_D(HbAbstractItemView);

    d->mPostponedScrollIndex = QPersistentModelIndex();

    if (d->mHitItem) {
        d->mHitItem->setPressed(false, false);
    }

    HbScrollArea::rightGesture(value);
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

#include "moc_hbabstractitemview.cpp"


