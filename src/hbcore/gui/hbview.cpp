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

#include "hbview.h"
#include "hbview_p.h"
#include "hbinstance.h"
#include "hbmenu.h"
#include "hbmenu_p.h"
#include "hbtoolbar.h"
#include "hbstackedlayout.h"
#include "hbdockwidget.h"
#include "hbwidget_p.h"
#include "hbmainwindow_p.h"
#include "hbtitlebar_p.h"
#include "hbstatusbar_p.h"
#include "hbsoftkeygroup_p.h"
#include "hbscreen_p.h"
#include "hbviewactionmanager_p.h"
#include "hbglobal_p.h"
#include <QPointer>

/*!
    @beta
    @hbcore
	\class HbView
	\brief HbView is a base class for all the views that can be set to HbMainWindow.
	
	HbView is a QGraphicsWidget, which contains a title, an icon, a menu and
	a tool bar for one view in HbMainWindow.
	
	HbMainWindow will handle updating title, icon, menu and tool bar, when
	the view is activated. Title is used in application's title pane area.
	
	HbView handles the visibility of tool bar, though HbMainWindow will
	reparent it when ever view comes active.
	
	Example 1 for creating a simple view
	
	\include view1/main.cpp
	
	Example 2 for creating view with title, icon, menu, toolbar and a widget.
	
	\include view2/main.cpp

    \sa HbMenu
 */
/*!
    \deprecated HbView::HbTitleBarFlag
        is deprecated. Use HbView::::HbViewFlag instead.
    \enum HbView::HbTitleBarFlag
    Defines policy for showing the HbTitleBar in the current HbView.
*/
/*!
    \var HbView::TitleBarFlagNone 
    Titlebar is shown with the default attributes.
*/
/*!
    \var HbView::TitleBarMinimizable When this flag is set, there will be an
    indication in titlebar showing the possibility to minimize (swipe out of the
    screen) titlebar.  */
/*!
    \var HbView::TitleBarMinimized
    When this flag is set, the titlebar is shown in minimized state.
*/
/*!
    \var HbView::TitleBarHidden When this flag is set, the titlebar is not shown
    at all, so that even the titlebar handle is not shown and it is not possible
    to maximize the titlebar.  */
/*!
    \var HbView::TitleBarTransparent When this flag is set, the titlebar is
    shown normally, but as transparent. This flag is normally used in
    combination with TitleBarFloating flag.  */
/*!
    \var HbView::TitleBarFloating When this flag is set, the titlebar is shown
    on top of underlying content. Setting this flag also changes application
    area to start from the top of the screen. This flag is normally used in
    combination with TitleBarTransparent flag.  */

/*!
    \enum HbView::HbViewFlag
    Defines policy for showing the HbTitleBar in the current HbView.
*/
/*!
    \var HbView::ViewFlagNone 
    Titlebar and statusbar is shown with the default attributes.
*/
/*!
    \var HbView::ViewTitleBarMinimizable
    When this flag is set, there will be an indication in titlebar showing the possibility to minimize (swipe out of the screen) titlebar.
*/
/*!
    \var HbView::ViewTitleBarMinimized
    When this flag is set, the titlebar is shown in minimized state.
*/
/*!
    \var HbView::ViewTitleBarHidden
    When this flag is set, the titlebar is not shown at all, so that even the titlebar handle is not shown and it is not possible to maximize the titlebar.
*/
/*!
    \var HbView::ViewTitleBarTransparent
    When this flag is set, the titlebar is shown normally, but as transparent. This flag is normally used in combination with ViewTitleBarFloating flag.
*/
/*!
    \var HbView::ViewTitleBarFloating
    When this flag is set, the titlebar is shown on top of underlying content. Setting this flag also changes application area to start from the top of the screen. This flag is normally used in combination with TitleBarTransparent flag.
*/
/*!
    \var HbView::ViewStatusBarHidden
    When this flag is set, the statusbar is not shown at all. This flag is normally used in combination with ViewTitleBarHidden flag.
*/
/*!
    \var HbView::ViewStatusBarTransparent
    When this flag is set, the statusbar is shown normally, but as transparent. This flag is normally used in combination with ViewStatusBarFloating flag.
*/
/*!
    \var HbView::ViewStatusBarFloating
    When this flag is set, the statusbar is shown on top of underlying content. Setting this flag also changes application area to start from the top of the screen. This flag is normally used in combination with ViewStatusBarTransparent flag.
*/

