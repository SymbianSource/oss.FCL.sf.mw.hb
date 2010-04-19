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

#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QGraphicsItem>
#include <QGraphicsScene>

#include "hbaction.h"
#include "hbbackgrounditem_p.h"
#include "hbdeviceprofile_p.h"
#include "hbindicatorgroup_p.h"
#include "hbinstance.h"
#include "hbinstance_p.h"
#include "hbgraphicsscene.h"
#include "hbmainwindow.h"
#include "hbmainwindow_p.h"
#include "hbnamespace.h"
#include "hbnamespace_p.h"
#include "hbsoftkey_p.h"
#include "hbsoftkeygroup_p.h"
#include "hbtitlebar_p.h"
#include "hbstatusbar_p.h"
#include "hbstyle.h"
#include "hbtitlepane_p.h"
#include "hbtoolbar.h"
#include "hbtoolbar_p.h"
#include "hbview.h"
#include "hblayoutdirectionnotifier_p.h"
#ifdef HB_EFFECTS
#include "hbeffect.h"
#include "hbeffectinternal_p.h"
#endif
#include "hbthetestutility_p.h"
#include "hbglobal_p.h"
#include "hbevent.h"
#include "hbsplashscreen_p.h"
#include "hbcontentwidget_p.h"
#include "hbscreen_p.h"
#include "hbmainwindoworientation_p.h"
#include "hbfeaturemanager_p.h"

#ifdef Q_OS_SYMBIAN
#include <coecntrl.h>
#include "hbnativewindow_sym_p.h"
#endif

/*!
    @beta
    @hbcore
    \class HbMainWindow

    \brief HbMainWindow is a GraphicsView instance, which can be initialized with S60
    style decorators.

    HbMainWindow is a specialized version of QGraphicsView. It represents the
    main window of an application. HbMainWindow constructs also HbGraphicsScene,
    which has separate layers where user can place graphics items. HbMainWindow
    constructs three layers which are drawn on top of each other: background
    layer, content layer and decorator layer. The level of depth of graphics
    items is set at the items creation time by setting its parent. To place an
    item on the content level use currentView() function to get content layer.

    Do not call QGraphicsView::setBackgroundBrush() on a HbMainWindow, prefer
    using QGraphicsScene::setBackgroundBrush() instead. By setting the
    background brush for the graphics view certain functionality provided by
    HbGraphicsScene's background drawing code would be lost. Also, the scene's
    background brush is only effective (visible) when the
    Hb::WindowFlagNoBackground flag is set for the HbMainWindow. Normally the
    standard background item fills the entire screen so the background is not
    really visible.

    HbMainWindow has a signalling mechanism for helping control the
    application's view construction. viewReady()-signal is emitted
    when view's internal construction is completed and basic parts of
    view are already drawn. Same signal is also emitted when current
    view is switched. This helps applications to split the view
    construction to reasonable tasks.  For example the lower priority
    tasks like opening network connection or preparing other currently
    hidden application views can happen on background when first view
    is already drawn.

    Example of simple Hb application constructing HbMainWindow:

    \include mainwindow1/main.cpp

    Example of adding stock widgets to HbMainWindow:

    \include mainwindow2/main.cpp

    Following example has a linear layout set to a widget, which is then added
    to the HbMainWindow. Layout contains two widgets HbLabel and HbPushButton.

    \include mainwindow3/main.cpp

    More examples can be found from HbView class.

    \sa HbView, Hb::ZValues, Hb::WindowFlags
*/

/*!
    \fn void HbMainWindow::aboutToChangeOrientation()

    This signal is emitted when the orientation change starts.
 */

/*!
    \fn void HbMainWindow::aboutToChangeOrientation(Qt::Orientation newOrientation, bool animated)

    This signal is emitted when the orientation change starts. This version of aboutToChangeOrientation also includes the information
    about the new orientation and whether the orientation change is animated or not. 
    This can be used for example by widgets who want to participate in the orientation change animation.
 */

/*!
    \fn void HbMainWindow::orientationChanged()

    This signal is emitted when animation related to orientation change has completed.
 */

/*!
    \deprecated HbMainWindow::currentViewIndexChanged(int)
        is deprecated.

    \fn void HbMainWindow::currentViewIndexChanged(int index)

    This signal is emitted when the current view index changes.
 */

/*!
    \fn void HbMainWindow::currentViewChanged(HbView *view)

    This signal is emitted when the current view changes.
 */

