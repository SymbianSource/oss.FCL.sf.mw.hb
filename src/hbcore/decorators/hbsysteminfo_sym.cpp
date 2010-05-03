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

#include "hbsysteminfo_sym_p_p.h"
#include <QDebug>

Q_GLOBAL_STATIC(HbSystemNetworkInfoPrivate, hbSystemNetworkInfoPrivate)

HbSystemNetworkInfoPrivate::HbSystemNetworkInfoPrivate(QObject *parent)
	: QObject(parent)
{
	networkSignalMonitor = new CNetworkSignalMonitor(*this);
	TRAP(iError, networkSignalMonitor->ConstructL();)
	networkModeMonitor = new CNetworkModeMonitor(*this);
	TRAP(iError, networkModeMonitor->ConstructL();)
}

HbSystemNetworkInfoPrivate::~HbSystemNetworkInfoPrivate()
{
	delete networkSignalMonitor;
	delete networkModeMonitor;
}

HbSystemNetworkInfoPrivate *HbSystemNetworkInfoPrivate::instance()
{ 
    return hbSystemNetworkInfoPrivate();
}

void HbSystemNetworkInfoPrivate::emitNetworkStatusChanged(HbSystemNetworkInfo::NetworkMode mode, 
	HbSystemNetworkInfo::NetworkStatus status)
{
    emit networkStatusChanged(mode, status);
}

void HbSystemNetworkInfoPrivate::emitNetworkSignalStrengthChanged(HbSystemNetworkInfo::NetworkMode mode, 
	int strength)
{
    emit networkSignalStrengthChanged(mode, strength);
}

