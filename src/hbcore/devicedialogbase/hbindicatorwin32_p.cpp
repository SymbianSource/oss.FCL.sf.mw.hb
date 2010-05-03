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
#include "hbindicatorpluginmanager_p.h"
#include "hbdevicedialogerrors_p.h"

HbIndicatorPluginManager *HbIndicatorPrivate::pluginManager()
{
    static HbIndicatorPluginManager *manager;
    if (!manager) {
        manager = new HbIndicatorPluginManager();
    }
    return manager;
}

// Indicators are implemented only for Symbian/S60 OS. All others use a stub which shows
// indicators in the calling process.
HbIndicatorPrivate::HbIndicatorPrivate()
: q_ptr(0), iLastError( HbDeviceDialogNoError ), iListening(false)
{
}

HbIndicatorPrivate::~HbIndicatorPrivate()
{
}

void HbIndicatorPrivate::init()
{

}

bool HbIndicatorPrivate::activate(const QString &indicatorType, const QVariant &parameter)
{
    bool ok = true;
    int result = 0;
    HbIndicatorPluginManager *pluginManager =
        HbIndicatorPrivate::pluginManager();
    QVariantMap securityCredentials;
    pluginManager->addIndicator(indicatorType, securityCredentials, &result);
    if (q_ptr && q_ptr->receivers(SIGNAL(userActivated(QString, QVariantMap))) > 0) {
        connect(pluginManager, SIGNAL(indicatorUserActivated(QVariantMap)), 
				this, SLOT(indicatorUserActivated(QVariantMap)));
    }

    if (result == 0) {
        pluginManager->activateIndicator(indicatorType, parameter, securityCredentials);
    } else {
        setError(result);
        ok = false;
    }
    return ok;
}

bool HbIndicatorPrivate::deactivate(const QString &indicatorType, const QVariant &parameter)
{
    pluginManager()->deactivateIndicator(indicatorType, parameter, QVariantMap());
    pluginManager()->disconnect(this, SLOT(indicatorUserActivated(QVariantMap)));
    return true;
}

void HbIndicatorPrivate::indicatorUserActivated(const QVariantMap& data)
{    
    emit q_ptr->userActivated(data.value("type").toString(), data.value("data").toMap());
}

bool HbIndicatorPrivate::startListen()
{
    HbIndicatorPluginManager *pluginManager =
            HbIndicatorPrivate::pluginManager();

    connect(pluginManager, SIGNAL(indicatorActivated(IndicatorClientInfo)),
            this, SLOT(indicatorActivated(IndicatorClientInfo)));
    connect(pluginManager, SIGNAL(indicatorRemoved(IndicatorClientInfo)),
            this, SLOT(indicatorRemoved(IndicatorClientInfo)));
    connect(pluginManager, SIGNAL(indicatorUpdated(IndicatorClientInfo)),
            this, SLOT(indicatorUpdated(IndicatorClientInfo)));

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
    HbIndicatorPluginManager *pluginManager =
            HbIndicatorPrivate::pluginManager();

    pluginManager->disconnect(this, SLOT(indicatorActivated(IndicatorClientInfo)));
    pluginManager->disconnect(this, SLOT(indicatorRemoved(IndicatorClientInfo)));
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

void HbIndicatorPrivate::indicatorActivated(
        const IndicatorClientInfo &clientInfo)
{
    QList<IndicatorClientInfo> clientInfoList;
    clientInfoList.append(clientInfo);
    emit activated(clientInfoList);
}
void HbIndicatorPrivate::indicatorRemoved(
        const IndicatorClientInfo &clientInfo)
{
    QList<IndicatorClientInfo> clientInfoList;
    clientInfoList.append(clientInfo);
    emit deactivated(clientInfoList);
}

void HbIndicatorPrivate::indicatorUpdated(const IndicatorClientInfo &clientInfo)
{
    QList<IndicatorClientInfo> clientInfoList;
    clientInfoList.append(clientInfo);
    emit updated(clientInfoList);
}

void HbIndicatorPrivate::timerEvent(QTimerEvent * event)
{
    killTimer(event->timerId());

    HbIndicatorPluginManager *pluginManager =
            HbIndicatorPrivate::pluginManager();

    QList<IndicatorClientInfo> clientInfoList =
            pluginManager->indicatorClientInfoList();

    if (clientInfoList.count() > 0) {
        emit allActivated(clientInfoList);
    }
}