/*!
    \fn void HbMainWindow::viewReady()

    This signal is emitted first time when window content is drawn on screen.
    It will only be emitted again when current view is changed and drawn on screen.

    If applicationlaunch or current view is switched with effect, signal is
    emitted after the effect has completed.

    Application developers may connect to this signal to do lower priority construction tasks. When
    thinking about the tasks, it's important to keep the UI responsive all the time.
 */

/*!
  \class HbRootItem

  \brief The parent of all graphics items (including the clipping item (HbScreen)
  and the background item). Basically it is a HbWidget with polishing disabled
  (so we won't waste time with that).

  \internal
 */

class HbRootItem : public HbWidget
{
public:
    explicit  HbRootItem( QGraphicsItem *parent = 0 );
    ~HbRootItem() {}
private:
    bool event(QEvent *event);
};

/*!
    Constructs an HbMainWindow object with \a parent.
    
    \a windowFlags can be used for specifying special functionality to HbMainWindow.
    
    \sa Hb::WindowFlag
*/
HbMainWindow::HbMainWindow(QWidget *parent, Hb::WindowFlags windowFlags):
        QGraphicsView(parent), d_ptr(new HbMainWindowPrivate)
{
    Q_D(HbMainWindow);
    d->q_ptr = this;

    // No need for any default (e.g. blank white) background for this window.
    // Setting this attribute is mandatory in order to have a flicker-less
    // startup (both with and without splash screen).
    setAttribute(Qt::WA_NoSystemBackground);

    // Continue with basic initialization. Note: Prefer doing everything that is
    // not absolutely compulsory in _q_delayedConstruction instead.

    d->init();

    if (windowFlags & Hb::WindowFlagTransparent) {
        setWindowFlags(Qt::FramelessWindowHint);
        setAttribute(Qt::WA_TranslucentBackground);
    }

    HbDeviceProfile profile(HbDeviceProfile::current());
    QSize pSize(profile.logicalSize());

    //Determine the default orientation width < height -> portrait
    if (pSize.width() < pSize.height()) {
        d->mDefaultOrientation = Qt::Vertical;
    } else {
        d->mDefaultOrientation = Qt::Horizontal;
    }

    // Check if there is special needs from application
    if (windowFlags & Hb::WindowFlagFixedVertical) {
        HbMainWindowOrientation::instance()->setFixedOrientation(Qt::Vertical);
        d->mOrientation = Qt::Vertical;
        d->mAutomaticOrientationSwitch = false;
    } else if (windowFlags & Hb::WindowFlagFixedHorizontal) {
        HbMainWindowOrientation::instance()->setFixedOrientation(Qt::Horizontal);
        d->mOrientation = Qt::Horizontal;
        d->mAutomaticOrientationSwitch = false;
    } else {
        d->mOrientation = HbMainWindowOrientation::instance()->sensorOrientation();
        d->mAutomaticOrientationSwitch = true;
    }

#if defined(Q_WS_S60) || defined(HB_Q_WS_MAEMO)
    setWindowState(Qt::WindowFullScreen);
#endif//Q_WS_S60
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);

    // create scene and style
    d->mScene = new HbGraphicsScene(this);

    // workaround for problems with BSP tree implementation in Qt
    d->mScene->setItemIndexMethod(QGraphicsScene::NoIndex);

    d->mScene->setSceneRect(0, 0, pSize.width(), pSize.height()); 
    setScene(d->mScene);

    // add root item
    d->mRootItem = new HbRootItem;
    d->mRootItem->setZValue(HbPrivate::RootItemZValue);
    d->mScene->addItem(d->mRootItem);

    // add background item
    if (!(windowFlags & Hb::WindowFlagNoBackground)) {
        d->addBackgroundItem();
    }

    // add clipping item
    d->mClippingItem = new HbScreen;
    d->mClippingItem->setParentItem(d->mRootItem);
    d->mClippingItem->setZValue(HbPrivate::ContentZValue);

    // create content (stacked) widget
    d->mViewStackWidget = new HbContentWidget(this, d->mClippingItem);
    d->mEffectItem = d->mViewStackWidget;
    d->mClippingItem->setStackWidget(d->mViewStackWidget);
    connect(d->mViewStackWidget, SIGNAL(currentChanged(int)),
            this, SLOT(_q_viewChanged(int)));
    connect(d->mViewStackWidget, SIGNAL(widgetRemoved(QGraphicsWidget*)),
            this, SLOT(_q_viewRemoved(QGraphicsWidget*)));

    // create Titlebar (container for indicators, titlepane and secondary softkey
    d->mTitleBar = new HbTitleBar(this, d->mClippingItem);
    d->mTitleBar->setZValue(HbPrivate::TitleBarZValue);

    // create StatusBar (container for clock, indicators, and battery and signal icons)
    d->mStatusBar = new HbStatusBar(this, d->mClippingItem);
    d->mStatusBar->setZValue(HbPrivate::StatusBarZValue);

    // At this point the mainwindow is considered more or less fully constructed.
    HbInstancePrivate::d_ptr()->addWindow(this);

    QRectF rect(0,0,pSize.width(),pSize.height());
    resize(pSize);
    d->mRootItem->setGeometry(rect);
    d->mClippingItem->setGeometry(rect);

    setSceneRect(0, 0, pSize.width(), pSize.height());

    setTransformationAnchor(QGraphicsView::AnchorViewCenter);

    // Rest of the initialization is done later, see paintEvent().
}

