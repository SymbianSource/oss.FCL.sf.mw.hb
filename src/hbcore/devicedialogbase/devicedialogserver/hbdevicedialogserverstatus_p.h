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

#ifndef HBDEVICEDIALOGSERVERSTATUS_P_H
#define HBDEVICEDIALOGSERVERSTATUS_P_H

#include <hbglobal.h>

#include <QtGlobal>
#include <QObject>

class HbDeviceDialogServerStatusPrivate;

class HB_CORE_PRIVATE_EXPORT HbDeviceDialogServerStatus : public QObject
{
    Q_OBJECT

public:

    enum StatusFlag{
        NoFlags = 0x00,
        ShowingDialog = 0x01
    };
    Q_DECLARE_FLAGS(StatusFlags, StatusFlag)

    explicit HbDeviceDialogServerStatus(bool isServer, QObject *parent = 0);
    ~HbDeviceDialogServerStatus();

    void setStatus(StatusFlags status);
    StatusFlags status();
    void enableMonitoring(bool enable);
    bool isRunning();

signals:
    void statusChanged();

private:
    friend class HbDeviceDialogServerStatusPrivate;
    HbDeviceDialogServerStatusPrivate *d;
};

#endif  // HBDEVICEDIALOGSERVERSTATUS_P_H
