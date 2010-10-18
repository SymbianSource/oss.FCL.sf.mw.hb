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

#include "hbdevicefadecontrol_p.h"

#include <e32base.h>
#include <e32std.h>
#include <e32property.h>

#include <QApplication>
#include <QEvent>

// There is only one fade control setter, all other applications are getters.
// Device dialog is the setter.
const TUid PropertyCategoryUid = {0x20022FC5};
const TUint FadeKey = 0x66616465; // 'fade';

NONSHARABLE_CLASS(HbDeviceFadeControlPrivate)  : public CActive
{
public:
    struct FadeControl {
        int fadeOff: 1; // command for application to unfade
        int spare: 31;
    };

public:
    HbDeviceFadeControlPrivate(HbDeviceFadeControl *parent);
    ~HbDeviceFadeControlPrivate();

    FadeControl get();
    void set(FadeControl control);

    void start();

    static void create(HbDeviceFadeControl *parent);

public: // from CActive
    void RunL();
    void DoCancel();

public:
    HbDeviceFadeControl *p;
    RProperty mProperty;
    bool mIsSetter;
    FadeControl mSetValue;
};

HbDeviceFadeControlPrivate::HbDeviceFadeControlPrivate(HbDeviceFadeControl *parent) :
    CActive(EPriorityHigh), p(parent)
{
    RProcess process;
    mIsSetter = process.SecureId().operator TUid() == PropertyCategoryUid;

    mSetValue.fadeOff = false;
    mSetValue.spare = 0;

    // Device dialog server is a writer and creates the property.
    if (mIsSetter) {
        _LIT_SECURITY_POLICY_PASS(KRdPolicy); // all pass
        _LIT_SECURITY_POLICY_S0(KWrPolicy, PropertyCategoryUid.iUid); // pass device dialog server
        mProperty.Define(PropertyCategoryUid, FadeKey, RProperty::EInt, KRdPolicy, KWrPolicy);
    }
    mProperty.Attach(PropertyCategoryUid, FadeKey);
    if (!mIsSetter) {
        CActiveScheduler::Add(this);
        start();
    }
    process.Close();
}

HbDeviceFadeControlPrivate::~HbDeviceFadeControlPrivate()
{
    Cancel();
    mProperty.Close();
}

// Get fade control
HbDeviceFadeControlPrivate::FadeControl HbDeviceFadeControlPrivate::get()
{
    union {
        FadeControl asBits;
        TInt asInt;
    } fadeControl;
    fadeControl.asInt = 0;
    if (!mIsSetter) {
        mProperty.Get(PropertyCategoryUid, FadeKey, fadeControl.asInt);
    }
    return fadeControl.asBits;
}

// Set (write) fade control
void HbDeviceFadeControlPrivate::set(FadeControl control)
{
    if (mIsSetter && control.fadeOff != mSetValue.fadeOff) {
        mSetValue.fadeOff = control.fadeOff;
        union {
            FadeControl asBits;
            TInt asInt;
        } fadeControl;
        fadeControl.asBits = mSetValue;
        mProperty.Set(PropertyCategoryUid, FadeKey, fadeControl.asInt);
        if (!mSetValue.fadeOff) {
            // If underlying applications are to restore their fading, set this process priority
            // to background to allow faster restore.
            //RProcess().SetPriority(EPriorityBackground);
        }
    }
}

// Start property watch request
void HbDeviceFadeControlPrivate::start()
{
    mProperty.Subscribe(iStatus);
    SetActive();
}

// Create private if not yet created
void HbDeviceFadeControlPrivate::create(HbDeviceFadeControl *parent)
{
    if (!parent->d) {
        parent->d = new HbDeviceFadeControlPrivate(parent);
    }
}


// Property value has changed
void HbDeviceFadeControlPrivate::RunL()
{
    TInt completionCode = iStatus.Int();
    if (completionCode != KErrCancel) {
        start();
    }
    if (completionCode == KErrNone) {
        emit p->fadeChange(get().fadeOff);
    }
}

// Cancel property subscribe
void HbDeviceFadeControlPrivate::DoCancel()
{
    mProperty.Cancel();
}

HbDeviceFadeControl::HbDeviceFadeControl(QObject *parent) : QObject(parent)
{
    // Creation of private is delayed until any popups are created
    d = 0;
}

HbDeviceFadeControl::~HbDeviceFadeControl()
{
    delete d;
}

// Return background fading off control
bool HbDeviceFadeControl::fadeOff()
{
    HbDeviceFadeControlPrivate::create(this); // create private if not created
    return d->get().fadeOff;
}

// Set (write) background fading off control
void HbDeviceFadeControl::setFadeOff(bool fadeOff)
{
    HbDeviceFadeControlPrivate::create(this); // create private if not created
    HbDeviceFadeControlPrivate::FadeControl control = {0, 0};
    control.fadeOff = fadeOff;
    d->set(control);
}

// Return if this fade control is a setter. There is only one setter
// (device dialog server)in a device. All other instances are getters.
bool HbDeviceFadeControl::isSetter()
{
    HbDeviceFadeControlPrivate::create(this); // create private if not created
    return d->mIsSetter;
}
