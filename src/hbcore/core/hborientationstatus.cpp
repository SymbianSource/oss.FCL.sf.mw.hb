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
#include "hborientationstatus_p.h"

#ifdef Q_OS_SYMBIAN
#include <e32std.h>
#include "hbcorepskeys_r.h"
#include "hbforegroundwatcher_p.h"
#include "hbglobal_p.h"
#include "hbdeviceprofile_p.h"

#ifdef HB_WSERV_HAS_RENDER_ORIENTATION
#define WS_RENDER_ORI_LISTENER
#include <wspublishandsubscribedata.h>
#include <e32debug.h>
#endif

// UID for process checking (write orientation value only when in theme server)
const TUid KWriterServerUid = KHbPsHardwareCoarseOrientationCategoryUid;

// PS related constants
_LIT_SECURITY_POLICY_PASS(KRdPolicy); // all pass
_LIT_SECURITY_POLICY_S0(KWrPolicy, KWriterServerUid.iUid); // pass writer server

#else
#include <QSettings>
#endif // Q_OS_SYMBIAN

static HbOrientationStatus *orientationStatus = 0;

void HbOrientationStatus::init(QObject *parent, Qt::Orientation defaultOrientation)
{
#ifdef Q_OS_SYMBIAN
    if (!orientationStatus) {
        // Create orientation publisher only if current process is the writer
        // server, that is, the theme server.
        RProcess process;
        if (process.SecureId().iId == KWriterServerUid.iUid) {
            orientationStatus = new HbOrientationStatus(parent, defaultOrientation);
        }
        process.Close();
    }
#else
    Q_UNUSED(parent);
    Q_UNUSED(defaultOrientation);
    orientationStatus = 0;
#endif
}

HbOrientationStatus::~HbOrientationStatus()
{
#ifdef Q_OS_SYMBIAN
    delete mSensorListener;
    mQtProperty.Close();
    mWsProperty.Close();
#endif
}

bool HbOrientationStatus::currentOrientation(Qt::Orientation &orientation)
{
    bool success = false;
#ifdef Q_OS_SYMBIAN
    if (!orientationStatus) {
        int currentOrientation = Qt::Vertical;
        if (RProperty::Get(KHbPsHardwareCoarseOrientationCategoryUid,
                           KHbPsHardwareCoarseQtOrientationKey, currentOrientation) == KErrNone) {
            orientation = (Qt::Orientation)currentOrientation;
            success = true;
        }
    }
#else
    //Read startup orientation from QSettings
    QSettings settings("Nokia", "Hb");
    settings.beginGroup("Sensors");

    if (settings.value("Orientation").toInt() == 1) {
        orientation = Qt::Horizontal;
        success = true;
    } else if (settings.value("Orientation").toInt() == 2) {
        orientation = Qt::Vertical;
        success = true;
    }
    settings.endGroup();
#endif
    return success;
}

HbOrientationStatus::HbOrientationStatus(QObject *parent, Qt::Orientation defaultOrientation)
    : QObject(parent)
#ifdef Q_OS_SYMBIAN
    , mSensorListener(new HbSensorListener(*this, defaultOrientation, false))
    , mDefaultOrientation(defaultOrientation)
#endif
{
#ifdef Q_OS_SYMBIAN
    // Set up the communication link between the foreground/sleep status handler
    // and the sensor listener.
    HbForegroundWatcher::instance()->setSensorListener(mSensorListener);

    // Create orientation property for Qt/Hb.
    RProperty::Define(KHbPsHardwareCoarseOrientationCategoryUid, KHbPsHardwareCoarseQtOrientationKey,
                      RProperty::EInt, KRdPolicy, KWrPolicy);
    mQtProperty.Attach(KHbPsHardwareCoarseOrientationCategoryUid, KHbPsHardwareCoarseQtOrientationKey);

    // Create orientation property for Window Server.
    RProperty::Define(KHbPsHardwareCoarseOrientationCategoryUid, KHbPsHardwareCoarseWsOrientationKey,
                      RProperty::EInt, KRdPolicy, KWrPolicy);
    mWsProperty.Attach(KHbPsHardwareCoarseOrientationCategoryUid, KHbPsHardwareCoarseWsOrientationKey);

    // Publish the default orientation, but storeOrientation() cannot be called directly
    // because it would lead to infinite loop in case of the themeserver process. If there
    // is another storeOrientation() call meanwhile, overwriting the value in
    // mOrientationToBeStored is perfectly fine because then there is no sense in
    // publishing the default orientation anymore.
    mOrientationToBeStored = defaultOrientation;
    QMetaObject::invokeMethod(this, "storeOrientation", Qt::QueuedConnection);

#else
    Q_UNUSED(parent);
    Q_UNUSED(defaultOrientation);
#endif
}

