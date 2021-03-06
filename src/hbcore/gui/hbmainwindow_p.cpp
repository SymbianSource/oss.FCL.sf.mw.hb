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

#include "hbmainwindow_p.h"
#include "hbgraphicsscene.h"
#include "hbindicatorbutton_p.h"
#include "hbtitlebar_p.h"
#include "hbstatusbar_p.h"
#include "hbmenu.h"
#include "hbnamespace.h"
#include "hbnamespace_p.h"
#include "hbstackedwidget.h"
#include "hbtoolbar.h"
#include "hbview.h"
#include "hbview_p.h"
#include "hbevent.h"
#include "hbinstance.h"
#include "hbtoolbar_p.h"
#include "hbdockwidget.h"
#include "hbeffectinternal_p.h"
#include "hbthetestutility_p.h"
#include "hbinputmethod.h"
#include "hbfadeitem_p.h"
#include "hbcontentwidget_p.h"
#include "hbscreen_p.h"
#include "hbbackgrounditem_p.h"
#include "hbscreenshotitem_p.h"
#include "hbcorepskeys_r.h"
#include "hborientationstatus_p.h"
#include "hbglobal_p.h"

#include <QApplication>
#include <QGraphicsLayout>
#include <QLocale>
#include <QDir>

#ifdef Q_OS_SYMBIAN
#include "hbnativewindow_sym_p.h"
#include "hbdevicedialogserverdefs_p.h"
#include <qsymbianevent.h>
#include <w32std.h>
#include <coemain.h>

const TUid deviceDialogUid = {0x20022FC5};
#endif //Q_OS_SYMBIAN

#if defined(Q_WS_X11)
#include <X11/Xlib.h>
#include <qx11info_x11.h>
#endif // defined(Q_WS_X11)

const int HbMainWindowPrivate::IdleEvent = QEvent::registerEventType();
const int HbMainWindowPrivate::IdleOrientationEvent = QEvent::registerEventType();
const int HbMainWindowPrivate::IdleOrientationFinalEvent = QEvent::registerEventType();

//#define ORIENTATION_CHANGE_DEBUGPRINTS

// for working around invalid device profile data
const int QVga_res = 76800;

// Setting for resizing by dragging
bool HbMainWindowPrivate::dragToResizeEnabled = false;
bool HbMainWindowPrivate::initializeInputs = true;

HbMainWindowPrivate::HbMainWindowPrivate() :
    mScene(0),
    mBgItem(0),
    mClippingItem(0),
    mScreenshotItem(0),
    mViewStackWidget(0),
    mTitleBar(0),
    mStatusBar(0),
    mFadeItem(0),
    mRootItem(0),
    mPendingOrientationValue(0),
    mAutomaticOrientationSwitch(true),
    mUserOrientationSwitch(false),
    mOrientationChangeOngoing(false),
    mRootItemFinalPhaseDone(false),
    mOrientationEffectFinished(false),
    mGVOrientationChangeEffectEnabled(false),
    mPendingPsPublish(false),
    mMainWindowActive(false),
    mOrientation(Qt::Vertical),
    mRequestedOrientation(Qt::Vertical),
    mCurrentToolbar(0),
    mCurrentDockWidget(0),
    mVisibleItems(Hb::AllItems),
    mForceSetOrientation(false),
    mDelayedConstructionHandled(false),
    q_ptr(0),
    mTheTestUtility(0),
    mIdleEventHandled(false),
    mToolbarWasAdded(false),
    mAutomaticOrientationChangeAnimation(true),
    mObscuredState(true)
#ifdef Q_OS_SYMBIAN
    ,
    mNativeWindow(0),
    mDevDlgClientSession(0)
#endif
{
#if !defined(Q_WS_X11) && !defined(Q_OS_SYMBIAN)
    mObscuredState = false;  //For non-x11 and non-symbian we start with revealed state.
#endif

    qApp->installEventFilter(this);
}

HbMainWindowPrivate::~HbMainWindowPrivate()
{
#ifdef Q_OS_SYMBIAN
    if (mNativeWindow) {
        delete mNativeWindow;
        mNativeWindow = 0;
    }
#endif

    delete mTheTestUtility;
    mEffectItem = 0;
}

void HbMainWindowPrivate::init()
{
    Q_Q(HbMainWindow);

    mGVWrapperItem.setMainWindow(*q);
}

void HbMainWindowPrivate::initTranslations()
{
    QString lang = QLocale::system().name();
#ifdef Q_OS_SYMBIAN
    // Use Z drive. Anything else is insecure as anyone could install
    // a fake common translation to C drive for example.
    QString commonname = QDir(QLatin1String("Z:/resource/qt/translations")).filePath(QLatin1String("common_") + lang);
    QString qtname = QDir(QLatin1String("Z:/resource/qt/translations")).filePath(QLatin1String("qt_") + lang);
#else
    QString commonname = QDir(QLatin1String(HB_TRANSLATIONS_DIR)).filePath(QLatin1String("commonstrings_") + lang);
#endif
    if (mCommonTranslator.load(commonname)) {
        QCoreApplication::installTranslator(&mCommonTranslator);
    } else {
        qWarning("initTranslations: Failed to load translator based on name %s", qPrintable(commonname));
    }
#ifdef Q_OS_SYMBIAN
    if (mQtTranslator.load(qtname)) {
        QCoreApplication::installTranslator(&mQtTranslator);
    } // may not exist in all cases, just let it fail silently
#endif
}

HbToolBar *HbMainWindowPrivate::toolBar() const
{
    HbView *view = qobject_cast<HbView *>(mViewStackWidget->currentWidget());
    return view ? HbViewPrivate::d_ptr(view)->toolBar : 0;
}

