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

#ifndef HBSYSTEMINFO_WIN_P_P_H
#define HBSYSTEMINFO_WIN_P_P_H

#include <QObject>
#include <QBasicTimer>

#include <winsock2.h>

#ifdef Q_CC_MSVC
#include <ntddndis.h>
#else
//#include <ddk/ntddndis.h>
#endif

#include "hbsysteminfo_p.h"

class WMIHelper;

class HbSystemNetworkInfoPrivate : public QObject
{
    Q_OBJECT

public:

    HbSystemNetworkInfoPrivate(QObject *parent = 0);
    virtual ~HbSystemNetworkInfoPrivate();

    HbSystemNetworkInfo::NetworkStatus networkStatus(HbSystemNetworkInfo::NetworkMode mode);
    qint32 networkSignalStrength(HbSystemNetworkInfo::NetworkMode mode);
    int cellId();
    int locationAreaCode();

    QString currentMobileCountryCode(); // Mobile Country Code
    QString currentMobileNetworkCode(); // Mobile Network Code

    QString homeMobileCountryCode();
    QString homeMobileNetworkCode();

    QString networkName(HbSystemNetworkInfo::NetworkMode mode);
    QString macAddress(HbSystemNetworkInfo::NetworkMode mode);

    QNetworkInterface interfaceForMode(HbSystemNetworkInfo::NetworkMode mode);

    void emitNetworkStatusChanged(HbSystemNetworkInfo::NetworkMode, HbSystemNetworkInfo::NetworkStatus);
    void emitNetworkSignalStrengthChanged(HbSystemNetworkInfo::NetworkMode,int);

    static HbSystemNetworkInfoPrivate *instance();

signals:
   void networkStatusChanged(HbSystemNetworkInfo::NetworkMode, HbSystemNetworkInfo::NetworkStatus);
   void networkSignalStrengthChanged(HbSystemNetworkInfo::NetworkMode,int);
   void currentMobileCountryCodeChanged(const QString &);
   void currentMobileNetworkCodeChanged(const QString &);
   void networkNameChanged(HbSystemNetworkInfo::NetworkMode, const QString &);
   void networkModeChanged(HbSystemNetworkInfo::NetworkMode);

protected:
   void timerEvent(QTimerEvent *event);

private slots:
   void nlaNetworkChanged();
   void networkStrengthTimeout();
   void networkStatusTimeout();

private:
    quint32 wifiStrength;
    quint32 ethStrength;

   QBasicTimer netStrengthTimer;
   QBasicTimer netStatusTimer;

#ifndef Q_CC_MINGW
    WMIHelper *wHelper;
#endif
};

class HbSystemDeviceInfoPrivate : public QObject
{
    Q_OBJECT

public:

    HbSystemDeviceInfoPrivate(QObject *parent = 0);
    virtual ~HbSystemDeviceInfoPrivate();

    QString imei();
    QString imsi();
    QString manufacturer();
    QString model();
    QString productName();

    HbSystemDeviceInfo::InputMethodFlags inputMethodType();

    int  batteryLevel();
    HbSystemDeviceInfo::BatteryStatus batteryStatus();

    HbSystemDeviceInfo::SimStatus simStatus();
    bool isDeviceLocked();
    HbSystemDeviceInfo::Profile currentProfile();

    HbSystemDeviceInfo::PowerState currentPowerState();
    void setConnection();

signals:
    void batteryLevelChanged(int);
    void batteryStatusChanged(HbSystemDeviceInfo::BatteryStatus);

    void powerStateChanged(HbSystemDeviceInfo::PowerState);
    void currentProfileChanged(HbSystemDeviceInfo::Profile);
    void bluetoothStateChanged(bool);

protected:
   void timerEvent(QTimerEvent *event);

private:
    QBasicTimer batteryLevelTimer;
    int currentLevel;
};

#endif /*HBSYSTEMINFO_WIN_P_P_H*/


