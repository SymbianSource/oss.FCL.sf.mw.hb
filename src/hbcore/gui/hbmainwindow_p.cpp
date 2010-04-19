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

#include <QApplication>
#include <QGraphicsLayout>
#include <QDebug>
#include "hbgraphicsscene.h"
#include "hbindicatorbutton_p.h"
#include "hbtitlebar_p.h"
#include "hbstatusbar_p.h"
#include "hbmainwindow_p.h"
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
#include "hbinputsettingproxy.h"
#include "hbinputmethod.h"
#include "hbfadeitem_p.h"
#include "hbcontentwidget_p.h"
#include "hbscreen_p.h"
#include "hbbackgrounditem_p.h"

#ifdef Q_OS_SYMBIAN
#include "hbnativewindow_sym_p.h"
#endif

#ifdef HB_GESTURE_FW
#include "hbmousepangesturerecognizer_p.h"
#include "hbswipegesturerecognizer_p.h"
#include "hbtapgesturerecognizer_p.h"
#include "hbtapandholdgesturerecognizer_p.h"
#endif

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
    mViewStackWidget(0),
    mTitleBar(0),
    mStatusBar(0),
    mFadeItem(0),
    mRootItem(0),    
    mAutomaticOrientationSwitch(true),
    mOrientationChangeOngoing(false),
    mOrientation(Qt::Vertical),
    mCurrentToolbar(0),
    mCurrentDockWidget(0),
    mVisibleItems(Hb::AllItems),
    mForceSetOrientation(false),
    mDelayedConstructionHandled(false),
    q_ptr(0),
    mTheTestUtility(0),
    mIdleEventHandled(false)
#ifdef Q_OS_SYMBIAN
    ,
    mNativeWindow(0)
#endif
{   
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
    if (HbDeviceProfile::profile(q).touch()) {
        // Touch devices doesn't have visible primary or middle soft key
        mVisibleItems &= ~Hb::MiddleSoftKeyItem;
        mVisibleItems &= ~Hb::PrimarySoftKeyItem;
    }

    mOrientation = hbInstance->orientation();
}

HbToolBar *HbMainWindowPrivate::toolBar() const
{
    HbView *view = qobject_cast<HbView *>(mViewStackWidget->currentWidget());
    return view?HbViewPrivate::d_ptr(view)->toolBar:0;
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
            if (toolBar->scene() != mScene && (q->isItemVisible(Hb::ToolBarItem))) { // check just to avoid warnings
                // update the toolbar to use the current orientation and layout direction
                if (toolBar->layoutDirection() != q->layoutDirection() &&
                    !toolBar->testAttribute(Qt::WA_SetLayoutDirection)){

                    toolBar->setLayoutDirection(q->layoutDirection());
                    toolBar->setAttribute(Qt::WA_SetLayoutDirection, false);
                }                
                HbToolBarPrivate *toolBarD = HbToolBarPrivate::d_ptr(toolBar);
                toolBarD->mDoLayout = false;
                if (mViewStackWidget->isSwitchingViews()) {
                    toolBarD->suppressNextAppearEffect();
                }
                mScene->addItem(toolBar); // top level
            }

            toolBar->setZValue(HbPrivate::ToolBarZValue);
            toolBar->setParentItem( mClippingItem );
            mClippingItem->setToolBar(toolBar);
            mCurrentToolbar = toolBar;
        }
    }
}

void HbMainWindowPrivate::removeToolBarFromLayout(HbToolBar *toolBar)
{
    Q_Q(HbMainWindow);
    if (mCurrentToolbar && mCurrentToolbar->scene() && mCurrentToolbar == toolBar) {
        mClippingItem->setToolBar( 0 );
        mScene->removeItem(mCurrentToolbar);
        mCurrentToolbar->disconnect(q);
        mCurrentToolbar = 0;
    }
}

HbDockWidget *HbMainWindowPrivate::dockWidget() const
{
    HbView *view = qobject_cast<HbView *>(mViewStackWidget->currentWidget());
    return view?HbViewPrivate::d_ptr(view)->dockWidget:0;
}

