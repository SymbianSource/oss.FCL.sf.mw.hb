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

#include <QByteArray>
#include <qlist.h>
#include <qdatastream.h>
#include <hbdevicedialogtrace_p.h>
#include <hbdevicedialogerrors_p.h>
#include "hbdevicedialogsession_p.h"
#include "hbdevicedialogserver_p.h"
#include "hbdevicedialogserversym_p_p.h"
#include "hbdevicedialogserverdefs_p.h"
#include "hbindicatorsessionhandler_p.h"
#include <e32base.h>

/*!
    HbDeviceDialogSession
    \internal
*/

/*!
    \internal
*/
HbDeviceDialogSession* HbDeviceDialogSession::NewL()
{
    TRACE_STATIC_ENTRY
    HbDeviceDialogSession* session = new (ELeave) HbDeviceDialogSession;
    TRACE_EXIT
    return session;
}

/*!
    \internal
*/
HbDeviceDialogSession::~HbDeviceDialogSession()
{
    TRACE_ENTRY
    Server().deviceDialogClientClosing(reinterpret_cast<quintptr>(this));
    // Session has closed
    Server().RemoveSessionFromList( this );
    mEventList.clear();
    if (!iKeepAfterClose) {
        for (int i(0); i < mIdList.count(); i++ ) {
            Server().closeDeviceDialog( mIdList[i] );
        }
        mIdList.clear();
        if (!iUpdateChannel.IsNull()) {
            iUpdateChannel.Complete(KErrBadHandle);
        }
    }
    delete iIndicatorSessionHandler;
    Server().RemoveSession();
    TRACE_EXIT
}

/*!
    \internal
    Update server session counter.
*/
void HbDeviceDialogSession::CreateL()
{
    TRACE_ENTRY
    Server().AddSession();
    TRACE_EXIT
}

/*!
    \internal
    Handles the servicing of a client request that has been passed
    to the server.
*/
void HbDeviceDialogSession::ServiceL(const RMessage2& aMessage)
{
    TRACE_ENTRY
    // If ServiceL leaves, default implementation in server framework
    // completes the RMessage2 with the leave code.
    DispatchMessageL( aMessage );
    TRACE_EXIT
}

/*!
    \internal
    Handles the servicing of a client request that has been passed
    to the server.
*/
void HbDeviceDialogSession::DispatchMessageL( const RMessage2 &aMessage )
{
    TRACE_ENTRY
    TInt function = aMessage.Function();
    if (function > EHbSrvIndicatorCommandsStart
        && function < EHbSrvIndicatorCommandsEnd) {
        if (!iIndicatorSessionHandler) {
            iIndicatorSessionHandler = HbIndicatorSessionHandler::NewL(this);
        }
        iIndicatorSessionHandler->HandleMessageL(aMessage);
        return;

    }
    switch( function ) {

    case EHbSrvShowDeviceDialog: {
        ShowDeviceDialogL( aMessage );
        break;
    }

    case EHbSrvUpdateDeviceDialog: {
        UpdateDeviceDialogL( aMessage );
        break;
    }

    case EHbSrvClientClosing: {
        ClientClosing( aMessage );
        break;
    }

    case EHbSrvCancelUpdateChannel: {
        CancelUpdateChannel(aMessage);
        break;
    }

    case EHbSrvCancelDeviceDialog: {
        CancelDeviceDialog( aMessage );
        break;
    }

    case EHbSrvOpenUpdateChannel: {
        UpdateChannelRequestL( aMessage );
        break;
    }

    case EHbSrvUpdateData: {
        UpdateDataRequestL( aMessage );
        break;
    }
    case EHbSrvPublishOrientation: {
        PublishOrientation( aMessage );
        break;
    }    

    default: {
        break;
    }

    }; // end switch
    TRACE_EXIT
}

/*!
    \internal
    Returns device dialog identifier stored to this session. Used to identify correct
    client.
*/
int HbDeviceDialogSession::DeviceDialogIdentifier() const
{
    TRACE_ENTRY
    int identifier(0);

    if ( !mIdList.isEmpty() ) {
        identifier = mIdList.first();
    }
    TRACE_EXIT_ARGS("identifier" << identifier)
    return identifier;
}

/*!
    \internal
    Handle device dialog update event.
*/
void HbDeviceDialogSession::DeviceDialogUpdate( const QVariantMap& parameters )
{
    TRACE_ENTRY
    int id = DeviceDialogIdentifier();
    if (id) {
        WriteUpdateData(parameters, id);
    }
    TRACE_EXIT
}

/*!
    \internal
    Handle device dialog close event.
*/
void HbDeviceDialogSession::DeviceDialogClosed( TInt identifier, TInt closeReason )
{
    TRACE_ENTRY
    Q_ASSERT(identifier != 0);
    if (mIdList.startsWith(identifier)) {
        if (iUpdateChannelOpen && mEventList.count() == 0) {
            WriteCloseData(identifier, closeReason);
        } else {
            iWriteCloseId = identifier;
            iWriteCloseReason = closeReason;
        }
    }
    mIdList.removeOne( identifier );
    TRACE_EXIT
}

