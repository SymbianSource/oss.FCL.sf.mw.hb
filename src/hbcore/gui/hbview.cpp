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
#include "hbscreen_p.h"
#include "hbviewactionmanager_p.h"
#include "hbglobal_p.h"
#include <QPointer>

/*!
    @stable
    @hbcore
	\class HbView
	\brief The HbView class defines one screen of user interface content.
    
    The screen area of a mobile device is small, so an application's user interface is often composed of a set of separate screens of content or "views". Each view can contain a number of widgets. A view can also contain a title, an icon (not used at present), a menu and a toolbar. These additional components are called decorators and together they make up the "chrome" of the interface.
    
    An application will have a number of views to present different kinds of information to the user. You can add multiple views to the \link HbMainWindow main window \endlink of your application and then manage them through the main window object. When a view is activated, the main window handles the updating of the view contents, i.e. the title, icon, menu and toolbar. The view object itself handles the visibility of the toolbar, but the main window will repaint it whenever the view is activated.

    \image html hbview_wireframes.png "Wireframes of two different views in a messaging application"

   
    \section _usecases_hbview Using the HbView class
    In general, the actual widgets, menu and toolbar that make up the content of a view will be designed with a UI layout tool. This means you do not need to use code to place each widget into the layout but instead load the layout from a file with HbDocumentLoader. Once the view is populated with widgets, you can manipulate them and connect their signals to methods in your application.

    See the HbMainWindow class for information about switching between different views.

    \subsection _uc_001_hbview Hiding and showing the chrome 
    The chrome, (i.e. title, menu and toolbar of the view) can be hidden to allow your application to fill the whole screen. Also, individual components of the chrome can be hidden if desired.

    \code 
    // Hide all the chrome items
    myView.setContentFullScreen(true)

    // Show all the chrome items
    myView.setContentFullScreen(false)

    // Hide the toolbar
    myView.hideItems(Hb::SceneItem::ToolBarItem)

    // Show the toolbar
    myView.showItems(Hb::SceneItem::ToolBarItem)
    \endcode

    \subsection _uc_002_hbview Creating a simple view
    This code snippet shows the most basic way to create an application with a single view.

    \code
    #include <hbapplication.h>
    #include <hbmainwindow.h>
    #include <hbview.h>

    int main(int argc, char *argv[])
    {
        HbApplication app(argc, argv);
        HbMainWindow window;

        HbView *view = new HbView();
        view->setTitle("My View");

        window.addView(view);

        window.show();
        return app.exec();
    }
    \endcode

    \section _methodgroups_hbview Methods and properties grouped
    \li \b Chrome: setViewFlags(), showItems(), hideItems(), visibleItems(), isItemVisible(), setItemVisible(), unsetVisibleItems(),visibleItemsChanged() [signal], setContentFullScreen(), contentFullScreenChanged() [signal], isContentFullScreen(), contentFullScreen [property]
    \li \b Title: title(), setTitle(), titleChanged() [signal], title [property]
    \li \b Icon: icon(), setIcon(), iconChanged() [signal], icon [property]
    \li \b Menu: menu(), setMenu(), takeMenu()
    \li \b Toolbar: toolBar(), setToolBar(), toolBarChanged() [signal], takeToolBar()
    \li \b Actions: addAction(), setNavigationAction()
    \li \b Event: event()
    \li \b TitleBar: setTitleBarVisible()
    \li \b StatusBar: setStatusBarVisible()
    \li \b DockWidget: dockWidget(), setDockWidget(), dockWidgetChanged() [signal]
    \li \b Widget: widget(), setWidget(), takeWidget()

    \sa HbApplication, HbMainWindow, HbDocumentLoader, HbMenu, HbToolBar, HbDockWidget
 */
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
    When this flag is set, there will be an indication in the titlebar showing the possibility to minimize the titlebar, e.g. swipe it off the screen.
*/
/*!
    \var HbView::ViewTitleBarMinimized
    Show the title bar in minimized state.
*/
/*!
    \var HbView::ViewTitleBarHidden
    Do not show the title bar at all. The title bar handle is not shown and it is not possible to maximize the title bar.
*/
/*!
    \var HbView::ViewTitleBarTransparent
    Show normal style title bar but make it transparent. This flag is normally used in combination with TitleBarFloating flag.
*/
/*!
    \var HbView::ViewTitleBarFloating
    Show the title bar floating on top of the underlying content. Setting this flag also make the application area start from the top of the screen. This flag is normally used in combination with TitleBarTransparent flag.
*/
/*!
    \var HbView::ViewStatusBarHidden
    Do not show the statusbar at all. This flag is normally used in combination with the ViewTitleBarHidden flag.
*/
/*!
    \var HbView::ViewStatusBarTransparent
    Show the statusbar with normal content but transparent. This flag is normally used in combination with ViewStatusBarFloating flag.
*/
/*!
    \var HbView::ViewStatusBarFloating
    Show the statusbar on top of the underlying content. Setting this flag also changes the application area to start from the top of the screen. This flag is normally used in combination with the ViewStatusBarTransparent flag.
*/