void HbMainWindowPrivate::addDockWidgetToLayout(HbDockWidget *dockWidget)
{
    Q_Q(HbMainWindow);
    if (mCurrentDockWidget != dockWidget) {

        // Remove old toolbar
        removeDockWidgetFromLayout(mCurrentDockWidget);

        // Add new toolbar
        if (dockWidget) {
            dockWidget->setParentItem(0);
            if (dockWidget->scene() != mScene && (q->isItemVisible(Hb::DockWidgetItem))) { // check just to avoid warnings
                mScene->addItem(dockWidget); // top level
            }
            
            dockWidget->setZValue(HbPrivate::DockWidgetZValue);

            dockWidget->setParentItem( mClippingItem );
            mClippingItem->setDockWidget(dockWidget);            
            mCurrentDockWidget = dockWidget;
        }
    }
}

void HbMainWindowPrivate::removeDockWidgetFromLayout(HbDockWidget *dockWidget)
{
    Q_Q(HbMainWindow);
    if (mCurrentDockWidget && mCurrentDockWidget->scene() && mCurrentDockWidget == dockWidget) {
        mClippingItem->setDockWidget( 0 );
        mScene->removeItem(mCurrentDockWidget);
        mCurrentDockWidget->disconnect(q);
        mCurrentDockWidget = 0;
    }
}

