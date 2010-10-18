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
#include "hbdevicedialoglaunchhelper_p.h"
#include "hbdevicedialogsymbian.h"
#include "hbsymbianvariant.h"
#include "hbsymbianvariantconverter_p.h"
#include "hbdeleteguardsymbian_p.h"

/*!
    \stable
    \hbcore

    \class CHbDeviceDialogSymbian
    \brief CHbDeviceDialogSymbian is a Symbian implementation of HbDeviceDialog. 

    <b>This class is Symbian only. Not available on other platforms.</b>

    See HbDeviceDialog documentation to find out more about device dialogs.

    CHbDeviceDialogSymbian is intended for use by Symbian servers that don't run Qt event loop
    and cannot use HbDeviceDialog.

    The class is accompanied by classes CHbSymbianVariant and CHbSymbianVariantMap which
    are used to encapsulate device dialog parameters. Device dialog framework takes care of
    translating Symbian data types to/from format understood by device dialog plugins.

    CHbDeviceDialogSymbian provides an asynchronous interface to show dialogs. Dialog closing
    and data events are indicated by MHbDeviceDialogObserver callback interface. 

    The code below launches a device dialog. Observer is not set and the dialog
    closes itself by timeout.
    \code

    CHbDeviceDialogSymbian* dialog = CHbDeviceDialogSymbian::NewL();
    CleanupStack::PushL(dialog);
    CHbSymbianVariantMap* map = CHbSymbianVariantMap::NewL();
    CleanupStack::PushL(map);

    _LIT(KTimeoutKey, "timeout");
    const TInt timeoutValue = 10000;
    CHbSymbianVariant* timeout = CHbSymbianVariant::NewL(&timeoutValue, CHbSymbianVariant::EInt);
    CleanupStack::PushL(timeout);
    User::LeaveIfError(map->Add(KTimeoutKey, timeout));
    CleanupStack::Pop(); // timeout

    _LIT(KTitleValue, "Sample title");
    _LIT(KTitleKey, "title");
    CHbSymbianVariant* title = CHbSymbianVariant::NewL(&KTitleValue(), CHbSymbianVariant::EDes);
    CleanupStack::PushL(title);
    User::LeaveIfError(map->Add(KTitleKey, title));
    CleanupStack::Pop(); // title

    _LIT(KTextValue, "Sample text");
    _LIT(KTextKey, "text");
    CHbSymbianVariant* text = CHbSymbianVariant::NewL(&KTextValue(), CHbSymbianVariant::EDes);
    CleanupStack::PushL(text);
    User::LeaveIfError(map->Add(KTextKey, text));
    CleanupStack::Pop(); // text

    _LIT(KDeviceDialogType, "com.nokia.hb.devicenotificationdialog/1.0");
    dialog->Show(KDeviceDialogType, *map);
    CleanupStack::PopAndDestroy(2); // dialog, map
    \endcode

    \sa HbDeviceDialog, CHbSymbianVariant, CHbSymbianVariantMap, MHbDeviceDialogObserver
 */

/*!
    \enum CHbDeviceDialogSymbian::TDeviceDialogError
    Defines device dialog error codes and ranges.
*/
/*!
    \var CHbDeviceDialogSymbian::TDeviceDialogError CHbDeviceDialogSymbian::EFrameworkErrors
    Start of an error range for errors originating from device dialog framework (client or server).
*/
/*!
    \var CHbDeviceDialogSymbian::TDeviceDialogError CHbDeviceDialogSymbian::EPluginErrors
    Start of an error range for errors originating from device dialog plugins. The framework passes
    these from the plugin unmodified.
*/
/*!
    \var CHbDeviceDialogSymbian::TDeviceDialogError CHbDeviceDialogSymbian::EErrorTypeMask
    Mask for error type part of the error code.
*/
/*!
    \var CHbDeviceDialogSymbian::TDeviceDialogError CHbDeviceDialogSymbian::ECancelledError
    Operation was cancelled by Cancel().
*/
/*!
    \var CHbDeviceDialogSymbian::TDeviceDialogError CHbDeviceDialogSymbian::ESystemCancelledError
    Operation was cancelled by device dialog framework.
*/
/*!
    \var CHbDeviceDialogSymbian::TDeviceDialogError CHbDeviceDialogSymbian::EInstanceExistsError
    A single instance device dialog widget exists already (has been launched).
    See HbDeviceDialogPlugin::SingleInstance.
*/

/*!
    \enum CHbDeviceDialogSymbian::TDeviceDialogFlag
    Defines construct flags.
*/
/*!
    \var CHbDeviceDialogSymbian::TDeviceDialogFlag CHbDeviceDialogSymbian::ENoDeviceDialogFlags
    No flags specified.
*/
/*!
    \var CHbDeviceDialogSymbian::TDeviceDialogFlag CHbDeviceDialogSymbian::EImmediateResourceReservation
    Reserves resources immediately instead of delaying until Show() is called.
*/
/*!
    \var CHbDeviceDialogSymbian::TDeviceDialogFlag CHbDeviceDialogSymbian::EASyncServerStartup
    Starts device dialog service asynchronously.
*/