void HbMainWindowPrivate::addToolBarToLayout(HbToolBar *toolBar)
{
    Q_Q(HbMainWindow);

    if (mCurrentToolbar != toolBar) {

        // Remove old toolbar
        removeToolBarFromLayout(mCurrentToolbar);

        // Add new toolbar
        if (toolBar) {
            toolBar->setParentItem(0);
            HbView *currentView = q->currentView();
            if (toolBar->scene() != mScene
                    && currentView
                    && currentView->isItemVisible(Hb::ToolBarItem)) { // check just to avoid warnings
                // Update the toolbar to use the current orientation
                // and layout direction.
                if (toolBar->layoutDirection() != q->layoutDirection() &&
                        !toolBar->testAttribute(Qt::WA_SetLayoutDirection)) {

                    toolBar->setLayoutDirection(q->layoutDirection());
                    toolBar->setAttribute(Qt::WA_SetLayoutDirection, false);
                }
                HbToolBarPrivate *toolBarD = HbToolBarPrivate::d_ptr(toolBar);
                // No "appear" effect when changing views or when the first
                // toolbar is shown. The latter is needed to prevent bad UX when
                // a splash screen containing a toolbar is shown before the
                // mainwindow.
                if (mViewStackWidget->isSwitchingViews() || !mToolbarWasAdded) {
                    toolBarD->suppressNextAppearEffect();
                    mToolbarWasAdded = true;
                } else if (mOrientationChangeOngoing && !mAnimateOrientationSwitch) {
                    toolBarD->suppressNextAppearEffect();
                }
                mScene->addItem(toolBar); // top level
            }

            toolBar->setZValue(HbPrivate::ToolBarZValue);
            toolBar->setParentItem(mClippingItem);
            mClippingItem->setToolBar(toolBar);
            mCurrentToolbar = toolBar;

            toolBar->resetTransform();
        }
    }
}

void HbMainWindowPrivate::removeToolBarFromLayout(HbToolBar *toolBar)
{
    Q_Q(HbMainWindow);
    if (mCurrentToolbar && mCurrentToolbar->scene() && mCurrentToolbar == toolBar) {
        mClippingItem->setToolBar(0);
        mScene->removeItem(mCurrentToolbar);
        mCurrentToolbar->disconnect(q);
        mCurrentToolbar = 0;
    }
}

HbDockWidget *HbMainWindowPrivate::dockWidget() const
{
    HbView *view = qobject_cast<HbView *>(mViewStackWidget->currentWidget());
    return view ? HbViewPrivate::d_ptr(view)->dockWidget : 0;
}

void HbMainWindowPrivate::addDockWidgetToLayout(HbDockWidget *dockWidget)
{
    Q_Q(HbMainWindow);
    if (mCurrentDockWidget != dockWidget) {

        // Remove old dock
        removeDockWidgetFromLayout(mCurrentDockWidget);

        // Add new dock
        if (dockWidget) {
            dockWidget->setParentItem(0);
            if (dockWidget->scene() != mScene) {
                HbView *currentView = q->currentView();
                if (currentView && currentView->isItemVisible(Hb::DockWidgetItem)) {
                    mScene->addItem(dockWidget); // top level
                }
            }

            dockWidget->setZValue(HbPrivate::DockWidgetZValue);

            dockWidget->setParentItem(mClippingItem);
            mClippingItem->setDockWidget(dockWidget);
            mCurrentDockWidget = dockWidget;
        }
    }
}

void HbMainWindowPrivate::removeDockWidgetFromLayout(HbDockWidget *dockWidget)
{
    Q_Q(HbMainWindow);
    if (mCurrentDockWidget && mCurrentDockWidget->scene() && mCurrentDockWidget == dockWidget) {
        mClippingItem->setDockWidget(0);
        mScene->removeItem(mCurrentDockWidget);
        mCurrentDockWidget->disconnect(q);
        mCurrentDockWidget = 0;
    }
}

void HbMainWindowPrivate::setTransformedOrientation(Qt::Orientation orientation, bool animate)
{
    Q_Q(HbMainWindow);
    mRequestedOrientation = orientation;

    if (mOrientationChangeOngoing) {
        if (!mForceSetOrientation && !mUserOrientationSwitch) {
            return;
        } else if (mOrientation != orientation) {
            HbEffectInternal::cancelAll();
            // Possibly posted idle events must not be delivered,
            // there will be new ones due to this new orientation switch.
            QCoreApplication::removePostedEvents(q, HbMainWindowPrivate::IdleOrientationEvent);
            QCoreApplication::removePostedEvents(q, HbMainWindowPrivate::IdleOrientationFinalEvent);
            QCoreApplication::removePostedEvents(q, QEvent::MetaCall);
        }
    }

    if (mOrientation == orientation && !mForceSetOrientation && mEffectItem) {
        return;
    }

    // skip transition if graphicsview is not visible
    mAnimateOrientationSwitch = animate;

    if (!q->isVisible()) {
        mAnimateOrientationSwitch = false;
    }

    // calling due to resize, orientation remains the same -> no signalling
    if (!((mOrientation == orientation) && mForceSetOrientation)) {
        // cancel all effects
        HbEffectInternal::cancelAll();
        mOrientationChangeOngoing = true;
        mRootItemFinalPhaseDone = false;
        mOrientationEffectFinished = false;
        emit q->aboutToChangeOrientation();
        emit q->aboutToChangeOrientation(orientation, mAnimateOrientationSwitch);
    }

    mOrientation = orientation;

    if (!mAnimateOrientationSwitch) {
        HbEffect::disable(mEffectItem);
        HbEffect::disable(&mGVWrapperItem);
    } else {
        HbEffect::enable(mEffectItem);
        HbEffect::enable(&mGVWrapperItem);
    }

    // Sets graphicsview rotation effect to either 90/270 degrees
    updateRotationEffects();

    //For mirroring case
    changeSceneSize();

#ifdef Q_OS_SYMBIAN
    updateForegroundOrientationPSKey();
#endif

    HbEffect::start(mEffectItem, "rootItemFirstPhase", q, "rootItemFirstPhaseDone");

    if (mAnimateOrientationSwitch) {
        if (mStatusBar) {
            HbEffect::start(mStatusBar, "statusbar", "disappear_orient");
        }
        if (mTitleBar) {
            HbEffect::start(mTitleBar, "titlebar", "disappear_orient");
        }
        if (mCurrentToolbar) {
            HbToolBarPrivate *toolBarD = HbToolBarPrivate::d_ptr(mCurrentToolbar);
            toolBarD->startDisappearOrientEffect();
        }
    }
}