void HbMainWindowPrivate::setTransformedOrientation(Qt::Orientation orientation, bool animate)
{
    if ( (mOrientation == orientation) && !mForceSetOrientation && mEffectItem )
        return;
       
    Q_Q(HbMainWindow);


        // skip transition if graphicsview is not visible
    mAnimateOrientationSwitch = animate;
    if (!q->isVisible())
        mAnimateOrientationSwitch = false;

    // calling due to resize, orientation remains the same -> no signalling
    if ( !((mOrientation == orientation) && mForceSetOrientation) ) {
        // cancel all effects
        HbEffectInternal::cancelAll();
        mOrientationChangeOngoing = true;
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
    if (!mGVOrientationChangeEffectEnabled)
        return;

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
    // wrapper item holds the degree value for optimisation purposes (set effect only once)
    if (mOrientationAngle == 270 && (mGVWrapperItem.transformDegrees != 270) ) {
        mGVWrapperItem.transformDegrees = 270;
        ret = HbEffectInternal::add(&mGVWrapperItem, "toDefaultOrientation270", "toDefault");
        ret2 = HbEffectInternal::add(&mGVWrapperItem, "toRotatedOrientation270", "toRotated");
    }
    else if(mOrientationAngle == 90 &&(mGVWrapperItem.transformDegrees != 90)) {
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
        if(mAlternateProfile.isNull()) {
            mAlternateProfile = HbDeviceProfile(profile.alternateProfileName());
            return mAlternateProfile;
        }
        else {
            return mAlternateProfile;
        }
     }
     return result;
 }


void HbMainWindowPrivate::orientationEffectFinished(const HbEffect::EffectStatus& status)
{
    Q_Q(HbMainWindow);
    QSize newSize;

 
    
    HbDeviceProfile o = HbDeviceProfile::profile(q);
    newSize = o.logicalSize(); 

    // do some sanity checking for the size got from device profile
    if( newSize.isNull() || ((newSize.width()*newSize.height()) < QVga_res) ) { // the resolution must be at least QVGA..
        qWarning("Orient. change error: size from device profile is faulty!");
    }
    
    q->setSceneRect(0,0,newSize.width(),newSize.height());
    
    // re-layouting, skip if size does not change
    if (mClippingItem->size() != newSize) {
        mClippingItem->resize(newSize);
        
    // reset transformation
    q->resetTransform(); 
    
    // if not default rotation, rotate to the defined angle no matter what the effect did
    if( mOrientation != mDefaultOrientation)
        q->rotate(mOrientationAngle);

        // handle actual orientation change only if the orientation really changes (not just a resize)
        if (mOrientationChangeOngoing) {
            // signal only if layout changes (=orientation changes)
            // Background item is re-layouted from device profile changed event
            emit q->orientationChanged(mOrientation);
            HbDeviceProfile oldProfile = HbDeviceProfile(o.alternateProfileName());
            HbDeviceProfileChangedEvent *event = new HbDeviceProfileChangedEvent(o, oldProfile);
            broadcastEvent(event);
            // Also complete input orientation change
            HbInputSettingProxy::instance()->setScreenOrientation(mOrientation);
        }
    }

    if (status.reason == Hb::EffectCancelled) {
        mOrientationChangeOngoing = false;
        HbEffect::EffectStatus dummy;
        rootItemFinalPhaseDone(dummy);
    }
}


void HbMainWindowPrivate::rootItemFirstPhaseDone(const HbEffect::EffectStatus& status)
{
    Q_Q(HbMainWindow);

    if (status.reason == Hb::EffectCancelled) {
        // if using opacity effect make sure that opacity is 1 if first phase effect
        // is cancelled
        if (mEffectItem) {
            mEffectItem->setOpacity(1.0f);
            // also reset transform since root item first phase isn't the end state
            mEffectItem->resetTransform();
        }
        HbEffect::EffectStatus empty;
        orientationEffectFinished(empty);
        return;
    }

   if (mOrientation == mDefaultOrientation)
        HbEffect::start(&mGVWrapperItem,"toDefault", q, "orientationEffectFinished");
    else 
        HbEffect::start(&mGVWrapperItem,"toRotated", q, "orientationEffectFinished");
}

void HbMainWindowPrivate::rootItemFinalPhaseDone(const HbEffect::EffectStatus& status)
{
    Q_UNUSED(status);
        
    if (mEffectItem) {
        HbEffect::enable(mEffectItem);
        // make sure effect does not leave anything in wrong state
        mEffectItem->setOpacity(1.0f);
        mEffectItem->resetTransform();
    }

    HbEffect::enable(&mGVWrapperItem);

    postIdleEvent(HbMainWindowPrivate::IdleOrientationFinalEvent);
    mOrientationChangeOngoing = false;
}
	
void HbMainWindowPrivate::addOrientationChangeEffects()
{
    // Effects for root item
    // If effect loading fails, remove both effects.
	bool ret = HbEffectInternal::add(mEffectItem, "rootitem_orientation_firstPhase", "rootItemFirstPhase");
	bool ret2 = HbEffectInternal::add(mEffectItem, "rootitem_orientation_finalPhase", "rootItemFinalPhase");
    
    if (!ret || !ret2)
        HbEffectInternal::remove(mEffectItem);

    mOrientationChangeEffectItems.append(mEffectItem);
    mOrientationChangeEffectItems.append(&mGVWrapperItem);
}

void HbMainWindowPrivate::addViewEffects()
{
    // Register the view switch animations from the theme.
    bool ok = HbEffectInternal::add(
        QStringList() << "HB_view" << "HB_view" << "HB_view" << "HB_view",
        QStringList() << "view_show_normal" << "view_hide_normal" <<  "view_show_back" << "view_hide_back",
        QStringList() << "show" << "hide" << "show_back" << "hide_back");
    if (!ok) {
        qWarning("HbMainWindow: addViewEffects: atomic registration for show/hide effects failed");
    }

    // Register the alternative default.
    ok = HbEffectInternal::add(
        QStringList() << "HB_view" << "HB_view" << "HB_view" << "HB_view",
        QStringList() << "view_show_normal_alt" << "view_hide_normal_alt" << "view_show_back_alt" << "view_hide_back_alt",
        QStringList() << "show_alt" << "hide_alt" << "show_alt_back" << "hide_alt_back");
    if (!ok) {
        qWarning("HbMainWindow: addViewEffects: atomic registration for alternative show/hide effects failed");
    }

    // Register titlebar effects.
    ok = HbEffect::add(
        QStringList() << "titlebar" << "titlebar" << "titlebar" << "titlebar",
        QStringList() << "titlebar_disappear" <<  "titlebar_appear" << "titlebar_orient_disappear" << "titlebar_orient_appear",
        QStringList() << "disappear" << "appear" <<  "disappear_orient" << "appear_orient");
    if (!ok) {
        qWarning("HbMainWindow: addViewEffects: atomic registration for titlebar effects failed");
    }

    // Register statusbar effects.
    ok = HbEffect::add(
        QStringList() << "statusbar" << "statusbar" << "statusbar" << "statusbar",
        QStringList() << "statusbar_disappear" <<  "statusbar_appear" << "statusbar_orient_disappear" << "statusbar_orient_appear",
        QStringList() << "disappear" << "appear" <<  "disappear_orient" << "appear_orient");
    if (!ok) {
        qWarning("HbMainWindow: addViewEffects: atomic registration for statusbar effects failed");
    }
}

/*
    Updates UI according to current view and sync it with the tab bar.
*/
void HbMainWindowPrivate::_q_viewChanged(int index)
{
    Q_Q(HbMainWindow);

    HbView *view = qobject_cast<HbView *>(mViewStackWidget->currentWidget());
    if (view) {        
        if(!HbViewPrivate::d_ptr(view)->mVisited) {
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

    emit q->currentViewIndexChanged(index);
    emit q->currentViewChanged(view);
}

void HbMainWindowPrivate::_q_viewRemoved(QGraphicsWidget *widget)
{
    Q_Q(HbMainWindow);
    widget->disconnect(q);

    HbView *view = qobject_cast<HbView*>(widget);
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
        if(view == q->currentView()) {
            addToolBarToLayout(HbViewPrivate::d_ptr(view)->toolBar);
        }
    }
}

void HbMainWindowPrivate::_q_viewDockWidgetChanged()
{
    Q_Q(HbMainWindow);
    HbView *view = qobject_cast<HbView *>(q->sender());
    if (view) {
        if(view == q->currentView()) {
            addDockWidgetToLayout(HbViewPrivate::d_ptr(view)->dockWidget);
        }
    }
}

/*
    Launches the menu of the current view at given pos.
*/
void HbMainWindowPrivate::_q_launchMenu(const QPointF &pos)
{
    Q_Q(HbMainWindow);
    QPointer<HbView> view = q->currentView();
    if (view) {
        HbMenu *menu = view->menu();

        if (!menu->isEmpty()) {
            q->connect(menu, SIGNAL(aboutToClose()), q, SLOT(_q_restoreTitlePane()));
            menu->setTimeout(HbPopup::NoTimeout);
            QPointer<HbMenu> menuAlive(menu);
            menu->exec(pos);
            if (menuAlive) {
                q->disconnect(menu, SIGNAL(aboutToClose()), q, SLOT(_q_restoreTitlePane()));
            }
            if (view) {
                view->setFocus();
            }
        } else {
            _q_restoreTitlePane();
        }
    }
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
    if (mFadeItem) {
        mFadeItem->setZValue(zValue);
        mFadeItem->show();
    }
}

/*
    Removes fade effect from the screen.

    \sa fadeScreen
*/
void HbMainWindowPrivate::unfadeScreen()
{
    if (mFadeItem) {
        mFadeItem->hide();
    }
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
        view->setTitleBarVisible(visibleItems & Hb::TitleBarItem); // also handles updating of the navigation button
        mTitleBar->titlePane()->setVisible(visibleItems & Hb::TitlePaneItem);
        
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
    broadcastEvent( HbEvent::ThemeChanged );
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
    if ( inform.count() ) {
        foreach( QGraphicsItem *item, inform ) {
            if ( item->isWidget() ) {
                QGraphicsWidget *widget = static_cast<QGraphicsWidget *>(item);
                if ( item->isVisible() ) {
                    informWidget(bcItem, widget);
                } else {
                    bcItem.mPending.append(QPointer<QGraphicsWidget>( widget ));
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
void HbMainWindowPrivate::broadcastEvent( int eventType )
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
void HbMainWindowPrivate::broadcastEvent( QEvent *event )
{
    Q_Q(HbMainWindow);

    int type = event->type();
    bool previousEvent(mBroadcastItems.contains( type ));

    BroadcastItem& broadcastItem = mBroadcastItems[type];
    broadcastItem.mEvent = event;

    // cancel previous requests
    if (previousEvent) {
        mBroadcastItems[type].mPending.clear();
    }

    // create high priority items
    QList<QGraphicsItem *> priorityItems;
    if ( type == HbEvent::ThemeChanged ) {
        if ( q->currentView() ) {
            priorityItems.append( q->currentView() );
        }
    }
    
    // inform prority items now
    informItems(broadcastItem, priorityItems);

    // inform root items in the scene
    QList<QGraphicsItem *> sceneItems = mScene->items();
    QList<QGraphicsItem *> rootItems;
    foreach( QGraphicsItem *item, sceneItems ) {
        if ( !item->parentItem() && !priorityItems.contains(item)) {
            rootItems.append( item );
        }
    }
    informItems(broadcastItem, rootItems);

    // create asynchronous broadcast loop if needed.
    if ( broadcastItem.mPending.count() ) {
        Q_Q(HbMainWindow);
        QMetaObject::invokeMethod( q, "_q_continueBroadcasting", Qt::QueuedConnection, Q_ARG(int, type) );
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

    BroadcastItem& broadcastItem = mBroadcastItems[type];

    // take a copy and then clear the pending items
    QList<HbMainWindowPrivate::QGraphicsWidgetPtr> informItems = broadcastItem.mPending;
    broadcastItem.mPending.clear();

    foreach( QPointer<QGraphicsWidget> widgetPtr, informItems ) {
        if ( !widgetPtr.isNull() ) {
            informWidget(broadcastItem, widgetPtr.data());
        }
    }

    // create another broadcast loop if needed.
    if ( broadcastItem.mPending.count() ) {
        Q_Q(HbMainWindow);
        QMetaObject::invokeMethod( q, "_q_continueBroadcasting", Qt::QueuedConnection, Q_ARG(int, type) );
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
    if (!mDelayedConstructionHandled){
        Q_Q(HbMainWindow);
        mDelayedConstructionHandled = true;

        if (initializeInputs) {
            initializeInputs = false;
            HbInputMethod::initializeFramework(*qApp);
            initGestures();
        }
        HbInputSettingProxy::instance()->initializeOrientation(mOrientation);

        addOrientationChangeEffects();
        addViewEffects();
        mClippingItem->delayedConstruction();

        mViewStackWidget->delayedConstruction();

        connect(hbInstance->theme(), SIGNAL(changed()),
                q, SLOT(_q_themeChanged()) );

        connect(q, SIGNAL(currentViewChanged(HbView*)),
                mClippingItem, SLOT(currentViewChanged(HbView*)));

        connect(mTitleBar->titlePane(), SIGNAL(visibilityChanged()),
                mClippingItem, SLOT(decoratorVisibilityChanged()));
        connect(mTitleBar, SIGNAL(titleBarStateChanged()),
                mClippingItem, SLOT(decoratorVisibilityChanged()));

         connect(mStatusBar, SIGNAL(notificationCountChanged(int)),
                 mTitleBar, SIGNAL(notificationCountChanged(int)));

        mFadeItem = new HbFadeItem;
        mFadeItem->setZValue(HbPrivate::FadingItemZValue);
        mFadeItem->hide();
        mScene->addItem(mFadeItem);

        _q_viewReady();

        postIdleEvent(HbMainWindowPrivate::IdleEvent);
    }
}

void HbMainWindowPrivate::_q_viewReady()
{
        Q_Q(HbMainWindow);
        emit q->viewReady();
}

void HbMainWindowPrivate::addBackgroundItem()
{
    Q_Q(HbMainWindow);
    if (!mBgItem && mRootItem) {
        mBgItem = new HbBackgroundItem(q);
        mBgItem->setZValue(HbPrivate::BackgroundZValue);
        mBgItem->setParentItem(mRootItem);
    }
}

void HbMainWindowPrivate::removeBackgroundItem()
{
    if (mBgItem && mRootItem) {
        delete mBgItem;
        mBgItem = 0;
    }
}

void HbMainWindowPrivate::initGestures()
{
#ifdef HB_GESTURE_FW
// ### TODO enable this (and remove the similar calls in HbApplication ctor) once QGestureManager problems are fixed
/*    QGestureRecognizer::unregisterRecognizer(Qt::TapGesture);
    QGestureRecognizer::unregisterRecognizer(Qt::TapAndHoldGesture);
    QGestureRecognizer::unregisterRecognizer(Qt::PanGesture);
    QGestureRecognizer::unregisterRecognizer(Qt::SwipeGesture);

    QGestureRecognizer::registerRecognizer(new HbTapGestureRecognizer);
    QGestureRecognizer::registerRecognizer(new HbTapAndHoldGestureRecognizer);
    QGestureRecognizer::registerRecognizer(new HbMousePanGestureRecognizer);
    QGestureRecognizer::registerRecognizer(new HbSwipeGestureRecognizer);*/
#endif
}

// end of file
