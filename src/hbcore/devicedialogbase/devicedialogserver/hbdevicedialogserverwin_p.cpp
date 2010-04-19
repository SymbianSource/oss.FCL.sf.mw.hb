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

#include "hbdevicedialogserver_p.h"
#include "hbdevicedialogserverwin_p_p.h"

/*!
    \class HbDeviceDialogServerPrivate
    \internal
*/

/*!
    \internal
 */
HbDeviceDialogServerPrivate::HbDeviceDialogServerPrivate()
{
    //do nothing
}

/*!
    \internal
 */
HbDeviceDialogServerPrivate::~HbDeviceDialogServerPrivate()
{
    //do nothing
}

/*!
    \internal
    Initialisation step. Starts a server with given name.
*/
void HbDeviceDialogServerPrivate::Init()
{
    //do nothing
}

/*!
    \internal
    Find correct session to send device dialog initiated update event to client.
*/
void HbDeviceDialogServerPrivate::deviceDialogUpdate( int id, const QVariantMap &data ) const
{
    Q_UNUSED(id)
    Q_UNUSED(data)
}

/*!
    \internal
    Find correct session to send device dialog initiated close event to client.
*/
void HbDeviceDialogServerPrivate::deviceDialogClosed( int identifier, int closeReason )
{
    Q_UNUSED(identifier)
    Q_UNUSED(closeReason)
}

/*!
    \internal
    Forward client initiated show event.
*/
int HbDeviceDialogServerPrivate::showDeviceDialog(
    HbDeviceDialogServer::DialogParameters &parameters)
{
    return q_func()->showDeviceDialog(parameters);
}

/*!
    \internal
    Forward client initiated update event.
*/
int HbDeviceDialogServerPrivate::updateDeviceDialog( int id, QVariantMap &data )
{
    return q_func()->updateDeviceDialog( id, data );
}

/*!
    \internal
    Forward client initiated close event.
*/
int HbDeviceDialogServerPrivate::closeDeviceDialog( int id )
{
    return q_func()->closeDeviceDialog(id);
}

/*!
    \internal
    activate indicator
*/
int HbDeviceDialogServerPrivate::activateIndicator(
    HbDeviceDialogServer::IndicatorParameters &parameters)
{
    return q_func()->activateIndicator(parameters);
}

/*!
    \internal
    deactivate indicator
*/
int HbDeviceDialogServerPrivate::deactivateIndicator(
    HbDeviceDialogServer::IndicatorParameters &parameters)
{
    return q_func()->deactivateIndicator(parameters);
}

/*!
    \internal
    return indicator info needed in client side.
*/
QList<IndicatorClientInfo> HbDeviceDialogServerPrivate::indicatorClientInfoList() const
{
    return q_func()->indicatorClientInfoList();
}

void HbDeviceDialogServerPrivate::IndicatorsActivated(
        const QList<IndicatorClientInfo> &indicatorInfoList)
{
    Q_UNUSED(indicatorInfoList)
}

void HbDeviceDialogServerPrivate::IndicatorsUpdated(
        const QList<IndicatorClientInfo> &indicatorInfoList)
{
    Q_UNUSED(indicatorInfoList)
}

void HbDeviceDialogServerPrivate::IndicatorsDeactivated(
        const QList<IndicatorClientInfo> &indicatorInfoList)
{
    Q_UNUSED(indicatorInfoList)
}