void HbMainWindowPrivate::changeSceneSize()
{
    // no need to perform the scene size change if there's no (rotation) effect for graphicsview
    if (!mGVOrientationChangeEffectEnabled) {
        return;
    }

    Q_Q(HbMainWindow);
    HbDeviceProfile profile = HbDeviceProfile::profile(q);
    QSize pSize(profile.logicalSize());

    // For RightToLeft we need to switch sceneRect values.
    if (q->layoutDirection() == Qt::RightToLeft && q->orientation() == Qt::Vertical) {
        mScene->setSceneRect(0, 0, pSize.height(), pSize.width());
    } else if (q->layoutDirection() == Qt::RightToLeft && q->orientation() == Qt::Horizontal) {
        mScene->setSceneRect(0, 0, pSize.width(), pSize.height());
    } else {
        mScene->setSceneRect(0, 0, pSize.height(), pSize.width());
    }
    q->setScene(0);
    q->setScene(mScene);
}

void HbMainWindowPrivate::updateRotationEffects()
{
    Q_Q(HbMainWindow);

    // if this is the default orientation the there's no transform, orientation angle is 0
    if (mOrientation == mDefaultOrientation) {
        mOrientationAngle = 0;
        return;
    }

    HbDeviceProfile o = HbDeviceProfile::profile(q);
    mOrientationAngle = o.orientationAngle();

    // Effects for GraphicsView (HbMainWindow)
    bool ret(true);
    bool ret2(true);
    // wrapper item holds the degree value for optimization purposes (set effect only once)
    if (mOrientationAngle == 270 && (mGVWrapperItem.transformDegrees != 270)) {
        mGVWrapperItem.transformDegrees = 270;
        ret = HbEffectInternal::add(&mGVWrapperItem, "toDefaultOrientation270", "toDefault");
        ret2 = HbEffectInternal::add(&mGVWrapperItem, "toRotatedOrientation270", "toRotated");
    } else if (mOrientationAngle == 90 && (mGVWrapperItem.transformDegrees != 90)) {
        mGVWrapperItem.transformDegrees = 90;
        ret = HbEffectInternal::add(&mGVWrapperItem, "toDefaultOrientation90", "toDefault");
        ret2 = HbEffectInternal::add(&mGVWrapperItem, "toRotatedOrientation90", "toRotated");
    }

    mGVOrientationChangeEffectEnabled = true;
    if (!ret || !ret2) {
        HbEffectInternal::remove(&mGVWrapperItem);
        mGVOrientationChangeEffectEnabled = false;
    }
}

qreal HbMainWindowPrivate::rotation() const
{
    return mOrientationAngle;
}

void HbMainWindowPrivate::select(const HbDeviceProfile &profile, HbDeviceProfile *oldGlobalProfile)
{
    // this window has a window-specific profile, do not override with global change
    if (!mCurrentProfile.isNull() && oldGlobalProfile) {
        return;
    }

    HbDeviceProfile oldProfile = oldGlobalProfile ? *oldGlobalProfile : mCurrentProfile;
    // only change mainwindow's profile if it's a non-global change
    if (!oldGlobalProfile) {
        mCurrentProfile = profile;
        mAlternateProfile = HbDeviceProfile(mCurrentProfile.alternateProfileName());
    }

    // if setting the same orientation send the profile changed event here since the setTransformedOrientation won't do it
    if ((mOrientation == profile.orientation()) &&
            mClippingItem->size() == profile.logicalSize()) {
        HbDeviceProfileChangedEvent *event = new HbDeviceProfileChangedEvent(profile, oldProfile);
        broadcastEvent(event);
    }

    mForceSetOrientation = true;
    setTransformedOrientation(profile.orientation(), false);
    mForceSetOrientation = false;
}

HbDeviceProfile HbMainWindowPrivate::profile() const
{
    return adjustedProfile(mCurrentProfile);
}

HbDeviceProfile HbMainWindowPrivate::adjustedProfile(const HbDeviceProfile &profile) const
{
    HbDeviceProfile result = profile;
    if (!result.isNull() && result.orientation() != mOrientation) {
        if (mAlternateProfile.isNull()) {
            mAlternateProfile = HbDeviceProfile(profile.alternateProfileName());
            return mAlternateProfile;
        } else {
            return mAlternateProfile;
        }
    }
    return result;
}


