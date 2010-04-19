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

#include "hbextendeddeviceprofile_p.h"
#include "hbdeviceprofile_p.h"

/*
    \class HbExtendedDeviceProfile
    \brief HbExtendedDeviceProfile provides setter methods for \c HbDeviceProfile and 
    additional internal parameter accessors & setters.

    Note that it's assumed that each device profile has unique name. In addition,
    each device profile has 'alternate' profile for the other orientation.

    \sa HbDeviceProfile
    \internal
    \proto
*/


/*!
    Constructor.
*/
HbExtendedDeviceProfile::HbExtendedDeviceProfile()
: HbDeviceProfile()
{
}

/*!
    Copy constructor.
    \param other source profile.
*/
HbExtendedDeviceProfile::HbExtendedDeviceProfile(const HbExtendedDeviceProfile &other)
: HbDeviceProfile(other)
{
}

/*!
    Conversion constructor from \c HbDeviceProfile to \c HbExtendedDeviceProfile.
    \param other source profile.
*/
HbExtendedDeviceProfile::HbExtendedDeviceProfile(const HbDeviceProfile &other)
: HbDeviceProfile(other)
{
}

/*!
    Constructor for information based on profile name.
    If there is no profile with that name, result is default constructed instance.
*/
HbExtendedDeviceProfile::HbExtendedDeviceProfile(const QString &name)
: HbDeviceProfile(name)
{
}

/*!
    Assignment operator.
    \param other source profile.
    \return reference to this profile.
*/
HbExtendedDeviceProfile &HbExtendedDeviceProfile::operator=(const HbExtendedDeviceProfile &other)
{
    if (this != &other) {
		d_ptr = other.d_ptr;
    }
    return *this;
}

/*!
    Sets name of this profile.
    \param name name.
*/
void HbExtendedDeviceProfile::setName(const QString &name)
{
    d_ptr->mProfile.mName = name;
}

/*!
    Sets logical screen size in pixels.
    \param size logical screen size.
*/
void HbExtendedDeviceProfile::setLogicalSize(const QSize &size)
{
     d_ptr->mProfile.mLogicalSize = size;
}

/*!
    Sets physical screen size in mm.
    \param size physical screen size.
*/
void HbExtendedDeviceProfile::setPhysicalSize(const QSizeF &size)
{
     d_ptr->mProfile.mPhysicalSize = size;
}

/*!
    Sets orientation.
    \param orientation desired orientation.
*/
void HbExtendedDeviceProfile::setOrientation(Qt::Orientation orientation)
{
    d_ptr->mProfile.mOrientation = orientation;
}

/*!
    Sets name of alternate profile.
    Typically, orientation of alternative profile is opposite of this profile.
    \param name name of alternate profile.
*/
void HbExtendedDeviceProfile::setAlternateProfileName(const QString &name)
{
    d_ptr->mProfile.mAltName = name;
}

/*!
    Sets unit value.
    \param value desired new value.
*/
void HbExtendedDeviceProfile::setUnitValue(qreal value)
{
     d_ptr->mProfile.mUnitValue = value;
}

// end of file
