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

/*!
    \class MHbIndicatorSymbianObserver
    \brief MHbIndicatorSymbianObserver is an observer interface for observing CHbIndicatorSymbian.
*/

/*!
    \fn void MHbIndicatorSymbianObserver::IndicatorUserActivated(const TDesC& aType, CHbSymbianVariantMap& aData)

    This callback is called when user has interacted with an indicator on the indicator
    menu.
    
    \a aType - Type of the indicator that user interacted with.
    \a aData - Data sent by the indicator.
*/

/*!
    \class CHbIndicatorSymbian
    \brief CHbIndicatorSymbian can be used to activate and deactivate indicators. It is a client 
    interface for Symbian applications to Hb indicators.

    CHbIndicatorSymbian sends a request for indicator activation and deactivation to
    server side. Indicators are identified by their type-string and there must be a
    server side indicator implementation for that type.

    Active indicators may appear in the status indicator area as an icon,
    and/or inside universal indicator menu showing icon and text.
    Depending on the indicator implementation, activated indicator may also show up with
    a notification dialog and some indicators can be interacted by the user in universal indicator menu.

    User can interact with an indicator from the indicator menu. Client is notified about
    the user interaction via MHbIndicatorSymbianObserver observer interface. Interaction 
    notification and data sent by the indicator is a contract between HbIndicator class 
    and indicator.
    
    When deactivated, icons are removed from the status
    indicator area and in universal indicator menu.

    \sa HbIndicator

    \stable
    \hbcore
*/

/*!
    \enum CHbIndicatorSymbian::TIndicatorError
    Defines the indicator errors.
*/
/*!
    \var CHbIndicatorSymbian::TIndicatorError CHbIndicatorSymbian::EFrameworkErrors
    Start of an error range for errors originating from device dialog framework.
    Error codes are defined in hbdevicedialogerrors.h
*/
/*!
    \var CHbIndicatorSymbian::TIndicatorError CHbIndicatorSymbian::EPluginErrors
    Start of an error range for errors originating from indicator plugins.
*/
/*!
    \var CHbIndicatorSymbian::TIndicatorError CHbIndicatorSymbian::EErrorTypeMask
    Mask for error type part of the error code.
*/

#include <e32base.h>
#include <QVariant>

#include "hbindicatorsymbian.h"
#include "hbsymbianvariant.h"
#include "hbsymbianvariantconverter_p.h"
#include "hbdevicedialogclientsession_p.h"
#include "hbdevicedialogerrors_p.h"
#include "hbdevicedialogserverdefs_p.h"

class CHbIndicatorSymbianPrivate : public CActive {
public:
    CHbIndicatorSymbianPrivate() : CActive(EPriorityStandard), iInitialized(EFalse), iLastError(HbDeviceDialogNoError),   iMsgTypePtr(NULL,0,0),
    		  iBuffer(NULL),
    		  iDataPtr(NULL,0,0) { CActiveScheduler::Add(this); }
    ~CHbIndicatorSymbianPrivate() {
    	Cancel();
        if (iInitialized) {
            iHbSession.Close();
            iInitialized = EFalse;
        }
    }
    TBool Initialize();
    void SetError( int error )
    {
        iLastError = error;
    }
    bool sendActivateMessage(const TDesC& aIndicatorType,
            TBool activate, const CHbSymbianVariant* aParameter);

    void Start();
	    
protected:	
    // CActive
    void RunL();
    void DoCancel();
    TInt RunError( TInt aError );

public:
    RHbDeviceDialogClientSession iHbSession;
    TBool iInitialized;
    TInt iLastError;
    MHbIndicatorSymbianObserver* iObserver;
    
    TInt iMsgType;
    TPtr8 iMsgTypePtr;
    HBufC8* iBuffer;
    TPtr8 iDataPtr;
    TBool iRequesting;
};

TBool CHbIndicatorSymbianPrivate::Initialize()
{
    if (!iInitialized) { 
        TInt error = iHbSession.Connect();
        if ( error != KErrNone ) {
            SetError( HbDeviceDialogConnectError );
        } else {
            iInitialized = ETrue;
        }
    }
    return iInitialized;
}

bool CHbIndicatorSymbianPrivate::sendActivateMessage(const TDesC& aIndicatorType,
        TBool activate, const CHbSymbianVariant* aParameter)
{
    iLastError = HbDeviceDialogNoError;

    if (!Initialize()) {
        return false;
    }
    QString indicatorType = QString::fromUtf16(aIndicatorType.Ptr(), 
                                               aIndicatorType.Length());
    
    QVariant parameter;
    HbSymbianVariantConverter::toQtVariant(aParameter, parameter);
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
    int error = iHbSession.SendSyncRequest( message, ptr );

    bool result(true);

    if ( error != KErrNone ) {
        SetError( error );
        result = false;
    }
    
    if (activate && result && iObserver) {
		Start();
    }
       
    return result;
}

