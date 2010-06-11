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

#include "hbdevicedialogsymbian.h"
#include "hbsymbianvariant.h"
#include "hbsymbianvariantconverter_p.h"
#include "hbdeleteguardsymbian_p.h"

/*!
 \class CHbDeviceDialogSymbian
 \brief CHbDeviceDialog displays dialogs on top of applications. It is a client interface for Symbian applications to Hb
    device dialogs.
 \sa HbDeviceDialog

 Data given to this API in symbian format is packed to QVariantMap. See
 CHbSymbianVariant to see which Qt datatypes are supported.
 \sa CHbSymbianVariant
 \sa CHbSymbianVariantMap

 When plugin returns data in Qt format, the data is converted, if possible,
 to CHbSymbianVariantMap.

 \stable

 \hbcore
 */

/*!
    \enum CHbDeviceDialogSymbian::TDeviceDialogError
    Defines device dialog error codes and ranges.
*/
/*!
    \var TDeviceDialogError::DeviceDialogError HbDeviceDialog::EFrameworkErrors
    Start of an error range for errors originating from device dialog framework (client or server).
*/
/*!
    \var TDeviceDialogError::DeviceDialogError HbDeviceDialog::EPluginErrors
    Start of an error range for errors originating from device dialog plugins. The framework passes
    these from the plugin unmodified.
*/
/*!
    \var TDeviceDialogError::DeviceDialogError HbDeviceDialog::EErrorTypeMask
    Mask for error type part of the error code.
*/
/*!
    \var TDeviceDialogError::DeviceDialogError HbDeviceDialog::ECancelledError
    Operation was cancelled by Cancel().
*/
/*!
    \var TDeviceDialogError::DeviceDialogError HbDeviceDialog::ESystemCancelledError
    Operation was cancelled by device dialog framework.
*/
/*!
    \var TDeviceDialogError::DeviceDialogError HbDeviceDialog::EInstanceExistsError
    A single instance device dialog widget exists already (has been launched).
    See HbDeviceDialogPlugin::SingleInstance.
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
    set in CHbDeviceDialogSymbian::Show the latest data can be retrieved with
    CHbDeviceDialogSymbian::receivedData()

    \a aCompletionCode gives the result of the dialog completion. Code can be
    either Symbian error code or device dialog error code.

    \sa DataReceived() ReceivedData()
*/

// Device dialogs are implemented only for Symbian/S60 OS.

class CHbDeviceDialogSymbianPrivate : public CActive
{
public:
        CHbDeviceDialogSymbianPrivate(TInt aFlags);
        ~CHbDeviceDialogSymbianPrivate();
        TInt Initialize();
        TInt Show(const QByteArray& aArray);
        TInt Update(const QByteArray& aArray);
        void CancelDialog();
        TInt Error() const;
        void SetObserver(MHbDeviceDialogObserver* aObserver);

        // CActive
        void RunL();
        void DoCancel();
        TInt RunError(TInt aError);

        void Start();
        TInt SymToDeviceDialogError(TInt errorCode);
        void SetError(TInt aError);
        bool CallDialogClosedObserver(TInt aCompletionCode);
        bool CallDataReceivedObserver(CHbSymbianVariantMap& aData);

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
    bool *iDeleted;
};

CHbDeviceDialogSymbianPrivate::CHbDeviceDialogSymbianPrivate(TInt aFlags):
CActive(EPriorityStandard),
iFlags(aFlags),
iDataPtr(NULL, 0, 0)
{
    CActiveScheduler::Add(this);
}

CHbDeviceDialogSymbianPrivate::~CHbDeviceDialogSymbianPrivate()
{
    // Inform the server to finish the dialog session and not to cancel it
    if (!iObserver) {
        iHbSession.SendSyncRequest(EHbSrvClientClosing);
    }

    Cancel();
    iHbSession.Close();

    delete iBuffer;
    iBuffer = NULL;

    iObserver= NULL;
    // Set object deleted flag
    if (iDeleted) {
        // Mark the object as deleted.
        *iDeleted = true;
        iDeleted = 0;
    }
}

TInt CHbDeviceDialogSymbianPrivate::Initialize()
{
    if (!iBuffer) {
        TRAP_IGNORE(iBuffer = HBufC8::NewL(64));
        if (iBuffer) {
            iDataPtr.Set(iBuffer->Des());
        } else {
            return KErrNoMemory;
        }
    }
    return iHbSession.Connect();
}

