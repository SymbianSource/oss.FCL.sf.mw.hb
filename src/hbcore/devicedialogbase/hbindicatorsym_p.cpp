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

#include "hbindicatorsym_p.h"
#include "hbdevicedialogerrors_p.h"
#include <QVariant>
#include <QPointer>
#include <e32base.h>
#include <hbdevicedialogserverdefs_p.h>
#include <hbdevicedialogtrace_p.h>

class CAsyncListener : public CActive
{
public:
    CAsyncListener(HbIndicatorPrivate *indicator) : CActive(EPriorityStandard), iIndicator(indicator) 
    {
        CActiveScheduler::Add(this);
    }
    TInt Start() 
    {
        TInt error = KErrNone;
        if (!IsActive()) {
            // Async connect to prevent deadlock if application is started from a
            // device dialog server plugin.
            iStatus = KRequestPending;
            error = iIndicator->mHbSession.Connect(&iStatus);
            if (error == KErrNone) {
                SetActive();
            }
        }
        return error;
    }
    void RunL()
    {
        if (iStatus == KErrNone) {
            iIndicator->Start();
        }
    }
    void DoCancel()
    {
        // No Cancel() available for RSessionBase::Connect()
        // Nevertheless we must do something to prevent deadlocking.
        TRequestStatus *rs = &iStatus;
        User::RequestComplete(rs, KErrCancel);
    }
private:
    HbIndicatorPrivate *iIndicator;
};

/*!
    \class HbIndicatorPrivate
    \internal
*/

/*!
    \internal
*/
HbIndicatorPrivate::HbIndicatorPrivate()
: CActive( EPriorityStandard ),
  iInitialized(EFalse),
  iRequesting(EFalse),
  iMsgTypePtr(NULL,0,0),
  iBuffer(0),
  iDataPtr(NULL,0,0),
  iLastError(HbDeviceDialogNoError),
  iAsyncListener(0)
{
    TRACE_ENTRY
    TRACE_EXIT
}

/*!
    \internal
*/
HbIndicatorPrivate::~HbIndicatorPrivate()
{
    TRACE_ENTRY
    stopListen();
    delete iBuffer;
    if (iAsyncListener) {
        iAsyncListener->Cancel();
        delete iAsyncListener;
    }
    TRACE_EXIT
}

/*!
    \internal
    Initialisation step.
*/
void HbIndicatorPrivate::init()
{
    TRACE_ENTRY
    iLastError = HbDeviceDialogNoError;
    CActiveScheduler::Add( this );
    TRACE_EXIT
}

/*!
    \internal
    Initialisation step. Session to server is created.
*/
void HbIndicatorPrivate::initializeL(bool forListening)
{
    TRACE_ENTRY
    TInt error = KErrNone;
    bool asynchConnect = false;
    if (forListening) {
        CreateReceiveBufferL(DefaultReceiveBufferSize);
        // If device dialog server is running, connect asynchronously. This is to
        // prevent a deadlock if server plugin is starting an application.
        asynchConnect = RHbDeviceDialogClientSession::ServerRunning();
        if (asynchConnect){
            if (!iAsyncListener) {
                iAsyncListener = new (ELeave) CAsyncListener(this);
            }
            error = iAsyncListener->Start();
        } else {
            error = mHbSession.Connect();
        }
    } else {
        error = mHbSession.Connect();
    }

    if (error != KErrNone) {
        TRACE("initialize error: " << error);
        setError(HbDeviceDialogConnectError);
        User::Leave(error);
    }
    iInitialized = ETrue;
    if (forListening && !asynchConnect) {
        Start();
    }
    TRACE_EXIT
}

bool HbIndicatorPrivate::activate(const QString &indicatorType, const QVariant &parameter)
{
    TRACE_ENTRY
    return sendActivateMessage(indicatorType, ETrue, parameter);
}

