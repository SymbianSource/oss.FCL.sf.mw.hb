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

#include "hbabstractviewitem_p.h"

#include <hbabstractviewitem.h>
#include <hbabstractitemview.h>
#include <hbnamespace.h>
#include <hbstyleoptionabstractviewitem.h>
#include <hbstyle.h>
#include <hbiconitem.h>
#include <hbframebackground.h>
#include <hbabstractitemview_p.h>
#include <hbwidgetfeedback.h>
#include <hbtapgesture.h>

#include <QPersistentModelIndex>
#include <QGraphicsLayout>
#include <QVariant>
#include <QCoreApplication>
#include <QEvent>
#include <QDebug>

#include <QGesture>
#include <QGestureEvent>

#ifdef QMAP_INT__ITEM_STATE_DEPRECATED
#define HB_ITEM_STATE_ASSERT Q_ASSERT_X(0, "", "QMap<int,QVariant> based view item state system is deprecated. Use QHash<QString, QVariant> based instead" )
#else
#define HB_ITEM_STATE_ASSERT
#endif

const QString KDefaultLayoutOption = "default";
const int HbAbstractViewItemShared::ViewItemDeferredDeleteEvent = QEvent::registerEventType();

/*!
    @alpha
    @hbwidgets
    \class HbAbstractViewItem
    \brief The HbAbstractViewItem class represents a single item in a AbstractItemView.  

    The HbAbstractViewItem class provides an item that is used by HbAbstractItemView class to
    visualize content within single model index. By default HbAbstractViewItem supports QString
    that is stored into Qt::DisplayRole role and QIcon or HbIcon that is stored into
    Qt::DecoratorRole role within the index. 

    This class is provided mainly for customization purposes but it also acts as a default
    item prototype inside HbAbstractItemView. See HbAbstractItemView how to set customized class as a item prototype.

    \b Subclassing

    When subclassing HbAbstractViewItem, childclass must provide implementations of createItem() and updateChildItems() functions.
    
    To support multiple Abstractview items within single AbstractItemview, you must also provide an implementation of canSetModelIndex().

    If derived abstract view item has transient state information that is not meaningful to store within model index (child item cursor 
    position selection areas etc.) view item can use abstract views internal state model to store this information. This feature can
    be taken into use by implementing transientState() and setTransientState() functions in derived class.
*/

/*!
    \deprecated HbAbstractViewItem::StateKey
        is deprecated. Please use string based state keys.

    \enum HbAbstractViewItem::StateKey

    HbAbstractViewItem's predefined set of state keys.

    This enum describes state keys for HbAbstractViewItem state values. State value can be accessed using this key.

    \sa HbAbstractViewItem::transientState()
*/

/*!
    \deprecated HbAbstractViewItem::FocusKey
        is deprecated. Please use string based state keys. This key is replaced by "focused".

    \var HbAbstractViewItem::FocusKey
         Predefined key for focus state value.
*/

/*!
    \deprecated HbAbstractViewItem::CheckStateKey
        is deprecated. Please use string based state keys. This key is replaced by "checkState".

    \var HbAbstractViewItem::CheckStateKey
        Predefined key for check state value. Default value is Qt::Unchecked.
*/

/*!
    \deprecated HbAbstractViewItem::UserKey
        is deprecated. Please use string based state keys.

    \var HbAbstractViewItem::UserKey
         First key that can be used by the derived class for it's own purposes.
*/

/*!
    \fn void HbAbstractViewItem::pressed(const QPointF &position)

    This signal is emitted when a touch down event is received for this view item.
    \a position is position of touch event in view item coordinates.

    \sa HbAbstractViewItem::released(const QPointF &position)
    \sa HbAbstractViewItem::activated(const QPointF &position)
*/

/*!
    \fn void HbAbstractViewItem::released(const QPointF &position)

    This signal is emitted when a touch release event is received for this view item.
    \a position is position of touch event in view item coordinates.

    \sa HbAbstractViewItem::pressed(const QPointF &position)
    \sa HbAbstractViewItem::activated(const QPointF &position)
*/

/*!
    \fn void HbAbstractViewItem::activated(const QPointF &position)

    This signal is emitted when view item is activated by the user.
    How to activate items depends on the input method; e.g., with mouse by clicking the item
    or with touch input by tapping the item.
    \a position is position of touch event in view item coordinates.

    \sa HbAbstractViewItem::pressed(const QPointF &position)
    \sa HbAbstractViewItem::released(const QPointF &position)
*/

/*!
    \fn void HbAbstractViewItem::longPressed(const QPointF &position)

    This signal is emitted when long press event is received for this view item and long press is enabled in itemview.
    \a position is position of touch event in view item coordinates.

    \sa HbAbstractItemView::longPressEnabled()
*/


/*!
    \fn HbAbstractViewItem::createItem 

    Creates a new item. 

    In most subclasses, createItem should be implemented like this:

    \snippet{ultimatecodesnippet/customlistviewitem.cpp,1}
*/

