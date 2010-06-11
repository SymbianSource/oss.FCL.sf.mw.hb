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

#include "hbsleepmodelistener_p.h"
#include "hbsleepmodelistener_p_p.h"
#include <hbinstance.h>
#include <hbevent.h>

#if defined(Q_OS_SYMBIAN)

#include <hwrmlight.h>
#include <hwrmextendedlight.h>
#include "hbcorepskeys_p.h"

// app uid of sleepmode application
const TUid KSleepModeProperty = {0x101F7A01};
const TInt KSleepModeOn = 0x00002001;

// Maximum number of possible lights targets
const TInt KHWRMLightMaxTargets(31);

// First target value in targets enumeration
const TInt KHWRMLightFirstTarget(CHWRMExtendedLight::EPrimaryDisplay);

CSleepModeListenerPrivate::~CSleepModeListenerPrivate()
{
    Cancel();
    sleepModeState.Close();
}

CSleepModeListenerPrivate::CSleepModeListenerPrivate()
    : CActive(EPriorityNormal)
{
    User::LeaveIfError(sleepModeState.Attach(KSleepModeProperty, KSleepModeOn));
    CActiveScheduler::Add(this);
    // initial subscription
    sleepModeState.Subscribe(iStatus);
    SetActive();
}

void CSleepModeListenerPrivate::RunL()
{
    sleepModeState.Subscribe(iStatus);
    SetActive();
    CHWRMLight::TLightStatus status(CHWRMLight::ELightStatusUnknown);
    RLightStatusArray currentStatus(KHWRMLightMaxTargets);
    TInt err = InitializeStatusArray(currentStatus);
    if (err == KErrNone) {
        TInt arraySize = sizeof(THWRMStatusInfo) * KHWRMLightMaxTargets;
        TPtr8 arrayPtr((TUint8 *)&currentStatus[0], arraySize, arraySize);
        err = sleepModeState.Get(arrayPtr);
        if (err == KErrNone) {
            TInt index = currentStatus.FindInOrder(KHWRMLightFirstTarget, FindByTarget);
            if (index >= 0 && index < KHWRMLightMaxTargets) {
                status = static_cast<CHWRMLight::TLightStatus>(currentStatus[index].iStatus);
                RProcess process;
                //If prosess is something else than themeserver
                if (process.SecureId().iId != KHbPsHardwareCoarseOrientationCategoryUid.iUid) {
                    QList<HbMainWindow *> mainWindowList = hbInstance->allMainWindows();
                    for (int i = 0; i < mainWindowList.count(); ++i) {
                        if (status == CHWRMLight::ELightOff) {
                            mainWindowList[i]->broadcastEvent(HbEvent::SleepModeEnter);
                        } else {
                            mainWindowList[i]->broadcastEvent(HbEvent::SleepModeExit);
                        }
                    }
                } else {
                    HbEvent event(status == CHWRMLight::ELightOff ? HbEvent::SleepModeEnter : HbEvent::SleepModeExit);
                    QCoreApplication::sendEvent(qApp, &event);
                }
            }
        }
    }

    // Clean up
    currentStatus.Close();
}

void CSleepModeListenerPrivate::DoCancel()
{
    sleepModeState.Cancel();
}

TInt CSleepModeListenerPrivate::InitializeStatusArray(
    RLightStatusArray &aArray) const
{
    TInt err = KErrNone;
    TInt currentTarget(KHWRMLightFirstTarget);
    for (TInt i = 0; i < KHWRMLightMaxTargets; ++i) {
        THWRMStatusInfo info;
        info.iTarget = currentTarget;
        info.iStatus = CHWRMLight::ELightStatusUnknown;
        err = aArray.Append(info);
        if (err != KErrNone) {
            break;
        }

        // shift the mask and target ready for next loop
        currentTarget <<= 1;
    }

    return err;
}

TInt CSleepModeListenerPrivate::FindByTarget(const TInt *aTarget,
        const THWRMStatusInfo &aItem)
{
    if (*aTarget < aItem.iTarget) {
        return -1;
    } else if (*aTarget > aItem.iTarget) {
        return 1;
    }
    return 0;
}

#else

HbSleepModeListenerPrivate::HbSleepModeListenerPrivate()
{

}

HbSleepModeListenerPrivate::~HbSleepModeListenerPrivate()
{

}

#endif

/*!
    Returns static instance
 */
HbSleepModeListener *HbSleepModeListener::instance()
{
    static HbSleepModeListener theInstance;
    return &theInstance;
}

/*!
    Constructor
*/
HbSleepModeListener::HbSleepModeListener() : d(0)
{
#if defined(Q_OS_SYMBIAN)

    d = new CSleepModeListenerPrivate();

#else

    d = new HbSleepModeListenerPrivate();

#endif
}

/*!
    Destructor
*/
HbSleepModeListener::~HbSleepModeListener()
{
    delete d;
}
