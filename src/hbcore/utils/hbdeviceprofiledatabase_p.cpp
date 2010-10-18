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

#include "hbdeviceprofiledatabase_p.h"

#include <QFile>
#include "hbdeviceprofilereader_p.h"
#include "hbdevicemodeinfo_p.h"
#include "hbscreenmode_p.h"
#ifdef Q_OS_SYMBIAN
#include "hborientationstatus_p.h"
#endif //Q_OS_SYMBIAN

#include <QCoreApplication>
#include <QApplication>

#define SKIP_WSINI

#define DISPLAY_DEFINITION_FILE "displaydefinition.xml"


/*
    \class HbDeviceProfileDatabase
    \brief HbDeviceProfileDatabase provides access to supported profile information.

    This uses HbDeviceProfileReader to parse the display definition xml and the
    device mode ini files respectively. After parsing the files
    HbDeviceProfileDatabase will store the list of profiles i.e
    HbDeviceProfileList in to the shared memory and maintains the offset of it,
    this is done in the Theme Server.

    Clients only use this class directly when they are unable to get the
    deviceprofile information from the Theme Server, normally the only instance
    of this class is the one used by Theme Server.

    This class is not intended to be used directly. Instead, use \c HbDeviceProfile.
    
    \sa HbDeviceProfile
    \internal
*/

HbDeviceProfileDatabase *HbDeviceProfileDatabase::instance(HbMemoryManager::MemoryType type)
{
    static HbDeviceProfileDatabase info(type);
    return &info;
}

/*!
    Constructor.
*/
HbDeviceProfileDatabase::HbDeviceProfileDatabase(HbMemoryManager::MemoryType type)
    : mDeviceProfiles(0), mDeviceModes(0), mDeviceProfilesOffset(-1), mType(type)
{
    GET_MEMORY_MANAGER(mType);
    try {
        mDeviceProfilesOffset = manager->alloc(sizeof(HbDeviceProfileList));
        mDeviceProfiles = new((char *)manager->base() + mDeviceProfilesOffset)
        HbDeviceProfileList(mType);
        init();
    } catch (std::exception &) {
        if (mDeviceProfilesOffset != -1) {
            if (mDeviceProfiles) {
                mDeviceProfiles->~HbDeviceProfileList();
                mDeviceProfiles = 0;
            }
            manager->free(mDeviceProfilesOffset);
            mDeviceProfilesOffset = -1;
        }
    }
}

qptrdiff  HbDeviceProfileDatabase::deviceProfilesOffset()
{
    return mDeviceProfilesOffset;
}


/*!
    Initialization method.
*/
void HbDeviceProfileDatabase::init()
{
    HbDeviceProfileReader reader(mDeviceProfiles, mType);

    // resolve correct displaydefinition.xml path for emulator and HW (z:/resource)
    // or desktop (Qt resource)

    // from HW and emulator
    QFile file("z:/resource/displaydefinition.xml");

    if (!file.exists()) {
        file.setFileName(":displaydefinition.xml");
    }
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning()
                << "HbDeviceProfileDatabase::init : opening file failed:";
    } else {
        reader.read(&file);
    }

#ifndef SKIP_WSINI
    // Get the device modes from the device itself
    mDeviceModes = new HbDeviceModeInfo();




#if defined(Q_OS_SYMBIAN)
    const QRectF screenGeometry(qApp->desktop()->screenGeometry());
    QSizeF screenSize(screenGeometry.size());

    int i = 0;
    bool done = false;
    while (i < mDeviceProfiles->size() && !done) {
        DeviceProfile profile = mDeviceProfiles->at(i);
        if (profile.mLogicalSize.width() == screenSize.width() &&
                profile.mLogicalSize.height() == screenSize.height()) {
            mDeviceProfiles->remove(i);
            mDeviceProfiles->insert(0, profile);
            done = true;
        }
        i++;
    }
#endif

    completeProfileData();
#endif
#ifdef Q_OS_SYMBIAN
    initOrientationStatus();
#endif //Q_OS_SYMBIAN
}

void HbDeviceProfileDatabase::completeProfileData()
{
    // Mark the device profiles as valid or not depending whether they match the device modes
    // If there are no device modes read in, then no removal is done at all
    if (mDeviceProfiles && mDeviceModes && mDeviceModes->modeNumbers().count() > 0) {
        for (int i = mDeviceProfiles->size() - 1; i >= 0 ; --i) {
            QSize size(mDeviceProfiles->at(i).mLogicalSize);
            bool found(false);
            QList<int> l = mDeviceModes->modeNumbers();
            for (int j = 0; j < l.count(); j++) {
                HbScreenMode *mode = mDeviceModes->mode(l[j]);
                if (mode->pixelSize() == size) {
                    found = true;
                }
            }
            if (!found && mDeviceProfiles->size() > 1) {
                // Do not remove the 0-th display mode if is the only one left!
                mDeviceProfiles->remove(i);
            }
        }
    }

    if (mDeviceProfiles) {
        const int numberOfProfiles = mDeviceProfiles->size();
        for (int i = 0; i < numberOfProfiles; ++i) {
            if (mDeviceProfiles->at(i).mAltName.isEmpty()) {
                QSize size(mDeviceProfiles->at(i).mLogicalSize);
                QSize altLogicalSize(size.height(), size.width());
                for (int j = 0; j < numberOfProfiles; ++j) {
                    if (i != j && altLogicalSize == mDeviceProfiles->at(j).mLogicalSize) {
                        mDeviceProfiles->at(i).mAltName = mDeviceProfiles->at(j).mName;
                        break;
                    }
                }
            }
        }
    }
}


#ifdef Q_OS_SYMBIAN
void HbDeviceProfileDatabase::initOrientationStatus()
{
    // The ThemeServer will publish orientation data as a P&S property.
    // This is done in HbOrientationStatus, so initialize it here.
    // Skip if an app instantiates this class as a fallback.
    if (HbMemoryManager::SharedMemory == mType) {
        Qt::Orientation defaultOrientation = Qt::Vertical;
        if (mDeviceProfiles && mDeviceProfiles->count()) {
            QSize s = mDeviceProfiles->at(0).mLogicalSize;
            defaultOrientation = (s.width() > s.height()) ? Qt::Horizontal : Qt::Vertical;
        }
        // Orientation status keeps track of current device orientation.
        // It is initialized at device profile database creation.
        HbOrientationStatus::init(qApp, defaultOrientation);
    }
}
#endif //Q_OS_SYMBIAN