void HbAbstractViewItemPrivate::init()
{
    Q_Q(HbAbstractViewItem);

    q->setProperty("state", "normal");

    if (isPrototype()) {
        q->setFocusPolicy(Qt::ClickFocus);
    } else {
        q->grabGesture(Qt::TapGesture);
        QGraphicsItem::GraphicsItemFlags itemFlags = q->flags();
        itemFlags |= QGraphicsItem::ItemIsFocusable;
        q->setFlags(itemFlags);

        q->setFocusPolicy(mSharedData->mPrototype->focusPolicy());

        mSharedData->mCloneItems.append(q);
    }
}

int HbAbstractViewItemPrivate::modelItemType() const
{
    return mIndex.data(Hb::ItemTypeRole).toInt();
}

void HbAbstractViewItemPrivate::_q_animationFinished(const HbEffect::EffectStatus &status)
{
    Q_UNUSED(status);
    Q_Q(HbAbstractViewItem);

    if (mFocusItem) {
        QCoreApplication::postEvent(q, new QEvent((QEvent::Type)HbAbstractViewItemShared::ViewItemDeferredDeleteEvent));
    }
}

void HbAbstractViewItemPrivate::repolishCloneItems()
{
    int count(mSharedData->mCloneItems.count());
    for (int i = 0; i < count; ++i) {
        mSharedData->mCloneItems.at(i)->repolish();
    }
}

void HbAbstractViewItemPrivate::updateCloneItems(bool updateChildItems)
{
    int count(mSharedData->mCloneItems.count());
    for (int i = 0; i < count; ++i) {
        if (updateChildItems) {
            mSharedData->mCloneItems.at(i)->updateChildItems();
        } else {
            mSharedData->mCloneItems.at(i)->updatePrimitives();
        }
    }
}

void HbAbstractViewItemPrivate::setInsidePopup(bool insidePopup)
{
    Q_Q(HbAbstractViewItem);

    HbWidgetPrivate::setInsidePopup(insidePopup);
    if (q) {
        themingPending = true;
        q->updatePrimitives();
        q->repolish();
    }
}

void HbAbstractViewItemPrivate::tapTriggered(QGestureEvent *event)
{
    Q_Q(HbAbstractViewItem);

    HbTapGesture *gesture = static_cast<HbTapGesture *>(event->gesture(Qt::TapGesture));
    QPointF position = event->mapToGraphicsScene(gesture->hotSpot());
    position = q->mapFromScene(position);

    switch (gesture->state()) {
        case Qt::GestureStarted: {
            HbWidgetFeedback::triggered(q, Hb::InstantPressed, 0);
            q->setPressed(true);
            emit q->pressed(position);

            break;
        }
        case Qt::GestureUpdated: {
            if (gesture->tapStyleHint() == HbTapGesture::TapAndHold 
                && mSharedData->mItemView
                && mSharedData->mItemView->longPressEnabled()) {
                q->setPressed(false);
                emit q->longPressed(position);
                revealItem();
            }
            break;
        }
        case Qt::GestureFinished: {
            HbWidgetFeedback::triggered(q, Hb::InstantReleased, 0);

            if (gesture->tapStyleHint() == HbTapGesture::Tap 
			    || (mSharedData->mItemView
                && !mSharedData->mItemView->longPressEnabled())) {
                q->setPressed(false);

                HbWidgetFeedback::triggered(q, Hb::InstantClicked);
                emit q->activated(position);
                emit q->released(position);
                revealItem();
            } else {
                emit q->released(position);
            }

            break;
        }
        case Qt::GestureCanceled: {
            HbWidgetFeedback::triggered(q, Hb::InstantReleased, 0);

            // hides focus immediately
            q->setPressed(false, false);

            emit q->released(position);
            break;
        }
        default:
            break;
    }

    event->accept();
}

void HbAbstractViewItemPrivate::revealItem()
{
    Q_Q(HbAbstractViewItem);

    if (mSharedData->mItemView) {
        static_cast<HbAbstractItemViewPrivate *>(mSharedData->mItemView->d_func())->revealItem(q, HbAbstractItemView::EnsureVisible);
    }
}

/*!
    Constructs an abstract view item with the given parent.
*/
HbAbstractViewItem::HbAbstractViewItem(QGraphicsItem *parent) : 
    HbWidget( *new HbAbstractViewItemPrivate( this ), parent )
{
    Q_D( HbAbstractViewItem );
    d->q_ptr = this;

    d->init();
}

/*!
    Creates a separate graphics widget with same Abstract view item state as \a source.
*/
HbAbstractViewItem::HbAbstractViewItem( HbAbstractViewItemPrivate &dd, QGraphicsItem *parent):
                    HbWidget( dd, parent )
{
    Q_D( HbAbstractViewItem );
    d->q_ptr = this;

    d->init();
}

