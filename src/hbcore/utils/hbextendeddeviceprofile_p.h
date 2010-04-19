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

#ifndef HBEXTENDEDDEVICEPROFILE_H
#define HBEXTENDEDDEVICEPROFILE_H

#include <hbglobal.h>
#include <QtGlobal>
#include <QSizeF>
#include <hbdeviceprofile.h>

class HB_AUTOTEST_EXPORT HbExtendedDeviceProfile : public HbDeviceProfile
{
public:
    HbExtendedDeviceProfile();
    HbExtendedDeviceProfile(const HbExtendedDeviceProfile &other);
    explicit HbExtendedDeviceProfile(const HbDeviceProfile &other);
    explicit HbExtendedDeviceProfile(const QString &name);

    HbExtendedDeviceProfile &operator=(const HbExtendedDeviceProfile &other);   

    void setName(const QString &name);
    void setLogicalSize(const QSize &size);
    void setPhysicalSize(const QSizeF &size);
    void setOrientation(Qt::Orientation orientation);
    void setAlternateProfileName(const QString &name);
    void setUnitValue(qreal value);
};


#endif // HBEXTENDEDDEVICEPROFILE_H