void HbOrientationStatus::sensorOrientationChanged(Qt::Orientation newOrientation)
{
    mOrientationToBeStored = newOrientation;
    storeOrientation();
}

void HbOrientationStatus::sensorStatusChanged(bool status, bool resetOrientation)
{
    Q_UNUSED(resetOrientation)
#ifdef Q_OS_SYMBIAN
    // Lights go off => sensor usage by Hb stops => must reset to the default
    // orientation because the state when lights come back can be anything so we
    // need to have a clean start then.
    if (status == false) {
        mOrientationToBeStored = mDefaultOrientation;
        storeOrientation();
    }
#else
    Q_UNUSED(status)
#endif
}

void HbOrientationStatus::storeOrientation()
{
#ifdef Q_OS_SYMBIAN

    mQtProperty.Set(mOrientationToBeStored);

#ifdef HB_WSERV_HAS_RENDER_ORIENTATION
    int orientationForWserv = mapOriToRenderOri(mOrientationToBeStored);
    mWsProperty.Set(orientationForWserv);
#endif

#endif // Q_OS_SYMBIAN
}

/*!
  Maps a Qt::Orientation value to a TRenderOrientation, i.e. one of
  EDisplayOrientationNormal, EDisplayOrientation90CW, EDisplayOrientation180,
  EDisplayOrientation270CW.

  \internal
*/
int HbOrientationStatus::mapOriToRenderOri(Qt::Orientation orientation)
{
#if defined(Q_OS_SYMBIAN) && defined(HB_WSERV_HAS_RENDER_ORIENTATION)
    HbDeviceProfileList *deviceProfiles = HbDeviceProfilePrivate::deviceProfiles();
    if (deviceProfiles) {
        // Usually the default orientation is portrait.
        Qt::Orientation defaultOrientation = Qt::Vertical;
        // However the default orientation for the device is landscape if there
        // is a profile where the angle is 0 and width is greater then height.
        // (note: this requires that the displaydefinition file contains
        // orientationAngle="0" whenever defaultMode is true, even though
        // malformed displaydefinitions may usually get away with an arbitrary
        // angle for the default mode...)
        foreach (const DeviceProfile &profile, *deviceProfiles) {
            if (profile.mOrientationAngle == 0
                && profile.mLogicalSize.width() > profile.mLogicalSize.height()) {
                defaultOrientation = Qt::Horizontal;
            }
        }
        // If we got the default orientation then stop right away.
        if (orientation == defaultOrientation) {
            return EDisplayOrientationNormal;
        }
        // Otherwise pick a matching profile. Multiple screen sizes on the same
        // device cannot be supported here because the only thing we have is a
        // Qt::Orientation value.
        qreal matchingAngle = 0;
        foreach (const DeviceProfile &profile, *deviceProfiles) {
            if (orientation == Qt::Vertical
                && profile.mLogicalSize.width() < profile.mLogicalSize.height()) {
                matchingAngle = profile.mOrientationAngle;
                break;
            } else if (orientation == Qt::Horizontal
                       && profile.mLogicalSize.width() > profile.mLogicalSize.height()) {
                matchingAngle = profile.mOrientationAngle;
                break;
            }
        }
        // If there was a match pick the result based on the rotation angle.
        if (matchingAngle != 0) {
            if (matchingAngle == 90 || matchingAngle == -270) {
                return EDisplayOrientation90CW;
            } else if (matchingAngle == 180) {
                return EDisplayOrientation180;
            } else if (matchingAngle == 270 || matchingAngle == -90) {
                return EDisplayOrientation270CW;
            }
        }
    }
    // If all else fails, just return "default".
    return EDisplayOrientationNormal;
#else
    Q_UNUSED(orientation);
    return 0;
#endif
}

/*!
  Maps one of the four explicit TRenderOrientation values to a
  Qt::Orientation.

  Returns false if the value is unknown or is EDisplayOrientationAuto or
  EDisplayOrientationIgnore.

  Note that there is no mapping in case of EDisplayOrientationAuto, this is
  because the clients need to know the difference between an automatic and a
  forced orientation.

  \internal
 */