/*!
    Destructs the main window.
*/
HbMainWindow::~HbMainWindow()
{
    Q_D(HbMainWindow);
    // cancel any orientation change related effects
    if (d->mEffectItem) {
        HbEffect::enable(d->mEffectItem);
        HbEffect::cancel(d->mEffectItem, QString(), true, false, false);
    }
    HbEffect::enable(&(d->mGVWrapperItem));
    HbEffect::cancel(&(d->mGVWrapperItem), QString(), true, false, false);

    // Cancel all remaining effects without any notifications.  This is needed
    // to prevent crashes when widgets (and views) do not get a chance to cancel
    // their effects properly due to their dtor being called only after the
    // QWidget dtor already initiates some shutdown activities.
    HbEffectInternal::safeCancelAll();

    HbInstancePrivate::d_ptr()->removeWindow(this);
    delete d_ptr;
    // to workaround problem when creating/destroying multiple hbmainwindow's in unit tests (win env)
    #ifdef Q_OS_WIN
        destroy();
    #endif
}

/*!
    Adds a \a widget to the HbMainWindow object. Passing 0 in
    \a widget creates an empty HbView.

    The \a widget can be either a HbView or QGraphicsWidget. If it is
    the QGraphicsWidget then HbMainWindow will create a HbView and set
    \a widget as the new HbView's content widget.

    A HbMainWindow should only have one of each view and adding a view
    it already has will not cause the same view to be in the
    HbMainWindow twice.

    After calling addView() the caller does not need to care about
    destroying \a widget, the framework will take care of that by
    reparenting \a widget if needed.

    \return the new view

    \sa insertView removeView
*/
HbView *HbMainWindow::addView(QGraphicsWidget *widget)
{
    Q_D(HbMainWindow);
    HbView *view = 0;
    if (!widget) {
        view = new HbView;
    } else {
        view = qobject_cast<HbView *>(widget);
        if (!view) {
            view = new HbView;
            view->setWidget(widget);
        }
    }

    // Remove view if it is already inserted
    int index = d->mViewStackWidget->indexOf(view);
    if (index != -1) {
        d->mViewStackWidget->removeAt(index);
    }

    d->mViewStackWidget->insertWidget(-1, view);

    return view;
}

/*!
    Inserts a \a widget to a given \a index to the HbMainWindow object.

    The \a widget can be either a HbView or QGraphicsWidget. If it is
    the QGraphicsWidget then HbMainWindow will create a HbView and set
    it as the internal widget.

    If \a index is out of range, the new view is appended.
    \return the new view.

    \sa addView removeView
*/
HbView *HbMainWindow::insertView(int index, QGraphicsWidget *widget)
{    
    Q_D(HbMainWindow);
    HbView *view = 0;
    if (!widget) {
        view = new HbView(d->mViewStackWidget);
    } else {
        view = qobject_cast<HbView *>(widget);
        if (!view) {
            view = new HbView();
            view->setWidget(widget);
        }
    }
    d->mViewStackWidget->insertWidget(index, view);

    return view;
}

/*!
    Removes a \a widget from the HbMainWindow object. The \a widget must be either
    a HbView or a QGraphicsWidget that is the internal widget of a HbView.

    If \a widget is a QGraphicsWidget, it is not deleted, but it's view is deleted.
    If \a widget is a HbView, then neither view or widget inside it is not deleted.

    The ownership of the \a widget is transferred to the caller. Notice that \a widget
    is not removed from the scene.

    \sa addView insertView
*/
void HbMainWindow::removeView(QGraphicsWidget *widget)
{
    Q_D(HbMainWindow);
    if (!widget) {
        qWarning("HbMainWindow::removeView: widget is null");
        return;
    }

    HbView *view = qobject_cast<HbView *>(widget);
    if (view) {
        d->mViewStackWidget->removeWidget(view);
    } else {
        // Check if it is a widget inside a view and delete that view
        for (int n=0; n<d->mViewStackWidget->count(); n++) {
            HbView *tempView = qobject_cast<HbView*>(d->mViewStackWidget->widgetAt(n));
            if (tempView->widget() == widget) {
                d->mViewStackWidget->removeWidget(tempView);
                // Take a widget out from the view, before deleting it.
                tempView->takeWidget();
                delete tempView;
            }
        }
    }
}

