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

#include <QGraphicsSceneMouseEvent>

#include <hbview.h>
#include <hbdeviceprofile.h>
#include <hbmenu.h>
#include <hbaction.h>
#include <hbwidgetfeedback.h>
#include <hbinstance.h>
#include <hbnamespace.h>
#include <hbpangesture.h>

#include "hbtitlebar_p.h"
#include "hbtitlebar_p_p.h"

#include "hbmainwindow_p.h"
#include "hbnavigationbutton_p.h"
#include "hbindicatorbutton_p.h"
#include "hbtitlebarhandle_p.h"
#include "hbtitlepane_p.h"

/*
    \class HbTitleBar
    \brief HbTitleBar is the container class for common decorators. Decorators are
    components that are always available even if the view contents change.
 */

HbTitleBarPrivate::HbTitleBarPrivate() : 
    mMainWindow(0), mTitlePane(0), mNavigationButton(0), mIndicatorButton(0),
    mTitleBarHandle(0), mTouchAreaItem(0)
{
}

void HbTitleBarPrivate::delayedConstruction()
{
    Q_Q(HbTitleBar);

    mTitlePane->delayedConstruction();
    mIndicatorButton->delayedConstruction();
    mNavigationButton->delayedConstruction();

#ifdef HB_EFFECTS
    HbEffect::add(
        QStringList() << "decorator" << "decorator" << "decorator",
        QStringList() << "decorator_pressed" <<  "decorator_released" << "decorator_latched",
        QStringList() << "pressed" << "released" <<  "latched");
#endif

    q->connect(mTitlePane, SIGNAL(launchPopup(QPointF)), mMainWindow, SLOT(_q_launchMenu(QPointF)));
    q->connect(q, SIGNAL(activated(const QList<IndicatorClientInfo> &)), 
        mIndicatorButton, SLOT(activate(const QList<IndicatorClientInfo> &)));
    q->connect(q, SIGNAL(deactivated(const QList<IndicatorClientInfo> &)), 
        mIndicatorButton, SLOT(deactivate(const QList<IndicatorClientInfo> &)));
    q->connect(mMainWindow, SIGNAL(currentViewChanged(HbView*)), q, SLOT(currentViewChanged(HbView*)));
    q->connect(mDefaultNavigationAction, SIGNAL(triggered()), qApp, SLOT(quit()));  

    q->setFlag(QGraphicsItem::ItemIsPanel, true);
}

void HbTitleBarPrivate::init()
{
    Q_Q(HbTitleBar);

    // create title pane
    mTitlePane = new HbTitlePane(q);
    mTitlePane->setZValue(HbPrivate::TitlePaneZValue);

    mIndicatorButton = new HbIndicatorButton(q);
    mNavigationButton = new HbNavigationButton(q);
    // add default quit action
    mDefaultNavigationAction = new HbAction(Hb::QuitNaviAction, q);
    HbIcon icon = HbIcon(mDefaultNavigationAction->icon().iconName());
    // the default action's icon should mirror in case of a layout direction change
    icon.setMirroringMode(HbIcon::LayoutDirection);
    mDefaultNavigationAction->setIcon(icon);

    mDefaultNavigationAction->setText("Quit");
    mNavigationButton->setAction(mDefaultNavigationAction);

    HbStyle::setItemName(q, "titlebar");
    HbStyle::setItemName(mTitlePane, "title");
    HbStyle::setItemName(mIndicatorButton, "status");
    HbStyle::setItemName(mNavigationButton, "back");
    
    mPreviousTitleBarProperties = 0; // view not yet ready

    QObject::connect(this->mTitlePane, SIGNAL(panRight()), q, SLOT(gestureRight()));
    QObject::connect(this->mTitlePane, SIGNAL(panLeft()), q, SLOT(gestureLeft()));
}

void HbTitleBarPrivate::initSceneEventFilters(HbView *view)
{
    Q_Q(HbTitleBar);
    if (view->viewFlags() & HbView::ViewTitleBarMinimizable) {
        if(!mTouchAreaItem) {
            // Install sceneEvent filter(s)
            mTouchAreaItem = q->style()->createPrimitive(HbStyle::P_TitleBar_toucharea, q);
            mTouchAreaItem->setAcceptedMouseButtons(Qt::LeftButton);
            mTouchAreaItem->installSceneEventFilter(q);
            QGraphicsObject *touchAreaItemGraphicsObject = static_cast<QGraphicsObject*>(mTouchAreaItem);
            touchAreaItemGraphicsObject->grabGesture(Qt::PanGesture);
        }
    } else { // Remove scene event filter
        if (mTouchAreaItem) {
            mTouchAreaItem->removeSceneEventFilter(q);
            delete mTouchAreaItem;
            mTouchAreaItem = 0;
        }
    }
}

