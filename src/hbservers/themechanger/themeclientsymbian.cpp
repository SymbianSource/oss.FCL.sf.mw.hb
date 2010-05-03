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

#include "themeclientsymbian.h"
#include <e32property.h>
#include <QDebug>


const TUint kDefaultMessageSlots=4;


/**
 * Constructor
 */
ThemeClientSymbian::ThemeClientSymbian(): connected(false)
{
}


/**
 * Connects to the  server using 4 message slots.
 */
bool ThemeClientSymbian::connectToServer()
{
    qDebug() << "HbSymbianThemeClient::Connect ++";

    TInt error = CreateSession(KThemeServerName,Version(),kDefaultMessageSlots);

    if (KErrNone != error) {
        qDebug() << "ThemeClientSymbian:: could not create session" << error;
    }
    connected = (KErrNone == error);
    return (KErrNone == error); 
}


/**
 * Returns the earliest version number of the server that we can tal to.
 */  
TVersion ThemeClientSymbian::Version(void) const
{
    return(TVersion(KThemeServerMajorVersionNumber,KThemeServerMinorVersionNumber,KThemeServerBuildVersionNumber));
}


/**
 * Closing the server and tidying up.
 */
void ThemeClientSymbian::Close()
{
    RSessionBase::Close();
}

/**
 * changeTheme
 */
TInt ThemeClientSymbian::changeTheme(const QString& aString )
{
    TInt err = KErrGeneral;
    RProperty themeRequestProp;
    
    User::LeaveIfError( themeRequestProp.Attach( KServerUid3, KNewThemeForThemeChanger ) );
    
    TBuf<256> newThemenameChangeRequest;
    _LIT(KThemeRequestFormatter, "%d:%S");
    TBuf<256> newThemename(aString.utf16());
    newThemenameChangeRequest.Format( KThemeRequestFormatter, EThemeSelection, &newThemename);
    err = themeRequestProp.Set(newThemenameChangeRequest);
    themeRequestProp.Close();
    return err;
}

bool ThemeClientSymbian::isConnected()
{
    return connected;
}
