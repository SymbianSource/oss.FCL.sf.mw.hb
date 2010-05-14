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

#ifndef HBDEVICEDIALOGPLUGIN_H
#define HBDEVICEDIALOGPLUGIN_H

#include <QObject>
#include <QVariantMap>
#include <QStringList>
#include <QtPlugin>
#include <QFlags>

#include <hbglobal.h>
#include <hbdevicedialog.h>

class HbDeviceDialogInterface;

class HbDeviceDialogPluginInterface
{
public:
    virtual HbDeviceDialogInterface *createDeviceDialog(const QString &deviceDialogType,
        const QVariantMap &parameters) = 0;
    virtual ~HbDeviceDialogPluginInterface() {}
};
Q_DECLARE_INTERFACE(HbDeviceDialogPluginInterface, "com.nokia.hb.HbDeviceDialogPlugin/1.0")

class HB_CORE_EXPORT HbDeviceDialogPlugin : public QObject, public HbDeviceDialogPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(HbDeviceDialogPluginInterface)

public:
    enum PluginFlag{
        NoPluginFlags = 0x00,
        PreloadPlugin = 0x01,
        KeepPluginLoaded = 0x02
    };
    Q_DECLARE_FLAGS(PluginFlags, PluginFlag)

    enum DeviceDialogGroup{
        GenericDeviceDialogGroup,
        DeviceNotificationDialogGroup,
        IndicatorGroup,
        SecurityGroup,
        CriticalGroup
    };

    enum DeviceDialogFlag{
        NoDeviceDialogFlags = 0x00,
        SharedDeviceDialog = 0x01,
        SecurityCheck = 0x02
    };
    Q_DECLARE_FLAGS(DeviceDialogFlags, DeviceDialogFlag)

    enum {
        DefaultPriority = 0
    };

    struct DeviceDialogInfo {
        DeviceDialogGroup group;
        int flags;
        int priority;
        quint32 spare[8];
    };

    virtual bool accessAllowed(const QString &deviceDialogType,
        const QVariantMap &parameters, const QVariantMap &securityInfo) const = 0;
    virtual bool deviceDialogInfo(const QString &deviceDialogType,
        const QVariantMap &parameters, DeviceDialogInfo *info) const = 0;
    virtual QStringList deviceDialogTypes() const = 0;
    virtual PluginFlags pluginFlags() const = 0;
    virtual int error() const = 0;
    virtual ~HbDeviceDialogPlugin();

protected:

    HbDeviceDialogPlugin();
};

#endif // HBDEVICEDIALOGPLUGIN_H
