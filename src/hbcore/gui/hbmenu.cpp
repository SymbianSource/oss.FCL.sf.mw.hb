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

#include "hbmenu.h"
#include "hbmenu_p.h"
#include "hbmenuitem_p.h"
#include "hbaction.h"
#include <hbwidgetfeedback.h>
#include "hbinstance.h"
#include "hbmenucontainer_p.h"
#ifdef HB_EFFECTS
#include "hbeffect.h"
#include "hbeffectinternal_p.h"
bool HbMenuPrivate::menuEffectsLoaded = false;
#endif
#include "hbglobal_p.h"

#include <QPointer>

Q_DECLARE_METATYPE (QAction*)// krazy:exclude=qclasses

HbMenuPrivate::HbMenuPrivate():
    HbPopupPrivate(),
    menuItemView(0),
    subMenuAction(0),
    activeSubMenu(0),
    resultAction(0),
    actionTriggered(false),
    menuType(HbMenu::ContextMenu),
    mSubMenuItem(0),
    mRightMargin(0.0),
    mDownMargin(0.0),
    delayMenuConstruction(true),
    receiverToDisconnectOnClose(0)
{
}

HbMenuPrivate::~HbMenuPrivate()
{
}

void HbMenuPrivate::init()
{
    Q_Q(HbMenu);

    subMenuAction = new HbAction(q);
    subMenuAction->setMenu(q);
    q->setTimeout(HbPopup::ContextMenuTimeout);

    q->setBackgroundFaded(false);
}

void HbMenuPrivate::addPopupEffects()
{
#ifdef HB_EFFECTS
    effectType = "HB_MENU";
    hasEffects = menuEffectsLoaded;
    if (menuEffectsLoaded)
        return;
    menuEffectsLoaded = true;
    hasEffects = HbEffectInternal::add("HB_MENU", "menu_appear", "appear");
    if (hasEffects) {
        //We load the disappear effect only if appear effect was also loaded
        hasEffects = HbEffectInternal::add("HB_MENU", "menu_disappear", "disappear");

        if (hasEffects) {
            hasEffects = HbEffectInternal::add("HB_menuitem", "menuitem_press", "clicked");
        }
        if (hasEffects ) {
            hasEffects = HbEffectInternal::add("HB_POPUP",
                                               "dialog_rotate",
                                               "orientationswitch");
        }
    }
#endif
}

void HbMenuPrivate::_q_triggerAction(HbMenuItem *currentItem)
{
    Q_Q(HbMenu);
    if (currentItem && currentItem->action()){
        HbWidgetFeedback::triggered(currentItem, Hb::InstantClicked);
#ifdef HB_EFFECTS
        if (hasEffects) {
            HbEffect::start(currentItem, "HB_menuitem", "clicked");
        }
#endif
        HbAction *hbAction = qobject_cast<HbAction *>(currentItem->action());
        if (hbAction && hbAction->menu() && !actionTriggered) {
            hbAction->trigger();
            stopTimeout();
            openSubmenu(currentItem);
        } else {
            q->close();
            
            resultAction = hbAction;

            if (!actionTriggered) { // prevent duplicate events
                currentItem->action()->trigger();
            }
            actionTriggered = true;
        }
    }
}

void HbMenuPrivate::createMenuView()
{
    Q_Q(HbMenu);
    if (!menuItemView && q->actions().count()){
        menuItemView = new HbMenuListView(q, q);
        HbStyle::setItemName(menuItemView, "content");
        //This optimises case of options menu which otherwise updates its primitives twice.
        if (menuType ==  HbMenu::OptionsMenu)
            q->setFrameType(HbPopup::Strong);
    	else
            q->setFrameType(HbPopup::Weak);
        if (polished)//This check can be removed once base class repolish is fixed.
            q->repolish();
    }	
}

void HbMenuPrivate::delayedLayout()
{
    createMenuView();
    if(menuItemView)
        menuItemView->doDelayedLayout();
    delayMenuConstruction = false;
}

void HbMenuPrivate::changeToOptionsMenu()
{
    menuType = HbMenu::OptionsMenu;
}

HbMenuItem *HbMenuPrivate::subMenuItem()
{
    return mSubMenuItem;
}

void HbMenuPrivate::setSubMenuItem(HbMenuItem *menuItem)
{
    mSubMenuItem = menuItem;
}