void HbMainWindowPrivate::orientationEffectFinished(const HbEffect::EffectStatus &status)
{
    Q_UNUSED(status);
    Q_Q(HbMainWindow);

    mOrientationEffectFinished = true;
    QSize newSize;

    HbDeviceProfile o = HbDeviceProfile::profile(q);
    newSize = o.logicalSize();

    // do some sanity checking for the size got from device profile
    if (newSize.isNull() || ((newSize.width()*newSize.height()) < QVga_res)) {  // the resolution must be at least QVGA..
        hbWarning("Orient. change error: size from device profile is faulty!");
    }

    q->setSceneRect(0, 0, newSize.width(), newSize.height());

    if (mBgItem) {
        mBgItem->updateBackgroundImage();
    }

    // re-layouting, skip if size does not change
    if (mOrientationChangeOngoing || mClippingItem->size() != newSize) {
        mClippingItem->resize(newSize);
        if (mScreenshotItem) {
            mScreenshotItem->resize(newSize);
        }
        mLayoutRect = QRectF(QPointF(0, 0), newSize);

        // reset transformation
        q->resetTransform();

        // if not default rotation, rotate to the defined angle no matter what the effect did
        if (mOrientation != mDefaultOrientation) {
            q->rotate(mOrientationAngle);
        }

        // handle actual orientation change only if the orientation really changes (not just a resize)
        if (mOrientationChangeOngoing) {
            // signal only if layout changes (=orientation changes)
            // Background item is re-layouted from device profile changed event
            emit q->orientationChanged(mOrientation);
            HbDeviceProfile oldProfile = HbDeviceProfile(o.alternateProfileName());
            HbDeviceProfileChangedEvent *event = new HbDeviceProfileChangedEvent(o, oldProfile);
            broadcastEvent(event);
        }
    }
    updateOrientationChangeStatus();
}

void HbMainWindowPrivate::rootItemFirstPhaseDone(const HbEffect::EffectStatus &status)
{
    Q_UNUSED(status)
    Q_Q(HbMainWindow);

    if (mOrientation == mDefaultOrientation) {
        HbEffect::start(&mGVWrapperItem, "toDefault", q, "orientationEffectFinished");
    } else {
        HbEffect::start(&mGVWrapperItem, "toRotated", q, "orientationEffectFinished");
    }
}

void HbMainWindowPrivate::rootItemFinalPhaseDone(const HbEffect::EffectStatus &status)
{
    Q_UNUSED(status);

    if (mOrientationChangeOngoing) {
        mRootItemFinalPhaseDone = true;
        if (mEffectItem) {
            HbEffect::enable(mEffectItem);
            // make sure effect does not leave anything in wrong state
            mEffectItem->setOpacity(1.0f);
            mEffectItem->resetTransform();
        }

        HbEffect::enable(&mGVWrapperItem);

        postIdleEvent(HbMainWindowPrivate::IdleOrientationFinalEvent);
        updateOrientationChangeStatus();
    }
}

void HbMainWindowPrivate::updateOrientationChangeStatus()
{
    if (mOrientationChangeOngoing && mRootItemFinalPhaseDone && mOrientationEffectFinished) {
        mOrientationChangeOngoing = false;
    }
}

void HbMainWindowPrivate::addOrientationChangeEffects()
{
    // Effects for root item
    // If effect loading fails, remove both effects.
    bool ret = HbEffectInternal::add(mEffectItem, "rootitem_orientation_firstPhase", "rootItemFirstPhase");
    bool ret2 = HbEffectInternal::add(mEffectItem, "rootitem_orientation_finalPhase", "rootItemFinalPhase");

    if (!ret || !ret2) {
        HbEffectInternal::remove(mEffectItem);
    }

    mOrientationChangeEffectItems.append(mEffectItem);
    mOrientationChangeEffectItems.append(&mGVWrapperItem);
}

void HbMainWindowPrivate::addViewEffects()
{
    // Register the view switch animations from the theme.
    // Use HbEffectInternal and the HB_ prefix to prevent general overriding of these effects.
    // Instead, view switch effects can be overridden on a per-instance basis.
    const char *vsType[] = { "HB_view", "HB_view", "HB_view", "HB_view" };
    const char *vsPath[] = { "view_show_normal", "view_hide_normal", "view_show_back", "view_hide_back" };
    const char *vsEvent[] = { "show", "hide", "show_back", "hide_back" };
    bool ok = HbEffectInternal::add(vsType, vsPath, vsEvent, 4);
    if (!ok) {
        hbWarning("HbMainWindow: addViewEffects: atomic registration for show/hide effects failed");
    }

    // Register the alternative default.
    const char *vsAltPath[] = { "view_show_normal_alt", "view_hide_normal_alt", "view_show_back_alt", "view_hide_back_alt" };
    const char *vsAltEvent[] = { "show_alt", "hide_alt", "show_alt_back", "hide_alt_back" };
    ok = HbEffectInternal::add(vsType, vsAltPath, vsAltEvent, 4);
    if (!ok) {
        hbWarning("HbMainWindow: addViewEffects: atomic registration for alternative show/hide effects failed");
    }

    // Register titlebar effects.
    // These should be overridable in general (so we use HbEffect and no HB_ prefix).
    const char *tbType[] = { "titlebar", "titlebar", "titlebar", "titlebar" };
    const char *tbPath[] = { "titlebar_disappear", "titlebar_appear", "titlebar_orient_disappear", "titlebar_orient_appear" };
    const char *tbEvent[] = { "disappear", "appear",  "disappear_orient", "appear_orient" };
    ok = HbEffectInternal::add(tbType, tbPath, tbEvent, 4);
    if (!ok) {
        hbWarning("HbMainWindow: addViewEffects: atomic registration for titlebar effects failed");
    }

    // Register statusbar effects.
    const char *sbType[] = { "statusbar", "statusbar", "statusbar", "statusbar" };
    const char *sbPath[] = { "statusbar_disappear",  "statusbar_appear", "statusbar_orient_disappear", "statusbar_orient_appear" };
    ok = HbEffectInternal::add(sbType, sbPath, tbEvent, 4);
    if (!ok) {
        hbWarning("HbMainWindow: addViewEffects: atomic registration for statusbar effects failed");
    }

    // Register faded background effects.
    const char *fiType[] = { "fadeitem", "fadeitem"};
    const char *fiPath[] = { "fadeitem_disappear",  "fadeitem_appear" };
    const char *fiEvent[] = { "disappear",  "appear" };
    ok = HbEffectInternal::add(fiType, fiPath, fiEvent, 2);
    if (!ok) {
        hbWarning("HbMainWindow: addViewEffects: atomic registration for fadeitem effects failed");
    }
}

