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
#include "hbcorepskeys_p.h"
#include "hbforegroundwatcher_p.h"

// UID for process checking (write orientation value only when in theme server)
const TUid KWriterServerUid = KHbPsHardwareCoarseOrientationCategoryUid;

// PS related constants
_LIT_SECURITY_POLICY_PASS(KRdPolicy); // all pass
_LIT_SECURITY_POLICY_S0(KWrPolicy, KWriterServerUid.iUid); // pass writer server
#else
#include <QSettings>
#endif

static HbOrientationStatus *orientationStatus = 0;

void HbOrientationStatus::init(QObject *parent, Qt::Orientation defaultOrientation)
{
#ifdef Q_OS_SYMBIAN
    if (!orientationStatus) {
        // Create orientation publisher only if current process is writer server
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
    mProperty.Close();
#endif
}

bool HbOrientationStatus::currentOrientation(Qt::Orientation &orientation)
{
    bool success = false;
#ifdef Q_OS_SYMBIAN
    if (!orientationStatus) {
        int currentOrientation = Qt::Vertical;
        if (RProperty::Get(KHbPsHardwareCoarseOrientationCategoryUid,
                           KHbPsHardwareCoarseOrientationKey, currentOrientation) == KErrNone) {
            orientation = (Qt::Orientation)currentOrientation;
            success = true;
        }
    }
#else
    //Read startup orientation from QSettings
    QSettings settings("Nokia", "HbStartUpDeskTopSensors");
    if (settings.value("Orientation").toInt() == 1) {
        orientation = Qt::Horizontal;
        success = true;
    } else if (settings.value("Orientation").toInt() == 2) {
        orientation = Qt::Vertical;
        success = true;
    }
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
    HbForegroundWatcher::instance()->setSensorListener(mSensorListener);
    // Create orientation property
    RProperty::Define(KHbPsHardwareCoarseOrientationCategoryUid, KHbPsHardwareCoarseOrientationKey,
                      RProperty::EInt, KRdPolicy, KWrPolicy);
    mProperty.Attach(KHbPsHardwareCoarseOrientationCategoryUid, KHbPsHardwareCoarseOrientationKey);
    storeOrientation(defaultOrientation);
#else
    Q_UNUSED(parent);
    Q_UNUSED(defaultOrientation);
#endif
}

void HbOrientationStatus::sensorOrientationChanged(Qt::Orientation newOrientation)
{
    storeOrientation(newOrientation);
}

void HbOrientationStatus::sensorStatusChanged(bool status, bool notify)
{
    Q_UNUSED(notify)
#ifdef Q_OS_SYMBIAN
    if (status == false) {
        storeOrientation(mDefaultOrientation);
    }
#else
    Q_UNUSED(status)
#endif
}

void HbOrientationStatus::storeOrientation(Qt::Orientation orientation)
{
#ifdef Q_OS_SYMBIAN
    mProperty.Set(orientation);
#else
    Q_UNUSED(orientation);
#endif
}