/*!
    \internal
*/
HbDeviceDialogServerPrivate& HbDeviceDialogSession::Server()
{
    return *static_cast<HbDeviceDialogServerPrivate*>(const_cast<CServer2*>(CSession2::Server()));
}

/*!
    \internal
    Client has closed session.
*/
void HbDeviceDialogSession::Disconnect( const RMessage2& aMessage )
{
    TRACE_ENTRY
    CSession2::Disconnect( aMessage );
    TRACE_EXIT
}

/*!
    \internal
    Forward call to HbDeviceDialogManager. Store dialog id if call is successful.
*/
void HbDeviceDialogSession::ShowDeviceDialogL( const RMessage2 &aMessage )
{
    TRACE_ENTRY
    mEventList.clear();
    iKeepAfterClose = EFalse;
    iWriteCloseId = 0;
    TInt dataSize = aMessage.GetDesLength( KSlot0 );

    if (dataSize < 0) {
        User::Leave(KErrBadDescriptor);
    }

    HBufC8* data = HBufC8::NewLC( dataSize );
    TPtr8 ptr( data->Des() );
    aMessage.ReadL( KSlot0, ptr );

    QByteArray resArray( (const char*) ptr.Ptr(), ptr.Size() );
    QDataStream outStream( &resArray, QIODevice::ReadOnly );
    QString title;

    QVariant var;
    outStream >> title;
    outStream >> var;

    QVariantMap mapdata = var.toMap();

    CleanupStack::PopAndDestroy( data );

    int identifier = 0;
    int error = Server().AddSessionToList(this);
    if (error == HbDeviceDialogNoError) {
        HbDeviceDialogServer::DialogParameters showParameters(title, aMessage, mapdata,
            reinterpret_cast<quintptr>(this));
        identifier = Server().showDeviceDialog(showParameters);
        error = showParameters.mError;
        if (error != HbDeviceDialogNoError) {
            Server().RemoveSessionFromList(this);
        }
    }

    if (error == HbDeviceDialogNoError) {
        TPckgBuf<int> deviceDialogId(identifier);
        error = aMessage.Write(KSlot1, deviceDialogId);
        aMessage.Complete(error);
        mIdList.insert(0, identifier);
    } else {
        aMessage.Complete(error);
    }
    TRACE_EXIT_ARGS("identifier " << identifier)
}

/*!
    \internal
    Forward call to HbDeviceDialogManager. Return result.
*/
void HbDeviceDialogSession::UpdateDeviceDialogL( const RMessage2 &aMessage )
{
    TRACE_ENTRY
    TInt dataSize = aMessage.GetDesLength( KSlot0 );

    if (dataSize < 0) {
        User::Leave(KErrBadDescriptor);
    }

    HBufC8* data = HBufC8::NewLC( dataSize );
    TPtr8 ptr( data->Des() );
    aMessage.ReadL( KSlot0, ptr );

    QByteArray resArray( (const char*) ptr.Ptr(), ptr.Size() );
    QDataStream outStream( &resArray, QIODevice::ReadOnly );

    QVariant var;
    outStream >> var;

    QVariantMap mapdata = var.toMap();

    CleanupStack::PopAndDestroy( data );

    TInt result = Server().updateDeviceDialog( DeviceDialogIdentifier(), mapdata );
    aMessage.Complete( result );
    TRACE_EXIT_ARGS("result " << result)
}

/*!
    \internal
    Forward call to HbDeviceDialogManager. Return result.
*/
void HbDeviceDialogSession::CancelDeviceDialog( const RMessage2 &aMessage )
{
    TRACE_ENTRY
    TInt result = KErrNone;
    // Check that cancel is for most recent device dialog from this session
    int id = DeviceDialogIdentifier();
    if (id == aMessage.Int0()) {
        result = Server().closeDeviceDialog(id);
    }

    aMessage.Complete(result);
    TRACE_EXIT_ARGS("result " << result)
}

/*!
    \internal
    Client is about to be deleted and requests to keep device dialogs alive
    after session closes.
*/
void HbDeviceDialogSession::ClientClosing( const RMessage2 &aMessage )
    {
    TRACE_ENTRY
    iKeepAfterClose = true;
    aMessage.Complete(KErrNone);
    TRACE_EXIT
    }

/*!
    \internal
    Store asynchronous request from client.
*/
void HbDeviceDialogSession::UpdateChannelRequestL( const RMessage2 &aMessage )
{
    TRACE_ENTRY
    // We have a pending request
    iUpdateChannelOpen = ETrue;

    // Store message for later completion
    iUpdateChannel = aMessage;

    // Check if we have buffered data and complete right away.
    // Data is buffered if client is to be notified, but has
    // not made a new request for device dialog (processing the
    // previous message).
    if ( mEventList.count() > 0 ) {
        QVariantMap data = mEventList[0];

        WriteUpdateData(data, mIdList.isEmpty() ? 0 : mIdList.first());
        mEventList.removeFirst();
    } else if (iWriteCloseId) {
        WriteCloseData(iWriteCloseId, iWriteCloseReason);
    } else {
        iUpdateChannelOpen = ETrue;
    }
    TRACE_EXIT
}

