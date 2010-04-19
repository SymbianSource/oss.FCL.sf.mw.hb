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

#ifndef HBSYSTEMINFO_SYM_P_P_H
#define HBSYSTEMINFO_SYM_P_P_H

#include <QObject>

#include "hbsysteminfo_p.h"
#include "hbnetworksignalmonitor_sym_p.h"
#include "hbbatterymonitor_sym_p.h"
#include "hbindicatormonitor_sym_p.h"
#include "hbnetworkmodemonitor_sym_p.h"

class HbSystemNetworkInfoPrivate : public QObject, public MNetworkSignalObserver, public MNetworkModeObserver
{
    Q_OBJECT

public:

    HbSystemNetworkInfoPrivate(QObject *parent = 0);
    virtual ~HbSystemNetworkInfoPrivate();

    HbSystemNetworkInfo::NetworkStatus networkStatus(HbSystemNetworkInfo::NetworkMode mode) const;
    int networkSignalStrength(HbSystemNetworkInfo::NetworkMode mode) const;
    int cellId() const;
    int locationAreaCode() const;

    QString currentMobileCountryCode() const; // Mobile Country Code
    QString currentMobileNetworkCode() const; // Mobile Network Code

    QString homeMobileCountryCode() const;
    QString homeMobileNetworkCode() const;

    QString networkName(HbSystemNetworkInfo::NetworkMode mode) const;
    QString macAddress(HbSystemNetworkInfo::NetworkMode mode) const;

    QNetworkInterface interfaceForMode(HbSystemNetworkInfo::NetworkMode mode) const;

    void emitNetworkStatusChanged(HbSystemNetworkInfo::NetworkMode, HbSystemNetworkInfo::NetworkStatus);
    void emitNetworkSignalStrengthChanged(HbSystemNetworkInfo::NetworkMode,int);

    static HbSystemNetworkInfoPrivate *instance();
    
    // From MNetworkSignalObserver
    void SignalStatusL(TInt32 aStrength, TInt8 aBars);
    
    // From MNetworkModeObserver
    void NetWorkModeChanged(CTelephony::TNetworkMode aMode);
    
signals:
    void networkStatusChanged(HbSystemNetworkInfo::NetworkMode, HbSystemNetworkInfo::NetworkStatus);
    void networkSignalStrengthChanged(HbSystemNetworkInfo::NetworkMode, int);
    void currentMobileCountryCodeChanged(const QString &);
    void currentMobileNetworkCodeChanged(const QString &);
    void networkNameChanged(HbSystemNetworkInfo::NetworkMode, const QString &);
    void networkModeChanged(HbSystemNetworkInfo::NetworkMode);
    
private:
    HbSystemNetworkInfo::NetworkMode networkMode(CTelephony::TNetworkMode aMode) const;

private:
    quint32 wifiStrength;
    quint32 ethStrength;
    
    CNetworkSignalMonitor *networkSignalMonitor;
    CNetworkModeMonitor *networkModeMonitor;
    mutable int iError;
};

class HbSystemDeviceInfoPrivate : public QObject, public MBatteryObserver, public MIndicatorObserver
{
    Q_OBJECT

public:

    HbSystemDeviceInfoPrivate(QObject *parent = 0);
    virtual ~HbSystemDeviceInfoPrivate();

    QString imei() const;
    QString imsi() const;
    QString manufacturer() const;
    QString model() const;
    QString productName() const;

    HbSystemDeviceInfo::InputMethodFlags inputMethodType() const;

    int batteryLevel();
    HbSystemDeviceInfo::BatteryStatus batteryStatus() const;

    HbSystemDeviceInfo::SimStatus simStatus() const;
    bool isDeviceLocked() const;
    HbSystemDeviceInfo::Profile currentProfile() const;

    HbSystemDeviceInfo::PowerState currentPowerState() const;
    void setConnection();
    
    // From MBatteryObserver
    void BatteryLevelL(TUint aChargeLevel, CTelephony::TBatteryStatus aBatteryStatus);  
    // From MIndicatorObserver
    void ChargerConnected(TUint32 state);

signals:
    void batteryLevelChanged(int);
    void batteryStatusChanged(HbSystemDeviceInfo::BatteryStatus);

    void powerStateChanged(HbSystemDeviceInfo::PowerState);
    void currentProfileChanged(HbSystemDeviceInfo::Profile);
    void bluetoothStateChanged(bool);

private:    
    CBatteryMonitor *batteryMonitor;
    CIndicatorMonitor *indicatorMonitor;
    mutable int iError;
};

#endif /*HBSYSTEMINFO_SYM_P_P_H*/


