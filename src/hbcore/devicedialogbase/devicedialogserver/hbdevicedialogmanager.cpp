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

#include "hbdevicedialogmanager_p.h"
#include "hbdevicedialogmanager_p_p.h"
#include <hbdevicedialogtrace_p.h>

/*!
    \internal
    HbDeviceDialogManager manages device dialogs. It is meant to be
    created and used exclusively by HbDeviceDialogServer.

    HbDeviceDialogManager has following responsibilities:
    \li Loading and managing device dialog plugins via plugin manager
    \li Forwarding device dialog information to corresponding plugins
    \li Controlling device dialog visibility
    \li Managing device dialog popups and their identifiers via popup handler
    \li Forwarding client updates to correct device dialogs.
    \li Sending device dialog updates to client via device dialog server.
*/

/*!
    \internal
    void devicedialogUpdate(int identifier, QVariantMap data)

    This signal is emitted when a device dialog widget sends update to client.
*/

/*!
    \internal
    void deviceDialogClosed(int identifier)

    This signal is emitted when a device dialog widget is closed.
*/

/*!
    \internal
    Constructs the device dialog manager.
*/
HbDeviceDialogManager::HbDeviceDialogManager(QObject *parent)
    : QObject(parent), d(new HbDeviceDialogManagerPrivate(this))
{
    TRACE_ENTRY
    // Nothing
    TRACE_EXIT
}

/*!
    \internal
    Deletes the device dialog manager.
*/
HbDeviceDialogManager::~HbDeviceDialogManager()
{
    TRACE_ENTRY
    // Private implementation
    delete d;
    TRACE_EXIT
}

// Set main window
void HbDeviceDialogManager::setMainWindow(HbMainWindow *mainWindow)
{
    d->setMainWindow(mainWindow);
}

/*!
    \internal
    Instructs HbDeviceDialogManager to show device dialog.

    Returns the identifier of the concrete device dialog widget.
*/
int HbDeviceDialogManager::showDeviceDialog(HbDeviceDialogServer::DialogParameters &parameters)
{
    return d->showDeviceDialog(parameters);
}

/*!
     \internal
    Instructs HbDeviceDialogManager to update a concrete device dialog widget. Widget
    is identified by identifier and the updated parameters (the data) are wrapped
    inside parameters.

    Returns error code if updating fails. 0 if updating succeeds.
*/
int HbDeviceDialogManager::updateDeviceDialog(
    int identifier, const QVariantMap &parameters)
{
    return d->updateDeviceDialog(identifier, parameters);
}

/*!
    \internal
    Publish current orientation to PS-key
    Returns error code if updating fails. 0 if updating succeeds.
*/
int HbDeviceDialogManager::publishOrientation(int orientation)
{
    return d->publishOrientation(orientation);
}

/*!
    \internal
    Instructs HbDeviceDialogManager to close a concrete device dialog widget.
    Widget is identified by identifier.

    Returns error code if closing fails. 0 if closing succeeds.
*/
int HbDeviceDialogManager::closeDeviceDialog(int identifier)
{
    return d->closeDeviceDialog(identifier);
}

// Inform manager that client (session) is closing
void HbDeviceDialogManager::deviceDialogClientClosing(quintptr clientTag)
{
    return d->deviceDialogClientClosing(clientTag);
}

/*!
    \internal
    Activate indicator
*/
int HbDeviceDialogManager::activateIndicator(HbDeviceDialogServer::IndicatorParameters &parameters)
{
    return d->activateIndicator(parameters);
}

/*!
    \internal
    Deactivate indicator
*/
int HbDeviceDialogManager::deactivateIndicator(
    HbDeviceDialogServer::IndicatorParameters &parameters)
{
    return d->deactivateIndicator(parameters);
}

QList<IndicatorClientInfo> HbDeviceDialogManager::indicatorClientInfoList() const
{
    return d->indicatorClientInfoList();
}

