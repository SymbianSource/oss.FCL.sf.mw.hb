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

#include "hbdevicedialogsym_p.h"
#include "hbdevicedialogerrors_p.h"
#include <QVariant>
#include <QPointer>
#include <e32base.h>
#include <hbdevicedialogserverdefs_p.h>
#include <hbdevicedialogtrace_p.h>

/*!
    \class HbDeviceDialogPrivate
    \internal
*/

/*!
    \internal
*/
HbDeviceDialogPrivate::HbDeviceDialogPrivate()
: CActive(EPriorityStandard),
    iDeviceDialogId(0),
    iBuffer(NULL),
    iDataPtr(NULL, 0, 0),
    mEventLoop(0)
{
    TRACE_ENTRY
    TRACE_EXIT
}

/*!
    \internal
*/
HbDeviceDialogPrivate::~HbDeviceDialogPrivate()
{
    TRACE_ENTRY
    // If there are no signals connected or waitForClosed() is not executing, inform server
    // the client is closing. The server will keep device dialogs. Otherwise server closes
    // device dialogs when client session closes.
    if (receiverCount() == 0 && mEventLoop == 0) {
        mHbSession.SendSyncRequest(EHbSrvClientClosing);
    }
    Cancel();
    delete iBuffer;
    mHbSession.Close();
    if (mEventLoop && mEventLoop->isRunning()) {
        mEventLoop->exit(HbDeviceDialog::CancelledError);
    }
    TRACE_EXIT
}

/*!
    \internal
    Initialisation step.
*/
void HbDeviceDialogPrivate::init(HbDeviceDialog::DeviceDialogFlags f)
{
    TRACE_ENTRY
    mDeviceDialogFlags = f;
    mLastError = HbDeviceDialogNoError;
    iRequesting = EFalse;
    if (mDeviceDialogFlags & HbDeviceDialog::ImmediateResourceReservationFlag) {
        int error = symToDeviceDialogError(initialize());
        if (error != HbDeviceDialogNoError){
            setError(error);
        }
    }
    TRACE_EXIT
}

/*!
    \internal
    Initialisation step. Session to server is created.
*/
int HbDeviceDialogPrivate::initialize(bool connect)
{
    TRACE_ENTRY

    if (!IsAdded()) {
        CActiveScheduler::Add(this);
    }
    if (!iBuffer) {
        TRAP_IGNORE(iBuffer = HBufC8::NewL(64));
        if (iBuffer) {
            iDataPtr.Set(iBuffer->Des());
        } else {
          TRACE_EXIT_ARGS("error " << KErrNoMemory)
            return KErrNoMemory;
        }
    }

    int error = KErrNone;
    if (connect) {
        mHbSession.Connect();
    }
        
    TRACE_EXIT_ARGS("error" << error)
    return error;
}

/*!
    \internal

    Connects to server asynchronously.
*/

int HbDeviceDialogPrivate::connect()
{
    initialize(false);

    int error(HbDeviceDialogNoError);
    
    if (!IsActive() && !iConnecting) {
        bool connected = mHbSession.Handle() != 0;
        error = mHbSession.Connect(&iStatus);    

        // If not connected yet, connect and wait for eventloop to stop. Eventloop is
        // stopped in the RunL funtion.
        if (error == KErrNone && !connected && !mEventLoop) {
            iConnecting = ETrue;
            SetActive();
            QPointer<HbDeviceDialog> guard = q_ptr;
            QEventLoop eventLoop;
            mEventLoop = &eventLoop;
            error = eventLoop.exec(QEventLoop::AllEvents);
            error = symToDeviceDialogError(error);
            if (guard.isNull()) {
                return false;
            }
            mEventLoop = 0;
        }
    }
    TRACE_EXIT_ARGS("error" << error)
    return error;
}