/*!
    \fn void HbView::titleChanged(const QString &title)

    This signal is emitted when the title changes.
 */

/*!
    \fn void HbView::iconChanged(const HbIcon &icon)

    This signal is emitted when the icon changes.
 */

/*!
    \fn void HbView::toolBarChanged()

    This signal is emitted when tool bar changes.
 */

/*!
    \reimp
    \fn int HbView::type() const
 */

HbViewPrivate::HbViewPrivate()
    : mLayout(0), 
      menu(0), 
      toolBar(0), 
      dockWidget(0),
      widget(0),
      mVisibleItems(Hb::AllItems),
      mVisibleItemsSet(false),
      mFullscreen(false),
      mVisited(false),
      mTitleBarFlags(HbView::TitleBarFlagNone),
      mViewFlags(HbView::ViewFlagNone),
      actionManager(0),
      preferredActionContainer(HbView::NotSpecified),
      mNavigationAction(0),
      mNavigationActionSet(false)
{
}

HbViewPrivate::~HbViewPrivate()
{
}

/*!
    Constructs a view with given \a parent.
*/
HbView::HbView(QGraphicsItem *parent) :
    HbWidget(*new HbViewPrivate, parent)
{
    Q_D(HbView);
    d->q_ptr = this;
}

/*!
    \internal
 */
HbView::HbView( HbViewPrivate &dd, QGraphicsItem *parent ):
    HbWidget(dd, parent)
{
    Q_D(HbView);
    d->q_ptr = this; 
}

/*!
    Destructs the view.
*/
HbView::~HbView()
{
    Q_D(HbView);
    if (d->menu) {
        d->menu->deleteLater();
    }
    if (d->toolBar) {
        d->toolBar->deleteLater();
    }
    if (d->dockWidget) {
        d->dockWidget->deleteLater();
    }
}

/*!
    Returns the title of the view.

    The default value is a null string.

    \sa setTitle()
 */
QString HbView::title() const
{
    Q_D(const HbView);
    return d->title;
}

/*!
    Sets the title of the view.

    \sa title()
 */
void HbView::setTitle(const QString &title)
{
    Q_D(HbView);
    if (d->title != title) {
        d->title = title;
        emit titleChanged(title);
    }
}

/*!
    Returns the icon of the view.

    The default value is a null icon.

    \sa setIcon(), HbIcon::isNull()
 */
HbIcon HbView::icon() const
{
    Q_D(const HbView);
    return d->icon;
}

/*!
    Sets the icon of the view. The icon is associated with view.

    \sa icon()
 */
void HbView::setIcon(const HbIcon &icon)
{
    Q_D(HbView);
    if (d->icon != icon) {
        d->icon = icon;
        emit iconChanged(icon);
    }
}

/*!
    Returns the menu for the view. This function creates and returns an empty menu if it does not exist.

    Ownership is not transferred.

    \sa HbMenu setMenu()
*/
HbMenu *HbView::menu() const
{
    Q_D(const HbView);
    if (!d->menu) {
        const_cast<HbView *>(this)->setMenu(new HbMenu());
    }
    if (mainWindow()) {
        d->menu->setLayoutDirection(mainWindow()->layoutDirection());
    }
    return d->menu;
}

/*!
    Sets the menu for the view. Setting the menu to 0 will remove it from the view.

    Takes the ownership of the \a menu.

    \sa HbMenu menu()
*/
void HbView::setMenu(HbMenu *menu)
{
    Q_D(HbView);
    if (d->menu != menu) {
        if(d->menu) {
            d->menu->deleteLater();
        }
        d->menu = menu;
        if (d->menu) {
            HbMenuPrivate::d_ptr(d->menu)->changeToOptionsMenu();
            if (mainWindow()) {
                d->menu->setLayoutDirection(mainWindow()->layoutDirection());
            }
        }
    }
}