/*
    Updates UI according to current view and sync it with the tab bar.
*/
void HbMainWindowPrivate::_q_viewChanged()
{
    Q_Q(HbMainWindow);

    HbView *view = qobject_cast<HbView *>(mViewStackWidget->currentWidget());
    if (view) {
        if (!HbViewPrivate::d_ptr(view)->mVisited) {
            HbViewPrivate::d_ptr(view)->mVisited = true;
            // connect signals when the view is first time visited
            QObject::connect(view, SIGNAL(titleChanged(QString)), q, SLOT(_q_viewTitleChanged(QString)));
            QObject::connect(view, SIGNAL(toolBarChanged()), q, SLOT(_q_viewToolBarChanged()));
            QObject::connect(view, SIGNAL(dockWidgetChanged()), q, SLOT(_q_viewDockWidgetChanged()));
            QObject::connect(view, SIGNAL(visibleItemsChanged()), q, SLOT(_q_viewVisibleItemsChanged()));
            QObject::connect(view, SIGNAL(contentFullScreenChanged()), q, SLOT(_q_contentFullScreenChanged()));
        }

        mTitleBar->titlePane()->setText(view->title());
        addToolBarToLayout(HbViewPrivate::d_ptr(view)->toolBar);
        addDockWidgetToLayout(HbViewPrivate::d_ptr(view)->dockWidget);
        updateVisibleItems();
    } else {
        mTitleBar->titlePane()->setText(QString());
    }

    emit q->currentViewChanged(view);
}

void HbMainWindowPrivate::_q_viewRemoved(QGraphicsWidget *widget)
{
    Q_Q(HbMainWindow);
    widget->disconnect(q);

    HbView *view = qobject_cast<HbView *>(widget);
    if (view) {
        // Reset view visit flag
        HbViewPrivate::d_ptr(view)->mVisited = false;
        removeToolBarFromLayout(HbViewPrivate::d_ptr(view)->toolBar);
        removeDockWidgetFromLayout(HbViewPrivate::d_ptr(view)->dockWidget);
    }
}


void HbMainWindowPrivate::_q_viewTitleChanged(const QString &title)
{
    Q_Q(HbMainWindow);
    if (q->sender() == mViewStackWidget->currentWidget()) {
        mTitleBar->titlePane()->setText(title);
    }
}

void HbMainWindowPrivate::_q_viewToolBarChanged()
{
    Q_Q(HbMainWindow);
    HbView *view = qobject_cast<HbView *>(q->sender());
    if (view) {
        if (view == q->currentView()) {
            addToolBarToLayout(HbViewPrivate::d_ptr(view)->toolBar);
        }
    }
}

void HbMainWindowPrivate::_q_viewDockWidgetChanged()
{
    Q_Q(HbMainWindow);
    HbView *view = qobject_cast<HbView *>(q->sender());
    if (view) {
        if (view == q->currentView()) {
            addDockWidgetToLayout(HbViewPrivate::d_ptr(view)->dockWidget);
        }
    }
}

/*
    Launches the menu of the current view at given pos.
*/
void HbMainWindowPrivate::_q_launchMenu(const QPointF &pos)  // TODO - pos unused!
{
    Q_Q(HbMainWindow);
    Q_UNUSED(pos);
    mMenuView = q->currentView();
    if (mMenuView) {
        HbMenu *menu = mMenuView->menu();
        if (!menu->isEmpty()) {
            q->connect(menu, SIGNAL(aboutToClose()), q, SLOT(_q_restoreTitlePane()));
            menu->setTimeout(HbPopup::NoTimeout);
            menu->open(this, SLOT(menuClosed()));
        } else {
            _q_restoreTitlePane();
        }
    }
}

void HbMainWindowPrivate::menuClosed()
{
    if (mMenuView) {
        mMenuView->setFocus();
    }
    _q_restoreTitlePane();
}

/*
  Restore the mode of the title pane after menu is closed.
 */
void HbMainWindowPrivate::_q_restoreTitlePane()
{
    mTitleBar->titlePane()->resetMode();
    mTitleBar->titlePane()->updatePrimitives();
}

/*
    Updates the changed visible items in view.
*/
void HbMainWindowPrivate::_q_viewVisibleItemsChanged()
{
    Q_Q(HbMainWindow);
    HbView *view = qobject_cast<HbView *>(q->sender());
    if (view && view == q->currentView()) {
        updateVisibleItems();
    }
}

/*
    Updates the layout of current view when fullscreen
    property has been toggled.
*/
void HbMainWindowPrivate::_q_contentFullScreenChanged()
{
    Q_Q(HbMainWindow);
    HbView *view = qobject_cast<HbView *>(q->sender());
    if (view && view == q->currentView()) {
        mClippingItem->decoratorVisibilityChanged();
    }
}

/*
    Applies fade effect on the whole screen at \a zValue.

    \sa unfadeScreen
*/
void HbMainWindowPrivate::fadeScreen(qreal zValue)
{
    initFadeItem();
    mFadeItem->setZValue(zValue);
    if (!mFadeItem->isVisible()) {
        mFadeItem->show();
        HbEffect::start(mFadeItem, "fadeitem", "appear");
    }
}

