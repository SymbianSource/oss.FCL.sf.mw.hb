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

#include "hbsysteminfowin_p_p.h"
#include "hbsysteminfo_p.h"

HbSystemInfoPrivate::HbSystemInfoPrivate()
{
}

HbSystemInfoPrivate::~HbSystemInfoPrivate()
{
    delete mSystemNetworkInfo;
    delete mSystemDeviceInfo;
}

void HbSystemInfoPrivate::init(bool writer)
{
    Q_UNUSED(writer);
    Q_Q(HbSystemInfo);
    mSystemNetworkInfo = new QSystemNetworkInfo();
    connect(mSystemNetworkInfo, SIGNAL(networkSignalStrengthChanged(QSystemNetworkInfo::NetworkMode, int)),
            q, SIGNAL(networkSignalStrengthChanged(QSystemNetworkInfo::NetworkMode, int)));
    connect(mSystemNetworkInfo, SIGNAL(networkModeChanged(QSystemNetworkInfo::NetworkMode)),
            this, SLOT(setNetworkMode(QSystemNetworkInfo::NetworkMode)));

    mSystemDeviceInfo = new QSystemDeviceInfo();
    connect(mSystemDeviceInfo, SIGNAL(batteryLevelChanged(int)),
            q, SIGNAL(batteryLevelChanged(int)));
    connect(mSystemDeviceInfo, SIGNAL(powerStateChanged(QSystemDeviceInfo::PowerState)),
            q, SIGNAL(powerStateChanged(QSystemDeviceInfo::PowerState)));
    mWatcher = HbForegroundWatcher::instance();    
    connect(mWatcher, SIGNAL(foregroundGained()), SLOT(gainedForeground()));
    connect(mWatcher, SIGNAL(foregroundLost()), SLOT(lostForeground()));
}

void HbSystemInfoPrivate::lostForeground()
{
}

void HbSystemInfoPrivate::gainedForeground()
{
}

int HbSystemInfoPrivate::networkSignalStrength() const
{
    return mSystemNetworkInfo->networkSignalStrength(mNetworkMode);
}

QSystemNetworkInfo::NetworkMode HbSystemInfoPrivate::networkMode() const
{    
    return mNetworkMode;
}

int HbSystemInfoPrivate::batteryLevel() const
{
    return mSystemDeviceInfo->batteryLevel();
}

QSystemDeviceInfo::PowerState HbSystemInfoPrivate::powerState() const
{
    return mSystemDeviceInfo->currentPowerState();
}

void HbSystemInfoPrivate::setNetworkMode(QSystemNetworkInfo::NetworkMode mode)
{
    Q_Q(HbSystemInfo);
    mNetworkMode = mode;
    emit q->networkModeChanged(mode);
}