/*!
    Returns the tool bar for the view. This function creates and returns an empty tool bar if it does not exist.
    The toolBarChanged() signal is not emitted if a new tool bar is created.

    Ownership is not transferred.

    \sa HbToolBar setToolBar()
*/
HbToolBar *HbView::toolBar() const
{
    Q_D(const HbView);
    if (!d->toolBar) {
        HbView *that = const_cast<HbView *>(this);
        that->setToolBar(new HbToolBar);
    }
    return d->toolBar;
}

/*!
    Sets the toolbar for the view. Setting the tool bar to 0 will remove it from the view.

    Takes the ownership of the \a toolBar, though it is not a parent.

    \sa HbToolBar toolBar()
*/
void HbView::setToolBar(HbToolBar *toolBar)
{
    Q_D(HbView);
    if (d->toolBar != toolBar) {
        if (d->toolBar) {
            d->toolBar->deleteLater();
        }
        d->toolBar = toolBar;
        emit toolBarChanged();
    }
}

/*!
Removes the HbToolBar, which is set to view and returns it.
The ownership of the \a HbToolBar is transferred to the caller.

\note This function is particularly useful if one wants to switch between
different views without deleting previous toolbar.

    \sa setToolBar()
*/
HbToolBar* HbView::takeToolBar()
{
    Q_D(HbView);
    HbToolBar* toolBar = d->toolBar;
    d->toolBar = 0;

    // Reset the ownership
    if (toolBar) {
        toolBar->setParentItem(0);
    }

    return toolBar;
}


/*!
    Returns the dock widget for the view. This function creates and returns an empty dock widget if it does not exist.
    The dockWidgetChanged() signal is not emitted if a new dock widget is created.

    Ownership is not transferred.

    \sa HbDockWidget setDockWidget()
*/
HbDockWidget *HbView::dockWidget() const
{
    Q_D(const HbView);
    if (!d->dockWidget) {
        HbView *that = const_cast<HbView *>(this);
        that->setDockWidget(new HbDockWidget);
    }
    return d->dockWidget;
}

/*!
    Sets the dock widget for the view. Setting the dock widget to 0 will remove it from the view.

    Takes the ownership of the \a dockWidget, though it is not a parent.

    \sa HbDockWidget dockWidget()
*/
void HbView::setDockWidget(HbDockWidget *dockWidget)
{
    Q_D(HbView);
    if (d->dockWidget != dockWidget) {
        if (d->dockWidget) {
            d->dockWidget->deleteLater();
        }
        d->dockWidget = dockWidget;
        emit dockWidgetChanged();
    }
}

/*!
    Returns the widget if set, otherwise \c 0.

    \sa setWidget widget takeWidget
*/
QGraphicsWidget *HbView::widget() const
{
    Q_D(const HbView);
    return d->widget;
}

/*!
    Sets the widget to be \a widget. Setting the widget to 0 will remove it from the view. Already set
    widget is deleted.

    Takes the ownership of the \a widget.

    \sa widget takeWidget
*/
void HbView::setWidget(QGraphicsWidget *widget)
{
    Q_D(HbView);
    if (d->widget != widget) {
        if (widget) {
            if (!d->mLayout) {
                d->mLayout = new HbStackedLayout;
                d->mLayout->setContentsMargins( 0.0, 0.0, 0.0, 0.0 );
                d->mLayout->setMinimumSize( 0.0, 0.0 );
                setLayout( d->mLayout );
            } else {
                Q_ASSERT(d->mLayout->count() == 1);
                d->mLayout->removeAt( 0 );
            }
            d->mLayout->addItem( widget );
        } else {
            if (d->mLayout) {
                // "setLayout( 0 )" deletes the existing layout.
                setLayout( 0 );
                d->mLayout = 0;
            }
        }
        delete d->widget;
        d->widget = widget;            
    }
}

