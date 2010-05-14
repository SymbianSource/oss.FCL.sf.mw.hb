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

#include <QCoreApplication>
#include <hbdevicedialogtrace_p.h>
#include <hbdevicedialogerrors_p.h>
#include "hbindicatorsessionhandler_p.h"
#include "hbdevicedialogserver_p.h"
#include "hbdevicedialogserversym_p_p.h"
#include "hbdevicedialogsession_p.h"
#include "hbdevicedialogserverdefs_p.h"
#ifdef HB_HAVE_QT_MOBILITY
#include "hbsysteminfo_p.h"
#endif // HB_HAVE_QT_MOBILITY

/*!
    \class HbDeviceDialogServerPrivate
    \internal
*/

/*!
    \internal
 */
HbDeviceDialogServerPrivate::HbDeviceDialogServerPrivate()
    : CPolicyServer( CActive::EPriorityHigh, KHbDeviceDialogServerPolicy, EUnsharableSessions )
{
    TRACE_ENTRY
    mSessionCount = 0;
    // No need for timer until server automatic close will be again enabled
    // mExitTimer = CPeriodic::New(CActive::EPriorityLow);
#ifdef HB_HAVE_QT_MOBILITY
    mDeviceInfo = new HbSystemInfo(0, true);
#endif // HB_HAVE_QT_MOBILITY   
    TRACE_EXIT
}

/*!
    \internal
 */
HbDeviceDialogServerPrivate::~HbDeviceDialogServerPrivate()
{
#ifdef HB_HAVE_QT_MOBILITY
    delete mDeviceInfo;
#endif // HB_HAVE_QT_MOBILITY     
    
    // delete mExitTimer;
}

/*!
    \internal
    Initialisation step. Starts a server with given name.
*/
int HbDeviceDialogServerPrivate::Init()
{
    TRACE_ENTRY
    int error = Start(KHbServerName);
    TRACE_EXIT
    return error;
}

/*!
    \internal
    Increment client session count. Called by the session instance when it is
    instantiated.
*/
void HbDeviceDialogServerPrivate::AddSession()
{
    TRACE_ENTRY
    mSessionCount++;
    TRACE_EXIT
}

/*!
    \internal
    Decrement client session count. Called be the session instance when it is
    destroyed.
*/
void HbDeviceDialogServerPrivate::RemoveSession()
{
    TRACE_ENTRY
    mSessionCount--;
    //if (mSessionCount <= 0) {
        // Exit application when last session closes
        //const int QuitDelay = 30000000; // 30s
        //mExitTimer->Cancel();
        //TCallBack callback(exitTimerCallback, this);
        //mExitTimer->Start(QuitDelay, QuitDelay, callback);
    //}
    TRACE_EXIT
}

/*!
    \internal
    Add client session to session instance array. Array is used to identify correct client
    when update and close events are sent.
*/
TInt HbDeviceDialogServerPrivate::AddSessionToList( const HbDeviceDialogSession *aSession )
{
    TRACE_ENTRY
    TInt result( KErrNone );
    TInt index = iSessionList.Find( aSession );

    if ( index == KErrNotFound ) {
        result = iSessionList.Append( aSession );
    }
    if (result != KErrNone) {
        result = HbDeviceDialogGeneralError;
    }
    TRACE_EXIT_ARGS("result" << result)
    return result;
}

/*!
    \internal
    Remove client session from session instance array. Client has disconnected.
*/
void HbDeviceDialogServerPrivate::RemoveSessionFromList( const HbDeviceDialogSession *aSession )
{
    TRACE_ENTRY
    TInt index = iSessionList.Find( aSession );

    if ( index > KErrNotFound ) {
        // Session found. Remove.
        iSessionList.Remove( index );
    }
    TRACE_EXIT
}

/*!
    \internal
    Find correct session to send device dialog initiated update event to client.
*/
void HbDeviceDialogServerPrivate::deviceDialogUpdate( int id, const QVariantMap &data ) const
{
    TRACE_ENTRY
    for (TInt i(0); i < iSessionList.Count(); i++ ) {
        if ( iSessionList[i]->DeviceDialogIdentifier() == id ) {
            iSessionList[i]->DeviceDialogUpdate( data );
        }
    }
    TRACE_EXIT
}

/*!
    \internal
    Find correct session to send device dialog initiated close event to client.
*/
void HbDeviceDialogServerPrivate::deviceDialogClosed( int identifier, int closeReason )
{
    TRACE_ENTRY
    for (TInt i(0); i < iSessionList.Count(); i++ ) {
        if ( iSessionList[i]->DeviceDialogIdentifier() == identifier ) {
            iSessionList[i]->DeviceDialogClosed( identifier, closeReason );
            RemoveSessionFromList( iSessionList[i] );
        }
    }
    TRACE_EXIT
}

