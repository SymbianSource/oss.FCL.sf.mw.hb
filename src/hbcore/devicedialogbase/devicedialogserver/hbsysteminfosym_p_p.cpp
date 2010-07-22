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

#include "hbsysteminfosym_p_p.h"
#include <qapplication.h>
#include <qcoreevent.h>

const TUint32 secureId = 0x20022FC5;
const TUint32 splashGenServerSecureId = 0x2002E68B;

// publish & subscribe 
const TUid PropertyCategoryUid = {secureId};
const TUint InfoKey = 'Info';
_LIT_SECURITY_POLICY_PASS(KRdPolicy); // all pass
_LIT_SECURITY_POLICY_S0(KWrPolicy, PropertyCategoryUid.iUid); // pass device dialog server

HbSystemInfoPrivate::HbSystemInfoPrivate()
:   CActive(CActive::EPriorityStandard),
    mPtr(0,0),
    mListening(false),
    mWatcher(0)
{
}

HbSystemInfoPrivate::~HbSystemInfoPrivate()
{
    Cancel();
    if (!mWriter) {
        lostForeground();
    }
    delete mSystemNetworkInfo;
    delete mSystemDeviceInfo;
    mInfoProperty.Close(); 
    delete mDataBuffer;   
}

void HbSystemInfoPrivate::init(bool writer)
{
    // device dialog server is the writer
    if (writer) {
        RProcess me;
        if ((me.SecureId().iId != secureId )) {
            me.Close();
            return;
        }
        me.Close();
    }
    
    if (writer) {
        mSystemNetworkInfo = new QtMobility::QSystemNetworkInfo();
        mDeviceSystemInfo.networkMode = mSystemNetworkInfo->currentMode();
        mDeviceSystemInfo.networkStatus = mSystemNetworkInfo->networkStatus(mDeviceSystemInfo.networkMode);
        mDeviceSystemInfo.signalStrength = QSystemNetworkInfo::networkSignalStrength(mDeviceSystemInfo.networkMode);
        connect(mSystemNetworkInfo, SIGNAL(networkStatusChanged(QSystemNetworkInfo::NetworkMode, QSystemNetworkInfo::NetworkStatus)),
                this, SLOT(setNetworkStatus(QSystemNetworkInfo::NetworkMode, QSystemNetworkInfo::NetworkStatus)));
        connect(mSystemNetworkInfo, SIGNAL(networkSignalStrengthChanged(QSystemNetworkInfo::NetworkMode, int)), 
                this, SLOT(setNetworkSignalStrength(QSystemNetworkInfo::NetworkMode, int)));
        connect(mSystemNetworkInfo, SIGNAL(networkModeChanged(QSystemNetworkInfo::NetworkMode)), 
                this, SLOT(setNetworkMode(QSystemNetworkInfo::NetworkMode)));

        mSystemDeviceInfo = new QtMobility::QSystemDeviceInfo();
        mDeviceSystemInfo.batteryLevel = mSystemDeviceInfo->batteryLevel();
        mDeviceSystemInfo.powerState = mSystemDeviceInfo->currentPowerState();
        connect(mSystemDeviceInfo, SIGNAL(batteryLevelChanged(int)), 
                this, SLOT(setBatteryLevel(int)));
        connect(mSystemDeviceInfo, SIGNAL(powerStateChanged(QSystemDeviceInfo::PowerState)), 
                this, SLOT(setPowerState(QSystemDeviceInfo::PowerState)));
                
        int err = mInfoProperty.Define(PropertyCategoryUid, InfoKey, RProperty::EByteArray, KRdPolicy, KWrPolicy);
        if (err == KErrNone) {
            writeDeviceInfo();
        }
    } else {     
        CActiveScheduler::Add(this);
         
        qApp->installEventFilter(this);
        mWatcher = HbForegroundWatcher::instance();    
        connect(mWatcher, SIGNAL(foregroundGained()), SLOT(gainedForeground()));
        connect(mWatcher, SIGNAL(foregroundLost()), SLOT(lostForeground()));
    
        mDataBuffer = HBufC8::New(RProperty::KMaxPropertySize);
        if (mDataBuffer) {
            mPtr.Set(mDataBuffer->Des());
            mInfoProperty.Attach(PropertyCategoryUid, InfoKey);
        }

        // hbsplashgenerator will not have any Qt widget shown (and
        // thus created) so we cannot rely on foreground-background
        // notifications. Instead, invoke gainedForeground manually.
        RProcess me;
        if (me.SecureId().iId == splashGenServerSecureId) {
            QMetaObject::invokeMethod(this, "gainedForeground", Qt::QueuedConnection);
        }
        me.Close();
    }
}

void HbSystemInfoPrivate::start()
{
    if (mInfoProperty.Handle()) {
        mInfoProperty.Subscribe(iStatus);
        SetActive();
    }
}

void HbSystemInfoPrivate::readDeviceInfo()
{
    int error(KErrGeneral);
    if (mDataBuffer) {
        mPtr.Zero();
        error = mInfoProperty.Get(PropertyCategoryUid, InfoKey, mPtr);
    }

    if (error == KErrNone) {
        QByteArray resArray( (const char*) mPtr.Ptr(), mPtr.Size() );
        QDataStream stream( &resArray, QIODevice::ReadOnly);
        DeviceSystemInfo sysInfo;
        stream >> sysInfo;
        dataReceived(sysInfo);
    }
}

void HbSystemInfoPrivate::writeDeviceInfo()
{
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream << mDeviceSystemInfo;
    //write data
    TPtr8 ptr( reinterpret_cast<TUint8*>(array.data()),
                                         array.size(), array.size());
    mInfoProperty.Set(PropertyCategoryUid, InfoKey, ptr);
}

