/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbServers module of the UI Extensions for Mobile.
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

#include "hbthemewatcher_symbian_p.h"
#include "hbthemeserver_symbian_p_p.h"
#include "hbthemeutils_p.h"

#include <QFile>
#include <e32property.h>
#include <e32base.h>
#include <e32svr.h>

// Publish/Subscribe themeRequestProp specific
static _LIT_SECURITY_POLICY_PASS(KAllowAllPolicy);
static _LIT_SECURITY_POLICY_C1(KThemeChangerPolicy,ECapabilityWriteDeviceData);

CHbThemeWatcher::CHbThemeWatcher(HbThemeServerPrivate& aObserver) : CActive(EPriorityStandard), 
                                                                    iObserver(aObserver)
{ 
    CActiveScheduler::Add(this); 
}

void CHbThemeWatcher::ConstructL()
{
    User::LeaveIfError(iFs.Connect());
}

CHbThemeWatcher* CHbThemeWatcher::NewL(HbThemeServerPrivate& aObserver)
{
    CHbThemeWatcher* self = new (ELeave) CHbThemeWatcher(aObserver);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
}

CHbThemeWatcher::~CHbThemeWatcher()
{
    Cancel();
    iFs.Close();
}

void CHbThemeWatcher::startWatchingL(const QString &file)
{ 
    // Cancel ongoing watch
    if (IsActive()) {
        Cancel();
    }
    iFile = file;

    TBuf<256> fileToWatch(iFile.utf16());
    iFs.NotifyChange(ENotifyAll, iStatus, fileToWatch); 
    SetActive();
}

void CHbThemeWatcher::RunL()
{
    if (iStatus != KErrNone) {
        return;
    }
    
    if (QFile::exists(iFile)) {
        // theme exists continue watching
        TBuf<256> fileToWatch(iFile.utf16());
        iFs.NotifyChange(ENotifyAll, iStatus, fileToWatch);
        SetActive();
        return;
    }

    // theme doesn't exist, change active theme to default
    iObserver.HandleThemeSelection(HbThemeUtils::defaultTheme().name);
}
 
void CHbThemeWatcher::DoCancel()
{
    iFs.NotifyChangeCancel(iStatus);
}

CHbThemeChangeNotificationListener* CHbThemeChangeNotificationListener::NewL(HbThemeServerPrivate& aObserver)
{
    CHbThemeChangeNotificationListener* self = new (ELeave) CHbThemeChangeNotificationListener(aObserver);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
}

CHbThemeChangeNotificationListener::CHbThemeChangeNotificationListener(HbThemeServerPrivate& aObserver)
    :CActive(EPriorityStandard),iObserver(aObserver)
{

}

void CHbThemeChangeNotificationListener::ConstructL()
{
    TInt err = RProperty::Define( KServerUid3, KNewThemeForThemeChanger, RProperty::ELargeText, KAllowAllPolicy, KThemeChangerPolicy );
     if ( err != KErrAlreadyExists ) {
         User::LeaveIfError( err );
     }
    err = themeRequestProp.Attach(KServerUid3, KNewThemeForThemeChanger );
    User::LeaveIfError(err);

    CActiveScheduler::Add(this);
}

CHbThemeChangeNotificationListener::~CHbThemeChangeNotificationListener()
{
    stopListening();
}

void CHbThemeChangeNotificationListener::startListeningL()
{
    if (IsActive()) {
         return; //do nothing if already listening
    }

    User::LeaveIfError(themeRequestProp.Attach(KServerUid3,KNewThemeForThemeChanger));
    //Subscribe for updates
    themeRequestProp.Subscribe(iStatus);

    SetActive();

}

void CHbThemeChangeNotificationListener::stopListening()
{
     Cancel(); // cancel
     if(IsActive()) { // only if already listening
        themeRequestProp.Close(); // Close the handle since it is not needed anymore
   }
}

/*
 * Returns TRUE if parsing succeeded, FALSE otherwise
 */
bool CHbThemeChangeNotificationListener::parseData(const TDesC& requestData, HbThemeServerRequest& etype, TDes& data)
{
    TInt result = 0;
    const TChar delimiter = ':';
    // initialize return value as failed
    bool bSuccess = false;

    result = requestData.Locate( delimiter );
    if( KErrNotFound != result ) {
        TInt len = requestData.Length();
        const TDesC& typestr = requestData.Mid(0, result);
        TLex atype(typestr);
        TInt iType;
        atype.Val( iType );
        etype = static_cast<HbThemeServerRequest>(iType);
        data.Copy( requestData.Mid( result + 1, len - result - 1 ) );
        bSuccess = true;
    } else {
        bSuccess = false;
    }

    return bSuccess;
}

static const TInt KThemeChangeDataBufferSize = 256;

void CHbThemeChangeNotificationListener::RunL()
{
    // Subscribe first to make sure we don't miss any
    // when handling this one.
    themeRequestProp.Subscribe(iStatus);

    SetActive();

    TBuf<KThemeChangeDataBufferSize> requestData;
    TInt ret = themeRequestProp.Get(requestData);
    switch (ret) {
        case KErrNone:
            {
                QString qrequestData((QChar*)requestData.Ptr(),requestData.Length());
                HbThemeServerRequest etype = EInvalidServerRequest;
                TBuf<KThemeChangeDataBufferSize> data;
                ///Parse the data from the Publisher
                bool bSuccess = parseData( requestData, etype, data);
                if( bSuccess && EThemeSelection == etype) {
                    QString str((QChar*)data.Ptr(), data.Length());
                    str = str.trimmed();
                    iObserver.HandleThemeSelection( str );
                }
            }
            break;
        case KErrPermissionDenied:
            qDebug() << "KErrPermissionDenied";
            break;
        case KErrNotFound:
            qDebug() << "KErrNotFound";
            break;
        case KErrArgument:
            qDebug() << "KErrArgument";
            break;
        case KErrOverflow:
            qDebug() << "KErrOverflow";
            break;
    }
}

void CHbThemeChangeNotificationListener::DoCancel()
{
    themeRequestProp.Cancel();
}
