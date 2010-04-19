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

_LIT(KSemaName, "hbdevdlgcli_");

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
    TInt error = KErrNone;
    if (ServerRunning()) {
        if (!Handle()) {
            TUid serviceUid = { 0x10009822 };
            error = CreateSession(KHbServerName, *reinterpret_cast<TVersion*>(&serviceUid), 
                                  -1, EIpcSession_Unsharable, 0, aStatus);
        }
    } else {
        error = Connect();
        User::RequestComplete(aStatus, error);
    }
    TRACE_EXIT
    return error;
}

TBool RHbDeviceDialogClientSession::ServerRunning() const
{
    TRACE_ENTRY
    TFindServer findHbServer(KHbServerName);
    TFullName name;
    TRACE_EXIT
    return findHbServer.Next(name) == KErrNone;
}

/*!
    \internal
*/
TInt RHbDeviceDialogClientSession::Connect()
{
    TRACE_ENTRY

    TInt error = KErrNone;

    // Create semaphore. Sserver application signals it after server object is created. Also
    // existence of the semaphore tells server app that connection is in the progress and it
    // should not start exiting.
    RSemaphore serverStartedSema;
    TBuf<sizeof(KSemaName) + 16> semaName(KSemaName); // name + thread id
    semaName.AppendNum(RThread().Id().Id(), EHex);
    error = serverStartedSema.CreateGlobal(semaName, 0, EOwnerProcess);
    if (error != KErrNone) {
        return error;
    }

    TInt retry(3);
    if (!Handle()) {
        TUid serviceUid = { 0x10009822 };

        forever {
            error = CreateSession(KHbServerName, *reinterpret_cast<TVersion*>(&serviceUid));

            if (error != KErrNotFound && error != KErrServerTerminated) {
                // KErrNone, KErrPermissionDenied or other serious error.
                break;
            }

            if (--retry == 0) {
                // We have tried enough, return an error.
                break;
            }

            error = StartServer(serverStartedSema);

            if (error != KErrNone && error != KErrAlreadyExists) {
                // Unrecoverable error, return an error.
                break;
            }
        } // for-loop end
    }
    serverStartedSema.Close();
    TRACE_EXIT
    return error;
}

/*!
    \internal
*/
TInt RHbDeviceDialogClientSession::StartServer(RSemaphore &aServerStartedSema)
{
    TRACE_ENTRY
    TInt error(KErrNone);

    TFindServer findHbServer(KHbServerName);
    TFullName name;

    error = findHbServer.Next(name);

    if (error == KErrNone) {
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

    // QApplication calls Rendezvous() before device dialog server has created server object.
    // Therefore we wait here for server app to create the server object.
    if (error == KErrNone || error == KErrAlreadyExists) {
        forever {
            findHbServer.Find(KHbServerName);
            error = findHbServer.Next(name);
            if (error != KErrNotFound) {
                break;
            }
            if (process.ExitType() != EExitPending) {
                error = KErrGeneral;
                break;
            }
            const TInt KTimeout = 1000000; // 1 s
            aServerStartedSema.Wait(KTimeout);
        }
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
