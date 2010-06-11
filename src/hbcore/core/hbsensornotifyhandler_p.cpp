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

// These active objects observe the changing of the sensor cenrep value
// and notify the sensorlistener when the change has ocurred.

#include "hbsensornotifyhandler_p.h"
#include "hbcommoncrkeys.h"
#include <centralrepository.h>
#include <QtGlobal>

HbSensorNotifyHandler *HbSensorNotifyHandler::NewL(HbSensorListener &aObserver)
{
    HbSensorNotifyHandler *self = new(ELeave) HbSensorNotifyHandler(aObserver);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
}

HbSensorNotifyHandler::HbSensorNotifyHandler(HbSensorListener &aObserver)
    : CActive(EPriorityStandard), mObserver(aObserver)
{
}

void HbSensorNotifyHandler::ConstructL()
{
    TRAPD(err, mRepository = CRepository::NewL(KHbSensorCenrepUid));
    if (err) {
        qWarning("mRepository::NewL fails, error code = %d", err);
    }
    CActiveScheduler::Add(this);
}

HbSensorNotifyHandler::~HbSensorNotifyHandler()
{
    Cancel(); //first cancel because iRepository is used there
    delete mRepository;
}

void HbSensorNotifyHandler::startObservingL()
{
    if (IsActive()) {
        return; // do nothing if already observing
    }
    User::LeaveIfError(mRepository->NotifyRequest(KHbSensorCenrepKey, iStatus));
    SetActive();
}

void HbSensorNotifyHandler::stopObserving()
{
    Cancel();
}

void HbSensorNotifyHandler::DoCancel()
{
    mRepository->NotifyCancel(KHbSensorCenrepKey);
}

void HbSensorNotifyHandler::RunL()
{
    TInt value = 0;
    TInt error = mRepository->Get(KHbSensorCenrepKey, value);
    if (error == KErrNone) {
        mObserver.cenrepValueChanged(value);
    }
    // Re-subscribe
    error = mRepository->NotifyRequest(KHbSensorCenrepKey, iStatus);
    if (error == KErrNone) {
        SetActive();
    }
}

