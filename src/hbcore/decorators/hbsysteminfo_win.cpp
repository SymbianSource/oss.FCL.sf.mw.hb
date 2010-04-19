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

#include "hbsysteminfo_win_p_p.h"

#include <QTimerEvent>

#ifndef Q_CC_MINGW
#include "hbwmihelper_win_p.h"
#endif

#ifdef Q_OS_WINCE
#include <vibrate.h>
#include <Led_drvr.h>
#include <simmgr.h>
#include <Ifapi.h>
#include <Winbase.h>
#endif

Q_GLOBAL_STATIC(HbSystemNetworkInfoPrivate, hbSystemNetworkInfoPrivate)

HbSystemNetworkInfoPrivate::HbSystemNetworkInfoPrivate(QObject *parent)
    : QObject(parent)
{
#ifndef Q_CC_MINGW
    wHelper = new WMIHelper(this);
    wHelper->setWmiNamespace("root/cimv2");
    wHelper->setClassName("Win32_NetworkAdapter");
    wHelper->setClassProperty(QStringList() << "NetConnectionStatus");
#endif // Q_CC_MINGW
   
    netStrengthTimer.start(1000, this);
}

HbSystemNetworkInfoPrivate::~HbSystemNetworkInfoPrivate()
{
    netStrengthTimer.stop();
#ifndef Q_CC_MINGW
    delete wHelper;
#endif // Q_CC_MINGW
}

HbSystemNetworkInfoPrivate *HbSystemNetworkInfoPrivate::instance()
{
    return hbSystemNetworkInfoPrivate();
}

void HbSystemNetworkInfoPrivate::emitNetworkStatusChanged(HbSystemNetworkInfo::NetworkMode mode, HbSystemNetworkInfo::NetworkStatus status)
{
  // networkStatus(mode);
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
//    qWarning() << __FUNCTION__;
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
    case HbSystemNetworkInfo::GsmMode:
        break;
    case HbSystemNetworkInfo::CdmaMode:
        break;
    case HbSystemNetworkInfo::WcdmaMode:
        break;
    case HbSystemNetworkInfo::WlanMode:
        break;
    case HbSystemNetworkInfo::EthernetMode:
        {
#ifndef Q_CC_MINGW
            QVariant v = wHelper->getWMIData();
            if(v.toUInt() == 2) {
                return HbSystemNetworkInfo::Connected;
            }
#endif
        }
        break;
        case HbSystemNetworkInfo::BluetoothMode:
        {
#ifndef Q_CC_MINGW
            QString cond;
            cond = QString("WHERE MACAddress = '%1'").arg( interfaceForMode(mode).hardwareAddress());
            wHelper->setConditional(cond.toLatin1());
            QVariant v = wHelper->getWMIData();
            if(v.toUInt() == 2) {
                return HbSystemNetworkInfo::Connected;
            }
#endif
        }
        break;
    case HbSystemNetworkInfo::WimaxMode:
        break;
    default:
        break;
    };
    return HbSystemNetworkInfo::NoNetworkAvailable;
}

int HbSystemNetworkInfoPrivate::networkSignalStrength(HbSystemNetworkInfo::NetworkMode mode)
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
        {
//           qWarning() << "checking ethernet signal";
#ifndef Q_CC_MINGW
            QString cond;
            cond = QString("WHERE MACAddress = '%1'").arg(interfaceForMode(mode).hardwareAddress());
            wHelper->setConditional(cond.toLatin1());
            QVariant v = wHelper->getWMIData();
            quint32 strength = v.toUInt();
            quint32 tmpStrength;
            //qWarning() << strength << ethStrength;

            if( strength == 2
               || strength == 9) {
                tmpStrength = 100;
            } else {
               tmpStrength = 0;
            }

            if(tmpStrength != ethStrength) {
                ethStrength = tmpStrength;
                emit networkSignalStrengthChanged(mode, ethStrength);
            }
            return ethStrength;
#endif
        }
        break;
    case HbSystemNetworkInfo::BluetoothMode:
        break;
    case HbSystemNetworkInfo::WimaxMode:
        break;
    default:
        break;
    };
    return -1;
}