/*!
    \deprecated HbMainWindow::removeView(int)
        is deprecated. Use HbMainWindow::removeView(QGraphicsWidget *widget) instead!

    Removes a view from a given \a index from the HbMainWindow object. The view is not deleted but returned to caller.

    \return the removed widget.

    \sa addView insertView
*/
QGraphicsWidget *HbMainWindow::removeView(int index)
{
    HB_DEPRECATED("HbMainWindow::removeView is deprecated. Use HbMainWindow::removeView(QGraphicsWidget *widget) instead!");
    Q_D(HbMainWindow);
    return d->mViewStackWidget->removeAt(index);
}

/*!
    Returns the current view of the HbMainWindow object.
    \return pointer to the current view object, or 0 if undefined.

    \sa setCurrentView
*/
HbView *HbMainWindow::currentView() const
{
    Q_D(const HbMainWindow);
    return qobject_cast<HbView *>(d->mViewStackWidget->currentWidget());
}

/*!
  Sets the current \a view. The type of the view switch animation is controlled by \a
  flags.  If no view switch effect is registered to the HbView instance (\a view) or \a
  animate is false then there will be no view switch animation displayed.

  \sa currentView()

  \param view The new view to be shown.

  \param animate Whether to show animation on view switch or not.

  \param flags Additional options.

  Note that care must be taken when the application has not registered its own, custom
  hide and show effects for the current and the new view, respectively, because in this
  case some of the explicitly specified flags may not work well with some of the possible
  default effects (that are coming from the currently set theme). It is best not to
  specify any flags explicitly unless the application has registered its own view switch
  effects (so it has knowledge about what how the show and hide effects are defined, what
  kind of execution they expect, etc.)
 */
void HbMainWindow::setCurrentView(HbView *view, bool animate, Hb::ViewSwitchFlags flags)
{
    Q_D(HbMainWindow);
    HbView *oldView = currentView();
    // Switching to null view or to the one that is current will do nothing.
    if (view && oldView != view) {
        if (oldView && animate) {
            if (flags & Hb::ViewSwitchFullScreen) {
                flags |= Hb::ViewSwitchSequential;
            }
            if (flags & Hb::ViewSwitchUseBackAnim) {
                flags &= ~Hb::ViewSwitchUseNormalAnim;
            }
            if (flags & Hb::ViewSwitchUseAltEvent) {
                flags |= Hb::ViewSwitchSequential;
            }
            d->mViewStackWidget->setTargetView(view);
            d->mViewStackWidget->runViewSwitchEffectHide(oldView, flags);
        } else {
            // If animation is disabled or there is no view set currently then change
            // without animation.
            d->mViewStackWidget->setCurrentWidget(view);
            if (d->mDelayedConstructionHandled)
                QMetaObject::invokeMethod(this, "_q_viewReady", Qt::QueuedConnection);
        }
    }
}


/*!
    \deprecated HbMainWindow::currentViewIndex() const
        is deprecated. Use HbMainWindow::currentView() instead!

    Returns the current index of the HbMainWindow object.
    \return current index, or -1 if undefined.

    \sa setCurrentViewIndex setCurrentView currentView
*/
int HbMainWindow::currentViewIndex() const
{
    HB_DEPRECATED("HbMainWindow::currentViewIndex is deprecated. Use HbMainWindow::currentView() instead!");
    Q_D(const HbMainWindow);
    return d->mViewStackWidget->currentIndex();
}

/*!
    \deprecated HbMainWindow::viewCount() const
          is deprecated. Use HbMainWindow::views().count() instead!

    Returns the count of view objects of the HbMainWindow object.
    \return view object count.

    \sa addView insertView
*/
int HbMainWindow::viewCount() const
{
    HB_DEPRECATED("HbMainWindow::viewCount is deprecated. Use HbMainWindow::views().count() instead!");
    Q_D(const HbMainWindow);
    return d->mViewStackWidget->count();
}

/*!
   \deprecated HbMainWindow::indexOfView(HbView*) const
        is deprecated. Use HbMainWindow::views().indexOf(view) instead!

    Returns the index for a given \a view object of the HbMainWindow object.
    \return index of a view object.

    \sa currentViewIndex currentView
*/
int HbMainWindow::indexOfView(HbView *view) const
{
    HB_DEPRECATED("HbMainWindow::indexOfView is deprecated. Use HbMainWindow::views().indexOf(view) instead!");
    Q_D(const HbMainWindow);
    return d->mViewStackWidget->indexOf(view);
}