void CHbIndicatorSymbianPrivate::Start()
	{
	if (!IsActive() && !iRequesting) {
		SetActive();
		if (!iBuffer) {
			iBuffer = HBufC8::NewL( 256 );		
			iDataPtr.Set( iBuffer->Des() );
		}
		iDataPtr.Zero();
        TPckg<TInt> pckg( iMsgType );
        iMsgTypePtr.Set( pckg );
        iRequesting = ETrue;
		iHbSession.SendASyncRequest(EHbSrvGetActivatedIndicatorsStart, iDataPtr, iMsgTypePtr, iStatus);
    }       
       
}

void CHbIndicatorSymbianPrivate::RunL()
{
	TInt result = iStatus.Int();
		
	if (result < KErrNone) {
		SetError(result);
		iRequesting = EFalse;
	} else if (iMsgType == EHbIndicatorUserActivated && result >= 0) {
		iMsgType = -1;
		if (result > 0) {		
			delete iBuffer;
			iBuffer = NULL;
			iBuffer = HBufC8::NewL(result);
			iDataPtr.Set(iBuffer->Des());
			TInt error = iHbSession.SendSyncRequest(EHbSrvActivatedIndicatorData, iDataPtr, &iMsgTypePtr);							                
		}
	
		QByteArray resArray((const char*)iDataPtr.Ptr(), iDataPtr.Size());
		QDataStream stream(&resArray, QIODevice::ReadOnly);		
	
		QVariant var;
		stream >> var;
		QVariantMap varMap = var.toMap();
	
		if (iObserver) {
			QString type = varMap.value("type").toString();
			TPtrC descriptor(static_cast<const TUint16*>(type.utf16()),
								type.length());
			QVariantMap data = varMap.value("data").toMap();
			
			CHbSymbianVariantMap* symbianMap =
				HbSymbianVariantConverter::fromQVariantMapL(data);
			
			iObserver->IndicatorUserActivated(descriptor, *symbianMap);
			delete symbianMap;
			symbianMap = 0;
		}
	}
    // Make a new request if there were no errors.
    if ( result != KErrServerTerminated && result != KErrCancel && iRequesting) {        
		SetActive();
		iHbSession.SendASyncRequest(EHbSrvGetActivatedIndicatorContinue, iDataPtr, iMsgTypePtr, iStatus);
    }
}

void CHbIndicatorSymbianPrivate::DoCancel()
{
	if (iRequesting) {
		iHbSession.SendSyncRequest(EhbSrvGetActivatedIndicatorsClose);
		iRequesting = EFalse;    
	}
}

TInt CHbIndicatorSymbianPrivate::RunError( TInt aError )
	{
	SetError(aError);
	return KErrNone;
	}

EXPORT_C CHbIndicatorSymbian* CHbIndicatorSymbian::NewL()
    {
    CHbIndicatorSymbian *me = new CHbIndicatorSymbian();
    me->ConstructL();
    return me;
    }

EXPORT_C CHbIndicatorSymbian::~CHbIndicatorSymbian()
{
    delete d;
}

/*!
    Activates an indicator of type \a aIndicatorType.
    An extra parameter can be passed along to the indicator plugin.
    Returns true, if the indicator is activated, false, if an error occurred.  

    \sa Deactivate
 */
EXPORT_C TBool CHbIndicatorSymbian::Activate(const TDesC& aIndicatorType, const CHbSymbianVariant* aParameter)
{
    return d->sendActivateMessage(aIndicatorType, ETrue, aParameter);
}

/*!
    Deactivates an indicator of type \a indicatorType.
    An extra parameter can be passed along to the indicator plugin.
    If indicator is not currently active, does nothing and returns true.
    Returns false, if error occurred.

    \sa Activate
 */
EXPORT_C TBool CHbIndicatorSymbian::Deactivate(const TDesC& aIndicatorType, const CHbSymbianVariant* aParameter)
{
    return d->sendActivateMessage(aIndicatorType, EFalse, aParameter);
}

/*!
    Returns the last error code. The code is cleared when any other API function than error() is called.
*/
EXPORT_C TInt CHbIndicatorSymbian::Error() const
{
    return d->iLastError;
}

EXPORT_C void CHbIndicatorSymbian::SetObserver(MHbIndicatorSymbianObserver* aObserver)
{
	d->iObserver = aObserver;
}

CHbIndicatorSymbian::CHbIndicatorSymbian()
{
}

void CHbIndicatorSymbian::ConstructL()
{
    d = new (ELeave) CHbIndicatorSymbianPrivate();
}
