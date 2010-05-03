/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbCore module of the UI Extensions for Mobile.
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

#include "hbmenucontainer_p.h"
#include "hbmenu.h"
#include "hbmenu_p.h"
#include "hbmenuitem_p.h"
#include "hbaction.h"
#include "hbinstance.h"
#include <hbwidgetfeedback.h>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLayout>
#include <QDebug>

HbMenuContainerPrivate::HbMenuContainerPrivate(HbMenu *menu) :
        menu(menu), actionManager(0)
{
}
HbMenuContainerPrivate::~HbMenuContainerPrivate()
{
}

void HbMenuContainerPrivate::init()
{
    Q_Q(HbMenuContainer);
    mLayout = new QGraphicsLinearLayout(Qt::Vertical, q);
    mLayout->setContentsMargins(0.0, 0.0, 0.0, 0.0);
    mLayout->setSpacing(0.0);

    mLayout->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    q->setLayout(mLayout);
}

HbMenuContainer::HbMenuContainer(HbMenu *menu,QGraphicsItem *parent):
        HbWidget(*new HbMenuContainerPrivate(menu),parent)
{
    Q_D(HbMenuContainer);
    d->q_ptr = this;
    d->init();
}

QList<HbMenuItem *> HbMenuContainer::items() const
{
    Q_D(const HbMenuContainer);
    return d->mItems;
}

void HbMenuContainer::visibleItemsChanged()
{
    Q_D(HbMenuContainer);

    bool newItemFound(false);
    QList<HbMenuItem*>  visibleItems;
    QRectF menuRect = d->menu->boundingRect();
    foreach (HbMenuItem * item, d->mItems) {
        QRectF itemRect = item->boundingRect();
        itemRect.moveTo(item->pos() + pos());
        if (menuRect.contains(itemRect)) {
            visibleItems.append(item);
            if(!d->mOldVisibleItems.empty()){
                if (!d->mOldVisibleItems.contains(item)) {
                    newItemFound = true;
                }
            }
        }
    }
    if (newItemFound){
        HbWidgetFeedback::triggered(d->menu, Hb::InstantDraggedOver);
    }

    d->mOldVisibleItems.clear();
    d->mOldVisibleItems = visibleItems;
}

//Deletes MenuItem with corresponding action
void HbMenuContainer::removeActionItem(QAction *action)
{
    Q_D(HbMenuContainer);
    for (int i = 0; d->mItems.count(); i++) {
        HbMenuItem *item = d->mItems.at(i);
        if (item->action() == action) {
            d->mItems.removeAt(i);
            d->mLayout->removeItem(item);
            delete item;
            break;
        }
    }
}

void HbMenuContainer::delayedLayout()
{
    Q_D(HbMenuContainer);
    foreach (QAction *action, d->menu->actions()) {
        if (action->isVisible()) {
            d->mVisibleActions.append(action);
            QObject::connect(action, SIGNAL(triggered()), d->menu, SLOT(_q_onActionTriggered()));
            addItem(action);
        }
    }
}

//creates new menu items if needed. This is called when actionadded event is received by hbmenu.
void HbMenuContainer::addItem(QAction *action, HbMenuItem *item)
{
    Q_D(HbMenuContainer);
    if (!item) {
        item  = new HbMenuItem(d->menu, this);
        item->setAction(action);
    }
    int pos = 0;           
    HbAction *castedAction = qobject_cast<HbAction *>(action);
    if(!d->actionManager && castedAction && castedAction->commandRole() != HbAction::NoRole) {
        d->actionManager = new HbActionManager(HbView::OptionsMenu, d->menu, d->menu->mainWindow() );
    }
    if (d->actionManager && castedAction) {
        pos = d->actionManager->position(castedAction, d->menu->actions());
    } else {
        pos = d->mVisibleActions.indexOf(action);
    }
    if (castedAction && castedAction->menu()) {
        HbMenuPrivate::d_ptr(castedAction->menu())->setSubMenuItem(item);
    }
    /* Workaround for layout flushing problem */
    if (item->action()->isSeparator())
        item->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    /* Workaround ends */
    d->mItems.insert(pos, item);
    d->mLayout->insertItem(pos, item);    
}

//this is called when an existing items visibility has changed.
//Usually when actionchanged event is received by hbmenu
void HbMenuContainer::updateActionItem(QAction *action)
{
    Q_D(HbMenuContainer);
    //if existing action has become invisible remove it from list
    if (!action->isVisible()){
        for (int i = 0; i < d->mItems.count(); i++) {
            HbMenuItem *item = d->mItems.at(i);
            if (item->action() == action) {
                d->mItems.removeAt(i);                
                item->recycleItem();
                item->setVisible(false);
                d->mLayout->removeItem(item);
                d->mBufferItems.insert(action, item);
            }
        }
    } else {
        //check if item was already layouted
        for (int i = 0; i < d->mItems.count(); i++) {
            if (d->mItems.at(i)->action() == action)
                return;
        }
        HbMenuItem *item = d->mBufferItems.take(action);               
        if (item) {
            item->setAction(action);
            item->setVisible(true);
        }
        addItem(action, item);
    }
}