/*!
    Creates a separate graphics widget with same abstract view item state as \a source.
*/
HbAbstractViewItem::HbAbstractViewItem(const HbAbstractViewItem &source) :
    HbWidget(*new HbAbstractViewItemPrivate(*source.d_func()), 0)
{
    Q_D( HbAbstractViewItem );
    d->q_ptr = this;

    d->init();
}

/*!
    Assigns the \a source abstract view item to this abstract view item and returns a reference to this item.
*/
HbAbstractViewItem &HbAbstractViewItem::operator=(const HbAbstractViewItem &source)
{
    Q_D( HbAbstractViewItem );
    *d = *source.d_func();
    return *this;
}


/*!
    Destructor.
*/
HbAbstractViewItem::~HbAbstractViewItem()
{
    HB_SDD(HbAbstractViewItem);
    if (d && !d->isPrototype()) {
        sd->mCloneItems.removeOne(this);
    }
}

/*!
    Returns true if \a model index is supported by Abstract view item, otherwise returns false.
    This function is called for every item on the prototype list (, if several prototypes exist)
    until item is found, which can create item for \a index. The prototype list is gone 
    through from end to the beginning. 
    
    Thus specialized prototypes should be in the end of the list and 
    'default' prototype first one. The specialized prototypes usually can create only
    certain types of list view items. The default prototype usually return always true,
    meaning that it can create any type of list view item. 

    \sa HbAbstractItemView::setItemPrototype(HbAbstractViewItem *prototype), HbAbstractItemView::setItemPrototype(const QList<HbAbstractViewItem *> &prototypes)
*/
bool HbAbstractViewItem::canSetModelIndex(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return true;
}

/*!
    Returns model index tied into this Abstract view item.
*/
QModelIndex HbAbstractViewItem::modelIndex() const
{
    Q_D( const HbAbstractViewItem );
    return d->mIndex;
}

/*!
    \deprecated HbAbstractViewItem::type() const
        is deprecated.

    \reimp
*/
int HbAbstractViewItem::type() const
{
    qWarning("HbAbstractViewItem::type() const is deprecated");

    return Hb::ItemType_AbstractViewItem;
}


/*!
    Sets model \a index where this Abstract view item should retrieve it's content.
*/
void HbAbstractViewItem::setModelIndex(const QModelIndex &index)
{
    Q_D( HbAbstractViewItem );
    if (d->mIndex != index) {
        d->mIndex = index;

        updateChildItems();
    }
}

/*!
    \deprecated HbAbstractViewItem::receivedFocus()
        is deprecated.

    Called when item has received focus.
*/
void HbAbstractViewItem::receivedFocus()
{
    qWarning("HbAbstractViewItem::receivedFocus() is deprecated!");

    Q_D( HbAbstractViewItem );
    d->mFocused = true;
}

/*!
    \deprecated HbAbstractViewItem::lostFocus()
        is deprecated.

    Called when item has lost focus.
*/
void HbAbstractViewItem::lostFocus()
{
    qWarning("HbAbstractViewItem::lostFocus() is deprecated!");

    Q_D( HbAbstractViewItem );
    d->mFocused = false;
}

/*!
    \deprecated HbAbstractViewItem::isFocused() const
        is deprecated.

    Retuns true if item is currently focused, otherwise return false.
*/
bool HbAbstractViewItem::isFocused() const
{
    qWarning("HbAbstractViewItem::isFocused() is deprecated!");

    Q_D( const HbAbstractViewItem );
    return d->mFocused;
}

/*!
    Returns the saved item's transient state. Transient state can be seen as a state data that is 
    wanted to be preserved but it not meaningful to be stored inside model index because of it's
    momentary nature. States will be saved inside AbstractItemview and restored when current model index is
    assigned to certain Abstract view item.

    String in the returned table is usually name of a Qt property.
    Default values of properties should not be added into returned table.

    Derived class should first call base class implementation. After that it adds its own
    state items into returned table.
*/
QHash<QString, QVariant> HbAbstractViewItem::transientState() const
{
    Q_D( const HbAbstractViewItem );
    QHash<QString,QVariant> state;
    
    if (d->mFocused) {
        state.insert("focused", d->mFocused);
    }
    if (d->mCheckState != Qt::Unchecked) {
        state.insert("checkState", d->mCheckState);
    }

    return state;
}

/*!
    Sets the item's transient state using given \a state data.
*/
void HbAbstractViewItem::setTransientState(const QHash<QString, QVariant> &state)
{
    Q_D( HbAbstractViewItem );
    d->mFocused = state.value("focused").toBool();
    d->mCheckState = (Qt::CheckState)state.value("checkState").toInt();
}