/*!
    \enum HbView::ActionContainer
    Defines the default container when you add actions to a view.
    
    \sa navigationAction()
*/
/*!
    \var HbView::NotSpecified
    Indicate that no particular action container is preferred.
*/
/*!
    \var HbView::OptionsMenu
    Indicate that the options menu is the preferred location for an action.
*/
/*!
    \var HbView::ToolBar
    Indicate that the tool bar is the preferred location for an action.
*/

/*!
    \fn void HbView::titleChanged(const QString &title)

    This signal is emitted when the title is replaced by a different title.
    
    \sa setTitle()
 */

/*!
    \fn void HbView::iconChanged(const HbIcon &icon)

    This signal is emitted when the icon is replaced by a different icon.
    
    \sa setIcon()
 */

/*!
    \fn void HbView::toolBarChanged()

    This signal is emitted when toolbar is replaced by a different toolbar.
    
    \sa setToolBar()
 */

/*!
   \fn void HbView::visibleItemsChanged()
 
   This signal is emitted when items in the chrome (e.g. toolbar, menu) are made visible or hidden.
   
   \sa visibleItems()
*/

/*!
   \fn void HbView::contentFullScreenChanged()
 
   This signal is emitted when the view is set to occupy the whole screen.
   
   \sa setContentFullScreen()
*/

/*!
   \fn void HbView::dockWidgetChanged()
 
   This signal is emitted when the dock widget is replaced by a different dock widget.
   
   \sa setDockWidget()
*/

/*!
    \fn int HbView::type() const
 */

/*!
    \internal
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
      mViewFlags(HbView::ViewFlagNone),
      actionManager(0),
      preferredActionContainer(HbView::NotSpecified),
      mNavigationAction(0),
      mNavigationActionSet(false)
{
}

/*!
    \internal
 */
HbViewPrivate::~HbViewPrivate()
{
}

/*!
    Constructs a view with the given \a parent.
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
    Destructor.
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
  Returns the title of the view. If no title has been defined, the return value is a null string.

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
    Returns the icon associated with the view.
    The default value is a null icon.

    \sa setIcon(), HbIcon::isNull()
 */
HbIcon HbView::icon() const
{
    Q_D(const HbView);
    return d->icon;
}

/*!
    Sets the icon associated with the view.

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
    Returns the menu for the view. This function creates and returns an empty menu if one does not already exist.

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
    Sets the menu for the view. Setting the menu to 0 will remove any existing menu from the view.

    The view takes ownership of the \a menu.

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
        // Titlebar must be informed
        if (mainWindow()) {
            HbMainWindowPrivate::d_ptr(mainWindow())->mTitleBar->propertiesChanged();
        }
    }
}

/*!
    Returns the toolbar for the view. If the view does not already have a toolbar, an 
	empty toolbar is created and returned to the caller but the toolBarChanged() 
	signal is not emitted.

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
    Sets the toolbar for the view. Setting the toolbar to 0 will remove it from the view.

    The view takes ownership of the toolbar, but the view is not set as the parent.

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
Removes the toolbar from the view and returns it to the caller.
Ownership of the toolbar is transferred to the caller.

\note This function is particularly useful if you want to switch to a 
different view and keep the same toolbar.

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
    Returns the dock widget belonging to the view. If the view does not have a dock widget, an empty dock widget is created and returned.
    The dockWidgetChanged() signal is not emitted when a new dock widget is created by this method.

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
    Sets the dock widget for the view. Setting the dock widget to \c 0 will remove it from the view.

    This does take ownership of the \a dockWidget, but does not become the parent.

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
    Returns the widget that makes up the view. If the widget has not been set, this method will return 0.

    \sa setWidget takeWidget
*/
QGraphicsWidget *HbView::widget() const
{
    Q_D(const HbView);
    return d->widget;
}