/*
    Removes fade effect from the screen.

    \sa fadeScreen
*/
void HbMainWindowPrivate::unfadeScreen()
{
    initFadeItem();
    if (mFadeItem->isVisible()) {
       HbEffect::start(mFadeItem, "fadeitem", "disappear", this, "fadeItemEffectFinished");
    }
}

/*
    Creates the fade item if not yet done.
*/
void HbMainWindowPrivate::initFadeItem()
{
    if (!mFadeItem) {
        mFadeItem = new HbFadeItem;
        mFadeItem->setZValue(HbPrivate::FadingItemZValue);
        mFadeItem->hide();
        mScene->addItem(mFadeItem);
    }
}

/*
    Hides the fade item after disappear effect is finished
*/
void HbMainWindowPrivate::fadeItemEffectFinished(const HbEffect::EffectStatus &status)
{
    Q_UNUSED(status);
    mFadeItem->setVisible(false);
}

/*!
  Creates the screenshot item if not yet done.
*/
void HbMainWindowPrivate::initScreenshotItem()
{
    if (!mScreenshotItem) {
        Q_Q(HbMainWindow);
        mScreenshotItem = new HbScreenshotItem(q);
        mScreenshotItem->hide();
        mScreenshotItem->resize(mClippingItem->size());
        mScene->addItem(mScreenshotItem);
    }
}

/*!
  Returns the screenshot item, creating it if not yet done.
 */
HbScreenshotItem *HbMainWindowPrivate::screenshotItem()
{
    if (!mScreenshotItem) {
        initScreenshotItem();
    }
    return mScreenshotItem;
}

/*
    Updates visible items according to flags.
*/
void HbMainWindowPrivate::updateVisibleItems()
{
    Q_Q(HbMainWindow);

    HbView *view = q->currentView();
    if (view) {
        const Hb::SceneItems visibleItems(view->visibleItems());

        // set statusbar and titlebar flag
        HbView::HbViewFlags flags = view->viewFlags();
        flags = (visibleItems & Hb::StatusBarItem) ?
                    flags &~ HbView::ViewStatusBarHidden :
                    flags |  HbView::ViewStatusBarHidden;
        flags = (visibleItems & Hb::TitleBarItem) ?
                    flags &~ HbView::ViewTitleBarHidden :
                    flags |  HbView::ViewTitleBarHidden;
        view->setViewFlags( flags ); // also handles updating of the navigation button

        // ToolBar is a special case, since it depens on the current view's toolbar
        if (visibleItems & Hb::ToolBarItem) {
            if (q->currentView()) {
                addToolBarToLayout(HbViewPrivate::d_ptr(q->currentView())->toolBar);
            }
        } else {
            if (mCurrentToolbar) {
                removeToolBarFromLayout(mCurrentToolbar);
            }
        }

        // DockWidget is a special case, since it depens on the current view's dock widget
        if (visibleItems & Hb::DockWidgetItem) {
            if (q->currentView()) {
                addDockWidgetToLayout(HbViewPrivate::d_ptr(view)->dockWidget);
            }
        } else {
            if (mCurrentDockWidget) {
                removeDockWidgetFromLayout(mCurrentDockWidget);
            }
        }

        mClippingItem->decoratorVisibilityChanged();
    }
}

/*
    Called when theme changes.
*/
void HbMainWindowPrivate::_q_themeChanged()
{
    broadcastEvent(HbEvent::ThemeChanged);
}

static void informWidget(HbMainWindowPrivate::BroadcastItem &bcItem, QGraphicsWidget *widget);

/*
    Sends event of \eventType to all items in the \inform list
    excluding ones in the \a ignoreItems list.

    Invible widgets will be added onto \deferredItems list which will be
    informed asynchronously.
*/
static void informItems(HbMainWindowPrivate::BroadcastItem &bcItem, QList<QGraphicsItem *> &inform)
{
    if (inform.count()) {
        foreach(QGraphicsItem * item, inform) {
            if (item->isWidget()) {
                QGraphicsWidget *widget = static_cast<QGraphicsWidget *>(item);
                if (item->isVisible()) {
                    informWidget(bcItem, widget);
                } else {
                    bcItem.mPending.append(QPointer<QGraphicsWidget>(widget));
                }
            }
        }
    }
}

/*
    Sends event of \eventType to \a widget.

    If the \a widget does not consume the event, it will be delivered to it's children.
*/
static void informWidget(HbMainWindowPrivate::BroadcastItem &bcItem, QGraphicsWidget *widget)
{
    QApplication::sendEvent(widget, bcItem.mEvent);

    // go through child items
    QList<QGraphicsItem *> children = widget->childItems();
    informItems(bcItem, children);
}


/*
    Broadcasts event of type \a eventType into all widgets in the scene.

    This is a convenince method for void broadcastEvent( QEvent * ).

*/
void HbMainWindowPrivate::broadcastEvent(int eventType)
{
    HbEvent *event = new HbEvent(eventType);
    broadcastEvent(event);
}

/*
    Broadcasts \a event into all widgets in the scene.

    First the high priotity widgets are informed. Currently, the active view is treated as
    a priority item and it will receive the event first.

    Invisible child items are added into mPending array which will be handled
    asynchronously.
*/
void HbMainWindowPrivate::broadcastEvent(QEvent *event)
{
    Q_Q(HbMainWindow);

    int type = event->type();
    bool previousEvent(mBroadcastItems.contains(type));

    BroadcastItem &broadcastItem = mBroadcastItems[type];
    broadcastItem.mEvent = event;

    // cancel previous requests
    if (previousEvent) {
        mBroadcastItems[type].mPending.clear();
    }

    // create high priority items
    QList<QGraphicsItem *> priorityItems;
    if (type == HbEvent::ThemeChanged) {
        if (q->currentView()) {
            priorityItems.append(q->currentView());
        }
    }

    // inform prority items now
    informItems(broadcastItem, priorityItems);

    // inform root items in the scene
    QList<QGraphicsItem *> sceneItems = mScene->items();
    QList<QGraphicsItem *> rootItems;
    foreach(QGraphicsItem * item, sceneItems) {
        if (!item->parentItem() && !priorityItems.contains(item)) {
            rootItems.append(item);
        }
    }
    informItems(broadcastItem, rootItems);

    // create asynchronous broadcast loop if needed.
    if (broadcastItem.mPending.count()) {
        Q_Q(HbMainWindow);
        QMetaObject::invokeMethod(q, "_q_continueBroadcasting", Qt::QueuedConnection, Q_ARG(int, type));
    } else {
        // no pending items left, remove the broadcast item
        delete mBroadcastItems[type].mEvent;
        mBroadcastItems.remove(type);
    }
}