/*!
     \deprecated HbAbstractViewItem::state() const
        is deprecated. Please use HbAbstractViewItem::transientState() instead. 

    Returns the saved item's transient state. Transient state can be seen as a state data that is 
    wanted to be preserved but it not meaningful to be stored inside model index because of it's
    momentary nature. States will be saved inside AbstractItemview and restored when current model index is
    assigned to certain Abstract view item.
*/
QMap<int,QVariant> HbAbstractViewItem::state() const
{
    qWarning("HbAbstractViewItem::state() const is deprecated");
    HB_ITEM_STATE_ASSERT;
    Q_D( const HbAbstractViewItem );
    QMap<int,QVariant> state;

    state.insert(FocusKey, d->mFocused);
    state.insert(CheckStateKey, d->mCheckState);

    return state;
}

/*!
     \deprecated HbAbstractViewItem::setState(const QMap<int,QVariant> &)
        is deprecated. Please use HbAbstractViewItem::setTransientState(const QHash<QString, QVariant> &state) instead. 

    Restores the item's transient state using given \a state data.
*/
void HbAbstractViewItem::setState(const QMap<int,QVariant> &state)
{
    qWarning("HbAbstractViewItem::setState(const QMap<int,QVariant> &state) is deprecated");
    HB_ITEM_STATE_ASSERT;
    Q_D( HbAbstractViewItem );
    if (state.contains(FocusKey)) {
        d->mFocused = state.value(FocusKey).toBool();
    } else {
        d->mFocused = false; 
    }

    if (state.contains(CheckStateKey)) {
        d->mCheckState = (Qt::CheckState)state.value(CheckStateKey).toInt();
    } else {
        d->mCheckState = Qt::Unchecked;
    }
}

/*!
    Returns a pointer to HbAbstractViewItem prototype that was used to create this
    view item.
*/
HbAbstractViewItem *HbAbstractViewItem::prototype() const
{
    HB_SDD( const HbAbstractViewItem );
    return sd->mPrototype;
}

/*!
    Sets \a AbstractItemView that contains the Abstract view item.
*/
void HbAbstractViewItem::setItemView( HbAbstractItemView *itemView )
{
    HB_SDD( HbAbstractViewItem );
    sd->mItemView = itemView;
}

/*!
    Returns item view that contains the item view item.
*/
HbAbstractItemView *HbAbstractViewItem::itemView() const
{
    HB_SDD( const HbAbstractViewItem );
    return sd->mItemView;
}

/*!
    Populates a style option object for this widget based on its current state, and stores the output in \a option.
*/
void HbAbstractViewItem::initStyleOption(HbStyleOptionAbstractViewItem *option) const
{
    HB_SDD( const HbAbstractViewItem );

    HbWidget::initStyleOption(option);

    option->modelItemType = d->mModelItemType;
    option->index = d->mIndex;

    option->viewItemType = type();
    option->checkState = d->mCheckState;
    option->background = d->mBackground;
    if (!option->background.isValid()) {
        if (option->modelItemType == Hb::StandardItem 
            && !sd->mDefaultFrame.isNull()) {
            option->background = sd->mDefaultFrame;
        }
    }

    if (d->mPressed) {
        option->state |= QStyle::State_Sunken;
    }
    if (    sd->mItemView
        &&  sd->mItemView->selectionMode() == HbAbstractItemView::SingleSelection){
        option->singleSelectionMode = true;
    }

    option->insidePopup = testAttribute(Hb::InsidePopup);
}

/*!
    Check whether \a position is inside the selection area of the given selectionAreaType in the view item.

    Default selection areas are for
    \li HbAbstractViewItem::SingleSelection mode: whole item
    \li HbAbstractViewItem::MultiSelection mode: whole item.
    \li HbAbstractItemView::ContiguousSelection mode: whole item. (Note: HbAbstractItemView::ContiguousSelection is deprecated.)
    \li HbAbstractItemView::NoSelection mode: none

    The \a selectionAreaType tells what kind of selection area is requested.  The parameter value ContiguousSelection returns 
	the area where mouse movement will extend the selection to new items. By default this contiguous selection area is 
	the HbStyle::P_ItemViewItem_touchmultiselection.
    
*/
bool HbAbstractViewItem::selectionAreaContains(const QPointF &position, SelectionAreaType selectionAreaType) const
{
    Q_D(const HbAbstractViewItem);
    bool contains = false;
    if (selectionAreaType == ContiguousSelection) {
        if(     d->mMultiSelectionTouchArea 
            &&  !d->mMultiSelectionTouchArea->boundingRect().isEmpty()) {
                contains = d->mMultiSelectionTouchArea->boundingRect().contains(mapToItem(d->mMultiSelectionTouchArea, position));
            } else if (d->mSelectionItem) {
                contains = d->mSelectionItem->boundingRect().contains(mapToItem(d->mMultiSelectionTouchArea, position));
            }
    } else {
        switch (selectionAreaType) {
            case SingleSelection: 
            case MultiSelection: 
            case ContiguousSelection: 
                contains = true;
                break;
            default:
                break;
        }
    }
    return contains;
}


