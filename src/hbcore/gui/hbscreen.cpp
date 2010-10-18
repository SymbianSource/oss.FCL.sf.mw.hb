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

#include "hbscreen_p.h"
#include "hbstyle.h"
#include "hbtoolbar.h"
#include "hbtoolbar_p.h"
#include "hbdockwidget.h"
#include "hbdockwidget_p.h"
#include "hbview.h"
#include "hbcontentwidget_p.h"
#include "hbmainwindow.h"
#include "hbtooltip.h"
#include "hbgraphicsscene.h"
#include "hbevent.h"
#include "hbmainwindow_p.h"
#include <QGraphicsLayout>
#include <qcoreapplication.h>

/*!
  \class HbScreen

  \brief HbScreen represents the item that is the parent of all
  graphics items excluding the background.

  \internal
*/

HbScreen::HbScreen() :
    HbWidget(),
    mStack(0),
    mTb(0),
    mDock(0),
    mToolBarOrientation(0),
    mDelayedConstructionHandled(false),
    mScreenPolished(false)
{
}

void HbScreen::setStackWidget(HbContentWidget *stack)
{
    if (mStack != stack) {
        if (mStack) {
            HbStyle::setItemName(mTb, QString());
            HbStyle::setItemName(mDock, QString());
        }
        mStack = stack;
    }
}

void HbScreen::setToolBar(HbToolBar *tb)
{
    if (mTb != tb) {
        if (mTb) {
            disconnect(&HbToolBarPrivate::d_ptr(mTb)->core, 0, this, 0);
            HbStyle::setItemName(mTb, QString());
        }
        mTb = tb;
        if (mTb) {
            setToolBarOrientation(mTb->orientation());
            if (mDelayedConstructionHandled) {
                connect(&HbToolBarPrivate::d_ptr(mTb)->core, SIGNAL(orientationChanged()), this, SLOT(toolBarOrientationChanged()));
                connect(&HbToolBarPrivate::d_ptr(mTb)->core, SIGNAL(visibilityChanged()), this, SLOT(decoratorVisibilityChanged()));
            }
        } else {
            mToolBarOrientation = 0;
        }
    }
}

void HbScreen::setDockWidget(HbDockWidget *dock)
{
    if (mDock != dock) {
        if (mDock) {
            disconnect(&HbDockWidgetPrivate::d_ptr(mDock)->core, 0, this, 0);
            HbStyle::setItemName(mDock, QString());
        }
        mDock = dock;
        if (mDock && mDelayedConstructionHandled) {
            connect(&HbDockWidgetPrivate::d_ptr(mDock)->core, SIGNAL(visibilityChanged()), this, SLOT(decoratorVisibilityChanged()));
        }
    }
}

void setName(QGraphicsWidget *item, const QString &name)
{
    if (item && (item->isVisible())) {
        HbStyle::setItemName(item, name);
    } else {
        HbStyle::setItemName(item, QString());
    }
}

void HbScreen::setToolBarName(bool forced)
{
    //we handle toolbar as a special case to
    //avoid unnecessary complications in setName function.
    if ((mTb->isVisible() || forced) && mTb->actions().count()) {
        HbStyle::setItemName(mTb, "toolbar");
    } else {
        HbStyle::setItemName(mTb, QString());
    }
}

void HbScreen::polish(HbStyleParameters &params)
{
    if (!mScreenPolished) {
        mScreenOrientation = mainWindow()->orientation();
        connect(mainWindow(), SIGNAL(orientationChanged(Qt::Orientation)), this, SLOT(screenOrientationChanged(Qt::Orientation)));
    }

    if (mStack) {
        HbView *currentView = qobject_cast<HbView *>(mStack->currentWidget());
        if (currentView && currentView->isContentFullScreen()) {
            setName(mStack, "mainfull");
        } else {
            setName(mStack, "main");
        }
    }

    if (mTb) {
        bool hiddenForEffects = HbToolBarPrivate::d_ptr(mTb)->mOrientationEffectsRunning;
        setToolBarName(hiddenForEffects);
    }

    if (mDock) {
        setName(mDock, "dock");
    }

    mScreenPolished = true;
    HbWidget::polish(params);


    HbMainWindow *w = mainWindow();
    HbMainWindowPrivate::d_ptr(w)->postIdleEvent(HbMainWindowPrivate::IdleOrientationEvent);
}