TInt CHbDeviceDialogSymbianPrivate::Show(const QByteArray& aArray)
{
    TInt error = iLastError = KErrNone;

    error = SymToDeviceDialogError(Initialize());
    if (error != HbDeviceDialogNoError){
        SetError(error);
        return error;
    }

    TPtrC8 ptr( reinterpret_cast<const TUint8*>(aArray.data()), aArray.size() );
    // Synchronous call to server to show dialog.
    error = iHbSession.SendSyncRequest(EHbSrvShowDeviceDialog, ptr, &iDeviceDialogId);
    error = SymToDeviceDialogError(error);

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
TInt CHbDeviceDialogSymbianPrivate::Update( const QByteArray& aArray )
{
    TInt error = iLastError = KErrNone;
    if (iRequesting) {

        TPtrC8 ptr( reinterpret_cast<const TUint8*>(aArray.data()), aArray.size() );

        error = iHbSession.SendSyncRequest( EHbSrvUpdateDeviceDialog, ptr );
        error = SymToDeviceDialogError(error);
        if (error != KErrNone) {
            SetError(error);
        }
    }
    else {
        error = SymToDeviceDialogError(KErrBadHandle);
        SetError(error);
    }
    return error;
}

/*!
    \internal

    Cancel a scheduled popup on HbDeviceDialogManager. Event buffer is cleared
    at server.
*/
void CHbDeviceDialogSymbianPrivate::CancelDialog()
{
    iLastError = KErrNone;
    int error = SymToDeviceDialogError(KErrNotFound);

    if (iRequesting) {
        // Ignore other than server errors.
        error = iHbSession.SendSyncRequest(EHbSrvCancelDeviceDialog, iDeviceDialogId());
        error = SymToDeviceDialogError(error);
    }
    if (error != KErrNone) {
        SetError(error);
    }
}

/*!
    \internal

    Return last error.
*/
TInt CHbDeviceDialogSymbianPrivate::Error() const
{
    return iLastError;
}

void CHbDeviceDialogSymbianPrivate::SetObserver( MHbDeviceDialogObserver* aObserver )
{
    iObserver = aObserver;
}
/*!
    \internal
    RunL from CActive.
*/
void CHbDeviceDialogSymbianPrivate::RunL()
{
    TInt completionCode = iStatus.Int();
    int errorCode = SymToDeviceDialogError(completionCode);

    if (completionCode < KErrNone) {
        // Any Symbian error, stop requesting, sycnhoronous requests are stopped
        // in the end of the RunL
        iRequesting = EFalse;
        SetError(errorCode);
        if (CallDialogClosedObserver(errorCode)) {
            return; // observed deleted this object, do not touch it
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
                    errorCode = SymToDeviceDialogError(completionCode);

                    // data request failed
                    if (completionCode < KErrNone) {
                        iRequesting = EFalse;
                        SetError(errorCode);
                        if (CallDialogClosedObserver(errorCode)) {
                            return; // observed deleted this object, do not touch it
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

                    if (iObserver) {
                        CHbSymbianVariantMap* symbianMap =
                            HbSymbianVariantConverter::fromQVariantMapL(varMap);
                        bool thisIsDeleted = CallDataReceivedObserver(*symbianMap);
                        delete symbianMap;
                        symbianMap = 0;
                        if (thisIsDeleted) { // observer deleted this, do not touch anymore
                            return;
                        }
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
                    SetError(errorCode);
                }
                iRequesting = EFalse;
                if (CallDialogClosedObserver(errorCode)) {
                    return; // observed deleted this object, do not touch it
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
}

/*!
    \internal
    DoCancel from CActive.
*/
void CHbDeviceDialogSymbianPrivate::DoCancel()
{
    SetError(KErrCancel);
    iRequesting = EFalse;
    iHbSession.SendSyncRequest(EHbSrvCancelUpdateChannel);
}

/*!
    \internal
    RunError from CActive.
*/
TInt CHbDeviceDialogSymbianPrivate::RunError(TInt /*aError*/)
{
    SetError(SymToDeviceDialogError(KErrGeneral));
    return KErrNone;
}

/*!
    \internal
    Starts asynchronous message to receive update and close events from session.
*/
void CHbDeviceDialogSymbianPrivate::Start()
{
    iDataPtr.Zero();

    if ( !IsActive() ) {
        iHbSession.SendASyncRequest( EHbSrvOpenUpdateChannel, iDataPtr, iUpdateInfo, iStatus );
        SetActive();
        iRequesting = ETrue;
    }
}

// Convert symbian error code into HbDeviceDialog error code
int CHbDeviceDialogSymbianPrivate::SymToDeviceDialogError(TInt errorCode)
{
    if (errorCode != HbDeviceDialogNoError) {
        // Convert from internal to public error code
        if (errorCode == HbDeviceDialogAlreadyExists) {
            errorCode = CHbDeviceDialogSymbian::EInstanceExistsError;
        }
    }
    return errorCode;
}

void CHbDeviceDialogSymbianPrivate::SetError( TInt aError )
{
    iLastError = aError;
}

// Call device dialog closed observer. Return true if observer deleted this object.
bool CHbDeviceDialogSymbianPrivate::CallDialogClosedObserver(TInt aCompletionCode)
{
    if (iObserver) {
        RHbDeleteGuardSymbian guard;
        guard.OpenAndPushL(&iDeleted);
        iObserver->DeviceDialogClosed(aCompletionCode);
        return guard.PopAndClose();
    } else {
        return false;
    }
}

// Call device dialog data received observer. Return true if observer deleted this object.
bool CHbDeviceDialogSymbianPrivate::CallDataReceivedObserver(CHbSymbianVariantMap& aData)
{
    if (iObserver) {
        RHbDeleteGuardSymbian guard;
        guard.OpenAndPushL(&iDeleted);
        iObserver->DataReceived(aData);
        return guard.PopAndClose();
    } else {
        return false;
    }
}

/*!
    Constructs CHbDeviceDialogSymbian object. \a f contains construct flags.
*/
EXPORT_C CHbDeviceDialogSymbian* CHbDeviceDialogSymbian::NewL(TInt aFlags)
{
     CHbDeviceDialogSymbian* self = new (ELeave) CHbDeviceDialogSymbian;
     CleanupStack::PushL(self);
     self->d = new (ELeave) CHbDeviceDialogSymbianPrivate(aFlags);

     if (aFlags & EImmediateResourceReservation) {
         User::LeaveIfError(self->d->Initialize());
     }
     CleanupStack::Pop(); // self
     return self;
}

CHbDeviceDialogSymbian::CHbDeviceDialogSymbian()
{
}

EXPORT_C CHbDeviceDialogSymbian::~CHbDeviceDialogSymbian()
{
    delete d;
}

/*!
  Show of device dialog. Each time a Show() is called a new dialog is launched.
  aParameter data is sent to device dialog in Qt's QVariantMap object.
  The function is asynchronous and returns immediately. Closing and data events from the
  dialog can be received by observer interface. Deleting CHbDeviceDialogSymbian object
  closes and deletes the device dialog at server if observer is set. If observer is null
  the dialog is left executing at the server and it's assumed it closes itself by a timeout.
  Cancel() to closes the device dialog.

  \a aDeviceDialogType is name of the device dialog. Identifies the device
  dialog plugin. \a aParameters is a buffer containing data for the device dialog.

  \a aObserver is used to observe the session.

  Return value informs if the call was successful.

  \sa Update() Cancel()
 */
EXPORT_C TInt CHbDeviceDialogSymbian::Show(const TDesC& aDeviceDialogType, const CHbSymbianVariantMap& aParameters, MHbDeviceDialogObserver* aObserver)
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
EXPORT_C TInt CHbDeviceDialogSymbian::Update(const CHbSymbianVariantMap& aParameters)
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
EXPORT_C CHbSymbianVariantMap* CHbDeviceDialogSymbian::ReceivedDataL() const
{
    CHbSymbianVariantMap* map = HbSymbianVariantConverter::fromQVariantMapL(d->iDataReceived);
    return map;
}

/*!
    Cancel device dialog session. Visible dialog is removed from the screen.
    Waiting dialogs are canceled and no effect if dialog already dismissed
*/
EXPORT_C void CHbDeviceDialogSymbian::Cancel()
{
    d->CancelDialog();
}

/*!
    Set observer for device dialog events. \aObserver is pointer to the
    observer. Null disables observing.
*/
EXPORT_C void CHbDeviceDialogSymbian::SetObserver(MHbDeviceDialogObserver* aObserver)
{
    d->SetObserver(aObserver);
}
