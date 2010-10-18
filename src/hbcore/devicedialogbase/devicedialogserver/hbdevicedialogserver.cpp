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

#include "hbdevicedialogserver_p.h"
#if defined(Q_OS_SYMBIAN)
#include "hbdevicedialogserversym_p_p.h"
#else
#include "hbdevicedialogserverwin_p_p.h"
#endif
#include "hbdevicedialogmanager_p.h"
#include <hbdevicedialogtrace_p.h>
/*!
    \internal

    HbDeviceDialogServer is a class for managing communication between device dialogs
    and client application. Main responsibility is forwarding commands such as show(),
    update() and close() to HbDeviceDialogManager from HbDeviceDialog client and to
    send feedback from a dialog to HbDeviceDialog client. Mapping of a client session
    and a dialog is based on the dialog identifier received from HbDeviceDialogManager,
    when a dialog is successfully instantiated.

*/

/*!
    \internal
    Constructor for HbDeviceDialogServer class.
*/
HbDeviceDialogServer::HbDeviceDialogServer() : iManager(0), d_ptr(0), mError(0)
{
    TRACE_ENTRY
    d_ptr = new HbDeviceDialogServerPrivate;
    Q_D( HbDeviceDialogServer );
    d->q_ptr = this;
    mError = d->Init();
    // Error creating a server
    if (mError != 0) {
        return;
    }

    iManager = new HbDeviceDialogManager;

    connect( iManager, SIGNAL(deviceDialogUpdate(int, QVariantMap)),
             this, SLOT(deviceDialogUpdate(int, QVariantMap)));
    connect( iManager, SIGNAL(deviceDialogClosed(int, int)),
            this, SLOT(deviceDialogClosed(int, int)));

    connect( iManager, SIGNAL( indicatorActivated(QList<IndicatorClientInfo>) ),
             this, SLOT( indicatorActivated(QList<IndicatorClientInfo>) ) );
    connect( iManager, SIGNAL( indicatorUpdated(QList<IndicatorClientInfo>) ),
             this, SLOT( indicatorUpdated(QList<IndicatorClientInfo>) ) );
    connect( iManager, SIGNAL( indicatorRemoved(QList<IndicatorClientInfo>) ),
             this, SLOT( indicatorRemoved(QList<IndicatorClientInfo>) ) );
    connect( iManager, SIGNAL( indicatorUserActivated(const QVariantMap) ),
             this, SLOT( indicatorUserActivated(const QVariantMap) ) );
    TRACE_EXIT
}

/*!
    \internal
    Destructor.
*/
HbDeviceDialogServer::~HbDeviceDialogServer()
{
    TRACE_ENTRY
    delete d_ptr;
    delete iManager;
    TRACE_EXIT
}

// Set main window
void HbDeviceDialogServer::setMainWindow(HbMainWindow *mainWindow)
{
    iManager->setMainWindow(mainWindow);
}

/*!
    Calls HbDeviceDialogManager to show device dialog.
    On return the dialog identifier is received.
*/
int HbDeviceDialogServer::showDeviceDialog(DialogParameters &parameters)
{
    return iManager->showDeviceDialog(parameters);
}

/*!
    \internal

    Calls HbDeviceDialogManager to update device dialog. On return the status of the call is
    received.

    identifier identifies the device dialog.
    data contains update data for the dialog identified by id.
*/
int HbDeviceDialogServer::updateDeviceDialog(int identifier, const QVariantMap &data)
{
    return iManager->updateDeviceDialog( identifier, data );
}

/*!
    \internal

    Publish current orientation to PS-key
*/
int HbDeviceDialogServer::publishOrientation(int orientation)
{
    return iManager->publishOrientation( orientation );
}

/*!
    \internal

    Calls HbDeviceDialogManager to close device dialog. On return the status of the call is received.

    identifier identifies the device dialog.
*/
int HbDeviceDialogServer::closeDeviceDialog(int identifier)
{
    return iManager->closeDeviceDialog( identifier );
}

// Inform that client (session) is closing
void HbDeviceDialogServer::deviceDialogClientClosing(quintptr clientTag)
{
    return iManager->deviceDialogClientClosing(clientTag);
}

/*!
    \internal

    Calls device dialog manager to activate indicator.
    On return the status of the call is received.
*/
int HbDeviceDialogServer::activateIndicator(IndicatorParameters &parameters)
{
    return iManager->activateIndicator(parameters);
}

/*!
    \internal

    Calls device dialog manager to deactivate indicator.
    On return the status of the call is received.
*/
int HbDeviceDialogServer::deactivateIndicator(IndicatorParameters &parameters)
{
    return iManager->deactivateIndicator(parameters);
}

QList<IndicatorClientInfo> HbDeviceDialogServer::indicatorClientInfoList() const
{
    return iManager->indicatorClientInfoList();
}

/*!
    \internal

    Handles device dialog initiated update call. Data is forwarded to HbDeviceDialog.

    identifier identifies the dialog. Identifier is used to find the correct client instance the
          update data is sent.
    data contains data that is sent to client.
*/
void HbDeviceDialogServer::deviceDialogUpdate( int identifier, const QVariantMap &data ) const
{
    d_func()->deviceDialogUpdate( identifier, data );
}

/*!
    \internal

    Handles device dialog initiated dialog closed call. Client is notified of the close event.

    identifier identifies the dialog. Identifier is used to find the correct client instance.
*/
void HbDeviceDialogServer::deviceDialogClosed( int identifier, int closeReason )
{
    d_func()->deviceDialogClosed( identifier, closeReason );
}

void HbDeviceDialogServer::indicatorActivated(
        const QList<IndicatorClientInfo> &indicatorInfoList)
{
    d_func()->IndicatorsActivated(indicatorInfoList);
}

void HbDeviceDialogServer::indicatorUpdated(
        const QList<IndicatorClientInfo> &indicatorInfoList) {
    d_func()->IndicatorsUpdated(indicatorInfoList);
}

void HbDeviceDialogServer::indicatorRemoved(
        const QList<IndicatorClientInfo> &indicatorInfoList)
{
    d_func()->IndicatorsDeactivated(indicatorInfoList);
}

void HbDeviceDialogServer::indicatorUserActivated(const QVariantMap &data)
{
    d_func()->IndicatorUserActivated(data);
}
