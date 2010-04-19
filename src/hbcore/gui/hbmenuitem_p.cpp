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

#include "hbmenuitem_p.h"
#include "hbmenuitem_p_p.h"
#include "hbmenu.h"
#include "hbaction.h"
#include "hbstyle.h"
#include "hbstyleoptionmenuitem.h"
#include "hbframeitem.h"
#include "hbtextitem.h"
#include "hbevent.h"
#include "hbcolorscheme.h"

Q_DECLARE_METATYPE (QAction*)

/*
    \class HbMenuItem
    \brief HbMenuItem is a menu item graphics widget.

    HbMenuItem represents a single action in HbMenu.

    Usually touch and non-touch devices have mostly the same actions but a
    slightly different menu structure. High-end touch devices might prefer
    relatively large one level menus with direct access to all available
    actions, whereas low-end non-touch devices might fall back to using
    small but deep menus, offering the same actions but at different level.

    \sa HbAction HbMenu
*/

/*!
    \property HbMenuItem::menuType
    \brief
*/

        HbMenuItemPrivate::HbMenuItemPrivate() :
        HbWidgetPrivate(),
        action(0),
        menu(0),
        textItem(0),
        arrowItem(0),
        checkItem(0),
        separatorItem(0),
        mFocusItem(0),
        mFrame(0),
        mChecked(false),
        mRecycled(false)
{
}

HbMenuItemPrivate::~HbMenuItemPrivate()
{
}

bool HbMenuItemPrivate::createPrimitives(HbStyleOptionMenuItem const &option)
{
    Q_Q(HbMenuItem);
    bool repolishNeeded(false);

    HbAction *hbAction = qobject_cast<HbAction* >(action);

    if (!mFrame && action && !action->isSeparator()) {
        mFrame = q->style()->createPrimitive(HbStyle::P_MenuItem_frame, q);
        q->style()->updatePrimitive(mFrame, HbStyle::P_MenuItem_frame, &option);
        mFrame->setAcceptedMouseButtons(Qt::NoButton);
        mFrame->setZValue(-4.0);
        HbStyle::setItemName(mFrame, "frame");
        repolishNeeded = true;
    }

    if (action) {
        if (!textItem) {
            textItem = q->style()->createPrimitive(HbStyle::P_MenuItem_text, q);
            textItem->setAcceptedMouseButtons(Qt::NoButton);
            repolishNeeded = true;
        }
    } else if (textItem){
        delete textItem;
        textItem = 0;
        repolishNeeded = true;
    }

    if (hbAction && hbAction->menu() && !action->isSeparator()) {
        if (!arrowItem) {
            arrowItem = q->style()->createPrimitive(HbStyle::P_MenuItem_submenuindicator, q);
            q->style()->updatePrimitive(arrowItem, HbStyle::P_MenuItem_submenuindicator, &option);
            arrowItem->setAcceptedMouseButtons(Qt::NoButton);
            repolishNeeded = true;
        }
    } else if (arrowItem){
        delete arrowItem;
        arrowItem = 0;
        repolishNeeded = true;
    }

    if (action && action->isCheckable() && !action->isSeparator()) {
        if (!checkItem) {
            checkItem = q->style()->createPrimitive(HbStyle::P_MenuItem_checkindicator, q);
            checkItem->setAcceptedMouseButtons(Qt::NoButton);
            repolishNeeded = true;
        }
    } else if (checkItem) {
        delete checkItem;
        checkItem = 0;
        repolishNeeded = true;
    }

    if (action && action->isSeparator()) {
        if (!separatorItem) {
            separatorItem = q->style()->createPrimitive(HbStyle::P_MenuItem_separator, q);
            separatorItem->setAcceptedMouseButtons(Qt::NoButton);
            q->style()->updatePrimitive(separatorItem, HbStyle::P_MenuItem_separator, &option);
            repolishNeeded = true;
        }
    } else if (separatorItem){
        delete separatorItem;
        separatorItem = 0;
        repolishNeeded = true;
    }
    return repolishNeeded;
}

void HbMenuItemPrivate::_q_updateItem(bool forcedUpdate)
{
    Q_Q(HbMenuItem);
    if (!action)
        return;
    HbStyleOptionMenuItem option;
    q->initStyleOption(&option);
    bool isRepolishNeeded = createPrimitives(option);
    HbTextItem *item = qgraphicsitem_cast<HbTextItem *>(textItem);
    if (item && item->text() != action->text()) {
        item->setText(action->text());
    }
    if (checkItem && (mChecked != option.checked || forcedUpdate)) {
        q->style()->updatePrimitive(checkItem, HbStyle::P_MenuItem_checkindicator, &option);
        mChecked = option.checked;
    }
    if (action->isEnabled() != q->isEnabled() || forcedUpdate) {
        q->setEnabled(action->isEnabled());
    }

    if ((isRepolishNeeded && polished) || forcedUpdate) {
        q->repolish();
    }
}