void HbSystemInfoPrivate::dataReceived(const DeviceSystemInfo& info)
{
    Q_Q(HbSystemInfo);
    bool modeChanged = info.networkMode != mDeviceSystemInfo.networkMode;
    bool statusChanged = info.networkStatus != mDeviceSystemInfo.networkStatus;
    
    if (modeChanged) {
        mDeviceSystemInfo.networkMode = info.networkMode;
    }

    if (statusChanged) {
        mDeviceSystemInfo.networkStatus = info.networkStatus;  
    }
    if (modeChanged || statusChanged) {        
        emit q->networkModeChanged((QSystemNetworkInfo::NetworkMode)mDeviceSystemInfo.networkMode, 
                                   (QSystemNetworkInfo::NetworkStatus)mDeviceSystemInfo.networkStatus);
    } 
    
    if (info.signalStrength != mDeviceSystemInfo.signalStrength) {
        mDeviceSystemInfo.signalStrength = info.signalStrength;
        emit q->networkSignalStrengthChanged((QSystemNetworkInfo::NetworkMode)mDeviceSystemInfo.networkMode, 
                                          mDeviceSystemInfo.signalStrength);
    }

    if (info.powerState != mDeviceSystemInfo.powerState) {
        mDeviceSystemInfo.powerState = info.powerState;
        emit q->powerStateChanged(((QSystemDeviceInfo::PowerState)mDeviceSystemInfo.powerState));
    }

    if (info.batteryLevel != mDeviceSystemInfo.batteryLevel) {
        mDeviceSystemInfo.batteryLevel = info.batteryLevel;
        emit q->batteryLevelChanged(mDeviceSystemInfo.batteryLevel);
    }
}

void HbSystemInfoPrivate::DoCancel()
{
    if (mListening) {
        mInfoProperty.Cancel();
        mListening = false;
    }
}

void HbSystemInfoPrivate::RunL()
{
    int result = iStatus.Int();
    if (result != KErrCancel) {
        start();
    }    
    if (result == KErrNone) {
        readDeviceInfo();
    }
}

void HbSystemInfoPrivate::setNetworkStatus(
    QSystemNetworkInfo::NetworkMode networkMode, 
    QSystemNetworkInfo::NetworkStatus networkStatus)
{
    bool changed(false);
    if (networkMode != mDeviceSystemInfo.networkMode) {
        mDeviceSystemInfo.networkMode = networkMode;
        changed = true;
    }
    
    if (networkStatus != mDeviceSystemInfo.networkStatus) {
        mDeviceSystemInfo.networkStatus = networkStatus;
        changed = true;
    }
    
    if (changed) {
        writeDeviceInfo();    
    }
    
}

void HbSystemInfoPrivate::setNetworkSignalStrength(
    QSystemNetworkInfo::NetworkMode mode, 
    int strength)
{
    bool changed(false);

    if (mode != mDeviceSystemInfo.networkMode) {
        mDeviceSystemInfo.networkMode = mode;
        changed = true;
    }
    
    if (strength != mDeviceSystemInfo.signalStrength) {
        mDeviceSystemInfo.signalStrength = strength;
        changed = true;
    }

    if (changed) {
        writeDeviceInfo();
    }
}

void HbSystemInfoPrivate::setNetworkMode(QSystemNetworkInfo::NetworkMode mode)
{
    if (mode != mDeviceSystemInfo.networkMode) {
        mDeviceSystemInfo.networkMode = mode;
        writeDeviceInfo();
    }
}

void HbSystemInfoPrivate::setBatteryLevel(int level)
{
    if (level != mDeviceSystemInfo.batteryLevel) {
        mDeviceSystemInfo.batteryLevel = level;
        writeDeviceInfo();
    }
}

void HbSystemInfoPrivate::setPowerState(QSystemDeviceInfo::PowerState state)
{
    if (state != mDeviceSystemInfo.powerState) {
        mDeviceSystemInfo.powerState = state;
        writeDeviceInfo();
    }
}

void HbSystemInfoPrivate::lostForeground()
{
    // Statusbars are rendered in the background by hbsplashgenerator
    // so cannot stop listening in that case.
    RProcess me;
    if (mListening && me.SecureId().iId != splashGenServerSecureId) {
        mInfoProperty.Cancel();
        Cancel();
        mListening = false;
    }
    me.Close();
}

void HbSystemInfoPrivate::gainedForeground()
{
    if (!mListening) {
        start();
        readDeviceInfo();
        mListening = true;
    }
}

QSystemNetworkInfo::NetworkStatus HbSystemInfoPrivate::networkStatus() const
{
    return mDeviceSystemInfo.networkStatus;
}

int HbSystemInfoPrivate::networkSignalStrength() const
{
    return mDeviceSystemInfo.signalStrength;
}

QSystemNetworkInfo::NetworkMode HbSystemInfoPrivate::networkMode() const
{
    return mDeviceSystemInfo.networkMode;
}

int HbSystemInfoPrivate::batteryLevel() const
{
    return mDeviceSystemInfo.batteryLevel;
}

QSystemDeviceInfo::PowerState HbSystemInfoPrivate::powerState() const
{
    return mDeviceSystemInfo.powerState;
}

bool HbSystemInfoPrivate::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::ApplicationActivate) {
        gainedForeground();    
    } else if (event->type() == QEvent::ApplicationDeactivate) {
        lostForeground();
    }
    return QObject::eventFilter(obj, event);
}
