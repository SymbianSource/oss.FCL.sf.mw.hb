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

#include <hbdevicedialogconnecthelper_p.h>
#include <hbdevicedialogconnecthelper_p_p.h>

/*!
    HbDeviceDialogConnectHelper
    \internal
*/


HbDeviceDialogConnectHelperPrivate::HbDeviceDialogConnectHelperPrivate(HbDeviceDialogConnectHelper *wrapper)
: CActive(CActive::EPriorityStandard), q_ptr(wrapper), mSessionConnected(0)
{
    CActiveScheduler::Add(this);    
}

HbDeviceDialogConnectHelperPrivate::~HbDeviceDialogConnectHelperPrivate()
{
    Cancel();
    if (mSessionConnected) {
        mClientSession.Close();
        mSessionConnected = false;
    }
}

void HbDeviceDialogConnectHelperPrivate::connect()
{
    if (!IsActive()) {
        iStatus = KRequestPending;
        if (mClientSession.Connect(&iStatus) == KErrNone) {
            SetActive();
        }
    }
}

void HbDeviceDialogConnectHelperPrivate::RunL()
{
    if (iStatus == KErrNone) {
        emit q_ptr->sessionEstablished(&mClientSession);
        mSessionConnected = true;
    }
}

void HbDeviceDialogConnectHelperPrivate::DoCancel() 
{
    TRequestStatus *rs = &iStatus;
    User::RequestComplete(rs, KErrNone);
}



HbDeviceDialogConnectHelper::HbDeviceDialogConnectHelper(QObject *parent)
: QObject(parent)
{
    d_ptr = new HbDeviceDialogConnectHelperPrivate(this);    
}

HbDeviceDialogConnectHelper::~HbDeviceDialogConnectHelper()
{
    delete d_ptr;
}

void HbDeviceDialogConnectHelper::connect()
{   
    d_ptr->connect();
}