/*
    Constructs a new HbMenuItem with \a action and \a parent. Ownership of the
    \a action remains on it's parent.
*/
HbMenuItem::HbMenuItem(HbMenu *menu, QGraphicsItem *parent)
    : HbWidget(*new HbMenuItemPrivate(), parent)
{

    Q_D(HbMenuItem);

    d->q_ptr = this;
    d->menu = menu;
    setAcceptedMouseButtons(Qt::NoButton);
}

/*
    Destructs the menu item.
*/
HbMenuItem::~HbMenuItem()
{
}

/*
    Returns the action representing this menu item.
*/
QAction* HbMenuItem::action() const
{
    Q_D(const HbMenuItem);
    return d->action;
}

/*
    Returns the menu which handles this item.
*/
HbMenu* HbMenuItem::menu() const
{
    Q_D(const HbMenuItem);
    return d->menu;
}

/*
    \reimp
 */
void HbMenuItem::initStyleOption(HbStyleOptionMenuItem *option) const
{
    Q_D(const HbMenuItem);
    HbWidget::initStyleOption(option);

    Q_ASSERT(option);
    if (d->action) {
        option->text = d->action->text();
        option->separator = d->action->isSeparator();
        option->checkable = d->action->isCheckable();
        option->checked = d->action->isChecked();
        option->arrow = false;
        HbAction *hbAction = qobject_cast<HbAction *>(d->action);
        if (hbAction) {
            option->arrow = hbAction->menu();
        }
    }
}


/*
    \reimp
 */
void HbMenuItem::changeEvent(QEvent *event)
{
    HbWidget::changeEvent(event);
    if (event->type() == QEvent::LayoutDirectionChange) {
        Q_D(HbMenuItem);
        if (d->polished)
            repolish();
    }
    else if (event->type() == HbEvent::ThemeChanged) {
        Q_D(HbMenuItem);
        d->_q_updateItem();
    }
}

/*
    Sets the action,which is represented by the menu item.
*/
void HbMenuItem::setAction(QAction *action)
{
    Q_D(HbMenuItem);
    bool actionChanged = d->action != action;
    if (d->action && actionChanged){
        disconnect(d->action, SIGNAL(changed()), this, SLOT(_q_updateItem()));
    }
    if (actionChanged) {
        d->action = action;
        d->_q_updateItem();
        connect(d->action, SIGNAL(changed()), this, SLOT(_q_updateItem()));
    }
    else if (!actionChanged && d->mRecycled){
        d->_q_updateItem(true);
        connect(d->action, SIGNAL(changed()), this, SLOT(_q_updateItem()));
        d->mRecycled = false;
    }

}

/*
    This is for convienience.This functionality can be internal to menu item
    also and cal be done by following changed() signal emitted from action.But this gives more precise
    control for menu container.This is called when action has been made invisible and
    container recycles the item.
*/
void HbMenuItem::recycleItem()
{
    Q_D(HbMenuItem);
    d->mRecycled = true;
    if (d->action){
        disconnect(d->action, SIGNAL(changed()), this, SLOT(_q_updateItem()));
    }
}

bool HbMenuItem::checkboxExists()
{
    Q_D(HbMenuItem);
    return (d->checkItem != 0);
}

bool HbMenuItem::submenuExists()
{
    Q_D(HbMenuItem);
    return (d->arrowItem != 0);
}

bool HbMenuItem::separatorExists()
{
    Q_D(HbMenuItem);
    return (d->separatorItem != 0);
}

/*
    Returns the type of the menu where menu item belongs.
*/
HbMenu::MenuType HbMenuItem::menuType() const
{
    Q_D(const HbMenuItem);
    if (d->menu) {
        return menu()->menuType();
    } else {
        return HbMenu::ContextMenu;
    }
}

/*!
    \creates and updates the focusitem primitive
     when menu item has been pressed
*/
void HbMenuItem::pressStateChanged(bool value)
{
    Q_D(HbMenuItem);

    if (value) {
        if (!d->mFocusItem) {
            d->mFocusItem = style()->createPrimitive(HbStyle::P_MenuItem_focus, this);
        }
        HbStyleOptionMenuItem option;
        initStyleOption(&option);
        style()->updatePrimitive(d->mFocusItem, HbStyle::P_MenuItem_focus,&option);
        HbStyle::setItemName(d->mFocusItem,"focus-indicator");
        d->mFocusItem->setZValue(-1.0);
        HbFrameItem *focusItem = qgraphicsitem_cast<HbFrameItem*>(d->mFocusItem);
        if(focusItem){
            focusItem->setGeometry(boundingRect());
        }
    }  else if (d->mFocusItem){
        delete d->mFocusItem;
        d->mFocusItem = 0;
    }
}

#include "moc_hbmenuitem_p.cpp"