void HbMenuPrivate::_q_onActionTriggered()
{
    Q_Q(HbMenu);
       HbAction *action = qobject_cast<HbAction *>(q->sender());
    if (action && !action->menu() ) { // do not trigger from opening submenu
        emit q->triggered(action);
    }
}

void HbMenuPrivate::_q_subMenuItemTriggered(HbAction *action)
{
    Q_Q(HbMenu);    
 
    // do not close the menu tree if the triggered action is
    // submenu item
    if (!action->menu()) { 
        q->close();
    } else {
        stopTimeout();
    }
}

void HbMenuPrivate::actionAdded(QActionEvent *actionEvent)
{
    if (delayMenuConstruction)
        return;
    if (actionEvent->action()->isVisible()){
        Q_Q(HbMenu);
        createMenuView();
        QObject::connect(actionEvent->action(), SIGNAL(triggered()), q, SLOT(_q_onActionTriggered()));
        menuItemView->addActionItem(actionEvent->action());
    }
}

void HbMenuPrivate::actionRemoved(QActionEvent *actionEvent)
{
    if (delayMenuConstruction)
        return;
    Q_Q(HbMenu);
    QObject::disconnect(actionEvent->action(), 0, q, 0);
    if (menuItemView)
        menuItemView->removeActionItem(actionEvent->action());
}

void HbMenuPrivate::actionChanged(QActionEvent *actionEvent)
{
    if (delayMenuConstruction)
        return;
    if (menuItemView)
        menuItemView->updateActionItem(actionEvent->action());
}

/*
   Returns current focusable action based on current row of the menuItemView or index if specified.
   If there is no focusable action it returns 0.
   Also returns the active item representing the active action or 0.
*/
HbAction *HbMenuPrivate::activeAction(HbMenuItem *&activeItem) const
{
    if(!menuItemView)
        return 0;
    HbAction *action = 0;
    HbMenuItem *currentItem = menuItemView->currentItem();
    if(currentItem && currentItem->action() && currentItem->action()->isVisible() &&
       currentItem->action()->isEnabled() && !currentItem->action()->isSeparator()) {
        action = static_cast<HbAction*>(currentItem->action());
        activeItem = currentItem;
    }
    return action;
}

/*
  Convenience overload
*/
HbAction *HbMenuPrivate::activeAction() const
{
    HbMenuItem* activeItem = 0;
    return activeAction(activeItem);
}

/*
  Opens a submenu for activeItem. If activeItem is 0 it uses activeAction() to determine active item
  and opens submenu for it if active action has submenu.
*/
void HbMenuPrivate::openSubmenu(HbMenuItem *activeItem)
{
    Q_Q(HbMenu);

    if (!activeItem) {
        activeAction(activeItem);
    }

    if (activeItem && activeItem->action() && activeItem->action()->isEnabled()) {
        HbAction *hbAction = qobject_cast<HbAction *>(activeItem->action());
        if (!hbAction)
            return;
        HbMenu *subMenu = hbAction->menu();
        if ( subMenu ) {
            subMenu->setLayoutDirection(q->layoutDirection());

            activeSubMenu = subMenu; 
            subMenu->setTimeout(timeout); 
            QObject::disconnect(subMenu, SIGNAL(aboutToClose()), q, SLOT(_q_subMenuTimedOut()));
            QObject::connect(subMenu, SIGNAL(aboutToClose()), q, SLOT(_q_subMenuTimedOut()));
            QObject::connect(subMenu, SIGNAL(triggered(HbAction*)), q, SLOT(_q_subMenuItemTriggered(HbAction*)));
            subMenu->show();

            // Reset the active submenu so that mouse event handling works.
            // huh ? activeSubMenu = 0;
        }
    }
}

void HbMenuPrivate::_q_subMenuTimedOut()
{
    Q_Q(HbMenu);
    if( menuTimedOut (activeSubMenu) ) {
        if ( activeSubMenu ) {
            activeSubMenu->disconnect();
        }
        q->close();
    }
}

void HbMenuPrivate::_q_handleMenuAfterOrientationChange()
{
    Q_Q(HbMenu);
    if ( menuType == HbMenu::ContextMenu || menuType == HbMenu::OptionsMenu ) {
        if(activeSubMenu){
            HbAction* action = activeSubMenu->activeAction();
            if(action) {
                closeMenuRecursively(action);
            }
            closeSubmenu();
        }
        q->close();
    }
}

