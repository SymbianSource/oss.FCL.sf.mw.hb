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
#include "hbindicatormenuplugin_p.h"
#include "hbindicatormenu_p.h"
#include "hbindicatormenupluginerrors_p.h"

Q_EXPORT_PLUGIN2(indicatormenuplugin, HbIndicatorMenuPlugin)

// This plugin implements one device dialog type
static const struct {
    const char *mTypeString;
} noteInfos[] = {
    {"com.nokia.hb.indicatormenu/1.0"}
};

class HbIndicatorMenuPluginPrivate
{
public:
    HbIndicatorMenuPluginPrivate() {mError = NoError;}

    int mError;
};

// Constructor
HbIndicatorMenuPlugin::HbIndicatorMenuPlugin()
{
    TRACE_ENTRY
    d = new HbIndicatorMenuPluginPrivate;
    TRACE_EXIT
}

// Destructor
HbIndicatorMenuPlugin::~HbIndicatorMenuPlugin()
{
    TRACE_ENTRY
    delete d;
    TRACE_EXIT
}

// Check if client is allowed to use device dialog widget
bool HbIndicatorMenuPlugin::accessAllowed(const QString &deviceDialogType,
    const QVariantMap &parameters, const QVariantMap &securityInfo) const
{
    Q_UNUSED(deviceDialogType)
    Q_UNUSED(parameters)
    Q_UNUSED(securityInfo)

    // This plugin doesn't perform operations that may compromise security. All clients
    // are allowed to use.
    return true;
}

// Create device dialog widget
HbDeviceDialogInterface *HbIndicatorMenuPlugin::createDeviceDialog(
    const QString &deviceDialogType, const QVariantMap &parameters)
{
    TRACE_ENTRY
    d->mError = NoError;

    HbDeviceDialogInterface *ret(0);
    if (noteInfos[0].mTypeString == deviceDialogType) {
        QVariantMap params = parameters;
        HbIndicatorMenu *indMenu = new HbIndicatorMenu(params);
        d->mError = indMenu->deviceDialogError();
        if (d->mError != NoError) {
            delete indMenu;
            indMenu = 0;
        }
        ret = indMenu;
    } else {
        d->mError = UnknownDeviceDialogError;
        ret = 0;
    }
    TRACE_EXIT
    return ret;
}

bool HbIndicatorMenuPlugin::deviceDialogInfo(const QString &deviceDialogType,
        const QVariantMap &parameters, DeviceDialogInfo *info) const
{
    TRACE_ENTRY
    Q_UNUSED(deviceDialogType)
    Q_UNUSED(parameters)

    info->group = IndicatorGroup;
    info->flags = NoDeviceDialogFlags;
    info->priority = DefaultPriority;

    TRACE_EXIT
    return true;
}

// Return device dialog types this plugin implements
QStringList HbIndicatorMenuPlugin::deviceDialogTypes() const
{
    TRACE_ENTRY
    QStringList types;
    // Actually there is only one type
    const int numTypes = sizeof(noteInfos) / sizeof(noteInfos[0]);
    for(int i = 0; i < numTypes; i++) {
        types.append(noteInfos[i].mTypeString);
    }
    TRACE_EXIT
    return types;
}

// Return plugin flags
HbDeviceDialogPlugin::PluginFlags HbIndicatorMenuPlugin::pluginFlags() const
{
    TRACE_ENTRY
    TRACE_EXIT
    return NoPluginFlags;
}

// Return last error
int HbIndicatorMenuPlugin::error() const
{
    TRACE_ENTRY
    TRACE_EXIT
    return d->mError;
}