/*!
    \reimp
*/
bool HbAbstractViewItem::event(QEvent *e)
{
    if (e) {
        switch (e->type()) {
            case QEvent::GraphicsSceneResize: {
                Q_D(HbAbstractViewItem );
                if (d->mBackgroundItem || d->mFrame || d->mFocusItem) {
                    HbStyleOptionAbstractViewItem styleOption;
                    initStyleOption(&styleOption);
                    if (d->mFocusItem) {
                        style()->updatePrimitive(d->mFocusItem, HbStyle::P_ItemViewItem_focus, &styleOption);
                    }

                    if (d->mFrame) {
                        style()->updatePrimitive(d->mFrame, HbStyle::P_ItemViewItem_frame, &styleOption);
                    }

                    if (d->mBackgroundItem) {
                        style()->updatePrimitive(d->mBackgroundItem, HbStyle::P_ItemViewItem_background, &styleOption);
                    }
                }
                break;
            }
            case QEvent::LayoutDirectionChange: {
                repolish();
                break;
            }
            default: {
                if (e->type() == HbAbstractViewItemShared::ViewItemDeferredDeleteEvent) {
                    // cannot handle ViewItemDeferredDeleteEvent in the case statement!
                    Q_D(HbAbstractViewItem);
                    delete d->mFocusItem;
                    d->mFocusItem = 0;
               }
                break;
            }
        }

        return HbWidget::event(e);
    }

    return false;
}

/*!
    \reimp

    Invalidates parent layout when ItemTransformHasChanged is received.
*/
QVariant HbAbstractViewItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
        case ItemTransformHasChanged: {
            QGraphicsLayoutItem *parentLayoutItem = this->parentLayoutItem();
            if (parentLayoutItem && parentLayoutItem->isLayout()) {
                QGraphicsLayout *parentLayout = static_cast<QGraphicsLayout *>(parentLayoutItem);
                parentLayout->invalidate();
            }
            break;
        }
        default:
            break;
    }

    return HbWidget::itemChange(change, value);
}

/*!

    \deprecated HbAbstractViewItem::primitive(HbStyle::Primitive)
       is deprecated.

  Provides access to primitives of HbAbstractViewItem.
  \param primitive is the type of the requested primitive. The available primitives are 
  \c P_ItemViewItem_background
  \c P_ItemViewItem_frame
  \c P_ItemViewItem_selection
  \c P_ItemViewItem_focus
  \c P_ItemViewItem_touchmultiselection.
 */
QGraphicsItem *HbAbstractViewItem::primitive(HbStyle::Primitive primitive) const
{
    Q_D(const HbAbstractViewItem);

    if (primitive == HbStyle::P_ItemViewItem_background) {
        return d->mBackgroundItem;
    } else if (primitive == HbStyle::P_ItemViewItem_frame) {
        return d->mFrame;
    } else if (primitive == HbStyle::P_ItemViewItem_selection) {
        return d->mSelectionItem;
    } else if (primitive == HbStyle::P_ItemViewItem_focus) {
        return d->mFocusItem;
    } else if (primitive == HbStyle::P_ItemViewItem_touchmultiselection) {
        return d->mMultiSelectionTouchArea;
    }

    return HbWidget::primitive(primitive);
}

/*!
    \reimp

    To optimise loading css/xml definitions to take place only once, this function should be
    called only after other primitives (child items) has been created.

*/
void HbAbstractViewItem::updatePrimitives()
{
    Q_D( HbAbstractViewItem);
    HbWidget::updatePrimitives();

    // For debugging primitives
#if 0
    {
        QStringList listClasses;
        listClasses << "HbTreeViewItem";
        const QMetaObject *meta = metaObject();
        int count = listClasses.count();
        for (int i=0; i< count; i++) {
            if ( meta->className() == listClasses.at(i)) {
                qDebug() << "HbAbstractViewItem::updatePrimitives(): widget, row, item count, check state" 
                    << listClasses.at(i) << modelIndex().row() << childItems().count() << d->mCheckState;;
                int count = childItems().count();
                for (int i=0; i< count; i++) {
                    if (childItems().at(i)) {
                        HbTextItem *textItem = 0;
                        if (childItems().at(i)->isWidget()) {
                            textItem = qobject_cast<HbTextItem*>(static_cast<QGraphicsWidget*>(childItems().at(i)));
                        }
                        if (textItem) {
                            qDebug() << "  item #, item name, id: " << i << childItems().at(i)->data(0xfffe).toString() << textItem->text();
                        } else {
                            qDebug() << "  item #, item name: " << i << childItems().at(i)->data(0xfffe).toString();
                        }
                    }
                }
            }
        }
    }
#endif


    HbStyleOptionAbstractViewItem styleOption;
    initStyleOption(&styleOption);

    if (d->mBackgroundItem) {
        style()->updatePrimitive(d->mBackgroundItem, HbStyle::P_ItemViewItem_background, &styleOption);
    }

    if (d->mFrame) {
        style()->updatePrimitive(d->mFrame, HbStyle::P_ItemViewItem_frame, &styleOption);
    }

    if (d->mSelectionItem) {
        style()->updatePrimitive(d->mSelectionItem, HbStyle::P_ItemViewItem_selection, &styleOption);
    }

    if (d->mMultiSelectionTouchArea) {
        style()->updatePrimitive(d->mMultiSelectionTouchArea, HbStyle::P_ItemViewItem_touchmultiselection, &styleOption);
    }

    if (d->mFocusItem) {
        style()->updatePrimitive(d->mFocusItem, HbStyle::P_ItemViewItem_focus, &styleOption);
    }
}