/*
    For asynchronous event broadcasting.

    Items in the mPending array are informed.
*/
void HbMainWindowPrivate::_q_continueBroadcasting(int type)
{
    if (!mBroadcastItems.contains(type)) {
        // cancelled or items informed synchronously.
        return;
    }

    BroadcastItem &broadcastItem = mBroadcastItems[type];

    // take a copy and then clear the pending items
    QList<HbMainWindowPrivate::QGraphicsWidgetPtr> informItems = broadcastItem.mPending;
    broadcastItem.mPending.clear();

    foreach(QPointer<QGraphicsWidget> widgetPtr, informItems) {
        if (!widgetPtr.isNull()) {
            informWidget(broadcastItem, widgetPtr.data());
        }
    }

    // create another broadcast loop if needed.
    if (broadcastItem.mPending.count()) {
        Q_Q(HbMainWindow);
        QMetaObject::invokeMethod(q, "_q_continueBroadcasting", Qt::QueuedConnection, Q_ARG(int, type));
    } else {
        // no pending items left, remove the broadcast item
        delete mBroadcastItems[type].mEvent;
        mBroadcastItems.remove(type);
    }
}

void HbMainWindowPrivate::postIdleEvent(int eventId)
{
    Q_Q(HbMainWindow);
    HbEvent *idleEvent = new HbEvent(eventId);
    QCoreApplication::postEvent(q, idleEvent, Qt::LowEventPriority - 100); // magic low event priority
}

void HbMainWindowPrivate::_q_delayedConstruction()
{
    if (!mDelayedConstructionHandled) {
        Q_Q(HbMainWindow);
        mDelayedConstructionHandled = true;

        if (initializeInputs) {
            initializeInputs = false;
            HbInputMethod::initializeFramework(*qApp);
        }

        addOrientationChangeEffects();
        addViewEffects();
        HbView *view = qobject_cast<HbView *>(mViewStackWidget->currentWidget());
        if (view) {
            HbViewPrivate::d_ptr(view)->delayedConstruction();
        }
        mClippingItem->delayedConstruction();

        connect(hbInstance->theme(), SIGNAL(changed()),
                q, SLOT(_q_themeChanged()));

        connect(q, SIGNAL(currentViewChanged(HbView *)),
                mClippingItem, SLOT(currentViewChanged(HbView *)));

        mTitleBar->delayedConstruction();
        connect(mTitleBar->titlePane(), SIGNAL(visibilityChanged()),
                mClippingItem, SLOT(decoratorVisibilityChanged()));
        connect(mTitleBar, SIGNAL(titleBarStateChanged()),
                mClippingItem, SLOT(decoratorVisibilityChanged()));

        mStatusBar->delayedConstruction();
        connect(mStatusBar, SIGNAL(activated(const QList<IndicatorClientInfo> &)),
                mTitleBar, SIGNAL(activated(const QList<IndicatorClientInfo> &)));
        connect(mStatusBar, SIGNAL(deactivated(const QList<IndicatorClientInfo> &)),
                mTitleBar, SIGNAL(deactivated(const QList<IndicatorClientInfo> &)));
        connect(mStatusBar, SIGNAL(allActivated(const QList<IndicatorClientInfo> &)),
                mTitleBar, SIGNAL(allActivated(const QList<IndicatorClientInfo> &)));
        connect(mStatusBar, SIGNAL(updated(const QList<IndicatorClientInfo> &)),
                mTitleBar, SIGNAL(updated(const QList<IndicatorClientInfo> &)));

        initFadeItem();

#ifdef Q_OS_SYMBIAN
        mDevDlgConnectHelper = new HbDeviceDialogConnectHelper(this);
        connect(mDevDlgConnectHelper, SIGNAL(sessionEstablished(RHbDeviceDialogClientSession *)),
                this, SLOT(deviceDialogConnectionReady(RHbDeviceDialogClientSession *)));
        mDevDlgConnectHelper->connect();
#endif //Q_OS_SYMBIAN

        _q_viewReady();

        postIdleEvent(HbMainWindowPrivate::IdleEvent);
    }
}

void HbMainWindowPrivate::_q_viewReady()
{
    Q_Q(HbMainWindow);
    emit q->viewReady();
}

QGraphicsWidget *HbMainWindowPrivate::element(HbMainWindowPrivate::Element element) const
{
    if (element == HbMainWindowPrivate::RootItem) {
        return mRootItem;
    } else if (element == HbMainWindowPrivate::ViewportItem) {
        return mClippingItem;
    } else if (element == HbMainWindowPrivate::BackgroundItem) {
        return mBgItem;
    }
    return 0;
}

void HbMainWindowPrivate::addBackgroundItem()
{
    Q_Q(HbMainWindow);
    if (!mBgItem && mRootItem) {
        mBgItem = new HbBackgroundItem(q, mRootItem);
        mBgItem->setZValue(HbPrivate::BackgroundZValue);
    }
}

