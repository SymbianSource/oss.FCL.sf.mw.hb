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

#ifndef HBSYSTEMINFO_H
#define HBSYSTEMINFO_H

#include <hbglobal.h>
#include <QObject>
#include <hbforegroundwatcher_p.h>
#include <qsysteminfo.h>
using QtMobility::QSystemNetworkInfo;
using QtMobility::QSystemDeviceInfo;

class HbSystemInfoPrivate;

class HbSystemInfo : public QObject
{
    Q_OBJECT

public: 
    explicit HbSystemInfo(QObject *parent = 0, bool writer = false);
    virtual ~HbSystemInfo();

    QSystemNetworkInfo::NetworkStatus networkStatus() const;
    int networkSignalStrength() const;
    QSystemNetworkInfo::NetworkMode networkMode() const;
    int batteryLevel() const;
    QSystemDeviceInfo::PowerState powerState() const;
    
signals:    
    void networkSignalStrengthChanged(QSystemNetworkInfo::NetworkMode, int);
    void networkModeChanged(QSystemNetworkInfo::NetworkMode, QSystemNetworkInfo::NetworkStatus);
    void batteryLevelChanged(int);
    void powerStateChanged(QSystemDeviceInfo::PowerState);
        
private:
    HbSystemInfoPrivate *d_ptr;
    Q_DECLARE_PRIVATE_D(d_ptr, HbSystemInfo)
    Q_DISABLE_COPY(HbSystemInfo)             
};

#endif // HBSYSTEMINFO_H