/*!
* closes the menu after Orientation change
*/
void HbMenuPrivate::closeMenuAfterOrientationChange()
{
    Q_Q(HbMenu);
    HbMainWindow* w(q->mainWindow());
    if ( w ){
        QObject::disconnect( w, SIGNAL(aboutToChangeOrientation()),
                             q, SLOT(_q_handleMenuAfterOrientationChange()));
        QObject::connect( w, SIGNAL(aboutToChangeOrientation()),
                 q, SLOT(_q_handleMenuAfterOrientationChange()));
    }
}

void HbMenuPrivate::closeMenuRecursively(HbAction* menuAction)
{
    if(menuAction->menu() && menuAction->menu()->activeAction()) {
        closeMenuRecursively(menuAction->menu()->activeAction());
    }
    if(menuAction->menu()) {
        menuAction->menu()->close();
    }
}


bool HbMenuPrivate::menuTimedOut(HbMenu* menu)
{
    return (menu && menu->timeout() > 0 && HbMenuPrivate::d_ptr(menu)->timedOut);
}

void HbMenuPrivate::closeSubmenu()
{
    // Check whether this is a submenu.
    if (activeSubMenu) {
        activeSubMenu->close();
    }
}

void HbMenuPrivate::setSubMenuPosition()
{
    Q_Q(HbMenu);
    if (mSubMenuItem) {        
        qreal upperEdge = mSubMenuItem->scenePos().y() + mSubMenuItem->size().height() * 2 / 3;
        QSizeF windowSize = QSizeF(0,0);
        if (q->mainWindow()) {
            windowSize = q->mainWindow()->layoutRect().size();
        }
        if (windowSize.height() - mDownMargin - q->preferredHeight() < upperEdge) {
            upperEdge = windowSize.height() - mDownMargin - q->preferredHeight();
        }
        if (q->layoutDirection() == Qt::LeftToRight) {
            qreal leftEdge = mSubMenuItem->scenePos().x() +
                             mSubMenuItem->size().width() - mRightMargin;
            if ((windowSize.width() - q->size().width()) < leftEdge) {
                leftEdge = mSubMenuItem->scenePos().x() +
                           mSubMenuItem->size().width() +
                           mRightMargin - q->size().width();
            }
            q->setPreferredPos(QPointF(leftEdge, upperEdge));
        } else {
            qreal rightEdge = mSubMenuItem->scenePos().x() + mRightMargin;
            if ((rightEdge - q->size().width()) < 0) {
                rightEdge = mSubMenuItem->scenePos().x() - mRightMargin +
                            q->size().width();
            }
            q->setPreferredPos(QPointF(rightEdge, upperEdge), HbPopup::TopRightCorner);
        }
    }
}

/*!
    @beta
    @hbcore
    \class HbMenu
    \brief HbMenu is a menu widget for use in HbView.

    \image html hbmenu.png A menu with checkable items and a sub-menu.

    Use an HbMenu to show a list of options. There are two main types of menus:

    - The view options menu
    - Context menus (popup menus)

    There is one view options menu for each view.
    It is shown by tapping the title bar. You can access the view options menu by calling
    HbView::menu() which returns a pointer to a new empty menu if one does not already exist.

    You can create any number of context menus.
    Context menus are usually invoked by a user action, such as tapping a widget.

    A menu contains a list of items. You can add three kinds of items to a menu:

    - Actions
    - Sub-menus
    - Separators

    An action is an object of class HbAction that performs an action when it is triggered.
    Use addAction() to add an action to a menu. Actions can be checkable (QAction::setCheckable).
    Use clearActions() to clear all actions from a menu.
    Use removeAction() to remove individual actions from a menu.

    A sub-menu is a menu that is nested within another menu. Use addMenu() or insertMenu() to add a sub-menu to a menu.
    Sub-menus can be nested within sub-menus.

    Separators group related items in a menu.
    Use addSeparator() or insertSeparator() to create and add a separator to a menu.

    \image html hbmenu.png A menu with checkable actions and a sub-menu.

    After you add an action to your menu, you specify a receiver object and its slot (you can also
    add an action and specify a receiver slot at the same time).
    The receiver is notifed when the action is triggered (QAction::triggered()).
    HbMenu also has a triggered() menu signal, which signals which HbAction was triggered in the menu.

    Context menu example:

    A menu and a few actions are created. The triggered() signal of the menu is connected to
    the mute() function of the enclosing class (implementation not shown).
    The exec() function shows the menu.

    User needs to connect to "longPress" signal and implement corresponding slot. This enables
    longpress events to be received from list.

    \dontinclude decoratorlistdemo/contentwidget.cpp
    \skip // Create new menu
    \until ( coords );

    \sa HbDialog, HbView
*/

