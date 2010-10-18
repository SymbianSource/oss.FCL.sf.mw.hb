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
    \brief HbIndicatorPluginInterface is an abstract interface class for indicator plugins.

    Indicators are activated, deactivated and updated using HbIndicator class. Indicator
    framework takes care of displaying indicators in satus bar and/or indicator menu.

    Indicator plugins contain implentation of indicators. Single pluging can implement
    one or several indicators. HbIndicatorInterface defines functionality required from an
    indicator implementation.

    Indicators are identified by a string. By convention the string should follow
    inverted domain name format. For example com.nokia.hb.indicator.xxx/1.0.
    Function indicatorTypes() returns a list of indicators the plugin implements.

    Appending version number into indicator type string enables versioning.
    A plugin should handle versioning by returning indicator type string for all versions it
    implements in indicatorTypes() and then in createIndicator() create an indicator
    instance compatible with the version requested. This could always be the latest version if it
    is backwards compatible with older versions. Indicator framework is unaware of version
    numbers in type strings. It performs string comparison of the whole string when searching
    for a plugin.

    Plugins are responsible for maintaining system security for their own part. If plugin
    performs operations that may compromise security or want's to limit access to specific
    clients, it should check client security credentials in accessAllowed() function.
    Indicator framework calls this function before activating/deactivating indicators.

    \section _exceptions Exception handling

    Indicator service uses two strategies in exception handling: Avoidance and trapping.
    Memory allocation exceptions while an indicator is running are avoided by ensuring there is
    sufficient heap space available before allowing new indicators to be activated.
    Trapping is used while an indicator is created. A call to createIndicator() is enclosed
    in try/catch block. Memory allocation exception causes indicator activation to fail
    and an error is returned to a client. Plugin should take care there are no memory leaks
    if exception is thrown inside createIndicator(). Calls to HbIndicatorInterface
    functions are trapped and thrown allocation exceptions are ignored. Plugins can
    provide more fine grained exception handling by trapping exceptions themselves.

    \section _platform_hbindicatorplugin Platform-specific implementation notes for HbIndicatorPluginInterface

    \subsection _nonsymbian Non-Symbian

    Indicator plugins are loaded into client process. Plugin executables are searched from
    application's current directory and HB_PLUGINS_DIR/indicators directory.

    \subsection _symbian Symbian

    Plugins are run by a server with platform security capabilities ProtServ, SwEvent,
    TrustedUI and ReadDeviceData. If a plugin doesn't have required platform security
    capabilities it will not load.

    Device dialog plugin stubs (.qtplugin) are searched from /resource/plugins/indicators directory
    and executables in /sys/bin directory in each drive.

    \section _example_code Example code

    Below is an example of how to create a simple indicator plugin.

    If plugin implements only one indicator, plugin can also inherit from
    HbIndicatorInterface. Example header-file:

    \snippet{tsrc\unit\unittest_hbindicator\codesnippetplugin\hbcodesnippetplugin.h,1}

    Example source-file:
    \snippet{tsrc\unit\unittest_hbindicator\codesnippetplugin\hbcodesnippetplugin.cpp,1}

    If more than one indicators are implemented inside a plugin, plugin inherits from HbIndicatorPluginInterface.

    \snippet{tsrc\unit\unittest_hbindicator\codesnippetplugin\hbcodesnippetplugin.h,2}

    And createIndicator should create new object based on indicator type.

    \snippet{tsrc\unit\unittest_hbindicator\codesnippetplugin\hbcodesnippetplugin.cpp,2}

    \sa HbIndicator, HbIndicatorInterface

    \stable
    \hbcore
*/

/*!
    \fn virtual QStringList HbIndicatorPluginInterface::indicatorTypes() const = 0

    Should return the indicator types the plugin implements.
*/

/*!
    \fn virtual bool HbIndicatorPluginInterface::accessAllowed(
        const QString &indicatorType, const QVariantMap &securityInfo) const

    Checks if client is allowed to activate or deactivate the indicator.  The
    implementation is operating system dependent. On Symbian this may involve checking client's
    platform security capabilities or secure ID for example.

    \a indicatorType contains indicator type.
    \a securityInfo contains information for security check. See HbDeviceDialogPlugin::accessAllowed() for format.

    Should return true if client is allowed to activate and deactivate the indicator.

    \sa HbDeviceDialogPlugin::accessAllowed()
*/

/*!
    \fn virtual  HbIndicatorInterface *HbIndicatorPluginInterface::createIndicator(const QString &indicatorType) = 0

    Creates an indicator of type \a indicatorType. Ownership is passed to the caller.

    \sa HbIndicatorPluginInterface::indicatorTypes()
*/

/*!
    \fn virtual int HbIndicatorPluginInterface::error() const = 0

    Returns the last error code. The code is cleared when any other API function than error() is
    called.
*/