bool HbIndicatorPrivate::deactivate(const QString &indicatorType, const QVariant &parameter)
{
    TRACE_ENTRY
    return sendActivateMessage(indicatorType, EFalse, parameter);
}

bool HbIndicatorPrivate::startListen()
{
    TRACE_ENTRY
    if (!iInitialized) {
        TRAPD( error, initializeL(true) );

        if ( error != KErrNone ){
            setError( HbDeviceDialogConnectError );
            TRACE_EXIT_ARGS("error " << error)
            return false;
        }
    }
    TRACE_EXIT
    return true;
}

bool HbIndicatorPrivate::listening() const
{
    return iRequesting;
}

bool HbIndicatorPrivate::stopListen()
{
    TRACE_ENTRY
    // Close connections to server
    bool ok = false;
    if (iRequesting) {
        Close();
    }
    if (iInitialized) {
        mHbSession.Close();
        iInitialized = EFalse;
    }

    Cancel();
    ok = ETrue;
    TRACE_EXIT
    return ok;
}

/*!
    \internal

    Set and notify error status.
*/
void HbIndicatorPrivate::setError( int error )
{
    TRACE_ENTRY_ARGS("error " << error)
    iLastError = error;
    emit this->error(iLastError);
    TRACE_EXIT
}

/*!
    \internal

    Return last error.
*/
int HbIndicatorPrivate::error() const
{
    TRACE_ENTRY
    return iLastError;
}

/*!
    \internal
    RunL from CActive.
*/
void HbIndicatorPrivate::RunL()
    {
    TRACE_ENTRY
    iLastError = HbDeviceDialogNoError;
    TInt status = iStatus.Int();
    iRequesting = EFalse;
    
    if (status >= 0 && iMsgType == EHbIndicatorUserActivated) {
        iMsgType = -1;
        if (status > 0) {
            CreateReceiveBufferL(status);
            TInt error = mHbSession.SendSyncRequest(EHbSrvActivatedIndicatorData,
                iDataPtr, &iMsgTypePtr);
        }

        QByteArray resArray((const char*)iDataPtr.Ptr(), iDataPtr.Size());
        QDataStream stream(&resArray, QIODevice::ReadOnly);
        QVariant var;
        stream >> var;

        if (q_ptr && q_ptr->receivers(SIGNAL(userActivated(QString, QVariantMap))) > 0) {
            QVariantMap map = var.toMap();
            emit q_func()->userActivated(map.value("type").toString(), map.value("data").toMap());
        }
    } else if (status >= 0) {
        QByteArray resArray( (const char*) iDataPtr.Ptr(), iDataPtr.Size() );
        QDataStream stream( &resArray, QIODevice::ReadOnly);

        QList<IndicatorClientInfo> activatedList;
        QList<IndicatorClientInfo> updatedList;
        QList<IndicatorClientInfo> deactivatedList;

        quint16 count = 0;
        stream >> count;
        IndicatorClientInfo clientInfo;
        int state = 0;
        for (int i = 0; i < count; ++i) {
            stream >> state;
            stream >> clientInfo;
            
            switch(state) {
            case IndicatorUpdated:
                updatedList.append(clientInfo);
                break;
            case IndicatorActivated:
                activatedList.append(clientInfo);
                break;
            case IndicatorDeactivated:
                deactivatedList.append(clientInfo);
                break;
            default:
                break;
            }
        }
        if (iMsgType == EHbIndicatorAllActivated) {
            iActivatedBuffer += activatedList;
            activatedList.clear();
            if (status == 0 && !iActivatedBuffer.isEmpty()) {
                emit allActivated(iActivatedBuffer);
                iActivatedBuffer.clear();
            }
        }

        if (!activatedList.isEmpty()) {
            emit activated(activatedList);
        }
        if (!deactivatedList.isEmpty()) {
            emit deactivated(deactivatedList);
        }
        if (!updatedList.isEmpty()) {
            emit updated(updatedList);
        }

        if ( status > 0 && iBuffer->Size() < status ) {
            // Resize buffer.
            CreateReceiveBufferL(status);
        }
    } else {
        setError(status);
    }

    if ( status != KErrServerTerminated && status != KErrCancel ) {
        Continue();
    }
    TRACE_EXIT
}

