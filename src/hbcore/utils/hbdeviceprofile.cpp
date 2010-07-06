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

#include "hbdeviceprofile.h"
#include "hbdeviceprofiledatabase_p.h"
#include "hbinstance_p.h"
#include "hbmemorymanager_p.h"
#include "hbtheme_p.h"
#include "hbthemeclient_p.h"
#include <hbmainwindow.h>
#include "hbmainwindow_p.h"

#include <QDebug>

// To store the pointer to the deviceProfiles at the client side.
static HbDeviceProfileList *deviceProfilesList = 0;

#define MM_PER_INCH 25.4

/*!
    @stable
    @hbcore
    \class HbDeviceProfile
    \brief HbDeviceProfile holds read-only device parameters.

    Instances of this class hold both concrete device specific information
    (e.g. physical display size) and aspects that have been configured for
    a specific device (e.g. orientation, unit value).

*/

/*!
    Default constructor, a null profile.
*/
HbDeviceProfile::HbDeviceProfile(): d_ptr(new HbDeviceProfilePrivate)
{
}

/*!
    Copy constructor.
    \param other source profile.
*/
HbDeviceProfile::HbDeviceProfile(const HbDeviceProfile &other)
{
    d_ptr = other.d_ptr;
}

/*!
    Constructor for information based on profile name.
    If profile with specified name is not found, then default
    constructed instance is provided.

    \param name name of the profile
    \sa profileNames()
*/
HbDeviceProfile::HbDeviceProfile(const QString &name) : d_ptr(new HbDeviceProfilePrivate)
{
    if (d_ptr->deviceProfiles()) {
        int count = deviceProfilesList->count();
        bool found(false);
        for (int i = 0; !found && i < count; i++) {
            if (deviceProfilesList->at(i).mName == name) {
                d_ptr->mProfile = deviceProfilesList->at(i);
                found = true;
            }
        }
        if (!found) {
            qWarning() << "Device profile" << name << "not found!";
        }
    }
}


/*!
    Destructor.
*/
HbDeviceProfile::~HbDeviceProfile()
{
}

/*!
    Assignment operator.
    \param other source profile.
    \return reference to this profile.
*/
HbDeviceProfile &HbDeviceProfile::operator=(const HbDeviceProfile &other)
{
    if (this != &other) {
        d_ptr = other.d_ptr;
    }
    return *this;
}

/*!
    Returns \c true if this profile is a null profile (default constructed profile).
*/
bool HbDeviceProfile::isNull() const
{
    return d_ptr->mProfile.mName.isEmpty();
}

/*!
    Returns name of this profile.
    Note: name is supposed to be unique.
*/
QString HbDeviceProfile::name() const
{
    return d_ptr->mProfile.mName;
}

/*!
    Returns logical screen size in pixels.
*/
QSize HbDeviceProfile::logicalSize() const
{
    return d_ptr->mProfile.mLogicalSize;
}

/*!
    Returns physical screen size in mm.
*/
QSizeF HbDeviceProfile::physicalSize() const
{
    QSizeF physicalSize(d_ptr->mProfile.mLogicalSize);
    physicalSize /= ppmValue();
    return physicalSize;
}

/*!
    Returns logical orientation (\c Qt::Vertical for portrait, \c Qt::Horizontal for landscape).
*/
Qt::Orientation HbDeviceProfile::orientation() const
{
    QSize s = d_ptr->mProfile.mLogicalSize;
    return (s.width() > s.height()) ? Qt::Horizontal : Qt::Vertical;
}

/*!
    Returns the amount of rotation in degrees for the transformed orientation.
    Supported ones are 0 (no rotate transform), 90 and 270.
*/
qreal HbDeviceProfile::orientationAngle() const
{
    return d_ptr->mProfile.mOrientationAngle;
}
/*!
  Returns the type of the screen: touch or non-touch
 */
bool HbDeviceProfile::touch() const
{
    return d_ptr->mProfile.mTouch;
}

