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
#include <hbstyle.h>
#include <hbiconitem.h>
#include <hbiconitem_p.h>
#include <hbframebackground.h>
#include <hbabstractitemview_p.h>
#include <hbwidgetfeedback.h>
#include <hbtapgesture.h>
#include <hbnamespace_p.h>
#include <hbevent.h>
#include <hbtextitem.h>
#include <hbiconitem.h>
#include <hbstyletextprimitivedata.h>
#include <hbstyleframeprimitivedata.h>
#include <hbstyleiconprimitivedata.h>
#include <hbstyletouchareaprimitivedata.h>

#include <QPersistentModelIndex>
#include <QGraphicsLayout>
#include <QVariant>
#include <QCoreApplication>
#include <QEvent>
#include <QTimer>
#include <QGraphicsScene>

#include <QPixmap>
#include <QChildEvent>

#include <QGesture>
#include <QGestureEvent>
#include <QGraphicsSceneEvent>

#include <QDebug>

const QString KDefaultLayoutOption = "default";
const int HbAbstractViewItemShared::ViewItemDeferredDeleteEvent = QEvent::registerEventType();
const int HbViewItemPressDelay = 100;

/*!
    @alpha
    @hbwidgets
    \class HbAbstractViewItem
    \brief The HbAbstractViewItem class represents a single item in a AbstractItemView.  

    The HbAbstractViewItem class provides an item that is used by HbAbstractItemView class to
    visualize content within single model index. 
    
    By default HbAbstractViewItem supports following model item types in Hb::ItemTypeRole role of data model
    \li Hb::StandardItem. This is normal item. Item is considered as Hb::StandardItem, if Hb::ItemTypeRole is not set.
    \li Hb::ParentItem. Item has children. This class provides default frame item for parent item, if Qt::BackgroundRole data is not specifically set.
    \li Hb::SeparatorItem. Separator is used as boundary between separate items. Item does not interact with user. This requires support from the model, too. Qt::ItemIsEnabled flag must be Off for the model item. This class provides default frame for separator item, if Qt::BackgroundRole data is not specifically set.

    Every subclass view item do not support every model item type. For more information see subclass documentation.

    Item data roles HbIcon, QBrush or HbFrameBackground are supported in Qt::BackgroundRole data role.
    Other supported item data roles can be found in subclass documentation.

    This class is provided mainly for customization purposes but it also acts as a default
    item prototype inside HbAbstractItemView. See HbAbstractItemView how to set customized class as a item prototype.

    \b Subclassing

    When subclassing HbAbstractViewItem, child class must provide implementations of createItem() and updateChildItems() functions.
    
    To support multiple Abstractview items within single AbstractItemview, you must also provide an implementation of canSetModelIndex().

    If derived abstract view item has transient state information that is not meaningful to store within model index (child item cursor 
    position selection areas etc.) this information can be supplied to transient state model. Transient state model is maintained 
    internally by abstract item view. 

    If item's pixmap cache is enabled derived class should call updatePixmapCache() when ever visual appearance of the item or its children is
    changed. For more information about enabling the pixmap cache see HbAbstractItemView::setItemPixmapCacheEnabled().

    \primitives
    \primitive{background} HbIconItem with item name "background" representing the item background. This primitive exists in cases the model's Qt::BackgroundRole returns HbIcon or QBrush for this item.
    \primitive{frame} HbFrameItem with item name "frame" representing the background frame of the item. This primitive exists if background primitive does not exist and the model's Qt::BackgroundRole returns HbFrameBackground or there is a default frame set with the setDefaultFrame(). An item can have either the frame or the background primitive, but not the both at the same time.
    \primitive{selection-icon} HbIconItem with item name "selection-icon" representing the checkbox in the multi selection mode.
    \primitive{multiselection-toucharea} HbTouchArea with item name "multiselection-toucharea" used in extending the touch area of the selection-icon. 
    \primitive{focus} HbFrameItem with item name "focus" for showing focus. 

    \sa HbListViewItem
    \sa HbGridViewItem
    \sa HbTreeViewItem
*/

/*!
    \enum HbAbstractViewItem::SelectionAreaType

    Enumeration specifies selection area types. 
    
    Multiselection selection mode may operate in contiguous selection mode, in which items are selected 
    or deselected by panning over items. Normal multiselection functionality is available also in this mode.
    Location of touch down gesture determines whether contiguous selection mode is activated.

    \sa HbAbstractViewItem::selectionAreaContains(const QPointF &position, SelectionAreaType selectionAreaType) const
*/

/*!
    \var HbAbstractViewItem::SingleSelection

    Selection area for single selection mode.

    \sa HbAbstractViewItem::selectionAreaContains(const QPointF &position, SelectionAreaType selectionAreaType) const
*/


/*!
    \var HbAbstractViewItem::MultiSelection

    Selection area for multiple selection mode.

    \sa HbAbstractViewItem::selectionAreaContains(const QPointF &position, SelectionAreaType selectionAreaType) const
*/


