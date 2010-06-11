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

#include "hbmainwindow.h"
#include "hbmainwindow_p.h"

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
#include "hbnamespace.h"
#include "hbnamespace_p.h"
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
#include "hbsplashscreen.h"
#include "hbcontentwidget_p.h"
#include "hbscreen_p.h"
#include "hbmainwindoworientation_p.h"
#include "hbfeaturemanager_p.h"
#include "hboogmwatcher_p.h"

#ifdef Q_OS_SYMBIAN
#include <coecntrl.h>
#include <w32std.h>
#include "hbnativewindow_sym_p.h"
#endif

/*!
    @stable
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
    application's view construction. The viewReady() signal is emitted when a
    view's internal construction is completed and basic parts of view are
    already drawn. Same signal is also emitted when current view has
    changed. This helps applications to split the view construction to
    reasonable tasks.  For example the lower priority tasks like opening network
    connection or preparing other currently hidden application views can happen
    on background when first view is already drawn.

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

    This signal is emitted when the orientation change starts. This version of
    aboutToChangeOrientation also includes the information about the new
    orientation and whether the orientation change is animated or not.  This can
    be used for example by widgets who want to participate in the orientation
    change animation.
 */

/*!
    \fn void HbMainWindow::orientationChanged()

    This signal is emitted when animation related to orientation change has completed.
 */

/*!
    \fn void HbMainWindow::aboutToChangeView(HbView *oldView, HbView *newView)

    This signal is emitted before starting a view change. Note that it is only
    emitted when there is a real view change (i.e. when setCurrentView() is
    used), it will not be emitted when views are added to the main window via
    addView().

    It is guaranteed that this signal is emitted before starting any view switch
    animation.
 */

/*!
    \fn void HbMainWindow::currentViewChanged(HbView *view)

    This signal is emitted when the current view changes. Do not make assumptions about
    the exact timing of this signal, it may be emitted at any time during the view switch
    (this is relevant if the view switch is animated, i.e. it is not guaranteed that the
    signal will be emitted at the end of the animation, in fact it is usually emitted
    already when the animation starts).

    \sa aboutToChangeView()
    \sa viewReady()
 */