/*!
    Sets the widget that makes up the view. Setting the \a widget value to 0 will delete the current widget.

    The view takes the ownership of the \a widget.

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
Removes the widget that makes up the view and returns the widget to the caller.
Ownership of the widget is transferred to the caller.

\note This function is particularly useful if you want to use 
different widgets in a view without deleting them.

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
    Makes the given scene items visible in this view. Changes are visible
    instantly if the view is active, otherwise they will be shown the next time the
    view is activated.
    
    The flag values in \a items override the corresponding settings in HbMainWindow.
    
    \sa hideItems() setItemVisible() isItemVisible() unsetVisibleItems() visibleItems() isContentFullScreen() setContentFullScreen()
*/
void HbView::showItems(Hb::SceneItems items)
{
    Q_D(HbView);
    d->mVisibleItems |= items;
    d->mVisibleItemsSet = true;

    if (items & Hb::TitleBarItem) {
        d->mViewFlags &= ~HbView::ViewTitleBarHidden;
    }
    if (items & Hb::StatusBarItem) {
        d->mViewFlags &= ~HbView::ViewStatusBarHidden;
    }

    emit visibleItemsChanged();
}

/*!
    Hides the given scene items in this view. Changes are visible
    instantly if the view is active, otherwise they will be shown the next time the
    view is activated.

    The flag values in \a items override the corresponding settings in HbMainWindow.
    
        
    \sa showItems() setItemVisible() isItemVisible() unsetVisibleItems() visibleItems() isContentFullScreen() setContentFullScreen()
*/
void HbView::hideItems(Hb::SceneItems items)
{
    Q_D(HbView);
    d->mVisibleItems &= ~items;
    d->mVisibleItemsSet = true;

    if (items & Hb::TitleBarItem) {
        d->mViewFlags |= HbView::ViewTitleBarHidden;
    }
    if (items & Hb::StatusBarItem) {
        d->mViewFlags |= HbView::ViewStatusBarHidden;
    }

    emit visibleItemsChanged();
}

/*!
    Returns the scene items that are visible in this view.

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
    Returns \c true if \a item is set to be visible, otherwise returns \c false.

    \sa setItemVisible() hideItems() showItems() unsetVisibleItems() visibleItems() isContentFullScreen() setContentFullScreen()
    
*/
bool HbView::isItemVisible(Hb::SceneItem item) const
{
    Q_D(const HbView);
    return d->mVisibleItems & item;
}

/*!
    Shows or hides the given scene item for the view. If \a visible is \c true,
    then the given \a item is shown. If \a visible is \c false, then
    the given \a item is hidden.
	Changes are visible instantly if the view is active, otherwise they will be shown the next time the
    view is activated.    

    This overrides the corresponding scene item settings in HbMainWindow.
           
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
    Returns \c true if view is set to use fullscreen.

    \sa setContentFullScreen() setItemVisible() isItemVisible() hideItems() showItems() visibleItems()
*/
bool HbView::isContentFullScreen() const
{
    Q_D(const HbView);
    return d->mFullscreen;
}