/*!
    \var HbAbstractViewItem::ContiguousSelection

    Selection area for contiguous selection mode. 

    \sa HbAbstractViewItem::selectionAreaContains(const QPointF &position, SelectionAreaType selectionAreaType) const
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
    Released signal is emitted also when user starts panning or flicks the view.
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

void HbAbstractViewItemShared::updateIconItemsAsyncMode()
{
    foreach (HbAbstractViewItem *item, mCloneItems) {
        QList<QGraphicsItem *> childItems = item->childItems();
        foreach (QGraphicsItem *child, childItems) {
            item->d_func()->updateIconItemsAsyncMode(child);
        }
    }
}

void HbAbstractViewItemShared::setItemView(HbAbstractItemView *view)
{
    if (view != mItemView) {
        if (mItemView) {
            disconnect(mItemView, SIGNAL(scrollingStarted()), this, SLOT(scrollingStarted()));
        }
        mItemView = view;
        if (mItemView) {
            connect(mItemView, SIGNAL(scrollingStarted()), this, SLOT(scrollingStarted()));
        }
    }
}

void HbAbstractViewItemShared::pressStateChangeTimerTriggered()
{
    if (mPressedItem) {
        HbWidgetFeedback::triggered(mPressedItem, Hb::InstantPressed, 0);
        mPressedItem->pressStateChanged(true, mAnimatePress);
    }
}

void HbAbstractViewItemShared::disablePixmapCaches()
{
    mLowGraphicsMemory = true;
    foreach (HbAbstractViewItem *item, mCloneItems) {
        item->update();
    }
}

void HbAbstractViewItemShared::enablePixmapCaches()
{
    mLowGraphicsMemory = false;
    foreach (HbAbstractViewItem *item, mCloneItems) {
        item->update();
    }
}

void HbAbstractViewItemShared::scrollingStarted()
{
    if (mItemView && mItemView->iconLoadPolicy() == HbAbstractItemView::LoadAsynchronouslyWhenScrolling) {
        updateIconItemsAsyncMode();
    }
}

void HbAbstractViewItemPrivate::init()
{
    Q_Q(HbAbstractViewItem);

    q->setProperty("state", "normal");
    q->setFlag(QGraphicsItem::ItemHasNoContents, false);

    if (isPrototype()) {
        q->setFocusPolicy(Qt::ClickFocus);
    } else {
        q->grabGesture(Qt::TapGesture);
        QGraphicsItem::GraphicsItemFlags itemFlags = q->flags();
        itemFlags |= QGraphicsItem::ItemIsFocusable;
        q->setFlags(itemFlags);

        q->setFocusPolicy(mSharedData->mPrototype->focusPolicy());

        mSharedData->mCloneItems.append(q);

        mFrontPixmapPainter = new HbViewItemPixmapPainter(1000, q);
        // setChildFlags gets called in the construction phase, but the values are wrong.
        // Here we set the flags to force FrontPixmap to obey parent opacity, this is needed in animations
        setChildFlags(mFrontPixmapPainter, false);

        mBackPixmapPainter = new HbViewItemPixmapPainter(-1000, q);
        // setChildFlags gets called in the construction phase, but the values are wrong.
        // Here we set the flags to force BackPixmap to obey parent opacity, this is needed in animations
        setChildFlags(mBackPixmapPainter, false);

        mChildren = q->childItems();
        q->connect(q, SIGNAL(childrenChanged()), q, SLOT(_q_childrenChanged()));
    }
}

/*!
    Returns Hb::ModelItemType of this view item.

    \sa Hb::ModelItemType
*/
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

