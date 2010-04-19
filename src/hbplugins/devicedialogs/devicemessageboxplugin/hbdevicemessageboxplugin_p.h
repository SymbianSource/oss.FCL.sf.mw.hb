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

#ifndef HBDEVICEMESSAGEBOXPLUGIN_P_H
#define HBDEVICEMESSAGEBOXPLUGIN_P_H

#include <QObject>
#include <QVariantMap>

#include <hbdevicedialogplugin.h>

class HbDeviceMessageBoxPluginPrivate;

class HbDeviceMessageBoxPlugin : public HbDeviceDialogPlugin
{
    Q_OBJECT

public:
    friend class HbDeviceMessageBoxPluginPrivate;

    HbDeviceMessageBoxPlugin();
    ~HbDeviceMessageBoxPlugin();

    bool accessAllowed(const QString &deviceDialogType,
        const QVariantMap &parameters, const QVariantMap &securityInfo) const;
    HbDeviceDialogInterface *createDeviceDialog(const QString &deviceDialogType,
        const QVariantMap &parameters);
    bool deviceDialogInfo(const QString &deviceDialogType,
        const QVariantMap &parameters, DeviceDialogInfo *info) const;
    QStringList deviceDialogTypes() const;
    PluginFlags pluginFlags() const;
    int error() const;

private:
    Q_DISABLE_COPY(HbDeviceMessageBoxPlugin)
    HbDeviceMessageBoxPluginPrivate *d;
};

#endif // HBDEVICEMESSAGEBOXPLUGIN_P_H