/*!
Removes the QGraphicsWidget, which is set to view and returns it. 
The ownership of the \a widget is transferred to the caller.

\note This function is particularly useful if one wants to switch between
different widgets inside a view without deleting them.

\sa widget() setWidget()
*/

QGraphicsWidget *HbView::takeWidget()
{
    Q_D(HbView);
    QGraphicsWidget *widget = d->widget;
    if ( d->mLayout ) {
        d->mLayout->removeAt( 0 );
        // "setLayout( 0 )" deletes the existing layout.
        setLayout( 0 );
        d->mLayout = 0;
    }
    d->widget = 0;
    // Reset the ownership
    if (widget) {
        widget->setParentItem(0);
    }
    return widget;
}

/*!
    Sets \a items to be visible in this view. Changes will be visible
    instantly if the view is currently active, otherwise next time when
    view is activated. View's visibility flag overrides the similar
    setting in HbMainWindow.
    
    \sa hideItems() setItemVisible() isItemVisible() unsetVisibleItems() visibleItems() isContentFullScreen() setContentFullScreen()
*/
void HbView::showItems(Hb::SceneItems items)
{
    Q_D(HbView);
    d->mVisibleItems |= items;
    d->mVisibleItemsSet = true;

    if (items & Hb::TitleBarItem) {
        d->mViewFlags &= ~HbView::ViewTitleBarHidden;
        d->mTitleBarFlags &= ~HbView::TitleBarHidden;
    }
    if (items & Hb::StatusBarItem) {
        d->mViewFlags &= ~HbView::ViewStatusBarHidden;
    }

    emit visibleItemsChanged();
}

/*!
    Sets \a items to be invisible in this view. Changes will be visible
    instantly if the view is currently active, otherwise next time when
    view is activated. View's visibility flag overrides the similar
    setting in HbMainWindow.
    
    \sa showItems() setItemVisible() isItemVisible() unsetVisibleItems() visibleItems() isContentFullScreen() setContentFullScreen()
*/
void HbView::hideItems(Hb::SceneItems items)
{
    Q_D(HbView);
    d->mVisibleItems &= ~items;
    d->mVisibleItemsSet = true;

    if (items & Hb::TitleBarItem) {
        d->mViewFlags |= HbView::ViewTitleBarHidden;
        d->mTitleBarFlags |= HbView::TitleBarHidden;
    }
    if (items & Hb::StatusBarItem) {
        d->mViewFlags |= HbView::ViewStatusBarHidden;
    }

    emit visibleItemsChanged();
}

/*!
    Returns visible items of this view.

    \sa isItemVisible() setItemVisible() hideItems() showItems() unsetVisibleItems() isContentFullScreen() setContentFullScreen()
    
*/
Hb::SceneItems HbView::visibleItems() const
{
    Q_D(const HbView);
    if (d->mVisibleItemsSet) {
        return d->mVisibleItems;
    } else {
        return HbMainWindowPrivate::d_ptr(mainWindow())->mVisibleItems;
    }
}

/*!
    Returns \c true if \a item is set to be visible.

    \sa setItemVisible() hideItems() showItems() unsetVisibleItems() visibleItems() isContentFullScreen() setContentFullScreen()
    
*/
bool HbView::isItemVisible(Hb::SceneItem item) const
{
    Q_D(const HbView);
    return d->mVisibleItems & item;
}

/*!
    Sets \a item to be \a visible in this view. Changes will be visible
    instantly if the view is currently active, otherwise next time when
    view is activated. View's visibility flag overrides the similar
    setting in HbMainWindow.
    
    \sa isItemVisible() hideItems() showItems() unsetVisibleItems() visibleItems() isContentFullScreen() setContentFullScreen()
*/
void HbView::setItemVisible(Hb::SceneItem item, bool visible)
{
    Q_D(HbView);
    visible ? d->mVisibleItems |= item : d->mVisibleItems &= ~item;
    d->mVisibleItemsSet = true;

    emit visibleItemsChanged();
}