/*!
    Returns name of alternate profile.

    Currently this holds information of the profile which is activated
    on layout switch. Use this information if you need to optimize
    layout switch.
*/
QString HbDeviceProfile::alternateProfileName() const
{
    return d_ptr->mProfile.mAltName;
}

/*!
    Returns standard unit value (denoted by 'un').

    The unit value is a display-specific multiplier. It is used in intenal
    layout calculation.
*/
qreal HbDeviceProfile::unitValue() const
{
    if (d_ptr->mProfile.mUnitValue <= 1) {
        return 6.7;
    }
    return d_ptr->mProfile.mUnitValue;
}

/*!
    Returns pixels per millimeter value.
*/
qreal HbDeviceProfile::ppmValue() const
{
    return d_ptr->mProfile.mPpiValue / MM_PER_INCH;
}

/*!
    Returns current global profile reflecting properties of primary display.

    Usually, you should not use this method. Instead, use one of \c profile
    methods.
*/
HbDeviceProfile HbDeviceProfile::current()
{
    return HbInstancePrivate::d_ptr()->profile();
}

/*!
    Returns current profile for this window.
*/
HbDeviceProfile HbDeviceProfile::profile(const HbMainWindow *window)
{
    if (!window) {
        return current();
    }
    HbMainWindow *wnd = const_cast<HbMainWindow *>(window);
    HbDeviceProfile profile = HbMainWindowPrivate::d_ptr(wnd)->profile();
    if (profile.isNull()) {
        profile = HbMainWindowPrivate::d_ptr(wnd)->adjustedProfile(current());
    }
    return profile;
}

/*!
    Returns current profile for this graphics item.

    Graphics item must be tied to a scene and scene needs to be part of
    main window. Otherwise, value returned by \c current is provided.

    \param item item of which profile is needed.
    \return device profile.
*/
HbDeviceProfile HbDeviceProfile::profile(const QGraphicsItem *item)
{
    if (!item) {
        return current();
    }
    QGraphicsScene *scene = item->scene();
    if (!scene) {
        return current();
    }
    QList<QGraphicsView *> views = scene->views();
    foreach(QGraphicsView * view, views) {
        HbMainWindow *window = qobject_cast<HbMainWindow *>(view);
        if (window) {
            HbDeviceProfile profile =
                HbMainWindowPrivate::d_ptr(window)->profile();

            if (!profile.isNull()) {
                return profile;
            } else {
                return HbMainWindowPrivate::d_ptr(window)->adjustedProfile(current());
            }
        }
    }
    return current();
}

/*!
    Returns a list of profile names (each representing a basic configuration).
*/
QStringList HbDeviceProfile::profileNames()
{
    return HbDeviceProfilePrivate::profileNames();
}

QStringList HbDeviceProfilePrivate::profileNames()
{
    QStringList profileNames;
    if (deviceProfiles()) {
        int profilesCount = deviceProfilesList->count();
        for (int i = 0; i < profilesCount ; i++) {
            profileNames.append(deviceProfilesList->at(i).mName);
        }
    }
    return profileNames;
}

HbDeviceProfilePrivate::HbDeviceProfilePrivate()
{
}

HbDeviceProfilePrivate::~HbDeviceProfilePrivate()
{
}

HbDeviceProfileList *HbDeviceProfilePrivate::deviceProfiles()
{
    if (!deviceProfilesList) {
        // Will result in IPC call. gets the shared memory offset from themeserver.
        deviceProfilesList = HbThemeClient::global()->deviceProfiles();
    }

    if (!deviceProfilesList) {
        // This is fall back.Create/Get the HbDeviceProfileDatabase Instance at
        // the client side and read the deviceProfilesList.
        HbDeviceProfileDatabase *deviceProfileDataBase =
            HbDeviceProfileDatabase::instance(HbMemoryManager::HeapMemory);
        deviceProfilesList = HbMemoryUtils::getAddress<HbDeviceProfileList>(HbMemoryManager::HeapMemory,
                             deviceProfileDataBase->deviceProfilesOffset());
    }
    return deviceProfilesList;
}
// end of file
