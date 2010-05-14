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

#ifndef HBSYSTEMINFOSYM_P_P_H
#define HBSYSTEMINFOSYM_P_P_H

#include <e32base.h>
#include <e32property.h>
#include "hbsysteminfo_p.h"

struct DeviceSystemInfo 
{
    inline DeviceSystemInfo() :
        networkMode(QSystemNetworkInfo::UnknownMode),
        signalStrength(0),
        batteryLevel(0),
        powerState(QSystemDeviceInfo::UnknownPower) {}
    
    QSystemNetworkInfo::NetworkMode networkMode;
    int signalStrength;
    int batteryLevel;
    QSystemDeviceInfo::PowerState powerState;    
};

inline QDataStream& operator << (QDataStream &outStream,
                                 const DeviceSystemInfo &obj)
{
    outStream << obj.networkMode;
    outStream << obj.signalStrength;
    outStream << obj.powerState;
    outStream << obj.batteryLevel;
    return outStream;
}

inline QDataStream& operator >> (QDataStream &inStream,
        DeviceSystemInfo &obj)
{
    int temp;
    inStream >> temp;
    obj.networkMode = (QSystemNetworkInfo::NetworkMode)temp;
    inStream >> obj.signalStrength;
    inStream >> temp;
    obj.powerState= (QSystemDeviceInfo::PowerState)temp;
    inStream >> obj.batteryLevel;
    return inStream;
}

class HbSystemInfoPrivate : public QObject,
                            public CActive
{
    Q_OBJECT
    
public:
    HbSystemInfoPrivate();
    virtual ~HbSystemInfoPrivate();
    void init(bool writer);
    
public:
    void start();
    void readDeviceInfo();
    void writeDeviceInfo();
    void dataReceived(const DeviceSystemInfo& info);
    bool eventFilter(QObject *obj, QEvent *event);
public:
    int networkSignalStrength() const;
    QSystemNetworkInfo::NetworkMode networkMode() const;
    int batteryLevel() const;
    QSystemDeviceInfo::PowerState powerState() const;
    
protected:
    virtual void DoCancel();
    virtual void RunL();    
    
public slots:
    void setNetworkSignalStrength(QSystemNetworkInfo::NetworkMode, int);
    void setNetworkMode(QSystemNetworkInfo::NetworkMode);
    void setBatteryLevel(int);
    void setPowerState(QSystemDeviceInfo::PowerState);

    void lostForeground();
    void gainedForeground();

public:
    DeviceSystemInfo mDeviceSystemInfo;
    Q_DECLARE_PUBLIC(HbSystemInfo)
private:
    QSystemNetworkInfo *mSystemNetworkInfo;
    QSystemDeviceInfo *mSystemDeviceInfo;
    
    RProperty mInfoProperty;
    HBufC8 *mDataBuffer;
    TPtr8 mPtr; 
    HbSystemInfo *q_ptr; 
    
    bool mWriter;
    bool mListening;
    HbForegroundWatcher *mWatcher;
};
#endif // HBSYSTEMINFOSYM_P_P_H