void HbSystemNetworkInfoPrivate::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == netStrengthTimer.timerId()) {
        networkStrengthTimeout();
        netStrengthTimer.start(100000, this);
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
    QList<QNetworkInterface> interfaceList;
    interfaceList = QNetworkInterface::allInterfaces();
//    qWarning() << "number of interfaces" << interfaceList.count();

    for(int i = 0; i < interfaceList.count(); i++) {
        QNetworkInterface netInterface = interfaceList.at(i);
        //qWarning() << netInterface.name() << netInterface.hardwareAddress() << netInterface.flags() << "mode" << mode;
        if (!netInterface.isValid() || (netInterface.flags() & QNetworkInterface::IsLoopBack)) {
            continue;
        }

#ifndef Q_CC_MINGW
        unsigned long oid;
        DWORD bytesWritten;

        NDIS_MEDIUM medium ;
        NDIS_PHYSICAL_MEDIUM physicalMedium = NdisPhysicalMediumUnspecified;

        HANDLE handle = CreateFile((TCHAR *)QString("\\\\.\\%1").arg(netInterface.name()).utf16(), 0,
                                   FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
        if (handle == INVALID_HANDLE_VALUE) {
            qWarning() << "Invalid handle";
            continue/*return QNetworkInterface()*/;
        }

        oid = OID_GEN_MEDIA_SUPPORTED;
        bytesWritten;
        bool result = DeviceIoControl(handle, IOCTL_NDIS_QUERY_GLOBAL_STATS, &oid, sizeof(oid),
                                      &medium, sizeof(medium), &bytesWritten, 0);
        if (!result) {
            CloseHandle(handle);
            qWarning() << "DeviceIo result is false";
//            return QNetworkInterface();
            continue;
        }

        oid = OID_GEN_PHYSICAL_MEDIUM;
        bytesWritten = 0;
        result = DeviceIoControl(handle, IOCTL_NDIS_QUERY_GLOBAL_STATS, &oid, sizeof(oid),
                                 &physicalMedium, sizeof(physicalMedium), &bytesWritten, 0);
        if (!result) {
            CloseHandle(handle);
            if (medium == NdisMedium802_3 && mode == HbSystemNetworkInfo::EthernetMode) {
                return netInterface;
            } else {
               continue;
            }
        }

        CloseHandle(handle);

        if (medium == NdisMedium802_3) {
            switch (physicalMedium) {
            case NdisPhysicalMediumWirelessLan:
                {
                    if(mode == HbSystemNetworkInfo::WlanMode) {
                        return netInterface;
                    }
                }
                break;
            case NdisPhysicalMediumBluetooth:
                {
                    if(mode == HbSystemNetworkInfo::BluetoothMode) {
                        return netInterface;
                    }
                }
                break;
            case NdisPhysicalMediumWiMax:
                {
                    if(mode == HbSystemNetworkInfo::WimaxMode) {
                        return netInterface;
                    }
                }
                break;
            };
        }
#endif
    } //end interfaceList

    return QNetworkInterface();
}

HbSystemDeviceInfoPrivate::HbSystemDeviceInfoPrivate(QObject *parent)
    : QObject(parent), currentLevel(0)
{
    batteryLevelTimer.start(1000, this);
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

    int mouseResult = GetSystemMetrics(SM_CMOUSEBUTTONS);
    if(mouseResult > 0) {
        if((methods & HbSystemDeviceInfo::Mouse) != HbSystemDeviceInfo::Mouse) {
            methods |= HbSystemDeviceInfo::Mouse;
        }
    }
    int tabletResult = GetSystemMetrics(SM_TABLETPC);
    if(tabletResult > 0) {
        if((methods & HbSystemDeviceInfo::SingleTouch) != HbSystemDeviceInfo::SingleTouch) {
            methods |= HbSystemDeviceInfo::SingleTouch;
        }
    }
    int keyboardType = GetKeyboardType(0);
    switch(keyboardType) {
    case 1:
    case 3:
        {
            if((methods & HbSystemDeviceInfo::Keyboard) != HbSystemDeviceInfo::Keyboard) {
                methods |= HbSystemDeviceInfo::Keyboard;
            }
        }
        break;
    case 2:
    case 4:
        {
            if((methods & HbSystemDeviceInfo::Keyboard) != HbSystemDeviceInfo::Keyboard) {
                methods |= HbSystemDeviceInfo::Keyboard;

            }
            if((methods & HbSystemDeviceInfo::Keypad) != HbSystemDeviceInfo::Keypad) {
                methods |= HbSystemDeviceInfo::Keypad;
            }
        }
        break;
    case 5:
        {
            if((methods & HbSystemDeviceInfo::Keypad) != HbSystemDeviceInfo::Keypad) {
                methods |= HbSystemDeviceInfo::Keypad;
            }
        }
        break;
    default:
        break;
    };

    return methods;
}

HbSystemDeviceInfo::PowerState HbSystemDeviceInfoPrivate::currentPowerState()
{
#ifdef Q_OS_WINCE
    SYSTEM_POWER_STATUS_EX status;
    GetSystemPowerStatusEx(&status, true);

#else
    SYSTEM_POWER_STATUS status;
    GetSystemPowerStatus(&status);
#endif

    if(status.BatteryFlag & BATTERY_FLAG_CHARGING)
        return HbSystemDeviceInfo::WallPowerChargingBattery;
    if(status.ACLineStatus  == AC_LINE_ONLINE)
        return HbSystemDeviceInfo::WallPower;
    if(status.ACLineStatus  == AC_LINE_OFFLINE)
        return HbSystemDeviceInfo::BatteryPower;

    return HbSystemDeviceInfo::UnknownPower;
}

QString HbSystemDeviceInfoPrivate::imei()
{
//    if(this->getSimStatus() == HbSystemDeviceInfo::SimNotAvailable)
        return "Sim Not Available";
}

QString HbSystemDeviceInfoPrivate::imsi()
{
//    if(getSimStatus() == HbSystemDeviceInfo::SimNotAvailable)
        return "Sim Not Available";
}

QString HbSystemDeviceInfoPrivate::manufacturer()
{
   QString manu;
#ifndef Q_CC_MINGW
    WMIHelper *wHelper;
    wHelper = new WMIHelper(this);
    wHelper->setWmiNamespace("root/cimv2");
    wHelper->setClassName("Win32_ComputerSystemProduct");
    wHelper->setClassProperty(QStringList() << "Vendor");
    QVariant v = wHelper->getWMIData();
    manu = v.toString();
    if(manu.isEmpty()) {
        manu = "System manufacturer";
    }
    delete wHelper;
#endif
    return manu;
}

QString HbSystemDeviceInfoPrivate::model()
{
    QString model;
#ifndef Q_CC_MINGW
    WMIHelper *wHelper;
    wHelper = new WMIHelper(this);
    wHelper->setWmiNamespace("root/cimv2");

    wHelper->setClassName("Win32_ComputerSystem");
    wHelper->setClassProperty(QStringList() << "Model");
    QVariant v = wHelper->getWMIData();
    model = v.toString();

    wHelper->setClassName("Win32_ComputerSystem");
    wHelper->setClassProperty(QStringList() << "PCSystemType");
    v = wHelper->getWMIData();
    switch(v.toUInt()) {
    case 0:
        model += "";
        break;
    case 1:
        model += "Desktop";
        break;
    case 2:
        model += "Mobile";
        break;
    case 3:
        model += "Workstation";
        break;
    case 4:
        model += "Enterprise Server";
        break;
    case 5:
        model += "Small/Home Server";
        break;
    case 6:
        model += "Applicance PC";
        break;
    case 7:
        model += "Performace Server";
        break;
    case 8:
        model += "Maximum";
        break;

    };
    delete wHelper;
#endif
    return model;
}

QString HbSystemDeviceInfoPrivate::productName()
{
   QString name;
#ifndef Q_CC_MINGW
    WMIHelper *wHelper;
    wHelper = new WMIHelper(this);
    wHelper->setWmiNamespace("root/cimv2");
    wHelper->setClassName("Win32_ComputerSystemProduct");
    wHelper->setClassProperty(QStringList() << "Name");
    QVariant v = wHelper->getWMIData();
    name = v.toString();

    if(name.isEmpty()) {
        wHelper->setClassName("Win32_ComputerSystem");
        wHelper->setClassProperty(QStringList() << "PCSystemType");
        v = wHelper->getWMIData();
        name = v.toString();
        if(name.isEmpty()) {
            name = "Unspecified product";
        }
    }

    delete wHelper;
#endif
    return name;
}

int HbSystemDeviceInfoPrivate::batteryLevel()
{
#ifdef Q_OS_WINCE
    SYSTEM_POWER_STATUS_EX status;
    if (GetSystemPowerStatusEx(&statusEx, true) ) {
        qWarning() <<"battery level" <</* status.ACLineStatus << status.BatteryFlag <<*/ status.BatteryLifePercent;
        return status.BatteryLifePercent;
} else {
       qWarning() << "Battery status failed";
    }
#else
    int level(0);
    SYSTEM_POWER_STATUS status;
    if (GetSystemPowerStatus(&status) ) {
        if (status.BatteryFlag == 128) {
            level = 100;
        } else {
            level =  status.BatteryLifePercent;
        }
    }
#endif
    return level;
}

HbSystemDeviceInfo::SimStatus HbSystemDeviceInfoPrivate::simStatus()
{
#ifdef Q_OS_WINCE
    HSIM handle;
    DWORD lockedState;
    HRESULT result = SimInitialize(0,NULL,NULL,&handle);
    if(result == S_OK) {
        SimGetPhoneLockedState(handle,&lockedState);
        if(lockedState == SIM_LOCKEDSTATE_READY) {
            return HbSystemDeviceInfo::SingleAvailable;
        } else {
            return HbSystemDeviceInfo::SimLocked;
        }


    } else if(result == SIM_E_NOSIM) {
        return HbSystemDeviceInfo::SimNotAvailable;
    }
    SimDeinitialize(handle);

#endif
    return HbSystemDeviceInfo::SimNotAvailable;
}

HbSystemDeviceInfo::BatteryStatus HbSystemDeviceInfoPrivate::batteryStatus()
{
    return HbSystemDeviceInfo::NoBatteryLevel;
}

bool HbSystemDeviceInfoPrivate::isDeviceLocked()
{
#ifdef Q_OS_WINCE
    HSIM handle;
    DWORD lockedState;
    HRESULT result = SimInitialize(0,NULL,NULL,&handle);
    if(result == S_OK) {
        SimGetPhoneLockedState(handle,&lockedState);
        if(lockedState != SIM_LOCKEDSTATE_READY) {
            return true;
        }
    }
#endif
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
        batteryLevelTimer.start(100000, this);
    } else {
      QObject::timerEvent(event);
    }
}