/*!
    \deprecated HbView::unsetVisibleItems()
        is deprecated.

    Resets the view specific definition of visible items and
    uses the HbMainWindow's default visible items. Changes will be visible
    instantly if the view is currently active, otherwise next time when
    view is activated.
    
    \sa setItemVisible() isItemVisible() hideItems() showItems() visibleItems() isContentFullScreen() setContentFullScreen()
*/
void HbView::unsetVisibleItems()
{
    HB_DEPRECATED("HbMainWindow::unsetVisibleItems is deprecated!");
    Q_D(HbView);
    d->mVisibleItemsSet = false;

    emit visibleItemsChanged();
}

/*!
    Returns \c true if view is set to use fullscreen.

    \sa setContentFullScreen() setItemVisible() isItemVisible() hideItems() showItems() visibleItems()
*/
bool HbView::isContentFullScreen() const
{
    Q_D(const HbView);
    return d->mFullscreen;
}

/*!
    Sets the view to use full content area for drawing. Decorators like
    signal bar, title pane etc can be shown on top of view content area.
    Changes will be visible instantly if the view is currently active, 
    otherwise next time when view is activated.

    \sa isContentFullScreen() setItemVisible() isItemVisible() hideItems() showItems() visibleItems()
*/
void HbView::setContentFullScreen(bool enable)
{
    Q_D(HbView);
    if (d->mFullscreen != enable) {
        d->mFullscreen = enable;
        emit contentFullScreenChanged();
    }
}

/*!
    \deprecated HbView::titleBarFlags()
        is deprecated. Use HbView::viewFlags() instead.

    Returns titlebar flags bit vector.
    It consists of bits specified in HbTitleBarFlag.

    \sa setTitleBarFlags()
*/
HbView::HbTitleBarFlags HbView::titleBarFlags() const
{
    Q_D(const HbView);
    int statusBarFlagMask = ViewStatusBarHidden | ViewStatusBarTransparent | ViewStatusBarFloating;
    int flags(d->mViewFlags & ~statusBarFlagMask);
    return HbView::HbTitleBarFlags(flags);
}

/*!
    Returns view flags bit vector.
    It consists of bits specified in HbViewFlag.

    \sa setTitleBarFlags()
*/
HbView::HbViewFlags HbView::viewFlags() const
{
    Q_D(const HbView);
    return d->mViewFlags;
}

/*!
    \deprecated HbView::setTitleBarFlags(HbView::HbTitleBarFlags flags)
        is deprecated. Use HbView::setViewFlags(HbView::HbViewFlags flags) instead.

    Sets the titlebar flags bit vector. It consists of bits specified
    in HbTitleBarFlag. These flags can be set before the view is
    added to the main window or while the view is active.

    \sa titleBarFlags()
*/
void HbView::setTitleBarFlags(HbView::HbTitleBarFlags flags)
{
    const int statusBarFlagMask = ViewStatusBarHidden | ViewStatusBarTransparent | ViewStatusBarFloating;

    Q_D(HbView);
    setViewFlags(HbView::HbViewFlags((d->mViewFlags & statusBarFlagMask) | flags));
}