void HbTitleBarPrivate::initTitleBarHandle(HbView *view)
{
    Q_Q(HbTitleBar);
    if (view->viewFlags() & HbView::ViewTitleBarMinimizable) {
        if (!mTitleBarHandle) { // Create title bar handle
            mTitleBarHandle = new HbTitleBarHandle(q);
            HbStyle::setItemName(mTitleBarHandle, "handle");
#ifdef HB_EFFECTS
            HbEffect::add(
                QStringList() << "titlebar" << "titlebar",
                QStringList() << "titlebar_minimize" <<  "titlebar_maximize",
                QStringList() << "minimize" << "maximize");
#endif
        }
    } else {
        if (mTitleBarHandle) { // Remove title bar handle
            delete mTitleBarHandle;
            mTitleBarHandle = 0;
#ifdef HB_EFFECTS
            HbEffect::remove("titlebar", "titlebar_minimize", "minimize");
            HbEffect::remove("titlebar", "titlebar_maximize", "maximize");
#endif
        }
    }
}

/*
    Constructor, the titlebar.
    \param mainWindow is the owner for this titlebar.
    The \a parent is an optional parameter.
*/
HbTitleBar::HbTitleBar(HbMainWindow *mainWindow, QGraphicsItem *parent /*= 0*/)
    : HbWidget(*new HbTitleBarPrivate, parent)
{
    Q_D(HbTitleBar);
    d->q_ptr = this;
    d->mMainWindow = mainWindow;
    d->init();
}

HbTitleBar::HbTitleBar(HbTitleBarPrivate &dd, HbMainWindow *mainWindow,
                       QGraphicsItem *parent)
    : HbWidget(dd, parent)
{
    Q_D(HbTitleBar);
    d->q_ptr = this;
    d->mMainWindow = mainWindow;
    d->init();
}

/*
    Destructor.
 */
HbTitleBar::~HbTitleBar()
{ 
    Q_D(HbTitleBar);
    // Remove scene event filter
    if (d->mTouchAreaItem) {
        d->mTouchAreaItem->removeSceneEventFilter(this);
        delete d->mTouchAreaItem;
        d->mTouchAreaItem = 0;
    }
}

/*
    Delayed constructor.
 */
void HbTitleBar::delayedConstruction()
{
       Q_D(HbTitleBar);
       d->delayedConstruction();
}

/*
    titlePane. Return titlepane decorator.
*/
HbTitlePane *HbTitleBar::titlePane() const
{
    Q_D(const HbTitleBar);
    return d->mTitlePane;
}

/*
    minimizable. Used to switch between layout with and without titlebar handle.
*/
bool HbTitleBar::minimizable() const
{
    Q_D(const HbTitleBar);
    if (!d->mMainWindow->currentView()) {
        return false;
    }
    return d->mMainWindow->currentView()->viewFlags() & HbView::ViewTitleBarMinimizable;
}

/*
    propertiesChanged. Do the repolish for titlebar and indicator group.
*/
void HbTitleBar::propertiesChanged()
{
    Q_D(HbTitleBar);
    if (d->mMainWindow->currentView()) {
        currentViewChanged(d->mMainWindow->currentView());
    }
}

/*
    position. Return titlebar position.
*/
HbTitleBar::Position HbTitleBar::position() const
{
    QTransform transf = transform();
    Position pos = Original;
    if ((qAbs(transf.dx()) > 1) && (qAbs(transf.dy()) < 1)) {
        pos = Minimized;
    } else if ((qAbs(transf.dx()) > 1) && (qAbs(transf.dy()) > 1)) {
        pos = DismissedUp;
    }

    return pos;
}

HbAction *HbTitleBar::navigationAction() const
{
    Q_D(const HbTitleBar);
    return d->mNavigationButton->action();
}

void HbTitleBar::setNavigationAction(HbAction *action)
{
    Q_D(HbTitleBar);
    d->mNavigationButton->setAction(action);
}