/*!
    \internal

    Device dialog plugin identifier and data send to device dialog server. On
    success received device dialog id is stored.
*/
bool HbDeviceDialogPrivate::show(
    const QString &deviceDialogType,
    const QVariantMap &parameters)
{
    TRACE_ENTRY
    mLastError = HbDeviceDialogNoError;
    mDataReceived.clear();

    int error = symToDeviceDialogError(initialize());
    if (error != HbDeviceDialogNoError){
        setError(error);
        TRACE_EXIT_ARGS("error " << error)
        return false;
    }

    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);

    QVariant var(parameters);
    stream << deviceDialogType;
    stream << var;

    TPtrC8 ptr(reinterpret_cast<TUint8*>(array.data()), array.size());

    // Synchronous call to server to show dialog.
    error = mHbSession.SendSyncRequest(EHbSrvShowDeviceDialog, ptr, &iDeviceDialogId);
    error = symToDeviceDialogError(error);
    bool result(true);

    if (error != HbDeviceDialogNoError) {
        setError(error);
        result = false;
    } else {
        // Start listening for server updates. Device dialog update and closing is
        // received via this channel. Error status received in RunL method.
        Start();
    }
    TRACE_EXIT_ARGS("result" << result)
    return result;
}

/*!
    \internal

    Send device dialog update.
*/
bool HbDeviceDialogPrivate::update(const QVariantMap& parameters)
{
    TRACE_ENTRY
    mLastError = HbDeviceDialogNoError;
    bool result(true);
    if (iRequesting) {

        QByteArray array;
        QDataStream stream(&array, QIODevice::WriteOnly);

        QVariant var(parameters);
        stream << var;

        TPtrC8 ptr(reinterpret_cast<TUint8*>(array.data()), array.size());

        int error = mHbSession.SendSyncRequest(EHbSrvUpdateDeviceDialog, ptr);
        error = symToDeviceDialogError(error);
        if (error != HbDeviceDialogNoError) {
            setError(error);
            result = false;
        }
    } else {
        setError(HbDeviceDialogIllegalOperationError);
        result = false;

    }
    TRACE_EXIT_ARGS("result" << result)
    return result;
}

/*!
    \internal

    Cancel a scheduled popup on HbDeviceDialogManager. Event buffer is cleared
    at server.
*/
bool HbDeviceDialogPrivate::cancel()
{
    TRACE_ENTRY
    mLastError = HbDeviceDialogNoError;
    int error = HbDeviceDialogNotFoundError;

    if (iRequesting) {
        // Ignore other than server errors.
        error = mHbSession.SendSyncRequest(EHbSrvCancelDeviceDialog, iDeviceDialogId());
        error = symToDeviceDialogError(error);
    }
    if (error != HbDeviceDialogNoError) {
        setError(error);
    }
    bool result = (error == HbDeviceDialogNoError);
    TRACE_EXIT_ARGS("result" << result)
    return result;
}

/*!
    \internal

    Start wait for the close event sent by server.
*/
bool HbDeviceDialogPrivate::waitForClosed(QEventLoop::ProcessEventsFlag flags)
{
    TRACE_ENTRY
    mLastError = HbDeviceDialogNoError;
    int error = HbDeviceDialogNoError;

    if (iRequesting && !mEventLoop) {
        QPointer<HbDeviceDialog> guard = q_ptr;
        QEventLoop eventLoop;
        mEventLoop = &eventLoop;
        error = eventLoop.exec(flags);
        error = symToDeviceDialogError(error);
        if (guard.isNull()) {
            return false;
        }
        mEventLoop = 0;
    } else {
        error = HbDeviceDialogGeneralError;
    }

    bool result(true);

    if (error != HbDeviceDialogNoError) {
        setError(error);
        result = false;
    }
    TRACE_EXIT_ARGS("result" << result)
    return result;
}

/*!
    \internal

    Return last received data from device dialog.
*/
QVariantMap HbDeviceDialogPrivate::receivedData() const
{
    TRACE_ENTRY
    return mDataReceived;
}

/*!
    \internal

    Set and notify error status.
*/
void HbDeviceDialogPrivate::setError(int error)
{
    TRACE_ENTRY_ARGS("error" << error)
    mLastError = error;
    emit q_func()->error(error);
    TRACE_EXIT
}

/*!
    \internal

    Return last error.
*/
int HbDeviceDialogPrivate::error() const
{
    TRACE_ENTRY
    return mLastError;
}

