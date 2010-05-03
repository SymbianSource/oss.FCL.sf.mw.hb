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

#include <QVariant>

#include "hbdevicedialogserverdefs_p.h"
#include "hbdevicedialogerrors_p.h"
#include "hbdevicedialogclientsession_p.h"
#include <e32cmn.h>

#include "hbsymbiandevicedialog.h"
#include "hbsymbianvariant.h"
#include "hbsymbianvariantconverter_p.h"

/*!
 \class CHbDeviceDialog
 \brief CHbDeviceDialog is a Symbian implementation of HbDeviceDialog.

 CHbDeviceDialog displays dialogs on top of applications. It is a client interface for Symbian applications to
 Hb device dialogs.

 \deprecated CHbDeviceDialog
     is deprecated. Replaced by a class CHbDeviceDialogSymbian.

 \sa HbDeviceDialog

 Data given to this API in s60 format is packed to QVariantMap. See
 CHbSymbianVariant to see which Qt datatypes are supported.
 \sa CHbSymbianVariant
 \sa CHbSymbianVariantMap

 When plugin returns data in Qt format, the data is converted, if possible,
 to CHbSymbianVariantMap.

 \proto

 \hbcore
 */

/*!
    \enum CHbDeviceDialog::TDeviceDialogError
    Defines device dialog error codes and ranges.
*/
/*!
    \var TDeviceDialogError::DeviceDialogError HbDeviceDialog::FrameworkErrors
    Start of an error range for errors originating from device dialog framework (client or server).
*/
/*!
    \var TDeviceDialogError::DeviceDialogError HbDeviceDialog::PluginErrors
    Start of an error range for errors originating from device dialog plugins. The framework passes
    these from the plugin unmodified.
*/
/*!
    \var TDeviceDialogError::DeviceDialogError HbDeviceDialog::ErrorTypeMask
    Mask for error type part of the error code.
*/
/*!
    \var TDeviceDialogError::DeviceDialogError HbDeviceDialog::CancelledError
    Operation was cancelled by Cancel().
*/
/*!
    \var DeviceDialogs60Error::DeviceDialogError HbDeviceDialog::SystemCancelledError
    Operation was cancelled by device dialog framework.
*/

/*!
   \fn void MHbDeviceDialogObserver::DataReceived(CHbSymbianVariantMap& aData)

    This callback is called when data is received from a device dialog.
    \a aData contains data from the dialog plugin.
    The structure and meaning of the data is a contract between the dialog and
    a client. Structure should be aligned with the data types supported by
	CHbSymbianVariantMap.

    \sa CHbSymbianVariantMap.
*/

/*!
    \fn void MHbDeviceDialogObserver::DeviceDialogClosed(TInt aCompletionCode)

    This callback is called when a device dialog is closed. Any data sent by
    the dialog is indicated by the dataReceived() callback. If no observer is
    set in CHbDeviceDialog::Show the latest data can be retrieved with
    CHbDeviceDialog::receivedData()

    \a aCompletionCode gives the result of the dialog completion. Code can be
    either Symbian error code or device dialog error code
    device dialog error code

    \sa DataReceived() ReceivedData()
*/

// Device dialogs are implemented only for Symbian/S60 OS.

class CHbDeviceDialogPrivate : public CActive
{
public:
        CHbDeviceDialogPrivate();
        ~CHbDeviceDialogPrivate();
        TInt Show( const QByteArray& aArray );
        TInt Update( const QByteArray& aArray );
        void CancelDialog();
        TInt Error() const;
        void SetObserver( MHbDeviceDialogObserver* aObserver );

        // CActive
        void RunL();
        void DoCancel();
        TInt RunError( TInt aError );

        void Start();
        TInt SymToDeviceDialogError( TInt errorCode );
        void SetError(TInt aError);

public:
    TInt iFlags;
    RHbDeviceDialogClientSession iHbSession;
    TInt iLastError;
    TPckgBuf<THbDeviceDialogSrvUpdateInfo> iUpdateInfo;
    TPckgBuf<int> iDeviceDialogId;
    HBufC8* iBuffer;
    TPtr8 iDataPtr;
    TBool iRequesting;
    QVariantMap iDataReceived;
    CActiveSchedulerWait* iWait;
    MHbDeviceDialogObserver* iObserver;

};

