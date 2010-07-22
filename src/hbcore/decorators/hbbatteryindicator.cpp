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

#include "hbbatteryindicator_p.h"
#include "hbbatteryindicator_p_p.h"
#include "hbstyleoptionbatteryindicator_p.h"

#include <hbinstance.h>
#include <hbevent.h>

/*
    \class HbBatteryIndicator
    \brief HbBatteryIndicator represents a battery indicator item.

    The battery indicator shows approximately how much charge is left in the battery of the device.
    It is created and managed by the HbStatusBar which in turn is part of the HbMainWindow.
 */

// 0-33% for low, 34-66% for medium and 67-100% for high
static const int batteryThreshold[] = {33, 66, 100};

HbBatteryIndicatorPrivate::HbBatteryIndicatorPrivate() :
    mLevelPercent(-1),
    mBatteryBackgroundIcon(0),
    mBatteryLevelIcon(0),
    mBatteryIcon(0),
#ifdef HB_HAVE_QT_MOBILITY
    mSystemDeviceInfo(new HbSystemInfo(0, false)),
#endif // HB_HAVE_QT_MOBILITY
    mChargingOn(false)
{
}

HbBatteryIndicatorPrivate::~HbBatteryIndicatorPrivate()
{
#ifdef HB_HAVE_QT_MOBILITY
    delete mSystemDeviceInfo;
#endif // HB_HAVE_QT_MOBILITY
}

/*
    Handles signal for changing the power state.
*/
#ifdef HB_HAVE_QT_MOBILITY
void HbBatteryIndicatorPrivate::_q_setPowerState(QSystemDeviceInfo::PowerState powerState)
{
    Q_Q(HbBatteryIndicator);
    if (powerState == QSystemDeviceInfo::WallPowerChargingBattery) {        
        q->setLevel(0);
        mChargingOn = true;
        if (mChargingTimer.isActive()) {
            mChargingTimer.stop();
        }
        mChargingTimer.start(250, q);
        emit q->levelChanged(); // to make sure the charging status is communicated
    } else {
        mChargingOn = false;
        mChargingTimer.stop();
        q->setLevel(mSystemDeviceInfo->batteryLevel());
        emit q->levelChanged(); // to make sure the non-charging status is communicated
    }
}
#endif // HB_HAVE_QT_MOBILITY

// ======== MEMBER FUNCTIONS ========

/*
    Constructs a battery indicator with \a parent.
    Also sets the default icon.
*/
HbBatteryIndicator::HbBatteryIndicator(QGraphicsItem *parent)
    : HbWidget(*new HbBatteryIndicatorPrivate, parent)
{
    createPrimitives();
    updatePrimitives();

#ifdef HB_HAVE_QT_MOBILITY
	Q_D(HbBatteryIndicator);
    connect(d->mSystemDeviceInfo, SIGNAL(batteryLevelChanged(int)), this, SLOT(setLevel(int)));
    connect(d->mSystemDeviceInfo, SIGNAL(powerStateChanged(QSystemDeviceInfo::PowerState)), this, 
        SLOT(_q_setPowerState(QSystemDeviceInfo::PowerState)));
#endif // HB_HAVE_QT_MOBILITY
}

/*
    Destructor
 */
HbBatteryIndicator::~HbBatteryIndicator()
{

}

/*
    Delayed constructor.
 */
void HbBatteryIndicator::delayedConstruction()
{

}

void HbBatteryIndicator::createPrimitives()
{
    Q_D(HbBatteryIndicator);
    d->mBatteryIcon = style()->createPrimitive(HbStyle::P_BatteryIndicator_icon, this);
    d->mBatteryBackgroundIcon = style()->createPrimitive(HbStyle::P_BatteryLevel_background, this);
    d->mBatteryLevelIcon = style()->createPrimitive(HbStyle::P_BatteryLevel_icon, this);
}

