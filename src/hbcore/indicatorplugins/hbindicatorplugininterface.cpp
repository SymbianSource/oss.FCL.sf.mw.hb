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

#include <hbindicatorplugininterface.h>

/*!
    \class HbIndicatorPluginInterface
    \brief HbIndicatorPluginInterface is an abstract base class for indicator plugins.

    Indicators are displayed in the status indicator area and/or in universal indicator popup.
    Applications can create own indicators by implementing this plugin interface and creating
    at least one class inherited from HbIndicatorInterface.

    One plugin can implement one or more indicators. Indicators are identified by a type string.
    The indicator types the plugin implements must be returned from the function
    indicatorTypes(). The indicator is created, when requested by the client,
    using createIndicator().

    Indicator plugin may limit access to an indicator. Before activating or deactivating an
    indicator, the framework calls accessAllowed() allowing the plugin to inspect client's security
    credentials. By returning false, the plugin denies client access.

    here's an example of how to create a simple indicator plugin.
    If plugin implements only one indicator, the pluginInterface class can also inherit from
    HbIndicatorInterface. Example header-file:

    \snippet{unit\unittest_hbindicator\codesnippetplugin\hbcodesnippetplugin.h,1}

    Example source-file:
    \snippet{unit\unittest_hbindicator\codesnippetplugin\hbcodesnippetplugin.cpp,1}

    If more than one indicators are implemented inside a plugin, createIndicator should create new object
    based on HbIndicatorInterface.
    \snippet{unit\unittest_hbindicator\codesnippetplugin\hbcodesnippetplugin.cpp,2}

    \sa HbIndicator, HbIndicatorInterface

    \alpha
    \hbcore
*/

/*!
    \fn virtual QStringList HbIndicatorPluginInterface::indicatorTypes() const = 0

    Should return the indicator types this plugin implements.
*/

/*!
    \fn virtual bool HbIndicatorPluginInterface::accessAllowed(
        const QString &indicatorType, const QVariantMap &securityInfo) const

    <b>Notice: This function has implementation until deprecation period ends.
    After that it will be pure virtual.</b>

    Checks if client is allowed to activate or deactivate the indicator.  The
    implementation is operating system dependent. On Symbian this may involve checking client's
    platform security capabilities or secure ID for example.

    \a indicatorType contains indicator type.
    \a securityInfo contains information for security check. See HbDeviceDialogPlugin::accessAllowed() for format.

    Should return true if client is allowed to activate and deactivate the indicator.

    \sa HbDeviceDialogPlugin::accessAllowed()
*/

/*!
    \fn virtual bool HbIndicatorPluginInterface::accessAllowed(
        const QString &indicatorType, const HbSecurityInfo *securityInfo) const

    Checks if client is allowed to activate or deactivate the indicator.  The
    implementation is operating system dependent. On Symbian this may involve checking client's
    platform security capabilities or secure ID for example.

    \a indicatorType contains indicator type.
    \a securityInfo contains information for security check. Information is operating system dependent.

    Should return true if client is allowed to activate and deactivate the indicator.

    Note! Not implemented yet. Plugin implementors should always return true from this function.

    \deprecated
    Replaced by
    virtual bool HbIndicatorPluginInterface::accessAllowed(
        const QString &indicatorType, const QVariantMap &securityInfo) const
*/
/*
    \deprecated virtual bool HbIndicatorPluginInterface::accessAllowed(
        const QString &indicatorType, const HbSecurityInfo *securityInfo) const
    is deprecated. Replaced by
    virtual bool HbIndicatorPluginInterface::accessAllowed(
        const QString &indicatorType, const QVariantMap &securityInfo) const
*/

/*!
    \fn virtual  HbIndicatorInterface *HbIndicatorPluginInterface::createIndicator(const QString &indicatorType) = 0
    creates an indicator of type \a indicatorType. Ownership is passed to the caller.
    \sa HbIndicatorPluginInterface::indicatorTypes()
*/

/*!
    \fn virtual int HbIndicatorPluginInterface::error() const = 0
    Returns the last error code. The code is cleared when any other API function than error() is
    called.
*/