void HbAbstractViewItemPrivate::_q_childrenChanged()
{
    Q_Q(HbAbstractViewItem);

    QList<QGraphicsItem *> childList = q->childItems();
    int currentCount = childList.count();
    int previousCount = mChildren.count();

    if (currentCount > previousCount) {
        QGraphicsItem *addedItem = 0;
        for (int i = 0; i < currentCount; ++i) {
            QGraphicsItem *item = childList.at(i);
            if (item != mChildren.value(i)) {
                addedItem = item;
                break;
            }
        }

        if (usePixmapCache()) {
            setChildFlags(addedItem, true);
        }

        updateIconItemsAsyncMode(addedItem);
    } else {
        if (mUpdateItems.count()) {
            int itemCount = mUpdateItems.count();
            for (int i = 0; i < itemCount; ++i) {
                QGraphicsItem *item = mUpdateItems.at(i);
                int index = childList.indexOf(item);
                if (index == -1) {
                    mUpdateItems.remove(i);
                    mUpdateItems.squeeze();
                }
            }
        }
    }

    mChildren = childList;
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
            q->scene()->setProperty(HbPrivate::OverridingGesture.latin1(),Qt::TapGesture);
            if (!gesture->property(HbPrivate::ThresholdRect.latin1()).toRect().isValid()) {
                gesture->setProperty(HbPrivate::ThresholdRect.latin1(), q->mapRectToScene(q->boundingRect()).toRect());
            }

            setPressed(true, true);
            emit q->pressed(position);
            break;
        }
        case Qt::GestureUpdated: {
            if (gesture->tapStyleHint() == HbTapGesture::TapAndHold 
                && mSharedData->mItemView
                && mSharedData->mItemView->longPressEnabled()) {
                q->scene()->setProperty(HbPrivate::OverridingGesture.latin1(),QVariant());
                setPressed(false, true);
                QPointer<HbAbstractViewItem> item = q;
                emit item->longPressed(position);
                if (item) {
                    revealItem();
                }
            }
            break;
        }
        case Qt::GestureFinished: {
            q->scene()->setProperty(HbPrivate::OverridingGesture.latin1(),QVariant());

            if (gesture->tapStyleHint() == HbTapGesture::Tap 
                || (mSharedData->mItemView
                && !mSharedData->mItemView->longPressEnabled())) {
                setPressed(false, true);

                HbWidgetFeedback::triggered(q, Hb::InstantReleased, 0);
                HbWidgetFeedback::triggered(q, Hb::InstantClicked);
                QPointer<HbAbstractViewItem> item = q;
                emit item->activated(position);
                // this viewItem may be deleted in the signal handling, so guarded pointer is used to 
                // to ensure that the item still exists when item is used
                if (item) {
                    emit item->released(position);
                    if (item) {
                        revealItem();
                    }
                }
            } else {
                HbWidgetFeedback::triggered(q, Hb::InstantReleased,0);
                emit q->released(position);
            }

            break;
        }
        case Qt::GestureCanceled: {
            q->scene()->setProperty(HbPrivate::OverridingGesture.latin1(),QVariant());
            // hides focus immediately
            setPressed(false, false);

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

void HbAbstractViewItemPrivate::setPressed(bool pressed, bool animate)
{
    Q_Q(HbAbstractViewItem);

    if (pressed != mPressed) {
        mPressed = pressed;

        if (mSharedData->mPressStateChangeTimer) {
            if(!pressed && animate && mSharedData->mPressStateChangeTimer->isActive()) {
                // Release happened while press still delayed
                mSharedData->mPressStateChangeTimer->stop();
                mSharedData->pressStateChangeTimerTriggered();
            } else {
                mSharedData->mPressStateChangeTimer->stop();
            }
        }

        if (mPressed) {
            if (!mSharedData->mPressStateChangeTimer) {
                mSharedData->mPressStateChangeTimer = new QTimer(mSharedData.data());
                mSharedData->mPressStateChangeTimer->setSingleShot(true);
                QObject::connect(mSharedData->mPressStateChangeTimer, SIGNAL(timeout()), mSharedData.data(), SLOT(pressStateChangeTimerTriggered()));
            }
            mSharedData->mPressedItem = q;
            mSharedData->mAnimatePress = animate;
            mSharedData->mPressStateChangeTimer->start(HbViewItemPressDelay);

            q->setProperty("state", "pressed");
        } else {
            q->pressStateChanged(mPressed, animate);

            q->setProperty("state", "normal");
        }
    }
}

void HbAbstractViewItemPrivate::drawSubPixmap(QPixmap *pixmap,
                                              QPainter *painter,
                                              const QStyleOptionGraphicsItem *option)
{
    Q_Q(HbAbstractViewItem);

    QPixmap subPix;
    QPainter pixmapPainter;
    QStyleOptionGraphicsItem pixmapOption(*option);

    QList<QGraphicsItem *> childList = q->childItems();

    foreach (QGraphicsItem *subChild, mUpdateItems) {
        pixmapOption.exposedRect = subChild->boundingRect();

        subPix = QPixmap(pixmapOption.exposedRect.toRect().size());
        subPix.fill(Qt::transparent);

        pixmapPainter.begin(&subPix);
        pixmapPainter.setRenderHints(pixmapPainter.renderHints(), false);
        pixmapPainter.setRenderHints(painter->renderHints(), true);
        
        // Draw items on the pixmap
        paintChildItemsRecursively(subChild, &pixmapPainter, &pixmapOption, QPointF());

        // Search & paint overlapping child items.
        QRectF subChildRectRelativeToParent(subChild->pos(), subChild->boundingRect().size());

        int currentIndex = childList.indexOf(subChild) + 1;
        int childCount = childList.count();
        while (currentIndex < childCount) {
            QGraphicsItem *item = childList.at(currentIndex);
            if (item != mNonCachableItem 
                && item != mFrontPixmapPainter 
                && item != mBackPixmapPainter) {
                QRectF itemRectRelativeToParent(item->pos(), item->boundingRect().size());

                QRectF intersectionRect(itemRectRelativeToParent.intersected(subChildRectRelativeToParent));
                if (!intersectionRect.isNull()) {
                    QStyleOptionGraphicsItem itemPixmapOption(*option);
                    itemPixmapOption.exposedRect = intersectionRect.translated(-item->pos());

                    paintChildItemsRecursively(item, &pixmapPainter, &itemPixmapOption, subChild->mapFromParent(itemRectRelativeToParent.topLeft()));
                }
            }
            ++currentIndex;
        }

        pixmapPainter.end();

        if (!subPix.isNull()) {
            // Blit the subpixmap into the main pixmap.
            pixmapPainter.begin(pixmap);
            pixmapPainter.translate(subChild->pos());
            pixmapPainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            pixmapPainter.setClipRect(pixmapOption.exposedRect);
            pixmapPainter.drawPixmap(pixmapOption.exposedRect.topLeft(), subPix);
            pixmapPainter.end();
        }
    }
    mUpdateItems.clear();
}

void HbAbstractViewItemPrivate::updatePixmap(QPixmap *pixmap, 
                                             QPainter *painter, 
                                             const QStyleOptionGraphicsItem *option, 
                                             QGraphicsItem *startItem, 
                                             QGraphicsItem *endItem)
{
    //Q_Q(HbAbstractViewItem);
    pixmap->fill(Qt::transparent);

    QPainter pixmapPainter;
    pixmapPainter.begin(pixmap);

    pixmapPainter.setRenderHints(pixmapPainter.renderHints(), false);
    pixmapPainter.setRenderHints(painter->renderHints());

    // Draw items on the pixmap
    QStyleOptionGraphicsItem pixmapOption(*option);
    paintItems(&pixmapPainter, &pixmapOption, startItem, endItem);
}

void HbAbstractViewItemPrivate::paintItems(QPainter *painter, QStyleOptionGraphicsItem *option, QGraphicsItem *startItem, QGraphicsItem *endItem)
{
    Q_Q(HbAbstractViewItem);

    mInPaintItems = true;

    bool itemPainted = false;

    bool startItemFound = false;
    if (!startItem) {
        startItemFound = true;
    }

    foreach (QGraphicsItem *child, q->childItems()) {
        if (!startItemFound) {
            if (child == startItem) {
                startItemFound = true;
            }
            continue;
        } else if (child == endItem) {
            break;
        }

        if (!child->isVisible() || child == mNonCachableItem || child == mFrontPixmapPainter || child == mBackPixmapPainter) {
            continue;
        }

        if (!itemPainted && child->zValue() >= 0) {
            option->exposedRect = q->boundingRect();
            q->paint(painter, option, 0);
            itemPainted = true;
        }

        if (child == mFrame) {
            QPainter::CompositionMode mode = painter->compositionMode();
            painter->setCompositionMode(QPainter::CompositionMode_Source);
            painter->translate(child->pos());
            option->exposedRect = child->boundingRect();
            child->paint(painter, option, 0);
            painter->setCompositionMode(mode);
            painter->translate(-child->pos());
            continue;
        }
        paintChildItemsRecursively(child,painter,option, child->pos());
    }
    mInPaintItems = false;
}

void HbAbstractViewItemPrivate::paintChildItemsRecursively(QGraphicsItem *child,
                                                           QPainter *painter,
                                                           QStyleOptionGraphicsItem *option,
                                                           const QPointF &translatePosition)
{
    if (!child->isVisible())
        return;
    int i = 0;
    QList<QGraphicsItem *> children =  child->childItems();
    int count(children.size());
    if (!translatePosition.isNull()) {
        painter->translate(translatePosition);
    }
    // Draw children behind
    for (i = 0; i < count; ++i) {
        QGraphicsItem *subChild = children.at(i);
        if (!(subChild->flags() & QGraphicsItem::ItemStacksBehindParent))
            break;
        paintChildItemsRecursively(subChild, painter,option, subChild->pos());
    }
    option->exposedRect = child->boundingRect();

    bool restorePainter = false;
    if (child->flags() & QGraphicsItem::ItemClipsToShape) {
        painter->save();
        restorePainter = true;

        painter->setClipRect(child->boundingRect());
    }
    child->paint(painter, option, 0);

    if (restorePainter) {
        painter->restore();
    }

    // Draw children in front
    for (; i < count; ++i) {
        QGraphicsItem *subChild = children.at(i);
        paintChildItemsRecursively(subChild, painter,option, subChild->pos());
    }

    if (!translatePosition.isNull()) {
        painter->translate(-translatePosition);
    }
}


void HbAbstractViewItemPrivate::setChildFlags(QGraphicsItem *child, bool pixmapCacheEnabled)
{
    QGraphicsItem::GraphicsItemFlags itemFlags = child->flags();
    if (pixmapCacheEnabled) {
        itemFlags |= QGraphicsItem::ItemHasNoContents;
        itemFlags |= QGraphicsItem::ItemIgnoresParentOpacity;
    }
    else {
        itemFlags &= ~QGraphicsItem::ItemHasNoContents;
        itemFlags &= ~QGraphicsItem::ItemIgnoresParentOpacity;
    }
    child->setFlags(itemFlags);
    foreach (QGraphicsItem *subChild, child->childItems()) {
        setChildFlags(subChild,pixmapCacheEnabled);
    }
}

void HbAbstractViewItemPrivate::setChildFlagRecursively(bool pixmapCacheEnabled)
{
    Q_Q(HbAbstractViewItem);
    foreach (QGraphicsItem *subChild, q->childItems()) {
        if (subChild == mNonCachableItem || subChild == mFrontPixmapPainter ||
            subChild == mBackPixmapPainter) {
            continue;
        }
        setChildFlags(subChild, pixmapCacheEnabled);
    }
}


void HbAbstractViewItemPrivate::releasePixmaps()
{
    if (mFrontPixmapPainter) {
        mFrontPixmapPainter->setPixmap(0);
    }

    if (mBackPixmapPainter) {
        mBackPixmapPainter->setPixmap(0);
    }
}

bool HbAbstractViewItemPrivate::iconLoadedCallback(HbIconItem *target, void *param)
{
    HbAbstractViewItemPrivate* d_ptr = (HbAbstractViewItemPrivate*)param;
    return d_ptr->iconLoaded(target);
}

bool HbAbstractViewItemPrivate::iconLoaded(HbIconItem *target)
{
    Q_Q(HbAbstractViewItem);
    
    bool result = false;

    if (usePixmapCache()) {
        mUpdateItems.append(target);
        if (mSharedData->mItemView && !mSharedData->mItemView->isScrolling()) {
            q->update();
        }

        result = true;
    }

    return result;
}

void HbAbstractViewItemPrivate::updateIconItemsAsyncMode(QGraphicsItem *item)
{
    if (mSharedData->mItemView) {
        HbIconItem *iconItem = qgraphicsitem_cast<HbIconItem *>(item);
        if (iconItem) { 
            HbAbstractItemView::IconLoadPolicy loadPolicy = mSharedData->mItemView->iconLoadPolicy();
            if (loadPolicy == HbAbstractItemView::LoadAsynchronouslyAlways 
                || (loadPolicy == HbAbstractItemView::LoadAsynchronouslyWhenScrolling
                && mSharedData->mItemView->isScrolling())) {
                HbIconItemPrivate::d_ptr(iconItem)->setAsyncCallbackFilter(&HbAbstractViewItemPrivate::iconLoadedCallback, this);
                iconItem->setAsync(true);
            } else {
                iconItem->setAsync(false);
            }
        }
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
    HbAbstractViewItemPrivate *d = d_func();
    if (d && !d->isPrototype()) {
        d->mSharedData->mCloneItems.removeOne(this);
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
    if (state.count()) {
        d->mCheckState = (Qt::CheckState)state.value("checkState").toInt();
    } else
        d->mCheckState = Qt::Unchecked;
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
    sd->setItemView(itemView);
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
    Check whether \a position is inside the selection area of the given selectionAreaType in the view item.

    Default selection areas are for
    \li HbAbstractViewItem::SingleSelection mode: whole item
    \li HbAbstractViewItem::MultiSelection mode: whole item.
    \li HbAbstractViewItem::ContiguousSelection mode: area of icon of primitive with item name "multiselection-toucharea".

    The \a selectionAreaType tells what kind of selection area is requested.  The parameter value ContiguousSelection returns 
    the area where mouse movement will extend the selection to new items. By default this contiguous selection is area of
    "multiselection-toucharea" primitive.
    
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
    switch (e->type()) {
        case QEvent::GraphicsSceneResize: {
            Q_D(HbAbstractViewItem );            
            QGraphicsSceneResizeEvent *resizeEvent = static_cast<QGraphicsSceneResizeEvent *>(e);

            if (d->mFrontPixmapPainter) {
                d->mFrontPixmapPainter->setSize(resizeEvent->newSize());
            }

            if (d->mBackPixmapPainter) {
                d->mBackPixmapPainter->setSize(resizeEvent->newSize());
            }

            if (d->mBackgroundItem || d->mFrame || d->mFocusItem) {
                if (d->mFocusItem) {
                    // HbFrameItem
                    HbStyleFramePrimitiveData primitiveData;
                    initPrimitiveData(&primitiveData, d->mFocusItem);
                    style()->updatePrimitive(d->mFocusItem, &primitiveData, this);
                }

                if (d->mFrame) {
                    // HbFrameItem
                    HbStyleFramePrimitiveData primitiveData;
                    initPrimitiveData(&primitiveData, d->mFrame);
                    style()->updatePrimitive(d->mFrame, &primitiveData, this);
                }

                if (d->mBackgroundItem) {
                    // HbIconItem
                    HbStyleIconPrimitiveData primitiveData;
                    initPrimitiveData(&primitiveData, d->mBackgroundItem);
                    style()->updatePrimitive(d->mBackgroundItem, &primitiveData, this);
                }
            }
            break;
        }
        case QEvent::LayoutDirectionChange: {
            repolish();
            updatePixmapCache();
            break;  
        }
        case QEvent::LayoutRequest: {
            updatePixmapCache();
            break;
        }
        default: {
            if (e->type() == HbAbstractViewItemShared::ViewItemDeferredDeleteEvent) {
                // cannot handle ViewItemDeferredDeleteEvent in the case statement!
                Q_D(HbAbstractViewItem);

                d->mNonCachableItem = 0;

                delete d->mFocusItem;
                d->mFocusItem = 0;

                updatePixmapCache();
            }
            break;
        }
    }
    return HbWidget::event(e);
}

/*!
    \reimp
*/
void HbAbstractViewItem::changeEvent(QEvent *event)
{
    if (event->type() == HbEvent::ThemeChanged) {
        updatePixmapCache();
    }
    HbWidget::changeEvent(event);
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
        case ItemEnabledHasChanged: {
            updateChildItems();
            break;
        }
        case ItemVisibleHasChanged: {
            Q_D(HbAbstractViewItem);
            if (!value.toBool() && d->usePixmapCache()) {
                d->setChildFlagRecursively(false);
                d->releasePixmaps();
                d->mResetPixmapCache = true;
            }
            break;
        }
        default:
            break;
    }

    return HbWidget::itemChange(change, value);
}

/*!
  Initializes the HbAbstractViewItem primitive data. 
  
  This function calls HbWidgetBase::initPrimitiveData().
  \a primitiveData is data object, which is populated with data. \a primitive is the primitive.
*/
void HbAbstractViewItem::initPrimitiveData( HbStylePrimitiveData     *primitiveData, 
                                            const QGraphicsObject    *primitive)
{
    Q_ASSERT_X(primitive && primitiveData, "HbAbstractViewItem::initPrimitiveData" , "NULL data not permitted");
    HB_SDD(HbAbstractViewItem);

    HbWidgetBase::initPrimitiveData(primitiveData, primitive);
    if (primitiveData->type == HbStylePrimitiveData::SPD_Icon) {
        HbStyleIconPrimitiveData *iconPrimitiveData = hbstyleprimitivedata_cast<HbStyleIconPrimitiveData*>(primitiveData);

        if (primitive ==d-> mBackgroundItem) {
            iconPrimitiveData->geometry = boundingRect();

            QVariant background = d->mBackground;
            if (!background.isValid()) {
                if (    d->mModelItemType == Hb::StandardItem 
                    && !sd->mDefaultFrame.isNull()) {
                    background = sd->mDefaultFrame;
                }
            }

            if (background.canConvert<HbIcon>()){
                iconPrimitiveData->icon = background.value<HbIcon>();
                iconPrimitiveData->brush = QBrush();
            } else if (background.canConvert<QBrush>()){
                iconPrimitiveData->icon = HbIcon();
                iconPrimitiveData->brush = background.value<QBrush>();
            } else {
                iconPrimitiveData->icon = HbIcon();
                iconPrimitiveData->brush = QBrush();
            }
        } else if (primitive == d->mSelectionItem) {
            int viewItemType = type();
            bool singleSelectionMode = false;
            if (sd->mItemView
                &&  sd->mItemView->selectionMode() == HbAbstractItemView::SingleSelection){
                singleSelectionMode = true;
            }

            if (viewItemType == Hb::ItemType_RadioButtonListViewItem) {
                if (d->mCheckState == Qt::Checked) {
                    iconPrimitiveData->iconState = QIcon::On;
                    if (isEnabled()) {
                        iconPrimitiveData->iconName = QLatin1String("qtg_small_radio_selected");
                    } else {
                        iconPrimitiveData->iconName = QLatin1String("qtg_small_radio_selected_disabled");
                    }
                } else {
                    iconPrimitiveData->iconState = QIcon::Off;
                    if (isEnabled()) {
                        iconPrimitiveData->iconName = QLatin1String("qtg_small_radio_unselected");
                    } else {
                        iconPrimitiveData->iconName = QLatin1String("qtg_small_radio_unselected_disabled");
                    }
                }
            } else {
                if (d->mCheckState == Qt::Checked) {
                    iconPrimitiveData->iconState = QIcon::On;
                    if (singleSelectionMode) {
                        iconPrimitiveData->iconName = QLatin1String("qtg_small_tick");
                    } else {
                        iconPrimitiveData->iconName = QLatin1String("qtg_small_selected");
                    }
                } else if (d->mCheckState == Qt::PartiallyChecked) {
                    iconPrimitiveData->iconState = QIcon::On;
                    iconPrimitiveData->iconName = QLatin1String("qtg_small_selected_partial");
                } else {
                    iconPrimitiveData->iconState = QIcon::Off;
                    if (singleSelectionMode) {
                        iconPrimitiveData->iconName = QLatin1String("");
                    } else {
                        iconPrimitiveData->iconName = QLatin1String("qtg_small_unselected");
                    }
                }
            }
        }
    } else if (primitiveData->type == HbStylePrimitiveData::SPD_Frame) {
        HbStyleFramePrimitiveData *framePrimitiveData = hbstyleprimitivedata_cast<HbStyleFramePrimitiveData*>(primitiveData);

        if (primitive == d->mFrame ) {
            framePrimitiveData->geometry = boundingRect();

            QVariant background = d->mBackground;
            if (!background.isValid()) {
                if (    d->mModelItemType == Hb::StandardItem 
                    && !sd->mDefaultFrame.isNull()) {
                    background = sd->mDefaultFrame;
                }
            }

            if (background.canConvert<HbFrameBackground>()) {
                HbFrameBackground frame = background.value<HbFrameBackground>();
                framePrimitiveData->frameType = frame.frameType();
                framePrimitiveData->frameGraphicsName = frame.frameGraphicsName();
            } else {
                int viewItemType = type();
                bool insidePopup = testAttribute(Hb::InsidePopup);
                if (viewItemType == Hb::ItemType_TreeViewItem) {
                    if (d->mModelItemType == Hb::ParentItem) {
                        framePrimitiveData->frameType = HbFrameDrawer::NinePieces;
                        framePrimitiveData->frameGraphicsName = insidePopup ?
                            QLatin1String("qtg_fr_popup_list_parent_normal") : QLatin1String("qtg_fr_list_parent_normal");
                    } else if (d->mModelItemType == Hb::SeparatorItem) {
                        framePrimitiveData->frameType = HbFrameDrawer::NinePieces;
                        framePrimitiveData->frameGraphicsName = QLatin1String("qtg_fr_list_separator");
                    } else {
                        framePrimitiveData->frameType = HbFrameDrawer::NinePieces;
                        framePrimitiveData->frameGraphicsName =  insidePopup ?
                            QLatin1String("qtg_fr_popup_list_normal") : QLatin1String("qtg_fr_list_normal");
                    }
                } else if (viewItemType == Hb::ItemType_ListViewItem
                            ||  viewItemType == Hb::ItemType_RadioButtonListViewItem) {
                    if (d->mModelItemType == Hb::SeparatorItem) {
                        framePrimitiveData->frameType = HbFrameDrawer::NinePieces;
                        framePrimitiveData->frameGraphicsName = QLatin1String("qtg_fr_list_separator");
                    } else {
                        framePrimitiveData->frameType = HbFrameDrawer::NinePieces;
                        framePrimitiveData->frameGraphicsName = insidePopup ?
                            QLatin1String("qtg_fr_popup_list_normal") : QLatin1String("qtg_fr_list_normal");
                    }
                } else if (viewItemType == Hb::ItemType_GridViewItem
                            || viewItemType == HbPrivate::ItemType_ColorGridViewItem) {
                    framePrimitiveData->frameType = HbFrameDrawer::NinePieces;
                    framePrimitiveData->frameGraphicsName = insidePopup ?
                        QLatin1String("qtg_fr_popup_grid_normal") : QLatin1String("qtg_fr_grid_normal");
                } else{
                    framePrimitiveData->frameGraphicsName = QString();
                }
            }
        } else if (primitive == d->mFocusItem) {
            framePrimitiveData->geometry = boundingRect();

            int viewItemType = type();
            bool insidePopup = testAttribute(Hb::InsidePopup);
            if (viewItemType == Hb::ItemType_TreeViewItem
                || viewItemType == Hb::ItemType_ListViewItem
                || viewItemType == Hb::ItemType_RadioButtonListViewItem) {
                framePrimitiveData->frameType = HbFrameDrawer::NinePieces;
                framePrimitiveData->frameGraphicsName = insidePopup ?
                    QLatin1String("qtg_fr_popup_list_pressed") : QLatin1String("qtg_fr_list_pressed");
            } else if (     viewItemType == Hb::ItemType_GridViewItem
                        ||  viewItemType == HbPrivate::ItemType_ColorGridViewItem) {
                framePrimitiveData->frameType = HbFrameDrawer::NinePieces;
                framePrimitiveData->frameGraphicsName = insidePopup ?
                    QLatin1String("qtg_fr_popup_grid_pressed") : QLatin1String("qtg_fr_grid_pressed");
            } else{
                framePrimitiveData->frameGraphicsName = QString();
            }
        }

    } // else if (primitiveData->type == HbStylePrimitiveData::SPD_TouchArea) {
        // mMultiSelectionTouchArea: no data provided
}



/*!
    \reimp

    To optimize loading css/xml definitions to take place only once, this function should be
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

    if (d->mBackgroundItem) {
        // HbIconItem
        HbStyleIconPrimitiveData primitiveData;
        initPrimitiveData(&primitiveData, d->mBackgroundItem);
        style()->updatePrimitive(d->mBackgroundItem, &primitiveData, this);
    }

    if (d->mFrame) {
        // HbFrameItem
        HbStyleFramePrimitiveData primitiveData;
        initPrimitiveData(&primitiveData, d->mFrame);
        style()->updatePrimitive(d->mFrame, &primitiveData, this);
    }

    if (d->mSelectionItem) {
        // HbIconItem
        HbStyleIconPrimitiveData primitiveData;
        initPrimitiveData(&primitiveData, d->mSelectionItem);
        style()->updatePrimitive(d->mSelectionItem, &primitiveData, this);
    }

    if (d->mMultiSelectionTouchArea) {
        // HbTouchArea
        HbStyleTouchAreaPrimitiveData primitiveData;
        initPrimitiveData(&primitiveData, d->mMultiSelectionTouchArea);
        style()->updatePrimitive(d->mMultiSelectionTouchArea, &primitiveData, this);
    }

    if (d->mFocusItem) {
        // HbFrameItem
        HbStyleFramePrimitiveData primitiveData;
        initPrimitiveData(&primitiveData, d->mFocusItem);
        style()->updatePrimitive(d->mFocusItem, &primitiveData, this);
    }
        
    updatePixmapCache();
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
         if this is HbFrameBackground, otherwise it is either created from sd->mDefaultFrame, 
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
                d->mBackgroundItem = style()->createPrimitive(HbStyle::PT_IconItem, QLatin1String("background"), 0);
                d->mBackgroundItem->setParentItem(this); // To enable asynchronous icon loading.
                d->mBackgroundItem->setZValue(-3.0);
                HbIconItem *iconItem = qobject_cast<HbIconItem*>(d->mBackgroundItem);
                if (iconItem) {
                    iconItem->setAspectRatioMode(Qt::IgnoreAspectRatio);
                }
                delete d->mFrame;
                d->mFrame = 0;
            }
        } else if (currentBackground.canConvert<HbFrameBackground>()) {
            if (!d->mFrame) {
                d->mItemsChanged = true;
                d->mFrame = style()->createPrimitive(HbStyle::PT_FrameItem, QLatin1String("frame"), this);
                d->mFrame->setZValue(-4.0);

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
                d->mFrame = style()->createPrimitive(HbStyle::PT_FrameItem, QLatin1String("frame"), this);
                d->mFrame->setZValue(-4.0);
            }
        } else if (d->mFrame) {
            d->mItemsChanged = true;
            delete d->mFrame;
            d->mFrame = 0;
        }
    } 

    GraphicsItemFlags itemFlags = flags();
    Qt::ItemFlags indexFlags = d->mIndex.flags();

    if ((indexFlags & Qt::ItemIsEnabled) && sd->mItemView && sd->mItemView->isEnabled()) {
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
                d->mSelectionItem = style()->createPrimitive(HbStyle::PT_IconItem, QLatin1String("selection-icon"), 0);
                d->mSelectionItem ->setParentItem(this); // To enable asynchronous icon loading.
                HbIconItem *iconItem = qobject_cast<HbIconItem*>(d->mSelectionItem);
                if (iconItem) {
                    iconItem->setAlignment(Qt::AlignCenter);
                }
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
        &&  selectionMode == HbAbstractItemView::MultiSelection) {
        if (!d->mMultiSelectionTouchArea) {
            d->mItemsChanged = true;
            d->mMultiSelectionTouchArea = style()->createPrimitive(HbStyle::PT_TouchArea, QLatin1String("multiselection-toucharea"), this);
        }
    } else if (d->mMultiSelectionTouchArea) {
        d->mItemsChanged = true;
        delete d->mMultiSelectionTouchArea;
        d->mMultiSelectionTouchArea = 0;
    }

    // items visibility or items content has really changed
    updatePrimitives();
    if (!d->mContentChangedSupported
        || d->mItemsChanged) {
        updateGeometry();   // ensures that sizehint is calculated again in case items have been created or deleted
        
        if (d->viewAnimating()) {
            // Force QEvent::Polish & QEvent::LayoutRequest event handling to be synchronous (handled when sizeHint() is called) 
            // when view is animating (=scrollling) because of better performance.
            d->mHandlingRepolishSynchronously = true;
        }
        repolish();

        d->mHandlingRepolishSynchronously = false;
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
            d->mFocusItem = style()->createPrimitive(HbStyle::PT_FrameItem, QLatin1String("focus") , this);
            d->mFocusItem->setZValue(-1.0);
            // setChildFlags gets called in the construction phase, but the values are wrong.
            // Here we set the flags to force focusItem to be painted even when itemPixmapCache is on
            d->setChildFlags(d->mFocusItem, false);
        }

        if (d->mFocusItem) {
            HbStyleFramePrimitiveData primitiveData;
            initPrimitiveData(&primitiveData, d->mFocusItem);
            style()->updatePrimitive(d->mFocusItem, &primitiveData, this);
        }

        if (doAnimate) {
            HbEffect::cancel(this, "released");
            HbEffect::cancel(d->mFocusItem, "released");

            d->mNonCachableItem = d->mFocusItem;
            updatePixmapCache();

            HbEffect::start(this, sd->mItemType, "pressed");
            HbEffect::start(d->mFocusItem, sd->mItemType + QString("-focus"), "pressed");
        }
    } else {
        if (doAnimate) {
            HbEffect::cancel(this, "pressed");
            HbEffect::cancel(d->mFocusItem, "pressed");

            d->mNonCachableItem = d->mFocusItem;
            updatePixmapCache();

            HbEffect::start(this, sd->mItemType, "released");
            HbEffect::start(d->mFocusItem, sd->mItemType + QString("-focus"), "released", this, "_q_animationFinished");
        } else {
            HbEffect::cancel(this, "pressed");
            HbEffect::cancel(this, "released");
            if (d->mFocusItem) {
                HbEffect::cancel(d->mFocusItem, "pressed");
                HbEffect::cancel(d->mFocusItem, "released");
                QCoreApplication::postEvent(this, new QEvent((QEvent::Type)HbAbstractViewItemShared::ViewItemDeferredDeleteEvent));
            }
        }
    }
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

    if (sd->mItemView) {
        setProperty("layoutName", sd->mItemView->layoutName());
    }

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

/*!
    \reimp
*/
QSizeF HbAbstractViewItem::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_D(const HbAbstractViewItem);
    if (d->repolishOutstanding) {
        // force the polish event in order to get the real size
        // updateGeometry() in ::updateChildItems() causes this function to be called
        // before QEvent::Polish of repolish() is handled from the event loop.
        QCoreApplication::sendPostedEvents(const_cast<HbAbstractViewItem*>(this), QEvent::Polish);
        QCoreApplication::sendPostedEvents(const_cast<HbAbstractViewItem *>(this), QEvent::LayoutRequest);
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

/*!
    \reimp
*/
void HbAbstractViewItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    Q_D(HbAbstractViewItem);
    
    if (!d->mFrontPixmapPainter) {
        return;
    }
    
    if (!d->mInPaintItems) {
        bool usePixmapCache = d->usePixmapCache();

        if (usePixmapCache) {
            QRect rect = boundingRect().toAlignedRect();
            if (    d->mFrontPixmapPainter->pixmap()
                &&  rect.size() != d->mFrontPixmapPainter->pixmap()->size()) {
                d->mResetPixmapCache = true;
                d->releasePixmaps();
            }

            if (!d->mResetPixmapCache && d->mUpdateItems.count() && d->mFrontPixmapPainter->pixmap()) {
                 d->drawSubPixmap(d->mFrontPixmapPainter->pixmap(), painter, option);
            }

            if (d->mResetPixmapCache) {
                if (!d->mFrontPixmapPainter->pixmap()) {
                    d->setChildFlagRecursively(true);
                    QPixmap *pixmap = new QPixmap(rect.size());
                    d->mFrontPixmapPainter->setPixmap(pixmap);
                }
                
                d->mUpdateItems.clear();

                d->updatePixmap(d->mFrontPixmapPainter->pixmap(), painter, option, d->mNonCachableItem, 0);

                if (d->mNonCachableItem) {
                    if (!d->mBackPixmapPainter->pixmap()) {
                        QPixmap *pixmap = new QPixmap(rect.size());
                        d->mBackPixmapPainter->setPixmap(pixmap);
                    } 
                    d->updatePixmap(d->mBackPixmapPainter->pixmap(), painter, option, 0, d->mNonCachableItem);
                } else if (d->mBackPixmapPainter) {
                    d->mBackPixmapPainter->setPixmap(0);
                }

                d->mResetPixmapCache = false;
            }
        } else {
            if (d->mFrontPixmapPainter->pixmap()) {
                d->setChildFlagRecursively(false);
                d->releasePixmaps();
            }
        }
    }
}

/*!
    Updates the pixmap cache.

    Call this function when cache pixmap requires updating due to item or child state change
    that affects its visual appearance.
*/
void HbAbstractViewItem::updatePixmapCache()
{
    Q_D(HbAbstractViewItem);

    if (!d->mResetPixmapCache && d->usePixmapCache()) {
        d->mResetPixmapCache = true;
        update();
    }
}

#include "moc_hbabstractviewitem.cpp"

