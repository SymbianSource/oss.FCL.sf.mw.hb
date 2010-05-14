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

//  Include Files  

#include "hbthemeserveroogmplugin_p.h"	// CThemeSrvOogmPlugin



//  Member Functions

// -----------------------------------------------------------------------------
// C++ constructor.
// -----------------------------------------------------------------------------
//
CThemeSrvOogmPlugin::CThemeSrvOogmPlugin():oogmClient(0)
{
}

// -----------------------------------------------------------------------------
// Destructor.
// -----------------------------------------------------------------------------
//
CThemeSrvOogmPlugin::~CThemeSrvOogmPlugin()
{

    delete oogmClient;

}

// -----------------------------------------------------------------------------
// Symbian two-phased constructor (2nd phase).
// -----------------------------------------------------------------------------
//
void CThemeSrvOogmPlugin::ConstructL()
{
    CGOomMonitorPluginBase::ConstructL();
    oogmClient = new(ELeave) HbThemeClientOogm();
}


// -----------------------------------------------------------------------------
// Tries to free RAM.
// -----------------------------------------------------------------------------
//
void CThemeSrvOogmPlugin::FreeRam(TInt aBytesToFree)
{
    if(aBytesToFree > 0) {
        oogmClient->FreeRam(aBytesToFree);
    }
}

void CThemeSrvOogmPlugin::FreeRam(TInt aBytesToFree, TBool aUseSwRendering)
{
    oogmClient->FreeRam(aBytesToFree, aUseSwRendering);
}

// -----------------------------------------------------------------------------
// CThemeSrvOogmPlugin::MemoryGood
// -----------------------------------------------------------------------------
//
void CThemeSrvOogmPlugin::MemoryGood()
{
    oogmClient->GoodMemory();
}

void CThemeSrvOogmPlugin::MemoryGood(TBool aUsingSwRendering)
{
    oogmClient->GoodMemory(aUsingSwRendering);
}

// -----------------------------------------------------------------------------
// Creates plugin.
// -----------------------------------------------------------------------------
//
CThemeSrvOogmPlugin* CThemeSrvOogmPlugin::NewL()
{
    CThemeSrvOogmPlugin* plugin = new(ELeave) CThemeSrvOogmPlugin;
    CleanupStack::PushL(plugin);
    plugin->ConstructL();
    CleanupStack::Pop(plugin);
    return plugin;
}