void HbMenuContainer::updateVisibleActionList()
{
    Q_D(HbMenuContainer);
    d->mVisibleActions.clear();
    foreach (QAction *action, d->menu->actions()) {
        if (action->isVisible()) {
            d->mVisibleActions.append(action);
        }
    }
}

void HbMenuContainer::polish(HbStyleParameters &params)
{
    Q_UNUSED(params);
    Q_D(HbMenuContainer);    
    d->polished = true;
}

HbMenuListViewPrivate::HbMenuListViewPrivate() :
        HbScrollAreaPrivate(),
        mHitItem(0),
        mCurrentItem(0),
        mCurrentIndex(-1),
        mWasScrolling(false)
{
}

HbMenuListView::HbMenuListView(HbMenu *menu,QGraphicsItem *parent)
               :HbScrollArea(* new HbMenuListViewPrivate,parent)
{
    Q_D(HbMenuListView);
    d->q_ptr = this;
    d->mClampingStyle = HbScrollArea::StrictClamping;
    d->mScrollDirections = Qt::Vertical;
    d->mFrictionEnabled = false;
    d->mContainer = new HbMenuContainer(menu, this);
    setContentWidget(d->mContainer);
    setScrollingStyle(HbScrollArea::Pan);
}

bool HbMenuListView::scrollByAmount(const QPointF& delta)
{
    Q_D( HbMenuListView );
    bool ret = HbScrollArea::scrollByAmount(delta);
    if(ret)
        d->mContainer->visibleItemsChanged();
    return ret;
}

void HbMenuListView::addActionItem(QAction *action)
{
    Q_D(HbMenuListView);
    if (action->isSeparator())
        return;
    d->mContainer->updateVisibleActionList();
    d->mContainer->addItem(action);
    d->mCurrentIndex = -1;
    updateGeometry();
}

void HbMenuListView::removeActionItem(QAction *action)
{
    Q_D(HbMenuListView);
    d->mContainer->updateVisibleActionList();
    d->mContainer->removeActionItem(action);
    d->mCurrentIndex = -1;
    updateGeometry();
}

void HbMenuListView::updateActionItem(QAction *action)
{
    Q_D(HbMenuListView);
    d->mContainer->updateVisibleActionList();
    d->mContainer->updateActionItem(action);
    d->mCurrentIndex = -1;
    updateGeometry();
}

HbMenuItem * HbMenuListView::currentItem() const
{
    Q_D(const HbMenuListView);
    return d->mCurrentItem;
}

void HbMenuListView::setCurrentItem(HbAction *action)
{
    Q_D(HbMenuListView);
    for (int i = 0; i < d->mContainer->items().count(); i++) {
        HbMenuItem *item = d->mContainer->items().at(i);
        if (item->action() == action) {
            ensureVisible(item->pos());
            d->mCurrentIndex = i;
            d->mCurrentItem = d->mContainer->items().at(d->mCurrentIndex);
            break;
        }
    }
}

/*!
    \reimp
*/
void HbMenuListView::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
#ifdef HB_GESTURE_FW
    Q_UNUSED(event);
    HbScrollArea::mousePressEvent(event);
    event->accept();
#else
    Q_D(HbMenuListView);
    d->mHitItem = itemAt(event->scenePos());
    if (d->mHitItem){
        Hb::InteractionModifiers modifiers = 0;
        if (d->mIsScrolling) {
            modifiers |= Hb::ModifierScrolling;
            d->mWasScrolling = true;
        }
        HbWidgetFeedback::triggered(d->mHitItem, Hb::InstantPressed, modifiers);
        if (!d->mWasScrolling){
            ensureVisible(d->mHitItem->pos());
            if(!isFocusable(d->mHitItem->action()))
                d->mHitItem = 0;
            else
                d->mHitItem->pressStateChanged(true);
        }
        else
            d->mHitItem = 0;
    }
    HbScrollArea::mousePressEvent(event);
    event->accept();
#endif
}

/*!
    \reimp
*/
void HbMenuListView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
#ifdef HB_GESTURE_FW
    HbScrollArea::mouseReleaseEvent(event);    
#else
    Q_D(HbMenuListView);

    HbScrollArea::mouseReleaseEvent(event);

    HbMenuItem* hitItem = itemAt(event->scenePos());
    if (hitItem){
        Hb::InteractionModifiers modifiers = 0;
        if (d->mWasScrolling) {
            modifiers |= Hb::ModifierScrolling;
            d->mWasScrolling = false;
        }
        HbWidgetFeedback::triggered(hitItem, Hb::InstantReleased, modifiers);
    }
    if (d->mHitItem){
        d->mHitItem->pressStateChanged(false);
        if (d->mHitItem == hitItem) {
            d->mCurrentItem = d->mHitItem;
            d->mCurrentIndex = d->mContainer->items().indexOf(d->mCurrentItem);
            HbMenuPrivate::d_ptr(d->mCurrentItem->menu())->_q_triggerAction(d->mCurrentItem);

        }
        d->mHitItem = 0;
    }
    event->accept();
#endif
}