/*!
    \internal
    Complete client synchronous request in device dialog initiated events. Called
    if client buffer has been too small to receive data.
*/
void HbDeviceDialogSession::UpdateDataRequestL( const RMessage2 &aMessage )
{
    TRACE_ENTRY

    TInt error = KErrNotFound;
    if (mEventList.count()) {

        QVariantMap data = mEventList[0];

        QByteArray array;
        QDataStream stream( &array, QIODevice::WriteOnly );

        QVariant var(data);
        stream << var;

        TPtr8 ptr( reinterpret_cast<TUint8*>(array.data()), array.size(), array.size());

        error = aMessage.Write( KSlot0, ptr );

        if ( error == KErrNone) {
            mEventList.removeFirst();
        }
    }
    aMessage.Complete( error );
    TRACE_EXIT_ARGS("error " << error)
}

/*!
    \internal
    Cancel update channel request.
*/
void HbDeviceDialogSession::CancelUpdateChannel(const RMessage2 aMessage)
{
    if (iUpdateChannelOpen) {
        // Complete update request
        iUpdateChannelOpen = EFalse;

        THbDeviceDialogSrvUpdateInfo updateInfo;
        TPckgBuf<THbDeviceDialogSrvUpdateInfo> updateInfoBuf(updateInfo);
        int error = iUpdateChannel.Write(KSlot1, updateInfoBuf);
        iUpdateChannel.Complete(error);
    }
    aMessage.Complete(KErrNone);
}

/*!
    \internal
    Complete client synchronous request in device dialog initiated events.
*/
void HbDeviceDialogSession::WriteUpdateData(const QVariantMap &parameters, int deviceDialogId)
{
    TRACE_ENTRY
    if ( iUpdateChannelOpen ) {
        // Client is listening and ready to receive data
        iUpdateChannelOpen = EFalse;

        // Get client data buffer size.
        TInt size = iUpdateChannel.GetDesMaxLength( KSlot0 );

        QByteArray array;
        QDataStream stream( &array, QIODevice::WriteOnly );

        QVariant var(parameters);
        stream << var;

        THbDeviceDialogSrvUpdateInfo updateInfo(EHbDeviceDialogUpdateData, deviceDialogId);
        updateInfo.iInfo.iDataInfo.iDataSize = 0;
        TPckgBuf<THbDeviceDialogSrvUpdateInfo> updateInfoBuf(updateInfo);

        if ( size >= array.size() ) {
            // Buffer size ok. Write data.
            TPtr8 ptr( reinterpret_cast<TUint8*>(array.data()), array.size(), array.size());
            TInt error = iUpdateChannel.Write( KSlot0, ptr );

            if (error == KErrNone) {
                error = iUpdateChannel.Write(KSlot1, updateInfoBuf);
            }
            iUpdateChannel.Complete( error );
        } else {
            // Client buffer size too small. Buffer data and complete request later
            // with correct data size after client has resized it's data buffer.
            // Complete message with correct data size. Client asks data synchronously.
            mEventList.append( parameters );
            updateInfoBuf().iInfo.iDataInfo.iDataSize = array.size();
            TInt error = iUpdateChannel.Write( KSlot1, updateInfoBuf );
            iUpdateChannel.Complete(error);
        }
    } else {
        // Client busy. Buffer message for next request.
        mEventList.append( parameters );
    }
    TRACE_EXIT
}

/*!
    \internal
    Complete client synchronous request in device dialog initiated events.
*/
int HbDeviceDialogSession::WriteCloseData(int deviceDialogId, int closeReason)
{
    TRACE_ENTRY
    iUpdateChannelOpen = EFalse;

    THbDeviceDialogSrvUpdateInfo updateInfo(EHbDeviceDialogUpdateClosed, deviceDialogId);

    TPckgBuf<THbDeviceDialogSrvUpdateInfo> updateInfoBuf(updateInfo);
    TInt error = iUpdateChannel.Write(KSlot1, updateInfoBuf);
    iUpdateChannel.Complete(error != HbDeviceDialogNoError ? error : closeReason);
    TRACE_EXIT_ARGS("error" << error)
    return error;
}

/*!
    \internal
    Publish current orientation to PS-key
*/
void HbDeviceDialogSession::PublishOrientation(const RMessage2 &aMessage)
{
    TRACE_ENTRY
    TInt result = KErrNone;
    TInt val0 = aMessage.Int0();
    result = Server().publishOrientation( val0 );
    
    aMessage.Complete( result );
    TRACE_EXIT_ARGS("result " << result)
}

/*!
    \internal
*/
HbDeviceDialogSession::HbDeviceDialogSession()
{
    TRACE_ENTRY
    iKeepAfterClose = false;
    iWriteCloseId = 0;
    iWriteCloseReason = HbDeviceDialogNoError;
    iUpdateChannelOpen = false;
    iIndicatorSessionHandler = 0;
    TRACE_EXIT
}