/*!
    \internal
    Forward client initiated show event.
*/
int HbDeviceDialogServerPrivate::showDeviceDialog(
    HbDeviceDialogServer::DialogParameters &parameters)
{
    return q_func()->showDeviceDialog(parameters);
}

/*!
    \internal
    Forward client initiated update event.
*/
int HbDeviceDialogServerPrivate::updateDeviceDialog( int id, QVariantMap &data )
{
    return q_func()->updateDeviceDialog( id, data );
}

/*!
    \internal
    Forward client initiated close event.
*/
int HbDeviceDialogServerPrivate::closeDeviceDialog( int id )
{
    return q_func()->closeDeviceDialog(id);
}

// Inform server that client (session) is closing
void HbDeviceDialogServerPrivate::deviceDialogClientClosing(quintptr sessionTag)
{
    return q_func()->deviceDialogClientClosing(sessionTag);
}

int HbDeviceDialogServerPrivate::activateIndicator(
    HbDeviceDialogServer::IndicatorParameters &parameters)
{
    return q_func()->activateIndicator(parameters);
}

int HbDeviceDialogServerPrivate::deactivateIndicator(
    HbDeviceDialogServer::IndicatorParameters &parameters)
{
    return q_func()->deactivateIndicator(parameters);
}

QList<IndicatorClientInfo> HbDeviceDialogServerPrivate::indicatorClientInfoList() const
{
    return q_func()->indicatorClientInfoList();
}

TInt HbDeviceDialogServerPrivate::AddIndicatorHandlerToList(
        const HbIndicatorSessionHandler *aSession)
{
    TRACE_ENTRY
    TInt result( KErrNone );
    TInt index = iIndicatorSessionList.Find( aSession );

    if ( index == KErrNotFound ) {
        result = iIndicatorSessionList.Append( aSession );
    }
    if (result != KErrNone) {
        result = HbDeviceDialogGeneralError;
    }
    TRACE_EXIT_ARGS("result" << result)
    return result;
}

void HbDeviceDialogServerPrivate::RemoveIndicatorHandlerFromList(
        const HbIndicatorSessionHandler *aSession)
{
    TRACE_ENTRY
    TInt index = iIndicatorSessionList.Find( aSession );

    if ( index > KErrNotFound ) {
        // Session found. Remove.
        iIndicatorSessionList.Remove( index );
    }
    TRACE_EXIT
}

void HbDeviceDialogServerPrivate::IndicatorsActivated(
        const QList<IndicatorClientInfo> &indicatorInfoList)
{
    TRACE_ENTRY
    for (TInt i(0); i < iIndicatorSessionList.Count(); ++i ) {
        iIndicatorSessionList[i]->IndicatorsActivated( indicatorInfoList );
    }
    TRACE_EXIT
}

void HbDeviceDialogServerPrivate::IndicatorsUpdated(
        const QList<IndicatorClientInfo> &indicatorInfoList)
{
    TRACE_ENTRY
    for (TInt i(0); i < iIndicatorSessionList.Count(); ++i ) {
        iIndicatorSessionList[i]->IndicatorsUpdated( indicatorInfoList );
    }
    TRACE_EXIT
}

void HbDeviceDialogServerPrivate::IndicatorsDeactivated(
        const QList<IndicatorClientInfo> &indicatorInfoList)
{
    TRACE_ENTRY
    for (TInt i(0); i < iIndicatorSessionList.Count(); ++i ) {
        iIndicatorSessionList[i]->IndicatorsDeactivated( indicatorInfoList );
    }
    TRACE_EXIT
}

void HbDeviceDialogServerPrivate::IndicatorUserActivated(const QVariantMap &data) const
{    
    TRACE_ENTRY
    for (TInt i(0); i < iIndicatorSessionList.Count(); ++i) {
        iIndicatorSessionList[i]->IndicatorUserActivated(data);
    }
    TRACE_EXIT
}

/*!
    \internal
    Check if application should exit.
*/
TInt HbDeviceDialogServerPrivate::exitTimerCallback(TAny *aServer)
{
    // Exit if there are 0 sessions and no client is initiating a connect
    HbDeviceDialogServerPrivate *server = static_cast<HbDeviceDialogServerPrivate*>(aServer);
    if (server->mSessionCount > 0) {
        // Sessions exist, no exit
        // No timer in use currently as server never closes
        // server->mExitTimer->Cancel();
    } else {
        // Check if client is in the middle of connect
        _LIT(KFindPattern, "hbdevdlgcli_?*");
        TFindSemaphore find(KFindPattern);
        TFullName findResult;
        if (find.Next(findResult) != KErrNone) {
            QCoreApplication::instance()->quit();
        }
    }
    return EFalse; // return value doesn't matter
}

/*!
    \internal
    Create new client session.
*/
CSession2 *HbDeviceDialogServerPrivate::NewSessionL( const TVersion &/*aVersion*/,
                                    const RMessage2 &/*aMessage*/) const
{
    return HbDeviceDialogSession::NewL();
}