void HbTitleBar::setDefaultNavigationAction()
{
    Q_D(HbTitleBar);
    setNavigationAction(d->mDefaultNavigationAction);
}

/*
    gestureRight. Handles left-to-right flick.
            if(layoutDirection() == Qt::LeftToRight) {
*/

void HbTitleBar::gestureRight()
{
    Q_D(HbTitleBar);

    if (!minimizable()) {
        return;
    }
    HbWidgetFeedback::triggered(this, Hb::InstantFlicked);

    Position p(position());
    QSize screenSize = HbDeviceProfile::profile(this).logicalSize();
    QRectF handleRect = d->mTitleBarHandle->boundingRect();

    if (d->mMainWindow &&
        d->mIndicatorButton->isVisible() &&
        p == HbTitleBar::Original) {
#ifdef HB_EFFECTS
        //grabMouse(); // this prevents taps/gestures on top of animating titlebar
        QRectF extRect(scenePos().x(), 0.0, screenSize.width(), 10.0);
        HbEffect::start(this, "titlebar", "minimize", this, "effectFinished", QVariant(), extRect);
#else // no effects, just do the translation
        translate(screenSize.width()-handleRect.width(), 0);
#endif //HB_EFFECTS
    } 
}

/*
    gestureLeft. Handles right-to-left flick.
*/

void HbTitleBar::gestureLeft()
{
    Q_D(HbTitleBar);

    if (!minimizable()) {
        return;
    }
    HbWidgetFeedback::triggered(this, Hb::InstantFlicked);

    Position p(position());
    QSize screenSize = HbDeviceProfile::profile(this).logicalSize();
    QRectF handleRect = d->mTitleBarHandle->boundingRect();
    // only way to reliable find the position of titlebar is using
    // titlebar's transformation information
    if (d->mMainWindow &&
        d->mIndicatorButton->isVisible() &&
        p == HbTitleBar::Minimized) {
#ifdef HB_EFFECTS
        //grabMouse(); // this prevents taps/gestures on top of animating titlebar
        // effect translates widget from rect's right x-coordinate to left x-coordinate
        QRectF extRect(-handleRect.width(), 0.0, scenePos().x(), 10.0); // height not used in effect
        HbEffect::start(this, "titlebar", "maximize", this, "effectFinished", QVariant(), extRect);
#else // no effects, just do the translation
        translate(x()-scenePos().x(), y()-scenePos().y());
#endif //HB_EFFECTS
    } 
}

#ifdef HB_EFFECTS
void HbTitleBar::effectFinished(const HbEffect::EffectStatus &status)
{
    Q_D(HbTitleBar);
    //ungrabMouse();
    if (status.reason == Hb::EffectFinished) {
        // change titlebar property accordingly
        if (d->mMainWindow->currentView()) {
            if (position() == Minimized) {
                d->mMainWindow->currentView()->setViewFlags(
                    d->mMainWindow->currentView()->viewFlags() | HbView::ViewTitleBarMinimized);
                    d->mTitlePane->resetMode();
                    d->mTitlePane->updatePrimitives();
            } else {
                d->mMainWindow->currentView()->setViewFlags(
                    d->mMainWindow->currentView()->viewFlags() & (~HbView::ViewTitleBarMinimized));
            }
        }
        emit titleBarStateChanged();
    }
}
#endif //HB_EFFECTS

/*!
This slot is called when active HbView changes.
*/ 
void HbTitleBar::currentViewChanged(HbView *view)
{
    Q_D(HbTitleBar);
    // this can happpen when e.g. view is destroyed
    if (!view) {
        return;
    }
    QTransform transform;
    // first check if we shouldn't be minimized
    if (position() == HbTitleBar::Minimized) {
        if (!(view->viewFlags() & HbView::ViewTitleBarMinimizable)) {
            setTransform(transform); // this sets titlebar to (0,0)
        } else if ((view->viewFlags() & HbView::ViewTitleBarMinimizable) &&
            !(view->viewFlags() & HbView::ViewTitleBarMinimized)) {
            setTransform(transform); // this sets titlebar to (0,0)
        }
    } else if (view->viewFlags() & HbView::ViewTitleBarMinimized) {
        setTransform(transform); // this sets titlebar to (0,0)
        QSize screenSize = HbDeviceProfile::profile(this).logicalSize();
        if (layoutDirection() == Qt::LeftToRight) {
            translate(screenSize.width(), 0);
        } else {
            translate(-screenSize.width(), 0);
        }
    }

    // only do repolish if titlebar properties have changed
    if (d->mPreviousTitleBarProperties != view->viewFlags()) {
        d->initTitleBarHandle(view);
        d->initSceneEventFilters(view);
        d->mPreviousTitleBarProperties = view->viewFlags();
        d->mTitlePane->updatePrimitives();
        d->mIndicatorButton->updatePrimitives();
        d->mNavigationButton->updatePrimitives();
        repolish();
    }
    if (mainWindow() && mainWindow()->currentView()) {
        mainWindow()->currentView()->menu()->installEventFilter(d->mTitlePane);
    }
    d->mTitlePane->repolish();
}