/*!
    \fn void HbMainWindow::viewReady()

    This signal is emitted first time when window content is drawn on screen.
    It will only be emitted again when current view is changed and drawn on screen.

    This means that this signal is emitted in the following cases:

    - When the mainwindow is fully constructed, this happens shortly after
      painting it for the first time.

    - When a new view is added using addView() or insertView() after the
      mainwindow is fully constructed and the newly added view becomes the
      current view. It will not be emitted when calling addView() or
      insertView() before showing the mainwindow or entering the event loop
      because in that case the signal will anyway be emitted later, when the
      mainwindow becomes ready. It is also not emitted when the newly added view
      does not become the current view.

    - When the current view is changed using setCurrentView().

    If the view switch is animated, the signal is emitted only after the effect has
    completed.

    Application developers may connect to this signal to do lower priority construction
    tasks. Note however that when thinking about the tasks, it is important to keep the UI
    responsive all the time.
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
    explicit  HbRootItem(QGraphicsItem *parent = 0);
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
    } else if (HbMainWindowOrientation::instance()->isEnabled()) {
        d->mOrientation = HbMainWindowOrientation::instance()->sensorOrientation();
        d->mAutomaticOrientationSwitch = true;
    } else {
        d->mOrientation = d->mDefaultOrientation;
        d->mAutomaticOrientationSwitch = false;
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
            this, SLOT(_q_viewChanged()));
    connect(d->mViewStackWidget, SIGNAL(widgetRemoved(QGraphicsWidget *)),
            this, SLOT(_q_viewRemoved(QGraphicsWidget *)));

    // create Titlebar (container for indicators, titlepane and secondary softkey
    d->mTitleBar = new HbTitleBar(this, d->mClippingItem);
    d->mTitleBar->setZValue(HbPrivate::TitleBarZValue);

    // create StatusBar (container for clock, indicators, and battery and signal icons)
    d->mStatusBar = new HbStatusBar(this, d->mClippingItem);
    d->mStatusBar->setZValue(HbPrivate::StatusBarZValue);

    // At this point the mainwindow is considered more or less fully constructed.
    HbInstancePrivate::d_ptr()->addWindow(this);

    QRectF rect(0, 0, pSize.width(), pSize.height());
    resize(pSize);
    d->mLayoutRect = rect;
    d->mRootItem->setGeometry(rect);
    d->mClippingItem->setGeometry(rect);
    setSceneRect(0, 0, pSize.width(), pSize.height());
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);

    // Initialize common translations. This must be done before
    // creating any widget that can potentially use localized strings
    // and therefore cannot be delayed.
    d->initTranslations();

    // Make sure the oogm watcher is initialized (so that wserv events
    // are routed to it properly).
    HbOogmWatcher::instance();

#ifdef HB_GESTURE_FW
    // @todo remove after view auto-subscribes to gestures
    viewport()->grabGesture(Qt::TapGesture);
    viewport()->grabGesture(Qt::TapAndHoldGesture);
    viewport()->grabGesture(Qt::PanGesture);
    viewport()->grabGesture(Qt::SwipeGesture);
    viewport()->grabGesture(Qt::PinchGesture);
#endif


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
    a QGraphicsWidget (or any subclass that is not HbView) then
    HbMainWindow will create a HbView and set \a widget as the new
    HbView's content widget.

    When \a widget is a HbView, use HbView::setWidget() to set the content
    widget for the view. Note that you should never attach child items directly
    to the HbView instance, even though HbView is also a HbWidget. Instead, create
    a content widget, set it to the view via HbView::setWidget(), and attach children
    to that.

    Use setCurrentView() to switch between the added views. (only one of them is visible at a time)
    The view-specific decorators (toolbar, Options menu, title in the titlebar) and of course
    the visibility of the view's content widgets will be updated and managed automatically by the framework
    when switching views.

    For a detailed description of views see the HbView class.
    
    Note that using view switching (i.e. several HbView instances, setCurrentView(), etc.) in
    Hb applications is not mandatory, it is purely optional. For applications that are not really
    view based (e.g. because they only have one screen of content or because they have more "fluid" UI where
    the traditional view separation does not make that much sense) it may sometimes be better (and may provide more freedom)
    to have just one view and manage the content entirely via the content widget of that one view.
    (one can still use HbStackedLayout and manual visibility management of child widgets to achieve a traditional view-like look,
    even when the Hb view management is not used)

    After calling addView() the caller does not need to care about
    destroying \a widget, the framework will take care of that by
    reparenting \a widget if needed.

    A HbMainWindow should only have one of each view and adding a view
    it already has will not cause the same view to be in the
    HbMainWindow twice.

    \return the new view

    \sa insertView() removeView() setCurrentView()
    \sa HbView HbStackedLayout
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

    // If the newly added view becomes the current one then emit the viewReady
    // signal (unless the delayed construction is still pending).
    if (d->mDelayedConstructionHandled && currentView() == view) {
        QMetaObject::invokeMethod(this, "_q_viewReady", Qt::QueuedConnection);
    }

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

    // If the newly inserted view becomes the current one then emit the
    // viewReady signal (unless the delayed construction is still pending).
    if (d->mDelayedConstructionHandled && currentView() == view) {
        QMetaObject::invokeMethod(this, "_q_viewReady", Qt::QueuedConnection);
    }

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
        for (int n = 0; n < d->mViewStackWidget->count(); n++) {
            HbView *tempView = qobject_cast<HbView *>(d->mViewStackWidget->widgetAt(n));
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
        emit aboutToChangeView(oldView, view);
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
            if (d->mDelayedConstructionHandled) {
                QMetaObject::invokeMethod(this, "_q_viewReady", Qt::QueuedConnection);
            }
        }
    }
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
    for (int i = 0; i < n; ++i) {
        HbView *view = qobject_cast<HbView *>(stackWidget->widgetAt(i));
        Q_ASSERT_X(view, "HbMainWindow::views()", "HbView was expected");
        result.append(view);
    }

    return result;
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
    d->mUserOrientationSwitch = true;
    d->setTransformedOrientation(orientation, animate);
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
        d->mUserOrientationSwitch = false;
        if (HbMainWindowOrientation::instance()->isEnabled()) {
            d->setTransformedOrientation(HbMainWindowOrientation::instance()->sensorOrientation(), animate);
        }
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
    return d->mLayoutRect;
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
  Sets the background image drawing mode. This setting controls how
  the background image is displayed.

  By default the mode is set to Hb::ScaleBackgroundToFit.

  \sa backgroundImageMode()
  \sa Hb::BackgroundImageMode
 */
void HbMainWindow::setBackgroundImageMode(Hb::BackgroundImageMode mode)
{
    Q_D(HbMainWindow);
    if (d->mBgItem) {
        d->mBgItem->setImageMode(mode);
    }
}

/*!
  Returns the currently set background image drawing mode.

  \sa setBackgroundImageMode()
  \sa Hb::BackgroundImageMode
 */
Hb::BackgroundImageMode HbMainWindow::backgroundImageMode() const
{
    Q_D(const HbMainWindow);
    return d->mBgItem ? d->mBgItem->imageMode() : Hb::ScaleBackgroundToFit;
}

/*!
  Sets the animations enabled when the orientation is changed automatically.
  By default animations are enabled.

  \sa automaticOrientationEffectEnabled()
 */

void HbMainWindow::setAutomaticOrientationEffectEnabled(bool enabled)
{
    Q_D(HbMainWindow);
    d->mAutomaticOrientationChangeAnimation = enabled;
}

/*!
  Returns boolean value to signify whether animations enabled/disabled during
  automatic orientation change. By default animations are enabled.

  \sa setAutomaticOrientationEffectEnabled()
 */