/*!
    \deprecated HbMainWindow::viewAt(int) const
        is deprecated. Use HbMainWindow::views() instead!

    Returns \a view object for a given \a index of the HbMainWindow object.
    \return view object.

    \sa currentViewIndex currentView indexOfView
*/
HbView *HbMainWindow::viewAt(int index) const
{
    HB_DEPRECATED("HbMainWindow::viewAt is deprecated. Use HbMainWindow::views() instead!");
    Q_D(const HbMainWindow);
    return qobject_cast<HbView *>(d->mViewStackWidget->widgetAt(index));
}

/*!

    Returns list of all views.
 */
QList<HbView *> HbMainWindow::views() const
{
    Q_D(const HbMainWindow);
    HbContentWidget *stackWidget = d->mViewStackWidget;
    
    const int n = stackWidget->count();
    QList<HbView *> result;
    for ( int i=0; i<n; ++i ) {
        HbView *view = qobject_cast<HbView *>(stackWidget->widgetAt(i));
        Q_ASSERT_X(view, "HbMainWindow::views()", "HbView was expected");
        result.append(view);
    }
    
    return result;
}
/*!
    \deprecated HbMainWindow::element(HbMainWindow::Element) const
        is deprecated.

    Returns user interface elements of mainwindow as QGraphicsWidgets. The term "element" here is used since each of these
    elements consist of one or multiple items. The term "item" is used since each element is represented by a single 
    QGraphicsWidget (which is also a QGraphicsItem).

    The returned elements can be used for example to run effects using the HbEffect API.
    For example an application could use the ViewPortItem to do an effect on all the other parts of the UI except the background item.

    Currently supported elements are:
    - HbMainWindow::RootItem, this is an item that contains all other items including the background element.
    - HbMainWindow::ViewportItem, this contains all other items except the background, it can be thought as the viewport for widgets.
      This item gets resized when the orientation changes.
    - HbMainWindow::BackgroundItem, as the name says the enum represents the background item, nothing else.
      Note that in this case the returned pointer may be null if the main window was constructed with Hb::WindowFlagNoBackground.

    \param element Enumeration of the element. 

*/
QGraphicsWidget *HbMainWindow::element(HbMainWindow::Element element) const
{
    HB_DEPRECATED("HbMainWindow::element is deprecated!");
    Q_D(const HbMainWindow);
    if( element == HbMainWindow::RootItem )
        return d->mRootItem;
    else if( element == HbMainWindow::ViewportItem )
        return d->mClippingItem;
    else if( element == HbMainWindow::BackgroundItem )
        return d->mBgItem;

    return 0;
}

/*!
    Returns orientation of the window.

    \sa setOrientation() unsetOrientation()
 */
Qt::Orientation HbMainWindow::orientation() const
{
    Q_D(const HbMainWindow);
    return d->mOrientation;
}

/*!
    Sets orientation of the window and disables the automatic orientation switching. This means
    that orientation will stay in the \a orientation as long as unsetOrientation is called.

    Triggers orientation switch if the orientation was changed from the previous one.
    Orientation switch animation can be skipped by using the boolean parameter.

    \sa orientation() unsetOrientation()

 */

void HbMainWindow::setOrientation(Qt::Orientation orientation, bool animate)
{
    Q_D(HbMainWindow);
    d->mAutomaticOrientationSwitch = false;
    d->setTransformedOrientation(orientation, animate);
}

/*!
    \deprecated HbMainWindow::toggleOrientation()
        is deprecated.

    @proto
    
    Switches orientation of main window.
        
 */
void HbMainWindow::toggleOrientation()
{
    HB_DEPRECATED("HbMainWindow::toggleOrientation is deprecated!");
    Q_D(HbMainWindow);
    d->mAutomaticOrientationSwitch = false;
    if (d->mOrientation == Qt::Horizontal) {
        d->setTransformedOrientation(Qt::Vertical,false);
    } else {
        d->setTransformedOrientation(Qt::Horizontal,false);
    }
}

/*!
    Unsets orientation of the window. Application will listen orientation events coming from system.
    Orientation switch animation can be skipped by using the boolean parameter.

    \sa orientation() setOrientation()
 */
void HbMainWindow::unsetOrientation(bool animate)
{
    Q_D(HbMainWindow);
    if (!d->mAutomaticOrientationSwitch) {
        d->mAutomaticOrientationSwitch = true;
        if(HbMainWindowOrientation::instance()->isEnabled())
            d->setTransformedOrientation(HbMainWindowOrientation::instance()->sensorOrientation(), animate);
    }
}