HbSystemNetworkInfo::NetworkStatus HbSystemNetworkInfoPrivate::networkStatus(
	HbSystemNetworkInfo::NetworkMode mode) const
{
    switch(mode) {
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

int HbSystemNetworkInfoPrivate::networkSignalStrength(HbSystemNetworkInfo::NetworkMode mode) const
{
    switch(mode) {
    case HbSystemNetworkInfo::GsmMode:
    	return networkSignalMonitor->signalStrength();
    case HbSystemNetworkInfo::CdmaMode:
        return networkSignalMonitor->signalStrength();
    case HbSystemNetworkInfo::WcdmaMode:
        return networkSignalMonitor->signalStrength();
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

int HbSystemNetworkInfoPrivate::cellId() const
{
    return -1;
}

int HbSystemNetworkInfoPrivate::locationAreaCode() const
{
    return -1;
}

// Mobile Country Code
QString HbSystemNetworkInfoPrivate::currentMobileCountryCode() const
{
    return "No Network";
}

// Mobile Network Code
QString HbSystemNetworkInfoPrivate::currentMobileNetworkCode() const
{
    return "No Network";
}

QString HbSystemNetworkInfoPrivate::homeMobileCountryCode() const
{
    return "No Network";
}

QString HbSystemNetworkInfoPrivate::homeMobileNetworkCode() const
{
    return "No Network";
}

QString HbSystemNetworkInfoPrivate::networkName(HbSystemNetworkInfo::NetworkMode mode) const
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

QString HbSystemNetworkInfoPrivate::macAddress(HbSystemNetworkInfo::NetworkMode mode) const
{
    return interfaceForMode(mode).hardwareAddress();
}

QNetworkInterface HbSystemNetworkInfoPrivate::interfaceForMode(HbSystemNetworkInfo::NetworkMode mode) const
{
    QList<QNetworkInterface> interfaceList;
    interfaceList = QNetworkInterface::allInterfaces();
    qWarning() << "number of interfaces" << interfaceList.count();

    for(int i = 0; i < interfaceList.count(); i++) {
        QNetworkInterface netInterface = interfaceList.at(i);
        qWarning() << netInterface.name() << netInterface.hardwareAddress()
        << netInterface.flags() << "mode" << mode;
        if (!netInterface.isValid() || (netInterface.flags() & QNetworkInterface::IsLoopBack)) {
            continue;
        }
    } //end interfaceList

    return QNetworkInterface();
}

// From MNetworkSignalObserver
void HbSystemNetworkInfoPrivate::SignalStatusL(TInt32 aStrength, TInt8 aBars)
{
	switch (aBars)
	{
	case 0:
		aStrength = 0;
		break;
	case 1:
		aStrength = 14;
		break;
	case 2:
		aStrength = 29;
		break;
	case 3:
		aStrength = 43;
		break;
	case 4:
		aStrength = 57;
		break;
	case 5:
		aStrength = 72;
		break;
	case 6:
		aStrength = 86;
		break;
	case 7:
		aStrength = 100;
		break;
	default:
		aStrength = aBars;
		break;
	}
    emit networkSignalStrengthChanged(networkMode(networkModeMonitor->networkMode()), aStrength);
}

// From MNetworkModeObserver
void HbSystemNetworkInfoPrivate::NetWorkModeChanged(CTelephony::TNetworkMode aMode)
{
    emit networkModeChanged(networkMode(aMode));
}

HbSystemNetworkInfo::NetworkMode HbSystemNetworkInfoPrivate::networkMode(CTelephony::TNetworkMode aMode) const
{
    HbSystemNetworkInfo::NetworkMode mode(HbSystemNetworkInfo::UnknownMode);
    switch (aMode)
    {
        case CTelephony::ENetworkModeUnknown:
            mode = HbSystemNetworkInfo::UnknownMode;
            break;
        case CTelephony::ENetworkModeUnregistered:
            mode = HbSystemNetworkInfo::UnknownMode;
            break;
        case CTelephony::ENetworkModeGsm:
            mode = HbSystemNetworkInfo::GsmMode;
            break;
        case CTelephony::ENetworkModeCdma95:
        case CTelephony::ENetworkModeCdma2000:
            mode = HbSystemNetworkInfo::WcdmaMode;
            break;
        case CTelephony::ENetworkModeWcdma:
        case CTelephony::ENetworkModeTdcdma:
            mode = HbSystemNetworkInfo::WcdmaMode;
            break;
        default:
            mode = HbSystemNetworkInfo::UnknownMode;
            break;
    }
    return mode;
}

HbSystemDeviceInfoPrivate::HbSystemDeviceInfoPrivate(QObject *parent)
        : QObject(parent)
{
	batteryMonitor = new CBatteryMonitor(*this);
	TRAP(iError, batteryMonitor->ConstructL();)
	indicatorMonitor = new CIndicatorMonitor(*this);
	TRAP(iError, indicatorMonitor->ConstructL();)
}

HbSystemDeviceInfoPrivate::~HbSystemDeviceInfoPrivate()
{
	delete batteryMonitor;
	delete indicatorMonitor;
}

HbSystemDeviceInfo::Profile HbSystemDeviceInfoPrivate::currentProfile() const
{
    return HbSystemDeviceInfo::UnknownProfile;
}

HbSystemDeviceInfo::InputMethodFlags HbSystemDeviceInfoPrivate::inputMethodType() const
{
    HbSystemDeviceInfo::InputMethodFlags methods;
    return methods;
}

HbSystemDeviceInfo::PowerState HbSystemDeviceInfoPrivate::currentPowerState() const
{
    return HbSystemDeviceInfo::UnknownPower;
}

QString HbSystemDeviceInfoPrivate::imei() const
{
    return "Sim Not Available";
}

QString HbSystemDeviceInfoPrivate::imsi() const
{
    return "Sim Not Available";
}

QString HbSystemDeviceInfoPrivate::manufacturer() const
{
   return "Unknown";
}

QString HbSystemDeviceInfoPrivate::model() const
{
    return "Unknown";
}

QString HbSystemDeviceInfoPrivate::productName() const
{
    return "Unknown";
}

int HbSystemDeviceInfoPrivate::batteryLevel()
{
    return batteryMonitor->batteryLevel();
}

HbSystemDeviceInfo::SimStatus HbSystemDeviceInfoPrivate::simStatus() const
{
    return HbSystemDeviceInfo::SimNotAvailable;
}

HbSystemDeviceInfo::BatteryStatus HbSystemDeviceInfoPrivate::batteryStatus() const
{
    return HbSystemDeviceInfo::NoBatteryLevel;
}

bool HbSystemDeviceInfoPrivate::isDeviceLocked() const
{
    return false;
}

// From MNetworkSignalObserver
void HbSystemDeviceInfoPrivate::BatteryLevelL(TUint aChargeLevel, CTelephony::TBatteryStatus aBatteryStatus)
{
	Q_UNUSED(aBatteryStatus);
	emit batteryLevelChanged(aChargeLevel);
}

// From MIndicatorObserver
void HbSystemDeviceInfoPrivate::ChargerConnected(TUint32 state)
{
	if (state) {
		emit powerStateChanged(HbSystemDeviceInfo::WallPowerChargingBattery);
	} else {
		emit powerStateChanged(HbSystemDeviceInfo::BatteryPower);
	}
}

