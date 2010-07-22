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

#include <hbdevicedialogtrace_p.h>
#include <hbdevicedialogserverdefs_p.h>
#include <apacmdln.h>

#include "hbdevicedialogclientsession_p.h"

// Check if the client is running in device dialog server
static bool IsDeviceDialogServer()
{
    const TUid ServerUid = {0x20022FC5};
    return RProcess().SecureId().operator TUid() == ServerUid;
}

// Preallocate 1 message slot for sessions
static const int KNumAsyncMessageSlots = 1;

/*!
    \internal
*/
RHbDeviceDialogClientSession::RHbDeviceDialogClientSession()
{
    TRACE_ENTRY
    TRACE_EXIT
}

/*!
    \internal
*/
RHbDeviceDialogClientSession::~RHbDeviceDialogClientSession()
{
    TRACE_ENTRY
    TRACE_EXIT
}

TInt RHbDeviceDialogClientSession::Connect(TRequestStatus *aStatus)
{
    TRACE_ENTRY
    // Check server is not trying to connect to itself
    if (IsDeviceDialogServer()) {
      return KErrNotSupported;
    }

    TInt error = KErrNone;
    if (!Handle()) {
        TVersion serverVersion(KHbServerMajor, KHbServerMinor, KHbServerBuild);
        error = CreateSession(KHbServerName, serverVersion, KNumAsyncMessageSlots,
            EIpcSession_Unsharable, 0, aStatus);
    }
    TRACE_EXIT
    return error;
}

/*!
    \internal
*/
TInt RHbDeviceDialogClientSession::Connect()
{
    TRACE_ENTRY

    // Check server is not trying to connect to itself
    if (IsDeviceDialogServer()) {
      return KErrNotSupported;
    }

    TInt error = KErrNone;

    TInt retry(3);
    if (!Handle()) {
        forever {
            TVersion serverVersion(KHbServerMajor, KHbServerMinor, KHbServerBuild);
            error = CreateSession(KHbServerName, serverVersion, KNumAsyncMessageSlots);

            if (error != KErrNotFound && error != KErrServerTerminated) {
                // KErrNone, KErrPermissionDenied or other serious error.
                break;
            }

            if (--retry == 0) {
                // We have tried enough, return an error.
                break;
            }

            error = StartServer();
            if (error != KErrNone && error != KErrAlreadyExists) {
                // Unrecoverable error, return an error.
                break;
            }
        } // for-loop end
    }
    TRACE_EXIT
    return error;
}

TBool RHbDeviceDialogClientSession::ServerRunning()
{
    TRACE_STATIC_ENTRY
    TFindServer findHbServer(KHbServerName);
    TFullName name;
    TRACE_EXIT
    return findHbServer.Next(name) == KErrNone;
}

/*!
    \internal
*/
TInt RHbDeviceDialogClientSession::StartServer()
{
    TRACE_ENTRY
    TInt error(KErrNone);

    if (ServerRunning()) {
        // Already running, return error.
        return KErrAlreadyExists;
    }

    RProcess process;

    // Create the server process
    error = process.Create(KHbServerExe, KNullDesC);

    if (error != KErrNone) {
        // Process could not be created, return error.
        return error;
    }

    CApaCommandLine* apaCommandLine = 0;
    TRAP(error, apaCommandLine = CApaCommandLine::NewL());
    if (error != KErrNone) {
        process.Close();
        return error;
    }
    TRAP(error,
        apaCommandLine->SetExecutableNameL(KHbServerExe);
        apaCommandLine->SetCommandL(EApaCommandBackground);
        apaCommandLine->SetProcessEnvironmentL(process);
    );
    delete apaCommandLine;
    apaCommandLine = 0;

    if (error != KErrNone) {
        process.Close();
        return error;
    }

    TRequestStatus status;
    process.Rendezvous(status);

    if (status != KRequestPending) {
        process.Close();
        return KErrGeneral;
    }

    process.Resume();    // logon OK - start the server

    // Wait for start or death
    User::WaitForRequest(status);

    // We can't use the 'exit reason' if the server panicked as this
    // is the panic 'reason' and may be '0' which cannot be distinguished
    // from KErrNone
    if (process.ExitType() == EExitPanic) {
        error = KErrGeneral;
    } else {
        error = status.Int();
    }

    process.Close();
    TRACE_EXIT
    return error;
}

/*!
    \internal
*/
void RHbDeviceDialogClientSession::Close()
{
    TRACE_ENTRY
    RSessionBase::Close();
    TRACE_EXIT
}

/*!
    \internal
*/
int RHbDeviceDialogClientSession::SendSyncRequest(int aCommand, int aInt0)
{
    TRACE_ENTRY
    TIpcArgs args(aInt0);
    TInt err(KErrBadHandle);

    if (Handle()){
        err = SendReceive( aCommand, args );
    }
    TRACE_EXIT
    return err;
}

/*!
    \internal
*/
int RHbDeviceDialogClientSession::SendSyncRequest(int aCommand, const TDesC8& aData, TDes8 *aReceiveData)
{
    TRACE_ENTRY
    TIpcArgs args( &aData );
    args.Set(KSlot1, aReceiveData);
    TInt err (KErrBadHandle);

    if (Handle()) {
        err = SendReceive(aCommand, args);
    }
    TRACE_EXIT
    return err;
}

/*!
    \internal
*/
int RHbDeviceDialogClientSession::SendSyncRequest(int aCommand, TDes8& aData, TDes8 *aReceiveData)
{
    TRACE_ENTRY
    TIpcArgs args( &aData );
    args.Set(KSlot1, aReceiveData);
    TInt err ( KErrBadHandle );

    if (Handle()) {
        err = SendReceive(aCommand, args);
    }
    TRACE_EXIT
    return err;
}

/*!
    \internal
*/
int RHbDeviceDialogClientSession::SendASyncRequest(int aCommand, TDes8& aData, TDes8& aType, TRequestStatus& aStatus)
{
    TRACE_ENTRY
    TIpcArgs args(&aData, &aType);
    TInt err(KErrBadHandle);

    if (Handle()) {
        err = KErrNone;
        SendReceive(aCommand, args, aStatus);
    }
    TRACE_EXIT
    return err;
}
