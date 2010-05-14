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


#include "hbsysteminfo_p.h"
#if defined(Q_OS_SYMBIAN)
#include "hbsysteminfosym_p_p.h"
#else
#include "hbsysteminfowin_p_p.h"
#endif // defined(Q_OS_SYMBIAN)


HbSystemInfo::HbSystemInfo(QObject *parent, bool writer)
: QObject(parent), d_ptr(new HbSystemInfoPrivate)
{
    Q_D(HbSystemInfo);
    d->q_ptr = this;
    d->init(writer);    
}

HbSystemInfo::~HbSystemInfo()
{
    delete d_ptr;
}

int HbSystemInfo::networkSignalStrength() const
{
    Q_D(const HbSystemInfo);
    return d->networkSignalStrength();
}

QSystemNetworkInfo::NetworkMode HbSystemInfo::networkMode() const
{
    Q_D(const HbSystemInfo);
    return d->networkMode();
}

int HbSystemInfo::batteryLevel() const
{
    Q_D(const HbSystemInfo);
    return d->batteryLevel();
}

QSystemDeviceInfo::PowerState HbSystemInfo::powerState() const
{
    Q_D(const HbSystemInfo);
    return d->powerState();
}