void HbScreen::delayedConstruction()
{
    if (mDelayedConstructionHandled) {
        return;
    }
    mDelayedConstructionHandled = true;
    if (mTb) {
        connect(&HbToolBarPrivate::d_ptr(mTb)->core, SIGNAL(orientationChanged()), this, SLOT(toolBarOrientationChanged()));
        connect(&HbToolBarPrivate::d_ptr(mTb)->core, SIGNAL(visibilityChanged()), this, SLOT(decoratorVisibilityChanged()));
    }
    if (mDock) {
        connect(&HbDockWidgetPrivate::d_ptr(mDock)->core, SIGNAL(visibilityChanged()), this, SLOT(decoratorVisibilityChanged()));
    }
}

Qt::Orientation HbScreen::toolBarOrientation() const
{
    return (Qt::Orientation)mToolBarOrientation;
}

Qt::Orientation HbScreen::screenOrientation() const
{
    return mScreenOrientation;
}

bool HbScreen::contentUnderTitleBar() const
{
    bool contentUnderTitleBar = false;
    if (mStack) {
        HbView *currentView = qobject_cast<HbView *>(mStack->currentWidget());
        if (currentView && !(currentView->viewFlags() & HbView::ViewDisableRelayout)) {
            if (currentView->viewFlags() & (HbView::ViewTitleBarMinimized
                                            | HbView::ViewTitleBarFloating | HbView::ViewTitleBarHidden)) {
                contentUnderTitleBar = true;
            }
        }
    }
    return contentUnderTitleBar;
}

bool HbScreen::contentUnderStatusBar() const
{
    bool contentUnderStatusBar = false;
    if (mStack) {
        HbView *currentView = qobject_cast<HbView *>(mStack->currentWidget());
        if (currentView && !(currentView->viewFlags() & HbView::ViewDisableRelayout)) {
            if (currentView->viewFlags() & (HbView::ViewStatusBarFloating
                                            | HbView::ViewStatusBarHidden)) {
                contentUnderStatusBar = true;
            }
        }
    }
    return contentUnderStatusBar;
}

bool HbScreen::titleBarMinimizable() const
{
    bool titleBarMinimizable = false;
    if (mStack) {
        HbView *currentView = qobject_cast<HbView *>(mStack->currentWidget());
        if (currentView && (currentView->viewFlags() & (HbView::ViewTitleBarMinimizable))) {
            titleBarMinimizable = true;
        }
    }
    return titleBarMinimizable;
}

void HbScreen::screenOrientationChanged(Qt::Orientation orientation)
{
    if (mScreenOrientation != orientation) {
        mScreenOrientation = orientation;
        repolish();
        prepareGeometryChange();
    }
}

void HbScreen::toolBarOrientationChanged()
{
    setToolBarOrientation(mTb->orientation());
}

void HbScreen::setToolBarOrientation(Qt::Orientation orientation)
{
    if (orientation != mToolBarOrientation) {
        if (mTb) {
            repolish();
        }
        mToolBarOrientation = orientation;
    }
}

void HbScreen::decoratorVisibilityChanged()
{
    HbMainWindow *window = mainWindow();
    // Do not repolish if orientation is about to change
    if (window && HbMainWindowPrivate::d_ptr(window)->mOrientationChangeOngoing
            && mScreenOrientation != HbMainWindowPrivate::d_ptr(window)->mOrientation) {
        return;
    }
    repolish();
}

void HbScreen::currentViewChanged(HbView *view)
{
    Q_UNUSED(view);
    HbToolTip::hideText(qobject_cast<HbGraphicsScene *>(scene()));
    repolish();
}

bool HbScreen::event(QEvent *e)
{
    if (e->type() == HbEvent::DeviceProfileChanged) {
        // suppress polishEvent() [which is called in HbWidget::event()] into repolish()
        repolish();
        return QGraphicsWidget::event(e);
    } else {
        return HbWidget::event(e);
    }
}