/*!
    Updates child graphics items to represent current state and content. 

    \note It is a good habit to reuse child items as much as possible as this improves
    performance, especially when item recycling is used. 

    Most of the HbAbstractViewItem derived classes inside Hb library are optimised for performance.
    Layout files are loaded only if child items are created or deleted. Loading layout
    files is triggered by calling HbWidget::repolish(). 
    Classes deriving from HbAbstractViewItem outside Hb, should either always call explicitly
    repolish() or if they are also optimised for performance only when child items are created or deleted
    in the custom view item.

    Here is an example of custom view item that reuses its child items. The custom view item does not
    create or delete child items.

    \snippet{ultimatecodesnippet/customlistviewitem.cpp,2}

    \sa HbWidget::polish()
*/
void HbAbstractViewItem::updateChildItems()
{
    HB_SDD(HbAbstractViewItem);

    int itemType = d->modelItemType();
    if (itemType != d->mModelItemType) {
        d->mModelItemType = itemType;
        d->mItemsChanged = true;
        d->themingPending = true;
    }

    /* Summary of background and frame handling:
         d->mBackground is read from Qt::BackgroundRole of model
         d->mBackgroundItem is created from d-mBackground (Qt::BackgroundRole), if this is HbIcon or QBrush.

         If d->mBackgroundItem does not exist, d->mFrame is created from d-mBackground (Qt::BackgroundRole), 
         if this is HbFrameBackground otherwise it either is created from sd->mDefaultFrame, 
         not created at all or from system default.
    */
 
    // background
    QVariant currentBackground = d->mIndex.data(Qt::BackgroundRole);
    if (currentBackground != d->mBackground) {
        d->mBackground = currentBackground;
        if (currentBackground.canConvert<HbIcon>() 
            || currentBackground.canConvert<QBrush>()) {
            if (!d->mBackgroundItem) {  
                d->mItemsChanged = true;
                d->mBackgroundItem = style()->createPrimitive(HbStyle::P_ItemViewItem_background, this);
                delete d->mFrame;
                d->mFrame = 0;
            }
        } else if (currentBackground.canConvert<HbFrameBackground>()) {
            if (!d->mFrame) {
                d->mItemsChanged = true;
                d->mFrame = style()->createPrimitive(HbStyle::P_ItemViewItem_frame, this);
                delete d->mBackgroundItem;
                d->mBackgroundItem = 0;
            }
        } else if (d->mBackgroundItem) {
            d->mItemsChanged = true;
            delete d->mBackgroundItem;
            d->mBackgroundItem = 0;
        }
    }

    // frame
    if (!d->mBackgroundItem) {
        if (    d->mModelItemType == Hb::ParentItem
            ||  d->mModelItemType == Hb::SeparatorItem
            ||  (   d->mModelItemType == Hb::StandardItem
                &&  (   d->mBackground.canConvert<HbFrameBackground>()
                    ||  sd->mDefaultFrame.frameGraphicsName().length() > 0    
                    ||  sd->mDefaultFrame.isNull()))) { 
            if (!d->mFrame) {
                d->mItemsChanged = true;
                d->mFrame = style()->createPrimitive(HbStyle::P_ItemViewItem_frame, this);
            }
        } else if (d->mFrame) {
            d->mItemsChanged = true;
            delete d->mFrame;
            d->mFrame = 0;
        }
    } 

    GraphicsItemFlags itemFlags = flags();
    Qt::ItemFlags indexFlags = d->mIndex.flags();

    if (indexFlags & Qt::ItemIsEnabled) {
        if (!(itemFlags & QGraphicsItem::ItemIsFocusable)) {
            itemFlags |= QGraphicsItem::ItemIsFocusable;
            setFocusPolicy(sd->mPrototype->focusPolicy());
            setProperty("state", "normal");
        grabGesture(Qt::TapGesture);
        }
    } else {
        if (itemFlags & QGraphicsItem::ItemIsFocusable) {
            itemFlags &= ~QGraphicsItem::ItemIsFocusable;
            setFocusPolicy(Qt::NoFocus);
            setProperty("state", "disabled");
        ungrabGesture(Qt::TapGesture);
        }
    }

    // selection
    HbAbstractItemView::SelectionMode selectionMode = HbAbstractItemView::NoSelection;
    if (sd->mItemView) {
        selectionMode = sd->mItemView->selectionMode();
    }

    bool previousSelectable = itemFlags & QGraphicsItem::ItemIsSelectable;
    bool itemSelectable = false;

    if (indexFlags & Qt::ItemIsSelectable 
        && selectionMode != HbAbstractItemView::NoSelection
        && indexFlags & Qt::ItemIsEnabled) {
        itemFlags |= QGraphicsItem::ItemIsSelectable;
        itemSelectable = true;
    } else {
        itemFlags &= ~QGraphicsItem::ItemIsSelectable;
    }

    if (previousSelectable != itemSelectable) {
        if (itemSelectable) {
            if (!d->mSelectionItem) {
                d->mItemsChanged = true;
                d->mSelectionItem = style()->createPrimitive(HbStyle::P_ItemViewItem_selection, this);
            }
        } else {
            d->mItemsChanged = true;
            delete d->mSelectionItem;
            d->mSelectionItem = 0;
        }
    }

    setFlags(itemFlags);

    // multiselection area
    if (    itemSelectable 
        &&  (   selectionMode == HbAbstractItemView::MultiSelection
            ||  selectionMode == HbAbstractItemView::ContiguousSelection)) {
        if (!d->mMultiSelectionTouchArea) {
            d->mItemsChanged = true;
            d->mMultiSelectionTouchArea = style()->createPrimitive(HbStyle::P_ItemViewItem_touchmultiselection, this);
        }
    } else if (d->mMultiSelectionTouchArea) {
        d->mItemsChanged = true;
        delete d->mMultiSelectionTouchArea;
        d->mMultiSelectionTouchArea = 0;
    }

    // items visibility or items content has really changed
    d->mSizeHintPolish = false;
    updatePrimitives();
    if (!d->mContentChangedSupported
        || d->mItemsChanged) {
        updateGeometry();   // ensures that sizehint is calculated again in case items have been created or deleted
        d->mRepolishRequested = true;
        repolish();
    }
    d->mItemsChanged = false;
}

