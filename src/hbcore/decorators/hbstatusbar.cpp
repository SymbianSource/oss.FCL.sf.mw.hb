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

#include <QTime>

#include <hbtextitem.h>
#include <hbmainwindow.h>
#include <hbview.h>
#include <hbextendedlocale.h>
#include <hbevent.h>

#include "hbstatusbar_p.h"
#include "hbstatusbar_p_p.h"
#include "hbstyleoptionstatusbar_p.h"
#include "hbsignalindicator_p.h"
#include "hbbatteryindicator_p.h"
#include "hbindicatorgroup_p.h"

#if defined(Q_OS_SYMBIAN)
#include "hbindicatorsym_p.h"
#include <bacntf.h>  // CEnvironmentChangeNotifier
#include <coemain.h> // EActivePriorityLogonA
#else
#include "hbindicatorwin32_p.h"
#endif // defined(Q_OS_SYMBIAN)

const int clockUpdateDelay = 10000; // 10 s

/*
    \class HbStatusBar
    \brief HbStatusBar is the class implementing statusbar decorator.
    Statusbar is a container for two indicator groups (left and right),
    a clock which is located in the middle of indicator groups and for
	a battery and signal status indicators.
 */

HbStatusBarPrivate::HbStatusBarPrivate() : 
    mTimeTextItem(0),
    mSignalIndicator(0),
    mBatteryIndicator(0),
    mNotificationIndicatorGroup(0),
    mSettingsIndicatorGroup(0),
    mMainWindow(0),
    mPreviousProperties(0),
    mIndicatorPrivate(0)
{
#if defined(Q_OS_SYMBIAN)
    // Register for system environment changes
    TCallBack envCallback(EnvChangeCallback, this);

    mEnvChangeNotifier =
        CEnvironmentChangeNotifier::NewL(EActivePriorityLogonA, envCallback);

    mEnvChangeNotifier->Start();
#endif
}

HbStatusBarPrivate::~HbStatusBarPrivate()
{
    mIndicatorPrivate->stopListen();     
    delete mIndicatorPrivate;

#if defined(Q_OS_SYMBIAN)
    // Stop environment change notifications
    if (mEnvChangeNotifier)
        {
        mEnvChangeNotifier->Cancel();
        delete mEnvChangeNotifier;
        }
#endif
}

void HbStatusBarPrivate::delayedConstruction()
{
    Q_Q(HbStatusBar);

    mSignalIndicator->delayedConstruction();
    mBatteryIndicator->delayedConstruction();
    mNotificationIndicatorGroup->delayedConstruction();
    mSettingsIndicatorGroup->delayedConstruction();

    q->connect(mIndicatorPrivate, SIGNAL(activated(const QList<IndicatorClientInfo> &)),
        mNotificationIndicatorGroup, SLOT(activate(const QList<IndicatorClientInfo> &)));
    q->connect(mIndicatorPrivate, SIGNAL(updated(const QList<IndicatorClientInfo> &)),
        mNotificationIndicatorGroup, SLOT(update(const QList<IndicatorClientInfo> &)));
    q->connect(mIndicatorPrivate, SIGNAL(allActivated(const QList<IndicatorClientInfo> &)),
        mNotificationIndicatorGroup, SLOT(activateAll(const QList<IndicatorClientInfo> &)));
    q->connect(mIndicatorPrivate, SIGNAL(deactivated(const QList<IndicatorClientInfo> &)),
        mNotificationIndicatorGroup, SLOT(deactivate(const QList<IndicatorClientInfo> &)));

    q->connect(mIndicatorPrivate, SIGNAL(activated(const QList<IndicatorClientInfo> &)),
        mSettingsIndicatorGroup, SLOT(activate(const QList<IndicatorClientInfo> &)));
    q->connect(mIndicatorPrivate, SIGNAL(updated(const QList<IndicatorClientInfo> &)),
        mSettingsIndicatorGroup, SLOT(update(const QList<IndicatorClientInfo> &)));
    q->connect(mIndicatorPrivate, SIGNAL(allActivated(const QList<IndicatorClientInfo> &)),
        mSettingsIndicatorGroup, SLOT(activateAll(const QList<IndicatorClientInfo> &)));
    q->connect(mIndicatorPrivate, SIGNAL(deactivated(const QList<IndicatorClientInfo> &)),
        mSettingsIndicatorGroup, SLOT(deactivate(const QList<IndicatorClientInfo> &)));

    q->connect(mIndicatorPrivate, SIGNAL(activated(const QList<IndicatorClientInfo> &)),
        q, SIGNAL(activated(const QList<IndicatorClientInfo> &)));
    q->connect(mIndicatorPrivate, SIGNAL(allActivated(const QList<IndicatorClientInfo> &)),
        q, SIGNAL(allActivated(const QList<IndicatorClientInfo> &)));
    q->connect(mIndicatorPrivate, SIGNAL(deactivated(const QList<IndicatorClientInfo> &)),
        q, SIGNAL(deactivated(const QList<IndicatorClientInfo> &)));

    q->connect(mIndicatorPrivate, SIGNAL(activated(const QList<IndicatorClientInfo> &)),
        q, SLOT(_q_indicatorsChanged()));
    q->connect(mIndicatorPrivate, SIGNAL(allActivated(const QList<IndicatorClientInfo> &)),
        q, SLOT(_q_indicatorsChanged()));
    q->connect(mIndicatorPrivate, SIGNAL(updated(const QList<IndicatorClientInfo> &)),
        q, SLOT(_q_indicatorsChanged()));
    q->connect(mIndicatorPrivate, SIGNAL(deactivated(const QList<IndicatorClientInfo> &)),
        q, SLOT(_q_indicatorsChanged()));

    mClockTimerId = q->startTimer(clockUpdateDelay);
    mIndicatorPrivate->startListen();

    q->grabGesture(Qt::TapGesture);
    q->grabGesture(Qt::TapAndHoldGesture);
    q->grabGesture(Qt::PanGesture);
    q->grabGesture(Qt::SwipeGesture);
    q->grabGesture(Qt::PinchGesture);
}

