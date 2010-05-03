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

#include "hbsysteminfo_linux_p_p.h"

#include <QTimerEvent>
#include <QDebug>

Q_GLOBAL_STATIC(HbSystemNetworkInfoPrivate, hbSystemNetworkInfoPrivate)

HbSystemNetworkInfoPrivate::HbSystemNetworkInfoPrivate(QObject *parent)
        : QObject(parent)
{
    netStrengthTimer.start(10000, this);
}

HbSystemNetworkInfoPrivate::~HbSystemNetworkInfoPrivate()
{
    netStrengthTimer.stop();
}

HbSystemNetworkInfoPrivate *HbSystemNetworkInfoPrivate::instance()
{
    return hbSystemNetworkInfoPrivate();
}

void HbSystemNetworkInfoPrivate::emitNetworkStatusChanged(HbSystemNetworkInfo::NetworkMode mode, HbSystemNetworkInfo::NetworkStatus status)
{
    emit networkStatusChanged(mode, status);
}

void HbSystemNetworkInfoPrivate::emitNetworkSignalStrengthChanged(HbSystemNetworkInfo::NetworkMode mode,int strength)
{
    emit networkSignalStrengthChanged(mode, strength);
}

void HbSystemNetworkInfoPrivate::nlaNetworkChanged()
{
    qWarning() << Q_FUNC_INFO;
}

void HbSystemNetworkInfoPrivate::networkStrengthTimeout()
{
    qWarning() << __FUNCTION__;
    QList<HbSystemNetworkInfo::NetworkMode> modeList;
    modeList << HbSystemNetworkInfo::GsmMode;
    modeList << HbSystemNetworkInfo::CdmaMode;
    modeList << HbSystemNetworkInfo::WcdmaMode;
    modeList << HbSystemNetworkInfo::WlanMode;
    modeList << HbSystemNetworkInfo::EthernetMode;
    modeList << HbSystemNetworkInfo::BluetoothMode;
    modeList << HbSystemNetworkInfo::WimaxMode;

    foreach(HbSystemNetworkInfo::NetworkMode mode, modeList) {
        networkSignalStrength(mode);
    }
 }

void HbSystemNetworkInfoPrivate::networkStatusTimeout()
{
    qWarning() << __FUNCTION__;
    QList<HbSystemNetworkInfo::NetworkMode> modeList;
    modeList << HbSystemNetworkInfo::GsmMode;
    modeList << HbSystemNetworkInfo::CdmaMode;
    modeList << HbSystemNetworkInfo::WcdmaMode;
    modeList << HbSystemNetworkInfo::WlanMode;
    modeList << HbSystemNetworkInfo::EthernetMode;
    modeList << HbSystemNetworkInfo::BluetoothMode;
    modeList << HbSystemNetworkInfo::WimaxMode;

    foreach(HbSystemNetworkInfo::NetworkMode mode, modeList) {
        networkStatus(mode);
    }
 }

HbSystemNetworkInfo::NetworkStatus HbSystemNetworkInfoPrivate::networkStatus(HbSystemNetworkInfo::NetworkMode mode)
{
    switch(mode) {
    case HbSystemNetworkInfo::UnknownMode:
        break;
    case HbSystemNetworkInfo::GsmMode:
        break;
    case HbSystemNetworkInfo::CdmaMode:
        break;
    case HbSystemNetworkInfo::WcdmaMode:
        break;
    case HbSystemNetworkInfo::WlanMode:
        break;
    case HbSystemNetworkInfo::EthernetMode:
        break;
    case HbSystemNetworkInfo::BluetoothMode:
        break;
    case HbSystemNetworkInfo::WimaxMode:
        break;
    };
    return HbSystemNetworkInfo::NoNetworkAvailable;
}

int HbSystemNetworkInfoPrivate::networkSignalStrength(HbSystemNetworkInfo::NetworkMode mode)
{
    switch(mode) {
    case HbSystemNetworkInfo::UnknownMode:
        break;
    case HbSystemNetworkInfo::GsmMode:
        break;
    case HbSystemNetworkInfo::CdmaMode:
        break;
    case HbSystemNetworkInfo::WcdmaMode:
        break;
    case HbSystemNetworkInfo::WlanMode:
        break;
    case HbSystemNetworkInfo::EthernetMode:
        break;
    case HbSystemNetworkInfo::BluetoothMode:
        break;
   case HbSystemNetworkInfo::WimaxMode:
        break;
    };
    return -1;
}