/*!
    \internal
    DoCancel from CActive.
*/
void HbIndicatorPrivate::DoCancel()
{
    TRACE_ENTRY
    TRequestStatus *rs = &iStatus;
    User::RequestComplete(rs, KErrCancel);
    TRACE_EXIT
}

/*!
    \internal
    RunError from CActive.
*/
TInt HbIndicatorPrivate::RunError( TInt /*aError*/ )
{
    TRACE_ENTRY
    setError( HbDeviceDialogGeneralError );
    TRACE_EXIT
    return KErrNone;
}

/*!
    \internal
    Starts asynchronous message to receive update and close events from session.
*/
void HbIndicatorPrivate::Start()
{
    TRACE_ENTRY

    if (!iRequesting && iInitialized && !IsActive()) {
        if (!iBuffer) {
            CreateReceiveBufferL(DefaultReceiveBufferSize);
        }
        iDataPtr.Zero();
        TPckg<TInt> pckg( iMsgType );
        iMsgTypePtr.Set( pckg ); //iMsgTypePtr is ignored in server side.
    
        mHbSession.SendASyncRequest( EHbSrvGetActivatedIndicatorsStart,
            iDataPtr, iMsgTypePtr, iStatus );
        SetActive();
        iRequesting = ETrue;
    }
    TRACE_EXIT
}

void HbIndicatorPrivate::Continue()
{
    TRACE_ENTRY

    if (!iRequesting && iInitialized && !IsActive()) {
        TPckg<TInt> pckg( iMsgType );
        iMsgTypePtr.Set( pckg );

        mHbSession.SendASyncRequest( EHbSrvGetActivatedIndicatorContinue,
                iDataPtr, iMsgTypePtr, iStatus );
        SetActive();
        iRequesting = ETrue;
    }
    TRACE_EXIT
}

void HbIndicatorPrivate::Close()
{
    TRACE_ENTRY
    if (iRequesting) {
        TRACE("Send close message")
        mHbSession.SendSyncRequest(EhbSrvGetActivatedIndicatorsClose);
        iRequesting = EFalse;
    }
    TRACE_EXIT
}

int HbIndicatorPrivate::sendActivateMessage(const QString &indicatorType,
        TBool activate, const QVariant& parameter)
{
    TRACE_ENTRY
    iLastError = HbDeviceDialogNoError;
    
    TBool listening = (activate && q_ptr->receivers(SIGNAL(userActivated(QString, QVariantMap))) > 0);
    
    if (!iInitialized) {
                            
        TRAPD( error, initializeL(listening) );

        if ( error != KErrNone ){
            setError( HbDeviceDialogConnectError );
            TRACE_EXIT_ARGS("error " << error)
            return false;
        }
    } else if (listening) {
        Start();
    }
    QByteArray array;
    QDataStream stream( &array, QIODevice::WriteOnly );
    stream << indicatorType;
    stream << parameter;

    TPtrC8 ptr( reinterpret_cast<TUint8*>(array.data()), array.size() );

    // Synchronic call to server to activate or deactivate indicator
    int message = EHbSrvActivateIndicator;
    if (!activate) {
        message = EHbSrvDeactivateIndicator;
    }
    int error = mHbSession.SendSyncRequest( message, ptr );

    bool result(true);

    if ( error != KErrNone ) {
        setError( error );
        result = false;
    }
    TRACE_EXIT
    return result;
}

void HbIndicatorPrivate::CreateReceiveBufferL(int size)
{
    delete iBuffer; iBuffer = 0;
    iBuffer = HBufC8::NewL(size);
    iDataPtr.Set(iBuffer->Des());
    iDataPtr.Zero();
}