void HbTitleBar::polish(HbStyleParameters &params)
{
    Q_D(HbTitleBar);

    // update handle visibility according to titlebar properties of the current view
    if (mainWindow() && mainWindow()->currentView()) {
        int viewFlags = mainWindow()->currentView()->viewFlags();
        if (viewFlags & HbView::ViewTitleBarMinimizable) {
            if (d->mTitleBarHandle) {
                d->mTitleBarHandle->setVisible(true);
            }
            d->mIndicatorButton->showHandleIndication(true);
        } else {
            d->mIndicatorButton->showHandleIndication(false);
        }
    }
    HbWidget::polish(params);
}

//filtering touchitem (titlebarhandle) mouse events.
bool HbTitleBar::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    Q_UNUSED(watched)

    Q_D(HbTitleBar);
    bool filterOutEvent = false;
    switch(event->type()){
    case QEvent::GraphicsSceneMousePress:
        HbWidgetFeedback::triggered(this, Hb::InstantPressed);
        event->accept(); //we need to catch the mouse release and move events also
        filterOutEvent = true;
        break;
    case QEvent::GraphicsSceneMouseRelease:
        HbWidgetFeedback::triggered(this, Hb::InstantClicked);
        if (d->mHandleDown) {
            d->mHandleDown = false;
            HbWidgetFeedback::triggered(this, Hb::InstantReleased);
        }
        filterOutEvent = true;
        break;
    case QEvent::GraphicsSceneMouseMove: {
        QGraphicsSceneMouseEvent *mouseEvent =
                static_cast<QGraphicsSceneMouseEvent*>(event);
        if (d->mTouchAreaItem && d->mTouchAreaItem->contains(
                d->mTouchAreaItem->mapFromScene(mouseEvent->scenePos()))) {
            if (!d->mHandleDown) {
                d->mHandleDown = true;
                HbWidgetFeedback::triggered(this, Hb::InstantPressed);
            }
        } else if (d->mHandleDown) {
            d->mHandleDown = false;
            HbWidgetFeedback::triggered(this, Hb::InstantReleased);
        }
        filterOutEvent = true;
        break;
    }
    case QEvent::Gesture: {
        QGestureEvent *gestureEvent = static_cast<QGestureEvent*>(event);
        if (HbPanGesture *pan = qobject_cast<HbPanGesture*>(gestureEvent->gesture(Qt::PanGesture))) {
            if(pan->state() == Qt::GestureUpdated || pan->state() == Qt::GestureFinished) {
                if(pan->sceneDelta().x() < -0) {
                    gestureLeft();
                }
                if(pan->sceneDelta().x() > 0) {
                    gestureRight();
                }
                gestureEvent->accept();
            }
        }
        filterOutEvent = true;
        break;
    }
    default:
        break;
    }
    return filterOutEvent;
}

/*!
    \reimp
*/
QGraphicsItem *HbTitleBar::primitive(const QString &itemName) const
{
    Q_D(const HbTitleBar);
    if (itemName == "") {
        return 0;
    } else {
        if (itemName == "title") {
            return d->mTitlePane;
        }
        else if (itemName == "status") {
            return d->mIndicatorButton;
        }
        else if (itemName == "back") {
            return d->mNavigationButton;
        }
        else if (itemName == "handle") {
            return d->mTitleBarHandle;
        }
        else if (itemName == "toucharea") {
            return d->mTouchAreaItem;
        } else {
            return 0;
        }
    }
}
