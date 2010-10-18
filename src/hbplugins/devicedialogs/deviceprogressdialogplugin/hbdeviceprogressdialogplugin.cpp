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
#include "hbdeviceprogressdialogplugin_p.h"
#include "hbdeviceprogressdialogwidget_p.h"
#include "hbdeviceprogressdialogpluginerrors_p.h"

Q_EXPORT_PLUGIN2(deviceprogressdialogplugin, HbDeviceProgressDialogPlugin)

// This plugin implements one device dialog type
static const struct {
    const char *mTypeString;
} dialogInfos[] = {
    {"com.nokia.hb.deviceprogressdialog/1.0"}
};

/// \cond
class HbDeviceProgressDialogPluginPrivate
{
public:
    HbDeviceProgressDialogPluginPrivate() {mError = NoError;}

    int mError;
};
/// \endcond

// Constructor
HbDeviceProgressDialogPlugin::HbDeviceProgressDialogPlugin()
{
    TRACE_ENTRY
    d = new HbDeviceProgressDialogPluginPrivate;
    TRACE_EXIT
}

// Destructor
HbDeviceProgressDialogPlugin::~HbDeviceProgressDialogPlugin()
{
    TRACE_ENTRY
    delete d;
    TRACE_EXIT
}

// Check if client is allowed to use device dialog widget
bool HbDeviceProgressDialogPlugin::accessAllowed(const QString &deviceDialogType,
    const QVariantMap &parameters, const QVariantMap &securityInfo) const
{
    TRACE_ENTRY
    Q_UNUSED(deviceDialogType)

#ifdef Q_OS_SYMBIAN
    // If show group is higher than normal, a swEvent capability is required
    if (showLevel(parameters) != NormalLevel) {
        return (securityInfo["sym-caps"].toInt() >> ECapabilitySwEvent) & 1;
    } else {
        return true; // all clients are allowed to use.
    }
#else
    Q_UNUSED(parameters)
    Q_UNUSED(securityInfo)
    // All clients are allowed to use.
    return true;
#endif // Q_OS_SYMBIAN
}

// Create device dialog widget
HbDeviceDialogInterface *HbDeviceProgressDialogPlugin::createDeviceDialog(
    const QString &deviceDialogType,
    const QVariantMap &parameters)
{
    TRACE_ENTRY
    d->mError = NoError;

    HbDeviceDialogInterface *ret(0);
    int i(0);
    const int numTypes = sizeof(dialogInfos) / sizeof(dialogInfos[0]);
    for(i = 0; i < numTypes; i++) {
        if (dialogInfos[i].mTypeString == deviceDialogType) {
            break;
        }
    }
    if (i < numTypes) {
        HbProgressDialog::ProgressDialogType dialogType;
        QVariantMap params = parameters;
        if (HbDeviceProgressDialogWidget::getDialogType(dialogType, params)) {
            HbDeviceProgressDialogWidget *deviceDialog =
                new HbDeviceProgressDialogWidget(dialogType, params);
            d->mError = deviceDialog->deviceDialogError();
            if (d->mError != NoError) {
                delete deviceDialog;
                deviceDialog = 0;
            }
            ret = deviceDialog;
        } else {
            d->mError = ParameterError;
            ret = 0;
        }
    } else {
        d->mError = UnknownDeviceDialogError;
        ret = 0;
    }

    TRACE_EXIT
    return ret;
}

// Return information of device dialog the plugin creates
bool HbDeviceProgressDialogPlugin::deviceDialogInfo(const QString &deviceDialogType,
    const QVariantMap &parameters, DeviceDialogInfo *info) const
{
    TRACE_ENTRY
    Q_UNUSED(deviceDialogType)

    unsigned int level = static_cast<unsigned int>(showLevel(parameters));
    if (level > CriticalLevel) {
        return false;
    }

    info->group = GenericDeviceDialogGroup;
    info->flags = level == NormalLevel ? NoDeviceDialogFlags : SecurityCheck;
    info->showLevel = static_cast<ShowLevel>(level);
    return true;
}

// Return device dialog types this plugin implements
QStringList HbDeviceProgressDialogPlugin::deviceDialogTypes() const
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
HbDeviceDialogPlugin::PluginFlags HbDeviceProgressDialogPlugin::pluginFlags() const
{
    TRACE_ENTRY
    TRACE_EXIT
    return NoPluginFlags;
}

// Return last error
int HbDeviceProgressDialogPlugin::error() const
{
    TRACE_ENTRY
    TRACE_EXIT
    return d->mError;
}

// Search parameters for show group
int HbDeviceProgressDialogPlugin::showLevel(const QVariantMap &parameters)
{
    int level = 0;
    const QString propertyName("showLevel"); 
    if (parameters.contains(propertyName)) {
        level = parameters[propertyName].toInt(); 
    }
    return level;
}
