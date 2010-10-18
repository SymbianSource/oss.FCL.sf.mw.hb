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
#include "hbdevicedialogserverstatus_p.h"

#if defined(Q_OS_SYMBIAN)
#include "hbindicatorsym_p.h"
#include <bacntf.h>  // CEnvironmentChangeNotifier
#include <coemain.h> // EActivePriorityLogonA
#else
#include "hbindicatorwin32_p.h"
#endif // defined(Q_OS_SYMBIAN)

/*
    \class HbStatusBar
    \brief HbStatusBar is the class implementing statusbar decorator.
    Statusbar is a container for two indicator groups (left and right),
    a clock which is located in the middle of indicator groups and for
    a battery and signal status indicators.
 */

HbStatusBarPrivate::HbStatusBarPrivate() : 
    mClockTimerId(0),
    mTimeTextItem(0),
    mSignalIndicator(0),
    mBatteryIndicator(0),
    mNotificationIndicatorGroup(0),
    mSettingsIndicatorGroup(0),
    mMainWindow(0),
    mPreviousProperties(0),
    mIndicatorPrivate(0),
    mServerStatus(0)
{
#if defined(Q_OS_SYMBIAN)
    // Register for system environment changes
    TCallBack envCallback(EnvChangeCallback, this);

    mEnvChangeNotifier =
        CEnvironmentChangeNotifier::NewL(EActivePriorityLogonA, envCallback);

    mEnvChangeNotifier->Start();
#endif
    mServerStatus = new HbDeviceDialogServerStatus(false);
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
    delete mServerStatus;
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
    q->connect(mIndicatorPrivate, SIGNAL(updated(const QList<IndicatorClientInfo> &)),
        q, SIGNAL(updated(const QList<IndicatorClientInfo> &)));

    q->connect(mIndicatorPrivate, SIGNAL(activated(const QList<IndicatorClientInfo> &)),
        q, SLOT(_q_indicatorsChanged()));
    q->connect(mIndicatorPrivate, SIGNAL(allActivated(const QList<IndicatorClientInfo> &)),
        q, SLOT(_q_indicatorsChanged()));
    q->connect(mIndicatorPrivate, SIGNAL(updated(const QList<IndicatorClientInfo> &)),
        q, SLOT(_q_indicatorsChanged()));
    q->connect(mIndicatorPrivate, SIGNAL(deactivated(const QList<IndicatorClientInfo> &)),
        q, SLOT(_q_indicatorsChanged()));

    q->connect(mMainWindow, SIGNAL(currentViewChanged(HbView*)), q, SLOT(currentViewChanged(HbView*)));

    q->connect(mMainWindow, SIGNAL(revealed()), q, SLOT(startClockTimer()));

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
    HbStyle::setItemName(mSignalIndicator, "signal");
    mBatteryIndicator = new HbBatteryIndicator(q);
    HbStyle::setItemName(mBatteryIndicator, "battery");

    mNotificationIndicatorGroup = new HbIndicatorGroup(HbIndicatorGroup::NotificationType, q);
    HbStyle::setItemName(mNotificationIndicatorGroup, "notificationindicators");

    mSettingsIndicatorGroup = new HbIndicatorGroup(HbIndicatorGroup::SettingsType, q);
    HbStyle::setItemName(mSettingsIndicatorGroup, "settingsindicators");

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

QString HbStatusBarPrivate::timeFormat()
{
    QString timeFormat;
    HbExtendedLocale extendedLocale = HbExtendedLocale();

    // time separator
    timeFormat.append("hh");
    // Index0 HH Index1 MM Index2 SS Index3
    int indexOfHourMinuteSeparator(1);
    timeFormat.append(extendedLocale.timeSeparator(indexOfHourMinuteSeparator));
    timeFormat.append("mm");

    if (extendedLocale.timeStyle() == HbExtendedLocale::Time24){
        // 24-hour format
        return timeFormat;
    }

    // 12-hour time format
    QString amPm = "ap";
    QString space = " ";
    if (extendedLocale.amPmSpace()){
        // case: space between time & amPm text
        if (extendedLocale.amPmSymbolPosition() == HbExtendedLocale::After){
            amPm.prepend(space);
            timeFormat.append(amPm);
        }else if (extendedLocale.amPmSymbolPosition() == HbExtendedLocale::Before){
            amPm.append(space);
            timeFormat.prepend(amPm);
        }
    } else {
        // case: no space between time & amPm text
        if (extendedLocale.amPmSymbolPosition() == HbExtendedLocale::After){
            timeFormat.append(amPm);
        }else if (extendedLocale.amPmSymbolPosition() == HbExtendedLocale::Before){
            timeFormat.prepend(amPm);
        }
    }
    return timeFormat;
}


void HbStatusBarPrivate::updateTime()
{
    Q_Q(HbStatusBar);

    QTime current = QTime::currentTime();

    // set time, using a proper formatting
    QString oldTimeText = mTimeText;
    mTimeText = current.toString(timeFormat());

    if (mTimeText != oldTimeText) {
        q->updatePrimitives();
        emit q->contentChanged(HbStatusBar::TimeChanged);
    }
}

void HbStatusBarPrivate::startClockTimer()
{
    Q_Q(HbStatusBar);
    updateTime();
    if (mClockTimerId == 0) {
        int delay = (60 - QTime::currentTime().second()) * 1000;
        mClockTimerId = q->startTimer(delay);
    }
}

void HbStatusBarPrivate::killClockTimer()
{
    Q_Q(HbStatusBar);
    if (mClockTimerId != 0) {
        q->killTimer(mClockTimerId);
        mClockTimerId = 0;
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
    setFlag(QGraphicsItem::ItemHasNoContents, true);
    createPrimitives();
    qApp->installEventFilter(this);
}

/*
    Destructor.
 */
HbStatusBar::~HbStatusBar()
{ 
    Q_D(HbStatusBar);
    
    d->killClockTimer();

    qApp->removeEventFilter(this);
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
        // only do repolish if properties have changed
        HbView *view = d->mMainWindow->currentView();
        if (d->mPreviousProperties != view->viewFlags()) {
            d->mPreviousProperties = view->viewFlags();
            repolish();
            updatePrimitives();
        }
    }
}

void HbStatusBar::createPrimitives()
{
    Q_D(HbStatusBar);

    d->mTimeTextItem = HbStylePrivate::createPrimitive(HbStylePrivate::P_StatusBar_timetext, this);
    d->setBackgroundItem(HbStylePrivate::P_StatusBar_background);

    d->updateTime();
}

void HbStatusBar::updatePrimitives()
{
    Q_D(HbStatusBar);
    HbStyleOptionStatusBar option;
    
    initStyleOption(&option);
    HbStylePrivate::updatePrimitive(backgroundItem(), HbStylePrivate::P_StatusBar_background, &option);
    HbStylePrivate::updatePrimitive(d->mTimeTextItem, HbStylePrivate::P_StatusBar_timetext, &option);
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

    propertiesChanged();
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
        d->killClockTimer();
        d->startClockTimer();
    }
}