/*!
    Sets the check state of the view item to state.

    \sa checkState().
*/
void HbAbstractViewItem::setCheckState(Qt::CheckState state)
{
    Q_D(HbAbstractViewItem);
    if (state != d->mCheckState) {
        d->mCheckState = state;
        updatePrimitives();
    }
}

/*!
    Returns the checked state of the view item (see Qt::CheckState).

    \sa setCheckState().
*/
Qt::CheckState HbAbstractViewItem::checkState() const
{
    Q_D(const HbAbstractViewItem);
    return d->mCheckState;
}

/*!
    \deprecated HbAbstractViewItem::setPressed(bool, bool)
        is deprecated.

    Sets the item press state to \a pressed. Animation is allowed
    if \a animate is set as true; otherwise animation should not
    be triggered.

    \sa isPressed, pressStateChanged
*/
void HbAbstractViewItem::setPressed(bool pressed, bool animate)
{
    qWarning("HbAbstractViewItem::setPressed(bool pressed, bool animate) is deprecated!");

    Q_D(HbAbstractViewItem);

    if (pressed != d->mPressed) {
        d->mPressed = pressed;
        pressStateChanged(d->mPressed, animate);
        if (d->mPressed) {
            setProperty("state", "pressed");
        } else {
            setProperty("state", "normal");
        }
    }
}

/*!
    This function is called whenever item press state changes. \a pressed is new state.

    Animation is allowed if \a animate is set as true; otherwise animation should not
    be triggered.

    Default implementation creates focus frame if item is currently pressed 
    and deletes the focus frame if item is not anymore pressed. It also triggers
    default animations.

    \sa setPressed
*/
void HbAbstractViewItem::pressStateChanged(bool pressed, bool animate)
{
    HB_SDD(HbAbstractViewItem);

    bool doAnimate = animate;
    if (sd->mItemView && !(sd->mItemView->enabledAnimations() & HbAbstractItemView::TouchDown)) {
        doAnimate = false;
    }
    if (pressed) {
        if (!d->mFocusItem) {
            d->mFocusItem = style()->createPrimitive(HbStyle::P_ItemViewItem_focus, this);
        }
            
        HbStyleOptionAbstractViewItem styleOption;
        initStyleOption(&styleOption);

        style()->updatePrimitive(d->mFocusItem, HbStyle::P_ItemViewItem_focus, &styleOption);

        if (doAnimate) {
            HbEffect::cancel(this, "released");
            HbEffect::cancel(d->mFocusItem, "released");

            HbEffect::start(this, sd->mItemType, "pressed");
            HbEffect::start(d->mFocusItem, sd->mItemType + QString("-focus"), "pressed");
        }
    } else {
        if (doAnimate) {
            HbEffect::cancel(this, "pressed");
            HbEffect::cancel(d->mFocusItem, "pressed");

            HbEffect::start(this, sd->mItemType, "released");
            HbEffect::start(d->mFocusItem, sd->mItemType + QString("-focus"), "released", this, "_q_animationFinished");
        } else {
            HbEffect::cancel(this, "pressed");
            HbEffect::start(this, sd->mItemType, "released");
            if (d->mFocusItem) {
                HbEffect::cancel(d->mFocusItem, "pressed");
                HbEffect::start(d->mFocusItem, sd->mItemType + QString("-focus"), "released", this, "_q_animationFinished");
                QCoreApplication::postEvent(this, new QEvent((QEvent::Type)HbAbstractViewItemShared::ViewItemDeferredDeleteEvent));
            }
        }
    }
}

