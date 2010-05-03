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

#include "hbthemeclientoogm_p.h"

const TUint KDefaultMessageSlots=4;


/*! 
* Constructor
*/
HbThemeClientOogm::HbThemeClientOogm(): connected(false)
{
}


/*! 
* Connects to the  server using 4 message slots.
* returns true if connection succeeds.
*/
bool HbThemeClientOogm::ConnectToServer()
{
    
    TInt error = CreateSession(KThemeServerName, Version(), KDefaultMessageSlots);

    if (KErrNone != error) {
        RDebug::Print(_L("ThemeServerPlugin : Server Connection request failed"));
        return false;
    }
    connected = true;
    return connected; 
}


/*!
* Returns the earliest version number of the server that we can talk to.
*/
TVersion HbThemeClientOogm::Version(void) const
{
    return(TVersion(KThemeServerMajorVersionNumber, KThemeServerMinorVersionNumber, KThemeServerBuildVersionNumber));
}


/*!
* Closing the server and tidying up.
*/
void HbThemeClientOogm::Close()
{
    RSessionBase::Close();
}

/*!
* Sends FreeRam request to HbThemeserver with the bytes to be freed.
* see goomconfig.xml as what is the threshold that triggers a call to this API.
*/

void HbThemeClientOogm::FreeRam(int bytes)
{
    if (!connected) {
        ConnectToServer();
    }
        
    if (connected) {
        TPckg<TInt> bytesToFree(bytes);
        TIpcArgs args(&bytesToFree, 0);

        TInt err = SendReceive(EFreeRam, args);
        if (KErrNone != err) {
            RDebug::Print(_L("ThemeServerPlugin: Free RAM notification sent to ThemeServer"));
        }
    }
}

/*!
* Sends MemoryGood request to HbThemeserver.
* see goomconfig.xml as what is the threshold that triggers a call to this API.
*/

void  HbThemeClientOogm::GoodMemory()
{
    if (!connected) {
        ConnectToServer();
    }

    if (connected) {
        TInt err = SendReceive(EMemoryGood);
        if (KErrNone != err) {
            RDebug::Print(_L("ThemeServerPlugin: Good Memory notification sent to ThemeServer"));
        }
    }
}

