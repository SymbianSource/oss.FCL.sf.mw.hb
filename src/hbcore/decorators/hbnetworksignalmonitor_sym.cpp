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

#include "hbnetworksignalmonitor_sym_p.h"

CNetworkSignalMonitor::~CNetworkSignalMonitor()
{
    Cancel();
    delete iTelephony;
}
 
void CNetworkSignalMonitor::ConstructL()
{
    iTelephony = CTelephony::NewL();   
    iTelephony->GetSignalStrength(iStatus, iSigStrengthV1Pckg);
    SetActive();
}
 
CNetworkSignalMonitor::CNetworkSignalMonitor(MNetworkSignalObserver& aObserver)
    : CActive(EPriorityStandard), iObserver(aObserver), iSigStrengthV1Pckg(iSigStrengthV1)
{
    CActiveScheduler::Add(this);
}

TInt32 CNetworkSignalMonitor::signalStrength() const
{
    TInt32 strength(0);
    switch (iSigStrengthV1.iBar)
        {
        case 0:
            strength = 0;
            break;
        case 1:
            strength = 14;
            break;
        case 2:
            strength = 29;
            break;
        case 3:
            strength = 43;
            break;
        case 4:
            strength = 57;
            break;
        case 5:
            strength = 72;
            break;
        case 6:
            strength = 86;
            break;
        case 7:
            strength = 100;
            break;
        default:
            strength = iSigStrengthV1.iBar;
            break;
        }
    return strength;
}
 
void CNetworkSignalMonitor::RunL()
{
    if (iStatus.Int() == KErrNone) {
        iTelephony->NotifyChange(iStatus, CTelephony::ESignalStrengthChange, iSigStrengthV1Pckg); 
        SetActive();
        iObserver.SignalStatusL(iSigStrengthV1.iSignalStrength, iSigStrengthV1.iBar);
    }
}
 
void CNetworkSignalMonitor::DoCancel()
{
    iTelephony->CancelAsync(CTelephony::ESignalStrengthChangeCancel);
}