/*!
    Makes the view content fill the whole screen area. The decorators that make up the chrome (such as 
    signal bar, title pane etc.) can still be shown on top of view content.
	Changes are visible instantly if the view is active, otherwise they will
	be shown the next time the view is activated.    

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
    Returns the view flags bit vector.
    This consists of values specified in HbViewFlag.

    \sa setViewFlags()
*/
HbView::HbViewFlags HbView::viewFlags() const
{
    Q_D(const HbView);
    return d->mViewFlags;
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

        // Statusbar-animation
        bool statusBarAnimating = false;
        HbStatusBar *statusBar = HbMainWindowPrivate::d_ptr(mainWindow())->mStatusBar;
        if ((d->mViewFlags & HbView::ViewStatusBarHidden) && !(originalFlags & HbView::ViewStatusBarHidden)) {
#ifdef HB_EFFECTS
            HbEffect::start(statusBar, "statusbar", "disappear", this, "statusBarEffectFinished");
#endif // HB_EFFECTS
            statusBarAnimating = true;
        } else if (!(d->mViewFlags & HbView::ViewStatusBarHidden) && (originalFlags & HbView::ViewStatusBarHidden)) {
#ifdef HB_EFFECTS
            HbEffect::start(statusBar, "statusbar", "appear", this, "statusBarEffectFinished");
#endif // HB_EFFECTS
            statusBarAnimating = true;
        }

        // Titlebar-animation
        bool titleBarAnimating = false;
        HbTitleBar *titleBar = HbMainWindowPrivate::d_ptr(mainWindow())->mTitleBar;
        if ((d->mViewFlags & HbView::ViewTitleBarHidden) && !(originalFlags & HbView::ViewTitleBarHidden)) {
#ifdef HB_EFFECTS
            HbEffect::start(titleBar, "titlebar", "disappear", this, "titleBarEffectFinished");
#endif // HB_EFFECTS
            titleBarAnimating = true;
        } else if (!(d->mViewFlags & HbView::ViewTitleBarHidden) && (originalFlags & HbView::ViewTitleBarHidden)) {
#ifdef HB_EFFECTS
            HbEffect::start(titleBar, "titlebar", "appear", this, "titleBarEffectFinished");
#endif // HB_EFFECTS
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
        if (d->toolBar) {
            d->toolBar->updatePrimitives();
        }

        // Statusbar-visibility
        if ( statusBar->isVisible() ){
            d->mVisibleItems |= Hb::StatusBarItem;
            d->mVisibleItemsSet = true;
        } else {
            d->mVisibleItems &= ~Hb::StatusBarItem;
            d->mVisibleItemsSet = true;
        }

        // Titlebar-visibility
        if (titleBar->isVisible()){
            d->mVisibleItems |= Hb::TitleBarItem;
            d->mVisibleItemsSet = true;
        }else{
            d->mVisibleItems &= ~Hb::TitleBarItem;
            d->mVisibleItemsSet = true;
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
    Changes the titlebar's visibility and uses the pre-defined effect.

\note This is a convenience function which uses setViewFlags() to (un)set
the HbView::ViewTitleBarHidden flag.

\sa setViewFlags()
*/
void HbView::setTitleBarVisible(bool visible)
{
    Q_D(HbView);
    if (visible) {
        setViewFlags(d->mViewFlags &~ HbView::ViewTitleBarHidden);
    } else {
        setViewFlags(d->mViewFlags | HbView::ViewTitleBarHidden);
    }  
}

/*!
    Changes the statusbar's visibility and uses the pre-defined effect.

\note This is a convenience function which uses setViewFlags() to (un)set
the HbView::ViewStatusBarHidden flag.

\sa setViewFlags()
*/
void HbView::setStatusBarVisible(bool visible)
{
    Q_D(HbView);
    if (visible) {
        setViewFlags(d->mViewFlags &~ HbView::ViewStatusBarHidden);
    } else {
        setViewFlags(d->mViewFlags | HbView::ViewStatusBarHidden);
    }
}

/*!
    \overload


    This adds the \a action to the list of actions in the view. You specify a preferred container 
    in the UI where you would like the action to be added, e.g. the options menu or the toolbar. 
    However, if you choose the toolbar as the preferred container but no more buttons can fit in the 
    the toolbar, then the action might be moved to the options menu or it might displace another item in the toolbar. 
    The result depends on the prioties of the actions already in the toolbar compared with the priority of 
    the action you are adding.
    
    Ownership of the \a action is not transferred.
*/
void HbView::addAction(HbAction *action, ActionContainer preferredActionContainer)
{
    Q_D(HbView);
    d->preferredActionContainer = preferredActionContainer;
    addAction(action);
}

/*!
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
Removes the menu from the view and returns it to the caller.
Ownership of the menu is transferred to the caller.

\note This function is particularly useful if you want to switch between
different views without deleting the menu.

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

    // Titlebar must be informed
    if (mainWindow()) {
        HbMainWindowPrivate::d_ptr(mainWindow())->mTitleBar->propertiesChanged();
    }

    return menu;
}

#ifdef HB_EFFECTS
/*
    Handles effect finished event for title bar animation    
 */
void HbView::titleBarEffectFinished(const HbEffect::EffectStatus &status)
{   
    Q_D(HbView);

    HbMainWindow *mw = mainWindow();

    if (d->mNavigationActionSet) {
        HbMainWindowPrivate::d_ptr(mw)->mTitleBar->setNavigationAction(d->mNavigationAction);
    } else {
        HbMainWindowPrivate::d_ptr(mw)->mTitleBar->setDefaultNavigationAction();
    }
    HbMainWindowPrivate::d_ptr(mw)->mTitleBar->propertiesChanged();
    HbMainWindowPrivate::d_ptr(mw)->mClippingItem->decoratorVisibilityChanged();

    if (mw) {
        HbTitleBar *titleBar = HbMainWindowPrivate::d_ptr(mw)->mTitleBar;
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
    Handles the effect finished event for title bar animation    
 */
void HbView::statusBarEffectFinished(const HbEffect::EffectStatus &status)
{   
    HbMainWindow *mw = mainWindow();

    HbStatusBar *statusBar = HbMainWindowPrivate::d_ptr(mw)->mStatusBar;
    if (!statusBar) {
        return;
    }

    statusBar->propertiesChanged();
    HbMainWindowPrivate::d_ptr(mw)->mClippingItem->decoratorVisibilityChanged();

    if (mw) {    
        if (status.effectEvent == "disappear") {
            statusBar->setVisible(false);
        } else {
            statusBar->setVisible(true);
        }
    }
}
#endif // HB_EFFECTS

/*!
  Returns the current navigation action. If no navigation action has been set, then
  0 is returned.

  If setNavigationAction() has not been called for this view, then the
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
  Sets the navigation action, i.e., the action associated with the
  button that is typically located in the top-right corner. By default the
  action is the "quit" action.

  If you set the \a action to 0, the previous navigation action is
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