HbMenuItem *HbMenuListView::itemAt(const QPointF& position) const
{
    HbMenuItem *hitItem = 0;
    const QList<QGraphicsItem *> items = scene()->items(position);

    const int count(items.count());
    for (int current = 0; current < count; ++current) {
        QGraphicsItem *item = items.at(current);
        if (item && item->isWidget()) {
            QGraphicsWidget *widget = static_cast<QGraphicsWidget *>(item);

            hitItem = qobject_cast<HbMenuItem *>(widget);
            // Workaround for QT bug which is not returning all the scene items properly
            if (!hitItem) {
                QGraphicsItem *parent = widget->parentItem();
                while (parent && parent->isWidget()) {
                    hitItem = qobject_cast<HbMenuItem *>(static_cast<QGraphicsWidget*>(parent));
                    if (hitItem)
                        break;
                    parent = parent->parentItem();
                }
            }
            //workaround ends
        }
        if (hitItem)
            break;
    }    
    return hitItem;
}

/*!
    \reimp
*/
void HbMenuListView::upGesture(int value)
{
    Q_D(HbMenuListView);

    if (d->mHitItem) {
        d->mHitItem->pressStateChanged(false);
        d->mHitItem = 0;
    }

    HbScrollArea::upGesture(value);
}

/*!
    \reimp
*/
void HbMenuListView::downGesture(int value)
{
    Q_D(HbMenuListView);

    if (d->mHitItem) {
        d->mHitItem->pressStateChanged(false);
        d->mHitItem = 0;
    }

    HbScrollArea::downGesture(value);
}

/*!
    \reimp
*/
void HbMenuListView::panGesture(const QPointF &point)
{
    Q_D(HbMenuListView);

    if (d->mHitItem) {
        d->mHitItem->pressStateChanged(false);
        d->mHitItem = 0;
    }
    HbScrollArea::panGesture(point);
}

bool HbMenuListView::isFocusable(QAction *action)// krazy:exclude=qclasses
{
    return action && action->isVisible() && !action->isSeparator() && action->isEnabled();
}

void HbMenuListView::doDelayedLayout()
{
        Q_D(HbMenuListView);
        d->mContainer->delayedLayout();
}

/*!
    \reimp
 */
QVariant HbMenuListView::itemChange(GraphicsItemChange change, const QVariant &value)
{
    Q_D(HbMenuListView);

    // Remove highlighting on a menuitem when the menu disappears.
    if (change == QGraphicsItem::ItemVisibleHasChanged && !value.toBool()) {
        if (d->mHitItem) {
            d->mHitItem->pressStateChanged(false);
        }
    }

    return HbScrollArea::itemChange(change, value);
}

void HbMenuListView::gestureEvent(QGestureEvent *event)
{
    Q_D(HbMenuListView);
    HbScrollArea::gestureEvent(event);
    //WORKAROUND for bug scene doesn't return all the items
    if(QTapGesture *gesture = static_cast<QTapGesture *>(event->gesture(Qt::TapGesture))) {
        // Stop scrolling on tap
        if (gesture->state() == Qt::GestureStarted) {
            event->accept();
            d->mHitItem = itemAt(gesture->position());
            if (d->mHitItem){
                Hb::InteractionModifiers modifiers = 0;
                if (d->mIsScrolling) {
                    modifiers |= Hb::ModifierScrolling;
                    d->mWasScrolling = true;
                }
                HbWidgetFeedback::triggered(d->mHitItem, Hb::InstantPressed, modifiers);
                if (!d->mWasScrolling){
                    ensureVisible(d->mHitItem->pos());
                    if(!isFocusable(d->mHitItem->action()))
                        d->mHitItem = 0;
                    else
                        d->mHitItem->pressStateChanged(true);
                }
                else
                    d->mHitItem = 0;
            }
        } else if (gesture->state() == Qt::GestureFinished) {

            HbMenuItem* hitItem = itemAt(gesture->position());
            if (hitItem){
                Hb::InteractionModifiers modifiers = 0;
                if (d->mWasScrolling) {
                    modifiers |= Hb::ModifierScrolling;
                    d->mWasScrolling = false;
                }
                HbWidgetFeedback::triggered(hitItem, Hb::InstantReleased, modifiers);
            }
            if (d->mHitItem){
                d->mHitItem->pressStateChanged(false);
                if (d->mHitItem == hitItem) {
                    d->mCurrentItem = d->mHitItem;
                    d->mCurrentIndex = d->mContainer->items().indexOf(d->mCurrentItem);
                    HbMenuPrivate::d_ptr(d->mCurrentItem->menu())->_q_triggerAction(d->mCurrentItem);
                }
                d->mHitItem = 0;
            }
        } else if (gesture->state() == Qt::GestureCanceled) {
            if (d->mHitItem) {
                d->mHitItem->pressStateChanged(false);
                d->mHitItem = 0;
            }
        }
    }
    //WORKAROUND
    
    if (QPanGesture *panGesture = qobject_cast<QPanGesture*>(event->gesture(Qt::PanGesture))) {
        event->accept(panGesture);
    }
}