/*!
   \fn void MHbDeviceDialogObserver::DataReceived(CHbSymbianVariantMap& aData)

    This callback is called when data is received from a device dialog.
    \a aData contains data from the dialog plugin. The structure and meaning
    of the data is a contract between the dialog and a client.

    If no observer is set in CHbDeviceDialogSymbian::Show() the latest data
    can be retrieved with CHbDeviceDialogSymbian::receivedData().

    \sa CHbSymbianVariantMap
*/

/*!
    \fn void MHbDeviceDialogObserver::DeviceDialogClosed(TInt aCompletionCode)

    This callback is called when a device dialog is closed. Any data sent by
    the dialog is indicated by the dataReceived() callback.

    \a aCompletionCode gives the result of the dialog completion. Code can be
    either Symbian error code or device dialog error code.

    \sa DataReceived(), ReceivedData()
*/

NONSHARABLE_CLASS(CHbDeviceDialogSymbianPrivate) : public CActive
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

    TInt error(KErrNone);
    if (iFlags & CHbDeviceDialogSymbian::EASyncServerStartup) {
        HbDeviceDialogLaunchHelper *helper(0);
        TRAP(error, helper = HbDeviceDialogLaunchHelper::NewL());
        
        if (helper) {            
            helper->Start();
            error = helper->Error();
            delete helper;
            helper = 0;
            }        
    }

    if (error == KErrNone || error == KErrAlreadyExists) {
        error = iHbSession.Connect();
    }
    return error;
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

// Send device dialog update.
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

// Cancel a scheduled popup on HbDeviceDialogManager. Event buffer is cleared at server.
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

// Return last error.
TInt CHbDeviceDialogSymbianPrivate::Error() const
{
    return iLastError;
}

void CHbDeviceDialogSymbianPrivate::SetObserver( MHbDeviceDialogObserver* aObserver )
{
    iObserver = aObserver;
}

// RunL from CActive.
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

// DoCancel from CActive.
void CHbDeviceDialogSymbianPrivate::DoCancel()
{
    SetError(KErrCancel);
    iRequesting = EFalse;
    iHbSession.SendSyncRequest(EHbSrvCancelUpdateChannel);
}

// RunError from CActive.
TInt CHbDeviceDialogSymbianPrivate::RunError(TInt /*aError*/)
{
    SetError(SymToDeviceDialogError(KErrGeneral));
    return KErrNone;
}

// Starts asynchronous message to receive update and close events from session.
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
    Constructs CHbDeviceDialogSymbian and returns a pointer to it.
    \a f contains construct flags.
*/
EXPORT_C CHbDeviceDialogSymbian* CHbDeviceDialogSymbian::NewL(TInt aFlags)
{
     CHbDeviceDialogSymbian* self = new (ELeave) CHbDeviceDialogSymbian;
     CleanupStack::PushL(self);
     self->d = new (ELeave) CHbDeviceDialogSymbianPrivate(aFlags);

     if (aFlags & EImmediateResourceReservation) {
         User::LeaveIfError(self->d->Initialize());
     }
     CleanupStack::Pop(self);
     return self;
}

/*!
    Constructs CHbDeviceDialogSymbian.
*/
CHbDeviceDialogSymbian::CHbDeviceDialogSymbian()
{
}

/*!
    Destructs CHbDeviceDialogSymbian. The dialog launched by ShowL()
    is closed if observer is set. If there is no observer, the dialog is
    left executing and should close itself by timeout.
*/
EXPORT_C CHbDeviceDialogSymbian::~CHbDeviceDialogSymbian()
{
    delete d;
}

/*!
    Shows a device dialog. Each time a Show() is called, a new dialog is launched.
    The function is asynchronous and returns after the service has accepted the dialog.
    Closing and data events from the dialog are indicated by observer interface.
    Deleting CHbDeviceDialogSymbian object closes and deletes the device dialog at server
    if observer is set. If no observer is set, the dialog is left executing at the server
    and it's assumed it closes itself by a timeout. Cancel() closes the device dialog.

    \param aDeviceDialogType Identifies a device dialog.
    \param aParameters Contains parameters for the device dialog.
    \param aObserver Contains observer for the dialog or 0 for no observer.

    \return returns 0 on success or error code.

    \sa Update(), Cancel()
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
    Update() can be called.

    \param aParameters Contains parameters for the device dialog.
    
    \return returns 0 on success or error code.

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
    Gets the latest data received from a device dialog. If observer is set, data received from
    device dialog is not saved and an empty container is returned.

    \return returns data received from device dialog. Ownership is transferred to caller.
*/
EXPORT_C CHbSymbianVariantMap* CHbDeviceDialogSymbian::ReceivedDataL() const
{
    CHbSymbianVariantMap* map = HbSymbianVariantConverter::fromQVariantMapL(d->iDataReceived);
    return map;
}

/*!
    Cancels a device dialog. Visible dialog is closed and a queued one is deleted.
    No effect if dialog has already closed.
*/
EXPORT_C void CHbDeviceDialogSymbian::Cancel()
{
    d->CancelDialog();
}

/*!
    Sets observer for device dialog events.

    \param aObserver Contains observer for the device dialog. 0 disables observing.
*/
EXPORT_C void CHbDeviceDialogSymbian::SetObserver(MHbDeviceDialogObserver* aObserver)
{
    d->SetObserver(aObserver);
}
