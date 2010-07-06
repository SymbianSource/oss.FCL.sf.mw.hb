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

#include <hbdevicedialoglaunchhelper_p.h>
#include <hbdevicedialogclientsession_p.h>
#include <hbdevicedialogserverdefs_p.h>
#include <apacmdln.h>
#include <e32cmn.h>

/*!
    HbDeviceDialogLaunchHelper
    \internal
*/
class HbDeviceDialogLaunchHelperPrivate : public CActive
    {
public:
    HbDeviceDialogLaunchHelperPrivate();
    ~HbDeviceDialogLaunchHelperPrivate();
    void Start();
    TInt Error() { return iError; }
    void StartServer();
        
protected:    
    void RunL();
    void DoCancel();
    TInt RunError(TInt aError);

public:    
    RProcess iProcess;
    CActiveSchedulerWait iWait;
    TInt iError;
    };
    
HbDeviceDialogLaunchHelperPrivate::HbDeviceDialogLaunchHelperPrivate()
: CActive(CActive::EPriorityStandard)
{    
    CActiveScheduler::Add(this);               
}

HbDeviceDialogLaunchHelperPrivate::~HbDeviceDialogLaunchHelperPrivate()
{
    if (iWait.IsStarted()) {
        iWait.AsyncStop();
    }
    Cancel();
}

void HbDeviceDialogLaunchHelperPrivate::Start()
{
    StartServer();
    
    if (iError == KErrNone) {
        SetActive();
        iWait.Start();            
    }
}

void HbDeviceDialogLaunchHelperPrivate::RunL()
{ 
    iError = iStatus.Int();
           
    if (iProcess.ExitType() == EExitPanic) {
        iError = KErrGeneral;
    }

    iProcess.Close();
       
    if (iWait.IsStarted()) {
        iWait.AsyncStop();
    }    
}

void HbDeviceDialogLaunchHelperPrivate::DoCancel()
{
    if (IsActive()) {        
        iProcess.RendezvousCancel(iStatus);
    }
}

TInt HbDeviceDialogLaunchHelperPrivate::RunError(TInt aError)
{
    iError = aError;
    return KErrNone;
}

void HbDeviceDialogLaunchHelperPrivate::StartServer()
{
    iError = KErrNone;

    if (RHbDeviceDialogClientSession::ServerRunning()) {
        // Already running, return error.
        iError = KErrAlreadyExists; 
        return;
    }        

    // Create the server process
    iError = iProcess.Create(KHbServerExe, KNullDesC);

    if (iError != KErrNone) {
        // Process could not be created, return error.
        return;
        }
    
    CApaCommandLine* apaCommandLine = 0;
    TRAP(iError, apaCommandLine = CApaCommandLine::NewL());
    if (iError != KErrNone) {
        iProcess.Close();
        return;
    }
    TRAP(iError,
        apaCommandLine->SetExecutableNameL(KHbServerExe);
        apaCommandLine->SetCommandL(EApaCommandBackground);
        apaCommandLine->SetProcessEnvironmentL(iProcess);
    );
    delete apaCommandLine;
    apaCommandLine = 0;

    if (iError != KErrNone) {
        iProcess.Close();
        return;
    }
    
    iProcess.Rendezvous(iStatus);
    iProcess.Resume();    // logon OK - start the server     
}

HbDeviceDialogLaunchHelper* HbDeviceDialogLaunchHelper::NewLC()
{
     HbDeviceDialogLaunchHelper* self = new (ELeave) HbDeviceDialogLaunchHelper;
     CleanupStack::PushL(self);
     self->d_ptr = new (ELeave) HbDeviceDialogLaunchHelperPrivate();
     return self;    
}

HbDeviceDialogLaunchHelper* HbDeviceDialogLaunchHelper::NewL()
{
    HbDeviceDialogLaunchHelper* self = NewLC();
    CleanupStack::Pop(self);
    return self;
}
    
HbDeviceDialogLaunchHelper::~HbDeviceDialogLaunchHelper()
{
    delete d_ptr;
}

void HbDeviceDialogLaunchHelper::Start()
{
    d_ptr->Start();
}

TInt HbDeviceDialogLaunchHelper::Error() 
{
    return d_ptr->iError; 
}

HbDeviceDialogLaunchHelper::HbDeviceDialogLaunchHelper()
{    
}