/*!
    Sets the view flags bit vector. It consists of bits specified
    in HbViewFlag. These flags can be set before the view is
    added to the main window or while the view is active.

    \sa viewFlags()
*/
void HbView::setViewFlags(HbView::HbViewFlags flags)
{
    if (!mainWindow()) {
        return;
    }
    Q_D(HbView);
    HbView::HbViewFlags originalFlags(d->mViewFlags);
    d->mViewFlags = flags;
    if (mainWindow()) {
        bool statusBarAnimating = false;
        HbStatusBar *statusBar = HbMainWindowPrivate::d_ptr(mainWindow())->mStatusBar;
        if ((d->mViewFlags & HbView::ViewStatusBarHidden) && statusBar->isVisible()) {
             HbEffect::start(statusBar, "statusbar", "disappear", this, "statusBarEffectFinished"); 
             statusBarAnimating = true;
        } else if (!(d->mViewFlags & HbView::ViewStatusBarHidden) && !statusBar->isVisible()) {
             HbEffect::start(statusBar, "statusbar", "appear", this, "statusBarEffectFinished");
             statusBarAnimating = true;
        }
        bool titleBarAnimating = false;
        HbTitleBar *titleBar = HbMainWindowPrivate::d_ptr(mainWindow())->mTitleBar;
        if ((d->mViewFlags & HbView::ViewTitleBarHidden) && titleBar->isVisible()) {
             HbEffect::start(titleBar, "titleBar", "disappear", this, "titleBarEffectFinished");
             titleBarAnimating = true;
        } else if (!(d->mViewFlags & HbView::ViewTitleBarHidden) && !titleBar->isVisible()) {
             HbEffect::start(titleBar, "titleBar", "appear", this, "titleBarEffectFinished");
             titleBarAnimating = true;
        }
        if (!statusBarAnimating) {
            statusBar->setVisible(!(d->mViewFlags & HbView::ViewStatusBarHidden));
            statusBar->propertiesChanged();
        }
        if (!titleBarAnimating) {
            titleBar->setVisible(!(d->mViewFlags & HbView::ViewTitleBarHidden));
            if (d->mNavigationActionSet) {
                titleBar->setNavigationAction(d->mNavigationAction);
            } else {
                titleBar->setDefaultNavigationAction();
            }
            titleBar->propertiesChanged();
        }
       
        // Repolish the screen if needed
        int visibilityFlags = HbView::ViewTitleBarMinimized | HbView::ViewTitleBarFloating 
            | HbView::ViewTitleBarMinimizable | HbView::ViewStatusBarHidden | HbView::ViewStatusBarFloating;
        if ((d->mViewFlags & visibilityFlags) != (originalFlags & visibilityFlags)) {
            HbMainWindowPrivate::d_ptr(mainWindow())->mClippingItem->decoratorVisibilityChanged();
        }
    }
}

/*!
    Changes titlebar's visibility with pre-defined effect.
*/
void HbView::setTitleBarVisible(bool visible)
{
    Q_D(HbView);
    if (visible) {
        setViewFlags(d->mViewFlags &~ HbView::ViewTitleBarHidden);
        d->mVisibleItems |= Hb::TitleBarItem;
        d->mVisibleItemsSet = true;
    } else {
        setViewFlags(d->mViewFlags | HbView::ViewTitleBarHidden);
        d->mVisibleItems &= ~Hb::TitleBarItem;
        d->mVisibleItemsSet = true;
    }  
}

/*!
    Changes statusbar's visibility with pre-defined effect.
*/
void HbView::setStatusBarVisible(bool visible)
{
    Q_D(HbView);
    if (visible) {
        setViewFlags(d->mViewFlags &~ HbView::ViewStatusBarHidden);
        d->mVisibleItems |= Hb::StatusBarItem;
        d->mVisibleItemsSet = true;
    } else {
        setViewFlags(d->mViewFlags | HbView::ViewStatusBarHidden);
        d->mVisibleItems &= ~Hb::StatusBarItem;
        d->mVisibleItemsSet = true;
    }
}

/*!
    \overload

    The function adds \a action to preferred container's list of actions.  The
    ownership of \a action is not transferred.  The preferred container
    specifies the action container (options menu, toolbar) where the action is
    placed. If the container is the toolbar and the maximum toolbar button count
    is exceeded then the action may be moved to the options menu (if there are
    enough more prioritized actions).
*/
void HbView::addAction(HbAction *action, ActionContainer preferredActionContainer)
{
    Q_D(HbView);
    d->preferredActionContainer = preferredActionContainer;
    addAction(action);
}

/*!
    \reimp
 */
bool HbView::event(QEvent *event)
{
    Q_D(HbView);
    if (event->type() == QEvent::ActionRemoved) {
        // Create action manager if needed
        if (!d->actionManager) {
            d->actionManager = new HbViewActionManager(this);
        }
        QActionEvent *actionEvent = static_cast<QActionEvent *>(event);
        HbAction *hbAction = qobject_cast<HbAction *>(actionEvent->action());
        // Forward command to action manager
        d->actionManager->removeAction(hbAction); 
        return true;
    }
    else if (event->type() == QEvent::ActionAdded) {
        // Create action manager if needed
        if (!d->actionManager) {
            d->actionManager = new HbViewActionManager(this);
        }
        QActionEvent *actionEvent = static_cast<QActionEvent *>(event);
        HbAction *hbAction = qobject_cast<HbAction *>(actionEvent->action());
        // Forward command to action manager
        d->actionManager->addAction(hbAction, d->preferredActionContainer); 
        // Clear the variable after used
        d->preferredActionContainer = HbView::NotSpecified;
        return true;
    }
    return HbWidget::event(event);
}