/*!
  \deprecated HbMainWindow::showItems(QFlags<Hb::SceneItem>)
        is deprecated. Use HbView::showItems instead.

    Sets \a items to be visible in main window.

    \sa isItemVisible() setItemVisible() hideItems() visibleItems()
*/
void HbMainWindow::showItems(Hb::SceneItems items)
{
    HB_DEPRECATED("HbMainWindow::showItems is deprecated! Use HbView::showItems instead.");
    Q_D(HbMainWindow);
    if (d->mVisibleItems ^ items) {
        d->mVisibleItems |= items;
        d->updateVisibleItems();
    }
}

/*!
    \deprecated HbMainWindow::hideItems(QFlags<Hb::SceneItem>)
        is deprecated. Use HbView::hideItems instead.

    Sets \a items to be invisible in main window.

    \sa isItemVisible() setItemVisible() showItems() visibleItems()
*/
void HbMainWindow::hideItems(Hb::SceneItems items)
{
    HB_DEPRECATED("HbMainWindow::hideItems is deprecated! Use HbView::hideItems instead.");
    Q_D(HbMainWindow);
    if (d->mVisibleItems & items) {
        d->mVisibleItems &= ~items;
        d->updateVisibleItems();
    }
}

/*!
    \deprecated HbMainWindow::visibleItems() const
        is deprecated. Use HbView::visibleItems instead.

    Returns visible items.

    \sa isItemVisible() setItemVisible() hideItems() showItems()

*/
Hb::SceneItems HbMainWindow::visibleItems() const
{
    HB_DEPRECATED("HbMainWindow::visibleItems is deprecated! Use HbView::visibleItems instead.");
    Q_D(const HbMainWindow);
    return d->mVisibleItems;
}

/*!
    \deprecated HbMainWindow::isItemVisible(Hb::SceneItem) const
        is deprecated. Use HbView::isItemVisible instead.

    Returns \c true if \a item is set to be visible.

    \sa setItemVisible() hideItems() showItems() visibleItems()

*/
bool HbMainWindow::isItemVisible(Hb::SceneItem item) const
{
    HB_DEPRECATED("HbMainWindow::isItemVisible is deprecated! Use HbView::isItemVisible instead.");
    Q_D(const HbMainWindow);
    return d->mVisibleItems & item;
}

/*!
    \deprecated HbMainWindow::setItemVisible(Hb::SceneItem, bool) 
        is deprecated. Use HbView::setItemVisible instead.

    Sets \a item to be \a visible in main window.

    \sa isItemVisible() hideItems() showItems() visibleItems()
*/
void HbMainWindow::setItemVisible(Hb::SceneItem item, bool visible)
{
    HB_DEPRECATED("HbMainWindow::setItemVisible is deprecated! Use HbView::setItemVisible instead.");
    if (visible) {
        showItems(item);
    } else {
        hideItems(item);
    }
}

/*!
    Returns empty drawable window.

    Returned window is inherited from CCoeControl in Symbian platform
    and by default you don't have any surface attached to it. In other
    platforms returns 0.

    This window is meant to be used when developer wants to use own surface
    running underneath the UI, e.g. video playback surface or HW accelerated
    3D content in own surface.

    See QWidget::winId

    \sa resetNativeWindow, HbTransparentWindow
*/
WId HbMainWindow::nativeBackgroundWindow()
{
#ifdef Q_OS_SYMBIAN
    Q_D(HbMainWindow);

    if (!d->mNativeWindow) {
        d->mNativeWindow = new HbNativeWindow;
        d->mNativeWindow->ConstructL();
        // Let's make sure, that native window is always behind HbMainWindow instance
        TInt mainWindowOrdinal = winId()->DrawableWindow()->OrdinalPosition();
        d->mNativeWindow->DrawableWindow()->SetOrdinalPosition(mainWindowOrdinal + 1);
    }

    return d->mNativeWindow;
#else
    return 0;
#endif
}

/*!
  Deletes the empty drawable window.

  \sa nativeWindow
*/
void HbMainWindow::resetNativeBackgroundWindow()
{
#ifdef Q_OS_SYMBIAN
    Q_D(HbMainWindow);

    if (d->mNativeWindow) {
        delete d->mNativeWindow;
        d->mNativeWindow = 0;
    }
#endif
}