/*!
    \reimp
*/
QGraphicsItem *HbStatusBar::primitive(const QString &itemName) const
{
    const Q_D(HbStatusBar);
    if (itemName.isEmpty()) {
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
    if (e->type() == HbEvent::SleepModeEnter || e->type() == QEvent::Hide) {
        d->killClockTimer();
    } else if (e->type() == HbEvent::SleepModeExit || e->type() == QEvent::Show) {
        d->startClockTimer();
    }
    return HbWidget::event(e);
}

bool HbStatusBar::eventFilter(QObject *obj, QEvent *event)
{
    Q_D(HbStatusBar);
    if (event->type() == QEvent::ApplicationActivate) {
        d->startClockTimer();
        d->mBatteryIndicator->chargingEvent(true);
    } else if (event->type() == QEvent::ApplicationDeactivate) {
        HbDeviceDialogServerStatus::StatusFlags flags = d->mServerStatus->status();
        if ((flags & HbDeviceDialogServerStatus::ShowingDialog) == HbDeviceDialogServerStatus::NoFlags ||
            flags & HbDeviceDialogServerStatus::ShowingScreenSaver) {
            d->killClockTimer();
            d->mBatteryIndicator->chargingEvent(false);
        }
    }
    return HbWidget::eventFilter(obj, event);
}

void HbStatusBar::startClockTimer()
{
    Q_D(HbStatusBar);
    d->startClockTimer();
}

#include "moc_hbstatusbar_p.cpp"