/*!
    \internal
    RunL from CActive.
*/
void HbDeviceDialogPrivate::RunL()
{
    TRACE_ENTRY
    TInt completionCode = iStatus.Int();
    int errorCode = symToDeviceDialogError(completionCode);
    if (iConnecting) {
        iConnecting = EFalse;
        if (mEventLoop && mEventLoop->isRunning()) {
            mEventLoop->exit(errorCode);
        }
        return;
    }

    if (completionCode < KErrNone) {
        // Any Symbian error, stop requesting
        iRequesting = EFalse;
        setError(errorCode);
    } else {
        // Check that event is for latest device dialog. iDeviceDialogId was updated by server
        // during show()
        THbDeviceDialogSrvUpdateInfo &updateInfo = iUpdateInfo();
        if (updateInfo.iDeviceDialogId == iDeviceDialogId()) {
            switch(updateInfo.iUpdateType) {
            case EHbDeviceDialogUpdateData: {
                if (completionCode == HbDeviceDialogNoError &&
                    updateInfo.iInfo.iDataInfo.iDataSize > 0) {
                    // Resize buffer and get new data synchronously
                    delete iBuffer;
                    iBuffer = NULL;
                    iBuffer = HBufC8::NewL(updateInfo.iInfo.iDataInfo.iDataSize);
                    iDataPtr.Set(iBuffer->Des());
                    completionCode = mHbSession.SendSyncRequest(EHbSrvUpdateData, iDataPtr);
                    errorCode = symToDeviceDialogError(completionCode);
                    if (completionCode < KErrNone) {
                        iRequesting = EFalse;
                        setError(errorCode);
                    }
                }
                if (completionCode == HbDeviceDialogNoError) {
                    // Signal data if there are connections. Otherwise keep a copy.
                    QByteArray resArray((const char*)iDataPtr.Ptr(), iDataPtr.Size());
                    QDataStream stream(&resArray, QIODevice::ReadOnly);

                    mDataReceived.clear();
                    QVariant var;
                    stream >> var;

                    if (q_ptr->receivers(SIGNAL(dataReceived(QVariantMap))) > 0) {
                        emit q_func()->dataReceived(var.toMap());
                    } else {
                        mDataReceived = var.toMap();
                    }
                }
                break;
            }
            case EHbDeviceDialogUpdateClosed:
                // Signal possible cancelled error
                if (errorCode != HbDeviceDialogNoError) {
                    setError(errorCode);
                }
                emit q_func()->deviceDialogClosed();
                iRequesting = EFalse;
                break;
            default:
                break;
            }
        }
    }
    // Make a new request if there was no errors and device dialog wasn't closed
    if (iRequesting) {
        Start();
    } else {
        if (mEventLoop && mEventLoop->isRunning()) {
            mEventLoop->exit(errorCode);
        }
    }
    TRACE_EXIT
}

/*!
    \internal
    DoCancel from CActive.
*/
void HbDeviceDialogPrivate::DoCancel()
{
    TRACE_ENTRY
    iRequesting = EFalse;
    mHbSession.SendSyncRequest(EHbSrvCancelUpdateChannel);
    TRACE_EXIT
}

/*!
    \internal
    RunError from CActive.
*/
TInt HbDeviceDialogPrivate::RunError(TInt /*aError*/)
{
    TRACE_ENTRY
    setError(HbDeviceDialogGeneralError);
    TRACE_EXIT
    return KErrNone;
}

/*!
    \internal
    Starts asynchronous message to receive update and close events from session.
*/
void HbDeviceDialogPrivate::Start()
{
    TRACE_ENTRY
    iDataPtr.Zero();

    if (!IsActive()) {
        mHbSession.SendASyncRequest(EHbSrvOpenUpdateChannel, iDataPtr, iUpdateInfo, iStatus);
        SetActive();
        iRequesting = ETrue;
    }
    TRACE_EXIT
}

int HbDeviceDialogPrivate::receiverCount()
{
    int receiverCount(0);
    receiverCount += q_ptr->receivers(SIGNAL(dataReceived(QVariantMap)));
    receiverCount += q_ptr->receivers(SIGNAL(deviceDialogClosed()));
    return receiverCount;
}

// Convert symbian error code into HbDeviceDialog error code
int HbDeviceDialogPrivate::symToDeviceDialogError(int errorCode)
{
    if (errorCode != HbDeviceDialogNoError) {
        // Any Symbian error, close session handle. It will be reopened on next show()
        if (errorCode < KErrNone) {
            mHbSession.Close();
            // All Symbian errors are converted to HbDeviceDialogConnectError
            errorCode = HbDeviceDialogConnectError;
        } else {
            // Convert from internal to public error code
            if (errorCode == HbDeviceDialogAlreadyExists) {
                errorCode = HbDeviceDialog::InstanceExistsError;
            }
        }
    }
    return errorCode;
}