CHbDeviceDialogPrivate::CHbDeviceDialogPrivate():
CActive( EPriorityStandard ),
iFlags(0),
iLastError(0),
iDeviceDialogId(0),
iBuffer(NULL),
iDataPtr(NULL, 0, 0),
iRequesting(EFalse),
iWait(NULL),
iObserver(NULL)
{
    if (!iBuffer) {
        iBuffer = HBufC8::NewL(64);
        if (iBuffer) {
            iDataPtr.Set(iBuffer->Des());
        }
    }
}

CHbDeviceDialogPrivate::~CHbDeviceDialogPrivate()
{
    // Inform the server to finish the dialog session and not to cancel it
    if(!iObserver) {
        iHbSession.SendSyncRequest(EHbSrvClientClosing);
    }

    Cancel();
    iHbSession.Close();

    delete iBuffer;
    iBuffer = NULL;

    iObserver= NULL;
    // stop synchorous calls
    if(iWait) {
        if( iWait->IsStarted() ) {
            iWait->AsyncStop();
        }
        delete iWait;
        iWait = NULL;
    }
}

TInt CHbDeviceDialogPrivate::Show(const QByteArray& aArray )
{
    TInt error = iLastError = KErrNone;

    TPtrC8 ptr( reinterpret_cast<const TUint8*>(aArray.data()), aArray.size() );
    // Synchronous call to server to show dialog.
    error = iHbSession.SendSyncRequest( EHbSrvShowDeviceDialog, ptr, &iDeviceDialogId );
    //error = SymToDeviceDialogError(error);

    if (error == KErrNone) {
        // Start listening for server updates. Device dialog update and closing is
        // received via this channel. Error status received in RunL method.
        Start();
    }
    // Error, just return the error code
    else {
        SetError(error);
    }
    return error;
}

/*!
    \internal

    Send device dialog update.
*/
TInt CHbDeviceDialogPrivate::Update( const QByteArray& aArray )
{
    TInt error = iLastError = KErrNone;
    if (iRequesting) {

        TPtrC8 ptr( reinterpret_cast<const TUint8*>(aArray.data()), aArray.size() );

        error = iHbSession.SendSyncRequest( EHbSrvUpdateDeviceDialog, ptr );
        //error = SymToDeviceDialogError(error);
        if (error != KErrNone) {
            SetError(error);
        }
    }
    else {
        SetError(KErrBadHandle);
        error = KErrBadHandle;
    }
    return error;
}

/*!
    \internal

    Cancel a scheduled popup on HbDeviceDialogManager. Event buffer is cleared
    at server.
*/
void CHbDeviceDialogPrivate::CancelDialog()
{
    iLastError = KErrNone;
    int error = KErrNotFound;

    if (iRequesting) {
        // Ignore other than server errors.
        error = iHbSession.SendSyncRequest(EHbSrvCancelDeviceDialog, iDeviceDialogId());
        // error = SymToDeviceDialogError(error);
    }
    if (error != KErrNone) {
        SetError(error);
    }
}

/*!
    \internal

    Return last error.
*/
TInt CHbDeviceDialogPrivate::Error() const
{
    return iLastError;
}