/*!
    Returns the rectangle which is used for layouting HbMainWindow contents. Updates on orientation change and is up to date
    after HbMainWindow orientationChanged() signal. Note that this is not the same thing as QGraphicsView (HbMainWindow) geometry. 
    HbMainWindow geometry does not update on orientation change since the contents are only transformed with a rotate transform.
    
*/
QRectF HbMainWindow::layoutRect() const
{
    Q_D(const HbMainWindow);
    if (d->mClippingItem) {
        return d->mClippingItem->geometry();
    }
    return QRectF();
}
/*!
    \deprecated HbMainWindow::setCurrentViewIndex(int)
        is deprecated. Use setCurrentView() instead.

    Sets the current view from a given \a index of the HbMainWindow object.

    \sa currentView currentViewIndex setCurrentView
*/
void HbMainWindow::setCurrentViewIndex(int index)
{
    HB_DEPRECATED("HbMainWindow::setCurrentViewIndex is deprecated! Use setCurrentView() instead.");
    Q_D(HbMainWindow);
    QGraphicsWidget *widget = d->mViewStackWidget->widgetAt(index);
    setCurrentView(qobject_cast<HbView*>(widget));
}

/*!
    \deprecated HbMainWindow::nextView()
        is deprecated. Use setCurrentView() instead.

    Switches to next view (view with bigger index). 
    If current view is last view it does nothing.
 */
void HbMainWindow::nextView()
{
    HB_DEPRECATED("HbMainWindow::nextView is deprecated! Use setCurrentView() instead.");
    int index = currentViewIndex()+1;
    if( index<viewCount() ) {
        setCurrentViewIndex(index);
    }
}

/*!
    \deprecated  HbMainWindow::previousView()
        is deprecated. Use setCurrentView() instead.

    Switches to previous view (view with smaller index). 
    If current view is first view it does nothing.
 */
void HbMainWindow::previousView()
{
    HB_DEPRECATED("HbMainWindow::previousView is deprecated! Use setCurrentView() instead.");
    int index = currentViewIndex()-1;
    if( index>=0 ) {
        setCurrentViewIndex(index);
    }
}

/*!
  Sets the \a name for the background image for the given \a orientation.  If
  the default image from the theme is suitable then there is no need to call
  this function.

  \sa backgroundImageName()
 */
void HbMainWindow::setBackgroundImageName(Qt::Orientation orientation, const QString &name)
{
    Q_D(HbMainWindow);
    if (d->mBgItem) {
        d->mBgItem->setImageName(orientation, name);
    }
}

/*!
  Returns the background image name for the given \a orientation.
  If no setBackgroundImageName() calls were made before then returns the default.

  \sa setBackgroundImageName()
 */
QString HbMainWindow::backgroundImageName(Qt::Orientation orientation) const
{
    Q_D(const HbMainWindow);
    return d->mBgItem ? d->mBgItem->imageName(orientation) : QString();
}

/*!
     Reimplemented from QWidget::changeEvent().
*/
void HbMainWindow::changeEvent(QEvent *event)
{
    QGraphicsView::changeEvent(event);
    if (event->type() == QEvent::LayoutDirectionChange) {
        // Notify layout direction change to the icon framework
        HbLayoutDirectionNotifier::instance()->notifyLayoutDirectionChange();

        foreach (QGraphicsItem *item, items()) {
            if (item->isWidget() && !item->parentItem() ) {
                QGraphicsWidget *widget = static_cast<QGraphicsWidget *>(item);
                if (!widget->testAttribute(Qt::WA_SetLayoutDirection)){
                    widget->setLayoutDirection(layoutDirection());
                    widget->setAttribute(Qt::WA_SetLayoutDirection, false);
                }
            }
        }
    }
}

/*!
     Reimplemented from QWidget::closeEvent().
*/
void HbMainWindow::closeEvent(QCloseEvent *event)
{
    QGraphicsView::closeEvent(event);
    HbInstancePrivate::d_ptr()->removeWindow(this);
}

/*!
    Reimplemented from QWidget::keyPressEvent().
*/
void HbMainWindow::keyPressEvent(QKeyEvent *event)
{
    // pass the soft key press into the soft key decorator class
    HbAction *action = 0;

    switch(event->key()) {

#ifdef Q_OS_SYMBIAN
    case Qt::Key_Context1:
#else
    case Hb::Key_SoftKeyPrimary:
#endif

        event->accept();
        break;

#ifdef Q_OS_SYMBIAN
    case Qt::Key_Select:
#else
    case Hb::Key_SoftKeyMiddle:
#endif

        event->accept();
        break;

#ifdef Q_OS_SYMBIAN
    case Qt::Key_Context2:
#else
    case Hb::Key_SoftKeySecondary:
#endif
        if (currentView()) {
            action = currentView()->navigationAction();
        }
        event->accept();
        break;

    default:
        QGraphicsView::keyPressEvent(event);
        break;
    }

    if (action) {
        action->trigger();
    }
}