void HbBatteryIndicator::updatePrimitives()
{
    Q_D(HbBatteryIndicator);
    HbStyleOptionBatteryIndicator option;
    initStyleOption(&option);
    style()->updatePrimitive(d->mBatteryIcon, HbStyle::P_BatteryIndicator_icon, &option);
    style()->updatePrimitive(d->mBatteryBackgroundIcon, HbStyle::P_BatteryLevel_background, &option);
    style()->updatePrimitive(d->mBatteryLevelIcon, HbStyle::P_BatteryLevel_icon, &option);
}

int HbBatteryIndicator::level() const
{ 
    Q_D(const HbBatteryIndicator);
    return d->mLevelPercent;
}

/*  
    set battery strength level. \a level value should be between 0 and 100
    \a level value is ignored if out of range.
*/
void HbBatteryIndicator::setLevel(int levelPercent)
{
    Q_D(HbBatteryIndicator);
    if (levelPercent < 0 || levelPercent > 100 || d->mChargingOn) {
        return;
    }

    if (d->mLevelPercent != levelPercent) {
        d->mLevelPercent = levelPercent;
        updatePrimitives();
        emit levelChanged(); // must be emitted whenever the visualization changes (i.e. also while charging)
    }
}

/*
    \reimp
*/
void HbBatteryIndicator::timerEvent(QTimerEvent *event)
{
    Q_D(HbBatteryIndicator);
    if (event->timerId() == d->mChargingTimer.timerId()) {
        int step(4);
        int currentLevel(0);
#ifdef HB_HAVE_QT_MOBILITY        
        currentLevel = d->mSystemDeviceInfo->batteryLevel();
#endif // HB_HAVE_QT_MOBILITY        
        if (currentLevel < batteryThreshold[0]) {
            currentLevel = batteryThreshold[0] - 1;
            step = 2;
        } else if (currentLevel < batteryThreshold[1]) {
            step = 3;
        }
        d->mChargingOn = false;
        if (d->mLevelPercent < currentLevel) {
            setLevel(d->mLevelPercent + step);
        } else {
            setLevel(0);
        }
        d->mChargingOn = true;
    }
}

void HbBatteryIndicator::initStyleOption(HbStyleOptionBatteryIndicator *option) const
{
    Q_D(const HbBatteryIndicator);
    HbWidget::initStyleOption(option);

    //battery level setting
    if (d->mLevelPercent >= 0 && d->mLevelPercent <= batteryThreshold[0]) { // low
        option->batteryLevel = HbStyleOptionBatteryIndicator::Low;
    } else if (d->mLevelPercent >= batteryThreshold[0] &&
               d->mLevelPercent <= batteryThreshold[1]) { // medium
        option->batteryLevel = HbStyleOptionBatteryIndicator::Medium;
    } else if (d->mLevelPercent >= batteryThreshold[1] &&
               d->mLevelPercent <= batteryThreshold[2]) { // high
        option->batteryLevel = HbStyleOptionBatteryIndicator::Full;
    } else {
        option->batteryLevel = HbStyleOptionBatteryIndicator::Zero;
    }
    option->batteryValue = d->mLevelPercent;
}

bool HbBatteryIndicator::isCharging() const
{
    Q_D(const HbBatteryIndicator);
    // Cannot use mChargingOn only because this function must work
    // reliably even when called while being in setLevel(). On the
    // other hand the timer is stopped while being in sleep mode. So
    // check both.
    return d->mChargingTimer.isActive() || d->mChargingOn;
}

/*!
    \reimp
*/
bool HbBatteryIndicator::event(QEvent *e)
{
    Q_D(HbBatteryIndicator);
    if (e->type() == HbEvent::SleepModeEnter) {
        d->mChargingTimer.stop();
    } else if (e->type() == HbEvent::SleepModeExit) {
        if (d->mChargingOn) {
            d->mChargingTimer.start(250, this);
        }
    }
    return HbWidget::event(e);
}
    
#include "moc_hbbatteryindicator_p.cpp"