bool HbOrientationStatus::mapRenderOriToOri(int renderOrientation, Qt::Orientation &orientation)
{
#if defined(Q_OS_SYMBIAN) && defined(HB_WSERV_HAS_RENDER_ORIENTATION)
    bool result = false;
    HbDeviceProfileList *deviceProfiles = HbDeviceProfilePrivate::deviceProfiles();
    if (deviceProfiles) {
        // The default profile must have rotation angle 0.
        qreal angles[] = { 0, 360 };
        switch (renderOrientation) {
        case EDisplayOrientationNormal:
            // already set up
            break;
        case EDisplayOrientation180:
            angles[0] = 180;
            angles[1] = -180;
            break;
        case EDisplayOrientation90CW:
            angles[0] = 90;
            angles[1] = -270;
            break;
        case EDisplayOrientation270CW:
            angles[0] = 270;
            angles[1] = -90;
            break;
        default:
            // stop right now if the value is unknown (or e.g. EDisplayOrientationAuto)
            return false;
        }
        foreach (const DeviceProfile &profile, *deviceProfiles) {
            for (int i = 0; i < sizeof(angles) / sizeof(qreal); ++i) {
                if (angles[i] == profile.mOrientationAngle) {
                    if (profile.mLogicalSize.width() < profile.mLogicalSize.height()) {
                        orientation = Qt::Vertical;
                    } else {
                        orientation = Qt::Horizontal;
                    }
                    result = true;
                    break;
                }
            }
            if (result) {
                break;
            }
        }
    }
    return result;
#else
    Q_UNUSED(renderOrientation);
    Q_UNUSED(orientation);
    return false;
#endif
}

QString HbOrientationStatus::oriToString(Qt::Orientation orientation)
{
    QString str;
    switch (orientation) {
    case Qt::Vertical:
        str = QLatin1String("prt");
        break;
    case Qt::Horizontal:
        str = QLatin1String("lsc");
        break;
    }
    return str;
}

QString HbOrientationStatus::renderOriToString(int renderOrientation)
{
    QString str;
#if defined(Q_OS_SYMBIAN) && defined(HB_WSERV_HAS_RENDER_ORIENTATION)
    switch (renderOrientation) {
    case EDisplayOrientationNormal:
        str = QLatin1String("normal");
        break;
    case EDisplayOrientation90CW:
        str = QLatin1String("90cw");
        break;
    case EDisplayOrientation180:
        str = QLatin1String("180");
        break;
    case EDisplayOrientation270CW:
        str = QLatin1String("270cw");
        break;
    case EDisplayOrientationAuto:
        str = QLatin1String("auto");
        break;
    case EDisplayOrientationIgnore:
        str = QLatin1String("ignore");
        break;
    }
#else
    Q_UNUSED(renderOrientation);
#endif
    return str;
}

#ifdef WS_RENDER_ORI_LISTENER

class HbWsOrientationListenerPrivate : public CActive
{
public:
    HbWsOrientationListenerPrivate();
    ~HbWsOrientationListenerPrivate();
    void listen();
    void DoCancel();
    void RunL();
    QObject *mReceiver;
    const char *mMember;
    RProperty mProperty;
};

HbWsOrientationListenerPrivate::HbWsOrientationListenerPrivate()
    : CActive(EPriorityStandard), mReceiver(0)
{
    CActiveScheduler::Add(this);
    mProperty.Attach(KRenderOrientationCategory, KRenderOrientationKey);
}

HbWsOrientationListenerPrivate::~HbWsOrientationListenerPrivate()
{
    Cancel();
    mProperty.Close();
}

void HbWsOrientationListenerPrivate::listen()
{
    if (!IsActive()) {
        mProperty.Subscribe(iStatus);
        SetActive();
    }
}

void HbWsOrientationListenerPrivate::DoCancel()
{
    mProperty.Cancel();
}

void HbWsOrientationListenerPrivate::RunL()
{
    if (iStatus == KErrNone) {
        listen();
        int val = 0;
        if (mReceiver && mProperty.Get(val) == KErrNone) {
            QMetaObject::invokeMethod(mReceiver, mMember, Q_ARG(int, val));
        }
    }
}

#endif // WS_RENDER_ORI_LISTENER

void HbOrientationStatus::HbWsOrientationListener::start(QObject *receiver, const char *member)
{
#ifdef WS_RENDER_ORI_LISTENER
    if (!d) {
        d = new HbWsOrientationListenerPrivate;
    }
    d->mReceiver = receiver;
    d->mMember = member;
    d->listen();
#else
    Q_UNUSED(receiver);
    Q_UNUSED(member);
#endif
}

void HbOrientationStatus::HbWsOrientationListener::stop()
{
#ifdef WS_RENDER_ORI_LISTENER
    if (d) {
        d->Cancel();
    }
#endif
}

bool HbOrientationStatus::HbWsOrientationListener::get(int &renderOrientation)
{
#ifdef WS_RENDER_ORI_LISTENER
    if (!d) {
        d = new HbWsOrientationListenerPrivate;
    }
    return d->mProperty.Get(renderOrientation) == KErrNone;
#else
    Q_UNUSED(renderOrientation);
    return false;
#endif
}

HbOrientationStatus::HbWsOrientationListener::HbWsOrientationListener()
    : d(0)
{
}

HbOrientationStatus::HbWsOrientationListener::~HbWsOrientationListener()
{
#ifdef WS_RENDER_ORI_LISTENER
    delete d;
#endif
}
