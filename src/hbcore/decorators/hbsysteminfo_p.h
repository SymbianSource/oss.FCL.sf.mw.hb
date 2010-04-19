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

#ifndef HBSYSTEMINFO_H
#define HBSYSTEMINFO_H

#include <QObject>
#include <QNetworkInterface>

#include "hbglobal.h"
#include "hbnamespace.h"

class HbSystemNetworkInfoPrivate;
class HbSystemDeviceInfoPrivate;

class HB_CORE_PRIVATE_EXPORT HbSystemNetworkInfo : public QObject
{
    Q_OBJECT
    Q_ENUMS(NetworkStatus)
    Q_ENUMS(NetworkMode)

public:

    HbSystemNetworkInfo(QObject *parent = 0);
    virtual ~HbSystemNetworkInfo();

    enum NetworkStatus {
        UndefinedStatus = 0,
        NoNetworkAvailable,
        EmergencyOnly,
        Searching,
        Busy,
        Connected,
        HomeNetwork,
        Denied,
        Roaming
    };

    enum NetworkMode {
        UnknownMode=0,
        GsmMode,
        CdmaMode,
        WcdmaMode,
        WlanMode,
        EthernetMode,
        BluetoothMode,
        WimaxMode
    };
    Q_DECLARE_FLAGS(NetworkModes, NetworkMode)

    HbSystemNetworkInfo::NetworkStatus networkStatus(HbSystemNetworkInfo::NetworkMode mode) const; 
    int networkSignalStrength(HbSystemNetworkInfo::NetworkMode mode) const; 
    QString macAddress(HbSystemNetworkInfo::NetworkMode mode) const;

    int cellId() const;
    int locationAreaCode() const;

    QString currentMobileCountryCode() const; 
    QString currentMobileNetworkCode() const; 
    QString homeMobileCountryCode() const;
    QString homeMobileNetworkCode() const;
    QString networkName(HbSystemNetworkInfo::NetworkMode mode) const;
    QNetworkInterface interfaceForMode(HbSystemNetworkInfo::NetworkMode mode) const;

signals:
   void networkStatusChanged(HbSystemNetworkInfo::NetworkMode, HbSystemNetworkInfo::NetworkStatus);
   void networkSignalStrengthChanged(HbSystemNetworkInfo::NetworkMode, int);
   void currentMobileCountryCodeChanged(const QString &);
   void currentMobileNetworkCodeChanged(const QString &);
   void networkNameChanged(HbSystemNetworkInfo::NetworkMode,const QString &);
   void networkModeChanged(HbSystemNetworkInfo::NetworkMode);

private:
    HbSystemNetworkInfoPrivate *d;
};

class HB_CORE_PRIVATE_EXPORT HbSystemDeviceInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Profile currentProfile READ currentProfile)
    Q_PROPERTY(PowerState currentPowerState READ currentPowerState)
    Q_PROPERTY(SimStatus simStatus READ simStatus)

    Q_ENUMS(BatteryLevel)
    Q_ENUMS(PowerState)
    Q_ENUMS(InputMethod)

public:

    HbSystemDeviceInfo(QObject *parent = 0);
    virtual ~HbSystemDeviceInfo();

    enum BatteryStatus {
        NoBatteryLevel = 0,
        BatteryCritical,
        BatteryVeryLow,
        BatteryLow,
        BatteryNormal
    };

    enum PowerState {
        UnknownPower = 0,
        BatteryPower,
        WallPower,
        WallPowerChargingBattery
    };

    enum InputMethod {
        Keys = 0x0000001,
        Keypad = 0x0000002,
        Keyboard = 0x0000004,
        SingleTouch = 0x0000008,
        MultiTouch = 0x0000010,
        Mouse = 0x0000020
    };
    Q_DECLARE_FLAGS(InputMethodFlags, InputMethod)

    HbSystemDeviceInfo::InputMethodFlags inputMethodType() const;

    QString imei() const;
    QString imsi() const;
    QString manufacturer() const;
    QString model() const; //external
    QString productName() const; //internal name

    int batteryLevel() const; //signal
    HbSystemDeviceInfo::BatteryStatus batteryStatus() const;

    enum Profile {
        UnknownProfile = 0,
        SilentProfile,
        NormalProfile,
        LoudProfile,
        VibProfile,
        OfflineProfile,
        PowersaveProfile,
        CustomProfile
    };

    enum SimStatus {
        SimNotAvailable = 0,
        SingleSimAvailable,
        DualSimAvailable,
        SimLocked
	};

    bool isDeviceLocked() const;
    HbSystemDeviceInfo::SimStatus simStatus() const;
    HbSystemDeviceInfo::Profile currentProfile() const; 
    HbSystemDeviceInfo::PowerState currentPowerState() const; 

signals:
    void batteryLevelChanged(int);
    void batteryStatusChanged(HbSystemDeviceInfo::BatteryStatus );
    void powerStateChanged(HbSystemDeviceInfo::PowerState);
    void currentProfileChanged(HbSystemDeviceInfo::Profile);
    void bluetoothStateChanged(bool);

private:
    HbSystemDeviceInfoPrivate *d;
};

#endif /*HBSYSTEMINFO_H*/