void HbStatusBarPrivate::init()
{
    Q_Q(HbStatusBar);

    HbStyle::setItemName(q, "statusbar");

    mSignalIndicator = new HbSignalIndicator(q);
    q->style()->setItemName(mSignalIndicator, "signal");
    mBatteryIndicator = new HbBatteryIndicator(q);
    q->style()->setItemName(mBatteryIndicator, "battery");

    mNotificationIndicatorGroup = new HbIndicatorGroup(HbIndicatorGroup::NotificationType, q);
    q->style()->setItemName(mNotificationIndicatorGroup, "notificationindicators");

    mSettingsIndicatorGroup = new HbIndicatorGroup(HbIndicatorGroup::SettingsType, q);
    q->style()->setItemName(mSettingsIndicatorGroup, "settingsindicators");

    mIndicatorPrivate = new HbIndicatorPrivate;
    mIndicatorPrivate->init();

    QObject::connect(mSignalIndicator, SIGNAL(levelChanged()), q, SLOT(_q_signalLevelChanged()));
    QObject::connect(mBatteryIndicator, SIGNAL(levelChanged()), q, SLOT(_q_batteryLevelChanged()));
}

void HbStatusBarPrivate::_q_signalLevelChanged()
{
    Q_Q(HbStatusBar);
    emit q->contentChanged(HbStatusBar::SignalLevelChanged);
}

void HbStatusBarPrivate::_q_batteryLevelChanged()
{
    Q_Q(HbStatusBar);
    HbStatusBar::ContentChangeFlags flags = HbStatusBar::BatteryLevelChanged;
    if (mBatteryIndicator->isCharging()) {
        flags |= HbStatusBar::BatteryCharging;
    }
    emit q->contentChanged(flags);
}

void HbStatusBarPrivate::_q_indicatorsChanged()
{
    Q_Q(HbStatusBar);
    emit q->contentChanged(HbStatusBar::IndicatorsChanged);
}

void HbStatusBarPrivate::updateTime()
{
    Q_Q(HbStatusBar);

    QString timeFormat;
    // set the time format accordingly
    if (HbExtendedLocale().timeStyle() == HbExtendedLocale::Time12) {
        timeFormat.clear();
        timeFormat.insert(0, "hh:mm ap");
    } else {
        timeFormat.clear();
        timeFormat.insert(0, "hh:mm");
    }

    QTime current = QTime::currentTime();

    // set time, using a proper formatting
    QString oldTimeText = mTimeText;
    mTimeText = current.toString(timeFormat);

    q->updatePrimitives();

    if (mTimeText != oldTimeText) {
        emit q->contentChanged(HbStatusBar::TimeChanged);
    }
}

#if defined(Q_OS_SYMBIAN)
TInt HbStatusBarPrivate::EnvChangeCallback(TAny *aObject)
{
    // Return value for functions used as TCallBack objects should be EFalse
    // unless the function is intended to be called again from a timer.
    return static_cast<HbStatusBarPrivate*>(aObject)->DoEnvChange();
}

TInt HbStatusBarPrivate::DoEnvChange()
{
    const TInt changes(mEnvChangeNotifier->Change());
    if ((changes & EChangesLocale) || (changes & EChangesSystemTime))
        {
        updateTime();
        }
    return EFalse ;
}
#endif

