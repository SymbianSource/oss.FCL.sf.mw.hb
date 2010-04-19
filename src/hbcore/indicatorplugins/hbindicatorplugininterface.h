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

#ifndef HBINDICATORPLUGININTERFACE_H
#define HBINDICATORPLUGININTERFACE_H
#include <hbglobal.h>

#include <QStringList>
#include <QtPlugin>
#include <QVariantMap>
#include <QtGlobal>

class HbIndicatorInterface;
class HbSecurityInfo; // deprecated

class HB_CORE_EXPORT HbIndicatorPluginInterface
{
public:
    virtual QStringList indicatorTypes() const = 0;
    // Deprecated
    virtual bool accessAllowed(const QString &indicatorType,
        const HbSecurityInfo *securityInfo) const /*= 0; implementation provided temporarily for deprecation period*/
        {Q_UNUSED(indicatorType); Q_UNUSED(securityInfo); return true;}
    // End deprecated
    virtual bool accessAllowed(const QString &indicatorType,
        const QVariantMap &securityInfo) const /*= 0; will be pure virtual when deprecation period ends*/
        {Q_UNUSED(indicatorType); Q_UNUSED(securityInfo); return true;}
    virtual HbIndicatorInterface *createIndicator(const QString &indicatorType) = 0;
    virtual int error() const = 0;
    virtual ~HbIndicatorPluginInterface() {}
};

Q_DECLARE_INTERFACE(HbIndicatorPluginInterface, "com.nokia.hb.HbIndicatorPluginInterface/1.0")

#endif // HBINDICATORPLUGININTERFACE_H