/*!
    \property HbMenu::menuType
    \brief
*/

/*!
    \fn void HbMenu::triggered(HbAction *action)

    This signal is emitted when one of the action items is selected.
    \param action the action that was triggered in the menu.
 */

/*!
    \enum HbMenu::MenuType

    This enum describes different types of HbMenu.
*/
/*!
    \var HbMenu::ContextMenu

    ContextMenu is a menu which position is set by user.
*/
/*!
    \var HbMenu::OptionMenu

    OptionMenu is set by HbView. Its position cannot be changed.
*/
/*!
    \var HbMenu::SubMenu

    Menu becomes SubMenu when it is added to another menu.
*/

/*!
    Constructs a menu with \a parent graphics item.

    \param parent is the parent graphics item.
*/
HbMenu::HbMenu(QGraphicsItem *parent) :
        HbPopup(*new HbMenuPrivate, parent)
{
    Q_D(HbMenu);
    d->q_ptr = this;
    d->init();
    setModal(true);
}

/*!
    Constructs a menu with \a title and \a parent graphics item.
    \param title is the menu title.
    \param parent is the parent graphics item.
*/
HbMenu::HbMenu(const QString &title, QGraphicsItem *parent) :
        HbPopup(*new HbMenuPrivate, parent)
{
    Q_D(HbMenu);
    d->q_ptr = this;
    d->init();
    setTitle(title);
}

HbMenu::HbMenu(HbMenuPrivate &dd, QGraphicsItem *parent) :
        HbPopup(dd, parent)
{
    Q_D(HbMenu);
    d->q_ptr = this;
    d->init();
}

HbMenu::~HbMenu()
{
    if (!scene() || !scene()->property("destructed").isValid()) {
        foreach (QAction *action, actions()) {// krazy:exclude=qclasses
            HbAction* hbAction = qobject_cast<HbAction *>(action);
            if (hbAction){
                if (hbAction->menu()) {
                    hbAction->menu()->deleteLater();
                }
                hbAction->setMenu((HbMenu*)0);
            }
        }
    }
}

/*!
 \deprecated HbMenu::exec(HbAction*)
         is deprecated. Please use
  void HbMenu::open( QObject *receiver, const char *member ) or 
  HbMenu::show() instead.

    Executes the menu synchronously so that given \a action
    is active.

    \param action is the action that is active when the menu is shown.

    Example usage:
    \code
    void MyGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
    {
        HbMenu menu();
        menu.addAction(...);
        ...
        menu.setPreferredPos(event->scenePos(), HbPopup::BottomEdgeCenter);
        menu.exec();
    }
    \endcode

    \return the triggered HbAction in either the popup menu or one
    of its sub-menus, or 0 if no item was triggered (normally because
    the user closed or cancelled the menu).
 */
HbAction *HbMenu::exec(HbAction *action)
{
    HB_DEPRECATED("HbMenu::exec is deprecated. Use HbMenu::show() or HbMenu::open() instead!");
    Q_D(HbMenu);
    if (actions().count() == 0) {
        return 0;
    }

    if(!action)
        action = qobject_cast<HbAction*>(actions().first());

    setActiveAction(action);

    // Reset state variables
    d->resultAction = 0;
    d->actionTriggered = false;

    if (d->menuType == SubMenu && d->polished) {
        d->setSubMenuPosition();
    }
    QPointer<HbMenu> menuAlive(this);
    HbPopup::exec();

    // HbMenu can be deleted while exec
    if (menuAlive) {
        return d->resultAction;
    } else {
        return 0;
    }
}

/*!
  \deprecated HbMenu::exec(const QPointF&, HbAction*)
     is deprecated. Please use
  void HbMenu::open( QObject *receiver, const char *member ) or 
  HbMenu::show() and setPreferredPos() instead.

    Executes the menu synchronously at \a pos so that given \a action
    is active.

    \param pos is the position at which the menu is shown.
    \param action is the action that is active when the menu is shown.

    Example usage:
    \code
    void MyGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
    {
        HbMenu menu();
        menu.addAction(...);
        ...
        menu.exec(event->scenePos());
    }
    \endcode

    \return the triggered HbAction in either the popup menu or one
    of its sub-menus, or 0 if no item was triggered (normally because
    the user closed or cancelled the menu).
 */