void CHbDeviceDialogPrivate::SetObserver( MHbDeviceDialogObserver* aObserver )
{
    iObserver = aObserver;
}
/*!
    \internal
    RunL from CActive.
*/
void CHbDeviceDialogPrivate::RunL()
{
    TInt completionCode = iStatus.Int();
    //int errorCode = SymToDeviceDialogError(completionCode);

    if (completionCode < KErrNone) {
        // Any Symbian error, stop requesting, sycnhoronous requests are stopped
        // in the end of the RunL
        iRequesting = EFalse;
        SetError(completionCode);
        if(iObserver) {
            iObserver->DeviceDialogClosed(completionCode);
        }
    }
    else {
        // Check that event is for latest device dialog. iDeviceDialogId was updated by server
        // during show()
        THbDeviceDialogSrvUpdateInfo &updateInfo = iUpdateInfo();
        if (updateInfo.iDeviceDialogId == iDeviceDialogId()) {
            switch(updateInfo.iUpdateType) {
            case EHbDeviceDialogUpdateData: {
                if (completionCode == KErrNone &&
                    updateInfo.iInfo.iDataInfo.iDataSize > 0) {
                    // Resize buffer and get new data synchronously
                    delete iBuffer;
                    iBuffer = NULL;
                    iBuffer = HBufC8::NewL(updateInfo.iInfo.iDataInfo.iDataSize);
                    iDataPtr.Set(iBuffer->Des());
                    completionCode = iHbSession.SendSyncRequest(EHbSrvUpdateData, iDataPtr);
                    //errorCode = SymToDeviceDialogError(completionCode);

                    // data request failed
                    if (completionCode < KErrNone) {
                        iRequesting = EFalse;
                        SetError(completionCode);
                        if(iObserver) {
                            iObserver->DeviceDialogClosed(completionCode);
                        }
                    }
                }
                if (completionCode == KErrNone) {
                    // Signal data if there are connections. Otherwise keep a copy.
                    QByteArray resArray((const char*)iDataPtr.Ptr(), iDataPtr.Size());
                    QDataStream stream(&resArray, QIODevice::ReadOnly);

                    iDataReceived.clear();

                    QVariant var;
                    stream >> var;
                    QVariantMap varMap = var.toMap();

                    if(iObserver) {
                        CHbSymbianVariantMap* symbianMap =
                            HbSymbianVariantConverter::fromQVariantMapL(varMap);
                        iObserver->DataReceived(*symbianMap);
                        delete symbianMap;
                        symbianMap = 0;
                    }
                    else {
                        iDataReceived = varMap;
                    }
                }
                break;
            }
            case EHbDeviceDialogUpdateClosed:
                // Signal possible cancelled error
                if (completionCode != KErrNone) {
                    SetError(completionCode);
                }
                iRequesting = EFalse;
                if(iObserver) {
                    iObserver->DeviceDialogClosed(completionCode);
                }
                break;
            default:
                break;
            }
        }
    }
    // Make a new request if there was no errors and device dialog wasn't closed
    if (iRequesting) {
        Start();
    }
    else if (iWait && iWait->IsStarted()){
        // No requests going so stop
        iWait->AsyncStop();
    }
}

/*!
    \internal
    DoCancel from CActive.
*/
void CHbDeviceDialogPrivate::DoCancel()
{
    SetError(KErrCancel);
    iRequesting = EFalse;
    iHbSession.SendSyncRequest(EHbSrvCancelUpdateChannel);
    if(iObserver) {
        iObserver->DeviceDialogClosed(KErrCancel);
    }
}

/*!
    \internal
    RunError from CActive.
*/
TInt CHbDeviceDialogPrivate::RunError( TInt /*aError*/ )
{
    SetError( KErrGeneral );
    return KErrNone;
}

/*!
    \internal
    Starts asynchronous message to receive update and close events from session.
*/
void CHbDeviceDialogPrivate::Start()
{
    iDataPtr.Zero();

    if ( !IsActive() ) {
        iHbSession.SendASyncRequest( EHbSrvOpenUpdateChannel, iDataPtr, iUpdateInfo, iStatus );
        SetActive();
        iRequesting = ETrue;
    }
}

// Convert symbian error code into HbDeviceDialog error code
int CHbDeviceDialogPrivate::SymToDeviceDialogError( TInt errorCode )
{
    if (errorCode != HbDeviceDialogNoError) {
        // Any Symbian error, close session handle. It will be reopened on next show()
        if (errorCode < KErrNone) {
            iHbSession.Close();
        }
        // All Symbian errors are connected to HbDeviceDialogConnectError
        if (errorCode < KErrNone) {
            errorCode = HbDeviceDialogConnectError;
        }
    }
    return errorCode;
}

void CHbDeviceDialogPrivate::SetError( TInt aError )
{
    iLastError = aError;
}

