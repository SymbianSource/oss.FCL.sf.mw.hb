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
#include <QGraphicsSceneMouseEvent>
#include <QPointer>
#include <QGraphicsLinearLayout>

Q_DECLARE_METATYPE (QAction*)// krazy:exclude=qclasses

HbMenuPrivate::HbMenuPrivate():
    HbPopupPrivate(),
    menuItemView(0),
    subMenuAction(0),
    activeSubMenu(0),
    actionTriggered(false),
    menuType(HbMenu::ContextMenu),
    mSubMenuItem(0),
    mRightMargin(0.0),
    mDownMargin(0.0),
    delayMenuConstruction(true),
    receiverToDisconnectOnClose(0),
    mNumberOfColumns(1),
    mCloseAllMenus(false)
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
    if (menuType == HbMenu::OptionsMenu) {
        effectType = "HB_MENU";
    } else {
        effectType = "HB_SUBMENU";
    }
    hasEffects = menuEffectsLoaded;
    if (menuEffectsLoaded)
        return;
    menuEffectsLoaded = true;
    hasEffects = HbEffectInternal::add("HB_MENU", "menu_appear", "appear");
    if (hasEffects) {
        hasEffects = HbEffectInternal::add("HB_MENU", "menu_disappear", "disappear");

        if (hasEffects) {
            hasEffects = HbEffectInternal::add("HB_menuitem", "menuitem_press", "clicked");
        }
        if (hasEffects) {
            hasEffects = HbEffectInternal::add("HB_SUBMENU", "submenu_appear", "appear");
        }
        if (hasEffects) {
            hasEffects = HbEffectInternal::add("HB_SUBMENU", "submenu_disappear", "disappear");
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
    if (delayMenuConstruction) {
        createMenuView();
        if(menuItemView) {
            menuItemView->doDelayedLayout();
        }
        addPopupEffects();
        delayMenuConstruction = false;
    }
}

void HbMenuPrivate::changeToOptionsMenu()
{
    menuType = HbMenu::OptionsMenu;
    effectType = "HB_MENU";
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
    if (action) {
        q->setActiveAction(action);
        if (!action->menu()) { // do not trigger from opening submenu
            emit q->triggered(action);
        }
    }
}

void HbMenuPrivate::_q_subMenuItemTriggered(HbAction *action)
{
    Q_Q(HbMenu);    
 
    // do not close the menu tree if the triggered action is
    // submenu item
    if (!action->menu()) { 
        mCloseAllMenus = true;
        q->close();
    } else {
        stopTimeout();
    }
}

void HbMenuPrivate::actionAdded(QActionEvent *actionEvent)
{
    if (delayMenuConstruction)
        return;
    Q_Q(HbMenu);
    QObject::connect(actionEvent->action(), SIGNAL(triggered()), q, SLOT(_q_onActionTriggered()));
    if (actionEvent->action()->isVisible()){        
        createMenuView();        
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

/*
    reimp
*/
void HbMenuPrivate::handleBackgroundMouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_Q(HbMenu);

    // Handle cases only for Background or Popup originated mouse presses and when
    // any dismiss policy defined
    if (mousePressLocation != None && dismissPolicy != HbPopup::NoDismiss) {

        MouseEventLocationType mouseReleaseLocation = Background;
        HbMenu* tempMenu = 0;
        QList<QGraphicsItem*> itemList = q->scene()->items(event->pos());
        foreach (QGraphicsItem* sceneItem, itemList ){
            QGraphicsWidget *focusWidget = 0;
            HbMenu *focusMenu = 0;
            if (sceneItem && sceneItem->isWidget()) {
                focusWidget = static_cast<QGraphicsWidget*>(sceneItem);
                focusMenu = qobject_cast<HbMenu *>(focusWidget);
            }
            if( focusMenu ) {
                mouseReleaseLocation = Popup;
                tempMenu = focusMenu;
                break;
            }
        }
        // Mouse is released within menu
        if (mouseReleaseLocation == Popup) {
            HbAction* menuAction = tempMenu->activeAction();
            if(menuAction){
                mCloseAllMenus = false;
                closeMenuRecursively(menuAction);
            }
        mousePressLocation = None;
        }
        else {
            // trigger the menu to close whole menu
            mCloseAllMenus = true;
            q->close();
            mousePressLocation = None;
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
        timedOut = true;
        q->close();
    } else {
        startTimeout();
        mCloseAllMenus = false;
    }
    if (activeSubMenu && HbMenuPrivate::d_ptr(activeSubMenu)->mCloseAllMenus) {
        mCloseAllMenus = true;
        q->close();
   }
}

void HbMenuPrivate::_q_handleMenuClose()
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

/*
    Handles menu close
*/
void HbMenuPrivate::closeMenu()
{
    Q_Q(HbMenu);
    HbMainWindow* w(q->mainWindow());
    if ( w ){
        QObject::disconnect( w, SIGNAL(aboutToChangeOrientation()),
                             q, SLOT(_q_handleMenuClose()));
        QObject::connect( w, SIGNAL(aboutToChangeOrientation()),
                        q, SLOT(_q_handleMenuClose()));
        QObject::disconnect( w, SIGNAL(aboutToChangeView(HbView*, HbView*)),
                          q, SLOT(_q_handleMenuClose()));
        QObject::connect( w, SIGNAL(aboutToChangeView(HbView*, HbView*)),
                          q, SLOT(_q_handleMenuClose()));
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
            q->setPreferredPos(QPointF(leftEdge, upperEdge), HbPopup::TopLeftCorner);
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
    \brief The HbMenu class provides a widget that shows a list of options.

    A menu consists of a list of options that the user can select. When the user selects
    an option, it triggers a command. Menus are hidden by default and open in response
    to an action from the user. For this reason, the toolbar (HbToolBar class) usually
    provides access to the most important commands (called first order commands) and
    menus provide access to second order commands.

    \image html hbmenu.png An options menu with a submenu that has checkable actions

    You can use the %HbMenu class to create the following:

    - An \b options \b menu, which shows options that relate to the whole view. It is
    a drop-down menu that is directly owned by the view (HbView object). It opens when the
    user taps on the view's title bar. You can access a view's options menu by calling
    HbView::menu(). This returns a pointer to a new empty menu if the menu does not
    already exist. A view can have only one options menu.

    - A \b context \b menu, which is a pop-up menu that shows options that relate to a
    specific item, rather than to the entire view. It typically opens in response to a
    UI event, such as a tap on a widget or a particular point within the view. You can
    create any number of context menus.

    A menu is a list of actions, which are objects of class HbAction. A menu can contain:

    - \b Separators, which are actions that group related items. Use addSeparator() or
    insertSeparator() to create a separator and add it to a menu.

    - Actions that trigger a \b submenu to open. Submenus are menus that are nested within
    another menu. Use addMenu() or insertMenu() to add a submenu to a menu. Although it is
    possible to nest submenus within other submenus, generally this is not considered good
    design practice.

    - \b Action \b items, which represent the menu options. Use addAction() and insertAction() to
    add an action to a menu. Use addActions() and insertActions() to add and insert multiple
    actions in one operation. Use clearActions() to clear all of the actions from a menu and
    removeAction() to remove individual actions from a menu.

    The order of the actions within the menu controls the order of the options that the user sees.
    When you add the actions directly to the menu, addAction() and addActions() append the actions
    to the end of the menu and insertAction() and insertActions() enable you to specify the position.
    For options menus, however, there is an alternative approach to ordering the action items.
    This is to call HbView::addAction() to add actions to the \b view and let the view distribute
    them to the options menu or toolbar, depending on the preference set, the UI command distribution
    template, and taking into account the available space in the toolbar. The menu and toolbar
    then order the actions according to their defined roles and the UI command container template.
    This approach makes it easier to create consistent user interfaces and applications that
    work well on a variety of different devices.

    An action item can be checkable, which means that it has an on/off state. You specify that an
    action item is checkable by calling \c setCheckable() on the action. Use \c isChecked()
    to discover if an action is checked. You can also use the QAction::toggled(bool) signal to
    receive notification of a change in the checked status.

    After you add an action item to a menu, you can connect its \link HbAction::triggered()
    triggered()\endlink signal to a slot on a receiver object. Alternatively you can use the
    addAction(const QString &, const QObject *, const char *) overload to add an action item and
    specify a receiver slot at the same time. The receiver is notified when the action item
    is \link HbAction::triggered() triggered()\endlink.

    You can also connect the HbMenu::triggered(HbAction*) signal to a receiver object's slot. This
    signal is emitted when any menu action is triggered. You can find out which action was
    triggered from the HbAction parameter.

    \section _usecases_hbmenu Using the HbMenu class

    \subsection _uc_001_hbmenu Creating an options menu

    The following example creates an options menu for a view.
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,2}

    \subsection _uc_002_hbmenu Creating a context menu

    The following example creates a context menu and shows it in response to a tap and hold gesture.
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,54}

    \subsection _uc_004_hbmenu Adding actions to the view

    The following example creates two action items, specifies their command roles, and then
    adds them to the view. The view places them in the menu or toolbar according to the
    priorities of the actions and the space available in the toolbar and the menu orders them
    according to their roles and priorities.

    \code
    HbAction* actionExit = new HbAction(tr("Exit"));
    actionExit->setCommandRole(HbAction::ExitRole);

    HbAction* actionHelp = new HbAction(tr("Help"));
    actionHelp->setCommandRole(HbAction::HelpRole);

    // Add actions to the view.
    myView->addAction(actionExit);
    myView->addAction(actionHelp);
    \endcode

    \subsection _uc_003_hbmenu Creating action items that are checkable

    You can create a menu that contains multiple checkable actions (actions with checkbox behavior).
    For simple checkbox behavior, just set the actions to be checkable by calling \c setCheckable(true).
    For example:

    \code
    ...
    checkAction1->setCheckable(true);
    checkAction2->setCheckable(true);
    checkAction3->setCheckable(true);
    ...
    \endcode

    \subsection _uc_005_hbmenu Creating radio button style options

    To create a group of related actions, only one of which can be checked (radio button behavior),
    create a QActionGroup object and add the actions to that. For example:

    \code
    HbAction *redAction = menu->addAction(tr("Red"));
    HbAction *blueAction = menu->addAction(tr("Blue"));

    redAction->setCheckable(true);
    blueAction->setCheckable(true);

    QActionGroup *actionGroup = new QActionGroup(view);
    actionGroup->addAction(redAction);
    actionGroup->addAction(blueAction);
    \endcode

    \sa HbView, HbToolBar, HbAction
*/

/*!
    \property HbMenu::menuType
    \brief
*/

/*!
    \fn void HbMenu::triggered(HbAction *action)

    This signal is emitted when one of the menu options is selected.
    \param action The action that was selected.
*/

/*!
    \enum HbMenu::MenuType

    The MenuType enum identifies the possible HbMenu types.
*/
/*!
    \var HbMenu::ContextMenu
    A popup menu.
*/
/*!
    \var HbMenu::OptionsMenu
    The main options menu in a view. Its position cannot be changed.
*/
/*!
    \var HbMenu::SubMenu
    A submenu, which is a menu that has been added to another menu.
*/

/*!
    Constructs a menu with \a parent graphics item.
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
*/
HbMenu::HbMenu(const QString &title, QGraphicsItem *parent) :
        HbPopup(*new HbMenuPrivate, parent)
{
    Q_D(HbMenu);
    d->q_ptr = this;
    d->init();
    setTitle(title);
}

/*!
    Protected constructor.
*/
HbMenu::HbMenu(HbMenuPrivate &dd, QGraphicsItem *parent) :
        HbPopup(dd, parent)
{
    Q_D(HbMenu);
    d->q_ptr = this;
    d->init();
}

/*!
    Destructor
*/
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
    Creates a new action and adds it to the end of the menu.

    \overload

    \param text The menu text for the new action.
    \return The new action.
*/
HbAction *HbMenu::addAction(const QString &text)
{
    HbAction *action = new HbAction(text, this);
    addAction(action);
    return action;
}

/*!
    Creates a new action, adds it to the end of the menu and connects the action's
    \link HbAction::triggered() triggered()\endlink signal to a receiver's slot.

    \overload

    \param text The menu text for the new action.
    \param receiver The object that is to receive the new action's signal.
    \param member The slot on the receiver to which the action's signal is to connect.
    \return The new action.
*/
HbAction *HbMenu::addAction(const QString &text, const QObject *receiver, const char *member)
{
    HbAction *action = new HbAction(text, this);
    connect(action, SIGNAL(triggered(bool)), receiver, member);
    addAction(action);
    return action;
}

/*!
    Adds \a menu to the current menu as a submenu.

    \return  The action for the added submenu.
*/
HbAction *HbMenu::addMenu(HbMenu *menu)
{
    return insertMenu(0, menu);
}

/*!
    Creates a new HbMenu with \a title and adds it to the current menu as a submenu.

    \return The new menu.
*/
HbMenu *HbMenu::addMenu(const QString &title)
{
    HbMenu *menu = new HbMenu(title);
    if ( menu ) {
        menu->setParent(this);
        addMenu(menu);
    }
    return menu;
}

/*!
    Inserts \a menu into the current menu as a submenu before the \a before action.

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
    Returns the action that is directly associated with a menu (rather than actions that are contained
    within the menu). Although all menus have an assoicated action, it is only actually used for submenus.
    The parent menu uses a submenu's action to trigger the opening of the submenu. A submenu's action
    also defines the submenu's title.
*/
HbAction *HbMenu::menuAction() const
{
    Q_D(const HbMenu);
    return d->subMenuAction;
}

/*!
    Creates a new separator and adds it to the current menu's list of actions. A separator
    is an action for which HbAction::isSeparator() returns \c true.

    \return The new separator action.

    \sa insertSeparator()
*/
HbAction *HbMenu::addSeparator()
{
        //functionality removed for now
    //return insertSeparator(0);
    return 0;
}

/*!
    Creates a new separator and inserts it into the current menu's list of actions before \a before.
    A separator is an action for which HbAction::isSeparator() returns \c true.

    \return The new separator action.
    \sa addSeparator()
*/
HbAction *HbMenu::insertSeparator(HbAction *before)
{
    Q_UNUSED(before);
    //functionality removed for now
    /*HbAction *action = new HbAction(this);
    action->setSeparator(true);
    action->setEnabled(true);
    action->setVisible(true);
    insertAction(before, action);
    return action;*/
    return 0;
}

/*!
    Returns the active action or 0 if no action item is currently active. The active action is
    the last action that was triggered, unless this has been overridden by a call to setActiveAction().
*/
HbAction *HbMenu::activeAction() const
{
    Q_D(const HbMenu);
    return d->activeAction();
}

/*!
    Sets \a action as the current active action in the menu. If \a action is not
    found in the list of menu actions, the action that is currently active remains active.

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
    Returns \c true if the menu contains no actions and \c false otherwise.

    \sa clearActions()
*/
bool HbMenu::isEmpty() const
{
    return actions().isEmpty();
}

/*!
    Sets the menu title. For a submenu, the title is the submenu's action text.

    \sa title()
*/
void HbMenu::setTitle(const QString &title)
{
    menuAction()->setText(title);
}

/*!
    Returns the menu title. For a submenu, the title is the submenu's action text.

    \sa setTitle()
*/
QString HbMenu::title() const
{
    return menuAction()->text();
}

/*!
    Returns the menu type. The default menu type is context menu.

*/
HbMenu::MenuType HbMenu::menuType() const
{
    Q_D(const HbMenu);
    return d->menuType;
}

QVariant HbMenu::itemChange( GraphicsItemChange change, const QVariant & value )
{
    Q_D(HbMenu);

    if (change == QGraphicsItem::ItemSceneHasChanged) {
        d->closeMenu();
    }
    if (change == QGraphicsItem::ItemVisibleChange) {
        if (value.toBool() && d->polished) {
            d->delayedLayout();
        }
        if (value.toBool()) {
            d->actionTriggered = false;
        }
        else if (!value.toBool() && !d->menuItemView){
            d->delayMenuConstruction = true;
        }
    }
    return HbPopup::itemChange(change,value);
}

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
    if (event->type() == QEvent::GraphicsSceneResize){
        if (d->mSubMenuItem)
            d->setSubMenuPosition();
    }

    return HbPopup::event(event);
}

void HbMenu::polish(HbStyleParameters &params)
{
    if (isVisible()) {
        Q_D(HbMenu);
        const QLatin1String NumberOfCols("number-of-columns");
        params.addParameter(NumberOfCols);

        if (d->mSubMenuItem) {
            const QLatin1String RightMargin("submenu-right-offset");
            const QLatin1String DownMargin("submenu-bottom-margin");
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

        if (!params.value(NumberOfCols).isNull()) {
            int cols = params.value(NumberOfCols).toInt();
            if (d->mNumberOfColumns != cols) {
                d->mNumberOfColumns = cols;
                if (d->menuItemView) {
                    d->menuItemView->updateContainer();
                }
            }
        }
        d->delayedLayout();
    } else {
        HbPopup::polish(params);
    }
}

/*!
    Returns the shape of this item as a QPainterPath.
*/
QPainterPath HbMenu::shape() const
{
    QRectF rect = QRectF(-1.0, -1.0, boundingRect().width() + 1.0, boundingRect().height() + 1.0);
    QRectF clipRect = rect.intersected(mapRectFromParent(QRectF(pos().x() - 1.0, pos().y() - 1.0, size().width() + 1.0, size().height() + 1.0)));

    QPainterPath path;
    path.addRect(clipRect);

    return path;
//    return HbPopup::shape();
}

/*!
    Displays the menu on the screen and returns immediately. It can also connect the HbMenu::triggered()
    signal to a specified slot. The signal is disconnected when the menu closes.

    \param receiver The object that is to receive the signal.
    \param member The slot on the receiver to which the signal is to connect.
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

/*!
    Overidden from popup. Sets the preferred position of the menu.

    \param preferredPos Defines the coordinates of preferred position on the screen where
           the menu is to open.
    \param placement The corner or edge of the dialog that is to be placed at \a preferredPos.

    \b Example:
    \code
    HbMenu *menu = new HbMenu();

    menu->setPreferredPos( QPointF(x,y), HbPopup::BottomEdgeCenter );
    menu->show();
    \endcode
 */
void HbMenu::setPreferredPos( const QPointF& preferredPos,
                               HbPopup::Placement placement )
{
    Q_D(HbMenu);
    bool layoutFlag = false;
    if (d->preferredPos != preferredPos ) {
        d->preferredPos = preferredPos;
        layoutFlag = true;
    }
    if (d->placement != placement) {
        d->placement = placement;
        layoutFlag = true;
    }
    d->preferredPosSet = true;
    //If position updated, informing layoutproxy with layoutrequest
    if (layoutFlag) {
        QEvent layoutRequest = QEvent::LayoutRequest;
        QApplication::sendEvent(this, &layoutRequest);
    }
}


#include "moc_hbmenu.cpp"