void HbSystemNetworkInfoPrivate::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == netStrengthTimer.timerId()) {
      networkStrengthTimeout();
    } else if (event->timerId() == netStatusTimer.timerId()) {
      networkStatusTimeout();
    } else {
      QObject::timerEvent(event);
    }
}

int HbSystemNetworkInfoPrivate::cellId()
{
    return -1;
}

int HbSystemNetworkInfoPrivate::locationAreaCode()
{
    return -1;
}

// Mobile Country Code
QString HbSystemNetworkInfoPrivate::currentMobileCountryCode()
{
    return "No Network";
}

// Mobile Network Code
QString HbSystemNetworkInfoPrivate::currentMobileNetworkCode()
{
    return "No Network";
}

QString HbSystemNetworkInfoPrivate::homeMobileCountryCode()
{
    return "No Network";
}

QString HbSystemNetworkInfoPrivate::homeMobileNetworkCode()
{
    return "No Network";
}

QString HbSystemNetworkInfoPrivate::networkName(HbSystemNetworkInfo::NetworkMode mode)
{
    QString netname = "No network available";
    switch(mode) {
    case HbSystemNetworkInfo::WlanMode:
        break;
    case HbSystemNetworkInfo::EthernetMode:
        break;
    default:
        break;
    };
    return netname;
}

QString HbSystemNetworkInfoPrivate::macAddress(HbSystemNetworkInfo::NetworkMode mode)
{
    return interfaceForMode(mode).hardwareAddress();
}

QNetworkInterface HbSystemNetworkInfoPrivate::interfaceForMode(HbSystemNetworkInfo::NetworkMode mode)
{
    Q_UNUSED(mode);
    return QNetworkInterface();
}

HbSystemDeviceInfoPrivate::HbSystemDeviceInfoPrivate(QObject *parent)
    : QObject(parent), currentLevel(0)
{
    batteryLevelTimer.start(10000, this);
}

HbSystemDeviceInfoPrivate::~HbSystemDeviceInfoPrivate()
{
    batteryLevelTimer.stop();
}

HbSystemDeviceInfo::Profile HbSystemDeviceInfoPrivate::currentProfile()
{
    return HbSystemDeviceInfo::UnknownProfile;
}

HbSystemDeviceInfo::InputMethodFlags HbSystemDeviceInfoPrivate::inputMethodType()
{
    HbSystemDeviceInfo::InputMethodFlags methods;
    return methods;
}

HbSystemDeviceInfo::PowerState HbSystemDeviceInfoPrivate::currentPowerState()
{
    return HbSystemDeviceInfo::UnknownPower;
}

QString HbSystemDeviceInfoPrivate::imei()
{
    return "Sim Not Available";
}

QString HbSystemDeviceInfoPrivate::imsi()
{
    return "Sim Not Available";
}

QString HbSystemDeviceInfoPrivate::manufacturer()
{
    return "Unknown";
}

QString HbSystemDeviceInfoPrivate::model()
{
    return "Unknown";
}

QString HbSystemDeviceInfoPrivate::productName()
{
    return "Unknown";
}

int HbSystemDeviceInfoPrivate::batteryLevel()
{
    return 0;
}

HbSystemDeviceInfo::SimStatus HbSystemDeviceInfoPrivate::simStatus()
{
    return HbSystemDeviceInfo::SimNotAvailable;
}

HbSystemDeviceInfo::BatteryStatus HbSystemDeviceInfoPrivate::batteryStatus()
{
    return HbSystemDeviceInfo::NoBatteryLevel;
}

bool HbSystemDeviceInfoPrivate::isDeviceLocked()
{
    return false;
}

void HbSystemDeviceInfoPrivate::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == batteryLevelTimer.timerId()) {
        int level = batteryLevel();
        if (level != currentLevel) {
            currentLevel = level;
            emit batteryLevelChanged(currentLevel);
        }
    } else {
      QObject::timerEvent(event);
    }
}