/*
    Constructor, the statusbar.
    The \a parent is an optional parameter.
*/
HbStatusBar::HbStatusBar(HbMainWindow *mainWindow, QGraphicsItem *parent)
    : HbWidget(*new HbStatusBarPrivate, parent)
{
    Q_D(HbStatusBar);
    d->q_ptr = this;
    d->mMainWindow = mainWindow;
    d->init();
    createPrimitives();
}

/*
    Destructor.
 */
HbStatusBar::~HbStatusBar()
{ 
    Q_D(HbStatusBar);
    
    if (d->mClockTimerId != 0) {
        killTimer(d->mClockTimerId);
        d->mClockTimerId = 0;
    }
}

/*
    Delayed constructor.
 */
void HbStatusBar::delayedConstruction()
{   
    Q_D(HbStatusBar);
    d->delayedConstruction();
}

void HbStatusBar::propertiesChanged()
{
    Q_D(HbStatusBar);
    if (d->mMainWindow && d->mMainWindow->currentView()) {
        currentViewChanged(d->mMainWindow->currentView());
    }
}

void HbStatusBar::createPrimitives()
{
    Q_D(HbStatusBar);

    d->mTimeTextItem = style()->createPrimitive(HbStyle::P_StatusBar_timetext, this);
    d->setBackgroundItem(HbStyle::P_StatusBar_background);

    d->updateTime();
}

void HbStatusBar::updatePrimitives()
{
    Q_D(HbStatusBar);
    HbStyleOptionStatusBar option;
    
    initStyleOption(&option);
    style()->updatePrimitive(backgroundItem(), HbStyle::P_StatusBar_background, &option);
    style()->updatePrimitive(d->mTimeTextItem, HbStyle::P_StatusBar_timetext, &option);
}

/*
This slot is called when active HbView changes.
*/ 
void HbStatusBar::currentViewChanged(HbView *view)
{
    Q_D(HbStatusBar);
    // this can happpen when e.g. view is destroyed
    if (!view) {
        return;
    }

    d->mNotificationIndicatorGroup->currentViewChanged(view);
    d->mSettingsIndicatorGroup->currentViewChanged(view);

    // only do repolish if properties have changed
    if (d->mPreviousProperties != view->viewFlags()) {
        d->mPreviousProperties = view->viewFlags();
        repolish();
        updatePrimitives();
    }
}

void HbStatusBar::initStyleOption(HbStyleOptionStatusBar *option) const
{
    const Q_D(HbStatusBar);
	HbWidget::initStyleOption(option);

    option->timeText = d->mTimeText;

    if (mainWindow() && mainWindow()->currentView()) {
        if (mainWindow()->currentView()->viewFlags() & HbView::ViewStatusBarTransparent) {
            option->transparent = true;
        }
    }
}

void HbStatusBar::timerEvent(QTimerEvent *event)
{
    Q_D(HbStatusBar);
    if (event->timerId() == d->mClockTimerId) {
        d->updateTime(); // get current time
    }
}

/*!
    \reimp
*/
QGraphicsItem *HbStatusBar::primitive(const QString &itemName) const
{
    const Q_D(HbStatusBar);
    if (itemName == "") {
        return 0;
    } else {
        if (itemName == "background") {
            return this->backgroundItem();
        }
        else if (itemName == "timetext") {
            return d->mTimeTextItem;
        }
        else if (itemName == "signal") {
            return d->mSignalIndicator;
        }
        else if (itemName == "battery") {
            return d->mBatteryIndicator;
        }
        else if (itemName == "notificationindicators") {
            return d->mNotificationIndicatorGroup;
        }
        else if (itemName == "settingsindicators") {
            return d->mSettingsIndicatorGroup;
        } else {
            return 0;
        }
    }
}

/*!
    \reimp
*/
void HbStatusBar::gestureEvent(QGestureEvent *event)
{
    Q_UNUSED(event);
    // all gesture events accepted by default
}

/*!
    \reimp
*/
bool HbStatusBar::event(QEvent *e)
{
    Q_D(HbStatusBar);
    if (e->type() == HbEvent::SleepModeEnter) {
        if (d->mClockTimerId != 0) {
            killTimer(d->mClockTimerId);
            d->mClockTimerId = 0;
        }
    } else if (e->type() == HbEvent::SleepModeExit) {
        d->updateTime();
        d->mClockTimerId = startTimer(clockUpdateDelay);
    }
    return HbWidget::event(e);
}

#include "moc_hbstatusbar_p.cpp"