bool HbMainWindow::automaticOrientationEffectEnabled() const
{
    Q_D(const HbMainWindow);
    return d->mAutomaticOrientationChangeAnimation;
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

        broadcastEvent(HbEvent::WindowLayoutDirectionChanged);

        foreach(QGraphicsItem * item, items()) {
            if (item->isWidget() && !item->parentItem()) {
                QGraphicsWidget *widget = static_cast<QGraphicsWidget *>(item);
                if (!widget->testAttribute(Qt::WA_SetLayoutDirection)) {
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

    switch (event->key()) {

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

    if (!HbMainWindowPrivate::dragToResizeEnabled) {
        // determine the default orientation width < height -> portrait
        if (event->size().width() < event->size().height()) {
            d->mDefaultOrientation = Qt::Vertical;
        } else {
            d->mDefaultOrientation = Qt::Horizontal;
        }
        d->mForceSetOrientation = true;
        d->setTransformedOrientation(d->mOrientation, false);
        d->mForceSetOrientation = false;
    } else {
        // RnD feature for resizing the window by dragging
        QSize newSize(event->size());
        setSceneRect(0, 0, newSize.width(), newSize.height());
        d->mClippingItem->resize(newSize);
        if (d->mBgItem) {
            d->mBgItem->resize(newSize);
        }
    }
}

/*!
    Reimplemented from QObject::customEvent().
*/
void HbMainWindow::customEvent(QEvent *event)
{
    Q_D(HbMainWindow);
    if (event->type() == HbMainWindowPrivate::IdleEvent) { // called asyncronously after the application start-up
        if (!d->mIdleEventHandled) {
            d->mIdleEventHandled = true;
            if (HbFeatureManager::instance()->featureStatus(HbFeatureManager::TheTestUtility)) {
                // create the test utility
                if (!d->mTheTestUtility) {
                    d->mTheTestUtility = new HbTheTestUtility(this);
                }
            }
            // get rid of the splash screen widget (it is not visible to the user anyway at this point)
            HbSplashScreen::destroy();
#ifdef Q_OS_SYMBIAN
            // disable surface transparency unless we were really asked to be transparent
            if (!testAttribute(Qt::WA_TranslucentBackground)) {
                RWindow *const window = static_cast<RWindow *>(effectiveWinId()->DrawableWindow());
                window->SetSurfaceTransparency(false);
            }
#endif
        }
        // Notify that mainwindow is (most probably) ready.
        // The signal must be emitted always, even when there was no need to do anything.
        emit d->idleEventDispatched();
    } else if (event->type() == HbMainWindowPrivate::IdleOrientationEvent) { // complete the orientation change effect chain
        if (d->mEffectItem && d->mOrientationChangeOngoing) {
            HbEffect::start(d->mEffectItem, "rootItemFinalPhase", this, "rootItemFinalPhaseDone");
        }
    } else if (event->type() == HbMainWindowPrivate::IdleOrientationFinalEvent) {
        if (d->mAnimateOrientationSwitch) {
            HbEffect::start(d->mTitleBar, "titlebar", "appear_orient");
            HbEffect::start(d->mStatusBar, "statusbar", "appear_orient");
            if (d->mCurrentToolbar) {
                HbToolBarPrivate *toolBarD = HbToolBarPrivate::d_ptr(d->mCurrentToolbar);
                toolBarD->startAppearOrientEffect();
            } else {
                foreach(HbView * view, views()) {
                    view->toolBar()->resetTransform();
                    view->toolBar()->setOpacity(1);
                    view->toolBar()->show();
                    HbToolBarPrivate::d_ptr(view->toolBar())->mOrientationEffectsRunning = false;
                }
            }
            d->updateOrientationChangeStatus();
            if (d->mAutomaticOrientationSwitch && HbMainWindowOrientation::instance()->isEnabled()) {
                d->setTransformedOrientation(HbMainWindowOrientation::instance()->sensorOrientation(), d->mAnimateOrientationSwitch);
            } else if (d->mRequestedOrientation != d->mOrientation) {
                d->setTransformedOrientation(d->mRequestedOrientation, d->mAnimateOrientationSwitch);
            }
        } else {
            // cancel() with clearEffect cannot be used because the effects may have already been canceled.
            if (d->mTitleBar) {
                d->mTitleBar->resetTransform();
                d->mTitleBar->setOpacity(1);
            }
            if (d->mStatusBar) {
                d->mStatusBar->resetTransform();
                d->mStatusBar->setOpacity(1);
            }
            if (d->mCurrentToolbar) {
                d->mCurrentToolbar->resetTransform();
                d->mCurrentToolbar->setOpacity(1);
                d->mCurrentToolbar->show();
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

void HbMainWindow::showEvent(QShowEvent *event)
{
#ifdef Q_OS_SYMBIAN
    // Enable surface transparency if QWidget did not do it already. This is a
    // workaround for having non-transparent surfaces filled automatically with
    // black color. The showEvent is a suitable place because the native control
    // is already created at this point, but it is not too late either.
    if (!testAttribute(Qt::WA_TranslucentBackground)) {
        RWindow *const window = static_cast<RWindow *>(effectiveWinId()->DrawableWindow());
        window->SetSurfaceTransparency(true);
    }
#endif
    QGraphicsView::showEvent(event);
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
void HbMainWindow::broadcastEvent(int eventType)
{
    Q_D(HbMainWindow);
    d->broadcastEvent(eventType);
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