HbAction *HbMenu::exec(const QPointF &pos, HbAction *action )
{
    Q_D(HbMenu);
    if (d->menuType == ContextMenu) {
        setPreferredPos(pos);
    }
    return exec(action);
}

void HbMenu::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    Q_D(HbMenu);
    d->actionTriggered = false;

    if (d->menuType == SubMenu && d->polished) {
        d->setSubMenuPosition();
    }

    HbPopup::showEvent(event);
}

/*!
    Creates a new action with title \a text. It adds the newly created action to the menu's list of actions.
    \param text is the text for the new action.
    \return the new action.
*/
HbAction *HbMenu::addAction(const QString &text)
{
    HbAction *action = new HbAction(text, this);
    addAction(action);
    return action;
}

/*!
    Creates a new action with \a text.
    The action's triggered() signal is connected to the
    \a receiver's \a member slot. The function adds the newly created
    action to the menu's list of actions.
    \return the new action.
 */
HbAction *HbMenu::addAction(const QString &text, const QObject *receiver, const char *member)
{
    HbAction *action = new HbAction(text, this);
    connect(action, SIGNAL(triggered(bool)), receiver, member);
    addAction(action);
    return action;
}

/*!
    Adds \a menu as a sub-menu.
    \param menu is the menu that is added to this one.
    \return the action for the added sub-menu.
 */
HbAction *HbMenu::addMenu(HbMenu *menu)
{
    return insertMenu(0, menu);
}

/*!
    Creates a new HbMenu with \a title and adds it to this menu.
    \param title is the menu title.
    \return the new menu.
*/
HbMenu *HbMenu::addMenu(const QString &title)
{
    HbMenu *menu = new HbMenu(title);
    addMenu(menu);
    return menu;
}

/*!
    Inserts \a menu before action \a before.
    \param before is the action before which this new menu is inserted.
    \param menu is the menu that is inserted.
    \return the action associated with the inserted menu.
 */
HbAction *HbMenu::insertMenu(HbAction *before, HbMenu *menu)
{    
    QObject::connect(menu, SIGNAL(triggered(HbAction*)), this, SLOT(_q_subMenuItemTriggered(HbAction*)));
    QObject::connect(menu, SIGNAL(triggered(HbAction*)), this, SIGNAL(triggered(HbAction*)));

    menu->d_func()->menuType = HbMenu::SubMenu;
    insertAction(before, menu->menuAction());
    return menu->menuAction();
}

/*!
    \return the action associated with this menu.
 */
HbAction *HbMenu::menuAction() const
{
    Q_D(const HbMenu);
    return d->subMenuAction;
}

/*!
    Creates a new separator action, which is an action that returns \c true from HbAction::isSeparator(),
    and adds it to this menu's list of actions.
    \return the new separator action

    \sa insertSeparator
 */
HbAction *HbMenu::addSeparator()
{
    return insertSeparator(0);
}

/*!
    Inserts a new separator action and inserts it into this menu's list of actions before \a action.
    \param before is the action before which the separator is inserted.
    \return the new action.

    \sa addSeparator
 */
HbAction *HbMenu::insertSeparator(HbAction *before)
{
    HbAction *action = new HbAction(this);
    action->setSeparator(true);
    action->setEnabled(false);
    action->setVisible(false);
    insertAction(before, action);
    return action;
}

/*!
    \return the current active action, or 0 if no action item is currently active.
 */
HbAction *HbMenu::activeAction() const
{
    Q_D(const HbMenu);
    return d->activeAction();
}

/*!
    Sets the active action in menu. If \a action is not found from the list of
    menu actions then the current active action remains active.

    \sa activeAction()
*/
void HbMenu::setActiveAction(HbAction *action)
{
    Q_D(HbMenu);
    if (d->menuItemView && action && action->isVisible() && !action->isSeparator()) {
        d->menuItemView->setCurrentItem(action);
    }
}

/*!
    \return \c true if the menu is empty (contains no actions) and \c false otherwise.

    \sa clear()
 */
bool HbMenu::isEmpty() const
{
    return actions().isEmpty();
}

