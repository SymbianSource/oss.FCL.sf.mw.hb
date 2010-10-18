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
#include <QVariantMap>
#include <hbdevicedialogtrace_p.h>
#include <hbdevicedialogerrors_p.h>
#include "hbdevicedialogsession_p.h"
#include "hbdevicedialogserversym_p_p.h"
#include "hbdevicedialogserverdefs_p.h"
#include "hbindicatorsessionhandler_p.h"
#include "hbdevicedialogserver_p.h"
#include <e32base.h>

/*!
    HbIndicatorSession
    \internal
*/

//stream operators for IndicatorQueueItem
QDataStream& operator << (QDataStream &outStream,
        const IndicatorQueueItem &obj)
{
    outStream << obj.state;
    outStream << obj.clientInfo;
    return outStream;
}

QDataStream& operator >> (QDataStream &inStream,
        IndicatorQueueItem &obj)
{
    int state;
    inStream >> state;
    obj.state = static_cast<IndicatorState>(state);
    inStream >> obj.clientInfo;
    return inStream;
}

/*!
    \internal
*/
HbIndicatorSessionHandler* HbIndicatorSessionHandler::NewL(
        HbDeviceDialogSession *aSession)
{
    TRACE_STATIC_ENTRY
    HbIndicatorSessionHandler* session =
        new (ELeave) HbIndicatorSessionHandler(aSession);
    TRACE_EXIT
    return session;
}

/*!
    \internal
*/
HbIndicatorSessionHandler::~HbIndicatorSessionHandler()
{
    TRACE_ENTRY
    Server().RemoveIndicatorHandlerFromList(this);
    TRACE_EXIT
}

