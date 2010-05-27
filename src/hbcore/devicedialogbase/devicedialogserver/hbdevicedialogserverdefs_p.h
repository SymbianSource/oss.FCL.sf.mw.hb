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

#ifndef HBDEVICEDIALOGSERVERDEFS_P_H
#define HBDEVICEDIALOGSERVERDEFS_P_H

//  INCLUDES
#include <e32base.h>

// CONSTANTS
// ----------------------------------------------------------------------------------------
// Server's policy
// ----------------------------------------------------------------------------------------
const TUint KHbDeviceDialogServerRangeCount = 1;

const TInt KHbDeviceDialogServerRanges[KHbDeviceDialogServerRangeCount] =
    {
    0  // 1st range messages 0 - 4
    };

const TUint8 KHbDeviceDialogServerElementsIndex[KHbDeviceDialogServerRangeCount] =
    {
    CPolicyServer::EAlwaysPass
    };

const CPolicyServer::TPolicyElement KHbDeviceDialogServerElements[] =
    {
    {_INIT_SECURITY_POLICY_PASS}
    };

const CPolicyServer::TPolicy KHbDeviceDialogServerPolicy =
    {
    CPolicyServer::EAlwaysPass, //specifies all connect attempts should pass
    KHbDeviceDialogServerRangeCount,
    KHbDeviceDialogServerRanges,
    KHbDeviceDialogServerElementsIndex,
    KHbDeviceDialogServerElements
    };

_LIT(KHbServerProcessName, "HbServer");
_LIT(KHbServerName,"!HbServer");    // add exclamation mark for protected server name
_LIT(KHbServerExe,"hbdevicedialogappserver.exe");

const TUint KHbServerMajor=1;
const TUint KHbServerMinor=0;
const TUint KHbServerBuild=0;

// CONSTANTS
const TInt KSlot0 = 0;
const TInt KSlot1 = 1;
const TInt KSlot2 = 2;
const TInt KSlot3 = 3;

// Commands for device dialogs
enum THbSrvCommands
    {
    EHbSrvShowDeviceDialog = 1,
    EHbSrvUpdateDeviceDialog,
    EHbSrvUpdateData,
    EHbSrvCancelDeviceDialog,
    EHbSrvClientClosing,
    EHbSrvCancelUpdateChannel,
    EHbSrvOpenUpdateChannel,
    EHbSrvPublishOrientation,

    //indicator commands
    EHbSrvIndicatorCommandsStart,
    EHbSrvActivateIndicator,
    EHbSrvDeactivateIndicator,
    EHbSrvGetActivatedIndicatorsStart,
    EHbSrvGetActivatedIndicatorContinue,
    EhbSrvGetActivatedIndicatorsClose,
    EHbSrvActivatedIndicatorData,
    EHbSrvIndicatorCommandsEnd
    };

enum THbDeviceDialogSrvUpdateType
    {
    EHbDeviceDialogUpdateCancel = 0,
    EHbDeviceDialogUpdateData,
    EHbDeviceDialogUpdateClosed
    };

enum THbSrvIndicatorChannelData
    {
    EHbIndicatorAllActivated = 0,
    EHbIndicatorUpdates,
    EHbIndicatorUserActivated
    };

struct THbDeviceDialogSrvUpdateInfo {
    THbDeviceDialogSrvUpdateType iUpdateType;
    TInt iDeviceDialogId;
    union {
        struct {
            TInt iDataSize;
        } iDataInfo;
    } iInfo;
    THbDeviceDialogSrvUpdateInfo()
        {iUpdateType = EHbDeviceDialogUpdateCancel; iDeviceDialogId = 0;}
    THbDeviceDialogSrvUpdateInfo(THbDeviceDialogSrvUpdateType aType, TInt aId)
        {iUpdateType = aType; iDeviceDialogId = aId;}
    };

#endif // HBDEVICEDIALOGSERVERDEFS_P_H

// End of File