/*!
    Sets the menu title. For a sub-menu, the title is the sub-menu action text.

    \sa title()
*/
void HbMenu::setTitle(const QString &title)
{
    menuAction()->setText(title);
}

/*!
    \return the menu title. For a sub-menu, the title is the sub-menu action text.

    \sa setTitle()
*/
QString HbMenu::title() const
{
    return menuAction()->text();
}

/*!
    \return the menu type. By default a menu is a context menu.
*/
HbMenu::MenuType HbMenu::menuType() const
{
    Q_D(const HbMenu);
    return d->menuType;
}

/*!
    \reimp
 */
QVariant HbMenu::itemChange( GraphicsItemChange change, const QVariant & value )
{
    Q_D(HbMenu);

    if (change == QGraphicsItem::ItemSceneHasChanged) {
        d->closeMenuAfterOrientationChange();
    }
    if (change == QGraphicsItem::ItemVisibleChange) {
        if (value.toBool() && d->delayMenuConstruction) {
            d->delayedLayout();
        }
        if (value.toBool()) {
            d->resultAction = 0;
            d->actionTriggered = false;
        }
        else if (!value.toBool() && !d->menuItemView){
            d->delayMenuConstruction = true;
        }
    }
    return HbPopup::itemChange(change,value);
}

/*!
   \reimp
*/
bool HbMenu::event(QEvent *event)
{
    Q_D(HbMenu);

    if(!d->inDestruction) {
        if (event->type() == QEvent::ActionAdded) {
            QActionEvent *actionEvent = static_cast<QActionEvent *>(event);
            d->actionAdded(actionEvent);            
            return true;
        } else if (event->type() == QEvent::ActionRemoved) {
            QActionEvent *actionEvent = static_cast<QActionEvent *>(event);
            d->actionRemoved(actionEvent);
            return true;
        } else if (event->type() == QEvent::ActionChanged) {
            QActionEvent *actionEvent = static_cast<QActionEvent *>(event);
            d->actionChanged(actionEvent);
            return true;
        }
    }
    if (event->type() == QEvent::LayoutRequest) {
        if(d->menuItemView)
            d->menuItemView->contentWidget()->adjustSize();
    } else if (event->type() == QEvent::GraphicsSceneResize){
        if (d->mSubMenuItem)
            d->setSubMenuPosition();
    }

    return HbPopup::event(event);
}

void HbMenu::polish(HbStyleParameters &params)
{
    Q_D(HbMenu);
    if (d->mSubMenuItem) {
        const QString RightMargin = "submenu-right-offset";
        const QString DownMargin = "submenu-bottom-margin";
        params.addParameter(RightMargin);
        params.addParameter(DownMargin);

        HbPopup::polish(params);

        if (!params.value(RightMargin).isNull()) {
            d->mRightMargin = params.value(RightMargin).toDouble();
        }
        if (!params.value(DownMargin).isNull()) {
            d->mDownMargin = params.value(DownMargin).toDouble();
        }
        d->setSubMenuPosition();
    } else {
        HbPopup::polish(params);
    }
}

QPainterPath HbMenu::shape() const
{
    QRectF sceneRect = mapRectToScene(QRectF(-0.5, -0.5, boundingRect().width() + 0.5, boundingRect().height() + 0.5));
    QRectF clipRect = sceneRect.intersected(QRectF(pos().x() - 0.5, pos().y() - 0.5, size().width() + 0.5, size().height() + 0.5));

    QPainterPath path;
    path.addRect(mapRectFromScene(clipRect));

    return path.intersected(HbPopup::shape());
}

/*!  @alpha
 *
 * Opens the menu and returns immediately.
 */
void HbMenu::open( QObject *receiver, const char *member )
{
    Q_D(HbMenu);
    if ( d->receiverToDisconnectOnClose ) { // cant do on closeevent
        disconnect(this, SIGNAL(triggered(HbAction*)),
                   d->receiverToDisconnectOnClose, d->memberToDisconnectOnClose);
        d->receiverToDisconnectOnClose = 0;
    }
    d->memberToDisconnectOnClose.clear();

    if ( receiver ) {
        connect(this, SIGNAL(triggered(HbAction*)), receiver, member);
        d->receiverToDisconnectOnClose = receiver;
        d->memberToDisconnectOnClose = member;
    } else {
        d->receiverToDisconnectOnClose = 0;
    }
    HbMenu::show();
}

#include "moc_hbmenu.cpp"