/*!
    Reimplemented from QWidget::keyReleaseEvent().
*/
void HbMainWindow::keyReleaseEvent(QKeyEvent *event)
{
    QGraphicsView::keyReleaseEvent(event);
}

/*!
    Reimplemented from QWidget::resizeEvent().
*/
void HbMainWindow::resizeEvent(QResizeEvent *event)
{
    Q_D(HbMainWindow);

    if ( !HbMainWindowPrivate::dragToResizeEnabled ) {
        // determine the default orientation width < height -> portrait
        if (event->size().width() < event->size().height())
            d->mDefaultOrientation = Qt::Vertical;
        else
            d->mDefaultOrientation = Qt::Horizontal;
        d->mForceSetOrientation = true;
        d->setTransformedOrientation(d->mOrientation, false);
        d->mForceSetOrientation = false;
    } else {
        // RnD feature for resizing the window by dragging
        QSize newSize(event->size());
        setSceneRect(0,0,newSize.width(),newSize.height());
        d->mClippingItem->resize(newSize);
        if (d->mBgItem) {
            d->mBgItem->resize(newSize);
        }
    }
}

/*!
    Reimplemented from QObject::customEvent().
*/
void HbMainWindow::customEvent( QEvent *event )
{
    Q_D(HbMainWindow);
    if (event->type() == HbMainWindowPrivate::IdleEvent) { // called asyncronously after the application start-up
        if (!d->mIdleEventHandled) {
            d->mIdleEventHandled = true;
            if ( HbFeatureManager::instance()->featureStatus( HbFeatureManager::TheTestUtility ) ) {
                // create the test utility
                if ( !d->mTheTestUtility ) {
                    d->mTheTestUtility = new HbTheTestUtility(this);
                }
            }
            // get rid of the splash screen widget (it is not visible to the user anyway at this point)
            HbSplashScreen::destroy();
        }
        // Notify that mainwindow is (most probably) ready.
        // The signal must be emitted always, even when there was no need to do anything.
        emit d->idleEventDispatched();
    } else if(event->type() == HbMainWindowPrivate::IdleOrientationEvent) { // complete the orientation change effect chain
        if (d->mEffectItem && d->mOrientationChangeOngoing) {
            HbEffect::start(d->mEffectItem, "rootItemFinalPhase", this, "rootItemFinalPhaseDone");
        }
    } else if (event->type() == HbMainWindowPrivate::IdleOrientationFinalEvent) {
        if (d->mAnimateOrientationSwitch) {
            HbEffect::start(d->mStatusBar, "statusbar", "appear_orient"); 
            HbEffect::start(d->mTitleBar, "titlebar", "appear_orient");
            if (d->mCurrentToolbar) {         
                HbToolBarPrivate *toolBarD = HbToolBarPrivate::d_ptr(d->mCurrentToolbar);
                toolBarD->startAppearOrientEffect();
            }
        }
    }
      
}

/*!
  \reimp
*/
void HbMainWindow::paintEvent(QPaintEvent *event)
{
    // Invoke the delayed construction slot asynchronously if not yet done.
    Q_D(HbMainWindow);
    // With this condition we may end up invoking the slot more than once but it
    // is not a problem as the slot will have proper guard conditions.
    if (!d->mDelayedConstructionHandled) {
        QMetaObject::invokeMethod(this, "_q_delayedConstruction", Qt::QueuedConnection);
    }

    QGraphicsView::paintEvent(event);
}

/*!
    Reimplemented from QAbstractScrollArea::scrollContentsBy().
*/
void HbMainWindow::scrollContentsBy(int dx, int dy)
{
    // ignore scrolling attempt
    Q_UNUSED(dx);
    Q_UNUSED(dy);
}

/*!
    Broadcasts \a eventType into all widgets in the scene.

    Visible widgets will receive the event synchronously and invisble widgets
    asynchronously.

    If the receiving widget has abstract items as child items, these will be informed
    after the widget has received the event. 
*/
void HbMainWindow::broadcastEvent( int eventType )
{
    Q_D(HbMainWindow);
    d->broadcastEvent( eventType );
}

HbRootItem::HbRootItem(QGraphicsItem *parent)
    : HbWidget(parent)
{
}

bool HbRootItem::event(QEvent *event)
{
    if (event->type() == QEvent::Polish) {
        // No need for any real polishing.
        static_cast<HbWidgetPrivate *>(d_ptr)->polished = true;
        return true;
    }
    return HbWidget::event(event);
}

#include "moc_hbmainwindow.cpp"