/*!
    Constructs CHbDeviceDialog object. \a f contains construct flags. Device
    dialog service will clean all dialogs launched when the instance is deleted.
*/

EXPORT_C CHbDeviceDialog* CHbDeviceDialog::NewL( TInt aFlags )
{
     CHbDeviceDialog* s60DeviceDialog = new CHbDeviceDialog(aFlags);
     int error = KErrNone;
     if(s60DeviceDialog->d) {
         error = s60DeviceDialog->d->iHbSession.Connect();
     }
     if(error != KErrNone) {
         CleanupStack::PushL(s60DeviceDialog);
         User::Leave(error);
         delete s60DeviceDialog;
         s60DeviceDialog = 0;
     }
     return s60DeviceDialog;
}

EXPORT_C CHbDeviceDialog::~CHbDeviceDialog()
{
    delete d;
}

/*!
  Show of device dialog. aParameter data is sent to
  device dialog in Qt's QVariantMap object. The method call is synchronous if
  no observer is set. When aObserver is set, the call is asynchronous. Deleting
  the CHbDeviceDialog object does not cancel the call but leaves the plugin on the
  server to complete the session. Use  CHbDeviceDialog::Cancel to cancel the device
  dialog.

  \a aDeviceDialogType is name of the device dialog. Identifies the device
  dialog plugin. \a aParameters is a buffer containing data for the device dialog.

  \a aObserver is used to observe the session. If set, the call is asynchronous.

  Return value informs if the call was successful.
 */
EXPORT_C TInt CHbDeviceDialog::Show(const TDesC& aDeviceDialogType, const CHbSymbianVariantMap& aParameters, MHbDeviceDialogObserver* aObserver)
{
    d->SetObserver(aObserver);

    QString deviceDialogType = QString::fromUtf16(aDeviceDialogType.Ptr(), aDeviceDialogType.Length());

    QVariantMap parameters;
    HbSymbianVariantConverter::toQtVariantMap(aParameters, parameters);

    QByteArray array;
    QDataStream stream( &array, QIODevice::WriteOnly );

    QVariant var( parameters );
    stream << deviceDialogType;
    stream << var;

    return d->Show(array);
}

/*!
    Updates device dialog parameters by a set of new values. Show() must be called before an
    Update() can be called. Returns true on success and false
    if error occurred.

    \sa Show()
*/
EXPORT_C TInt CHbDeviceDialog::Update(const CHbSymbianVariantMap& aParameters)
{
    if(!d) {
        return KErrNotReady;
    }
    QVariantMap parameters;

    HbSymbianVariantConverter::toQtVariantMap(aParameters, parameters);

    QByteArray array;
    QDataStream stream( &array, QIODevice::WriteOnly );

    QVariant var( parameters );
    stream << var;

    return d->Update(array);
}

/*!
    Get the data received from device dialog if using synchronous Show
    in s60 data types. Caller gets the ownership.
*/
EXPORT_C CHbSymbianVariantMap* CHbDeviceDialog::ReceivedDataL() const
{
    CHbSymbianVariantMap* map = HbSymbianVariantConverter::fromQVariantMapL(d->iDataReceived);
    return map;
}

/*!
    Cancel device dialog session. Visible dialog is removed from the screen.
    Waiting dialogs are canceled and no effect if dialog already dismissed
*/
EXPORT_C void CHbDeviceDialog::Cancel()
{
    d->CancelDialog();
}

/*!
    Set observer for device dialog events. \aObserver is pointer to the
    observer. Null disables observing.
*/
EXPORT_C void CHbDeviceDialog::SetObserver(MHbDeviceDialogObserver* aObserver)
{
    d->SetObserver(aObserver);
}

CHbDeviceDialog::CHbDeviceDialog(TInt aFlags) : d(NULL)
{
  d = new CHbDeviceDialogPrivate;
  d->iFlags = aFlags;
  CActiveScheduler::Add(d);

  // Is needed to implement?
  //if (mDeviceDialogFlags & HbDeviceDialog::ImmediateResourceReservationFlag)
}
