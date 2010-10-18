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

#include "hbdevicedialogserverstatus_p.h"

#include <e32base.h>
#include <e32std.h>
#include <e32property.h>

#include <QApplication>
#include <QEvent>

// There is only one fade control setter, all other applications are getters.
// Device dialog is the setter.
const TUid PropertyCategoryUid = {0x20022FC5};
const TUint StatusKey = 0x53746174; // 'Stat'
const TUint EventKey = 0x6e457674; // For HbIndicatorButton, to be removed when HbIndicatorButton is removed

NONSHARABLE_CLASS(HbDeviceDialogServerStatusPrivate)  : public CActive
{
public:
    HbDeviceDialogServerStatusPrivate(bool isServer, HbDeviceDialogServerStatus *parent);
    ~HbDeviceDialogServerStatusPrivate();

    HbDeviceDialogServerStatus::StatusFlags get();
    void set(HbDeviceDialogServerStatus::StatusFlags status);

    void start();

public: // from CActive
    void RunL();
    void DoCancel();

public:
    HbDeviceDialogServerStatus *p;
    RProperty mProperty;
    bool mIsSetter;
    bool mEnableMonitoring;
    HbDeviceDialogServerStatus::StatusFlags mSetValue;
};

HbDeviceDialogServerStatusPrivate::HbDeviceDialogServerStatusPrivate(
    bool isServer, HbDeviceDialogServerStatus *parent) :
    CActive(EPriorityHigh), p(parent)
{
    mIsSetter = isServer;
    mSetValue = HbDeviceDialogServerStatus::NoFlags;
    mEnableMonitoring = false;

    // Device dialog server is a writer and creates the property.
    if (mIsSetter) {
        _LIT_SECURITY_POLICY_PASS(KRdPolicy); // all pass
        _LIT_SECURITY_POLICY_S0(KWrPolicy, PropertyCategoryUid.iUid); // pass device dialog server
        mProperty.Define(PropertyCategoryUid, StatusKey, RProperty::EInt, KRdPolicy, KWrPolicy);
        
        // For HbIndicatorButton 
        RProperty::Define(PropertyCategoryUid, EventKey, RProperty::EInt, KRdPolicy, KRdPolicy);
    }
    mProperty.Attach(PropertyCategoryUid, StatusKey);
    if (!mIsSetter) {
        CActiveScheduler::Add(this);
    }
    else {
        mProperty.Set(PropertyCategoryUid, StatusKey, mSetValue);
        RProperty::Set(PropertyCategoryUid, EventKey, 0); // For HbIndicatorButton
    }
}

HbDeviceDialogServerStatusPrivate::~HbDeviceDialogServerStatusPrivate()
{
    Cancel();
    mProperty.Close();
}

// Get status
HbDeviceDialogServerStatus::StatusFlags HbDeviceDialogServerStatusPrivate::get()
{
    TInt status = HbDeviceDialogServerStatus::NoFlags;
    if (!mIsSetter) {
        mProperty.Get(PropertyCategoryUid, StatusKey, status);
    } else {
        status = mSetValue;
    }
    return static_cast<HbDeviceDialogServerStatus::StatusFlags>(status);
}

// Set (write) status
void HbDeviceDialogServerStatusPrivate::set(HbDeviceDialogServerStatus::StatusFlags status)
{
    if (mIsSetter && status != mSetValue) {
        mSetValue = status;
        mProperty.Set(PropertyCategoryUid, StatusKey, status);
    }
}

// Start property watch request
void HbDeviceDialogServerStatusPrivate::start()
{
    mProperty.Subscribe(iStatus);
    SetActive();
}

// Property value has changed
void HbDeviceDialogServerStatusPrivate::RunL()
{
    TInt completionCode = iStatus.Int();
    if (completionCode != KErrCancel) {
        start();
    }
    if (completionCode == KErrNone) {
        emit p->statusChanged();
    }
}

// Cancel property subscribe
void HbDeviceDialogServerStatusPrivate::DoCancel()
{
    mProperty.Cancel();
}

HbDeviceDialogServerStatus::HbDeviceDialogServerStatus(bool isServer, QObject *parent) :
    QObject(parent)
{
    d = new HbDeviceDialogServerStatusPrivate(isServer, this);
}

HbDeviceDialogServerStatus::~HbDeviceDialogServerStatus()
{
    delete d;
}

// Set (write) status
void HbDeviceDialogServerStatus::setStatus(StatusFlags status)
{
    d->set(status);
}

// Get (read) status
HbDeviceDialogServerStatus::StatusFlags HbDeviceDialogServerStatus::status()
{
    return d->get();
}

// Enable status monitoring and change signaling
void HbDeviceDialogServerStatus::enableMonitoring(bool enable)
{
    if (d->mEnableMonitoring != enable) {
        d->mEnableMonitoring = enable;
        if (d->mEnableMonitoring) {
            d->start();
        } else {
            d->Cancel();
        }
    }
}

// Verify that device dialog server is running
bool HbDeviceDialogServerStatus::isRunning()
{
    // Check if device dialog process exists. Assumes that if process exists,
    // the server is running properly.
    _LIT(KName, "hbdevicedialogappserver");
    RProcess process;
    bool exists = process.Open(KName) == KErrNone;
    process.Close();
    return exists;
}