/*!
Removes the HbMenu, which is set to view and returns it. 
The ownership of the \a HbMenu is transferred to the caller.

\note This function is particularly useful if one wants to switch between
different views without deleting previous menu.

    \sa setMenu()
*/
HbMenu* HbView::takeMenu()
{
    Q_D(HbView);
    HbMenu* menu = d->menu;
    d->menu = 0;

    // Reset the ownership
    if (menu) {
        menu->setParentItem(0);
    }

    return menu;
}

/*
    Handles effect finished event for title bar animation    
 */
void HbView::titleBarEffectFinished(const HbEffect::EffectStatus &status)
{   
    if (status.reason == Hb::EffectFinished || status.reason == Hb::EffectCancelled
        || status.reason == Hb::EffectNotStarted) {
        HbMainWindowPrivate::d_ptr(mainWindow())->mTitleBar->propertiesChanged();
        HbMainWindowPrivate::d_ptr(mainWindow())->mClippingItem->decoratorVisibilityChanged();
    }
    if (mainWindow()) {
        HbTitleBar *titleBar = HbMainWindowPrivate::d_ptr(mainWindow())->mTitleBar;
        if (titleBar) {
            if (status.effectEvent == "disappear") {
                titleBar->setVisible(false);
            } else {
                titleBar->setVisible(true);
            }
        }
    }
}

/*
    Handles effect finished event for title bar animation    
 */
void HbView::statusBarEffectFinished(const HbEffect::EffectStatus &status)
{   
    HbStatusBar *statusBar = HbMainWindowPrivate::d_ptr(mainWindow())->mStatusBar;
    if (!statusBar) {
        return;
    }

    if (status.reason == Hb::EffectFinished || status.reason == Hb::EffectCancelled
        || status.reason == Hb::EffectNotStarted) {
        statusBar->propertiesChanged();
        HbMainWindowPrivate::d_ptr(mainWindow())->mClippingItem->decoratorVisibilityChanged();
    }
    if (mainWindow()) {    
        if (status.effectEvent == "disappear") {
            statusBar->setVisible(false);
        } else {
            statusBar->setVisible(true);
        }
    }
}

/*!
  Returns the currently set navigation action or 0 if there is none.

  If setNavigationAction() was not called before for this view at all then the
  default action will be returned.
  
  \sa setNavigationAction()
 */
HbAction *HbView::navigationAction() const
{
    Q_D(const HbView);
    if (d->mNavigationActionSet) {
        return d->mNavigationAction;
    } else {
        HbMainWindow *mw = mainWindow();
        if (!mw) {
            QList<HbMainWindow *> mainWindows = hbInstance->allMainWindows();
            if (!mainWindows.isEmpty()) {
                mw = mainWindows.at(0);
            }
        }
        return mw ? HbMainWindowPrivate::d_ptr(mw)->mTitleBar->navigationAction() : 0;
    }
}

/*!
  Sets the navigation action, that is, the action that is associated with the
  button that is typically located in the top-right corner. By default the
  action is the "quit" action.

  \a action can also be 0 in which case the previous navigation action is
  unset and no new one will be set.

  Ownership of \a action is not taken.

  \sa navigationAction()
 */
void HbView::setNavigationAction(HbAction *action)
{
    Q_D(HbView);
    d->mNavigationAction = action;
    d->mNavigationActionSet = true;
    HbMainWindow *mw = mainWindow();
    if (isVisible() && mw && mw->currentView() == this) {
        HbMainWindowPrivate::d_ptr(mw)->mTitleBar->setNavigationAction(action);
    }
}

// end of file