/*!
    \deprecated HbAbstractViewItem::isPressed() const
        is deprecated.

    Returns true if the item is pressed; otherwise returns false

    \sa setPressed
*/
bool HbAbstractViewItem::isPressed() const
{
    qWarning("HbAbstractViewItem::isPressed() const is deprecated!");

    Q_D(const HbAbstractViewItem);
    return d->mPressed;
}

/*!
    Returns the model item type that is retrieved from model index.
*/
Hb::ModelItemType HbAbstractViewItem::modelItemType() const
{
    Q_D(const HbAbstractViewItem);
    return (Hb::ModelItemType)d->mModelItemType;
}

/*!
    \reimp
*/
void HbAbstractViewItem::polish(HbStyleParameters& params)
{
    HB_SDD(HbAbstractViewItem);

	if (!d->polished && layout()) {
		return;
	}
	
    if (d->mSizeHintPolish) {
        d->mSizeHintPolish = false;
        return;
    }

    if (sd->mItemView) {
        setProperty("layoutName", sd->mItemView->layoutName());
    }

    d->mRepolishRequested = false;
    HbWidget::polish(params);

    // TODO Brush background is overridden by css system even if bursh would not be set
    // explicitly by css/xml. This is feature, which will change
    // later in css system. Workaround for it. This overrides the background brush set by css. 
    {
        if (d->mBackground.isValid() 
            && d->mBackground.canConvert<QBrush>() 
            && d->mBackgroundItem
            && d->mBackgroundItem->isWidget()) {
            qgraphicsitem_cast<HbIconItem *>(static_cast<QGraphicsWidget*>(d->mBackgroundItem))->setBrush(d->mBackground.value<QBrush>());
        } 
    }
}

void HbAbstractViewItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    HbWidget::mousePressEvent(event);
}

void HbAbstractViewItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    HbWidget::mouseMoveEvent(event);
}

void HbAbstractViewItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    HbWidget::mouseReleaseEvent(event);
}

/*!
    \reimp
*/
QSizeF HbAbstractViewItem::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_D(const HbAbstractViewItem);
    if (d->mRepolishRequested) {
        // force the polish event in order to get the real size
        const_cast<HbAbstractViewItemPrivate *>(d)->mRepolishRequested = false;
        QEvent polishEvent(QEvent::Polish);
        QCoreApplication::sendEvent(const_cast<HbAbstractViewItem *>(this), &polishEvent);
        // HbAbstractItemView::scrollByAmount() [recycleItems()]
        // causes updateChildItems() to be called, which posts Polish to be posted via repolish().
        // Next statement in the scrollByAmount() [refreshContainerGeometry()]
        // causes synchronous call of this method. This is a quick way to disable another
        // ::polish() to be called.
        d->mSizeHintPolish = true;
    }
    return HbWidget::sizeHint(which, constraint);
}

/*!
    Sets the default frame for standard view items as \a frame.  
    
    This method will change the used frame for all view items that represent model index with Hb::StandardItem type.

    Input parameter with empty but non-null graphicsName string will remove the default frame.
    Input parameter with null graphicsName string will restore the system default frame.

    This method has not immediate effect, if Qt::BackgroundRole includes HbFrameBackground object. Qt::BackgroundRole of model
    has higher priority than any other frame type.

    Default frame is system default frame.

    \sa defaultFrame
*/
void HbAbstractViewItem::setDefaultFrame(const HbFrameBackground &frame)
{
    HB_SDD(HbAbstractViewItem);
    if (sd->mDefaultFrame != frame) {
        sd->mDefaultFrame = frame;
        
        int count(sd->mCloneItems.count());
        for (int i = 0; i < count; ++i) {
            sd->mCloneItems.at(i)->updateChildItems();
        }
    }
}

/*!
    Returns the current default frame.

    \sa setDefaultFrame
*/
HbFrameBackground HbAbstractViewItem::defaultFrame() const
{
    HB_SDD(const HbAbstractViewItem);
    return sd->mDefaultFrame;
}

/*!
    \reimp
*/
void HbAbstractViewItem::gestureEvent(QGestureEvent *event)
{
    if (event->gesture(Qt::TapGesture)) {
        Q_D(HbAbstractViewItem);
        d->tapTriggered(event);
    } else {
        HbWidget::gestureEvent(event);
    }
}


#include "moc_hbabstractviewitem.cpp"

