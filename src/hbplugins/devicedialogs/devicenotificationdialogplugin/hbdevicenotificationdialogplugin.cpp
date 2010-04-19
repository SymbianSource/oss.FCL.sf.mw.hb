/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbPlugins module of the UI Extensions for Mobile.
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

#include <QtPlugin>

#include <hbdevicedialog.h>
#include <hbdevicedialogtrace_p.h>
#include "hbdevicenotificationdialogplugin_p.h"
#include "hbdevicenotificationdialogwidget_p.h"
#include "hbdevicenotificationdialogpluginerrors_p.h"

Q_EXPORT_PLUGIN2(devicenotificationdialogplugin, HbDeviceNotificationDialogPlugin)

// This plugin implements one device dialog type
static const struct {
    const char *mTypeString;
} dialogInfos[] = {
    {"com.nokia.hb.devicenotificationdialog/1.0"}
};

class HbDeviceNotificationDialogPluginPrivate
{
public:
    HbDeviceNotificationDialogPluginPrivate() {mError = NoError;}

    int mError;
};

// Constructor
HbDeviceNotificationDialogPlugin::HbDeviceNotificationDialogPlugin()
{
    TRACE_ENTRY
    d = new HbDeviceNotificationDialogPluginPrivate;
    TRACE_EXIT
}

// Destructor
HbDeviceNotificationDialogPlugin::~HbDeviceNotificationDialogPlugin()
{
    TRACE_ENTRY
    delete d;
    TRACE_EXIT
}

// Check if client is allowed to use device dialog widget
bool HbDeviceNotificationDialogPlugin::accessAllowed(const QString &deviceDialogType,
    const QVariantMap &parameters, const QVariantMap &securityInfo) const
{
    TRACE_ENTRY
    Q_UNUSED(deviceDialogType)
    Q_UNUSED(parameters)
    Q_UNUSED(securityInfo)

    // This plugin doesn't perform operations that may compromise security. All clients
    // are allowed to use.
    return true;
    TRACE_EXIT
}

// Create device dialog widget
HbDeviceDialogInterface *HbDeviceNotificationDialogPlugin::createDeviceDialog(
    const QString &deviceDialogType, const QVariantMap &parameters)
{
    TRACE_ENTRY
    Q_UNUSED(deviceDialogType)
    d->mError = NoError;

    HbDeviceDialogInterface *ret(0);
    QVariantMap params = parameters;

    HbDeviceNotificationDialogWidget *deviceDialog =
        new HbDeviceNotificationDialogWidget(params);
    d->mError = deviceDialog->deviceDialogError();
    if (d->mError != NoError) {
        delete deviceDialog;
        deviceDialog = 0;
    }
    ret = deviceDialog;

    TRACE_EXIT
    return ret;
}

// Return device dialog flags
bool HbDeviceNotificationDialogPlugin::deviceDialogInfo(const QString &deviceDialogType,
        const QVariantMap &parameters, DeviceDialogInfo *info) const
{
    TRACE_ENTRY
    Q_UNUSED(deviceDialogType);
    Q_UNUSED(parameters);

    info->group = DeviceNotificationDialogGroup;
    info->flags = NoDeviceDialogFlags;
    info->priority = DefaultPriority;

    TRACE_EXIT
    return true;
}

// Return device dialog types this plugin implements
QStringList HbDeviceNotificationDialogPlugin::deviceDialogTypes() const
{
    TRACE_ENTRY
    QStringList types;
    const int numTypes = sizeof(dialogInfos) / sizeof(dialogInfos[0]);
    for(int i = 0; i < numTypes; i++) {
        types.append(dialogInfos[i].mTypeString);
    }
    TRACE_EXIT
    return types;
}

// Return plugin flags
HbDeviceDialogPlugin::PluginFlags HbDeviceNotificationDialogPlugin::pluginFlags() const
{
    TRACE_ENTRY
    TRACE_EXIT
    return NoPluginFlags;
}

// Return last error
int HbDeviceNotificationDialogPlugin::error() const
{
    TRACE_ENTRY
    TRACE_EXIT
    return d->mError;
}
