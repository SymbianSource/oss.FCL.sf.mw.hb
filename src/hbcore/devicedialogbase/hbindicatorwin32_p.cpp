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

#include <QtGlobal>
#include <QVariant>
#include <QTimerEvent>

#include "hbindicatorwin32_p.h"
#include "hbdevicedialogmanager_p.h"
#include "hbdevicedialogerrors_p.h"

extern HbDeviceDialogManager *managerInstance();
extern void releaseManagerInstance();

HbDeviceDialogManager *mDeviceDialogManager = 0;
// Indicators are implemented only for Symbian/S60 OS. All others use a stub which shows
// indicators in the calling process.
HbIndicatorPrivate::HbIndicatorPrivate()
: q_ptr(0), iLastError( HbDeviceDialogNoError ), iListening(false)
{
    mDeviceDialogManager = managerInstance();
}

HbIndicatorPrivate::~HbIndicatorPrivate()
{
    releaseManagerInstance();
}

void HbIndicatorPrivate::init()
{

}

bool HbIndicatorPrivate::activate(const QString &indicatorType, const QVariant &parameter)
{
    bool ok = true;
    int result = 0;

    const HbDeviceDialogServer::ClientCredentials cred(0);
    const QVariantMap securityCredentials;

    HbDeviceDialogServer::IndicatorParameters params(indicatorType, cred, parameter);
    result = mDeviceDialogManager->activateIndicator(params);

    if (q_ptr && q_ptr->receivers(SIGNAL(userActivated(QString, QVariantMap))) > 0) {
        connect(mDeviceDialogManager, SIGNAL(indicatorUserActivated(QVariantMap)),
                this, SLOT(indicatorUserActivated(QVariantMap)));
    }

    if (result != 0) {
        setError(result);
        ok = false;
    }
    return ok;
}

bool HbIndicatorPrivate::deactivate(const QString &indicatorType, const QVariant &parameter)
{
    const HbDeviceDialogServer::ClientCredentials cred(0);
    HbDeviceDialogServer::IndicatorParameters params(indicatorType, cred, parameter);
    mDeviceDialogManager->deactivateIndicator(params);
    mDeviceDialogManager->disconnect(this, SLOT(indicatorUserActivated(QVariantMap)));
    return true;
}

void HbIndicatorPrivate::indicatorUserActivated(const QVariantMap& data)
{    
    emit q_ptr->userActivated(data.value("type").toString(), data.value("data").toMap());
}

bool HbIndicatorPrivate::startListen()
{    
    connect( mDeviceDialogManager, SIGNAL( indicatorActivated(QList<IndicatorClientInfo>) ),
             this, SIGNAL(activated(QList<IndicatorClientInfo>)) );
    connect( mDeviceDialogManager, SIGNAL( indicatorUpdated(QList<IndicatorClientInfo>) ),
             this, SIGNAL(updated( QList<IndicatorClientInfo>) ) );
    connect( mDeviceDialogManager, SIGNAL( indicatorRemoved(QList<IndicatorClientInfo>) ),
             this, SIGNAL( deactivated(QList<IndicatorClientInfo>) ) );

    timer.start(1, this);
    iListening = true;
    return iListening;
}

bool HbIndicatorPrivate::listening() const
{
    return iListening;
}

bool HbIndicatorPrivate::stopListen()
{
    mDeviceDialogManager->disconnect(this, SIGNAL(activated(QList<IndicatorClientInfo>)));
    mDeviceDialogManager->disconnect(this, SIGNAL(deactivated(QList<IndicatorClientInfo>)));
    iListening = false;
    return true;
}

// Set error
void HbIndicatorPrivate::setError(int error)
{
    iLastError = error;
    emit this->error(iLastError);
}

// Return last error
int HbIndicatorPrivate::error() const
{
    return iLastError;
}

void HbIndicatorPrivate::timerEvent(QTimerEvent * event)
{
    killTimer(event->timerId());

    QList<IndicatorClientInfo> clientInfoList =
            mDeviceDialogManager->indicatorClientInfoList();

    if (clientInfoList.count() > 0) {
        emit allActivated(clientInfoList);
    }
}