void HbMainWindowPrivate::removeBackgroundItem()
{
    if (mBgItem && mRootItem) {
        delete mBgItem;
        mBgItem = 0;
    }
}

void HbMainWindowPrivate::setViewportSize(const QSizeF &newSize)
{
    mClippingItem->resize(newSize);
    mLayoutRect = QRectF(QPointF(0, 0), newSize);
    if (mScreenshotItem) {
        mScreenshotItem->resize(newSize);
    }
}

QSizeF HbMainWindowPrivate::viewPortSize() const
{
    return mClippingItem->size();
}

/*
    Sets the obscured state of the window and emits a signal if nessasary.
*/
void HbMainWindowPrivate::setObscuredState(bool state)
{
    Q_Q(HbMainWindow);
    if (state == true) {
        if (mObscuredState == false) {
            mObscuredState = true;
            emit q->obscured();
        }
    } else {
        if (mObscuredState == true) {
            mObscuredState = false;
            emit q->revealed();
        }
    }
}

bool HbMainWindowPrivate::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type()) {
        case QEvent::ApplicationActivate:
            mMainWindowActive = true;
#ifdef Q_OS_SYMBIAN           
            updateForegroundOrientationPSKey();
#endif           
            break;
        case QEvent::ApplicationDeactivate:
            mMainWindowActive = false;
            break;
        default:
            break;
    }
    return QObject::eventFilter(watched, event);
}

#if defined(Q_WS_X11)
bool HbMainWindowPrivate::x11HandleShowEvent(QShowEvent *)
{
    Q_Q(HbMainWindow);
    Display *dpy = QX11Info::display();
    WId id = q->effectiveWinId();
    XWindowAttributes attr;
    if (XGetWindowAttributes(dpy, id, &attr)) {
        long allEventMask = attr.all_event_masks;
        allEventMask |= VisibilityChangeMask;
        XSelectInput(dpy, id, allEventMask);
    }
    return false;
}
#endif //Q_WS_X11

#ifdef Q_OS_SYMBIAN
void HbMainWindowPrivate::updateForegroundOrientationPSKey()
{
    Q_Q(HbMainWindow);
    // Publish the orientation to device dialog server but only if the
    // app has focus (do nothing if we are called while the app is in
    // background).
    RProcess process;    
    if (process.SecureId().iId != deviceDialogUid.iUid) {
        if (mMainWindowActive) {
            if (mDevDlgClientSession && !mPendingPsPublish) {
                int orie = mOrientation;
                if (!mAutomaticOrientationSwitch) {
                    orie |= KHbFixedOrientationMask;
                }
                mDevDlgClientSession->SendSyncRequest(EHbSrvPublishOrientation, orie);
            } else if (mDevDlgClientSession && mPendingPsPublish) {
                mDevDlgClientSession->SendSyncRequest(EHbSrvPublishOrientation, mPendingOrientationValue);
                mPendingPsPublish = false;
                mPendingOrientationValue = 0;
            } else if (!mDevDlgClientSession && !mPendingPsPublish) {
                mPendingOrientationValue = mOrientation;
                if (!mAutomaticOrientationSwitch) {
                    mPendingOrientationValue |= KHbFixedOrientationMask;
                }
                mPendingPsPublish = true;
            }
        }
        // Keep also wserv up-to-date. This must be done even if we do
        // not have focus.
#ifdef HB_WSERV_HAS_RENDER_ORIENTATION
        CCoeEnv *env = CCoeEnv::Static();
        if (env) {
            TRenderOrientation orientationForWserv = EDisplayOrientationAuto;
            if (!mAutomaticOrientationSwitch) {
                orientationForWserv = (TRenderOrientation)
                    HbOrientationStatus::mapOriToRenderOri(mOrientation);
            }
            env->WsSession().IndicateAppOrientation(orientationForWserv);
        }
#endif
    } else {
        // This is the device dialog server. Just keep wserv up-to-date.
#ifdef HB_WSERV_HAS_RENDER_ORIENTATION
        CCoeEnv *env = CCoeEnv::Static();
        if (env) {
            env->WsSession().IndicateAppOrientation(EDisplayOrientationIgnore);
        }
#endif
    }
    process.Close();
}

void HbMainWindowPrivate::deviceDialogConnectionReady(RHbDeviceDialogClientSession *clientSession)
{
    mDevDlgClientSession = clientSession;
    if (mPendingPsPublish) {
        updateForegroundOrientationPSKey();
    }
}
#endif //Q_OS_SYMBIAN

/*!
  Any event filter installed via QCoreApplication::setEventFilter has to obey two rules:

  - Proper chaining must be done (i.e. calling the previously set event filter).

  - When there is no previous filter, this function must be called instead of
    just returning false.  This makes sure that certain events are suppressed
    (i.e. not processed by Qt, but seen by all Orbit filters).

    \internal
*/
bool HbMainWindowPrivate::shouldStopEvent(void *message, long *result)
{
#if defined(Q_OS_SYMBIAN) && defined(HB_DDS_KEEPS_SURFACE)
    QSymbianEvent *symEvent = static_cast<QSymbianEvent *>(message);
    if (symEvent->type() == QSymbianEvent::WindowServerEvent) {
        const TWsEvent *wsEvent = symEvent->windowServerEvent();
        if (wsEvent->Type() == EEventWindowVisibilityChanged) {
            // Window Server visibility events must be suppressed in the Device
            // Dialog Server so Qt (QApplication) cannot see them.
            RProcess process;
            bool isDDS = TUid(process.SecureId()) == deviceDialogUid;
            process.Close();
            return isDDS;
        }
    }
#else
    Q_UNUSED(message);
    Q_UNUSED(result);
#endif

    return false;
}
