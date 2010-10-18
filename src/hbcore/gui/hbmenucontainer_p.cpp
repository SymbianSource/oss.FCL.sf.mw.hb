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
#include <QGraphicsGridLayout>
#include <hbscrollbar_p.h>

HbMenuContainerPrivate::HbMenuContainerPrivate(HbMenu *menu) :
        menu(menu), actionManager(0), mDelayLayout(false)
{
}
HbMenuContainerPrivate::~HbMenuContainerPrivate()
{
}

void HbMenuContainerPrivate::init()
{
    Q_Q(HbMenuContainer);
    mLayout = new QGraphicsGridLayout(q);
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
    for (int i = 0; i < d->mItems.count(); i++) {
        HbMenuItem *item = d->mItems.at(i);
        if (item->action() == action) {
            d->mItems.removeAt(i);
            reLayout();
            delete item;
            break;
        }
    }
}

void HbMenuContainer::delayedLayout()
{
    Q_D(HbMenuContainer);
    d->mDelayLayout = true;
    foreach (QAction *action, d->menu->actions()) {
        QObject::connect(action, SIGNAL(triggered()), d->menu, SLOT(_q_onActionTriggered()));
        if (action->isVisible()) {
            d->mVisibleActions.append(action);            
            addItem(action);
        }
    }
    d->mDelayLayout = false;
    reLayout();
}

/*!
 Reforms the menu layout when needed, e.g. after an orientation change.
*/
void HbMenuContainer::reLayout()
{
    Q_D(HbMenuContainer);

    int layoutItems = d->mLayout->count();
    for ( int i = 0; i < layoutItems; i++ ) {
        d->mLayout->removeAt(0);
    }

    int pos = 0;
    int numCols = (HbMenuPrivate::d_ptr(d->menu))->mNumberOfColumns;

    for ( int i = 0; i < d->mItems.count(); i++ ) {
        int row = pos / numCols;
        int col = pos % numCols;

        if (!d->mItems.at(i)->action()->isSeparator()) {
            d->mLayout->addItem(d->mItems.at(i), row, col, Qt::AlignCenter);
            pos++;
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
        pos = d->actionManager->position(castedAction, d->mVisibleActions);
    } else {
        pos = d->mVisibleActions.indexOf(action);
    }
    if (castedAction && castedAction->menu()) {
        HbMenuPrivate::d_ptr(castedAction->menu())->setSubMenuItem(item);
    }
    d->mItems.insert(pos, item);
    if (!d->mDelayLayout)
        reLayout();
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
                reLayout();
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

/*!
    \reimp
 */
void HbMenuContainer::changeEvent(QEvent *event)
{
    switch (event->type()) {
        // suppress events (optimize)
    case QEvent::FontChange:
    case QEvent::PaletteChange:
    case QEvent::ParentChange:
    case QEvent::StyleChange: // flow through
        break;
    default:
        HbWidget::changeEvent(event);
        break;
    }
}

QVariant HbMenuContainer::itemChange(GraphicsItemChange change, const QVariant &value)
{
    Q_D(HbMenuContainer);
    if (change == QGraphicsItem::ItemVisibleChange) {
        if (value.toBool()) {
            if (d->polished && !testAttribute(Qt::WA_Resized)) {
                adjustSize();
                setAttribute(Qt::WA_Resized, false);
            }
            return QGraphicsItem::itemChange(change, value);
        }
    }
    return HbWidget::itemChange(change, value);
}

HbMenuListViewPrivate::HbMenuListViewPrivate() :
        HbScrollAreaPrivate(HbScrollAreaPrivate::VerticalScrollBar),
        mCurrentItem(0),
        mCurrentIndex(-1),
        mWasScrolling(false)
{
}

  void HbMenuListViewPrivate::updateScrollMetrics()
  {
      if (mContainer && mContainer->layout())
          mContainer->layout()->activate();
      HbScrollAreaPrivate::updateScrollMetrics();
  }

void HbMenuListViewPrivate::doLazyInit()
{
  //we dont use indicative scrollbars fro menu so no need to do the signal slot connections
    if (!mLazyInitDone) {
        if (mVerticalScrollBar) {
            if (mVerticalScrollBar->isVisible())
                HbScrollBarPrivate::d_ptr(mVerticalScrollBar)->loadEffects();
        }
        mLazyInitDone = true;
    }
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
    d->mContainer->setFlag(QGraphicsItem::ItemHasNoContents, true);
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

bool HbMenuListView::isFocusable(QAction *action)// krazy:exclude=qclasses
{
    return action && action->isVisible() && !action->isSeparator() && action->isEnabled();
}

void HbMenuListView::doDelayedLayout()
{
    Q_D(HbMenuListView);
    d->mContainer->delayedLayout();
    d->doLazyInit();
}

void HbMenuListView::updateContainer()
{
    Q_D(HbMenuListView);
    d->mContainer->reLayout();
    updateGeometry();
}

/*!
    \reimp
 */
QVariant HbMenuListView::itemChange(GraphicsItemChange change, const QVariant &value)
{
    Q_D(HbMenuListView);
    if (change == QGraphicsItem::ItemVisibleChange && value.toBool()) {
        if (d->polished && !testAttribute(Qt::WA_Resized)) {
            adjustSize();
            setAttribute(Qt::WA_Resized, false);
        }
        return QGraphicsItem::itemChange(change, value);
    }
    return HbScrollArea::itemChange(change, value);
}

/*!
    \reimp
 */
void HbMenuListView::changeEvent(QEvent *event)
{
    switch (event->type()) {
        // suppress events (optimize)
    case QEvent::FontChange:
    case QEvent::PaletteChange:
    case QEvent::ParentChange:
    case QEvent::StyleChange: // flow through
        break;
    default:
        HbWidget::changeEvent(event);
        break;
    }
}

void HbMenuListView::gestureEvent(QGestureEvent *event)
{
    HbScrollArea::gestureEvent(event);

    if (QPanGesture *panGesture = qobject_cast<QPanGesture*>(event->gesture(Qt::PanGesture))) {
        event->accept(panGesture);
    }
}