/*!
    \internal
    Handles the servicing of a client request that has been passed
    to the server.
*/
void HbIndicatorSessionHandler::HandleMessageL( const RMessage2 &aMessage )
{
    TRACE_ENTRY

    switch( aMessage.Function() ) {
    case EHbSrvActivateIndicator: {
        ActivateIndicatorL( aMessage );
        break;
    }
    case EHbSrvDeactivateIndicator: {
        DeactivateIndicatorL( aMessage );
        break;
    }
    case EHbSrvGetActivatedIndicatorsStart: {
        iIndicatorQueue.clear();
        addToIndicatorQueue(Server().indicatorClientInfoList(),
                IndicatorActivated);
        iMsgType = EHbIndicatorAllActivated;
        IndicatorChannelRequestL( aMessage );
        break;
    }
    case EHbSrvGetActivatedIndicatorContinue: {
        if (iMsgType == -1) {
            iMsgType = EHbIndicatorUpdates;
        }
        IndicatorChannelRequestL( aMessage );
        break;
    }
    case EhbSrvGetActivatedIndicatorsClose: {
        Server().RemoveIndicatorHandlerFromList(this);
        if (iIndicatorChannelOpen) {
            iIndicatorChannelOpen = EFalse;
            iIndicatorChannel.Complete(KErrNone);
        }
        aMessage.Complete( KErrNone );
        break;
    }
    case EHbSrvActivatedIndicatorData: {
        WriteIndicatorDataL(aMessage);
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
    Called, when indicator(s) have been activated.
*/
void HbIndicatorSessionHandler::IndicatorsActivated(
        const QList<IndicatorClientInfo> &activated)
{
    TRACE_ENTRY
    addToIndicatorQueue(activated, IndicatorActivated);
    TRAP_IGNORE( WriteIndicatorInfoL() );
    TRACE_EXIT
}

void HbIndicatorSessionHandler::IndicatorsUpdated(
        const QList<IndicatorClientInfo> &updated)
{
    TRACE_ENTRY
    addToIndicatorQueue(updated, IndicatorUpdated);
    TRAP_IGNORE( WriteIndicatorInfoL() );
    TRACE_EXIT
}

/*!
    \internal
    Called, when indicator(s) have been deactivated.
*/
void HbIndicatorSessionHandler::IndicatorsDeactivated(
        const QList<IndicatorClientInfo> &deactivated)
{
    TRACE_ENTRY
    addToIndicatorQueue(deactivated, IndicatorDeactivated);
    TRAP_IGNORE( WriteIndicatorInfoL() );
    TRACE_EXIT
}

void HbIndicatorSessionHandler::IndicatorUserActivated(const QVariantMap& data)
{
    QString type = data.value("type").toString();
        
    if (indicatorTypes.contains(type) && iIndicatorChannelOpen) {
        indicatorDataMap = data;
        TRAP_IGNORE(WriteIndicatorDataL(iIndicatorChannel));
    }
}

HbDeviceDialogServerPrivate& HbIndicatorSessionHandler::Server()
{
    return iSession->Server();
}

/*!
    \internal
    handle indicator activation.
*/
void HbIndicatorSessionHandler::ActivateIndicatorL(const RMessage2 &aMessage)
{
    TRACE_ENTRY
    
    QString type;
    QVariant parameter;
    indicatorTypeFromMessageL(aMessage, type, parameter);
    HbDeviceDialogServer::IndicatorParameters indicatorParameters(type, aMessage, parameter);
    TInt result = Server().activateIndicator(indicatorParameters);
    if (result == HbDeviceDialogNoError && !indicatorTypes.contains(type)) {
        indicatorTypes.append(type); // there is heap to append if activate succeeded
    }
    aMessage.Complete(result);
    TRACE_EXIT_ARGS("result: " << result)
}

/*!
    \internal
    handle indicator deactivation.
*/
void HbIndicatorSessionHandler::DeactivateIndicatorL( const RMessage2 &aMessage )
{
    TRACE_ENTRY
    QString type;
    QVariant parameter;
    indicatorTypeFromMessageL(aMessage, type, parameter);
    indicatorTypes.removeAll(type);
    
    if (indicatorTypes.isEmpty() && iIndicatorChannelOpen) {
        indicatorDataMap.clear();
        iIndicatorChannelOpen = false;
        iIndicatorChannel.Complete(KErrCancel);
    }    
    
    HbDeviceDialogServer::IndicatorParameters indicatorParameters(type, aMessage, parameter);
    TInt result = Server().deactivateIndicator(indicatorParameters);
    aMessage.Complete(result);
    TRACE_EXIT_ARGS("result: " << result)
}

void HbIndicatorSessionHandler::IndicatorChannelRequestL(
        const RMessage2 &aMessage )
{
    TRACE_ENTRY
    iIndicatorChannelOpen = ETrue;
    iIndicatorChannel = aMessage;

    TInt result = Server().AddIndicatorHandlerToList(this);
    if ( result != KErrNone ) {
        // Nothing to do if adding fails.
        iIndicatorChannel.Complete( result );
        iIndicatorChannelOpen = false;
        return;
    }

    WriteIndicatorInfoL();
    
    // check whether there is buffered indicator data also present.
    if (iIndicatorChannelOpen && !indicatorDataMap.isEmpty()) {
        WriteIndicatorDataL(iIndicatorChannel);
    }
    TRACE_EXIT
}

/*!
    \internal
    add client infos to queue, where they are sent to client.
*/
void HbIndicatorSessionHandler::addToIndicatorQueue(
        const QList<IndicatorClientInfo> &clientInfoList,
        IndicatorState state)
{
    foreach(const IndicatorClientInfo &clientInfo, clientInfoList) {
        iIndicatorQueue.append(IndicatorQueueItem(clientInfo, state));
    }
}

/*!
    \internal
    write indicator info from indicator queue to client.
*/
void HbIndicatorSessionHandler::WriteIndicatorInfoL()
{
    if ( iIndicatorChannelOpen ) {
        // Client is listening and ready to receive data

        if (iIndicatorQueue.count() > 0) {
            iIndicatorChannelOpen = EFalse;

            TPckgBuf<TInt> buf( iMsgType );
            TInt error = iIndicatorChannel.Write( KSlot1, buf );
            User::LeaveIfError( error );

            TInt leftOver = DoWriteIndicatorInfoL(error);
            if (leftOver >= 0) {
                //remove those, that were sent.
                iIndicatorQueue.erase(iIndicatorQueue.begin(),
                    iIndicatorQueue.begin()
                    + (iIndicatorQueue.count() - leftOver));
                if (iMsgType == EHbIndicatorAllActivated
                    && iIndicatorQueue.count() == 0) {
                        iMsgType = EHbIndicatorUpdates; //send only updates from now on.
                }
            }
            iIndicatorChannel.Complete( error );
        }
    }
}

/*!
    \internal
    helper function fo WriteIndicatorInfoL
*/
TInt HbIndicatorSessionHandler::DoWriteIndicatorInfoL(TInt &error)
{
    TRACE_ENTRY
    error = KErrNone;
    quint16 clientInfoCount = static_cast<quint16>(iIndicatorQueue.count());
    quint16 clientInfoStoreCount = 0;

    // Get client data buffer size.
    QByteArray array;
    TInt desSize = iIndicatorChannel.GetDesMaxLength( KSlot0 );
    if (desSize > sizeof(quint16) + 16) {
        //check, how many activated indicator infos will fit the size.
        QDataStream outStream(&array, QIODevice::WriteOnly);

        clientInfoStoreCount = clientInfoCount; //assume all fits.
        outStream << clientInfoCount;
        for(int i = 0; i < clientInfoCount; ++i) {
            outStream << iIndicatorQueue[i];
            if (array.size() > desSize) {
                clientInfoStoreCount = i; //can't fit all.
                break;
            }
        }
    }

    TBool hasLeftOver = ETrue;
    if (clientInfoStoreCount > 0) {
        hasLeftOver = EFalse;
        if (clientInfoStoreCount < clientInfoCount) {
            hasLeftOver = ETrue;
            // fill the buffer again with correct count.
            array.clear();
            QDataStream outStream(&array, QIODevice::WriteOnly);
            outStream << clientInfoStoreCount;
            for(int i = 0; i < clientInfoStoreCount; ++i) {
                outStream << iIndicatorQueue[i];
            }
        }
        //write data to message.
        TPtr8 ptr( reinterpret_cast<TUint8*>(array.data()),
                                             array.size(), array.size());
        error = iIndicatorChannel.Write( KSlot0, ptr );
        User::LeaveIfError( error );
    }

    if (hasLeftOver) {
        //not all infos fitted, calculate how much size is needed to get the rest.
        array.clear();
        QDataStream stream(&array, QIODevice::WriteOnly);
        stream << quint16(0);
        for (int i = clientInfoStoreCount; i < clientInfoCount; ++i) {
            stream << iIndicatorQueue[i];
        }

        // Complete with data length
        error = array.size();
    }

    TRACE_EXIT
    return clientInfoCount - clientInfoStoreCount;
}

void HbIndicatorSessionHandler::WriteIndicatorDataL(const RMessage2& aMessage)
{    
    iIndicatorChannelOpen = EFalse;
    QByteArray array;
    QDataStream stream( &array, QIODevice::WriteOnly );

    QVariant var(indicatorDataMap);
    stream << var;
      
    // Get client data buffer size
    TInt size = aMessage.GetDesMaxLength( KSlot0 );
    
    TPckgBuf<TInt> buf( EHbIndicatorUserActivated );    
    User::LeaveIfError(aMessage.Write(KSlot1, buf));

    if (size >= array.size()) {
        // Buffer ok. Write data.
        TPtr8 ptr( reinterpret_cast<TUint8*>(array.data()), array.size(), array.size());
        TInt error = aMessage.Write( KSlot0, ptr );     
        aMessage.Complete(error);
        indicatorDataMap.clear();
    } else {
        aMessage.Complete(array.size());
    }    
}
/*!
    \internal
    get the indicator type and parameter from the message.
*/
void HbIndicatorSessionHandler::indicatorTypeFromMessageL(const RMessage2 &aMessage,
    QString &indicatorType, QVariant &parameter) const
{
    TRACE_ENTRY
    TInt dataSize = aMessage.GetDesLength(KSlot0);
    if (dataSize < 0) {
        User::Leave(KErrBadDescriptor);
    }

    HBufC8* data = HBufC8::NewLC(dataSize);
    TPtr8 ptr(data->Des());
    aMessage.ReadL(KSlot0, ptr);

    int error = KErrNone;
    QT_TRYCATCH_ERROR(error,
        QByteArray resArray((const char*) ptr.Ptr(), ptr.Size());
        QDataStream outStream(&resArray, QIODevice::ReadOnly);
        outStream >> indicatorType;
        outStream >> parameter;
    )
    User::LeaveIfError(error);
    CleanupStack::PopAndDestroy(data);
    TRACE_EXIT
}

/*!
    \internal
*/
HbIndicatorSessionHandler::HbIndicatorSessionHandler(
        HbDeviceDialogSession *aSession) : iSession(aSession), iMsgType(-1)
{
    TRACE_ENTRY
    TRACE_EXIT
}
